#pragma once

#include <cstdint>
#include <consts.hpp>

// Packed 64-bit action representation
// Layout (from LSB to MSB):
//
// Type:                bits 0-5   (6 bits, max 34)
//
// Player ID:           bits 6-7   (2 bits, max 3)
//
// Resources (5 bits each, 25 bits total):
// - Brick:  bits 8-12  (5 bits, max 31)
// - Lumber: bits 13-17  (5 bits, max 31)
// - Wool:   bits 18-22  (5 bits, max 31)
// - Grain:  bits 23-27  (5 bits, max 31)
// - Ore:    bits 28-32  (5 bits, max 31)
//
// Arguments:
// - Arg1:   bits 33-40 (8 bits, max 255)
// - Arg2:   bits 41-48 (8 bits, max 255)
// - Arg3:   bits 49-56 (8 bits, max 255)

namespace Action {

using PackedAction = uint64_t;

// Action type packing/unpacking (bits 0-5)
constexpr PackedAction packType(PackedAction a, ActionType type) {
    return (a & ~0x3FULL) | (uint64_t(static_cast<uint8_t>(type) & 0x3F));
}
constexpr ActionType unpackType(PackedAction a) {
    return static_cast<ActionType>(a & 0x3F);
}

// Player ID packing/unpacking (bits 6-7)
constexpr PackedAction packPlayerID(PackedAction a, PlayerId playerID) {
    return (a & ~(0x3ULL << 6)) | (uint64_t(static_cast<uint8_t>(playerID) & 0x3) << 6);
}
constexpr PlayerId unpackPlayerID(PackedAction a) {
    return static_cast<PlayerId>((a >> 6) & 0x3);
}

// Resource packing/unpacking (bits 6-30, 5 bits each)
constexpr PackedAction packResource(PackedAction a, Resource r, uint8_t value) {
    uint8_t shift = 8 + (static_cast<uint8_t>(r) * 5);  // Resources start at bit 8
    a &= ~(0x1FULL << shift);
    a |= (uint64_t(value & 0x1F) << shift);
    return a;
}
constexpr uint8_t unpackResource(PackedAction a, Resource r) {
    uint8_t shift = 8 + (static_cast<uint8_t>(r) * 5);  // Resources start at bit 8
    return static_cast<uint8_t>((a >> shift) & 0x1F);
}

// Argument packing/unpacking
constexpr PackedAction packArg1(PackedAction a, uint8_t arg1) {
    return (a & ~(0xFFULL << 33)) | (uint64_t(arg1) << 33);
}
constexpr uint8_t unpackArg1(PackedAction a) {
    return static_cast<uint8_t>((a >> 33) & 0xFF);
}

constexpr PackedAction packArg2(PackedAction a, uint8_t arg2) {
    return (a & ~(0xFFULL << 41)) | (uint64_t(arg2) << 41);
}
constexpr uint8_t unpackArg2(PackedAction a) {
    return static_cast<uint8_t>((a >> 41) & 0xFF);
}

constexpr PackedAction packArg3(PackedAction a, uint8_t arg3) {
    return (a & ~(0xFFULL << 49)) | (uint64_t(arg3) << 49);
}
constexpr uint8_t unpackArg3(PackedAction a) {
    return static_cast<uint8_t>((a >> 49) & 0xFF);
}

} // namespace Action
