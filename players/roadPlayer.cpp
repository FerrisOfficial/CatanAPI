#include "roadPlayer.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include "playerHelpers.hpp"

namespace {

using PlayerHelpers::cost_for;
using PlayerHelpers::dice_pips;
using PlayerHelpers::effective_vp;
using PlayerHelpers::evaluate_position;
using PlayerHelpers::hand_count;
using PlayerHelpers::is_deterministic_action;
using PlayerHelpers::node_distance_rule_ok;
using PlayerHelpers::node_production_score;
using PlayerHelpers::unpack_resources;

uint8_t resource_weight_road_early(Resource r) {
    // Road-focused opening: ignore ore, prioritize road/settlement resources.
    switch (r) {
        case Resource::Brick:
            return 7;
        case Resource::Lumber:
            return 7;
        case Resource::Grain:
            return 4;
        case Resource::Wool:
            return 4;
        // Not the focus, but completely ignoring ore makes it hard to pivot
        // into cities/dev.
        case Resource::Ore:
            return 2;
        default:
            return 0;
    }
}

struct PlacementScore {
    int score = std::numeric_limits<int>::min();
    Action::PackedAction action = Action::getEmptyAction();
};

std::array<bool, 5> resources_at_node(const Board::BoardState* board,
                                      NodeId nodeId) {
    std::array<bool, 5> has{false, false, false, false, false};
    if (nodeId >= NODE_COUNT) return has;

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

int score_settlement_node_road(const Board::BoardState* board, NodeId nodeId,
                               bool secondPlacement,
                               const std::array<bool, 5>& firstRes) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    int productionScore = 0;
    std::array<uint8_t, 5> resourceCounts{0, 0, 0, 0, 0};

    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;

        const auto hex = board->hexes[hexId];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;

        const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
        productionScore += static_cast<int>(pips) *
                           static_cast<int>(resource_weight_road_early(r));

        if (static_cast<uint8_t>(r) < 5) {
            resourceCounts[static_cast<size_t>(r)]++;
        }
    }

    int unique = 0;
    for (size_t i = 0; i < 5; ++i) unique += resourceCounts[i] > 0 ? 1 : 0;
    int diversityBonus = unique * 20;

    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (resourceCounts[i] > 1) {
            duplicatePenalty += 18 * static_cast<int>(resourceCounts[i] - 1);
        }
    }

    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += secondPlacement ? 25 : 10;
        if (port == PortType::ThreeForOne) portBonus += 12;
        if (port == PortType::BrickPort) portBonus += 18;
        if (port == PortType::LumberPort) portBonus += 18;
    }

    int complementBonus = 0;
    if (secondPlacement) {
        for (size_t i = 0; i < 5; ++i) {
            if (resourceCounts[i] > 0 && !firstRes[i]) {
                complementBonus += 14;
            }
        }
        // Strongly ensure at least one of {Brick, Lumber}.
        if (!firstRes[static_cast<size_t>(Resource::Brick)] &&
            resourceCounts[static_cast<size_t>(Resource::Brick)] > 0) {
            complementBonus += 20;
        }
        if (!firstRes[static_cast<size_t>(Resource::Lumber)] &&
            resourceCounts[static_cast<size_t>(Resource::Lumber)] > 0) {
            complementBonus += 20;
        }
    }

    return productionScore + diversityBonus + portBonus + complementBonus -
           duplicatePenalty;
}

PlacementScore pick_best_road_placement(
    const Board::BoardState* board,
    const std::vector<Action::PackedAction>& actions, bool secondPlacement,
    const std::array<bool, 5>& firstRes) {
    PlacementScore best;

    for (const auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);
        if (nodeId >= NODE_COUNT) continue;
        if (edgeId == EdgeIdNone) continue;

        int s = score_settlement_node_road(board, nodeId, secondPlacement,
                                           firstRes);

        // Prefer roads that lead to a node with higher degree (more expansion
        // options).
        NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
        NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
        const NodeId other = (n0 == nodeId) ? n1 : n0;
        if (other < NODE_COUNT) {
            const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
            int deg = 0;
            for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
            s += 2 * deg;
        }

        if (s > best.score) {
            best.score = s;
            best.action = a;
        }
    }

    if (best.score == std::numeric_limits<int>::min()) {
        best.action =
            actions.empty() ? Action::getEmptyAction() : actions.front();
        best.score = 0;
    }

    return best;
}

int edge_network_score(const Board::BoardState* board, PlayerId selfId,
                       EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return -10000;

    int s = 0;

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);

    const auto score_node = [&](NodeId n) {
        if (n >= NODE_COUNT) return;
        const auto node = board->nodes[n];
        const auto owner = Board::Node::unpackOwner(node);
        const auto st = Board::Node::unpackStructure(node);

        if (owner == selfId &&
            (st == StructureType::Settlement || st == StructureType::City)) {
            s += 200;
        }

        for (uint8_t i = 0; i < 3; ++i) {
            const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
            if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
            if (Board::Edge::unpackHasRoad(board->edges[e]) &&
                Board::Edge::unpackOwner(board->edges[e]) == selfId) {
                s += 120;
                break;
            }
        }
    };

    score_node(n0);
    score_node(n1);

    if (n0 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n0]) != PortType::NoPort)
            s += 10;
    }
    if (n1 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n1]) != PortType::NoPort)
            s += 10;
    }

    return s;
}

int road_action_score(const Board::BoardState* board, PlayerId selfId,
                      EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT)
        return std::numeric_limits<int>::min();

    int s = edge_network_score(board, selfId, edgeId);

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    for (NodeId n : {n0, n1}) {
        if (!node_distance_rule_ok(board, n)) continue;
        s += 3 * node_production_score(board, n);
    }

    return s;
}

std::array<int, 5> road_keep_weights() {
    // Strongly keep Brick/Lumber to enable road spam.
    return {
        24,  // Brick
        24,  // Lumber
        10,  // Wool
        10,  // Grain
        2,   // Ore
    };
}

}  // namespace

std::pair<Action::PackedAction, Action::PackedAction>
RoadPlayer::getInitialPlacement() {
    auto actions =
        boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    firstPlacementResources = {false, false, false, false, false};

    const auto best = pick_best_road_placement(boardState, actions, false,
                                               firstPlacementResources);
    const NodeId nodeId = Action::unpackArg1(best.action);
    firstPlacementResources = resources_at_node(boardState, nodeId);

    return {best.action, best.action};
}

std::pair<Action::PackedAction, Action::PackedAction>
RoadPlayer::get2InitialPlacement() {
    auto actions =
        boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const auto best = pick_best_road_placement(boardState, actions, true,
                                               firstPlacementResources);
    return {best.action, best.action};
}

Action::PackedAction RoadPlayer::getDiscardAction() {
    const PlayerId selfId = boardState->currentPlayer;
    const auto packed = boardState->packedPlayers[static_cast<uint8_t>(selfId)];

    auto have = unpack_resources(packed);
    const uint8_t total = hand_count(have);
    if (total <= 9) return Action::getEmptyAction();

    const uint8_t toDiscard = static_cast<uint8_t>(total / 2);

    const auto need = cost_for(BuyableType::Road);
    const auto baseW = road_keep_weights();

    Action::PackedAction action = Action::getEmptyAction();

    uint8_t remaining = toDiscard;
    while (remaining > 0) {
        int best = std::numeric_limits<int>::max();
        int bestIdx = -1;

        for (int i = 0; i < 5; ++i) {
            if (have[static_cast<size_t>(i)] == 0) continue;

            const bool neededForRoad =
                have[static_cast<size_t>(i)] <= need[static_cast<size_t>(i)];
            int discardCost = baseW[static_cast<size_t>(i)];
            if (neededForRoad) discardCost += 600;

            if (discardCost < best) {
                best = discardCost;
                bestIdx = i;
            }
        }

        if (bestIdx < 0) break;

        const auto r = static_cast<Resource>(bestIdx);
        const uint8_t current = Action::unpackResource(action, r);
        action =
            Action::packResource(action, r, static_cast<uint8_t>(current + 1));
        have[static_cast<size_t>(bestIdx)]--;
        remaining--;
    }

    return action;
}

Action::PackedAction RoadPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    const int baseScore = evaluate_position(boardState, selfId);
    const auto basePacked =
        boardState->packedPlayers[static_cast<uint8_t>(selfId)];
    const int baseLen = Player::unpackLongestRoadLength(basePacked);
    const bool baseHasAward = Player::unpackLongestRoadFlag(basePacked);

    const PlayerId enemyId =
        (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    const int selfVP = effective_vp(boardState, selfId);
    const int enemyVP = effective_vp(boardState, enemyId);

    Action::PackedAction best = Action::getEmptyAction();
    int bestScore = std::numeric_limits<int>::min();

    auto consider = [&](Action::PackedAction a, int extraBonus = 0) {
        if (!is_deterministic_action(a)) return;

        boardState->applyAction(a);
        int s = evaluate_position(boardState, selfId) + extraBonus;

        // If this action immediately enables a city/settlement build next,
        // reward it. (Keeps It5's strong conversion behavior.)
        if (Action::unpackType(a) == ActionType::TradeBank ||
            Action::unpackType(a) == ActionType::BuildRoad) {
            auto next = boardState->getLegalActions(selfId);
            bool canCity = false;
            bool canSettle = false;
            for (const auto na : next) {
                if (Action::unpackType(na) == ActionType::BuildCity)
                    canCity = true;
                if (Action::unpackType(na) == ActionType::BuildSettlement)
                    canSettle = true;
            }
            if (canCity) s += 8000;
            if (canSettle) s += 5000;
        }

        // Mild road bias: prefer roads that grow/secure a continuous network.
        if (Action::unpackType(a) == ActionType::BuildRoad) {
            const auto packed =
                boardState->packedPlayers[static_cast<uint8_t>(selfId)];
            const int len = Player::unpackLongestRoadLength(packed);
            const bool hasAward = Player::unpackLongestRoadFlag(packed);
            const int deltaLen = len - baseLen;

            s += deltaLen * 18000;
            s += len * 2500;
            if (hasAward) s += 9000;
            if (!baseHasAward && hasAward) s += 12000;

            const EdgeId e = Action::unpackArg1(a);
            s += road_action_score(boardState, selfId, e) * 8;
        }

        boardState->undoLastAction();

        if (s > bestScore) {
            bestScore = s;
            best = a;
        }
    };

    // 1) If we can build a city/settlement, search best by simulation.
    for (const auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::BuildCity || t == ActionType::BuildSettlement) {
            consider(a);
        }
    }
    if (Action::unpackType(best) == ActionType::BuildCity ||
        Action::unpackType(best) == ActionType::BuildSettlement) {
        return best;
    }

    // 2) Evaluate all deterministic actions (roads/trades/end turn).
    for (const auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::BuildRoad || t == ActionType::TradeBank ||
            t == ActionType::EndTurn) {
            consider(a);
        }
    }

    // 3) Heuristic for dev-card buying without simulating (it is RNG).
    Action::PackedAction devBuy = Action::getEmptyAction();
    bool hasRoad = false;
    bool hasTrade = false;
    for (const auto a : actions) {
        if (Action::unpackType(a) == ActionType::BuyDevCard) devBuy = a;
        if (Action::unpackType(a) == ActionType::BuildRoad) hasRoad = true;
        if (Action::unpackType(a) == ActionType::TradeBank) hasTrade = true;
    }

    if (Action::unpackType(devBuy) == ActionType::BuyDevCard) {
        int devScore = baseScore + 1500;
        if (enemyVP > selfVP) devScore += 2500;
        if (selfVP >= 8) devScore -= 500;
        if (hasTrade) devScore -= 500;
        if (hasRoad) devScore -= 200;

        if (devScore > bestScore) {
            bestScore = devScore;
            best = devBuy;
        }
    }

    // If all else fails, fall back to It4.
    if (Action::unpackType(best) == ActionType::NoAction) {
        return It4Player::getTurnAction();
    }

    // Avoid choosing EndTurn when it doesn't improve the evaluation.
    if (Action::unpackType(best) == ActionType::EndTurn &&
        bestScore < baseScore) {
        return It4Player::getTurnAction();
    }

    return best;
}
