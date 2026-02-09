#pragma once
#include <vector>
#include <stdexcept>
#include <cstdint>

class PKCS7 {
public:
    static std::vector<uint8_t> pad(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> padded = data;

        uint8_t padding_len = (uint8_t)(16 - (data.size() % 16));
        if (padding_len == 0) padding_len = 16;

        padded.insert(padded.end(), padding_len, padding_len);
        return padded;
    }

    static std::vector<uint8_t> unpad(const std::vector<uint8_t>& data) {
        if (data.empty()) throw std::runtime_error("Empty data for unpadding");
        if (data.size() % 16 != 0) throw std::runtime_error("Invalid padded length");

        uint8_t padding_len = data.back();

        if (padding_len == 0 || padding_len > 16 || padding_len > data.size()) {
            throw std::runtime_error("Invalid PKCS#7 padding length");
        }

        for (size_t i = 0; i < padding_len; ++i) {
            if (data[data.size() - 1 - i] != padding_len) {
                throw std::runtime_error("Invalid PKCS#7 padding bytes");
            }
        }

        return std::vector<uint8_t>(data.begin(), data.end() - padding_len);
    }
};
