#ifndef HKDF_H
#define HKDF_H

#include <vector>
#include <string>
#include <cstdint>

class HKDF {
public:
    // Полный HKDF:
    // сначала Extract, потом Expand
    static std::vector<uint8_t> deriveKey(
        const std::vector<uint8_t>& ikm,
        const std::vector<uint8_t>& salt,
        const std::vector<uint8_t>& info,
        size_t length
    );

    // Фаза Extract:
    // PRK = HMAC(salt, IKM)
    static std::vector<uint8_t> extract(
        const std::vector<uint8_t>& salt,
        const std::vector<uint8_t>& ikm
    );

    // Фаза Expand:
    // OKM = HKDF-Expand(PRK, info, length)
    static std::vector<uint8_t> expand(
        const std::vector<uint8_t>& prk,
        const std::vector<uint8_t>& info,
        size_t length
    );

    // Удобный вариант для строк
    static std::vector<uint8_t> deriveKey(
        const std::string& ikm,
        const std::string& salt,
        const std::string& info,
        size_t length
    );
};

#endif