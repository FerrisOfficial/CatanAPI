#include "actions.hpp"
#include "board.hpp"
#include "consts.hpp"
#include "packedBank.hpp"
#include "utils/randomDevice.hpp"


namespace Board {

void handlePlaceInitialSettlement(BoardState& boardState, NodeId nodeId, PlayerId playerId) {
    boardState.nodes[nodeId] = Node::packStructure(boardState.nodes[nodeId], StructureType::Settlement);
    boardState.nodes[nodeId] = Node::packOwner(boardState.nodes[nodeId], playerId);

    // Update per-hex production counters for this settlement.
    // (Unlike the 2nd initial settlement, we do NOT grant immediate starting resources.)
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(boardState.nodes[nodeId], i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) {
            continue;
        }
        auto res = Hex::unpackResource(boardState.hexes[hexId]);
        if (res != Resource::NoResource) {
            boardState.hexes[hexId] = Hex::packPlayerValue(
                boardState.hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(boardState.hexes[hexId], playerId) + 1
            );
        }
    }

    // add victory point to player
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)]) + 1
        );

    // remove one available settlement from player
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) - 1
        );
}

void handlePlaceInitialRoad(BoardState& boardState, EdgeId edgeId, PlayerId playerId) {
    boardState.edges[edgeId] = Edge::packHasRoad(boardState.edges[edgeId], true);
    boardState.edges[edgeId] = Edge::packOwner(boardState.edges[edgeId], playerId);

    // remove one available road from player
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Road,
            Player::unpackAvailableStructures(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Road) - 1
        );
}

    void handleUndoPlaceInitialRoad(BoardState& boardState, EdgeId edgeId, PlayerId playerId);

void handlePlace2InitialSettlement(BoardState& boardState, NodeId nodeId, PlayerId playerId) {
    boardState.nodes[nodeId] = Node::packStructure(boardState.nodes[nodeId], StructureType::Settlement);
    boardState.nodes[nodeId] = Node::packOwner(boardState.nodes[nodeId], playerId);
    // add resources to player for 2nd settlement
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(boardState.nodes[nodeId], i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) {
            continue;
        }
        auto res = Hex::unpackResource(boardState.hexes[hexId]);
        if (res != Resource::NoResource) {
            Player::changeResourceQuantity(boardState.packedPlayers[static_cast<uint8_t>(playerId)], res, 1);
            Bank::changeResourceQuantity(boardState.packedBank, res, -1);

            boardState.hexes[hexId] = Hex::packPlayerValue(
                boardState.hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(boardState.hexes[hexId], playerId) + 1
            );
        }
    }
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)]) + 1
        );
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) - 1
        );
}

void BoardState::handlePlaceInitialStructures(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    auto edgeId = Action::unpackArg2(action);

    handlePlaceInitialSettlement(
        *this,
        nodeId,
        playerId
    );

    handlePlaceInitialRoad(
        *this,
        edgeId,
        playerId
    );
}

void BoardState::handleUndoPlaceInitialSettlement(Action::PackedAction action, PlayerId playerId) {
    NodeId nodeId = Action::unpackArg1(action);

    // Revert per-hex production counters for this settlement.
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(nodes[nodeId], i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) {
            continue;
        }
        auto res = Hex::unpackResource(hexes[hexId]);
        if (res != Resource::NoResource) {
            hexes[hexId] = Hex::packPlayerValue(
                hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(hexes[hexId], playerId) - 1
            );
        }
    }

    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);
    // remove victory point from player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) - 1
        );
    // add one available settlement to player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) + 1
        );
    
    EdgeId edgeId = Action::unpackArg2(action);
    handleUndoPlaceInitialRoad(*this, edgeId, playerId);
}

void BoardState::handlePlace2InitialStructures(Action::PackedAction action, PlayerId playerId) {
    NodeId nodeId = Action::unpackArg1(action);
    EdgeId edgeId = Action::unpackArg2(action);

    handlePlace2InitialSettlement(*this, nodeId, playerId);
    handlePlaceInitialRoad(*this, edgeId, playerId);
}

void handleUndoPlaceInitialRoad(BoardState& boardState, EdgeId edgeId, PlayerId playerId) {
    boardState.edges[edgeId] = Edge::packHasRoad(boardState.edges[edgeId], false);
    boardState.edges[edgeId] = Edge::packOwner(boardState.edges[edgeId], PlayerId::NoPlayer);

    // add one available road to player
    boardState.packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Road,
            Player::unpackAvailableStructures(
                boardState.packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Road) + 1
        );
}

void BoardState::handleUndoPlace2InitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);
    // remove resources from player for 2nd settlement
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(nodes[nodeId], i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) {
            continue;
        }
        auto res = Hex::unpackResource(hexes[hexId]);
        if (res != Resource::NoResource) {
            Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], res, -1);
            Bank::changeResourceQuantity(packedBank, res, 1);

            hexes[hexId] = Hex::packPlayerValue(
                hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(hexes[hexId], playerId) - 1
            );
        }
    }
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) - 1
        );
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) + 1
        );

    EdgeId edgeId = Action::unpackArg2(action);
    handleUndoPlaceInitialRoad(*this, edgeId, playerId);
}

Action::PackedAction BoardState::handleEndTurn() {
    // Advance to next player's turn
    currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    currentTurn += 1;

    return Action::packType(0, ActionType::EndTurn);
}

void BoardState::handleUndoEndTurn() {
    // Revert to previous player's turn
    currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    currentTurn -= 1;
}

Action::PackedAction BoardState::handleRollDice(Action::PackedAction action) {
    uint8_t dice = Action::unpackArg1(action);

    // For each hex with matching catan number, produce resources
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(hexes[h]) != dice) continue;
        Resource res = Hex::unpackResource(hexes[h]);

        auto &p0 = packedPlayers[static_cast<uint8_t>(PlayerId::Player0)];
        auto &p1 = packedPlayers[static_cast<uint8_t>(PlayerId::Player1)];

        auto bankResBefore = Bank::unpackResource(BoardState::packedBank, res);

        if (bankResBefore < Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) + Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)) {
            // Not enough resources in bank to fulfill the production
            continue;
            }

        Player::changeResourceQuantity(p0, res,
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player0));
        Player::changeResourceQuantity(p1, res, 
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1));
        Bank::changeResourceQuantity(BoardState::packedBank, res,
            -(Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) +
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)));
    }

    return Action::packArg1(action, dice);
}

void BoardState::handleUndoRollDice(Action::PackedAction action){
    uint8_t dice = Action::unpackArg1(action);

    // For each hex with matching catan number, produce resources
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(hexes[h]) != dice) continue;
        Resource res = Hex::unpackResource(hexes[h]);

        auto &p0 = packedPlayers[static_cast<uint8_t>(PlayerId::Player0)];
        auto &p1 = packedPlayers[static_cast<uint8_t>(PlayerId::Player1)];

        Player::changeResourceQuantity(p0, res,
            -Hex::unpackPlayerValue(hexes[h], PlayerId::Player0));
        Player::changeResourceQuantity(p1, res, 
            -Hex::unpackPlayerValue(hexes[h], PlayerId::Player1));
        Bank::changeResourceQuantity(BoardState::packedBank, res,
            (Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) +
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)));
    }
}

Action::PackedAction BoardState::handleMoveRobber(Action::PackedAction action, PlayerId playerId) {
    auto hexId = Action::unpackArg1(action);
    action = Action::packArg1(action, robberPosition);
    robberPosition = hexId;
    
    return action;
}

void BoardState::handleUndoMoveRobber(Action::PackedAction action, PlayerId playerId) {
    auto previousHexId = Action::unpackArg1(action);
    robberPosition = previousHexId;
}

static bool nodeBlocksRoad(BoardState board, PlayerId playerId, NodeId nodeId) {
    auto structure = Node::unpackStructure(board.nodes[nodeId]);
    auto owner = Node::unpackOwner(board.nodes[nodeId]);
    return (structure == StructureType::Settlement || structure == StructureType::City) && owner != playerId;
}

static int dfsLongestFromEdge(BoardState board, PlayerId playerId, EdgeId edgeId, NodeId cameFromNode,
                                  std::vector<uint8_t>& used)
{
    used[edgeId] = 1;
    int best = 1;

    NodeId n0 = Edge::unpackAdjacentNode(board.edges[edgeId], 0);
    NodeId n1 = Edge::unpackAdjacentNode(board.edges[edgeId], 1);
    NodeId currentNode = (n0 == cameFromNode) ? n1 : n0;

    if (nodeBlocksRoad(board, playerId, currentNode)) {
        used[edgeId] = 0;
        return best;
    }

    for (int i = 0; i < 3; ++i) {
        EdgeId next = Node::unpackAdjacentEdge(board.nodes[currentNode], i);
        if (next == EdgeIdNone) continue;
        if (used[next]) continue;

        if (!Edge::unpackHasRoad(board.edges[next])) continue;
        if (Edge::unpackOwner(board.edges[next]) != playerId) continue;

        best = std::max(best, 1 + dfsLongestFromEdge(board, playerId, next, currentNode, used));
    }

    used[edgeId] = 0;
    return best;
}

static uint8_t computeLongestRoad(BoardState board, PlayerId playerId) {
    int best = 0;
    std::vector<uint8_t> used(EDGE_COUNT, 0);

    for (EdgeId e = 0; e < EDGE_COUNT; ++e) {
        if (!Edge::unpackHasRoad(board.edges[e])) continue;
        if (Edge::unpackOwner(board.edges[e]) != playerId) continue;

        NodeId a = Edge::unpackAdjacentNode(board.edges[e], 0);
        NodeId b = Edge::unpackAdjacentNode(board.edges[e], 1);

        int leftLen  = dfsLongestFromEdge(board, playerId, e, a, used);
        int rightLen = dfsLongestFromEdge(board, playerId, e, b, used);

        // Combine both directions through edge e. Each dfs counts edge e once,
        // so subtract 1 to avoid double-counting the starting edge.
        best = std::max(best, leftLen + rightLen - 1);
    }

    if (best < 0) best = 0;
    if (best > 15) best = 15;
    return static_cast<uint8_t>(best);
}

static void updateLongestRoadAwards(BoardState &board, PlayerId playingPlayer, uint8_t playingPlayerLength) {
    auto enemyPlayer = (playingPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    uint8_t enemyPlayerLength = computeLongestRoad(board, enemyPlayer);
    board.packedPlayers[static_cast<uint8_t>(enemyPlayer)] =
        Player::packLongestRoadLength(
            board.packedPlayers[static_cast<uint8_t>(enemyPlayer)],
            enemyPlayerLength
        );

    if (playingPlayerLength >= 5 && playingPlayerLength > enemyPlayerLength) {
        // playing player takes longest road
        board.packedPlayers[static_cast<uint8_t>(playingPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(playingPlayer)], true);
        board.packedPlayers[static_cast<uint8_t>(enemyPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(enemyPlayer)], false);
        return;
    }

    if (enemyPlayerLength >= 5 && enemyPlayerLength > playingPlayerLength) {
        // enemy player takes longest road
        board.packedPlayers[static_cast<uint8_t>(playingPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(playingPlayer)], false);
        board.packedPlayers[static_cast<uint8_t>(enemyPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(enemyPlayer)], true);
        return;
    }

    // Tie or neither qualifies:
    // - If both lengths < 5, clear any flags
    // - If both lengths >= 5 and equal, preserve existing flags (do nothing)
    if (playingPlayerLength < 5 && enemyPlayerLength < 5) {
        board.packedPlayers[static_cast<uint8_t>(playingPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(playingPlayer)], false);
        board.packedPlayers[static_cast<uint8_t>(enemyPlayer)] =
            Player::packLongestRoadFlag(board.packedPlayers[static_cast<uint8_t>(enemyPlayer)], false);
    }
}

static void updateLongestRoadAwards(BoardState &board, PlayerId playingPlayer) {
    uint8_t len = computeLongestRoad(board, playingPlayer);
    updateLongestRoadAwards(board, playingPlayer, len);
}


void BoardState::handleBuildRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], true);
    edges[edgeId] = Edge::packOwner(edges[edgeId], playerId);

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Road);

    // Recompute longest road for player
    uint8_t longestRoad = computeLongestRoad(*this, playerId);
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packLongestRoadLength(
            packedPlayers[static_cast<uint8_t>(playerId)],
            longestRoad
        );

    updateLongestRoadAwards(*this, playerId, longestRoad);
}

void BoardState::handleUndoBuildRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], false);
    edges[edgeId] = Edge::packOwner(edges[edgeId], PlayerId::NoPlayer);

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Road, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);

    // Restore derived "longest road" fields exactly as they were before apply.
    // (Tie behavior depends on history, so recomputing here is not reversible.)
    {
        uint8_t p0Meta = Action::unpackArg2(action);
        uint8_t p1Meta = Action::unpackArg3(action);

        uint8_t p0Len = p0Meta & 0xF;
        bool p0Flag = (p0Meta & 0x10) != 0;
        uint8_t p1Len = p1Meta & 0xF;
        bool p1Flag = (p1Meta & 0x10) != 0;

        packedPlayers[0] = Player::packLongestRoadLength(packedPlayers[0], p0Len);
        packedPlayers[0] = Player::packLongestRoadFlag(packedPlayers[0], p0Flag);
        packedPlayers[1] = Player::packLongestRoadLength(packedPlayers[1], p1Len);
        packedPlayers[1] = Player::packLongestRoadFlag(packedPlayers[1], p1Flag);
    }
}    

void BoardState::handleBuildSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) + 1
            );
        }
    }

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Settlement);

    updateLongestRoadAwards(*this, playerId);
}

void BoardState::handleUndoBuildSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) - 1
            );
        }
    }

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Settlement, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement);

    // Restore derived "longest road" fields exactly as they were before apply.
    {
        uint8_t p0Meta = Action::unpackArg2(action);
        uint8_t p1Meta = Action::unpackArg3(action);

        uint8_t p0Len = p0Meta & 0xF;
        bool p0Flag = (p0Meta & 0x10) != 0;
        uint8_t p1Len = p1Meta & 0xF;
        bool p1Flag = (p1Meta & 0x10) != 0;

        packedPlayers[0] = Player::packLongestRoadLength(packedPlayers[0], p0Len);
        packedPlayers[0] = Player::packLongestRoadFlag(packedPlayers[0], p0Flag);
        packedPlayers[1] = Player::packLongestRoadLength(packedPlayers[1], p1Len);
        packedPlayers[1] = Player::packLongestRoadFlag(packedPlayers[1], p1Flag);
    }
}

void BoardState::handleBuildCity(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::City);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) + 1
            );
        }
    }

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::City);
}

void BoardState::handleUndoBuildCity(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) - 1
            );
        }
    }

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::City, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City);
}

Action::PackedAction BoardState::handleBuyDevCard(PlayerId playerId) {
    const uint32_t pick = RandomDevice::uniform_u32(Bank::unpackTotalDevCount(packedBank));
    uint32_t acc = 0;
    DevType DevTypeOfpick = DevType::NoDev;

    for (DevType d : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        uint32_t count = Bank::unpackDevCard(BoardState::packedBank, d);
        if (pick < acc + count){
            DevTypeOfpick = d;
            break;
        }
        acc += count;
    }

    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::DevCard, DevTypeOfpick);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::DevCard, DevTypeOfpick);

    Action::PackedAction action = 0;
    action = Action::packType(action, ActionType::BuyDevCard);
    action = Action::packPlayerID(action, playerId);
    action = Action::packArg1(action, static_cast<uint8_t>(DevTypeOfpick));
    return action;
}

void BoardState::handleUndoBuyDevCard(Action::PackedAction action, PlayerId playerId) {
    auto d = Action::unpackArg1(action);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::DevCard, static_cast<DevType>(d));
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::DevCard, static_cast<DevType>(d), false);
}

Action::PackedAction BoardState::handlePlayDevCardKnight(Action::PackedAction action, PlayerId playerId) {
    // Decrement knight dev card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto enemyPlayerId = (playerId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    auto previousRobberPosition = robberPosition;

    // Store previous Largest Army flags so undo can restore exactly.
    // This avoids recomputing edge-cases (e.g. ties) on undo.
    {
        const bool prevSelf = Player::unpackLargestArmyFlag(packedPlayers[static_cast<uint8_t>(playerId)]);
        const bool prevEnemy = Player::unpackLargestArmyFlag(packedPlayers[static_cast<uint8_t>(enemyPlayerId)]);
        const uint8_t flags = (prevSelf ? 1u : 0u) | (prevEnemy ? 2u : 0u);
        action = Action::packArg2(action, flags);
    }
    p = Player::packDevCard(
        p,
        DevType::Knight,
        Player::unpackDevCard(p, DevType::Knight) - 1
    );

    p = Player::packUsedKnights(
        p,
        Player::unpackUsedKnights(p) + 1
    );

    uint8_t usedKnights = Player::unpackUsedKnights(p);
    if ((usedKnights >= 3) && (Player::unpackUsedKnights(packedPlayers[static_cast<uint8_t>(enemyPlayerId)]) < usedKnights) && !(Player::unpackLargestArmyFlag(p))) {
        packedPlayers[static_cast<uint8_t>(playerId)] =
            Player::packLargestArmyFlag(
                packedPlayers[static_cast<uint8_t>(playerId)],
                true
            );
        packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
            Player::packLargestArmyFlag(
                packedPlayers[static_cast<uint8_t>(enemyPlayerId)],
                false
            );
    }

    handleMoveRobber(action, playerId);
    action = Action::packArg1(action, previousRobberPosition);

    return action;
}

void BoardState::handleUndoPlayDevCardKnight(Action::PackedAction action, PlayerId playerId) {
    // Increment knight dev card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto robberPosition = Action::unpackArg1(action);
    p = Player::packDevCard(
        p,
        DevType::Knight,
        Player::unpackDevCard(p, DevType::Knight) + 1
    );

    p = Player::packUsedKnights(
        p,
        Player::unpackUsedKnights(p) - 1
    );

    // Restore Largest Army flags exactly as before the Knight was played.
    auto enemyPlayerId = (playerId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    {
        const uint8_t flags = Action::unpackArg2(action);
        const bool prevSelf = (flags & 1u) != 0;
        const bool prevEnemy = (flags & 2u) != 0;
        packedPlayers[static_cast<uint8_t>(playerId)] =
            Player::packLargestArmyFlag(packedPlayers[static_cast<uint8_t>(playerId)], prevSelf);
        packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
            Player::packLargestArmyFlag(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], prevEnemy);
    }

    action = Action::packArg1(action, robberPosition);
    handleUndoMoveRobber(action, playerId);
}

void BoardState::handlePlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    p = Player::packDevCard(
        p,
        DevType::RoadBuilding,
        Player::unpackDevCard(p, DevType::RoadBuilding) - 1
    );

    // Build two roads
    auto firstEdgeId = Action::unpackArg1(action);
    edges[firstEdgeId] = Edge::packHasRoad(edges[firstEdgeId], true);
    edges[firstEdgeId] = Edge::packOwner(edges[firstEdgeId], playerId);
    
    auto secondEdgeId = Action::unpackArg2(action);
    edges[secondEdgeId] = Edge::packHasRoad(edges[secondEdgeId], true);
    edges[secondEdgeId] = Edge::packOwner(edges[secondEdgeId], playerId);

    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            p,
            StructureType::Road,
            Player::unpackAvailableStructures(
                p,
                StructureType::Road) - 2
        );
}

void BoardState::handleUndoPlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    p = Player::packDevCard(
        p,
        DevType::RoadBuilding,
        Player::unpackDevCard(p, DevType::RoadBuilding) + 1
    );

    // Remove two roads
    auto firstEdgeId = Action::unpackArg1(action);
    edges[firstEdgeId] = Edge::packHasRoad(edges[firstEdgeId], false);
    edges[firstEdgeId] = Edge::packOwner(edges[firstEdgeId], PlayerId::NoPlayer);
    
    auto secondEdgeId = Action::unpackArg2(action);
    edges[secondEdgeId] = Edge::packHasRoad(edges[secondEdgeId], false);
    edges[secondEdgeId] = Edge::packOwner(edges[secondEdgeId], PlayerId::NoPlayer);

    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            p,
            StructureType::Road,
            Player::unpackAvailableStructures(
                p,
                StructureType::Road) + 2
        );
}

void BoardState::handlePlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    p = Player::packDevCard(
        p,
        DevType::YearOfPlenty,
        Player::unpackDevCard(p, DevType::YearOfPlenty) - 1
    );

    Resource firstResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource secondResource = static_cast<Resource>(Action::unpackArg2(action));

    // Add first resource to player
    Player::changeResourceQuantity(p, firstResource, 1);
    Bank::changeResourceQuantity(packedBank, firstResource, -1);

    // Add second resource to player
    Player::changeResourceQuantity(p, secondResource, 1);
    Bank::changeResourceQuantity(packedBank, secondResource, -1);
}

void BoardState::handleUndoPlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    p = Player::packDevCard(
        p,
        DevType::YearOfPlenty,
        Player::unpackDevCard(p, DevType::YearOfPlenty) + 1
    );

    Resource firstResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource secondResource = static_cast<Resource>(Action::unpackArg2(action));

    // Remove first resource from player
    Player::changeResourceQuantity(p, firstResource, -1);
    Bank::changeResourceQuantity(packedBank, firstResource, 1);

    // Remove second resource from player
    Player::changeResourceQuantity(p, secondResource, -1);
    Bank::changeResourceQuantity(packedBank, secondResource, 1);
}

Action::PackedAction BoardState::handlePlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId) {
    // First, decrement the monopoly card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
   
    p = Player::packDevCard(
        p,
        DevType::Monopoly,
        Player::unpackDevCard(p, DevType::Monopoly) - 1
    );


    Resource targetResource = static_cast<Resource>(Action::unpackArg1(action));
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    
    auto enemyHave = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource);
    
    // Remove all of this resource from enemy
    if (enemyHave > 0) {
        packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
            Player::packResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource, 0);

        Player::changeResourceQuantity(p, targetResource, enemyHave);
    }

    action = Action::packArg2(action, enemyHave);
    return action;
}

void BoardState::handleUndoPlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId) {
    // First, increment the monopoly card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
   
    p = Player::packDevCard(
        p,
        DevType::Monopoly,
        Player::unpackDevCard(p, DevType::Monopoly) + 1
    );

    Resource targetResource = static_cast<Resource>(Action::unpackArg1(action));
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;

    uint8_t stolenAmount = Action::unpackArg2(action);

    // Return resources to enemy
    if (stolenAmount > 0) {
        Player::changeResourceQuantity(
            packedPlayers[static_cast<uint8_t>(enemyPlayerId)],
            targetResource,
            stolenAmount
        );

        Player::changeResourceQuantity(p, targetResource, -stolenAmount);
    }
}

void BoardState::handleTradeBank(Action::PackedAction action, PlayerId playerId) {
    Resource giveResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource receiveResource = static_cast<Resource>(Action::unpackArg2(action));
    uint8_t ratio = Action::unpackArg3(action);

    auto idx = static_cast<uint8_t>(playerId);
    auto &p = packedPlayers[idx];

    // Player gives 'ratio'
    Player::changeResourceQuantity(p, giveResource, -ratio);
    Player::changeResourceQuantity(p, receiveResource, 1);    

    uint8_t bankGive = Bank::unpackResource(packedBank, giveResource);
    Bank::changeResourceQuantity(packedBank, giveResource, ratio);
    Bank::changeResourceQuantity(packedBank, receiveResource, -1);
}

void BoardState::handleUndoTradeBank(Action::PackedAction action, PlayerId playerId) {
    Resource giveResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource receiveResource = static_cast<Resource>(Action::unpackArg2(action));
    uint8_t ratio = Action::unpackArg3(action);

    auto idx = static_cast<uint8_t>(playerId);
    auto &p = packedPlayers[idx];

    // Player gets back 'ratio'
    Player::changeResourceQuantity(p, giveResource, ratio);
    Player::changeResourceQuantity(p, receiveResource, -1);    

    Bank::changeResourceQuantity(packedBank, giveResource, -ratio);
    Bank::changeResourceQuantity(packedBank, receiveResource, 1);
}

void BoardState::handleReceiveResources(Action::PackedAction action, PlayerId playerId) {
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    uint8_t amount = Action::unpackArg2(action);
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    Player::changeResourceQuantity(p, res, amount);
    Bank::changeResourceQuantity(packedBank, res, -amount);
}

void BoardState::handleUndoReceiveResources(Action::PackedAction action, PlayerId playerId) {
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    uint8_t amount = Action::unpackArg2(action);
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    Player::changeResourceQuantity(p, res, -amount);
    Bank::changeResourceQuantity(packedBank, res, amount);
}

void BoardState::handleDiscardResources(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    Player::changeResourceQuantity(p, Resource::Brick, -Action::unpackResource(action, Resource::Brick));
    Player::changeResourceQuantity(p, Resource::Lumber, -Action::unpackResource(action, Resource::Lumber));
    Player::changeResourceQuantity(p, Resource::Wool, -Action::unpackResource(action, Resource::Wool));
    Player::changeResourceQuantity(p, Resource::Grain, -Action::unpackResource(action, Resource::Grain));
    Player::changeResourceQuantity(p, Resource::Ore, -Action::unpackResource(action, Resource::Ore));
}

void BoardState::handleUndoDiscardResources(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    Player::changeResourceQuantity(p, Resource::Brick, Action::unpackResource(action, Resource::Brick));
    Player::changeResourceQuantity(p, Resource::Lumber, Action::unpackResource(action, Resource::Lumber));
    Player::changeResourceQuantity(p, Resource::Wool, Action::unpackResource(action, Resource::Wool));
    Player::changeResourceQuantity(p, Resource::Grain, Action::unpackResource(action, Resource::Grain));
    Player::changeResourceQuantity(p, Resource::Ore, Action::unpackResource(action, Resource::Ore));
}

Action::PackedAction BoardState::handleStealResource(Action::PackedAction action, PlayerId playerId) {
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;

    uint32_t total = Player::totalResources(packedPlayers[static_cast<uint8_t>(enemyPlayerId)]);
    if (total == 0) {
        // Nothing to steal; keep action as-is for undo symmetry.
        return action;
    }
    uint32_t pick = RandomDevice::uniform_u32(total);
    int chosenRes = 0;
    uint32_t acc = 0;

    for (Resource r : {
        Resource::Brick,
        Resource::Lumber,
        Resource::Wool,
        Resource::Grain,
        Resource::Ore
    }) {
        uint8_t resCount = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], r);
        if (pick < acc + resCount) {
            chosenRes = static_cast<int>(r);
            break;
        }
        acc += resCount;
    }
    
    // handleDiscardResources();
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], static_cast<Resource>(chosenRes), -1);
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], static_cast<Resource>(chosenRes), 1);

    // Record which resource was stolen so undo can restore exactly.
    action = Action::packArg1(action, static_cast<uint8_t>(chosenRes));
    return action;
}

void BoardState::handleUndoStealResource(Action::PackedAction action, PlayerId playerId) {
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], res, 1);
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], res, -1);
}

void BoardState::applyAction(Action::PackedAction action) {
    auto type = Action::unpackType(action);
    auto playerId = Action::unpackPlayerID(action);

    switch (type) {
    case ActionType::EndTurn:
        action = handleEndTurn();
        break;
    case ActionType::RollDice:
        action |= handleRollDice(action);
        break;
    case ActionType::MoveRobber:
        action = handleMoveRobber(action, playerId);
        break;
    case ActionType::DiscardResources:
        handleDiscardResources(action, playerId);
        break;
    case ActionType::BuildRoad:
        // BuildRoad can affect derived "longest road" fields for both players.
        // Store the pre-action values so undo can restore them exactly.
        {
            uint8_t p0Meta = (Player::unpackLongestRoadLength(packedPlayers[0]) & 0xF)
                | (static_cast<uint8_t>(Player::unpackLongestRoadFlag(packedPlayers[0])) << 4);
            uint8_t p1Meta = (Player::unpackLongestRoadLength(packedPlayers[1]) & 0xF)
                | (static_cast<uint8_t>(Player::unpackLongestRoadFlag(packedPlayers[1])) << 4);
            action = Action::packArg2(action, p0Meta);
            action = Action::packArg3(action, p1Meta);
        }
        handleBuildRoad(action, playerId);
        break;
    case ActionType::BuildSettlement:
        // BuildSettlement can affect derived "longest road" fields (settlements can block roads).
        // Store the pre-action values so undo can restore them exactly.
        {
            uint8_t p0Meta = (Player::unpackLongestRoadLength(packedPlayers[0]) & 0xF)
                | (static_cast<uint8_t>(Player::unpackLongestRoadFlag(packedPlayers[0])) << 4);
            uint8_t p1Meta = (Player::unpackLongestRoadLength(packedPlayers[1]) & 0xF)
                | (static_cast<uint8_t>(Player::unpackLongestRoadFlag(packedPlayers[1])) << 4);
            action = Action::packArg2(action, p0Meta);
            action = Action::packArg3(action, p1Meta);
        }
        handleBuildSettlement(action, playerId);
        break;
    case ActionType::BuildCity:
        handleBuildCity(action, playerId);
        break;
    case ActionType::BuyDevCard:
        action |= handleBuyDevCard(playerId);
        break;
    case ActionType::PlayDevCardKnight:
        action = handlePlayDevCardKnight(action, playerId);
        break;
    case ActionType::PlayDevCardRoadBuilding:
        handlePlayDevCardRoadBuilding(action, playerId);
        break;
    case ActionType::PlayDevCardYearOfPlenty:
        handlePlayDevCardYearOfPlenty(action, playerId);
        break;
    case ActionType::PlayDevCardMonopoly:
        action = handlePlayDevCardMonopoly(action, playerId);
        break;
    case ActionType::TradeBank:
        handleTradeBank(action, playerId);
        break;
    case ActionType::ReceiveResources:
        handleReceiveResources(action, playerId);
        break;
    case ActionType::StealResource:
        action = handleStealResource(action, playerId);
        break;
    case ActionType::PlaceInitialStructures:
        handlePlaceInitialStructures(action, playerId);
        break;
    case ActionType::Place2InitialStructures:
        handlePlace2InitialStructures(action, playerId);
        break;

    default:
        break;
    }

    actionQueue.push_back(action);
}

void BoardState::undoLastAction() {
    Action::PackedAction action = actionQueue.back();
    actionQueue.pop_back();
    auto type = Action::unpackType(action);
    auto playerId = Action::unpackPlayerID(action);

    switch (type) {
    case ActionType::EndTurn:
        handleUndoEndTurn();
        break;
    case ActionType::PlaceInitialStructures:
        handleUndoPlaceInitialSettlement(action, playerId);
        break;
    case ActionType::Place2InitialStructures:
        handleUndoPlace2InitialSettlement(action, playerId);
        break;
    case ActionType::RollDice:
        handleUndoRollDice(action);
        break;
    case ActionType::MoveRobber:
        handleUndoMoveRobber(action, playerId);
        break;
    case ActionType::DiscardResources:
        handleUndoDiscardResources(action, playerId);
        break;
    case ActionType::BuildRoad:
        handleUndoBuildRoad(action, playerId);
        break;
    case ActionType::BuildSettlement:
        handleUndoBuildSettlement(action, playerId);
        break;
    case ActionType::BuildCity:
        handleUndoBuildCity(action, playerId);
        break;
    case ActionType::BuyDevCard:
        handleUndoBuyDevCard(action, playerId);
        break;
    case ActionType::PlayDevCardKnight:
        handleUndoPlayDevCardKnight(action, playerId);
        break;
    case ActionType::PlayDevCardRoadBuilding:
        handleUndoPlayDevCardRoadBuilding(action, playerId);
        break;
    case ActionType::PlayDevCardYearOfPlenty:
        handleUndoPlayDevCardYearOfPlenty(action, playerId);
        break;
    case ActionType::PlayDevCardMonopoly:
        handleUndoPlayDevCardMonopoly(action, playerId);
        break;
    case ActionType::TradeBank:
        handleUndoTradeBank(action, playerId);
        break;
    case ActionType::ReceiveResources:
        handleUndoReceiveResources(action, playerId);
        break;
    case ActionType::StealResource:
        handleUndoStealResource(action, playerId);
        break;
    default:
        break;
    }
}

void BoardState::generateRandomBoard() {
    auto& rng = RandomDevice::get_rng();

    uint8_t numberDistribution[HEX_COUNT] = {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12};
    Resource resourceDistribution[HEX_COUNT] =
    {
        Resource::Brick, Resource::Brick, Resource::Brick,
        Resource::Lumber, Resource::Lumber, Resource::Lumber, Resource::Lumber,
        Resource::Wool, Resource::Wool, Resource::Wool, Resource::Wool,
        Resource::Grain, Resource::Grain, Resource::Grain,
        Resource::Ore, Resource::Ore, Resource::Ore,
        Resource::NoResource
    };
    for (size_t i = 0; i < HEX_COUNT; ++i) {
        uint32_t j = rng() % (i + 1);
        std::swap(numberDistribution[i], numberDistribution[j]);
        std::swap(resourceDistribution[i], resourceDistribution[j]);
    }

    // Enforce: the desert (NoResource) always has number 7.
    // This guarantees 7 doesn't appear on any non-desert hex.
    size_t desertIdx = HEX_COUNT;
    size_t sevenIdx = HEX_COUNT;
    for (size_t i = 0; i < HEX_COUNT; ++i) {
        if (resourceDistribution[i] == Resource::NoResource) {
            desertIdx = i;
        }
        if (numberDistribution[i] == 7) {
            sevenIdx = i;
        }
    }
    if (desertIdx < HEX_COUNT && sevenIdx < HEX_COUNT && desertIdx != sevenIdx) {
        std::swap(numberDistribution[desertIdx], numberDistribution[sevenIdx]);
    }

    for (HexId h = 0; h < HEX_COUNT; ++h) {
        hexes[h] = Hex::packResource(hexes[h], resourceDistribution[h]);
        hexes[h] = Hex::packCatanNumber(hexes[h], numberDistribution[h]);

        if (resourceDistribution[h] == Resource::NoResource) {
            robberPosition = h;
        }
    }
}

} // namespace Board