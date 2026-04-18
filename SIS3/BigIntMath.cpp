#include "BigIntMath.h"

// Стандартный алгоритм Евклида
cpp_int BigIntMath::gcd(cpp_int a, cpp_int b) {
    while (b != 0) {
        cpp_int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Быстрое возведение в степень (Square-and-Multiply)
cpp_int BigIntMath::modExp(cpp_int base, cpp_int exponent, cpp_int modulus) {
    cpp_int result = 1;
    base = base % modulus;
    
    while (exponent > 0) {
        // Если степень нечетная, умножаем результат на текущее основание
        if (exponent % 2 == 1) {
            result = (result * base) % modulus;
        }
        // Сдвигаем степень вправо (эквивалентно делению на 2)
        exponent = exponent / 2; // в Boost можно использовать exponent >>= 1
        
        // Возводим основание в квадрат по модулю
        base = (base * base) % modulus;
    }
    return result;
}

// Расширенный алгоритм Евклида
cpp_int BigIntMath::modInverse(cpp_int a, cpp_int m) {
    cpp_int m0 = m;
    cpp_int y = 0, x = 1;

    if (m == 1) return 0;

    while (a > 1) {
        // q - частное
        cpp_int q = a / m;
        cpp_int t = m;

        // m становится остатком, как в обычном алгоритме
        m = a % m;
        a = t;
        t = y;

        // Обновляем x и y
        y = x - q * y;
        x = t;
    }

    // Если x стал отрицательным, переводим его в положительный диапазон модуля
    if (x < 0) {
        x += m0;
    }

    return x;
}