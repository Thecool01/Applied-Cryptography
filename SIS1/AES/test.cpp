#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <iomanip>

// Include your headers
#include "aes.h"
#include "AESModes.h" // This includes CustomRNG, Padding, GCM_Math internally

// --- Helper: Print bytes as Hex ---
static void printHex(const std::string& label, const std::vector<uint8_t>& data) {
    std::cout << label << ": ";
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    std::cout << std::dec << std::endl;
}

// --- Helper: Convert String to Bytes ---
static std::vector<uint8_t> strToBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

// ==========================================
// TEST 1: PKCS#7 Padding Logic
// ==========================================
static bool test_padding() {
    std::cout << "[TEST] PKCS#7 Padding... ";

    // Case A: Padding needed (5 bytes -> needs 11 padding bytes of value 0x0B)
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    std::vector<uint8_t> padded = PKCS7::pad(data);

    if (padded.size() != 16) return false;
    for (size_t i = 5; i < 16; i++) {
        if (padded[i] != 0x0B) return false;
    }

    // Case B: Unpadding
    std::vector<uint8_t> unpadded = PKCS7::unpad(padded);
    if (unpadded != data) return false;

    std::cout << "PASSED" << std::endl;
    return true;
}

// ==========================================
// TEST 2: Modes Round-Trip (Encrypt -> Decrypt)
// ==========================================
static bool test_modes() {
    std::cout << "\n=== Testing Modes of Operation ===\n";

    // 128-bit Key (16 bytes)
    std::vector<uint8_t> key(16, 0x33);
    AESModes cipher(key.data(), key.size());

    std::string secretText = "This is a strictly confidential message for SIS1.";
    std::vector<uint8_t> plaintext = strToBytes(secretText);

    // --- ECB Test ---
    std::cout << "[TEST] ECB Mode... ";
    try {
        auto enc = cipher.encryptECB(plaintext);
        auto dec = cipher.decryptECB(enc);
        if (dec != plaintext) throw std::runtime_error("Mismatch");
        std::cout << "PASSED (Size: " << enc.size() << " bytes)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return false;
    }

    // --- CBC Test ---
    std::cout << "[TEST] CBC Mode... ";
    try {
        auto enc = cipher.encryptCBC(plaintext);
        auto dec = cipher.decryptCBC(enc);
        if (dec != plaintext) throw std::runtime_error("Mismatch");
        // CBC adds one block for IV
        std::cout << "PASSED (Size: " << enc.size() << " bytes)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return false;
    }

    // --- CTR Test ---
    std::cout << "[TEST] CTR Mode... ";
    try {
        auto enc = cipher.encryptCTR(plaintext);
        auto dec = cipher.decryptCTR(enc);
        if (dec != plaintext) throw std::runtime_error("Mismatch");
        std::cout << "PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return false;
    }

    // --- GCM Test (Round Trip) ---
    std::cout << "[TEST] GCM Mode... ";
    try {
        std::vector<uint8_t> aad = strToBytes("HeaderData");
        auto enc = cipher.encryptGCM(plaintext, aad);
        auto dec = cipher.decryptGCM(enc, aad);
        if (dec != plaintext) throw std::runtime_error("Mismatch");
        std::cout << "PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// ==========================================
// TEST 3: GCM Security (Tamper Resistance)
// ==========================================
static bool test_gcm_tamper() {
    std::cout << "\n=== Testing GCM Security (Tamper Test) ===\n";
    std::vector<uint8_t> key(16, 0xAB);
    AESModes cipher(key.data(), key.size());

    std::vector<uint8_t> plaintext = strToBytes("Sensitive Financial Data");
    std::vector<uint8_t> aad = strToBytes("TransactionID:123");

    // 1. Encrypt normally
    auto encrypted = cipher.encryptGCM(plaintext, aad);

    // 2. TAMPER: Flip a single bit in the Ciphertext part
    // Structure: IV (12) || Ciphertext (N) || Tag (16)
    if (encrypted.size() > 12 + 16) {
        encrypted[12] ^= 0xFF; // first byte of ciphertext
    } else {
        // Edge-case fallback: flip something before tag
        encrypted[encrypted.size() - 17] ^= 0xFF;
    }

    std::cout << "[TEST] GCM Tamper (Modified Ciphertext)... ";
    try {
        auto decrypted = cipher.decryptGCM(encrypted, aad);
        (void)decrypted;
        std::cout << "FAILED (Decryption succeeded but should have failed!)" << std::endl;
        return false;
    } catch (const std::runtime_error& e) {
        std::cout << "PASSED (Caught expected error: " << e.what() << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "PASSED (Caught expected error: " << e.what() << ")" << std::endl;
    }

    return true;
}

// ==========================================
// TEST 4: RNG Entropy Check
// ==========================================
static bool test_rng() {
    std::cout << "\n=== Testing Custom RNG ===\n";
    CustomRNG rng;
    uint8_t block1[16];
    uint8_t block2[16];

    rng.getBytes(block1, 16);
    rng.getBytes(block2, 16);

    std::cout << "[TEST] Randomness Check... ";
    // Basic check: two consecutive blocks shouldn't be identical
    if (std::memcmp(block1, block2, 16) == 0) {
        std::cout << "FAILED (Generated identical blocks)" << std::endl;
        return false;
    }
    std::cout << "PASSED" << std::endl;
    return true;
}

// ==========================================
// MAIN
// ==========================================
int main() {
    std::cout << "********************************************\n";
    std::cout << "* SIS1 AES Implementation Test Suite       *\n";
    std::cout << "********************************************\n\n";

    bool all_passed = true;

    // 1. Utilities
    if (!test_padding()) all_passed = false;
    if (!test_rng()) all_passed = false;

    // 2. Modes
    if (!test_modes()) all_passed = false;

    // 3. Security
    if (!test_gcm_tamper()) all_passed = false;

    std::cout << "\n--------------------------------------------\n";
    if (all_passed) {
        std::cout << "✅ ALL TESTS PASSED SUCCESSFULLY\n";
        return 0;
    } else {
        std::cout << "❌ SOME TESTS FAILED\n";
        return 1;
    }
}
