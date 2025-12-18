#pragma once

#include <cstdint>
#include <vector>

#include <player.hpp>
#include <consts.hpp>
#include <actions.hpp>
#include <packedBank.hpp>

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
// Adjacent Edges:
// - Edge 1: bits 17-23 (7 bits, max 127)
// - Edge 2: bits 24-30 (7 bits, max 127)
// - Edge 3: bits 31-37 (7 bits, max 127)
// - Edge 4: bits 38-44 (7 bits, max 127)

using PackedEdge = uint64_t;

// Road packing/unpacking (bit 0)
constexpr PackedEdge packHasRoad(PackedEdge e, bool hasRoad) {
    return (e & ~1ULL) | uint64_t(hasRoad);
}
constexpr bool unpackHasRoad(PackedEdge e) {
    return e & 1;
}

// Owner packing/unpacking (bits 1-2)
constexpr PackedEdge packOwner(PackedEdge e, PlayerId owner) {
    return (e & ~(0x3ULL << 1)) | (uint64_t(static_cast<uint8_t>(owner) & 0x3) << 1);
}
constexpr PlayerId unpackOwner(PackedEdge e) {
    return static_cast<PlayerId>((e >> 1) & 0x3);
}

// Adjacent node packing/unpacking (bits 3-16, 7 bits each)
constexpr PackedEdge packAdjacentNode(PackedEdge e, uint8_t nodeIndex, NodeId nodeId) {
    uint8_t shift = 3 + (nodeIndex * 7);
    e &= ~(0x7FULL << shift);
    e |= (uint64_t(nodeId & 0x7F) << shift);
    return e;
}
constexpr NodeId unpackAdjacentNode(PackedEdge e, uint8_t nodeIndex) {
    uint8_t shift = 3 + (nodeIndex * 7);
    return (e >> shift) & 0x7F;
}

// Adjacent edge packing/unpacking (bits 17-44, 7 bits each)
constexpr PackedEdge packAdjacentEdge(PackedEdge e, uint8_t edgeIndex, EdgeId edgeId) {
    uint8_t shift = 17 + (edgeIndex * 7);
    e &= ~(0x7FULL << shift);
    e |= (uint64_t(edgeId & 0x7F) << shift);
    return e;
}
constexpr EdgeId unpackAdjacentEdge(PackedEdge e, uint8_t edgeIndex) {
    uint8_t shift = 17 + (edgeIndex * 7);
    return (e >> shift) & 0x7F;
}

// Convenience function to create a new edge
constexpr PackedEdge makeEdge(NodeId node1, NodeId node2,
                            EdgeId edge1, EdgeId edge2,
                            EdgeId edge3, EdgeId edge4,
                            bool hasRoad = false, PlayerId owner = PlayerId::NoPlayer) {
    PackedEdge e = 0;
    e = packHasRoad(e, hasRoad);
    e = packOwner(e, owner);
    e = packAdjacentNode(e, 0, node1);
    e = packAdjacentNode(e, 1, node2);
    e = packAdjacentEdge(e, 0, edge1);
    e = packAdjacentEdge(e, 1, edge2);
    e = packAdjacentEdge(e, 2, edge3);
    e = packAdjacentEdge(e, 3, edge4);
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
    Bank::PackedBank packedBank = Bank::makeNewBank();

    PlayerId currentPlayer = PlayerId::Player1;
    uint8_t currentTurn = 0;

    Action::PackedAction actionQueue[512] = {};
    uint16_t actionQueueSize = 0;

    constexpr BoardState() noexcept;
    void generateRandomBoard();
    void applyAction(Action::PackedAction action);
    void undoLastAction();

    void handlePlaceInitialSettlement(Action::PackedAction action, PlayerId playerId);
    void handlePlace2InitialSettlement(Action::PackedAction action, PlayerId playerId);
    void handlePlaceInitialRoad(Action::PackedAction action, PlayerId playerId);
    Action::PackedAction handleEndTurn();
    Action::PackedAction handleRollDice(Action::PackedAction action);
    Action::PackedAction handleMoveRobber(Action::PackedAction action, PlayerId playerId);
    void handleDiscardResources(Action::PackedAction action, PlayerId playerId);
    void handleBuildRoad(Action::PackedAction action, PlayerId playerId);
    void handleBuildSettlement(Action::PackedAction action, PlayerId playerId);
    void handleBuildCity(Action::PackedAction action, PlayerId playerId);
    Action::PackedAction handleBuyDevCard(PlayerId playerId);
    Action::PackedAction handlePlayDevCardKnight(Action::PackedAction action, PlayerId playerId);
    void handlePlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId);
    void handlePlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId);
    Action::PackedAction handlePlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId);
    void handleStealResource(Action::PackedAction action, PlayerId playerId);
    void handleTradeBank(Action::PackedAction action, PlayerId playerId);
    void handleReceiveResources(Action::PackedAction action, PlayerId playerId);

    void handleUndoEndTurn();
    void handleUndoRollDice(Action::PackedAction action);
    void handleUndoMoveRobber(Action::PackedAction action, PlayerId playerId);
    void handleUndoStealResource(Action::PackedAction action, PlayerId playerId);
    void handleUndoDiscardResources(Action::PackedAction action, PlayerId playerId);
    void handleUndoBuildRoad(Action::PackedAction action, PlayerId playerId);
    void handleUndoBuildSettlement(Action::PackedAction action, PlayerId playerId);
    void handleUndoBuildCity(Action::PackedAction action, PlayerId playerId);
    void handleUndoBuyDevCard(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlayDevCardKnight(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId);
    void handleUndoTradeBank(Action::PackedAction action, PlayerId playerId);
    void handleUndoReceiveResources(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlaceInitialSettlement(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlace2InitialSettlement(Action::PackedAction action, PlayerId playerId);
    void handleUndoPlaceInitialRoad(Action::PackedAction action, PlayerId playerId);

    std::vector<Action::PackedAction> getLegalActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuildActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuildRoadActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBankTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateTwoToOnePortTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateThreeToOnePortTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateDevCardActions(PlayerId playerId);
};

constexpr BoardState::BoardState() noexcept {
    nodes[0]  = Node::makeNode(HexIdNone, 0, HexIdNone);
    nodes[0] = Node::packPortType(nodes[0], PortType::NoPort);
    nodes[1]  = Node::makeNode(HexIdNone, HexIdNone, 0);
    nodes[1] = Node::packPortType(nodes[1], PortType::NoPort);
    nodes[2]  = Node::makeNode(0, 1, HexIdNone);
    nodes[2] = Node::packPortType(nodes[2], PortType::ThreeForOne);
    nodes[3]  = Node::makeNode(HexIdNone, HexIdNone, 1);
    nodes[3] = Node::packPortType(nodes[3], PortType::ThreeForOne);
    nodes[4]  = Node::makeNode(1, 2, HexIdNone);
    nodes[4] = Node::packPortType(nodes[4], PortType::NoPort);
    nodes[5]  = Node::makeNode(HexIdNone, HexIdNone, 2);
    nodes[5] = Node::packPortType(nodes[5], PortType::ThreeForOne);
    nodes[6]  = Node::makeNode(2, HexIdNone, HexIdNone);
    nodes[6] = Node::packPortType(nodes[6], PortType::ThreeForOne);
    nodes[7]  = Node::makeNode(HexIdNone, 3, HexIdNone);
    nodes[7] = Node::packPortType(nodes[7], PortType::WoolPort);
    nodes[8]  = Node::makeNode(HexIdNone, 0, 3);
    nodes[8] = Node::packPortType(nodes[8], PortType::WoolPort);
    nodes[9]  = Node::makeNode(3, 4, 0);
    nodes[9] = Node::packPortType(nodes[9], PortType::NoPort);
    nodes[10] = Node::makeNode(0, 1, 4);
    nodes[10] = Node::packPortType(nodes[10], PortType::NoPort);
    nodes[11] = Node::makeNode(4, 5, 1);
    nodes[11] = Node::packPortType(nodes[11], PortType::NoPort);
    nodes[12] = Node::makeNode(1, 2, 5);
    nodes[12] = Node::packPortType(nodes[12], PortType::NoPort);
    nodes[13] = Node::makeNode(5, 6, 2);
    nodes[13] = Node::packPortType(nodes[13], PortType::NoPort);
    nodes[14] = Node::makeNode(2, HexIdNone, 6);
    nodes[14] = Node::packPortType(nodes[14], PortType::NoPort);
    nodes[15] = Node::makeNode(6, HexIdNone, HexIdNone);
    nodes[15] = Node::packPortType(nodes[15], PortType::BrickPort);
    nodes[16] = Node::makeNode(HexIdNone, 7, HexIdNone);
    nodes[16] = Node::packPortType(nodes[16], PortType::ThreeForOne);
    nodes[17] = Node::makeNode(HexIdNone, 3, 7);
    nodes[17] = Node::packPortType(nodes[17], PortType::NoPort);
    nodes[18] = Node::makeNode(7, 8, 3);
    nodes[18] = Node::packPortType(nodes[18], PortType::NoPort);
    nodes[19] = Node::makeNode(3, 4, 8);
    nodes[19] = Node::packPortType(nodes[19], PortType::NoPort);
    nodes[20] = Node::makeNode(8, 9, 4);
    nodes[20] = Node::packPortType(nodes[20], PortType::NoPort);
    nodes[21] = Node::makeNode(4, 5, 9);
    nodes[21] = Node::packPortType(nodes[21], PortType::NoPort);
    nodes[22] = Node::makeNode(9,  10, 5);
    nodes[22] = Node::packPortType(nodes[22], PortType::NoPort);
    nodes[23] = Node::makeNode(5, 6,  10);
    nodes[23] = Node::packPortType(nodes[23], PortType::NoPort);
    nodes[24] = Node::makeNode(10, 11, 6);
    nodes[24] = Node::packPortType(nodes[24], PortType::NoPort);
    nodes[25] = Node::makeNode(6, HexIdNone,  11);
    nodes[25] = Node::packPortType(nodes[25], PortType::BrickPort);
    nodes[26] = Node::makeNode(11,  HexIdNone, HexIdNone);
    nodes[26] = Node::packPortType(nodes[26], PortType::NoPort);
    nodes[27] = Node::makeNode(HexIdNone, 7, HexIdNone);
    nodes[27] = Node::packPortType(nodes[27], PortType::ThreeForOne);
    nodes[28] = Node::makeNode(HexIdNone,  12, 7);
    nodes[28] = Node::packPortType(nodes[28], PortType::NoPort);
    nodes[29] = Node::makeNode(7, 8,  12);
    nodes[29] = Node::packPortType(nodes[29], PortType::NoPort);
    nodes[30] = Node::makeNode(12, 13, 8);
    nodes[30] = Node::packPortType(nodes[30], PortType::NoPort);
    nodes[31] = Node::makeNode(8, 9,  13);
    nodes[31] = Node::packPortType(nodes[31], PortType::NoPort);
    nodes[32] = Node::makeNode(13, 14, 9);
    nodes[32] = Node::packPortType(nodes[32], PortType::NoPort);
    nodes[33] = Node::makeNode(9,  10,  14);
    nodes[33] = Node::packPortType(nodes[33], PortType::NoPort);
    nodes[34] = Node::makeNode(14, 15,  10);
    nodes[34] = Node::packPortType(nodes[34], PortType::NoPort);
    nodes[35] = Node::makeNode(10, 11,  15);
    nodes[35] = Node::packPortType(nodes[35], PortType::NoPort);
    nodes[36] = Node::makeNode(15,  HexIdNone,  11);
    nodes[36] = Node::packPortType(nodes[36], PortType::LumberPort);
    nodes[37] = Node::makeNode(11,  HexIdNone, HexIdNone);
    nodes[37] = Node::packPortType(nodes[37], PortType::NoPort);
    nodes[38] = Node::makeNode(HexIdNone,  12, HexIdNone);
    nodes[38] = Node::packPortType(nodes[38], PortType::OrePort);
    nodes[39] = Node::makeNode(HexIdNone,  16,  12);
    nodes[39] = Node::packPortType(nodes[39], PortType::OrePort);
    nodes[40] = Node::makeNode(12, 13,  16);
    nodes[40] = Node::packPortType(nodes[40], PortType::NoPort);
    nodes[41] = Node::makeNode(16, 17,  13);
    nodes[41] = Node::packPortType(nodes[41], PortType::NoPort);
    nodes[42] = Node::makeNode(13, 14,  17);
    nodes[42] = Node::packPortType(nodes[42], PortType::NoPort);
    nodes[43] = Node::makeNode(17, 18,  14);
    nodes[43] = Node::packPortType(nodes[43], PortType::NoPort);
    nodes[44] = Node::makeNode(14, 15,  18);
    nodes[44] = Node::packPortType(nodes[44], PortType::NoPort);
    nodes[45] = Node::makeNode(18,  HexIdNone,  15);
    nodes[45] = Node::packPortType(nodes[45], PortType::NoPort);
    nodes[46] = Node::makeNode(15,  HexIdNone, HexIdNone);
    nodes[46] = Node::packPortType(nodes[46], PortType::LumberPort);
    nodes[47] = Node::makeNode(HexIdNone,  16, HexIdNone);
    nodes[47] = Node::packPortType(nodes[47], PortType::NoPort);
    nodes[48] = Node::makeNode(HexIdNone, HexIdNone,  16);
    nodes[48] = Node::packPortType(nodes[48], PortType::NoPort);
    nodes[49] = Node::makeNode(16, 17, HexIdNone);
    nodes[49] = Node::packPortType(nodes[49], PortType::GrainPort);
    nodes[50] = Node::makeNode(HexIdNone, HexIdNone,  17);
    nodes[50] = Node::packPortType(nodes[50], PortType::GrainPort);
    nodes[51] = Node::makeNode(17, 18, HexIdNone);
    nodes[51] = Node::packPortType(nodes[51], PortType::NoPort);
    nodes[52] = Node::makeNode(HexIdNone, HexIdNone,  18);
    nodes[52] = Node::packPortType(nodes[52], PortType::ThreeForOne);
    nodes[53] = Node::makeNode(18,  HexIdNone, HexIdNone);
    nodes[53] = Node::packPortType(nodes[53], PortType::ThreeForOne);

    // ------- EDGES -------
    edges[0]  = Edge::makeEdge(0, 1, 6, 1, EdgeIdNone, EdgeIdNone);
    edges[1]  = Edge::makeEdge(1, 2, 0, 7, 2, EdgeIdNone);
    edges[2]  = Edge::makeEdge(2, 3, 1, 7, 3, EdgeIdNone);
    edges[3]  = Edge::makeEdge(3, 4, 2, 8, 4, EdgeIdNone);
    edges[4]  = Edge::makeEdge(4, 5, 3, 8, 5, EdgeIdNone);
    edges[5]  = Edge::makeEdge(5, 6, 4, 9, EdgeIdNone, EdgeIdNone);
    edges[6]  = Edge::makeEdge(0, 8, 0, 10, 11, EdgeIdNone);
    edges[7]  = Edge::makeEdge(2, 10, 1, 2, 12, 13);
    edges[8]  = Edge::makeEdge(4, 12, 3, 4, 14, 15);
    edges[9]  = Edge::makeEdge(6, 14, 5, 16, 17, EdgeIdNone);
    edges[10] = Edge::makeEdge(7, 8, 6, 11, 18, EdgeIdNone);
    edges[11] = Edge::makeEdge(8, 9, 6, 10, 12, 19);
    edges[12] = Edge::makeEdge(9, 10, 7, 13, 11, 19);
    edges[13] = Edge::makeEdge(10, 11, 7, 12, 14, 20);
    edges[14] = Edge::makeEdge(11, 12, 8, 15, 13, 20);
    edges[15] = Edge::makeEdge(12, 13, 8, 14, 16, 21);
    edges[16] = Edge::makeEdge(13, 14, 9, 17, 15, 21);
    edges[17] = Edge::makeEdge(14, 15, 9, 16, 22, EdgeIdNone);
    edges[18] = Edge::makeEdge(7, 17, 10, 23, 24, EdgeIdNone);
    edges[19] = Edge::makeEdge(9, 19, 11, 12, 25, 26);
    edges[20] = Edge::makeEdge(11, 21, 13, 14, 27, 28);
    edges[21] = Edge::makeEdge(13, 23, 15, 16, 29, 30);
    edges[22] = Edge::makeEdge(15, 25, 17, 31, 32, EdgeIdNone);
    edges[23] = Edge::makeEdge(16, 17, 18, 24, 33, EdgeIdNone);
    edges[24] = Edge::makeEdge(17, 18, 18, 23, 25, 34);
    edges[25] = Edge::makeEdge(18, 19, 19, 26, 24, 34);
    edges[26] = Edge::makeEdge(19, 20, 19, 25, 27, 35);
    edges[27] = Edge::makeEdge(20, 21, 20, 28, 26, 35);
    edges[28] = Edge::makeEdge(21, 22, 20, 27, 29, 36);
    edges[29] = Edge::makeEdge(29, 23, 21, 30, 28, 36);
    edges[30] = Edge::makeEdge(23, 24, 21, 29, 31, 37);
    edges[31] = Edge::makeEdge(24, 25, 22, 32, 30, 37);
    edges[32] = Edge::makeEdge(25, 26, 22, 31, 38, EdgeIdNone);
    edges[33] = Edge::makeEdge(16, 27, 23, 39, EdgeIdNone, EdgeIdNone);
    edges[34] = Edge::makeEdge(18, 29, 24, 25, 40, 41);
    edges[35] = Edge::makeEdge(20, 31, 26, 27, 42, 43);
    edges[36] = Edge::makeEdge(22, 33, 28, 29, 44, 45);
    edges[37] = Edge::makeEdge(24, 35, 30, 31, 46, 47);
    edges[38] = Edge::makeEdge(26, 37, 32, 48, EdgeIdNone, EdgeIdNone);
    edges[39] = Edge::makeEdge(27, 28, 33, 40, 49, EdgeIdNone);
    edges[40] = Edge::makeEdge(28, 29, 34, 41, 39, 49);
    edges[41] = Edge::makeEdge(29, 30, 34, 40, 42, 50);
    edges[42] = Edge::makeEdge(30, 31, 35, 43, 41, 50);
    edges[43] = Edge::makeEdge(31, 32, 35, 42, 44, 51);
    edges[44] = Edge::makeEdge(32, 33, 36, 45, 43, 51);
    edges[45] = Edge::makeEdge(33, 34, 36, 44, 46, 52);
    edges[46] = Edge::makeEdge(34, 35, 37, 47, 45, 52);
    edges[47] = Edge::makeEdge(35, 36, 37, 46, 48, 53);
    edges[48] = Edge::makeEdge(36, 37, 38, EdgeIdNone, 47, 53);
    edges[49] = Edge::makeEdge(28, 38, 39, 40, 54, EdgeIdNone);
    edges[50] = Edge::makeEdge(30, 40, 41, 42, 55, 56);
    edges[51] = Edge::makeEdge(32, 42, 43, 44, 57, 58);
    edges[52] = Edge::makeEdge(34, 44, 45, 46, 59, 60);
    edges[53] = Edge::makeEdge(36, 46, 47, 48, 61, EdgeIdNone);
    edges[54] = Edge::makeEdge(38, 39, 49, 55, 62, EdgeIdNone);
    edges[55] = Edge::makeEdge(39, 40, 50, 56, 54, 62);
    edges[56] = Edge::makeEdge(40, 41, 50, 55, 57, 63);
    edges[57] = Edge::makeEdge(41, 42, 51, 58, 56, 63);
    edges[58] = Edge::makeEdge(42, 43, 51, 57, 59, 64);
    edges[59] = Edge::makeEdge(43, 44, 52, 60, 58, 64);
    edges[60] = Edge::makeEdge(44, 45, 52, 59, 61, 65);
    edges[61] = Edge::makeEdge(45, 46, 53, EdgeIdNone, 60, 65);
    edges[62] = Edge::makeEdge(39, 47, 54, 55, 66, EdgeIdNone);
    edges[63] = Edge::makeEdge(41, 49, 56, 57, 67, 68);
    edges[64] = Edge::makeEdge(43, 51, 58, 59, 69, 70);
    edges[65] = Edge::makeEdge(45, 53, 60, 61, 71, EdgeIdNone);
    edges[66] = Edge::makeEdge(47, 48, 62, 67, EdgeIdNone, EdgeIdNone);
    edges[67] = Edge::makeEdge(48, 49, 63, 68, 66, EdgeIdNone);
    edges[68] = Edge::makeEdge(49, 50, 63, 67, 69, EdgeIdNone);
    edges[69] = Edge::makeEdge(50, 51, 64, 70, 68, EdgeIdNone);
    edges[70] = Edge::makeEdge(51, 52, 64, 69, 71, EdgeIdNone);
    edges[71] = Edge::makeEdge(52, 53, 65, EdgeIdNone, 70, EdgeIdNone);
}

} // namespace Board