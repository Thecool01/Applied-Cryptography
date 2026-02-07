#pragma once
#include "aes.h"
#include "CustomRNG.h"
#include "Padding.h"
#include "GCM_Math.h"

#include <vector>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

// Helper to convert big-endian integer to bytes
static inline void store_be_uint64(uint8_t* ptr, uint64_t val) {
    for (int i = 0; i < 8; i++) ptr[i] = (uint8_t)((val >> (56 - 8 * i)) & 0xFF);
}

// Helper to increment 32-bit counter in a 16-byte block (last 4 bytes)
static inline void inc32(uint8_t block[16]) {
    uint32_t c = ((uint32_t)block[12] << 24) | ((uint32_t)block[13] << 16) |
                 ((uint32_t)block[14] << 8)  | ((uint32_t)block[15]);
    c++;
    block[12] = (uint8_t)((c >> 24) & 0xFF);
    block[13] = (uint8_t)((c >> 16) & 0xFF);
    block[14] = (uint8_t)((c >> 8) & 0xFF);
    block[15] = (uint8_t)(c & 0xFF);
}

class AESModes {
private:
    aes::KeySchedule ks;
    CustomRNG rng;

public:
    AESModes(const uint8_t* key, size_t keyLen) {
        ks = aes::expandKey(key, keyLen);
    }

    // ================== ECB MODE ==================
    std::vector<uint8_t> encryptECB(const std::vector<uint8_t>& plaintext) {
        auto data = PKCS7::pad(plaintext);
        std::vector<uint8_t> ciphertext;
        ciphertext.reserve(data.size());

        uint8_t in[16], out[16];
        for (size_t i = 0; i < data.size(); i += 16) {
            std::memcpy(in, &data[i], 16);
            aes::encryptBlock(in, out, ks);
            ciphertext.insert(ciphertext.end(), out, out + 16);
        }
        return ciphertext;
    }

    std::vector<uint8_t> decryptECB(const std::vector<uint8_t>& ciphertext) {
        if (ciphertext.size() % 16 != 0) throw std::invalid_argument("ECB ciphertext length invalid");

        std::vector<uint8_t> decrypted;
        decrypted.reserve(ciphertext.size());

        uint8_t in[16], out[16];
        for (size_t i = 0; i < ciphertext.size(); i += 16) {
            std::memcpy(in, &ciphertext[i], 16);
            aes::decryptBlock(in, out, ks);
            decrypted.insert(decrypted.end(), out, out + 16);
        }
        return PKCS7::unpad(decrypted);
    }

    // ================== CBC MODE ==================
    std::vector<uint8_t> encryptCBC(const std::vector<uint8_t>& plaintext) {
        auto data = PKCS7::pad(plaintext);

        uint8_t iv[16];
        rng.getBytes(iv, 16);

        std::vector<uint8_t> ciphertext;
        ciphertext.reserve(16 + data.size());
        ciphertext.insert(ciphertext.end(), iv, iv + 16);

        uint8_t prev[16], curr[16], out[16];
        std::memcpy(prev, iv, 16);

        for (size_t i = 0; i < data.size(); i += 16) {
            std::memcpy(curr, &data[i], 16);
            for (int j = 0; j < 16; j++) curr[j] ^= prev[j];

            aes::encryptBlock(curr, out, ks);
            ciphertext.insert(ciphertext.end(), out, out + 16);
            std::memcpy(prev, out, 16);
        }
        return ciphertext;
    }

    std::vector<uint8_t> decryptCBC(const std::vector<uint8_t>& ciphertext) {
        if (ciphertext.size() < 16 || (ciphertext.size() % 16) != 0) {
            throw std::invalid_argument("Invalid CBC ciphertext");
        }

        uint8_t prev[16], curr[16], out[16];
        std::memcpy(prev, ciphertext.data(), 16); // IV

        std::vector<uint8_t> decrypted;
        decrypted.reserve(ciphertext.size() - 16);

        for (size_t i = 16; i < ciphertext.size(); i += 16) {
            std::memcpy(curr, &ciphertext[i], 16);
            aes::decryptBlock(curr, out, ks);

            for (int j = 0; j < 16; j++) out[j] ^= prev[j];

            decrypted.insert(decrypted.end(), out, out + 16);
            std::memcpy(prev, curr, 16);
        }
        return PKCS7::unpad(decrypted);
    }

    // ================== CTR MODE ==================
    std::vector<uint8_t> ctrCore(const std::vector<uint8_t>& input, const uint8_t* nonce) {
        std::vector<uint8_t> output;
        output.reserve(input.size());

        uint8_t counterBlock[16];
        uint8_t keystream[16];

        std::memcpy(counterBlock, nonce, 12);
        std::memset(counterBlock + 12, 0, 4);

        for (size_t i = 0; i < input.size(); i++) {
            if (i % 16 == 0) {
                aes::encryptBlock(counterBlock, keystream, ks);
                inc32(counterBlock);
            }
            output.push_back((uint8_t)(input[i] ^ keystream[i % 16]));
        }
        return output;
    }

    std::vector<uint8_t> encryptCTR(const std::vector<uint8_t>& plaintext) {
        uint8_t nonce[12];
        rng.getBytes(nonce, 12);

        std::vector<uint8_t> ciphertext;
        ciphertext.reserve(12 + plaintext.size());
        ciphertext.insert(ciphertext.end(), nonce, nonce + 12);

        auto processed = ctrCore(plaintext, nonce);
        ciphertext.insert(ciphertext.end(), processed.begin(), processed.end());
        return ciphertext;
    }

    std::vector<uint8_t> decryptCTR(const std::vector<uint8_t>& ciphertext) {
        if (ciphertext.size() < 12) throw std::invalid_argument("CTR ciphertext too short");

        uint8_t nonce[12];
        std::memcpy(nonce, ciphertext.data(), 12);

        std::vector<uint8_t> data(ciphertext.begin() + 12, ciphertext.end());
        return ctrCore(data, nonce);
    }

    // ================== GCM MODE (AEAD) ==================
    // Output: IV(12) || Ciphertext || Tag(16)
    std::vector<uint8_t> encryptGCM(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& aad) {
        // H = AES_K(0^128)
        Block128 H;
        uint8_t zeroBlock[16] = {0};
        aes::encryptBlock(zeroBlock, H.bytes, ks);

        // IV (12)
        uint8_t iv[12];
        rng.getBytes(iv, 12);

        // J0 = IV || 0x00000001
        uint8_t J0[16];
        std::memcpy(J0, iv, 12);
        J0[12]=0; J0[13]=0; J0[14]=0; J0[15]=1;

        // CTR start at inc32(J0)
        uint8_t counterBlock[16];
        std::memcpy(counterBlock, J0, 16);
        inc32(counterBlock);

        std::vector<uint8_t> ciphertext;
        ciphertext.reserve(plaintext.size());

        uint8_t keystream[16];
        for (size_t i = 0; i < plaintext.size(); i++) {
            if (i % 16 == 0) {
                aes::encryptBlock(counterBlock, keystream, ks);
                inc32(counterBlock);
            }
            ciphertext.push_back((uint8_t)(plaintext[i] ^ keystream[i % 16]));
        }

        // GHASH(AAD, C)
        Block128 ghashState;

        // AAD blocks
        for (size_t i = 0; i < aad.size(); i += 16) {
            Block128 block;
            size_t copyLen = std::min((size_t)16, aad.size() - i);
            std::memcpy(block.bytes, &aad[i], copyLen);

            ghashState.XOR(block);
            ghashState = GCM_Math::gf128_multiply(ghashState, H);
        }

        // Ciphertext blocks
        for (size_t i = 0; i < ciphertext.size(); i += 16) {
            Block128 block;
            size_t copyLen = std::min((size_t)16, ciphertext.size() - i);
            std::memcpy(block.bytes, &ciphertext[i], copyLen);

            ghashState.XOR(block);
            ghashState = GCM_Math::gf128_multiply(ghashState, H);
        }

        // Length block: [len(AAD)*8 || len(C)*8]
        Block128 lenBlock;
        store_be_uint64(lenBlock.bytes,     (uint64_t)aad.size() * 8);
        store_be_uint64(lenBlock.bytes + 8, (uint64_t)ciphertext.size() * 8);

        ghashState.XOR(lenBlock);
        ghashState = GCM_Math::gf128_multiply(ghashState, H);

        // Tag = AES_K(J0) XOR GHASH
        uint8_t encryptedJ0[16];
        aes::encryptBlock(J0, encryptedJ0, ks);
        for (int i = 0; i < 16; i++) ghashState.bytes[i] ^= encryptedJ0[i];

        // result = IV || C || TAG
        std::vector<uint8_t> result;
        result.reserve(12 + ciphertext.size() + 16);
        result.insert(result.end(), iv, iv + 12);
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        result.insert(result.end(), ghashState.bytes, ghashState.bytes + 16);

        return result;
    }

    std::vector<uint8_t> decryptGCM(const std::vector<uint8_t>& input, const std::vector<uint8_t>& aad) {
        if (input.size() < 12 + 16) throw std::runtime_error("GCM input too short");

        // IV
        uint8_t iv[12];
        std::memcpy(iv, input.data(), 12);

        // Ciphertext and tag
        std::vector<uint8_t> ciphertext(input.begin() + 12, input.end() - 16);
        uint8_t receivedTag[16];
        std::memcpy(receivedTag, input.data() + input.size() - 16, 16);

        // H = AES_K(0^128)
        Block128 H;
        uint8_t zeroBlock[16] = {0};
        aes::encryptBlock(zeroBlock, H.bytes, ks);

        // J0 = IV || 0x00000001
        uint8_t J0[16];
        std::memcpy(J0, iv, 12);
        J0[12]=0; J0[13]=0; J0[14]=0; J0[15]=1;

        // GHASH(AAD, C)
        Block128 ghashState;

        for (size_t i = 0; i < aad.size(); i += 16) {
            Block128 block;
            size_t copyLen = std::min((size_t)16, aad.size() - i);
            std::memcpy(block.bytes, &aad[i], copyLen);
            ghashState.XOR(block);
            ghashState = GCM_Math::gf128_multiply(ghashState, H);
        }

        for (size_t i = 0; i < ciphertext.size(); i += 16) {
            Block128 block;
            size_t copyLen = std::min((size_t)16, ciphertext.size() - i);
            std::memcpy(block.bytes, &ciphertext[i], copyLen);
            ghashState.XOR(block);
            ghashState = GCM_Math::gf128_multiply(ghashState, H);
        }

        Block128 lenBlock;
        store_be_uint64(lenBlock.bytes,     (uint64_t)aad.size() * 8);
        store_be_uint64(lenBlock.bytes + 8, (uint64_t)ciphertext.size() * 8);
        ghashState.XOR(lenBlock);
        ghashState = GCM_Math::gf128_multiply(ghashState, H);

        // expectedTag = AES_K(J0) XOR GHASH
        uint8_t encryptedJ0[16];
        aes::encryptBlock(J0, encryptedJ0, ks);
        for (int i = 0; i < 16; i++) ghashState.bytes[i] ^= encryptedJ0[i];

        if (std::memcmp(ghashState.bytes, receivedTag, 16) != 0) {
            throw std::runtime_error("GCM Auth Failed: Invalid Tag");
        }

        // Decrypt (CTR with counter starting at inc32(J0))
        uint8_t counterBlock[16];
        std::memcpy(counterBlock, J0, 16);
        inc32(counterBlock);

        std::vector<uint8_t> plaintext;
        plaintext.reserve(ciphertext.size());

        uint8_t keystream[16];
        for (size_t i = 0; i < ciphertext.size(); i++) {
            if (i % 16 == 0) {
                aes::encryptBlock(counterBlock, keystream, ks);
                inc32(counterBlock);
            }
            plaintext.push_back((uint8_t)(ciphertext[i] ^ keystream[i % 16]));
        }

        return plaintext;
    }
};
