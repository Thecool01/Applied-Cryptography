#pragma once
#include <cstdint>
#include <chrono>

#ifdef _WIN32
#include <process.h>
#define GET_PID _getpid
#else
#include <unistd.h>
#define GET_PID getpid
#endif

class CustomRNG {
private:
    uint64_t state = 0;

    static inline uint64_t rotl(uint64_t x, int k) {
        k &= 63;
        if (k == 0) return x;
        return (x << k) | (x >> (64 - k));
    }

    static constexpr uint64_t A = 6364136223846793005ULL;
    static constexpr uint64_t C = 1442695040888963407ULL;

public:
    CustomRNG() { reseed(); }

    void reseed() {
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t time_entropy = (uint64_t)now.time_since_epoch().count();
        uint64_t pid_entropy  = (uint64_t)GET_PID();
        uint64_t mem_entropy  = (uint64_t)(uintptr_t)this;

        state = time_entropy ^ rotl(pid_entropy, 17) ^ rotl(mem_entropy, 31);
        for (int i = 0; i < 10; i++) (void)nextByte();
    }

    uint8_t nextByte() {
        state = state * A + C;
        return (uint8_t)(state >> 56);
    }

    void getBytes(uint8_t* buffer, size_t size) {
        for (size_t i = 0; i < size; ++i) buffer[i] = nextByte();
    }
};

