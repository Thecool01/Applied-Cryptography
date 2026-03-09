#include <iostream>
#include <string>
#include <vector>

#include "sha-256.h"
#include "hmac.h"
#include "pbkdf2.h"

int main() {

    // ==============================
    // SHA-256 TESTS
    // ==============================

    std::string test1 = "abc";
    std::string test2 = "";
    std::string test3 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    std::string expected1 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string expected2 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string expected3 = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";

    std::string result1 = SHA256::hashHex(test1);
    std::string result2 = SHA256::hashHex(test2);
    std::string result3 = SHA256::hashHex(test3);

    std::cout << "===== SHA-256 TESTS =====" << std::endl;

    std::cout << "\nTest 1: \"abc\"" << std::endl;
    std::cout << "Result:   " << result1 << std::endl;
    std::cout << "Expected: " << expected1 << std::endl;
    std::cout << (result1 == expected1 ? "PASS" : "FAIL") << std::endl;

    std::cout << "\nTest 2: \"\"" << std::endl;
    std::cout << "Result:   " << result2 << std::endl;
    std::cout << "Expected: " << expected2 << std::endl;
    std::cout << (result2 == expected2 ? "PASS" : "FAIL") << std::endl;

    std::cout << "\nTest 3: long message" << std::endl;
    std::cout << "Result:   " << result3 << std::endl;
    std::cout << "Expected: " << expected3 << std::endl;
    std::cout << (result3 == expected3 ? "PASS" : "FAIL") << std::endl;


    // ==============================
    // HMAC-SHA256 TEST
    // ==============================

    std::cout << "\n===== HMAC-SHA256 TEST =====" << std::endl;

    std::vector<uint8_t> key(20, 0x0b);
    std::string message = "Hi There";

    std::vector<uint8_t> msg(message.begin(), message.end());

    std::string hmacResult = HMAC_SHA256::computeHex(key, msg);

    std::string hmacExpected =
        "b0344c61d8db38535ca8afceaf0bf12b"
        "881dc200c9833da726e9376c2e32cff7";

    std::cout << "Result:   " << hmacResult << std::endl;
    std::cout << "Expected: " << hmacExpected << std::endl;

    std::cout << (hmacResult == hmacExpected ? "PASS" : "FAIL") << std::endl;


    // ==============================
    // PBKDF2 TEST
    // ==============================

    std::cout << "\n===== PBKDF2 TEST =====" << std::endl;

    std::vector<uint8_t> dk = PBKDF2::deriveKey(
        "password",
        "salt",
        1,
        20
    );

    std::string pbkdf2Result = SHA256::bytesToHex(dk);

    std::string pbkdf2Expected =
        "0c60c80f961f0e71f3a9b524af6012062fe037a6";

    std::cout << "Result:   " << pbkdf2Result << std::endl;
    std::cout << "Expected: " << pbkdf2Expected << std::endl;

    std::cout << (pbkdf2Result == pbkdf2Expected ? "PASS" : "FAIL") << std::endl;


    // ==============================
    // USER MODE
    // ==============================

    std::cout << "\n===== USER HASH MODE =====" << std::endl;

    std::string userInput;

    std::cout << "Enter text: ";
    std::getline(std::cin, userInput);

    std::string userHash = SHA256::hashHex(userInput);

    std::cout << "SHA-256: " << userHash << std::endl;

    return 0;
}