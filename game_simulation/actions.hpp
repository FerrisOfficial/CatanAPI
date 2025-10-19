#pragma once

#include <cstdint>
#include <consts.hpp>

// Packed 64-bit action representation
// Layout (from LSB to MSB):
//
// Type:                bits 0-3   (4 bits, max 15)
//
// Player ID:           bits 4-5   (2 bits, max 3)
//
// Resources (5 bits each, 25 bits total):
// - Brick:  bits 6-10  (5 bits, max 31)
// - Lumber: bits 11-15  (5 bits, max 31)
// - Wool:   bits 16-20  (5 bits, max 31)
// - Grain:  bits 21-25  (5 bits, max 31)
// - Ore:    bits 26-30  (5 bits, max 31)
//
// Arguments:
// - Arg1:   bits 31-38 (8 bits, max 255)
// - Arg2:   bits 39-46 (8 bits, max 255)

namespace Action {

using PackedAction = uint64_t;

// Action type packing/unpacking (bits 0-3)
constexpr PackedAction packType(PackedAction a, ActionType type) {
    return (a & ~0xFULL) | (uint64_t(static_cast<uint8_t>(type) & 0xF));
}
constexpr ActionType unpackType(PackedAction a) {
    return static_cast<ActionType>(a & 0xF);
}

// Player ID packing/unpacking (bits 4-5)
constexpr PackedAction packPlayerID(PackedAction a, PlayerId playerID) {
    return (a & ~(0x3ULL << 4)) | (uint64_t(static_cast<uint8_t>(playerID) & 0x3) << 4);
}
constexpr PlayerId unpackPlayerID(PackedAction a) {
    return static_cast<PlayerId>((a >> 4) & 0x3);
}

// Resource packing/unpacking (bits 6-30, 5 bits each)
constexpr PackedAction packResource(PackedAction a, Resource r, uint8_t value) {
    uint8_t shift = 6 + (r * 5);  // Resources start at bit 6
    a &= ~(0x1FULL << shift);
    a |= (uint64_t(value & 0x1F) << shift);
    return a;
}
constexpr uint8_t unpackResource(PackedAction a, Resource r) {
    uint8_t shift = 6 + (r * 5);  // Resources start at bit 6
    return (a >> shift) & 0x1F;
}

// Argument packing/unpacking
constexpr PackedAction packArg1(PackedAction a, uint8_t arg1) {
    return (a & ~(0xFFULL << 31)) | (uint64_t(arg1) << 31);
}
constexpr uint8_t unpackArg1(PackedAction a) {
    return (a >> 31) & 0xFF;
}

constexpr PackedAction packArg2(PackedAction a, uint8_t arg2) {
    return (a & ~(0xFFULL << 39)) | (uint64_t(arg2) << 39);
}
constexpr uint8_t unpackArg2(PackedAction a) {
    return (a >> 39) & 0xFF;
}

} // namespace Action
