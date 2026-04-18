#include "PrimeGenerator.h"
#include "BigIntMath.h"
#include "CustomRNG.h" // Подключаем твой класс
#include <iostream>
#include <vector>

// Создаем глобальный экземпляр твоего генератора
CustomRNG customRng;

cpp_int PrimeGenerator::generateRandomOdd(int bitLength) {
    // Вычисляем, сколько байтов нам нужно (например, для 512 бит нужно 64 байта)
    int byteLength = bitLength / 8;
    if (bitLength % 8 != 0) byteLength++; 

    // Создаем буфер и заполняем его случайными байтами из твоего генератора
    std::vector<uint8_t> buffer(byteLength);
    customRng.getBytes(buffer.data(), byteLength);

    // Склеиваем байты в одно большое число cpp_int
    cpp_int randomNum = 0;
    for (int i = 0; i < byteLength; i++) {
        randomNum = (randomNum << 8) | buffer[i];
    }
    
    // Устанавливаем старший бит в 1, чтобы число было ровно bitLength длины
    cpp_int mask = 1;
    mask <<= (bitLength - 1);
    randomNum |= mask;

    // Устанавливаем младший бит в 1, чтобы число гарантированно было нечетным
    randomNum |= 1;
    
    return randomNum;
}

bool PrimeGenerator::millerRabinTest(const cpp_int& n, int iterations) {
    // Базовые проверки
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false; // Исключаем четные числа

    // Представляем n-1 как (2^r) * d
    cpp_int d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }

    // Основной цикл Миллера-Рабина
    for (int i = 0; i < iterations; i++) {
        // Генерируем случайного "свидетеля" a с помощью твоего генератора.
        // Нам хватит 32 байт (256 бит) для случайности свидетеля.
        std::vector<uint8_t> a_buffer(32);
        customRng.getBytes(a_buffer.data(), 32);
        
        cpp_int a = 0;
        for (int j = 0; j < 32; j++) {
            a = (a << 8) | a_buffer[j];
        }
        
        // Ограничиваем a диапазоном [2, n-2]
        a = 2 + (a % (n - 3)); 

        // x = a^d mod n
        cpp_int x = BigIntMath::modExp(a, d, n);

        if (x == 1 || x == n - 1) {
            continue;
        }

        bool composite = true;
        
        for (int j = 0; j < r - 1; j++) {
            x = BigIntMath::modExp(x, 2, n);
            
            if (x == n - 1) {
                composite = false;
                break;
            }
        }

        if (composite) {
            return false; 
        }
    }

    return true; 
}

cpp_int PrimeGenerator::generatePrime(int bitLength) {
    cpp_int candidate;
    int attempts = 0;
    int iterations = 40; 

    while (true) {
        attempts++;
        candidate = generateRandomOdd(bitLength);
        
        if (millerRabinTest(candidate, iterations)) {
            std::cout << "Prime found after " << attempts << " attempts using CustomRNG!" << std::endl;
            return candidate;
        }
    }
}