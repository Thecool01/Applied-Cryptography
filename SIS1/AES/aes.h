#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace aes {
  struct AESKeySchedule {
    int Nr = 0;
    std::vector<uint8_t> rk;
  };

  AESKeySchedule expandKey(const uint8_t* key, size_t keyBytes);
  void encryptBlock(const uint8_t in[16], uint8_t out[16], const AESKeySchedule& ks);
  void decryptBlock(const uint8_t in[16], uint8_t out[16], const AESKeySchedule& ks);
}
