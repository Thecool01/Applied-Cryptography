#include "RSACore.h"
#include "PrimeGenerator.h"
#include "BigIntMath.h"
#include <iostream>

void RSACore::generateKeys(int keySizeBits, PublicKey& pubKey, PrivateKey& privKey) {
    // Размер каждого простого числа должен быть равен половине ключа
    int primeSize = keySizeBits / 2;
    
    cpp_int p, q, n, phi, e, d;

    // 1. Генерируем p и q
    std::cout << "Generating prime p (" << primeSize << " bits)..." << std::endl;
    p = PrimeGenerator::generatePrime(primeSize);
    
    std::cout << "Generating prime q (" << primeSize << " bits)..." << std::endl;
    do {
        q = PrimeGenerator::generatePrime(primeSize);
    } while (p == q); // Убеждаемся, что p и q не одинаковые

    // 2. Вычисляем модуль n = p * q
    std::cout << "Calculating n and phi..." << std::endl;
    n = p * q;

    // 3. Вычисляем функцию Эйлера phi = (p - 1) * (q - 1)
    phi = (p - 1) * (q - 1);

    // 4. Выбираем открытую экспоненту e
    e = 65537; // Общепринятый стандарт
    if (BigIntMath::gcd(e, phi) != 1) {
        // Если e и phi не взаимно простые (вероятность почти нулевая), 
        // математика RSA сломается. Выводим предупреждение.
        std::cout << "WARNING: e and phi are not coprime!" << std::endl;
    }

    // 5. Вычисляем закрытую экспоненту d = e^-1 mod phi
    std::cout << "Calculating private key (d)..." << std::endl;
    d = BigIntMath::modInverse(e, phi);

    // Сохраняем результаты в структуры
    pubKey.n = n;
    pubKey.e = e;

    privKey.n = n;
    privKey.d = d;
    privKey.p = p; 
    privKey.q = q;

    std::cout << ">>> RSA Key Pair Generation Complete! <<<" << std::endl;
} // <--- ВОТ ЗДЕСЬ ФУНКЦИЯ ЗАКРЫВАЕТСЯ

// --- ДАЛЬШЕ ИДУТ НОВЫЕ ФУНКЦИИ ---

// Шифрование: C = M^e mod n
cpp_int RSACore::encryptTextbook(const cpp_int& m, const PublicKey& pubKey) {
    if (m >= pubKey.n) {
        throw std::invalid_argument("Message must be smaller than modulus n");
    }
    return BigIntMath::modExp(m, pubKey.e, pubKey.n);
}

// Дешифрование: M = C^d mod n
cpp_int RSACore::decryptTextbook(const cpp_int& c, const PrivateKey& privKey) {
    return BigIntMath::modExp(c, privKey.d, privKey.n);
}

// БОНУС: Оптимизированное дешифрование через CRT
cpp_int RSACore::decryptCRT(const cpp_int& c, const PrivateKey& privKey) {
    // 1. Вычисляем экспоненты для p и q
    cpp_int dP = privKey.d % (privKey.p - 1);
    cpp_int dQ = privKey.d % (privKey.q - 1);

    // 2. Вычисляем qInv (обратное к q по модулю p)
    cpp_int qInv = BigIntMath::modInverse(privKey.q, privKey.p);

    // 3. Вычисляем частичные сообщения m1 и m2
    cpp_int m1 = BigIntMath::modExp(c, dP, privKey.p);
    cpp_int m2 = BigIntMath::modExp(c, dQ, privKey.q);

    // 4. Сборка по формуле Ганера (Garner's formula)
    cpp_int h = (qInv * (m1 - m2)) % privKey.p;
    
    // В C++ операция % может вернуть отрицательное число. 
    // Если это произошло, просто прибавляем модуль p
    if (h < 0) {
        h += privKey.p; 
    }

    // Итоговое сообщение
    cpp_int m = m2 + h * privKey.q;
    return m;
}