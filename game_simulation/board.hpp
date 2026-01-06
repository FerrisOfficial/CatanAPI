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

// Packed 64-bit node representation (expanded to fit all fields)
//
// Built Structure: bits 0-1   (2 bits, max 3)
// Owner:           bits 2-3   (2 bits, max 3) 
// Adjacent Hexes:
// - Hex 1: bits 4-8   (5 bits, max 19)
// - Hex 2: bits 9-13  (5 bits, max 19)
// - Hex 3: bits 14-18 (5 bits, max 19)
// Port Type:   bits 19-21 (3 bits, max 7)
// Adjacent Edges:
// - Edge 1: bits 22-28 (7 bits, max 127)
// - Edge 2: bits 29-35 (7 bits, max 127)
// - Edge 3: bits 36-42 (7 bits, max 127)

using PackedNode = uint64_t;

// Structure packing/unpacking (bits 0-1)
constexpr PackedNode packStructure(PackedNode n, StructureType structure) {
    return (n & ~0x3ULL) | (uint64_t(static_cast<uint8_t>(structure) & 0x3));
}
constexpr StructureType unpackStructure(PackedNode n) {
    return static_cast<StructureType>(n & 0x3);
}

// Owner packing/unpacking (bits 2-3)
constexpr PackedNode packOwner(PackedNode n, PlayerId owner) {
    return (n & ~(0x3ULL << 2)) | (uint64_t(static_cast<uint8_t>(owner) & 0x3) << 2);
}
constexpr PlayerId unpackOwner(PackedNode n) {
    return static_cast<PlayerId>((n >> 2) & 0x3);
}

// Adjacent hex packing/unpacking (bits 4-18, 5 bits each)
constexpr PackedNode packAdjacentHex(PackedNode n, uint8_t hexIndex, HexId hexId) {
    uint8_t shift = 4 + (hexIndex * 5);
    n &= ~(0x1FULL << shift);
    n |= (uint64_t(hexId & 0x1F) << shift);
    return n;
}
constexpr HexId unpackAdjacentHex(PackedNode n, uint8_t hexIndex) {
    uint8_t shift = 4 + (hexIndex * 5);
    return (n >> shift) & 0x1F;
}

// Port type packing/unpacking (bits 19-21)
constexpr PackedNode packPortType(PackedNode n, PortType portType) {
    return (n & ~(0x7ULL << 19)) | (uint64_t(static_cast<uint8_t>(portType) & 0x7) << 19);
}
constexpr PortType unpackPortType(PackedNode n) {
    return static_cast<PortType>((n >> 19) & 0x7);
}

// Adjacent edge packing/unpacking (bits 22-42, 7 bits each)
constexpr PackedNode packAdjacentEdge(PackedNode n, uint8_t edgeIndex, EdgeId edgeId) {
    uint8_t shift = 22 + (edgeIndex * 7);
    n &= ~(0x7FULL << shift);
    n |= (uint64_t(edgeId & 0x7F) << shift);
    return n;
}
constexpr EdgeId unpackAdjacentEdge(PackedNode n, uint8_t edgeIndex) {
    uint8_t shift = 22 + (edgeIndex * 7);
    return (n >> shift) & 0x7F;
}

// Convenience function to create a new node
constexpr PackedNode makeNode(HexId hex1, HexId hex2, HexId hex3,
                            EdgeId edge1, EdgeId edge2, EdgeId edge3,
                            StructureType structure = StructureType::NoStructure,
                            PlayerId owner = PlayerId::NoPlayer,
                            PortType portType = PortType::NoPort) {
    PackedNode n = 0;
    n = packStructure(n, structure);
    n = packOwner(n, owner);
    n = packAdjacentHex(n, 0, hex1);
    n = packAdjacentHex(n, 1, hex2);
    n = packAdjacentHex(n, 2, hex3);
    n = packAdjacentEdge(n, 0, edge1);
    n = packAdjacentEdge(n, 1, edge2);
    n = packAdjacentEdge(n, 2, edge3);
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
    Bank::PackedBank packedBank = Bank::makeNewBank();

    PlayerId currentPlayer = PlayerId::Player1;
    uint8_t currentTurn = 0;

    Action::PackedAction actionQueue[512] = {};
    uint16_t actionQueueSize = 0;

    constexpr BoardState() noexcept;
    void generateRandomBoard();
    void applyAction(Action::PackedAction action);
    void undoLastAction();

    void handlePlaceInitialStructures(Action::PackedAction action, PlayerId playerId);
    void handlePlace2InitialStructures(Action::PackedAction action, PlayerId playerId);
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

    std::vector<Action::PackedAction> getLegalActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuildRoadActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuildSettlementActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuildCityActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBankTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateTwoToOnePortTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateThreeToOnePortTradeActions(PlayerId playerId);
    std::vector<Action::PackedAction> generateBuyDevCardActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlayDevCardActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlayDevCardKnightActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlayDevCardRoadBuildingActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlayDevCardYearOfPlentyActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlayDevCardMonopolyActions(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlaceInitialStructures(PlayerId playerId);
    std::vector<Action::PackedAction> generatePlace2InitialStructures(PlayerId playerId);
    std::vector<Action::PackedAction> generateMoveRobberActions(PlayerId playerId);
};

constexpr BoardState::BoardState() noexcept {
    nodes[0]  = Node::makeNode(HexIdNone, 0, HexIdNone,  0, 6, EdgeIdNone);
    nodes[0] = Node::packPortType(nodes[0], PortType::NoPort);
    nodes[1]  = Node::makeNode(HexIdNone, HexIdNone, 0,  0, 1, EdgeIdNone);
    nodes[1] = Node::packPortType(nodes[1], PortType::NoPort);
    nodes[2]  = Node::makeNode(0, 1, HexIdNone, 1, 2, 7);
    nodes[2] = Node::packPortType(nodes[2], PortType::ThreeForOne);
    nodes[3]  = Node::makeNode(HexIdNone, HexIdNone, 1,  2, 3, EdgeIdNone);
    nodes[3] = Node::packPortType(nodes[3], PortType::ThreeForOne);
    nodes[4]  = Node::makeNode(1, 2, HexIdNone,  3, 4, 4);
    nodes[4] = Node::packPortType(nodes[4], PortType::NoPort);
    nodes[5]  = Node::makeNode(HexIdNone, HexIdNone, 2,  4, 5, EdgeIdNone);
    nodes[5] = Node::packPortType(nodes[5], PortType::ThreeForOne);
    nodes[6]  = Node::makeNode(2, HexIdNone, HexIdNone,  5, 9, EdgeIdNone);
    nodes[6] = Node::packPortType(nodes[6], PortType::ThreeForOne);
    nodes[7]  = Node::makeNode(HexIdNone, 3, HexIdNone,  10, 18, EdgeIdNone);
    nodes[7] = Node::packPortType(nodes[7], PortType::WoolPort);
    nodes[8]  = Node::makeNode(HexIdNone, 0, 3,  6, 10, 11);
    nodes[8] = Node::packPortType(nodes[8], PortType::WoolPort);
    nodes[9]  = Node::makeNode(3, 4, 0,  11, 12, 19);
    nodes[9] = Node::packPortType(nodes[9], PortType::NoPort);
    nodes[10] = Node::makeNode(0, 1, 4,  7, 12, 13);
    nodes[10] = Node::packPortType(nodes[10], PortType::NoPort);
    nodes[11] = Node::makeNode(4, 5, 1,  13, 14, 20);
    nodes[11] = Node::packPortType(nodes[11], PortType::NoPort);
    nodes[12] = Node::makeNode(1, 2, 5,  8, 14, 15);
    nodes[12] = Node::packPortType(nodes[12], PortType::NoPort);
    nodes[13] = Node::makeNode(5, 6, 2,  15, 16, 21);
    nodes[13] = Node::packPortType(nodes[13], PortType::NoPort);
    nodes[14] = Node::makeNode(2, HexIdNone, 6,  9, 16, 17);
    nodes[14] = Node::packPortType(nodes[14], PortType::NoPort);
    nodes[15] = Node::makeNode(6, HexIdNone, HexIdNone, 17, 22, EdgeIdNone);
    nodes[15] = Node::packPortType(nodes[15], PortType::BrickPort);
    nodes[16] = Node::makeNode(HexIdNone, 7, HexIdNone,  23, 33, EdgeIdNone);
    nodes[16] = Node::packPortType(nodes[16], PortType::ThreeForOne);
    nodes[17] = Node::makeNode(HexIdNone, 3, 7,  18, 23, 24);
    nodes[17] = Node::packPortType(nodes[17], PortType::NoPort);
    nodes[18] = Node::makeNode(7, 8, 3,  24, 25, 34);
    nodes[18] = Node::packPortType(nodes[18], PortType::NoPort);
    nodes[19] = Node::makeNode(3, 4, 8,  19, 25, 26);
    nodes[19] = Node::packPortType(nodes[19], PortType::NoPort);
    nodes[20] = Node::makeNode(8, 9, 4,  26, 27, 35);
    nodes[20] = Node::packPortType(nodes[20], PortType::NoPort);
    nodes[21] = Node::makeNode(4, 5, 9, 20, 27, 28);
    nodes[21] = Node::packPortType(nodes[21], PortType::NoPort);
    nodes[22] = Node::makeNode(9,  10, 5,  28, 29, 36);
    nodes[22] = Node::packPortType(nodes[22], PortType::NoPort);
    nodes[23] = Node::makeNode(5, 6, 10,  21, 29, 30);
    nodes[23] = Node::packPortType(nodes[23], PortType::NoPort);
    nodes[24] = Node::makeNode(10, 11, 6,  30, 31, 37);
    nodes[24] = Node::packPortType(nodes[24], PortType::NoPort);
    nodes[25] = Node::makeNode(6, HexIdNone, 11,  22, 31, 32);
    nodes[25] = Node::packPortType(nodes[25], PortType::BrickPort);
    nodes[26] = Node::makeNode(11,  HexIdNone, HexIdNone, 32, 38, EdgeIdNone);
    nodes[26] = Node::packPortType(nodes[26], PortType::NoPort);
    nodes[27] = Node::makeNode(HexIdNone, 7, HexIdNone, 33, 39, EdgeIdNone);
    nodes[27] = Node::packPortType(nodes[27], PortType::ThreeForOne);
    nodes[28] = Node::makeNode(HexIdNone, 12, 7,  39, 40, 49);
    nodes[28] = Node::packPortType(nodes[28], PortType::NoPort);
    nodes[29] = Node::makeNode(7, 8, 12, 34, 40, 41);
    nodes[29] = Node::packPortType(nodes[29], PortType::NoPort);
    nodes[30] = Node::makeNode(12, 13, 8, 41, 42, 50);
    nodes[30] = Node::packPortType(nodes[30], PortType::NoPort);
    nodes[31] = Node::makeNode(8, 9, 13, 35, 42, 43);
    nodes[31] = Node::packPortType(nodes[31], PortType::NoPort);
    nodes[32] = Node::makeNode(13, 14, 9, 43, 44, 51);
    nodes[32] = Node::packPortType(nodes[32], PortType::NoPort);
    nodes[33] = Node::makeNode(9, 10, 14, 36, 44, 45);
    nodes[33] = Node::packPortType(nodes[33], PortType::NoPort);
    nodes[34] = Node::makeNode(14, 15, 10, 45, 46, 52);
    nodes[34] = Node::packPortType(nodes[34], PortType::NoPort);
    nodes[35] = Node::makeNode(10, 11, 15,  37, 46, 47);
    nodes[35] = Node::packPortType(nodes[35], PortType::NoPort);
    nodes[36] = Node::makeNode(15, HexIdNone, 11,  47, 48, 53);
    nodes[36] = Node::packPortType(nodes[36], PortType::LumberPort);
    nodes[37] = Node::makeNode(11, HexIdNone, HexIdNone,  38, 48, EdgeIdNone);
    nodes[37] = Node::packPortType(nodes[37], PortType::NoPort);
    nodes[38] = Node::makeNode(HexIdNone, 12, HexIdNone,  49, 54, EdgeIdNone);
    nodes[38] = Node::packPortType(nodes[38], PortType::OrePort);
    nodes[39] = Node::makeNode(HexIdNone, 16, 12,  54, 55, 62);
    nodes[39] = Node::packPortType(nodes[39], PortType::OrePort);
    nodes[40] = Node::makeNode(12, 13, 16,  50, 55, 56);
    nodes[40] = Node::packPortType(nodes[40], PortType::NoPort);
    nodes[41] = Node::makeNode(16, 17, 13,  56, 57, 63);
    nodes[41] = Node::packPortType(nodes[41], PortType::NoPort);
    nodes[42] = Node::makeNode(13, 14, 17,  51, 57, 58);
    nodes[42] = Node::packPortType(nodes[42], PortType::NoPort);
    nodes[43] = Node::makeNode(17, 18, 14,  58, 59, 64);
    nodes[43] = Node::packPortType(nodes[43], PortType::NoPort);
    nodes[44] = Node::makeNode(14, 15, 18,  52, 59, 60);
    nodes[44] = Node::packPortType(nodes[44], PortType::NoPort);
    nodes[45] = Node::makeNode(18, HexIdNone, 15,  60, 61, 65);
    nodes[45] = Node::packPortType(nodes[45], PortType::NoPort);
    nodes[46] = Node::makeNode(15, HexIdNone, HexIdNone,  53, 61, EdgeIdNone);
    nodes[46] = Node::packPortType(nodes[46], PortType::LumberPort);
    nodes[47] = Node::makeNode(HexIdNone, 16, HexIdNone,  62, 66, EdgeIdNone);
    nodes[47] = Node::packPortType(nodes[47], PortType::NoPort);
    nodes[48] = Node::makeNode(HexIdNone, HexIdNone, 16,  66, 67, EdgeIdNone);
    nodes[48] = Node::packPortType(nodes[48], PortType::NoPort);
    nodes[49] = Node::makeNode(16, 17, HexIdNone,  63, 67, 68);
    nodes[49] = Node::packPortType(nodes[49], PortType::GrainPort);
    nodes[50] = Node::makeNode(HexIdNone, HexIdNone, 17,  68, 69, EdgeIdNone);
    nodes[50] = Node::packPortType(nodes[50], PortType::GrainPort);
    nodes[51] = Node::makeNode(17, 18, HexIdNone,  64, 69, 70);
    nodes[51] = Node::packPortType(nodes[51], PortType::NoPort);
    nodes[52] = Node::makeNode(HexIdNone, HexIdNone, 18,  70, 71, EdgeIdNone);
    nodes[52] = Node::packPortType(nodes[52], PortType::ThreeForOne);
    nodes[53] = Node::makeNode(18, HexIdNone, HexIdNone,  65, 71, EdgeIdNone);
    nodes[53] = Node::packPortType(nodes[53], PortType::ThreeForOne);

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

} // namespace Board