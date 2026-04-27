#ifndef SHA512_H
#define SHA512_H

#include <string>
#include <vector>
#include <cstdint>

class SHA512 {
public:
    SHA512();

    // Добавить строку в сообщение
    void update(const std::string& data);

    // Добавить массив байтов в сообщение
    void update(const std::vector<uint8_t>& data);

    // Получить итоговый хеш в виде массива из 64 байт
    std::vector<uint8_t> digest();

    // Получить итоговый хеш в hex-строке
    std::string hexdigest();

    // Удобные статические методы
    static std::vector<uint8_t> hash(const std::string& data);
    static std::string hashHex(const std::string& data);
    static std::string bytesToHex(const std::vector<uint8_t>& bytes);

private:
    // Здесь храним всё входное сообщение
    std::vector<uint8_t> buffer;

    // Для SHA-512 длина сообщения хранится как 128-битное число.
    // Для учебного проекта достаточно двух 64-битных частей.
    uint64_t bitLengthHigh;
    uint64_t bitLengthLow;

    // Текущее внутреннее состояние SHA-512:
    // 8 слов по 64 бита
    uint64_t H[8];

    // Обработка одного блока размером 1024 бит = 128 байт
    void processBlock(const uint8_t block[128]);

    // Выполнить padding сообщения
    void padMessage(std::vector<uint8_t>& padded) const;

    // Побитовый циклический сдвиг вправо
    static uint64_t rotr(uint64_t x, uint64_t n);

    // Обычный сдвиг вправо
    static uint64_t shr(uint64_t x, uint64_t n);

    // Функции SHA-512
    static uint64_t ch(uint64_t x, uint64_t y, uint64_t z);
    static uint64_t maj(uint64_t x, uint64_t y, uint64_t z);

    static uint64_t bigSigma0(uint64_t x);
    static uint64_t bigSigma1(uint64_t x);
    static uint64_t smallSigma0(uint64_t x);
    static uint64_t smallSigma1(uint64_t x);
};

#endif