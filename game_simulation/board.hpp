#pragma once

#include <cstdint>

#include <player.hpp>
#include <consts.hpp>
#include <actions.hpp>

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

// Full board state structure (exposed so tests can access members)
struct BoardState {
    HexId robberPosition = 0;
    Hex::PackedHex hexes[HEX_COUNT] = {0};
    Node::PackedNode nodes[NODE_COUNT] = {0};
    Edge::PackedEdge edges[EDGE_COUNT] = {0};

    Player::PackedPlayer packedPlayers[2] = {Player::makeNewPlayer(), Player::makeNewPlayer()};

    PlayerId currentPlayer = PlayerId::Player1;
    uint8_t currentTurn = 0;

    ActionType actionQueue[512] = {};
    uint8_t actionQueueSize = 0;

    constexpr BoardState() noexcept;
    void applyAction(Action::PackedAction action);
};

constexpr BoardState::BoardState() noexcept {
    nodes[0]  = Node::makeNode(HexIdNone, 0, HexIdNone);
    nodes[1]  = Node::makeNode(HexIdNone, HexIdNone, 0);
    nodes[2]  = Node::makeNode(0, 1, HexIdNone);
    nodes[3]  = Node::makeNode(HexIdNone, HexIdNone, 1);
    nodes[4]  = Node::makeNode(1, 2, HexIdNone);
    nodes[5]  = Node::makeNode(HexIdNone, HexIdNone, 2);
    nodes[6]  = Node::makeNode(2, HexIdNone, HexIdNone);
    nodes[7]  = Node::makeNode(HexIdNone, 3, HexIdNone);
    nodes[8]  = Node::makeNode(HexIdNone, 0, 3);
    nodes[9]  = Node::makeNode(3, 4, 0);
    nodes[10] = Node::makeNode(0, 1, 4);
    nodes[11] = Node::makeNode(4, 5, 1);
    nodes[12] = Node::makeNode(1, 2, 5);
    nodes[13] = Node::makeNode(5, 6, 2);
    nodes[14] = Node::makeNode(2, HexIdNone, 6);
    nodes[15] = Node::makeNode(6, HexIdNone, HexIdNone);
    nodes[16] = Node::makeNode(HexIdNone, 7, HexIdNone);
    nodes[17] = Node::makeNode(HexIdNone, 3, 7);
    nodes[18] = Node::makeNode(7, 8, 3);
    nodes[19] = Node::makeNode(3, 4, 8);
    nodes[20] = Node::makeNode(8, 9, 4);
    nodes[21] = Node::makeNode(4, 5, 9);
    nodes[22] = Node::makeNode(9,  10, 5);
    nodes[23] = Node::makeNode(5, 6,  10);
    nodes[24] = Node::makeNode(10, 11, 6);
    nodes[25] = Node::makeNode(6, HexIdNone,  11);
    nodes[26] = Node::makeNode(11,  HexIdNone, HexIdNone);
    nodes[27] = Node::makeNode(HexIdNone, 7, HexIdNone);
    nodes[28] = Node::makeNode(HexIdNone,  12, 7);
    nodes[29] = Node::makeNode(7, 8,  12);
    nodes[30] = Node::makeNode(12, 13, 8);
    nodes[31] = Node::makeNode(8, 9,  13);
    nodes[32] = Node::makeNode(13, 14, 9);
    nodes[33] = Node::makeNode(9,  10,  14);
    nodes[34] = Node::makeNode(14, 15,  10);
    nodes[35] = Node::makeNode(10, 11,  15);
    nodes[36] = Node::makeNode(15,  HexIdNone,  11);
    nodes[37] = Node::makeNode(11,  HexIdNone, HexIdNone);
    nodes[38] = Node::makeNode(HexIdNone,  12, HexIdNone);
    nodes[39] = Node::makeNode(HexIdNone,  16,  12);
    nodes[40] = Node::makeNode(12, 13,  16);
    nodes[41] = Node::makeNode(16, 17,  13);
    nodes[42] = Node::makeNode(13, 14,  17);
    nodes[43] = Node::makeNode(17, 18,  14);
    nodes[44] = Node::makeNode(14, 15,  18);
    nodes[45] = Node::makeNode(18,  HexIdNone,  15);
    nodes[46] = Node::makeNode(15,  HexIdNone, HexIdNone);
    nodes[47] = Node::makeNode(HexIdNone,  16, HexIdNone);
    nodes[48] = Node::makeNode(HexIdNone, HexIdNone,  16);
    nodes[49] = Node::makeNode(16, 17, HexIdNone);
    nodes[50] = Node::makeNode(HexIdNone, HexIdNone,  17);
    nodes[51] = Node::makeNode(17, 18, HexIdNone);
    nodes[52] = Node::makeNode(HexIdNone, HexIdNone,  18);
    nodes[53] = Node::makeNode(18,  HexIdNone, HexIdNone);

    // ------- EDGES -------
    edges[0]  = Edge::makeEdge(0, 1);
    edges[1]  = Edge::makeEdge(1, 2);
    edges[2]  = Edge::makeEdge(2, 3);
    edges[3]  = Edge::makeEdge(3, 4);
    edges[4]  = Edge::makeEdge(4, 5);
    edges[5]  = Edge::makeEdge(5, 6);
    edges[6]  = Edge::makeEdge(0, 8);
    edges[7]  = Edge::makeEdge(2, 10);
    edges[8]  = Edge::makeEdge(4, 12);
    edges[9]  = Edge::makeEdge(6, 14);
    edges[10] = Edge::makeEdge(7, 8);
    edges[11] = Edge::makeEdge(8, 9);
    edges[12] = Edge::makeEdge(9, 10);
    edges[13] = Edge::makeEdge(10, 11);
    edges[14] = Edge::makeEdge(11, 12);
    edges[15] = Edge::makeEdge(12, 13);
    edges[16] = Edge::makeEdge(13, 14);
    edges[17] = Edge::makeEdge(14, 15);
    edges[18] = Edge::makeEdge(7, 17);
    edges[19] = Edge::makeEdge(9, 19);
    edges[20] = Edge::makeEdge(11, 21);
    edges[21] = Edge::makeEdge(13, 23);
    edges[22] = Edge::makeEdge(15, 25);
    edges[23] = Edge::makeEdge(16, 17);
    edges[24] = Edge::makeEdge(17, 18);
    edges[25] = Edge::makeEdge(18, 19);
    edges[26] = Edge::makeEdge(19, 20);
    edges[27] = Edge::makeEdge(20, 21);
    edges[28] = Edge::makeEdge(21, 22);
    edges[29] = Edge::makeEdge(29, 23);
    edges[30] = Edge::makeEdge(23, 24);
    edges[31] = Edge::makeEdge(24, 25);
    edges[32] = Edge::makeEdge(25, 26);
    edges[33] = Edge::makeEdge(16, 27);
    edges[34] = Edge::makeEdge(18, 29);
    edges[35] = Edge::makeEdge(20, 31);
    edges[36] = Edge::makeEdge(22, 33);
    edges[37] = Edge::makeEdge(24, 35);
    edges[38] = Edge::makeEdge(26, 37);
    edges[39] = Edge::makeEdge(27, 28);
    edges[40] = Edge::makeEdge(28, 29);
    edges[41] = Edge::makeEdge(29, 30);
    edges[42] = Edge::makeEdge(30, 31);
    edges[43] = Edge::makeEdge(31, 32);
    edges[44] = Edge::makeEdge(32, 33);
    edges[45] = Edge::makeEdge(33, 34);
    edges[46] = Edge::makeEdge(34, 35);
    edges[47] = Edge::makeEdge(35, 36);
    edges[48] = Edge::makeEdge(36, 37);
    edges[49] = Edge::makeEdge(28, 38);
    edges[50] = Edge::makeEdge(30, 40);
    edges[51] = Edge::makeEdge(32, 42);
    edges[52] = Edge::makeEdge(34, 44);
    edges[53] = Edge::makeEdge(36, 46);
    edges[54] = Edge::makeEdge(38, 39);
    edges[55] = Edge::makeEdge(39, 40);
    edges[56] = Edge::makeEdge(40, 41);
    edges[57] = Edge::makeEdge(41, 42);
    edges[58] = Edge::makeEdge(42, 43);
    edges[59] = Edge::makeEdge(43, 44);
    edges[60] = Edge::makeEdge(44, 45);
    edges[61] = Edge::makeEdge(45, 46);
    edges[62] = Edge::makeEdge(39, 47);
    edges[63] = Edge::makeEdge(41, 49);
    edges[64] = Edge::makeEdge(43, 51);
    edges[65] = Edge::makeEdge(45, 53);
    edges[66] = Edge::makeEdge(47, 48);
    edges[67] = Edge::makeEdge(48, 49);
    edges[68] = Edge::makeEdge(49, 50);
    edges[69] = Edge::makeEdge(50, 51);
    edges[70] = Edge::makeEdge(51, 52);
    edges[71] = Edge::makeEdge(52, 53);
}

inline constexpr BoardState CompiledBoard{};
} // namespace Board