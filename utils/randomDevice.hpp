#pragma once

#include <random>

namespace RandomDevice {

// Returns a uniformly distributed integer in [0, maxExclusive).
inline uint32_t uniform_u32(uint32_t maxExclusive) {
    static thread_local std::mt19937 rng((std::random_device())());
    if (maxExclusive == 0) {
        return 0;
    }
    std::uniform_int_distribution<uint32_t> dist(0, maxExclusive - 1);
    return dist(rng);
}

// Returns a uniformly distributed integer in [minInclusive, maxInclusive].
inline uint32_t uniform_u32_range(uint32_t minInclusive, uint32_t maxInclusive) {
    static thread_local std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<uint32_t> dist(minInclusive, maxInclusive);
    return dist(rng);
}

inline std::mt19937& get_rng() {
    static thread_local std::mt19937 rng((std::random_device())());
    return rng;
}

inline int rollDices() {
    return uniform_u32_range(1, 6) + uniform_u32_range(1, 6);
}

} // namespace RandomDevice
