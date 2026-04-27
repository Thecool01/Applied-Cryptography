#ifndef FILE_INTEGRITY_H
#define FILE_INTEGRITY_H

#include <vector>
#include <string>
#include <cstdint>

// вычисление SHA256 файла
std::vector<uint8_t> sha256File(const std::string& filename);

// проверка файла
bool verifyFile(
    const std::string& filename,
    const std::vector<uint8_t>& storedHash
);

#endif