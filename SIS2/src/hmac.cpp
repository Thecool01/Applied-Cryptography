#include "hmac.h"
#include "sha-256.h"

static const size_t BLOCK_SIZE = 64;

static std::vector<uint8_t> xorPad(
    const std::vector<uint8_t>& key,
    uint8_t pad
) {
    std::vector<uint8_t> result(BLOCK_SIZE);

    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        result[i] = key[i] ^ pad;
    }

    return result;
}

std::vector<uint8_t> HMAC_SHA256::compute(
    const std::vector<uint8_t>& keyInput,
    const std::vector<uint8_t>& message
) {

    std::vector<uint8_t> key = keyInput;

    if (key.size() > BLOCK_SIZE) {
        key = SHA256::hash(std::string(key.begin(), key.end()));
    }

    if (key.size() < BLOCK_SIZE) {
        key.resize(BLOCK_SIZE, 0x00);
    }

    std::vector<uint8_t> o_key = xorPad(key, 0x5c);
    std::vector<uint8_t> i_key = xorPad(key, 0x36);

    std::vector<uint8_t> innerData = i_key;
    innerData.insert(innerData.end(), message.begin(), message.end());

    std::vector<uint8_t> innerHash =
        SHA256::hash(std::string(innerData.begin(), innerData.end()));

    std::vector<uint8_t> outerData = o_key;
    outerData.insert(outerData.end(), innerHash.begin(), innerHash.end());

    return SHA256::hash(std::string(outerData.begin(), outerData.end()));
}

std::string HMAC_SHA256::computeHex(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& message
) {
    auto result = compute(key, message);
    return SHA256::bytesToHex(result);
}