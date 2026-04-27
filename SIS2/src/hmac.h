#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <vector>
#include <string>
#include <cstdint>

class HMAC_SHA256 {
public:
    static std::vector<uint8_t> compute(
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& message
    );

    static std::string computeHex(
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& message
    );
};

#endif