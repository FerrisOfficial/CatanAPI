#pragma once

#include <random>

namespace RandomDevice {

inline std::mt19937& get_rng() {
    static thread_local std::mt19937 rng((std::random_device())());
    return rng;
}

// RAII helper: saves/restores RNG state for simulations.
// Useful when you need to use RandomDevice during search/rollouts without
// perturbing the real game's randomness.
struct ScopedState {
    std::mt19937 saved;
    ScopedState() : saved(get_rng()) {}
    ScopedState(const ScopedState&) = delete;
    ScopedState& operator=(const ScopedState&) = delete;
    ~ScopedState() { get_rng() = saved; }
};

// Returns a uniformly distributed integer in [0, maxExclusive).
inline uint32_t uniform_u32(uint32_t maxExclusive) {
    if (maxExclusive == 0) {
        return 0;
    }
    std::uniform_int_distribution<uint32_t> dist(0, maxExclusive - 1);
    return dist(get_rng());
}

// Returns a uniformly distributed integer in [minInclusive, maxInclusive].
inline uint32_t uniform_u32_range(uint32_t minInclusive, uint32_t maxInclusive) {
    std::uniform_int_distribution<uint32_t> dist(minInclusive, maxInclusive);
    return dist(get_rng());
}

inline int rollDices() {
    return uniform_u32_range(1, 6) + uniform_u32_range(1, 6);
}

} // namespace RandomDevice
