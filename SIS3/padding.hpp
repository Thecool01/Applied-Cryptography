#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <random>
#include <algorithm>
#include "sha256.hpp"

/**
 * Padding Schemes for RSA
 *  - PKCS#1 v1.5  (encryption and signature)
 *  - OAEP         (encryption, RFC 8017)
 *  - PSS          (signatures, RFC 8017) — BONUS
 */

// ──────────────────────────────────────────────────────────────────
// Secure random byte generator (uses std::random_device + mt19937)
// ──────────────────────────────────────────────────────────────────
inline std::vector<uint8_t> randomBytes(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<uint8_t> out(count);
    for (auto& b : out) b = static_cast<uint8_t>(dist(gen));
    return out;
}

// Same but non-zero bytes only (used in PKCS#1 v1.5)
inline std::vector<uint8_t> randomNonZeroBytes(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 255);
    std::vector<uint8_t> out(count);
    for (auto& b : out) b = static_cast<uint8_t>(dist(gen));
    return out;
}

// ──────────────────────────────────────────────────────────────────
// PKCS#1 v1.5
// ──────────────────────────────────────────────────────────────────
class PKCS1v15 {
public:
    /**
     * Encryption padding (type 0x02)
     * EM = 0x00 || 0x02 || PS || 0x00 || M
     * PS = k - mLen - 3 random non-zero bytes (min 8 bytes)
     */
    static std::vector<uint8_t> encPad(const std::vector<uint8_t>& M, size_t k) {
        if (M.size() > k - 11)
            throw std::invalid_argument("PKCS1v15: message too long");
        size_t psLen = k - M.size() - 3;
        auto PS = randomNonZeroBytes(psLen);
        std::vector<uint8_t> EM;
        EM.reserve(k);
        EM.push_back(0x00);
        EM.push_back(0x02);
        EM.insert(EM.end(), PS.begin(), PS.end());
        EM.push_back(0x00);
        EM.insert(EM.end(), M.begin(), M.end());
        return EM;
    }

    /**
     * Encryption unpadding
     * Returns the message M, throws on malformed padding
     */
    static std::vector<uint8_t> encUnpad(const std::vector<uint8_t>& EM) {
        if (EM.size() < 11 || EM[0] != 0x00 || EM[1] != 0x02)
            throw std::runtime_error("PKCS1v15: invalid padding");
        size_t i = 2;
        while (i < EM.size() && EM[i] != 0x00) ++i;
        if (i < 10 || i == EM.size())
            throw std::runtime_error("PKCS1v15: padding separator not found");
        ++i; // skip the 0x00 separator
        return std::vector<uint8_t>(EM.begin() + i, EM.end());
    }

    /**
     * Signature padding (type 0x01)
     * EM = 0x00 || 0x01 || PS || 0x00 || T
     * PS = 0xff bytes
     * T  = DER-encoded DigestInfo (ASN.1 prefix for SHA-256 + hash)
     */
    static std::vector<uint8_t> sigPad(const std::vector<uint8_t>& digest, size_t k) {
        // ASN.1 / DER prefix for SHA-256: OID 2.16.840.1.101.3.4.2.1
        static const uint8_t SHA256_ASN1_PREFIX[] = {
            0x30, 0x31,                   // SEQUENCE (49 bytes)
            0x30, 0x0d,                   // SEQUENCE (13 bytes)
            0x06, 0x09,                   // OID (9 bytes)
            0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,
            0x05, 0x00,                   // NULL
            0x04, 0x20                    // OCTET STRING (32 bytes)
        };
        std::vector<uint8_t> T(std::begin(SHA256_ASN1_PREFIX), std::end(SHA256_ASN1_PREFIX));
        T.insert(T.end(), digest.begin(), digest.end());

        if (k < T.size() + 11)
            throw std::invalid_argument("PKCS1v15: key too short for signature");
        size_t psLen = k - T.size() - 3;

        std::vector<uint8_t> EM;
        EM.reserve(k);
        EM.push_back(0x00);
        EM.push_back(0x01);
        EM.insert(EM.end(), psLen, 0xff);
        EM.push_back(0x00);
        EM.insert(EM.end(), T.begin(), T.end());
        return EM;
    }

    /**
     * Signature unpadding / verification
     * Returns the embedded T (DigestInfo), throws if malformed
     */
    static std::vector<uint8_t> sigUnpad(const std::vector<uint8_t>& EM) {
        if (EM.size() < 11 || EM[0] != 0x00 || EM[1] != 0x01)
            throw std::runtime_error("PKCS1v15 sig: invalid padding header");
        size_t i = 2;
        while (i < EM.size() && EM[i] == 0xff) ++i;
        if (EM[i] != 0x00)
            throw std::runtime_error("PKCS1v15 sig: padding terminator missing");
        ++i;
        return std::vector<uint8_t>(EM.begin() + i, EM.end());
    }

    // Extract the 32-byte SHA-256 digest from a DigestInfo T blob
    static std::vector<uint8_t> extractDigest(const std::vector<uint8_t>& T) {
        // prefix is 19 bytes, then 32 bytes of hash
        if (T.size() < 19 + 32)
            throw std::runtime_error("PKCS1v15: DigestInfo too short");
        return std::vector<uint8_t>(T.end() - 32, T.end());
    }
};

// ──────────────────────────────────────────────────────────────────
// OAEP (Optimal Asymmetric Encryption Padding) — RFC 8017 §7.1
// Hash = SHA-256, MGF = MGF1-SHA-256, Label = ""
// ──────────────────────────────────────────────────────────────────
class OAEP {
public:
    static constexpr size_t HASH_LEN = 32; // SHA-256

    /**
     * MGF1 mask generation function with SHA-256
     */
    static std::vector<uint8_t> mgf1(const std::vector<uint8_t>& seed, size_t maskLen) {
        std::vector<uint8_t> mask;
        for (uint32_t counter = 0; mask.size() < maskLen; ++counter) {
            std::vector<uint8_t> input = seed;
            // Append counter as big-endian 4 bytes
            input.push_back((counter >> 24) & 0xff);
            input.push_back((counter >> 16) & 0xff);
            input.push_back((counter >>  8) & 0xff);
            input.push_back( counter        & 0xff);
            auto h = SHA256::hash(input);
            mask.insert(mask.end(), h.begin(), h.end());
        }
        mask.resize(maskLen);
        return mask;
    }

    /**
     * OAEP Encoding
     * EM = 0x00 || maskedSeed || maskedDB
     * DB = lHash || PS (0x00...) || 0x01 || M
     */
    static std::vector<uint8_t> pad(const std::vector<uint8_t>& M, size_t k,
                                    const std::vector<uint8_t>& label = {}) {
        size_t mLen = M.size();
        if (mLen > k - 2*HASH_LEN - 2)
            throw std::invalid_argument("OAEP: message too long");

        // lHash = SHA-256(label)
        auto lHash = SHA256::hash(label);

        // DB = lHash || PS || 0x01 || M
        size_t dbLen = k - HASH_LEN - 1;
        std::vector<uint8_t> DB;
        DB.insert(DB.end(), lHash.begin(), lHash.end());
        size_t psLen = dbLen - mLen - HASH_LEN - 1;
        DB.insert(DB.end(), psLen, 0x00);
        DB.push_back(0x01);
        DB.insert(DB.end(), M.begin(), M.end());

        // Random seed
        auto seed = randomBytes(HASH_LEN);

        // maskedDB = DB xor MGF1(seed, dbLen)
        auto dbMask = mgf1(seed, dbLen);
        std::vector<uint8_t> maskedDB(dbLen);
        for (size_t i = 0; i < dbLen; ++i)
            maskedDB[i] = DB[i] ^ dbMask[i];

        // maskedSeed = seed xor MGF1(maskedDB, hLen)
        auto seedMask = mgf1(maskedDB, HASH_LEN);
        std::vector<uint8_t> maskedSeed(HASH_LEN);
        for (size_t i = 0; i < HASH_LEN; ++i)
            maskedSeed[i] = seed[i] ^ seedMask[i];

        // EM = 0x00 || maskedSeed || maskedDB
        std::vector<uint8_t> EM;
        EM.push_back(0x00);
        EM.insert(EM.end(), maskedSeed.begin(), maskedSeed.end());
        EM.insert(EM.end(), maskedDB.begin(), maskedDB.end());
        return EM;
    }

    /**
     * OAEP Decoding
     */
    static std::vector<uint8_t> unpad(const std::vector<uint8_t>& EM, size_t k,
                                      const std::vector<uint8_t>& label = {}) {
        if (EM.size() != k || k < 2*HASH_LEN + 2)
            throw std::runtime_error("OAEP: invalid ciphertext length");
        if (EM[0] != 0x00)
            throw std::runtime_error("OAEP: leading byte not zero");

        auto lHash = SHA256::hash(label);
        size_t dbLen = k - HASH_LEN - 1;

        std::vector<uint8_t> maskedSeed(EM.begin()+1,        EM.begin()+1+HASH_LEN);
        std::vector<uint8_t> maskedDB  (EM.begin()+1+HASH_LEN, EM.end());

        // Recover seed
        auto seedMask = mgf1(maskedDB, HASH_LEN);
        std::vector<uint8_t> seed(HASH_LEN);
        for (size_t i = 0; i < HASH_LEN; ++i)
            seed[i] = maskedSeed[i] ^ seedMask[i];

        // Recover DB
        auto dbMask = mgf1(seed, dbLen);
        std::vector<uint8_t> DB(dbLen);
        for (size_t i = 0; i < dbLen; ++i)
            DB[i] = maskedDB[i] ^ dbMask[i];

        // Verify lHash
        if (!std::equal(lHash.begin(), lHash.end(), DB.begin()))
            throw std::runtime_error("OAEP: label hash mismatch");

        // Find 0x01 separator
        size_t i = HASH_LEN;
        while (i < DB.size() && DB[i] == 0x00) ++i;
        if (i == DB.size() || DB[i] != 0x01)
            throw std::runtime_error("OAEP: separator byte 0x01 not found");

        return std::vector<uint8_t>(DB.begin() + i + 1, DB.end());
    }
};

// ──────────────────────────────────────────────────────────────────
// PSS (Probabilistic Signature Scheme) — BONUS — RFC 8017 §9.1
// Hash = SHA-256, MGF = MGF1-SHA-256, sLen = 32
// ──────────────────────────────────────────────────────────────────
class PSS {
public:
    static constexpr size_t HASH_LEN = 32;
    static constexpr size_t SALT_LEN = 32;

    /**
     * PSS encoding
     * EM = maskedDB || H || 0xbc
     */
    static std::vector<uint8_t> encode(const std::vector<uint8_t>& mHash, size_t emBits) {
        size_t emLen = (emBits + 7) / 8;
        if (emLen < HASH_LEN + SALT_LEN + 2)
            throw std::invalid_argument("PSS: encoding too long");

        auto salt = randomBytes(SALT_LEN);

        // M' = 0x00*8 || mHash || salt
        std::vector<uint8_t> Mprime(8, 0x00);
        Mprime.insert(Mprime.end(), mHash.begin(), mHash.end());
        Mprime.insert(Mprime.end(), salt.begin(),  salt.end());
        auto H = SHA256::hash(Mprime);

        // DB = PS || 0x01 || salt
        size_t dbLen = emLen - HASH_LEN - 1;
        std::vector<uint8_t> DB(dbLen - SALT_LEN - 1, 0x00);
        DB.push_back(0x01);
        DB.insert(DB.end(), salt.begin(), salt.end());

        // maskedDB = DB xor MGF1(H, dbLen)
        auto dbMask = OAEP::mgf1(H, dbLen);
        std::vector<uint8_t> maskedDB(dbLen);
        for (size_t i = 0; i < dbLen; ++i)
            maskedDB[i] = DB[i] ^ dbMask[i];

        // Clear top bits
        uint8_t topMask = 0xff >> (8*emLen - emBits);
        maskedDB[0] &= topMask;

        std::vector<uint8_t> EM;
        EM.insert(EM.end(), maskedDB.begin(), maskedDB.end());
        EM.insert(EM.end(), H.begin(), H.end());
        EM.push_back(0xbc);
        return EM;
    }

    /**
     * PSS verification
     */
    static bool verify(const std::vector<uint8_t>& mHash,
                       const std::vector<uint8_t>& EM, size_t emBits) {
        size_t emLen = (emBits + 7) / 8;
        if (emLen < HASH_LEN + SALT_LEN + 2) return false;
        if (EM.back() != 0xbc) return false;

        size_t dbLen = emLen - HASH_LEN - 1;
        std::vector<uint8_t> maskedDB(EM.begin(), EM.begin() + dbLen);
        std::vector<uint8_t> H(EM.begin() + dbLen, EM.end() - 1);

        // Check top bits are zero
        uint8_t topMask = 0xff >> (8*emLen - emBits);
        if (maskedDB[0] & ~topMask) return false;

        // Recover DB
        auto dbMask = OAEP::mgf1(H, dbLen);
        std::vector<uint8_t> DB(dbLen);
        for (size_t i = 0; i < dbLen; ++i)
            DB[i] = maskedDB[i] ^ dbMask[i];
        DB[0] &= topMask;

        // Check PS and 0x01
        size_t i = dbLen - SALT_LEN - 1;
        if (DB[i] != 0x01) return false;
        std::vector<uint8_t> salt(DB.begin() + i + 1, DB.end());

        // Recompute H
        std::vector<uint8_t> Mprime(8, 0x00);
        Mprime.insert(Mprime.end(), mHash.begin(), mHash.end());
        Mprime.insert(Mprime.end(), salt.begin(), salt.end());
        auto Hprime = SHA256::hash(Mprime);

        return H == Hprime;
    }
};
