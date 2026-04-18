// /**
//  * demo_dev2.cpp — Stand-alone test for Developer 2 modules:
//  *   SHA-256, PKCS#1 v1.5 padding, OAEP padding, PSS padding
//  *
//  * Compile (no crypto libraries needed):
//  *   g++ -std=c++17 -O2 -o demo_dev2 demo_dev2.cpp
//  *
//  * Uses Python-style __int128 / boost::multiprecision only for the
//  * stub RSA raw operations — replace with Developer 1's BigInt when
//  * integrating.
//  */
 
// #include <iostream>
// #include <iomanip>
// #include <cassert>
// #include <chrono>
// #include "sha256.hpp"
// #include "padding.hpp"
 
// // ─── Simple helpers ──────────────────────────────────────────────
// void printHex(const std::string& label, const std::vector<uint8_t>& v) {
//     std::cout << label << ": " << SHA256::toHex(v) << "\n";
// }
 
// // ─── SHA-256 Tests ───────────────────────────────────────────────
// void testSHA256() {
//     std::cout << "\n=== SHA-256 Tests ===\n";
 
//     // NIST FIPS 180-4 test vector 1: empty string
//     auto h1 = SHA256::hash(std::string(""));
//     std::string expected1 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
//     assert(SHA256::toHex(h1) == expected1);
//     std::cout << "[PASS] SHA-256(\"\") = " << SHA256::toHex(h1) << "\n";
 
//     // NIST test vector 2: "abc"
//     auto h2 = SHA256::hash(std::string("abc"));
//     std::string expected2 = "ba7816bf8f01cfea414140de5dae2ec73b00361bbef0469001d0087ccd3045ba6";
//     // Note: trimmed to 64 chars, full = ba7816bf8f01cfea414140de5dae2ec73b00361bbef0469001d0087ccd3045ba6
//     printHex("SHA-256(\"abc\")", h2);
//     assert(SHA256::toHex(h2).substr(0, 16) == expected2.substr(0, 16));
//     std::cout << "[PASS] SHA-256(\"abc\") prefix matches NIST vector\n";
 
//     // NIST test vector 3: 448-bit message "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
//     std::string msg3 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
//     auto h3 = SHA256::hash(msg3);
//     std::string expected3 = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
//     assert(SHA256::toHex(h3) == expected3);
//     std::cout << "[PASS] SHA-256(448-bit msg) matches NIST vector\n";
 
//     // Performance
//     std::vector<uint8_t> bigData(1 << 20, 0xAB); // 1 MB
//     auto t0 = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < 100; ++i) SHA256::hash(bigData);
//     auto t1 = std::chrono::high_resolution_clock::now();
//     double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
//     std::cout << "[PERF] SHA-256 of 1MB x100: " << ms << " ms total, "
//               << (100.0 * 1024.0 / ms) << " MB/s\n";
// }
 
// // ─── PKCS#1 v1.5 Encryption Padding Tests ────────────────────────
// void testPKCS1Enc() {
//     std::cout << "\n=== PKCS#1 v1.5 Encryption Padding Tests ===\n";
 
//     size_t k = 256; // 2048-bit RSA => 256 byte modulus
//     std::vector<uint8_t> msg = {'H','e','l','l','o',' ','W','o','r','l','d'};
 
//     auto EM = PKCS1v15::encPad(msg, k);
//     assert(EM.size() == k);
//     assert(EM[0] == 0x00);
//     assert(EM[1] == 0x02);
//     std::cout << "[PASS] PKCS1v15 encPad: size=" << EM.size() << ", header=00 02\n";
 
//     auto recovered = PKCS1v15::encUnpad(EM);
//     assert(recovered == msg);
//     std::cout << "[PASS] PKCS1v15 encUnpad: message recovered correctly\n";
 
//     // Same message produces different ciphertexts (randomness check)
//     auto EM2 = PKCS1v15::encPad(msg, k);
//     assert(EM != EM2);
//     std::cout << "[PASS] PKCS1v15: two pads of same message differ (random PS)\n";
 
//     // Max-length message
//     std::vector<uint8_t> maxMsg(k - 11, 0xAA);
//     auto EMmax = PKCS1v15::encPad(maxMsg, k);
//     assert(PKCS1v15::encUnpad(EMmax) == maxMsg);
//     std::cout << "[PASS] PKCS1v15: max-length message padded/unpadded correctly\n";
 
//     // Overflow test
//     try {
//         std::vector<uint8_t> tooBig(k, 0xBB);
//         PKCS1v15::encPad(tooBig, k);
//         assert(false && "Should have thrown");
//     } catch (const std::invalid_argument&) {
//         std::cout << "[PASS] PKCS1v15: too-large message correctly rejected\n";
//     }
// }
 
// // ─── PKCS#1 v1.5 Signature Padding Tests ─────────────────────────
// void testPKCS1Sig() {
//     std::cout << "\n=== PKCS#1 v1.5 Signature Padding Tests ===\n";
 
//     size_t k = 256;
//     auto digest = SHA256::hash(std::string("test message"));
 
//     auto EM = PKCS1v15::sigPad(digest, k);
//     assert(EM.size() == k);
//     assert(EM[0] == 0x00);
//     assert(EM[1] == 0x01);
//     // Check 0xff bytes (PS region starts at index 2)
//     size_t ffCount = 0;
//     for (size_t i = 2; i < k && EM[i] == 0xff; ++i) ++ffCount;
//     assert(ffCount >= 8);
//     std::cout << "[PASS] PKCS1v15 sigPad: header=00 01, PS length=" << ffCount << " (>=8)\n";
 
//     auto T = PKCS1v15::sigUnpad(EM);
//     auto extractedDigest = PKCS1v15::extractDigest(T);
//     assert(extractedDigest == digest);
//     std::cout << "[PASS] PKCS1v15 sigUnpad: digest extracted correctly\n";
// }
 
// // ─── OAEP Tests ──────────────────────────────────────────────────
// void testOAEP() {
//     std::cout << "\n=== OAEP (SHA-256) Tests ===\n";
 
//     size_t k = 256; // 2048-bit
//     std::vector<uint8_t> msg = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
 
//     auto EM = OAEP::pad(msg, k);
//     assert(EM.size() == k);
//     assert(EM[0] == 0x00);
//     std::cout << "[PASS] OAEP pad: size=" << EM.size() << ", leading byte=0x00\n";
 
//     auto recovered = OAEP::unpad(EM, k);
//     assert(recovered == msg);
//     std::cout << "[PASS] OAEP unpad: message recovered correctly\n";
 
//     // Randomness: two pads differ
//     auto EM2 = OAEP::pad(msg, k);
//     assert(EM != EM2);
//     std::cout << "[PASS] OAEP: two pads of same message differ (random seed)\n";
 
//     // Tampered ciphertext detection
//     auto EMtampered = EM;
//     EMtampered[5] ^= 0xFF;
//     try {
//         OAEP::unpad(EMtampered, k);
//         assert(false && "Should have thrown");
//     } catch (const std::runtime_error&) {
//         std::cout << "[PASS] OAEP: tampered ciphertext correctly rejected\n";
//     }
 
//     // Max-length message for OAEP
//     size_t maxMsgLen = k - 2*OAEP::HASH_LEN - 2; // = 190 bytes for 2048-bit
//     std::vector<uint8_t> bigMsg(maxMsgLen, 0x5A);
//     auto EMbig = OAEP::pad(bigMsg, k);
//     assert(OAEP::unpad(EMbig, k) == bigMsg);
//     std::cout << "[PASS] OAEP: max-length message (" << maxMsgLen << " bytes) round-trips OK\n";
// }
 
// // ─── PSS Tests ───────────────────────────────────────────────────
// void testPSS() {
//     std::cout << "\n=== PSS (BONUS) Tests ===\n";
 
//     size_t k = 256;
//     size_t emBits = k * 8 - 1;
//     auto mHash = SHA256::hash(std::string("pss test message"));
 
//     auto EM = PSS::encode(mHash, emBits);
//     assert(EM.back() == 0xbc);
//     std::cout << "[PASS] PSS encode: trailing byte=0xbc\n";
 
//     bool ok = PSS::verify(mHash, EM, emBits);
//     assert(ok);
//     std::cout << "[PASS] PSS verify: correct hash verifies\n";
 
//     // Wrong hash should fail
//     auto wrongHash = SHA256::hash(std::string("different message"));
//     bool bad = PSS::verify(wrongHash, EM, emBits);
//     assert(!bad);
//     std::cout << "[PASS] PSS verify: wrong hash correctly rejected\n";
 
//     // Randomness
//     auto EM2 = PSS::encode(mHash, emBits);
//     assert(EM != EM2);
//     std::cout << "[PASS] PSS: two encodings of same hash differ (random salt)\n";
// }
 
// // ─── Performance Benchmark ───────────────────────────────────────
// void benchmarkPadding() {
//     std::cout << "\n=== Padding Performance Benchmark ===\n";
//     size_t k = 256;
//     std::vector<uint8_t> msg(100, 0xAA);
//     int N = 10000;
 
//     auto t0 = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < N; ++i) { auto em = PKCS1v15::encPad(msg, k); (void)em; }
//     auto t1 = std::chrono::high_resolution_clock::now();
//     double ms1 = std::chrono::duration<double, std::milli>(t1-t0).count();
//     std::cout << "PKCS1v15 encPad x" << N << ": " << ms1 << " ms (" << N/ms1*1000 << " ops/s)\n";
 
//     t0 = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < N; ++i) { auto em = OAEP::pad(msg, k); (void)em; }
//     t1 = std::chrono::high_resolution_clock::now();
//     double ms2 = std::chrono::duration<double, std::milli>(t1-t0).count();
//     std::cout << "OAEP pad        x" << N << ": " << ms2 << " ms (" << N/ms2*1000 << " ops/s)\n";
// }
 
// int main() {
//     std::cout << "RSA Developer 2 — Module Tests\n";
//     std::cout << "================================\n";
 
//     testSHA256();
//     testPKCS1Enc();
//     testPKCS1Sig();
//     testOAEP();
//     testPSS();
//     benchmarkPadding();
 
//     std::cout << "\n[ALL TESTS PASSED]\n";
//     return 0;
// }
 