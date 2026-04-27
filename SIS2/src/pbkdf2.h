#ifndef PBKDF2_H
#define PBKDF2_H

#include <vector>
#include <string>
#include <cstdint>

class PBKDF2 {
public:
    static std::vector<uint8_t> deriveKey(
        const std::string& password,
        const std::string& salt,
        int iterations,
        size_t keyLength
    );
};

#endif