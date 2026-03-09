#include <iostream>
#include <string>
#include <vector>

#include "sha-256.h"
#include "hmac.h"
#include "pbkdf2.h"
#include "hkdf.h"
#include "password_manager.h"
#include "file_integrity.h"

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
    // HKDF TEST
    // ==============================

    // Здесь не обязателен fixed expected value, если у тебя нет
    // официального test vector под конкретные ikm/salt/info.
    // Но мы можем проверить, что:
    // 1) длина результата правильная
    // 2) функция работает стабильно
    // 3) одинаковые входы дают одинаковый результат

    std::cout << "\n===== HKDF TEST =====" << std::endl;

    std::string ikm = "input key material";
    std::string salt = "my salt";
    std::string info = "context info";

    std::vector<uint8_t> hkdfKey1 = HKDF::deriveKey(ikm, salt, info, 32);
    std::vector<uint8_t> hkdfKey2 = HKDF::deriveKey(ikm, salt, info, 32);

    std::string hkdfResult1 = SHA256::bytesToHex(hkdfKey1);
    std::string hkdfResult2 = SHA256::bytesToHex(hkdfKey2);

    std::cout << "Derived key 1: " << hkdfResult1 << std::endl;
    std::cout << "Derived key 2: " << hkdfResult2 << std::endl;

    bool sameOutput = (hkdfResult1 == hkdfResult2);
    bool correctLength = (hkdfKey1.size() == 32);

    std::cout << "Same output check: " << (sameOutput ? "PASS" : "FAIL") << std::endl;
    std::cout << "Length check:      " << (correctLength ? "PASS" : "FAIL") << std::endl;


    // ==============================
    // USER SHA-256 MODE
    // ==============================

    std::cout << "\n===== USER SHA-256 MODE =====" << std::endl;

    std::string userInput;

    std::cout << "Enter text: ";
    std::getline(std::cin, userInput);

    std::string userHash = SHA256::hashHex(userInput);

    std::cout << "SHA-256: " << userHash << std::endl;


    // ==============================
    // USER HKDF MODE
    // ==============================

    // Здесь пользователь может сам ввести IKM, salt, info
    // и желаемую длину выходного ключа

    std::cout << "\n===== USER HKDF MODE =====" << std::endl;

    std::string userIKM;
    std::string userSalt;
    std::string userInfo;
    size_t userLength;

    std::cout << "Enter IKM: ";
    std::getline(std::cin, userIKM);

    std::cout << "Enter salt: ";
    std::getline(std::cin, userSalt);

    std::cout << "Enter info: ";
    std::getline(std::cin, userInfo);

    std::cout << "Enter output length in bytes: ";
    std::cin >> userLength;

    std::vector<uint8_t> userHKDF = HKDF::deriveKey(userIKM, userSalt, userInfo, userLength);

    std::cout << "HKDF output: " << SHA256::bytesToHex(userHKDF) << std::endl;


    // ==============================
    // PASSWORD MANAGER TEST
    // ==============================
    
    std::cout << "\n===== PASSWORD MANAGER TEST =====" << std::endl;
    
    StoredPassword stored = storePassword("mypassword");
    
    bool ok1 = verifyPassword("mypassword", stored);
    bool ok2 = verifyPassword("wrongpassword", stored);
    
    std::cout << "Correct password: "
              << (ok1 ? "PASS" : "FAIL") << std::endl;
    
    std::cout << "Wrong password: "
              << (!ok2 ? "PASS" : "FAIL") << std::endl;


    return 0;


    // ==============================
    // FILE INTEGRITY TEST
    // ==============================

    std::cout << "\n===== FILE INTEGRITY TEST =====" << std::endl;

    std::vector<uint8_t> fileHash = sha256File("main.cpp");

    bool fileOk = verifyFile("main.cpp", fileHash);

    std::cout << "File integrity: "
             << (fileOk ? "PASS" : "FAIL")
             << std::endl;

}