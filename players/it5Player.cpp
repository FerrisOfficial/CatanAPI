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

std::array<uint8_t, 5> unpack_resources(Player::PackedPlayer p) {
    return {
        Player::unpackResource(p, Resource::Brick),
        Player::unpackResource(p, Resource::Lumber),
        Player::unpackResource(p, Resource::Wool),
        Player::unpackResource(p, Resource::Grain),
        Player::unpackResource(p, Resource::Ore),
    };
}

uint8_t hand_count(const std::array<uint8_t, 5>& have) {
    uint8_t s = 0;
    for (auto v : have) s = static_cast<uint8_t>(s + v);
    return s;
}

std::array<uint8_t, 5> cost_for(BuyableType b) {
    const auto& c = StructureCost[static_cast<size_t>(b)];
    return {c[0], c[1], c[2], c[3], c[4]};
}

uint16_t deficit(const std::array<uint8_t, 5>& have, const std::array<uint8_t, 5>& need) {
    uint16_t d = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i]) d = static_cast<uint16_t>(d + (need[i] - have[i]));
    }
    return d;
}

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

int spend_reduction_for_action(Action::PackedAction a) {
    switch (Action::unpackType(a)) {
        case ActionType::BuildCity: return 5;
        case ActionType::BuildSettlement: return 4;
        case ActionType::BuyDevCard: return 3;
        case ActionType::BuildRoad: return 2;
        case ActionType::TradeBank: {
            const uint8_t ratio = Action::unpackArg3(a);
            if (ratio < 2) return 0;
            return static_cast<int>(ratio) - 1;
        }
        default:
            return 0;
    }
}

uint8_t dice_pips(uint8_t diceNumber) {
    switch (diceNumber) {
        case 2:  return 1;
        case 3:  return 2;
        case 4:  return 3;
        case 5:  return 4;
        case 6:  return 5;
        case 8:  return 5;
        case 9:  return 4;
        case 10: return 3;
        case 11: return 2;
        case 12: return 1;
        default: return 0;
    }
}

int node_production_score(const Board::BoardState* board, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    auto res_weight = [](Resource r) -> int {
        switch (r) {
            case Resource::Ore: return 14;
            case Resource::Grain: return 13;
            case Resource::Brick: return 12;
            case Resource::Lumber: return 12;
            case Resource::Wool: return 11;
            default: return 0;
        }
    };

    int s = 0;
    for (int i = 0; i < 3; ++i) {
        const HexId h = Board::Node::unpackAdjacentHex(node, i);
        if (h == HexIdNone || h >= HEX_COUNT) continue;
        const auto hex = board->hexes[h];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;
        const uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
        s += static_cast<int>(p) * res_weight(r);
    }

    const auto pt = Board::Node::unpackPortType(node);
    if (pt != PortType::NoPort) {
        if (pt == PortType::ThreeForOne) s += 35;
        else s += 70;
    }

    return s;
}

int production_score_for_player(const Board::BoardState* board, PlayerId pid) {
    int s = 0;
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = board->nodes[n];
        if (Board::Node::unpackOwner(node) != pid) continue;
        const auto st = Board::Node::unpackStructure(node);
        if (st == StructureType::Settlement) {
            s += node_production_score(board, n);
        } else if (st == StructureType::City) {
            s += 2 * node_production_score(board, n);
        }
    }
    return s;
}

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

bool node_distance_rule_ok(const Board::BoardState* board, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return false;
    const auto node = board->nodes[nodeId];
    if (Board::Node::unpackStructure(node) != StructureType::NoStructure) return false;

    for (int i = 0; i < 3; ++i) {
        const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
        if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
        const auto edge = board->edges[e];
        const NodeId a = Board::Edge::unpackAdjacentNode(edge, 0);
        const NodeId b = Board::Edge::unpackAdjacentNode(edge, 1);
        for (NodeId adj : {a, b}) {
            if (adj == nodeId || adj >= NODE_COUNT) continue;
            const auto st = Board::Node::unpackStructure(board->nodes[adj]);
            if (st == StructureType::Settlement || st == StructureType::City) return false;
        }
    }

    return true;
}

bool node_is_adjacent_to_own_road(const Board::BoardState* board, PlayerId pid, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return false;
    const auto node = board->nodes[nodeId];
    for (uint8_t i = 0; i < 3; ++i) {
        const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
        if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
        if (!Board::Edge::unpackHasRoad(board->edges[e])) continue;
        if (Board::Edge::unpackOwner(board->edges[e]) != pid) continue;
        return true;
    }
    return false;
}

int settlement_potential_score(const Board::BoardState* board, PlayerId pid) {
    // Approximate how good our next settlement could be from current road network.
    // Sum top 3 reachable candidate nodes.
    std::array<int, 3> best = {0, 0, 0};

    const auto packed = board->packedPlayers[static_cast<uint8_t>(pid)];
    if (Player::unpackAvailableStructures(packed, StructureType::Settlement) == 0) return 0;

    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        if (!node_distance_rule_ok(board, n)) continue;
        if (!node_is_adjacent_to_own_road(board, pid, n)) continue;

        const int sc = node_production_score(board, n);
        if (sc > best[0]) {
            best[2] = best[1];
            best[1] = best[0];
            best[0] = sc;
        } else if (sc > best[1]) {
            best[2] = best[1];
            best[1] = sc;
        } else if (sc > best[2]) {
            best[2] = sc;
        }
    }

    return best[0] + best[1] + best[2];
}

int evaluate_position(const Board::BoardState* board, PlayerId selfId) {
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked = board->packedPlayers[static_cast<uint8_t>(enemyId)];

    const int selfVP = effective_vp(board, selfId);
    const int enemyVP = effective_vp(board, enemyId);
    const int vpTerm = 50000 * (selfVP - enemyVP);

    const int prodTerm = 35 * (production_score_for_player(board, selfId) - production_score_for_player(board, enemyId));
    const int potTerm = 12 * (settlement_potential_score(board, selfId) - settlement_potential_score(board, enemyId));

    const auto selfHave = unpack_resources(selfPacked);
    const auto enemyHave = unpack_resources(enemyPacked);
    const int selfHand = static_cast<int>(hand_count(selfHave));
    const int enemyHand = static_cast<int>(hand_count(enemyHave));

    // Prefer being closer to building cities/settlements; bank trades should be pulled by this.
    const int cityDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::City)));
    const int settleDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::Settlement)));
    const int devDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::DevCard)));
    const int deficitTerm = -2200 * cityDef - 1600 * settleDef - 650 * devDef;

    // Mild hand-size risk management (discard triggers at 10+ when 7 is rolled).
    const int overLimit = std::max(0, selfHand - 9);
    const int riskTerm = -180 * overLimit;

    const int resTerm = 60 * (selfHand - enemyHand);

    const int devTerm = 250 * (static_cast<int>(Player::totalDevCards(selfPacked)) - static_cast<int>(Player::totalDevCards(enemyPacked)));
    const int roadLenTerm = 200 * (static_cast<int>(Player::unpackLongestRoadLength(selfPacked)) - static_cast<int>(Player::unpackLongestRoadLength(enemyPacked)));

    return vpTerm + prodTerm + potTerm + deficitTerm + riskTerm + resTerm + devTerm + roadLenTerm;
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

} // namespace

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

    auto is_safe_to_simulate = [](Action::PackedAction a) {
        switch (Action::unpackType(a)) {
            case ActionType::BuildCity:
            case ActionType::BuildSettlement:
            case ActionType::BuildRoad:
            case ActionType::TradeBank:
            case ActionType::EndTurn:
                return true;
            default:
                // Avoid applying actions with RNG side-effects (e.g., BuyDevCard, StealResource).
                return false;
        }
    };

    auto consider = [&](Action::PackedAction a, int extraBonus = 0) {
        if (!is_safe_to_simulate(a)) return;
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
