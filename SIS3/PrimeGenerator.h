#ifndef PRIMEGENERATOR_H
#define PRIMEGENERATOR_H

#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

class PrimeGenerator {
public:
    // Генерирует случайное нечетное число заданной длины в битах
    static cpp_int generateRandomOdd(int bitLength);

    // Вероятностный тест простоты Миллера-Рабина
    static bool millerRabinTest(const cpp_int& n, int iterations);

    // Главная функция: ищет и возвращает простое число нужной длины
    static cpp_int generatePrime(int bitLength);
};

#endif // PRIMEGENERATOR_H