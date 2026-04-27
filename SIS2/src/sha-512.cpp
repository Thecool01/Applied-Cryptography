#include "sha-512.h"
#include <sstream>
#include <iomanip>

// ------------------------------------------------------------
// Константы K для SHA-512
// Используются в 80 раундах сжатия.
// ------------------------------------------------------------
static const uint64_t K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
    0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
    0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
    0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
    0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
    0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
    0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
    0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
    0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
    0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
    0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
    0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
    0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
    0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
    0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
    0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
    0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
    0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
    0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
    0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

// ------------------------------------------------------------
// Конструктор
// Здесь записываются стартовые значения H для SHA-512.
// ------------------------------------------------------------
SHA512::SHA512() : bitLengthHigh(0), bitLengthLow(0) {
    H[0] = 0x6a09e667f3bcc908ULL;
    H[1] = 0xbb67ae8584caa73bULL;
    H[2] = 0x3c6ef372fe94f82bULL;
    H[3] = 0xa54ff53a5f1d36f1ULL;
    H[4] = 0x510e527fade682d1ULL;
    H[5] = 0x9b05688c2b3e6c1fULL;
    H[6] = 0x1f83d9abfb41bd6bULL;
    H[7] = 0x5be0cd19137e2179ULL;
}

// ------------------------------------------------------------
// Добавление строки в buffer
// ------------------------------------------------------------
void SHA512::update(const std::string& data) {
    for (char c : data) {
        buffer.push_back(static_cast<uint8_t>(c));
    }

    uint64_t bitsToAdd = static_cast<uint64_t>(data.size()) * 8ULL;
    uint64_t oldLow = bitLengthLow;
    bitLengthLow += bitsToAdd;

    if (bitLengthLow < oldLow) {
        bitLengthHigh++;
    }
}

// ------------------------------------------------------------
// Добавление массива байтов в buffer
// ------------------------------------------------------------
void SHA512::update(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        buffer.push_back(byte);
    }

    uint64_t bitsToAdd = static_cast<uint64_t>(data.size()) * 8ULL;
    uint64_t oldLow = bitLengthLow;
    bitLengthLow += bitsToAdd;

    if (bitLengthLow < oldLow) {
        bitLengthHigh++;
    }
}

// ------------------------------------------------------------
// ROTR(x, n)
// ------------------------------------------------------------
uint64_t SHA512::rotr(uint64_t x, uint64_t n) {
    return (x >> n) | (x << (64 - n));
}

// ------------------------------------------------------------
// SHR(x, n)
// ------------------------------------------------------------
uint64_t SHA512::shr(uint64_t x, uint64_t n) {
    return x >> n;
}

// ------------------------------------------------------------
// Ch(x, y, z)
// ------------------------------------------------------------
uint64_t SHA512::ch(uint64_t x, uint64_t y, uint64_t z) {
    return (x & y) ^ (~x & z);
}

// ------------------------------------------------------------
// Maj(x, y, z)
// ------------------------------------------------------------
uint64_t SHA512::maj(uint64_t x, uint64_t y, uint64_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

// ------------------------------------------------------------
// Большие сигмы SHA-512
// ------------------------------------------------------------
uint64_t SHA512::bigSigma0(uint64_t x) {
    return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39);
}

uint64_t SHA512::bigSigma1(uint64_t x) {
    return rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41);
}

// ------------------------------------------------------------
// Малые сигмы SHA-512
// ------------------------------------------------------------
uint64_t SHA512::smallSigma0(uint64_t x) {
    return rotr(x, 1) ^ rotr(x, 8) ^ shr(x, 7);
}

uint64_t SHA512::smallSigma1(uint64_t x) {
    return rotr(x, 19) ^ rotr(x, 61) ^ shr(x, 6);
}

// ------------------------------------------------------------
// padMessage()
// Для SHA-512:
// 1. Добавляем 0x80
// 2. Добиваем нулями до 112 mod 128
// 3. Добавляем длину в битах как 128-битное big-endian число
// ------------------------------------------------------------
void SHA512::padMessage(std::vector<uint8_t>& padded) const {
    padded = buffer;

    padded.push_back(0x80);

    while ((padded.size() % 128) != 112) {
        padded.push_back(0x00);
    }

    // Старшие 64 бита длины
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<uint8_t>((bitLengthHigh >> (i * 8)) & 0xFF));
    }

    // Младшие 64 бита длины
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<uint8_t>((bitLengthLow >> (i * 8)) & 0xFF));
    }
}

// ------------------------------------------------------------
// processBlock()
// Обрабатывает один блок из 128 байт
// ------------------------------------------------------------
void SHA512::processBlock(const uint8_t block[128]) {
    uint64_t W[80];

    // Первые 16 слов по 64 бита
    for (int t = 0; t < 16; ++t) {
        W[t] =
            (static_cast<uint64_t>(block[t * 8]) << 56) |
            (static_cast<uint64_t>(block[t * 8 + 1]) << 48) |
            (static_cast<uint64_t>(block[t * 8 + 2]) << 40) |
            (static_cast<uint64_t>(block[t * 8 + 3]) << 32) |
            (static_cast<uint64_t>(block[t * 8 + 4]) << 24) |
            (static_cast<uint64_t>(block[t * 8 + 5]) << 16) |
            (static_cast<uint64_t>(block[t * 8 + 6]) << 8)  |
            (static_cast<uint64_t>(block[t * 8 + 7]));
    }

    // Достраиваем W[16..79]
    for (int t = 16; t < 80; ++t) {
        W[t] = smallSigma1(W[t - 2])
             + W[t - 7]
             + smallSigma0(W[t - 15])
             + W[t - 16];
    }

    // Рабочие переменные
    uint64_t a = H[0];
    uint64_t b = H[1];
    uint64_t c = H[2];
    uint64_t d = H[3];
    uint64_t e = H[4];
    uint64_t f = H[5];
    uint64_t g = H[6];
    uint64_t h = H[7];

    // 80 раундов
    for (int t = 0; t < 80; ++t) {
        uint64_t T1 = h + bigSigma1(e) + ch(e, f, g) + K[t] + W[t];
        uint64_t T2 = bigSigma0(a) + maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    // Обновляем состояние
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

// ------------------------------------------------------------
// digest()
// ------------------------------------------------------------
std::vector<uint8_t> SHA512::digest() {
    std::vector<uint8_t> padded;
    padMessage(padded);

    for (size_t i = 0; i < padded.size(); i += 128) {
        processBlock(&padded[i]);
    }

    std::vector<uint8_t> result;
    result.reserve(64);

    for (int i = 0; i < 8; ++i) {
        result.push_back(static_cast<uint8_t>((H[i] >> 56) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 48) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 40) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 32) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 24) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 16) & 0xFF));
        result.push_back(static_cast<uint8_t>((H[i] >> 8) & 0xFF));
        result.push_back(static_cast<uint8_t>(H[i] & 0xFF));
    }

    return result;
}

// ------------------------------------------------------------
// bytesToHex()
// ------------------------------------------------------------
std::string SHA512::bytesToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;

    for (uint8_t b : bytes) {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(b);
    }

    return oss.str();
}

// ------------------------------------------------------------
// hexdigest()
// ------------------------------------------------------------
std::string SHA512::hexdigest() {
    return bytesToHex(digest());
}

// ------------------------------------------------------------
// SHA512::hash(...)
// ------------------------------------------------------------
std::vector<uint8_t> SHA512::hash(const std::string& data) {
    SHA512 sha;
    sha.update(data);
    return sha.digest();
}

// ------------------------------------------------------------
// SHA512::hashHex(...)
// ------------------------------------------------------------
std::string SHA512::hashHex(const std::string& data) {
    SHA512 sha;
    sha.update(data);
    return sha.hexdigest();
}