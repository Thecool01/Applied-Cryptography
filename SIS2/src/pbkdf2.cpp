#include "pbkdf2.h"
#include "hmac.h"
#include <cmath>

static std::vector<uint8_t> intToBytes(uint32_t i) {
    std::vector<uint8_t> b(4);
    b[0] = (i >> 24) & 0xff;
    b[1] = (i >> 16) & 0xff;
    b[2] = (i >> 8) & 0xff;
    b[3] = i & 0xff;
    return b;
}

static std::vector<uint8_t> xorVectors(
    const std::vector<uint8_t>& a,
    const std::vector<uint8_t>& b
) {
    std::vector<uint8_t> r(a.size());

    for (size_t i = 0; i < a.size(); i++) {
        r[i] = a[i] ^ b[i];
    }

    return r;
}

std::vector<uint8_t> PBKDF2::deriveKey(
    const std::string& password,
    const std::string& salt,Z
    int iterations,
    size_t keyLength
) {

    const size_t hashLen = 32;

    size_t blocks = std::ceil((double)keyLength / hashLen);

    std::vector<uint8_t> dk;

    std::vector<uint8_t> key(password.begin(), password.end());
    std::vector<uint8_t> saltBytes(salt.begin(), salt.end());

    for (uint32_t i = 1; i <= blocks; i++) {

        std::vector<uint8_t> msg = saltBytes;
        auto ib = intToBytes(i);
        msg.insert(msg.end(), ib.begin(), ib.end());

        std::vector<uint8_t> U =
            HMAC_SHA256::compute(key, msg);

        std::vector<uint8_t> T = U;

        for (int j = 1; j < iterations; j++) {

            U = HMAC_SHA256::compute(key, U);

            T = xorVectors(T, U);
        }

        dk.insert(dk.end(), T.begin(), T.end());
    }

    dk.resize(keyLength);

    return dk;
}