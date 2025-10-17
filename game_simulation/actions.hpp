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

} // namespace Action
