#include "it3Player.hpp"
#include "playerHelpers.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace {

using PlayerHelpers::dice_pips;

uint8_t resource_weight_early(Resource r) {
    // Small bias towards early expansion resources.
    switch (r) {
        case Resource::Brick:  return 5;
        case Resource::Lumber: return 5;
        case Resource::Grain:  return 4;
        case Resource::Wool:   return 4;
        case Resource::Ore:    return 3;
        default:               return 0;
    }
}

struct PlacementScore {
    int score = std::numeric_limits<int>::min();
    Action::PackedAction action = Action::getEmptyAction();
};

std::array<bool, 5> resources_at_node(const Board::BoardState* board, NodeId nodeId) {
    std::array<bool, 5> has {false, false, false, false, false};
    const auto node = board->nodes[nodeId];
    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
        const Resource r = Board::Hex::unpackResource(board->hexes[hexId]);
        if (r == Resource::NoResource) continue;
        if (static_cast<uint8_t>(r) < 5) {
            has[static_cast<size_t>(r)] = true;
        }
    }
    return has;
}

int score_settlement_node(const Board::BoardState* board, NodeId nodeId, bool secondPlacement,
                          const std::array<bool, 5>& firstRes) {
    const auto node = board->nodes[nodeId];

    // Expected production (pips * structure resource weight).
    int productionScore = 0;
    std::array<uint8_t, 5> resourceCounts {0, 0, 0, 0, 0};

    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;

        const auto hex = board->hexes[hexId];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;

        const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
        productionScore += static_cast<int>(pips) * static_cast<int>(resource_weight_early(r));

        if (static_cast<uint8_t>(r) < 5) {
            resourceCounts[static_cast<size_t>(r)]++;
        }
    }

    // Diversity bonus (prefer covering more different resources).
    int unique = 0;
    for (size_t i = 0; i < 5; ++i) {
        unique += resourceCounts[i] > 0 ? 1 : 0;
    }
    int diversityBonus = unique * 25;

    // Penalty for duplicates on the same node (e.g., double lumber).
    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (resourceCounts[i] > 1) {
            duplicatePenalty += 20 * static_cast<int>(resourceCounts[i] - 1);
        }
    }

    // Port bonus: modest in first placement, stronger in second.
    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += secondPlacement ? 25 : 15;
        // Extra small bonus for 3:1 (generic) and for matching resource ports.
        if (port == PortType::ThreeForOne) portBonus += 10;
        if (port == PortType::BrickPort)  portBonus += 5;
        if (port == PortType::LumberPort) portBonus += 5;
        if (port == PortType::WoolPort)   portBonus += 5;
        if (port == PortType::GrainPort)  portBonus += 5;
        if (port == PortType::OrePort)    portBonus += 5;
    }

    // Second placement: bonus for covering resources we don't have yet.
    int complementBonus = 0;
    if (secondPlacement) {
        for (size_t i = 0; i < 5; ++i) {
            if (resourceCounts[i] > 0 && !firstRes[i]) {
                complementBonus += 18;
            }
        }
        // Slightly prioritize getting at least one of {Brick,Lumber} if missing.
        if (!firstRes[static_cast<size_t>(Resource::Brick)] && resourceCounts[static_cast<size_t>(Resource::Brick)] > 0) {
            complementBonus += 10;
        }
        if (!firstRes[static_cast<size_t>(Resource::Lumber)] && resourceCounts[static_cast<size_t>(Resource::Lumber)] > 0) {
            complementBonus += 10;
        }
    }

    return productionScore + diversityBonus + portBonus + complementBonus - duplicatePenalty;
}

PlacementScore pick_best_placement(const Board::BoardState* board,
                                  const std::vector<Action::PackedAction>& actions,
                                  bool secondPlacement,
                                  const std::array<bool, 5>& firstRes) {
    PlacementScore best;

    for (const auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);

        // Basic validity: ignore placeholder/sentinel ids.
        if (nodeId >= NODE_COUNT) continue;
        if (edgeId == EdgeIdNone) continue;

        int s = score_settlement_node(board, nodeId, secondPlacement, firstRes);

        // Tiny bonus if the road leads away from the board edge (more options later):
        // prefer roads that connect to a node with 3 adjacent edges.
        NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
        NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
        const NodeId other = (n0 == nodeId) ? n1 : n0;
        if (other < NODE_COUNT) {
            const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
            int deg = 0;
            for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
            s += deg; // 1..3
        }

        if (s > best.score) {
            best.score = s;
            best.action = a;
        }
    }

    if (best.score == std::numeric_limits<int>::min()) {
        // Fallback: first action if everything was filtered.
        best.action = actions.empty() ? Action::getEmptyAction() : actions.front();
        best.score = 0;
    }

    return best;
}

Action::PackedAction pick_best_robber_move(const Board::BoardState* board, PlayerId selfId,
                                          const std::vector<Action::PackedAction>& actions) {
    if (actions.empty()) return Action::getEmptyAction();

    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    int bestScore = std::numeric_limits<int>::min();
    Action::PackedAction best = actions.front();

    for (const auto a : actions) {
        const HexId hexId = Action::unpackArg1(a);
        if (hexId >= HEX_COUNT) continue;

        const auto hex = board->hexes[hexId];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) {
            // Usually desert; almost never worth placing robber here.
            continue;
        }

        const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
        const uint8_t selfVal = Board::Hex::unpackPlayerValue(hex, selfId);
        const uint8_t enemyVal = Board::Hex::unpackPlayerValue(hex, enemyId);

        // Prefer hurting enemy production, avoid blocking ourselves.
        // Strongly prefer hexes where enemy has buildings and we don't.
        int s = 0;
        s += static_cast<int>(pips) * 30 * static_cast<int>(enemyVal);
        s -= static_cast<int>(pips) * 45 * static_cast<int>(selfVal);

        if (enemyVal > 0 && selfVal == 0) s += 500;
        if (enemyVal == 0) s -= 200;

        // Slight preference for high-frequency numbers even when values equal.
        s += static_cast<int>(pips) * 2;

        if (s > bestScore) {
            bestScore = s;
            best = a;
        }
    }

    // If we filtered out everything (e.g., all deserts?), fall back to random-ish first.
    if (bestScore == std::numeric_limits<int>::min()) {
        return actions.front();
    }
    return best;
}

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> It3Player::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    // Clear state for a new game/setup.
    firstSettlementNode = 0xFF;
    firstPlacementResources = {false, false, false, false, false};

    const auto best = pick_best_placement(boardState, actions, false, firstPlacementResources);

    firstSettlementNode = Action::unpackArg1(best.action);
    firstPlacementResources = resources_at_node(boardState, firstSettlementNode);

    return {best.action, best.action};
}

std::pair<Action::PackedAction, Action::PackedAction> It3Player::get2InitialPlacement() {
    auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    // If something went odd (e.g., getInitialPlacement not called), keep the heuristic safe.
    const auto best = pick_best_placement(boardState, actions, true, firstPlacementResources);
    return {best.action, best.action};
}

Action::PackedAction It3Player::getMoveRobber() {
    auto actions = boardState->generateMoveRobberActions(boardState->currentPlayer);
    return pick_best_robber_move(boardState, boardState->currentPlayer, actions);
}
