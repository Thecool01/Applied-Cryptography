#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <cstdint>

class SHA256 {
public:
    SHA256();

    // Добавить строку в сообщение
    void update(const std::string& data);

    // Добавить массив байтов в сообщение
    void update(const std::vector<uint8_t>& data);

    // Получить итоговый хеш в виде массива из 32 байт
    std::vector<uint8_t> digest();

    // Получить итоговый хеш в hex-строке
    std::string hexdigest();

    // Удобные статические методы:
    // можно сразу вызвать SHA256::hashHex("abc")
    static std::vector<uint8_t> hash(const std::string& data);
    static std::string hashHex(const std::string& data);

private:
    // Здесь храним всё входное сообщение
    std::vector<uint8_t> buffer;

    // Длина исходного сообщения в битах
    uint64_t bitLength;

    // Текущее внутреннее состояние SHA-256:
    // 8 слов по 32 бита
    uint32_t H[8];

    // Обработка одного блока размером 512 бит = 64 байта
    void processBlock(const uint8_t block[64]);

    // Выполнить padding сообщения
    void padMessage(std::vector<uint8_t>& padded) const;

    // Побитовый циклический сдвиг вправо
    static uint32_t rotr(uint32_t x, uint32_t n);

    // Обычный сдвиг вправо
    static uint32_t shr(uint32_t x, uint32_t n);

    // Функции SHA-256
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z);
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z);

    static uint32_t bigSigma0(uint32_t x);
    static uint32_t bigSigma1(uint32_t x);
    static uint32_t smallSigma0(uint32_t x);
    static uint32_t smallSigma1(uint32_t x);

    static std::string bytesToHex(const std::vector<uint8_t>& bytes);
};

#endif