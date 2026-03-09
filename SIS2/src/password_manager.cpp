#include "password_manager.h"
#include "hkdf.h"
#include <random>

// ------------------------------------------------------------
// Генерация случайной соли
// ------------------------------------------------------------
std::vector<uint8_t> generateSalt(size_t size) {

    std::vector<uint8_t> salt(size);

    std::random_device rd;

    for (size_t i = 0; i < size; i++) {
        salt[i] = rd() % 256;
    }

    return salt;
}

// ------------------------------------------------------------
// Сохранение пароля
//
// Пароль преобразуется в байты,
// затем через HKDF создаётся hash
// ------------------------------------------------------------
StoredPassword storePassword(const std::string& password) {

    StoredPassword data;

    data.salt = generateSalt();

    std::vector<uint8_t> passwordBytes(
        password.begin(),
        password.end()
    );

    data.hash = HKDF::deriveKey(
        passwordBytes,
        data.salt,
        std::vector<uint8_t>{'p','a','s','s','w','o','r','d'},
        32
    );

    return data;
}

// ------------------------------------------------------------
// Проверка пароля
// ------------------------------------------------------------
bool verifyPassword(
    const std::string& password,
    const StoredPassword& stored)
{

    std::vector<uint8_t> passwordBytes(
        password.begin(),
        password.end()
    );

    std::vector<uint8_t> newHash = HKDF::deriveKey(
        passwordBytes,
        stored.salt,
        std::vector<uint8_t>{'p','a','s','s','w','o','r','d'},
        32
    );

    return newHash == stored.hash;
}