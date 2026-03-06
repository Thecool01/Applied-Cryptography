#include <iostream>
#include <string>
#include "sha-256.h"

int main() {
    // Тестовые строки из задания
    std::string test1 = "abc";
    std::string test2 = "";
    std::string test3 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    // Ожидаемые значения SHA-256
    std::string expected1 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string expected2 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string expected3 = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";

    // Вычисляем хеши
    std::string result1 = SHA256::hashHex(test1);
    std::string result2 = SHA256::hashHex(test2);
    std::string result3 = SHA256::hashHex(test3);

    // Выводим результаты первого теста
    std::cout << "===== SHA-256 Test 1 =====" << std::endl;
    std::cout << "Input: \"abc\"" << std::endl;
    std::cout << "Result:   " << result1 << std::endl;
    std::cout << "Expected: " << expected1 << std::endl;

    // Проверяем, совпал ли результат
    if (result1 == expected1) {
        std::cout << "Status: PASS" << std::endl;
    } else {
        std::cout << "Status: FAIL" << std::endl;
    }

    std::cout << std::endl;

    // Выводим результаты второго теста
    std::cout << "===== SHA-256 Test 2 =====" << std::endl;
    std::cout << "Input: \"\"" << std::endl;
    std::cout << "Result:   " << result2 << std::endl;
    std::cout << "Expected: " << expected2 << std::endl;

    // Проверяем, совпал ли результат
    if (result2 == expected2) {
        std::cout << "Status: PASS" << std::endl;
    } else {
        std::cout << "Status: FAIL" << std::endl;
    }

    std::cout << std::endl;

    // Выводим результаты третьего теста
    std::cout << "===== SHA-256 Test 3 =====" << std::endl;
    std::cout << "Input: \"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\"" << std::endl;
    std::cout << "Result:   " << result3 << std::endl;
    std::cout << "Expected: " << expected3 << std::endl;

    // Проверяем, совпал ли результат
    if (result3 == expected3) {
        std::cout << "Status: PASS" << std::endl;
    } else {
        std::cout << "Status: FAIL" << std::endl;
    }

    std::cout << std::endl;

    // Небольшой пользовательский режим:
    // можно ввести свою строку и получить её SHA-256
    std::string userInput;
    std::cout << "Enter your own text to hash: ";
    std::getline(std::cin, userInput);

    std::string userHash = SHA256::hashHex(userInput);

    std::cout << "SHA-256: " << userHash << std::endl;

    return 0;
}