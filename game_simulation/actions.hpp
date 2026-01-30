#pragma once

#include <cstdint>
#include <cassert>
#include <consts.hpp>

// Packed 64-bit action representation
// Layout (from LSB to MSB):
//
// Type:                bits 0-4   (5 bits, max 18)
//
// Player ID:           bits 5-6   (2 bits, max 3)
//
// Resources (5 bits each, 25 bits total):
// - Brick:  bits 7-11  (5 bits, max 31)
// - Lumber: bits 12-16  (5 bits, max 31)
// - Wool:   bits 17-21  (5 bits, max 31)
// - Grain:  bits 22-26  (5 bits, max 31)
// - Ore:    bits 27-31  (5 bits, max 31)
//
// Arguments:
// - Arg1:   bits 32-39 (8 bits, max 255)
// - Arg2:   bits 40-47 (8 bits, max 255)
// - Arg3:   bits 48-55 (8 bits, max 255)

namespace Action {

using PackedAction = uint64_t;

constexpr PackedAction getEmptyAction() {return 0;}

constexpr bool isValidPackedResource(Resource r) {
    return static_cast<uint8_t>(r) <= static_cast<uint8_t>(Resource::Ore);
}

// Action type packing/unpacking (bits 0-4)
constexpr PackedAction packType(PackedAction a, ActionType type) {
    return (a & ~0x1FULL) | (uint64_t(static_cast<uint8_t>(type) & 0x1F));
}
constexpr ActionType unpackType(PackedAction a) {
    return static_cast<ActionType>(a & 0x1F);
}

// Player ID packing/unpacking (bits 5-6)
constexpr PackedAction packPlayerID(PackedAction a, PlayerId playerID) {
    return (a & ~(0x3ULL << 5)) | (uint64_t(static_cast<uint8_t>(playerID) & 0x3) << 5);
}
constexpr PlayerId unpackPlayerID(PackedAction a) {
    return static_cast<PlayerId>((a >> 5) & 0x3);
}

// Resource packing/unpacking (bits 7-31, 5 bits each)
constexpr PackedAction packResource(PackedAction a, Resource r, uint8_t value) {
#ifndef NDEBUG
    if (!isValidPackedResource(r)) {
        assert(false && "Action::packResource called with invalid Resource");
        return a;
    }
#endif
    if (!isValidPackedResource(r)) {
        return a;
    }
    uint8_t shift = 7 + (static_cast<uint8_t>(r) * 5);  // Resources start at bit 7
    a &= ~(0x1FULL << shift);
    a |= (uint64_t(value & 0x1F) << shift);
    return a;
}
constexpr uint8_t unpackResource(PackedAction a, Resource r) {
#ifndef NDEBUG
    if (!isValidPackedResource(r)) {
        assert(false && "Action::unpackResource called with invalid Resource");
        return 0;
    }
#endif
    if (!isValidPackedResource(r)) {
        return 0;
    }
    uint8_t shift = 7 + (static_cast<uint8_t>(r) * 5);  // Resources start at bit 7
    return static_cast<uint8_t>((a >> shift) & 0x1F);
}

// Argument packing/unpacking
constexpr PackedAction packArg1(PackedAction a, uint8_t arg1) {
    return (a & ~(0xFFULL << 32)) | (uint64_t(arg1) << 32);
}
constexpr uint8_t unpackArg1(PackedAction a) {
    return static_cast<uint8_t>((a >> 32) & 0xFF);
}

constexpr PackedAction packArg2(PackedAction a, uint8_t arg2) {
    return (a & ~(0xFFULL << 40)) | (uint64_t(arg2) << 40);
}
constexpr uint8_t unpackArg2(PackedAction a) {
    return static_cast<uint8_t>((a >> 40) & 0xFF);
}

constexpr PackedAction packArg3(PackedAction a, uint8_t arg3) {
    return (a & ~(0xFFULL << 48)) | (uint64_t(arg3) << 48);
}
constexpr uint8_t unpackArg3(PackedAction a) {
    return static_cast<uint8_t>((a >> 48) & 0xFF);
}

} // namespace Action
