#pragma once
#include <cstdint>
#include <cstring>

// Represents a 128-bit block (16 bytes), used in GCM operations
struct Block128 {
    uint8_t bytes[16];

    // Constructor: initialize all bytes to zero
    Block128() {
        std::memset(bytes, 0, 16);
    }

    // XOR this block with another 128-bit block (byte by byte)
    // XOR is the "addition" operation in GF(2^128)
    void XOR(const Block128& other) {
        for (int i = 0; i < 16; i++) {
            bytes[i] ^= other.bytes[i];
        }
    }
};

// Contains mathematical operations required for AES-GCM
class GCM_Math {
public:
    // Multiply two 128-bit values in the finite field GF(2^128)
    // This function is used by GHASH to authenticate data
    static Block128 gf128_multiply(Block128 X, Block128 Y) {
        Block128 Z;        // Result block, initially zero
        Block128 V = Y;    // Temporary value, starts as Y

        // Reduction constant defined by the GCM standard polynomial
        const uint8_t R = 0xE1;

        // Process all 128 bits of X
        for (int i = 0; i < 128; ++i) {

            // Check the current bit of X
            // If the bit is 1, XOR V into the result
            if ((X.bytes[i / 8] >> (7 - (i % 8))) & 1) {
                Z.XOR(V);
            }

            // Check if the least significant bit of V is set
            bool lsb_set = (V.bytes[15] & 1);

            // Shift V right by one bit (across all 16 bytes)
            for (int j = 15; j > 0; --j) {
                V.bytes[j] = (V.bytes[j] >> 1) | (V.bytes[j - 1] << 7);
            }
            V.bytes[0] >>= 1;

            // If the shifted-out bit was 1, apply reduction
            // This keeps the value inside GF(2^128)
            if (lsb_set) {
                V.bytes[0] ^= R;
            }
        }

        // Return the final multiplication result
        return Z;
    }
};
