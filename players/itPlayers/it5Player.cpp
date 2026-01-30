#include "it5Player.hpp"
#include "playerHelpers.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using PlayerHelpers::effective_vp;
using PlayerHelpers::unpack_resources;
using PlayerHelpers::hand_count;
using PlayerHelpers::cost_for;
using PlayerHelpers::deficit;
using PlayerHelpers::node_production_score;
using PlayerHelpers::production_score_for_player;
using PlayerHelpers::node_distance_rule_ok;
using PlayerHelpers::node_is_adjacent_to_own_road;
using PlayerHelpers::settlement_potential_score;
using PlayerHelpers::evaluate_position;
using PlayerHelpers::is_deterministic_action;

int priority_index(BuyableType b) {
    switch (b) {
        case BuyableType::City: return 0;
        case BuyableType::Settlement: return 1;
        case BuyableType::Road: return 2;
        case BuyableType::DevCard: return 3;
        default: return 99;
    }
}

BuyableType best_target_for_discard(const std::array<uint8_t, 5>& have) {
    BuyableType best = BuyableType::Road;
    uint16_t bestDef = std::numeric_limits<uint16_t>::max();

    for (BuyableType t : {BuyableType::City, BuyableType::Settlement, BuyableType::Road, BuyableType::DevCard}) {
        const auto need = cost_for(t);
        const auto d = deficit(have, need);
        if (d < bestDef || (d == bestDef && priority_index(t) < priority_index(best))) {
            bestDef = d;
            best = t;
        }
    }

    return best;
}

std::array<int, 5> base_keep_weights_for(BuyableType target) {
    // Base (slight city/dev bias).
    std::array<int, 5> w = {
        10, // Brick
        10, // Lumber
        8,  // Wool
        12, // Grain
        13, // Ore
    };

    auto add = [&](Resource r, int v) {
        w[static_cast<size_t>(r)] += v;
    };

    switch (target) {
        case BuyableType::City:
            add(Resource::Ore, 10);
            add(Resource::Grain, 8);
            break;
        case BuyableType::Settlement:
            add(Resource::Brick, 7);
            add(Resource::Lumber, 7);
            add(Resource::Wool, 6);
            add(Resource::Grain, 6);
            break;
        case BuyableType::Road:
            add(Resource::Brick, 8);
            add(Resource::Lumber, 8);
            break;
        case BuyableType::DevCard:
            add(Resource::Ore, 7);
            add(Resource::Grain, 7);
            add(Resource::Wool, 7);
            break;
        default:
            break;
    }

    return w;
}


} // namespace

int edge_network_score(const Board::BoardState* board, PlayerId selfId, EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return -10000;

    int s = 0;

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);

    const auto score_node = [&](NodeId n) { 
        if (n >= NODE_COUNT) return;
        const auto node = board->nodes[n];
        const auto owner = Board::Node::unpackOwner(node);
        const auto st = Board::Node::unpackStructure(node);

        if (owner == selfId && (st == StructureType::Settlement || st == StructureType::City)) {
            s += 200;
        }

        for (uint8_t i = 0; i < 3; ++i) {
            const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
            if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
            if (Board::Edge::unpackHasRoad(board->edges[e]) && Board::Edge::unpackOwner(board->edges[e]) == selfId) {
                s += 120;
                break;
            }
        }
    };

    score_node(n0);
    score_node(n1);

    if (n0 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n0]) != PortType::NoPort) s += 10;
    }
    if (n1 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n1]) != PortType::NoPort) s += 10;
    }

    return s;
}

int road_action_score(const Board::BoardState* board, PlayerId selfId, EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return std::numeric_limits<int>::min();

    int s = edge_network_score(board, selfId, edgeId);

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    for (NodeId n : {n0, n1}) {
        if (!node_distance_rule_ok(board, n)) continue;
        s += 3 * node_production_score(board, n);
    }

    return s;
}

Action::PackedAction It5Player::getDiscardAction() {
    const PlayerId selfId = boardState->currentPlayer;
    const auto packed = boardState->packedPlayers[static_cast<uint8_t>(selfId)];

    auto have = unpack_resources(packed);
    const uint8_t total = hand_count(have);
    if (total <= 9) return Action::getEmptyAction();

    const uint8_t toDiscard = static_cast<uint8_t>(total / 2);

    const BuyableType target = best_target_for_discard(have);
    const auto need = cost_for(target);
    const auto baseW = base_keep_weights_for(target);

    Action::PackedAction action = Action::getEmptyAction();

    uint8_t remaining = toDiscard;
    while (remaining > 0) {
        int best = std::numeric_limits<int>::max();
        int bestIdx = -1;

        for (int i = 0; i < 5; ++i) {
            if (have[static_cast<size_t>(i)] == 0) continue;

            // Prefer discarding surplus over required parts of the target.
            const bool neededForTarget = have[static_cast<size_t>(i)] <= need[static_cast<size_t>(i)];
            int discardCost = baseW[static_cast<size_t>(i)];
            if (neededForTarget) discardCost += 500;

            if (discardCost < best) {
                best = discardCost;
                bestIdx = i;
            }
        }

        if (bestIdx < 0) break;

        const auto r = static_cast<Resource>(bestIdx);
        const uint8_t current = Action::unpackResource(action, r);
        action = Action::packResource(action, r, static_cast<uint8_t>(current + 1));
        have[static_cast<size_t>(bestIdx)]--;
        remaining--;
    }

    return action;
}

Action::PackedAction It5Player::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    const int baseScore = evaluate_position(boardState, selfId);

    Action::PackedAction best = Action::getEmptyAction();
    int bestScore = std::numeric_limits<int>::min();

    auto consider = [&](Action::PackedAction a, int extraBonus = 0) {
        if (!is_deterministic_action(a)) return;
        boardState->applyAction(a);
        int s = evaluate_position(boardState, selfId) + extraBonus;

        // If this action immediately enables a city/settlement build next, reward it.
        // (This helps trades/roads that unlock a strong build.)
        if (Action::unpackType(a) == ActionType::TradeBank || Action::unpackType(a) == ActionType::BuildRoad) {
            auto next = boardState->getLegalActions(selfId);
            bool canCity = false;
            bool canSettle = false;
            for (const auto na : next) {
                if (Action::unpackType(na) == ActionType::BuildCity) canCity = true;
                if (Action::unpackType(na) == ActionType::BuildSettlement) canSettle = true;
            }
            if (canCity) s += 8000;
            if (canSettle) s += 5000;
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
    if (Action::unpackType(best) == ActionType::BuildCity || Action::unpackType(best) == ActionType::BuildSettlement) {
        return best;
    }

    // 2) Evaluate all deterministic actions (roads/trades/end turn).
    for (const auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::BuildRoad || t == ActionType::TradeBank || t == ActionType::EndTurn) {
            consider(a);
        }
    }

    // 3) Heuristic for dev-card buying without simulating (it is RNG).
    // Prefer dev-buy when it doesn't block an imminent city/settlement and we're not already far ahead.
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    const int selfVP = effective_vp(boardState, selfId);
    const int enemyVP = effective_vp(boardState, enemyId);

    Action::PackedAction devBuy = Action::getEmptyAction();
    bool hasRoad = false;
    bool hasTrade = false;
    for (const auto a : actions) {
        if (Action::unpackType(a) == ActionType::BuyDevCard) devBuy = a;
        if (Action::unpackType(a) == ActionType::BuildRoad) hasRoad = true;
        if (Action::unpackType(a) == ActionType::TradeBank) hasTrade = true;
    }

    if (Action::unpackType(devBuy) == ActionType::BuyDevCard) {
        // If we're behind or midgame and no strong deterministic improvement exists, dev-buy is often good.
        int devScore = baseScore + 1500;
        if (enemyVP > selfVP) devScore += 2500;
        if (selfVP >= 8) devScore -= 500; // late-game: prefer deterministic builds/trades.
        if (hasTrade) devScore -= 500;    // trades can be more targeted.
        if (hasRoad) devScore -= 200;     // roads might open deterministic settlement.

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
    if (Action::unpackType(best) == ActionType::EndTurn && bestScore < baseScore) {
        return It4Player::getTurnAction();
    }

    return best;
}
