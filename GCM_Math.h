#pragma once
#include <cstdint>
#include <cstring>

struct Block128 {
    uint8_t bytes[16];
    Block128() { std::memset(bytes, 0, 16); }

    void XOR(const Block128& other) {
        for (int i = 0; i < 16; i++) bytes[i] ^= other.bytes[i];
    }
};

class GCM_Math {
public:
    static Block128 gf128_multiply(Block128 X, Block128 Y) {
        Block128 Z;
        Block128 V = Y;
        const uint8_t R = 0xE1;

        for (int i = 0; i < 128; ++i) {
            if ((X.bytes[i / 8] >> (7 - (i % 8))) & 1) {
                Z.XOR(V);
            }

            bool lsb_set = (V.bytes[15] & 1);
            for (int j = 15; j > 0; --j) {
                V.bytes[j] = (V.bytes[j] >> 1) | (V.bytes[j - 1] << 7);
            }
            V.bytes[0] >>= 1;

            if (lsb_set) V.bytes[0] ^= R;
        }
        return Z;
    }
};
