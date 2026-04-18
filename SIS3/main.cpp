#include <iostream>
#include "BigIntMath.h"
#include "RSACore.h"
#include "PrimeGenerator.h" // Обязательно подключаем новый файл

using namespace std;
using namespace boost::multiprecision;

int main() {
    cout << "--- Testing BigIntMath ---" << endl;
    cpp_int a = 48, b = 18;
    cout << "GCD(" << a << ", " << b << ") = " << BigIntMath::gcd(a, b) << " (Expected: 6)" << endl;
    
    cpp_int base = 2, exp = 10, mod = 1000;
    cout << base << "^" << exp << " mod " << mod << " = " << BigIntMath::modExp(base, exp, mod) << " (Expected: 24)" << endl;

    cpp_int e = 3, phi = 11;
    cout << "Modular inverse of " << e << " mod " << phi << " = " << BigIntMath::modInverse(e, phi) << " (Expected: 4)" << endl;
    cout << "-------------------------------" << endl;

    // ТЕСТИРУЕМ ГЕНЕРАЦИЮ ПРОСТЫХ ЧИСЕЛ
    cout << "\n--- Testing Prime Generation ---" << endl;
    cout << "Generating a 512-bit prime number. Please wait..." << endl;
    
    cpp_int p = PrimeGenerator::generatePrime(512);
    
    cout << "Generated Prime (p): " << endl;
    cout << p << endl;
    cout << "-------------------------------" << endl;


    cout << "--- RSA Key Generation ---" << endl;
    PublicKey pubKey;
    PrivateKey privKey;
    
    // Генерируем небольшие ключи для быстрого теста (например, 512 бит)
    RSACore::generateKeys(512, pubKey, privKey);
    
    cout << "\n--- Testing Encryption & Decryption ---" << endl;
    
    // Наше "сообщение" (пока это просто число, так как мы не добавили паддинг)
    cpp_int originalMessage = 123456789;
    cout << "Original Message: " << originalMessage << endl;

    // 1. Шифруем
    cpp_int ciphertext = RSACore::encryptTextbook(originalMessage, pubKey);
    cout << "Ciphertext: \n" << ciphertext << "\n" << endl;

    // 2. Дешифруем обычным (медленным) способом
    cpp_int decryptedTextbook = RSACore::decryptTextbook(ciphertext, privKey);
    cout << "Decrypted (Textbook): " << decryptedTextbook << endl;

    // 3. Дешифруем БОНУСНЫМ (быстрым) способом
    cpp_int decryptedCRT = RSACore::decryptCRT(ciphertext, privKey);
    cout << "Decrypted (CRT Method): " << decryptedCRT << endl;

    if (originalMessage == decryptedTextbook && originalMessage == decryptedCRT) {
        cout << "\nSUCCESS: Both decryption methods work perfectly!" << endl;
    } else {
        cout << "\nERROR: Decryption failed!" << endl;
    }


    return 0;
}