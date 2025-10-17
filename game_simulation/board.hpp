#pragma once

#include <cstdint>
#include <consts.hpp>

// Packed board representation

namespace Board {

namespace Hex {

// Packed 16-bit hex representation
//
// Catan Number: bits 0-3   (4 bits, max 15)
// Resource:     bits 4-6   (3 bits, max 7)
// Player1 val:  bits 7-9  (3 bits, max 7)
// Player2 val:  bits 10-12 (3 bits, max 7)

} // namespace Hex

namespace Node {

// Packed 8-bit node representation
//
// Built Structure: bits 0-1 (2 bits, max 3)
// Owner:           bits 1 (1 bits, max 1)
// Adjacent Hexes:
// - Hex 1: bits 2-6 (5 bits, max 19)
// - Hex 2: bits 7-11 (5 bits, max 19)
// - Hex 3: bits 12-16 (5 bits, max 19)
// Port Type:   bits 17-19 (3 bits, max 7)

} // namespace Node

namespace Edge {

// Packed 8-bit edge representation
//
// Built Road: bits 0 (1 bits, max 1)
// Owner:      bits 1 (1 bits, max 1)
// Adjacent Nodes:
// - Node 1: bits 2-8 (7 bits, max 128)
// - Node 2: bits 9-15 (7 bits, max 128)

} // namespace Edge

uint8_t RobberPosition = 0;

} // namespace Board