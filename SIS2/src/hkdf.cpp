#include "hkdf.h"
#include "hmac.h"
#include <stdexcept>

// ------------------------------------------------------------
// Для SHA-256 длина хеша = 32 байта
// Именно такой длины будет PRK после Extract.
// ------------------------------------------------------------
static const size_t HASH_LEN = 32;

// ------------------------------------------------------------
// Перевод string в vector<uint8_t>
// Нужен для удобной перегрузки deriveKey(...)
// ------------------------------------------------------------
static std::vector<uint8_t> stringToBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

// ------------------------------------------------------------
// HKDF-Extract
// Формула: PRK = HMAC(salt, IKM)
// Где:
// - salt = необязательная соль
// - IKM  = input key material
// - PRK  = pseudorandom key
// Если salt пустой, по стандартной идее HKDF можно использовать
// массив из HashLen нулевых байтов.
// Для SHA-256 это 32 нулевых байта.
// ------------------------------------------------------------
std::vector<uint8_t> HKDF::extract(
    const std::vector<uint8_t>& salt,
    const std::vector<uint8_t>& ikm
) {
    std::vector<uint8_t> actualSalt = salt;

    // Если salt не задан, используем 32 нулевых байта
    if (actualSalt.empty()) {
        actualSalt.resize(HASH_LEN, 0x00);
    }

    return HMAC_SHA256::compute(actualSalt, ikm);
}

// ------------------------------------------------------------
// HKDF-Expand
// Формулы:
// N = ceil(L / HashLen)
// T(0) = empty
// T(i) = HMAC(PRK, T(i-1) || info || i)
// OKM = first L bytes of T(1) || T(2) || ...
// Важно:
// - i добавляется как один байт
// - максимальное число блоков = 255
// - значит максимальная длина = 255 * HashLen
// ------------------------------------------------------------
std::vector<uint8_t> HKDF::expand(
    const std::vector<uint8_t>& prk,
    const std::vector<uint8_t>& info,
    size_t length
) {
    // Если длина 0, возвращаем пустой результат
    if (length == 0) {
        return {};
    }

    // Максимум по HKDF: 255 блоков по HashLen байт
    if (length > 255 * HASH_LEN) {
        throw std::invalid_argument("HKDF output length is too large");
    }

    // Сколько блоков T(i) нужно получить
    size_t n = (length + HASH_LEN - 1) / HASH_LEN;

    std::vector<uint8_t> okm;      // Output Keying Material
    std::vector<uint8_t> previous; // T(i-1), сначала пустой

    okm.reserve(n * HASH_LEN);

    for (size_t i = 1; i <= n; i++) {
        // data = T(i-1) || info || i
        std::vector<uint8_t> data = previous;
        data.insert(data.end(), info.begin(), info.end());

        // Номер блока добавляем как 1 байт
        data.push_back(static_cast<uint8_t>(i));

        // T(i) = HMAC(PRK, data)
        previous = HMAC_SHA256::compute(prk, data);

        // Добавляем T(i) в общий результат
        okm.insert(okm.end(), previous.begin(), previous.end());
    }

    // Оставляем только первые length байт
    okm.resize(length);

    return okm;
}

// ------------------------------------------------------------
// Полный HKDF:
// 1. Extract
// 2. Expand
// ------------------------------------------------------------
std::vector<uint8_t> HKDF::deriveKey(
    const std::vector<uint8_t>& ikm,
    const std::vector<uint8_t>& salt,
    const std::vector<uint8_t>& info,
    size_t length
) {
    std::vector<uint8_t> prk = extract(salt, ikm);
    return expand(prk, info, length);
}

// ------------------------------------------------------------
// Удобная версия для строк
// ------------------------------------------------------------
std::vector<uint8_t> HKDF::deriveKey(
    const std::string& ikm,
    const std::string& salt,
    const std::string& info,
    size_t length
) {
    return deriveKey(
        stringToBytes(ikm),
        stringToBytes(salt),
        stringToBytes(info),
        length
    );
}