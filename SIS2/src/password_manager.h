#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include <vector>
#include <string>
#include <cstdint>

// Структура для хранения пароля
struct StoredPassword {
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
};

// генерация salt
std::vector<uint8_t> generateSalt(size_t size = 16);

// сохранение пароля
StoredPassword storePassword(const std::string& password);

// проверка пароля
bool verifyPassword(
    const std::string& password,
    const StoredPassword& stored
);

#endif