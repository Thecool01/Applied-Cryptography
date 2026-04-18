#include "file_integrity.h"
#include "sha-256.h"
#include <fstream>

// ------------------------------------------------------------
// вычисление SHA256 файла
// ------------------------------------------------------------
std::vector<uint8_t> sha256File(const std::string& filename) {

    // открываем файл для чтения
    std::ifstream file(filename, std::ios::binary);

    SHA256 sha;

    char buffer[4096];

    // читаем файл по частям
    while (file.good()) {

        file.read(buffer, sizeof(buffer));

        std::vector<uint8_t> chunk(
            buffer,
            buffer + file.gcount()
        );

        sha.update(chunk);
    }

    // возвращаем hash файла
    return sha.digest();
}


// ------------------------------------------------------------
// проверка целостности файла
// ------------------------------------------------------------
bool verifyFile(
    const std::string& filename,
    const std::vector<uint8_t>& storedHash)
{

    std::vector<uint8_t> currentHash = sha256File(filename);

    return currentHash == storedHash;
}