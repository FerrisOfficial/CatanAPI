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
// Player1 val:  bits 7-9   (3 bits, max 7)
// Player2 val:  bits 10-12 (3 bits, max 7)

using PackedHex = uint16_t;

// Catan number packing/unpacking (bits 0-3)
constexpr PackedHex packCatanNumber(PackedHex h, uint8_t number) {
    return (h & ~0xFU) | (uint16_t(number & 0xF));
}
constexpr uint8_t unpackCatanNumber(PackedHex h) {
    return h & 0xF;
}

// Resource packing/unpacking (bits 4-6)
constexpr PackedHex packResource(PackedHex h, Resource resource) {
    return (h & ~(0x7U << 4)) | (uint16_t(static_cast<uint8_t>(resource) & 0x7) << 4);
}
constexpr Resource unpackResource(PackedHex h) {
    return static_cast<Resource>((h >> 4) & 0x7);
}

// Player values packing/unpacking (bits 7-9 and 10-12)
constexpr PackedHex packPlayerValue(PackedHex h, PlayerId player, uint8_t value) {
    uint8_t shift = 7 + (static_cast<uint8_t>(player) * 3);
    h &= ~(0x7U << shift);
    h |= (uint16_t(value & 0x7) << shift);
    return h;
}
constexpr uint8_t unpackPlayerValue(PackedHex h, PlayerId player) {
    uint8_t shift = 7 + (static_cast<uint8_t>(player) * 3);
    return (h >> shift) & 0x7;
}

// Convenience function to create a new hex
constexpr PackedHex makeHex(uint8_t catanNumber, Resource resource, 
                           uint8_t player0Value = 0, uint8_t player1Value = 0) {
    PackedHex h = 0;
    h = packCatanNumber(h, catanNumber);
    h = packResource(h, resource);
    h = packPlayerValue(h, PlayerId::Player0, player0Value);
    h = packPlayerValue(h, PlayerId::Player1, player1Value);
    return h;
}

} // namespace Hex

namespace Node {

// Packed 32-bit node representation (expanded to fit all fields)
//
// Built Structure: bits 0-1   (2 bits, max 3)
// Owner:           bits 2-3   (2 bits, max 3) 
// Adjacent Hexes:
// - Hex 1: bits 4-8   (5 bits, max 19)
// - Hex 2: bits 9-13  (5 bits, max 19)
// - Hex 3: bits 14-18 (5 bits, max 19)
// Port Type:   bits 19-21 (3 bits, max 7)

using PackedNode = uint32_t;

// Structure packing/unpacking (bits 0-1)
constexpr PackedNode packStructure(PackedNode n, StructureType structure) {
    return (n & ~0x3U) | (uint32_t(static_cast<uint8_t>(structure) & 0x3));
}
constexpr StructureType unpackStructure(PackedNode n) {
    return static_cast<StructureType>(n & 0x3);
}

// Owner packing/unpacking (bits 2-3)
constexpr PackedNode packOwner(PackedNode n, PlayerId owner) {
    return (n & ~(0x3U << 2)) | (uint32_t(static_cast<uint8_t>(owner) & 0x3) << 2);
}
constexpr PlayerId unpackOwner(PackedNode n) {
    return static_cast<PlayerId>((n >> 2) & 0x3);
}

// Adjacent hex packing/unpacking (bits 4-18, 5 bits each)
constexpr PackedNode packAdjacentHex(PackedNode n, uint8_t hexIndex, HexId hexId) {
    uint8_t shift = 4 + (hexIndex * 5);
    n &= ~(0x1FU << shift);
    n |= (uint32_t(hexId & 0x1F) << shift);
    return n;
}
constexpr HexId unpackAdjacentHex(PackedNode n, uint8_t hexIndex) {
    uint8_t shift = 4 + (hexIndex * 5);
    return (n >> shift) & 0x1F;
}

// Port type packing/unpacking (bits 19-21)
constexpr PackedNode packPortType(PackedNode n, PortType portType) {
    return (n & ~(0x7U << 19)) | (uint32_t(static_cast<uint8_t>(portType) & 0x7) << 19);
}
constexpr PortType unpackPortType(PackedNode n) {
    return static_cast<PortType>((n >> 19) & 0x7);
}

// Convenience function to create a new node
constexpr PackedNode makeNode(HexId hex1, HexId hex2, HexId hex3,
                            StructureType structure = StructureType::NoStructure,
                            PlayerId owner = PlayerId::NoPlayer,
                            PortType portType = PortType::NoPort) {
    PackedNode n = 0;
    n = packStructure(n, structure);
    n = packOwner(n, owner);
    n = packAdjacentHex(n, 0, hex1);
    n = packAdjacentHex(n, 1, hex2);
    n = packAdjacentHex(n, 2, hex3);
    n = packPortType(n, portType);
    return n;
}

} // namespace Node

namespace Edge {

// Packed 32-bit edge representation (expanded to fit 7-bit node IDs)
//
// Has Road:     bit 0      (1 bit, boolean)
// Owner:        bits 1-2   (2 bits, max 3)
// Adjacent Nodes:
// - Node 1: bits 3-9   (7 bits, max 127)
// - Node 2: bits 10-16 (7 bits, max 127)

using PackedEdge = uint32_t;

// Road packing/unpacking (bit 0)
constexpr PackedEdge packHasRoad(PackedEdge e, bool hasRoad) {
    return (e & ~1U) | uint32_t(hasRoad);
}
constexpr bool unpackHasRoad(PackedEdge e) {
    return e & 1;
}

// Owner packing/unpacking (bits 1-2)
constexpr PackedEdge packOwner(PackedEdge e, PlayerId owner) {
    return (e & ~(0x3U << 1)) | (uint32_t(static_cast<uint8_t>(owner) & 0x3) << 1);
}
constexpr PlayerId unpackOwner(PackedEdge e) {
    return static_cast<PlayerId>((e >> 1) & 0x3);
}

// Adjacent node packing/unpacking (bits 3-16, 7 bits each)
constexpr PackedEdge packAdjacentNode(PackedEdge e, uint8_t nodeIndex, NodeId nodeId) {
    uint8_t shift = 3 + (nodeIndex * 7);
    e &= ~(0x7FU << shift);
    e |= (uint32_t(nodeId & 0x7F) << shift);
    return e;
}
constexpr NodeId unpackAdjacentNode(PackedEdge e, uint8_t nodeIndex) {
    uint8_t shift = 3 + (nodeIndex * 7);
    return (e >> shift) & 0x7F;
}

// Convenience function to create a new edge
constexpr PackedEdge makeEdge(NodeId node1, NodeId node2,
                             bool hasRoad = false, PlayerId owner = PlayerId::NoPlayer) {
    PackedEdge e = 0;
    e = packHasRoad(e, hasRoad);
    e = packOwner(e, owner);
    e = packAdjacentNode(e, 0, node1);
    e = packAdjacentNode(e, 1, node2);
    return e;
}

} // namespace Edge

// Forward declaration of BoardState
struct BoardState;

} // namespace Board