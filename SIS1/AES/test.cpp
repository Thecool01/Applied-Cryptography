#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "aes.h"

// --- HEX utils ---
static int hexVal(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::vector<uint8_t> hexToBytes(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::invalid_argument("Hex length must be even");
    std::vector<uint8_t> out(hex.size()/2);
    for (size_t i = 0; i < out.size(); i++) {
        int hi = hexVal(hex[2*i]);
        int lo = hexVal(hex[2*i+1]);
        if (hi < 0 || lo < 0) throw std::invalid_argument("Invalid hex");
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return out;
}

static std::string bytesToHex(const uint8_t* data, size_t n) {
    static const char* H = "0123456789abcdef";
    std::string s;
    s.reserve(n*2);
    for (size_t i = 0; i < n; i++) {
        s.push_back(H[data[i] >> 4]);
        s.push_back(H[data[i] & 0x0f]);
    }
    return s;
}

static void run_single_test(const std::string& keyHex,
                            const std::string& ptHex,
                            const std::string& expectedCtHex) {
    auto key = hexToBytes(keyHex);
    auto pt  = hexToBytes(ptHex);

    if (pt.size() != 16) throw std::runtime_error("Plaintext must be 16 bytes");

    auto ks = aes::expandKey(key.data(), key.size());

    uint8_t ct[16]{};
    uint8_t dec[16]{};

    aes::encryptBlock(pt.data(), ct, ks);
    aes::decryptBlock(ct, dec, ks);

    std::string gotCt = bytesToHex(ct, 16);
    std::string gotDec = bytesToHex(dec, 16);

    std::cout << "KEY: " << keyHex << "\n";
    std::cout << "PT : " << ptHex  << "\n";
    std::cout << "CT : " << gotCt  << "\n";
    std::cout << "EXP: " << expectedCtHex << "\n";
    std::cout << "CT match? " << (gotCt == expectedCtHex ? "YES" : "NO") << "\n";
    std::cout << "DEC back : " << gotDec << "\n";
    std::cout << "Round-trip OK? " << (std::memcmp(dec, pt.data(), 16) == 0 ? "YES" : "NO") << "\n";
    std::cout << "---------------------------------\n";
}

int main() {
    // AES-128 NIST test vector
    run_single_test(
        "000102030405060708090a0b0c0d0e0f1011121314151617",
        "00112233445566778899aabbccddeeff",
        "dda97ca4864cdfe06eaf70a0ec0d7191"
    );

    

    return 0;
}
