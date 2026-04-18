#ifndef RSACORE_H
#define RSACORE_H

#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

// Структура открытого ключа (n, e)
struct PublicKey {
    cpp_int n;
    cpp_int e;
};

// Структура закрытого ключа (n, d)
struct PrivateKey {
    cpp_int n;
    cpp_int d;
    // Сохраняем p и q на будущее для бонусного задания (CRT оптимизация)
    cpp_int p;
    cpp_int q;
};

class RSACore {
public:
    // Функция генерации пары ключей
    static void generateKeys(int keySizeBits, PublicKey& pubKey, PrivateKey& privKey);

    // Базовое математическое шифрование (Textbook RSA)
    static cpp_int encryptTextbook(const cpp_int& m, const PublicKey& pubKey);
    
    // Базовое математическое дешифрование (Textbook RSA)
    static cpp_int decryptTextbook(const cpp_int& c, const PrivateKey& privKey);

    // БОНУС: Дешифрование с использованием Китайской теоремы об остатках (CRT)
    static cpp_int decryptCRT(const cpp_int& c, const PrivateKey& privKey);
};

#endif // RSACORE_H