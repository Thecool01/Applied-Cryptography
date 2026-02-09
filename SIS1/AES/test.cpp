// test.cpp — SIS1 Test Suite (with NIST AES vectors + edge cases + GCM tamper tests)
//
// Build example (Linux/macOS):
//   g++ -std=c++17 -O2 -Wall -Wextra -pedantic aes.cpp main.cpp -o app      (your CLI)
//   g++ -std=c++17 -O2 -Wall -Wextra -pedantic aes.cpp test.cpp -o test
//
// Build example (Windows / MinGW):
//   g++ -std=c++17 -O2 -Wall -Wextra aes.cpp test.cpp -o test.exe
//
// NOTE: test.cpp depends on: aes.cpp, aes.h, AESModes.h, CustomRNG.h, Padding.h, GCM_Math.h

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

#include "aes.h"
#include "AESModes.h"

// -------------------- Helpers --------------------

static void printHex(const std::string& label, const std::vector<uint8_t>& data) {
    std::cout << label << ": ";
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    std::cout << std::dec << "\n";
}

static std::vector<uint8_t> strToBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

static int hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static std::vector<uint8_t> hexToBytesStrict(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::runtime_error("Hex string length must be even");
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = hexNibble(hex[i]);
        int lo = hexNibble(hex[i + 1]);
        if (hi < 0 || lo < 0) throw std::runtime_error("Invalid hex character");
        out.push_back((uint8_t)((hi << 4) | lo));
    }
    return out;
}

static bool bytesEq(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    return std::memcmp(a.data(), b.data(), a.size()) == 0;
}

static bool bytesEqArr16(const uint8_t a[16], const uint8_t b[16]) {
    return std::memcmp(a, b, 16) == 0;
}

// -------------------- TEST 1: PKCS#7 Padding --------------------

static bool test_padding() {
    std::cout << "[TEST] PKCS#7 Padding... ";

    // Case A: 5 bytes -> needs 11 padding bytes of value 0x0B
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    std::vector<uint8_t> padded = PKCS7::pad(data);

    if (padded.size() != 16) return false;
    for (size_t i = 5; i < 16; i++) {
        if (padded[i] != 0x0B) return false;
    }

    // Case B: Unpadding
    std::vector<uint8_t> unpadded = PKCS7::unpad(padded);
    if (!bytesEq(unpadded, data)) return false;

    // Case C: Exactly 16 bytes -> should add full block of 0x10
    std::vector<uint8_t> exact16(16, 0xAA);
    auto padded2 = PKCS7::pad(exact16);
    if (padded2.size() != 32) return false;
    for (size_t i = 16; i < 32; i++) if (padded2[i] != 0x10) return false;
    auto unpadded2 = PKCS7::unpad(padded2);
    if (!bytesEq(unpadded2, exact16)) return false;

    std::cout << "PASSED\n";
    return true;
}

// -------------------- TEST 2: Custom RNG --------------------

static bool test_rng() {
    std::cout << "[TEST] Custom RNG basic sanity... ";

    CustomRNG rng;
    uint8_t block1[16];
    uint8_t block2[16];

    rng.getBytes(block1, 16);
    rng.getBytes(block2, 16);

    // Basic check: consecutive blocks should not be identical
    if (std::memcmp(block1, block2, 16) == 0) {
        std::cout << "FAILED (identical consecutive blocks)\n";
        return false;
    }

    std::cout << "PASSED\n";
    return true;
}

// -------------------- TEST 3: NIST AES Core Vectors (ECB single-block) --------------------
// Standard NIST SP 800-38A AES ECB test vectors (one-block)
// PT = 00112233445566778899aabbccddeeff
// AES-128 key = 000102030405060708090a0b0c0d0e0f -> CT = 69c4e0d86a7b0430d8cdb78070b4c55a
// AES-192 key = 000102030405060708090a0b0c0d0e0f1011121314151617 -> CT = dda97ca4864cdfe06eaf70a0ec0d7191
// AES-256 key = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f -> CT = 8ea2b7ca516745bfeafc49904b496089

static bool test_nist_aes_vectors() {
    std::cout << "\n=== NIST AES Core Vectors (Single Block) ===\n";

    const std::string PT_HEX  = "00112233445566778899aabbccddeeff";

    struct Vec {
        std::string name;
        std::string keyHex;
        std::string ctHex;
    };

    std::vector<Vec> tests = {
        {"AES-128", "000102030405060708090a0b0c0d0e0f", "69c4e0d86a7b0430d8cdb78070b4c55a"},
        {"AES-192", "000102030405060708090a0b0c0d0e0f1011121314151617", "dda97ca4864cdfe06eaf70a0ec0d7191"},
        {"AES-256", "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "8ea2b7ca516745bfeafc49904b496089"},
    };

    auto pt = hexToBytesStrict(PT_HEX);
    if (pt.size() != 16) throw std::runtime_error("Internal error: PT vector not 16 bytes");

    uint8_t ptBlock[16];
    std::memcpy(ptBlock, pt.data(), 16);

    bool ok = true;

    for (const auto& tv : tests) {
        std::cout << "[TEST] " << tv.name << " NIST vector... ";

        auto key = hexToBytesStrict(tv.keyHex);
        auto expectedCT = hexToBytesStrict(tv.ctHex);
        if (expectedCT.size() != 16) throw std::runtime_error("Internal error: CT vector not 16 bytes");

        aes::KeySchedule ks = aes::expandKey(key.data(), key.size());

        uint8_t ctBlock[16];
        uint8_t decBlock[16];

        aes::encryptBlock(ptBlock, ctBlock, ks);
        if (!bytesEqArr16(ctBlock, expectedCT.data())) {
            std::cout << "FAILED (encrypt mismatch)\n";
            printHex("  expected", expectedCT);
            printHex("  got     ", std::vector<uint8_t>(ctBlock, ctBlock + 16));
            ok = false;
            continue;
        }

        aes::decryptBlock(ctBlock, decBlock, ks);
        if (std::memcmp(decBlock, ptBlock, 16) != 0) {
            std::cout << "FAILED (decrypt mismatch)\n";
            ok = false;
            continue;
        }

        std::cout << "PASSED\n";
    }

    return ok;
}

// -------------------- TEST 4: Modes Round-Trip (Encrypt -> Decrypt) --------------------

static bool test_modes_roundtrip() {
    std::cout << "\n=== Modes Round-Trip Tests ===\n";

    // 128-bit key
    std::vector<uint8_t> key(16, 0x33);
    AESModes cipher(key.data(), key.size());

    std::string secretText = "This is a strictly confidential message for SIS1.";
    std::vector<uint8_t> plaintext = strToBytes(secretText);

    // ECB
    std::cout << "[TEST] ECB round-trip... ";
    try {
        auto enc = cipher.encryptECB(plaintext);
        auto dec = cipher.decryptECB(enc);
        if (!bytesEq(dec, plaintext)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    // CBC
    std::cout << "[TEST] CBC round-trip... ";
    try {
        auto enc = cipher.encryptCBC(plaintext);
        auto dec = cipher.decryptCBC(enc);
        if (!bytesEq(dec, plaintext)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    // CTR
    std::cout << "[TEST] CTR round-trip... ";
    try {
        auto enc = cipher.encryptCTR(plaintext);
        auto dec = cipher.decryptCTR(enc);
        if (!bytesEq(dec, plaintext)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    // GCM
    std::cout << "[TEST] GCM round-trip... ";
    try {
        std::vector<uint8_t> aad = strToBytes("HeaderData");
        auto enc = cipher.encryptGCM(plaintext, aad);
        auto dec = cipher.decryptGCM(enc, aad);
        if (!bytesEq(dec, plaintext)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    return true;
}

// -------------------- TEST 5: Edge cases (empty, exact block) --------------------

static bool test_edge_cases() {
    std::cout << "\n=== Edge Case Tests ===\n";

    std::vector<uint8_t> key(16, 0x42);
    AESModes cipher(key.data(), key.size());

    // Empty plaintext
    std::vector<uint8_t> empty;

    std::cout << "[TEST] ECB empty plaintext... ";
    try {
        auto enc = cipher.encryptECB(empty);
        auto dec = cipher.decryptECB(enc);
        if (!dec.empty()) throw std::runtime_error("Expected empty after decrypt");
        // ECB empty should encrypt to exactly one full padding block = 16 bytes
        if (enc.size() != 16) throw std::runtime_error("Expected 16 bytes ciphertext for empty plaintext in ECB");
        std::cout << "PASSED\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    std::cout << "[TEST] CBC empty plaintext... ";
    try {
        auto enc = cipher.encryptCBC(empty);
        auto dec = cipher.decryptCBC(enc);
        if (!dec.empty()) throw std::runtime_error("Expected empty after decrypt");
        // CBC adds IV(16) + padding block(16) = 32
        if (enc.size() != 32) throw std::runtime_error("Expected 32 bytes ciphertext for empty plaintext in CBC");
        std::cout << "PASSED\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    // Exactly 16 bytes
    std::vector<uint8_t> exact16(16, 0x7F);

    std::cout << "[TEST] ECB exactly 16 bytes (should add full padding block)... ";
    try {
        auto enc = cipher.encryptECB(exact16);
        auto dec = cipher.decryptECB(enc);
        if (!bytesEq(dec, exact16)) throw std::runtime_error("Mismatch");
        if (enc.size() != 32) throw std::runtime_error("Expected 32 bytes ciphertext for 16-byte input in ECB");
        std::cout << "PASSED\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    std::cout << "[TEST] CBC exactly 16 bytes (IV + 2 blocks)... ";
    try {
        auto enc = cipher.encryptCBC(exact16);
        auto dec = cipher.decryptCBC(enc);
        if (!bytesEq(dec, exact16)) throw std::runtime_error("Mismatch");
        if (enc.size() != 48) throw std::runtime_error("Expected 48 bytes ciphertext for 16-byte input in CBC");
        std::cout << "PASSED\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    return true;
}

// -------------------- TEST 6: GCM Tamper Tests (ciphertext / AAD / tag) --------------------

static bool test_gcm_tamper_all() {
    std::cout << "\n=== GCM Tamper Resistance Tests ===\n";

    std::vector<uint8_t> key(16, 0xAB);
    AESModes cipher(key.data(), key.size());

    std::vector<uint8_t> plaintext = strToBytes("Sensitive Financial Data");
    std::vector<uint8_t> aad = strToBytes("TransactionID:123");

    auto encrypted = cipher.encryptGCM(plaintext, aad);

    auto mustFail = [&](const std::string& label,
                        std::vector<uint8_t> encCopy,
                        std::vector<uint8_t> aadCopy) -> bool {
        std::cout << "[TEST] " << label << "... ";
        try {
            auto dec = cipher.decryptGCM(encCopy, aadCopy);
            (void)dec;
            std::cout << "FAILED (decryption succeeded but must fail)\n";
            return false;
        } catch (const std::exception& e) {
            std::cout << "PASSED (caught: " << e.what() << ")\n";
            return true;
        }
    };

    // 1) Tamper ciphertext (flip first byte of ciphertext part)
    {
        auto tampered = encrypted;
        // Structure: IV(12) || C(N) || TAG(16)
        if (tampered.size() <= 12 + 16) {
            std::cout << "[TEST] GCM ciphertext tamper... FAILED (ciphertext too short)\n";
            return false;
        }
        tampered[12] ^= 0xFF;
        if (!mustFail("GCM ciphertext tamper", tampered, aad)) return false;
    }

    // 2) Tamper AAD (use different AAD during decrypt)
    {
        auto aadBad = aad;
        if (!aadBad.empty()) aadBad[0] ^= 0x01;
        if (!mustFail("GCM AAD tamper", encrypted, aadBad)) return false;
    }

    // 3) Tamper tag (flip last byte)
    {
        auto tampered = encrypted;
        tampered[tampered.size() - 1] ^= 0xAA;
        if (!mustFail("GCM tag tamper", tampered, aad)) return false;
    }

    return true;
}

// -------------------- TEST 7: Large buffer (>= 1MB) --------------------

static bool test_large_data() {
    std::cout << "\n=== Large Data Test (>= 1MB) ===\n";

    std::vector<uint8_t> key(32, 0x11); // AES-256
    AESModes cipher(key.data(), key.size());

    // 2 MB of patterned data
    const size_t N = 2 * 1024 * 1024;
    std::vector<uint8_t> plain;
    plain.reserve(N);
    for (size_t i = 0; i < N; i++) plain.push_back((uint8_t)(i & 0xFF));

    std::cout << "[TEST] CTR large buffer round-trip... ";
    try {
        auto enc = cipher.encryptCTR(plain);
        auto dec = cipher.decryptCTR(enc);
        if (!bytesEq(dec, plain)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    std::cout << "[TEST] GCM large buffer round-trip... ";
    try {
        auto aad = strToBytes("LargeAAD");
        auto enc = cipher.encryptGCM(plain, aad);
        auto dec = cipher.decryptGCM(enc, aad);
        if (!bytesEq(dec, plain)) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (enc size: " << enc.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << "\n";
        return false;
    }

    return true;
}

// -------------------- MAIN --------------------

int main() {
    std::cout << "********************************************\n";
    std::cout << "* SIS1 AES Implementation Test Suite       *\n";
    std::cout << "* (NIST vectors + modes + security tests)  *\n";
    std::cout << "********************************************\n\n";

    bool all_passed = true;

    try {
        if (!test_padding()) all_passed = false;
        if (!test_rng()) all_passed = false;

        if (!test_nist_aes_vectors()) all_passed = false;

        if (!test_modes_roundtrip()) all_passed = false;
        if (!test_edge_cases()) all_passed = false;

        if (!test_gcm_tamper_all()) all_passed = false;

        if (!test_large_data()) all_passed = false;

    } catch (const std::exception& e) {
        std::cout << "\n[UNCAUGHT ERROR]: " << e.what() << "\n";
        all_passed = false;
    }

    std::cout << "\n--------------------------------------------\n";
    if (all_passed) {
        std::cout << "✅ ALL TESTS PASSED SUCCESSFULLY\n";
        return 0;
    } else {
        std::cout << "❌ SOME TESTS FAILED\n";
        return 1;
    }
}
