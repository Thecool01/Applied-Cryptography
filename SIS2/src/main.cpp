#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <thread>
#include <chrono>
#include <ctime>

#include "sha-256.h"
#include "sha-512.h"
#include "hmac.h"
#include "pbkdf2.h"
#include "hkdf.h"
#include "password_manager.h"
#include "file_integrity.h"



// ------------------------------------------------------------
// Небольшая задержка в миллисекундах
// ------------------------------------------------------------
void pauseMs(int milliseconds) {
    clock_t start = clock();
    clock_t delay = milliseconds * CLOCKS_PER_SEC / 1000;

    while (clock() - start < delay) {
    }
}

void typeText(const std::string& text, int delayMs = 15) {
    for (char c : text) {
        std::cout << c << std::flush;
        pauseMs(delayMs);
    }
}

void showLoading(const std::string& text, int dots = 3, int delayMs = 300) {
    std::cout << text;
    for (int i = 0; i < dots; i++) {
        std::cout << "." << std::flush;
        pauseMs(delayMs);
    }
    std::cout << std::endl;
}

// ------------------------------------------------------------
// Лого программы
// ------------------------------------------------------------
void printLogo() {
    std::cout << "\n";
    typeText("========================================\n", 2);
    typeText("         SIS2 CRYPTOGRAPHY TOOL         \n", 2);
    typeText("========================================\n", 2);
    typeText("SHA-256 | HMAC | PBKDF2 | HKDF | SHA-512\n", 2);
    typeText("========================================\n\n", 2);

    pauseMs(400);
}


// ------------------------------------------------------------
// Подсчёт количества отличающихся битов между двумя массивами байтов
// Нужен для avalanche effect
// ------------------------------------------------------------
int countDifferentBits(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) {
        return -1;
    }

    int diffBits = 0;

    for (size_t i = 0; i < a.size(); i++) {
        uint8_t x = a[i] ^ b[i];

        for (int j = 0; j < 8; j++) {
            diffBits += (x >> j) & 1;
        }
    }

    return diffBits;
}

// ------------------------------------------------------------
// Чтение строки с дефолтным значением
// Если пользователь ничего не вводит, возвращается defaultValue
// ------------------------------------------------------------
std::string readLineOrDefault(const std::string& prompt, const std::string& defaultValue) {
    std::string input;

    std::cout << prompt << " [default: " << defaultValue << "]: ";
    std::getline(std::cin, input);

    if (input.empty()) {
        return defaultValue;
    }

    return input;
}

// ------------------------------------------------------------
// Чтение size_t с дефолтным значением
// Если пользователь ничего не вводит, возвращается defaultValue
// ------------------------------------------------------------
size_t readSizeOrDefault(const std::string& prompt, size_t defaultValue) {
    std::string input;

    std::cout << prompt << " [default: " << defaultValue << "]: ";
    std::getline(std::cin, input);

    if (input.empty()) {
        return defaultValue;
    }

    return static_cast<size_t>(std::stoul(input));
}

// ------------------------------------------------------------
// SHA-256 тесты
// ------------------------------------------------------------
void runSHA256Tests() {
    std::string test1 = "abc";
    std::string test2 = "";
    std::string test3 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    std::string expected1 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string expected2 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string expected3 = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";

    std::string result1 = SHA256::hashHex(test1);
    std::string result2 = SHA256::hashHex(test2);
    std::string result3 = SHA256::hashHex(test3);

    std::cout << "\n===== SHA-256 TESTS =====" << std::endl;
    pauseMs(250);

    std::cout << "\nTest 1: \"abc\"" << std::endl;
    pauseMs(150);
    std::cout << "Result:   " << result1 << std::endl;
    std::cout << "Expected: " << expected1 << std::endl;
    std::cout << "Status:   " << (result1 == expected1 ? "PASS" : "FAIL") << std::endl;
    pauseMs(250);

    std::cout << "\nTest 2: \"\"" << std::endl;
    pauseMs(150);
    std::cout << "Result:   " << result2 << std::endl;
    std::cout << "Expected: " << expected2 << std::endl;
    std::cout << "Status:   " << (result2 == expected2 ? "PASS" : "FAIL") << std::endl;
    pauseMs(250);

    std::cout << "\nTest 3: long message" << std::endl;
    pauseMs(150);
    std::cout << "Result:   " << result3 << std::endl;
    std::cout << "Expected: " << expected3 << std::endl;
    std::cout << "Status:   " << (result3 == expected3 ? "PASS" : "FAIL") << std::endl;
    pauseMs(250);
}

// ------------------------------------------------------------
// HMAC тест
// ------------------------------------------------------------
void runHMACTest() {
    std::cout << "\n===== HMAC-SHA256 TEST =====" << std::endl;
    showLoading("Computing HMAC", 3, 250);

    std::vector<uint8_t> key(20, 0x0b);
    std::string message = "Hi There";
    std::vector<uint8_t> msg(message.begin(), message.end());

    std::string hmacResult = HMAC_SHA256::computeHex(key, msg);
    std::string hmacExpected =
        "b0344c61d8db38535ca8afceaf0bf12b"
        "881dc200c9833da726e9376c2e32cff7";

    std::cout << "Result:   " << hmacResult << std::endl;
    std::cout << "Expected: " << hmacExpected << std::endl;
    std::cout << "Status:   " << (hmacResult == hmacExpected ? "PASS" : "FAIL") << std::endl;
}

// ------------------------------------------------------------
// PBKDF2 тест
// ------------------------------------------------------------
void runPBKDF2Test() {
    std::cout << "\n===== PBKDF2 TEST =====" << std::endl;
    
    showLoading("Testing", 3, 250);
    std::vector<uint8_t> dk = PBKDF2::deriveKey("password", "salt", 1, 20);
    std::string pbkdf2Result = SHA256::bytesToHex(dk);
    std::string pbkdf2Expected = "120fb6cffcf8b32c43e7225256c4f837a86548c9";

    std::cout << "Result:   " << pbkdf2Result << std::endl;
    std::cout << "Expected: " << pbkdf2Expected << std::endl;
    std::cout << "Status:   " << (pbkdf2Result == pbkdf2Expected ? "PASS" : "FAIL") << std::endl;
}

// ------------------------------------------------------------
// HKDF тест
// ------------------------------------------------------------
void runHKDFTest() {
    std::cout << "\n===== HKDF TEST =====" << std::endl;

    std::string ikm = "input key material";
    std::string salt = "my salt";
    std::string info = "context info";

    std::vector<uint8_t> hkdfKey1 = HKDF::deriveKey(ikm, salt, info, 32);
    std::vector<uint8_t> hkdfKey2 = HKDF::deriveKey(ikm, salt, info, 32);

    std::string hkdfResult1 = SHA256::bytesToHex(hkdfKey1);
    std::string hkdfResult2 = SHA256::bytesToHex(hkdfKey2);

    bool sameOutput = (hkdfResult1 == hkdfResult2);
    bool correctLength = (hkdfKey1.size() == 32);
    showLoading("Testing", 3, 250);
    std::cout << "Derived key 1: " << hkdfResult1 << std::endl;
    std::cout << "Derived key 2: " << hkdfResult2 << std::endl;
    std::cout << "Same output check: " << (sameOutput ? "PASS" : "FAIL") << std::endl;
    std::cout << "Length check:      " << (correctLength ? "PASS" : "FAIL") << std::endl;
}

// ------------------------------------------------------------
// Functional tests
// ------------------------------------------------------------
void runFunctionalTests() {
    std::cout << "\n===== FUNCTIONAL TESTS =====" << std::endl;

    // 1. Collision Resistance Demo
    std::string collisionInput1 = "hello";
    std::string collisionInput2 = "world";

    std::string collisionHash1 = SHA256::hashHex(collisionInput1);
    std::string collisionHash2 = SHA256::hashHex(collisionInput2);

    std::cout << "\n[1] Collision Resistance Demo" << std::endl;
    showLoading("Testing", 3, 250);
    std::cout << "Input 1 hash: " << collisionHash1 << std::endl;
    std::cout << "Input 2 hash: " << collisionHash2 << std::endl;
    std::cout << "Status: " << (collisionHash1 != collisionHash2 ? "PASS" : "FAIL") << std::endl;

    // 2. Avalanche Effect
    std::string avalancheInput1 = "hello";
    std::string avalancheInput2 = "hellp";

    std::vector<uint8_t> avalancheHash1 = SHA256::hash(avalancheInput1);
    std::vector<uint8_t> avalancheHash2 = SHA256::hash(avalancheInput2);

    int changedBits = countDifferentBits(avalancheHash1, avalancheHash2);

    std::cout << "\n[2] Avalanche Effect" << std::endl;
    showLoading("Testing", 3, 250);
    std::cout << "Hash 1: " << SHA256::bytesToHex(avalancheHash1) << std::endl;
    std::cout << "Hash 2: " << SHA256::bytesToHex(avalancheHash2) << std::endl;
    std::cout << "Different bits: " << changedBits << " / 256" << std::endl;
    std::cout << "Status: " << (changedBits > 100 ? "PASS" : "FAIL") << std::endl;

    // 3. File Integrity
    std::cout << "\n[3] File Integrity" << std::endl;
    showLoading("Testing", 3, 250);
    std::string testFileName = "integrity_test.txt";

    {
        std::ofstream out(testFileName, std::ios::binary);
        out << "Original file content";
    }

    std::vector<uint8_t> originalHash = sha256File(testFileName);
    bool unmodifiedOk = verifyFile(testFileName, originalHash);

    {
        std::ofstream out(testFileName, std::ios::binary);
        out << "Modified file content";
    }

    bool modifiedDetected = !verifyFile(testFileName, originalHash);

    std::cout << "Unmodified file check: " << (unmodifiedOk ? "PASS" : "FAIL") << std::endl;
    std::cout << "Modified file detected: " << (modifiedDetected ? "PASS" : "FAIL") << std::endl;

    // 4. HMAC Verification
    std::cout << "\n[4] HMAC Verification" << std::endl;
    showLoading("Testing", 3, 250);
    std::vector<uint8_t> key(20, 0x0b);
    std::string message = "Hi There";
    std::vector<uint8_t> msg(message.begin(), message.end());

    std::string validTag = HMAC_SHA256::computeHex(key, msg);
    std::string invalidTag = validTag;
    invalidTag[0] = (invalidTag[0] == '0') ? '1' : '0';

    bool validAccepted = (validTag == HMAC_SHA256::computeHex(key, msg));
    bool invalidRejected = (invalidTag != HMAC_SHA256::computeHex(key, msg));

    std::cout << "Valid tag accepted:   " << (validAccepted ? "PASS" : "FAIL") << std::endl;
    std::cout << "Invalid tag rejected: " << (invalidRejected ? "PASS" : "FAIL") << std::endl;

    // 5. Password Storage
    std::cout << "\n[5] Password Storage" << std::endl;
    showLoading("Testing", 3, 250);
    StoredPassword stored = storePassword("mypassword");
    bool correctPassword = verifyPassword("mypassword", stored);
    bool wrongPassword = verifyPassword("wrongpassword", stored);

    std::cout << "Correct password: " << (correctPassword ? "PASS" : "FAIL") << std::endl;
    std::cout << "Wrong password:   " << (!wrongPassword ? "PASS" : "FAIL") << std::endl;

    // 6. Different Salts
    std::cout << "\n[6] Different Salts" << std::endl;
    showLoading("Testing", 3, 250);
    StoredPassword stored1 = storePassword("samepassword");
    StoredPassword stored2 = storePassword("samepassword");

    bool saltsAreDifferent = (stored1.salt != stored2.salt);
    bool hashesAreDifferent = (stored1.hash != stored2.hash);

    std::cout << "Salts are different:  " << (saltsAreDifferent ? "PASS" : "FAIL") << std::endl;
    std::cout << "Hashes are different: " << (hashesAreDifferent ? "PASS" : "FAIL") << std::endl;
}

// ------------------------------------------------------------
// Пользовательский SHA-256 режим
// ------------------------------------------------------------
void runUserSHA256Mode() {
    std::cout << "\n===== USER SHA-256 MODE =====" << std::endl;

    std::string userInput = readLineOrDefault("Enter text", "hello");
    std::string userHash = SHA256::hashHex(userInput);

    std::cout << "SHA-256: " << userHash << std::endl;
}

// ------------------------------------------------------------
// Пользовательский HKDF режим
// ------------------------------------------------------------
void runUserHKDFMode() {
    std::cout << "\n===== USER HKDF MODE =====" << std::endl;

    std::string userIKM = readLineOrDefault("Enter IKM", "input key material");
    std::string userSalt = readLineOrDefault("Enter salt", "my salt");
    std::string userInfo = readLineOrDefault("Enter info", "context info");
    size_t userLength = readSizeOrDefault("Enter output length in bytes", 32);

    std::vector<uint8_t> userHKDF = HKDF::deriveKey(userIKM, userSalt, userInfo, userLength);

    std::cout << "HKDF output: " << SHA256::bytesToHex(userHKDF) << std::endl;
}

// ------------------------------------------------------------
// Пользовательский HMAC режим
// ------------------------------------------------------------
void runUserHMACMode() {
    std::cout << "\n===== USER HMAC MODE =====" << std::endl;

    std::string keyStr = readLineOrDefault("Enter key", "secretkey");
    std::string msgStr = readLineOrDefault("Enter message", "Hello HMAC");

    std::vector<uint8_t> key(keyStr.begin(), keyStr.end());
    std::vector<uint8_t> msg(msgStr.begin(), msgStr.end());

    std::cout << "HMAC-SHA256: " << HMAC_SHA256::computeHex(key, msg) << std::endl;
}

// ------------------------------------------------------------
// Пользовательский PBKDF2 режим
// ------------------------------------------------------------
void runUserPBKDF2Mode() {
    std::cout << "\n===== USER PBKDF2 MODE =====" << std::endl;

    std::string password = readLineOrDefault("Enter password", "password");
    std::string salt = readLineOrDefault("Enter salt", "salt");
    size_t iterations = readSizeOrDefault("Enter iterations", 1000);
    size_t keyLength = readSizeOrDefault("Enter key length", 32);

    std::vector<uint8_t> dk = PBKDF2::deriveKey(password, salt, static_cast<int>(iterations), keyLength);

    std::cout << "PBKDF2 output: " << SHA256::bytesToHex(dk) << std::endl;
}

void runSHA512Tests() {
    std::cout << "\n===== SHA-512 TESTS =====" << std::endl;
    
    std::string test1 = "abc";
    std::string test2 = "";

    std::string expected1 =
        "ddaf35a193617abacc417349ae204131"
        "12e6fa4e89a97ea20a9eeee64b55d39a"
        "2192992a274fc1a836ba3c23a3feebbd"
        "454d4423643ce80e2a9ac94fa54ca49f";

    std::string expected2 =
        "cf83e1357eefb8bdf1542850d66d8007"
        "d620e4050b5715dc83f4a921d36ce9ce"
        "47d0d13c5d85f2b0ff8318d2877eec2f"
        "63b931bd47417a81a538327af927da3e";

    std::string result1 = SHA512::hashHex(test1);
    std::string result2 = SHA512::hashHex(test2);

    std::cout << "\nTest 1: \"abc\"" << std::endl;
    showLoading("Testing", 3, 250);
    std::cout << "Result:   " << result1 << std::endl;
    std::cout << "Expected: " << expected1 << std::endl;
    std::cout << "Status:   " << (result1 == expected1 ? "PASS" : "FAIL") << std::endl;

    std::cout << "\nTest 2: \"\"" << std::endl;
    showLoading("Testing", 3, 250);
    std::cout << "Result:   " << result2 << std::endl;
    std::cout << "Expected: " << expected2 << std::endl;
    std::cout << "Status:   " << (result2 == expected2 ? "PASS" : "FAIL") << std::endl;
}


// ------------------------------------------------------------
// Запуск всех тестов
// ------------------------------------------------------------
void runAllTests() {
    std::cout << "\nStarting all tests..." << std::endl;
    pauseMs(500);

    std::cout << "\n[Step 1/6] Running SHA-256 tests..." << std::endl;
    pauseMs(500);
    runSHA256Tests();

     std::cout << "\n[Step 2/6] Running SHA-512 tests..." << std::endl;
    pauseMs(500);
    runSHA512Tests();

    std::cout << "\n[Step 3/6] Running HMAC test..." << std::endl;
    pauseMs(500);
    runHMACTest();

    std::cout << "\n[Step 4/6] Running PBKDF2 test..." << std::endl;
    pauseMs(500);
    runPBKDF2Test();

    std::cout << "\n[Step 5/6] Running HKDF test..." << std::endl;
    pauseMs(500);
    runHKDFTest();

    std::cout << "\n[Step 6/6] Running functional tests..." << std::endl;
    pauseMs(500);
    runFunctionalTests();

    std::cout << "\nAll tests completed." << std::endl;
    pauseMs(300);
}

// ------------------------------------------------------------
// Меню
// ------------------------------------------------------------
void printMenu() {
    std::cout << "\n";
    std::cout << "========================================" << std::endl;
    std::cout << "              MAIN MENU                 " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Run all tests" << std::endl;
    std::cout << "2. Run SHA-256 tests" << std::endl;
    std::cout << "3. Run HMAC test" << std::endl;
    std::cout << "4. Run PBKDF2 test" << std::endl;
    std::cout << "5. Run HKDF test" << std::endl;
    std::cout << "6. Run functional tests" << std::endl;
    std::cout << "7. User SHA-256 mode" << std::endl;
    std::cout << "8. User HMAC mode" << std::endl;
    std::cout << "9. User PBKDF2 mode" << std::endl;
    std::cout << "10. User HKDF mode" << std::endl;
    std::cout << "11. (BONUS TASK) Run SHA-512 tests" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Choose an option: ";
}   


int main() {
    printLogo();

    while (true) {
        printMenu();

        std::string choiceStr;
        std::getline(std::cin, choiceStr);

        int choice = -1;

        if (!choiceStr.empty()) {
            try {
                choice = std::stoi(choiceStr);
            } catch (...) {
                choice = -1;
            }
        }

        switch (choice) {
            case 1:
                runAllTests();
                break;
            case 2:
                runSHA256Tests();
                break;
            case 3:
                runHMACTest();
                break;
            case 4:
                runPBKDF2Test();
                break;
            case 5:
                runHKDFTest();
                break;
            case 6:
                runFunctionalTests();
                break;
            case 7:
                runUserSHA256Mode();
                break;
            case 8:
                runUserHMACMode();
                break;
            case 9:
                runUserPBKDF2Mode();
                break;
            case 10:
                runUserHKDFMode();
                break;
            case 11:
                runSHA512Tests();
                break;    
            case 0:
                std::cout << "Exiting program..." << std::endl;
                return 0;
            default:
                std::cout << "Invalid option. Please try again." << std::endl;
                break;
        }
    }
}