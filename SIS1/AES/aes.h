#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace aes {

    // KeySchedule: хранит все round keys подряд
    // rounds: 10 / 12 / 14
    // roundKeys: 16*(rounds+1) байт
    struct KeySchedule {
        int rounds = 0;
        std::vector<uint8_t> roundKeys;
    };

    // keyBytes must be 16, 24, or 32
    KeySchedule expandKey(const uint8_t* key, size_t keyBytes);

    // Encrypt / decrypt exactly one 16-byte block
    void encryptBlock(const uint8_t plaintext[16], uint8_t ciphertext[16], const KeySchedule& ks);
    void decryptBlock(const uint8_t ciphertext[16], uint8_t plaintext[16], const KeySchedule& ks);

} // namespace aes
