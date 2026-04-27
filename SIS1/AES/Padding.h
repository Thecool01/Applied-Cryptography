#pragma once
#include <vector>
#include <stdexcept>
#include <cstdint>

// Implements PKCS#7 padding for 16-byte block ciphers (AES)
class PKCS7 {
public:
    // Add PKCS#7 padding to the input data
    // Output size is always a multiple of 16 bytes
    static std::vector<uint8_t> pad(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> padded = data;

        // Calculate how many padding bytes are needed
        uint8_t padding_len = (uint8_t)(16 - (data.size() % 16));

        // If data already fits exactly, add a full padding block
        if (padding_len == 0) padding_len = 16;

        // Append padding_len bytes, each with value padding_len
        padded.insert(padded.end(), padding_len, padding_len);

        return padded;
    }

    // Remove and validate PKCS#7 padding
    static std::vector<uint8_t> unpad(const std::vector<uint8_t>& data) {
        // Data must not be empty
        if (data.empty()) {
            throw std::runtime_error("Empty data for unpadding");
        }

        // Padded data length must be a multiple of block size
        if (data.size() % 16 != 0) {
            throw std::runtime_error("Invalid padded length");
        }

        // The last byte tells how many padding bytes were added
        uint8_t padding_len = data.back();

        // Validate padding length
        if (padding_len == 0 || padding_len > 16 || padding_len > data.size()) {
            throw std::runtime_error("Invalid PKCS#7 padding length");
        }

        // Check that all padding bytes have the correct value
        for (size_t i = 0; i < padding_len; ++i) {
            if (data[data.size() - 1 - i] != padding_len) {
                throw std::runtime_error("Invalid PKCS#7 padding bytes");
            }
        }

        // Remove padding and return original data
        return std::vector<uint8_t>(data.begin(), data.end() - padding_len);
    }
};
