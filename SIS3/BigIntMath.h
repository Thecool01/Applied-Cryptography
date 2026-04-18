#ifndef BIGINTMATH_H
#define BIGINTMATH_H

#include <boost/multiprecision/cpp_int.hpp>

// Используем пространство имен для удобства
using namespace boost::multiprecision;

class BigIntMath {
public:
    // Нахождение НОД (Алгоритм Евклида)
    static cpp_int gcd(cpp_int a, cpp_int b);

    // Быстрое возведение в степень по модулю (Square-and-Multiply)
    static cpp_int modExp(cpp_int base, cpp_int exponent, cpp_int modulus);

    // Расширенный алгоритм Евклида (возвращает обратное число d)
    static cpp_int modInverse(cpp_int a, cpp_int m);
};

#endif // BIGINTMATH_H