#include "settlePrioGreedyPlayer.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace {
constexpr uint8_t pipWeight[13] = {0, 0, 1, 2, 3, 4, 5, 0, 5, 4, 3, 2, 1};

constexpr uint8_t bit(Resource r) {
    return uint8_t(1u << static_cast<uint8_t>(r));
}

constexpr uint8_t kTargetMask = bit(Resource::Brick) | bit(Resource::Lumber) | bit(Resource::Wool) | bit(Resource::Grain);

uint8_t popcount8(uint8_t v) {
    uint8_t c = 0;
    while (v) {
        v &= static_cast<uint8_t>(v - 1);
        ++c;
    }
    return c;
}

uint8_t nodeResourceMask(NodeId nodeId, Board::BoardState* board) {
    uint8_t mask = 0;

    HexId adjHex[3] = {
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 0),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 1),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 2),
    };

    for (HexId h : adjHex) {
        if (h == HexIdNone) continue;
        const auto hex = board->hexes[h];
        const Resource res = Board::Hex::unpackResource(hex);
        if (res == Resource::NoResource) continue;
        if (static_cast<uint8_t>(res) <= static_cast<uint8_t>(Resource::Ore)) {
            mask |= bit(res);
        }
    }

    return mask;
}

uint16_t nodePipScoreForMask(NodeId nodeId, Board::BoardState* board, uint8_t resMask) {
    uint16_t score = 0;

    HexId adjHex[3] = {
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 0),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 1),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 2),
    };

    for (HexId h : adjHex) {
        if (h == HexIdNone) continue;
        const auto hex = board->hexes[h];
        const Resource res = Board::Hex::unpackResource(hex);
        if (res == Resource::NoResource) continue;
        if ((resMask & bit(res)) == 0) continue;

        const uint8_t number = Board::Hex::unpackCatanNumber(hex);
        score += static_cast<uint16_t>(pipWeight[number]);
    }

    return score;
}

uint16_t placementNodeScore(NodeId nodeId, Board::BoardState* board, uint8_t missingTargetMask) {
    const uint8_t resMask = nodeResourceMask(nodeId, board);

    const uint8_t newCovered = popcount8(static_cast<uint8_t>(resMask & missingTargetMask & kTargetMask));
    const uint8_t totalTarget = popcount8(static_cast<uint8_t>(resMask & kTargetMask));
    const uint8_t distinctAll = popcount8(static_cast<uint8_t>(resMask & (kTargetMask | bit(Resource::Ore))));

    const bool hasOre = (resMask & bit(Resource::Ore)) != 0;

    const uint16_t pipScoreTarget = nodePipScoreForMask(nodeId, board, kTargetMask);

    uint16_t score = 0;
    score += static_cast<uint16_t>(newCovered) * 100;
    score += static_cast<uint16_t>(totalTarget) * 15;
    score += static_cast<uint16_t>(distinctAll) * 3;
    score += static_cast<uint16_t>(pipScoreTarget) * 4;

    if (hasOre) {
        score = static_cast<uint16_t>(score > 5 ? score - 5 : 0);
    }

    return score;
}

uint16_t roadEdgeScoreFromNode(NodeId startNodeId, EdgeId edgeId, Board::BoardState* board, uint8_t missingTargetMask) {
    NodeId nodeA = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    NodeId nodeB = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    NodeId neighbourNode = (nodeA == startNodeId) ? nodeB : nodeA;

    uint16_t best = 1;

    std::vector<EdgeId> adjEdges = Board::Node::getAdjacentEdges(board->nodes[neighbourNode]);
    for (EdgeId adjEdgeId : adjEdges) {
        if (adjEdgeId == edgeId) continue;

        NodeId adjNodeA = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 0);
        NodeId adjNodeB = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 1);
        NodeId adjNeighbourNode = (adjNodeA == neighbourNode) ? adjNodeB : adjNodeA;

        if (adjNeighbourNode >= NODE_COUNT) continue;
        if (Board::Node::unpackStructure(board->nodes[adjNeighbourNode]) != StructureType::NoStructure) continue;

        best = std::max<uint16_t>(best, placementNodeScore(adjNeighbourNode, board, missingTargetMask));
    }

    return best;
}

uint8_t computeCoveredTargetResources(Board::BoardState* board, PlayerId playerId) {
    uint8_t covered = 0;

    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        const auto s = Board::Node::unpackStructure(board->nodes[nodeId]);
        const auto owner = Board::Node::unpackOwner(board->nodes[nodeId]);
        if (owner != playerId) continue;
        if (s != StructureType::Settlement && s != StructureType::City) continue;

        covered |= static_cast<uint8_t>(nodeResourceMask(nodeId, board) & kTargetMask);
    }

    return covered;
}

std::pair<Action::PackedAction, Action::PackedAction> pickInitialPlacement(
    Board::BoardState* board,
    PlayerId playerId,
    const std::vector<Action::PackedAction>& actions
) {
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const uint8_t alreadyCovered = computeCoveredTargetResources(board, playerId);
    const uint8_t missingMask = static_cast<uint8_t>(kTargetMask & ~alreadyCovered);

    NodeId bestNode = Action::unpackArg1(actions[0]);
    uint16_t bestNodeScore = 0;

    for (auto action : actions) {
        const NodeId nodeId = Action::unpackArg1(action);
        const uint16_t s = placementNodeScore(nodeId, board, missingMask);
        if (s > bestNodeScore) {
            bestNodeScore = s;
            bestNode = nodeId;
        }
    }

    const uint8_t bestNodeResMask = static_cast<uint8_t>(nodeResourceMask(bestNode, board) & kTargetMask);
    const uint8_t missingAfter = static_cast<uint8_t>(missingMask & ~bestNodeResMask);

    Action::PackedAction bestAction = actions[0];
    uint16_t bestEdgeScore = 0;

    for (auto action : actions) {
        const NodeId nodeId = Action::unpackArg1(action);
        if (nodeId != bestNode) continue;

        const EdgeId edgeId = Action::unpackArg2(action);
        const uint16_t s = roadEdgeScoreFromNode(bestNode, edgeId, board, missingAfter);
        if (s > bestEdgeScore) {
            bestEdgeScore = s;
            bestAction = action;
        }
    }

    return {bestAction, bestAction};
}

uint8_t pipHexScore(HexId hexId, Board::BoardState* board) {
    const auto hex = board->hexes[hexId];
    const Resource resource = Board::Hex::unpackResource(hex);
    const uint8_t number = Board::Hex::unpackCatanNumber(hex);

    const auto current = board->currentPlayer;
    const auto enemy = (current == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const uint8_t personalValue = Player::unpackResource(
        board->packedPlayers[static_cast<uint8_t>(current)],
        resource
    );
    if (personalValue > 0) return 0;

    const uint8_t enemyValue = Player::unpackResource(
        board->packedPlayers[static_cast<uint8_t>(enemy)],
        resource
    );

    return static_cast<uint8_t>(pipWeight[number] * 2 + enemyValue);
}

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> SettlePrioGreedyPlayer::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    return pickInitialPlacement(boardState, boardState->currentPlayer, actions);
}

std::pair<Action::PackedAction, Action::PackedAction> SettlePrioGreedyPlayer::get2InitialPlacement() {
    auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    return pickInitialPlacement(boardState, boardState->currentPlayer, actions);
}

Action::PackedAction SettlePrioGreedyPlayer::getDevAction() {
    return Action::getEmptyAction();
}

Action::PackedAction SettlePrioGreedyPlayer::getDiscardAction() {
    Action::PackedAction action = Action::getEmptyAction();

    const uint8_t totalResources = Player::totalResources(
        boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)]
    );
    const uint8_t toDiscard = totalResources / 2;
    uint8_t discarded = 0;

    std::vector<Resource> priority = {
        Resource::Wool,
        Resource::Brick,
        Resource::Lumber,
        Resource::Grain,
        Resource::Ore,
    };

    for (Resource r : priority) {
        const uint8_t count = Player::unpackResource(
            boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)],
            r
        );
        const uint8_t toGive = std::min<uint8_t>(
            static_cast<uint8_t>(count / 2 + 1),
            static_cast<uint8_t>(toDiscard - discarded)
        );
        action = Action::packResource(action, r, toGive);
        discarded = static_cast<uint8_t>(discarded + toGive);
        if (discarded >= toDiscard) break;
    }

    for (Resource r : priority) {
        if (discarded >= toDiscard) break;

        const uint8_t currentAmount = Action::unpackResource(action, r);
        const uint8_t available = static_cast<uint8_t>(
            Player::unpackResource(
                boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)],
                r
            ) - currentAmount
        );
        const uint8_t toGive = std::min<uint8_t>(
            available,
            static_cast<uint8_t>(toDiscard - discarded)
        );

        action = Action::packResource(action, r, static_cast<uint8_t>(currentAmount + toGive));
        discarded = static_cast<uint8_t>(discarded + toGive);
    }

    return action;
}

Action::PackedAction SettlePrioGreedyPlayer::getMoveRobber() {
    auto actions = boardState->generateMoveRobberActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }

    Action::PackedAction bestAction = actions[0];
    uint8_t bestScore = 0;

    for (auto action : actions) {
        const HexId hexId = Action::unpackArg1(action);
        const uint8_t s = pipHexScore(hexId, boardState);
        if (s > bestScore) {
            bestScore = s;
            bestAction = action;
        }
    }

    return bestAction;
}

Action::PackedAction SettlePrioGreedyPlayer::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }

    std::vector<Action::PackedAction> structureActions;
    std::vector<Action::PackedAction> roadActions;
    std::vector<Action::PackedAction> devActions;
    Action::PackedAction endTurn = Action::getEmptyAction();

    for (auto a : actions) {
        const auto t = Action::unpackType(a);
        switch (t) {
        case ActionType::BuildSettlement:
        case ActionType::BuildCity:
            structureActions.push_back(a);
            break;
        case ActionType::BuildRoad:
            roadActions.push_back(a);
            break;
        case ActionType::BuyDevCard:
            devActions.push_back(a);
            break;
        case ActionType::EndTurn:
            endTurn = a;
            break;
        case ActionType::TradeBank:
            break; // never trade
        default:
            break;
        }
    }

    if (!structureActions.empty()) {
        const uint8_t covered = computeCoveredTargetResources(boardState, boardState->currentPlayer);
        const uint8_t missing = static_cast<uint8_t>(kTargetMask & ~covered);

        Action::PackedAction best = structureActions[0];
        uint16_t bestScore = 0;

        for (auto a : structureActions) {
            const NodeId nodeId = Action::unpackArg1(a);
            uint16_t s = placementNodeScore(nodeId, boardState, missing);
            if (Action::unpackType(a) == ActionType::BuildCity) {
                s = static_cast<uint16_t>(s + 30);
            }
            if (s > bestScore) {
                bestScore = s;
                best = a;
            }
        }

        return best;
    }

    if (!roadActions.empty()) {
        const uint8_t covered = computeCoveredTargetResources(boardState, boardState->currentPlayer);
        const uint8_t missing = static_cast<uint8_t>(kTargetMask & ~covered);

        Action::PackedAction best = roadActions[0];
        uint16_t bestScore = 0;

        for (auto a : roadActions) {
            const EdgeId edgeId = Action::unpackArg1(a);

            NodeId nodeA = Board::Edge::unpackAdjacentNode(boardState->edges[edgeId], 0);
            NodeId nodeB = Board::Edge::unpackAdjacentNode(boardState->edges[edgeId], 1);

            uint16_t sA = (nodeA < NODE_COUNT) ? placementNodeScore(nodeA, boardState, missing) : 0;
            uint16_t sB = (nodeB < NODE_COUNT) ? placementNodeScore(nodeB, boardState, missing) : 0;

            const uint16_t s = std::max(sA, sB);
            if (s > bestScore) {
                bestScore = s;
                best = a;
            }
        }

        return best;
    }

    if (!devActions.empty()) {
        return devActions[0];
    }

    return endTurn;
}
