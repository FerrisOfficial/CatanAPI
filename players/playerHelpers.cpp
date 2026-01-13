#include "playerHelpers.hpp"

#include "../game_simulation/board.hpp"
#include "../game_simulation/player.hpp"

#include <algorithm>
#include <limits>

namespace PlayerHelpers {

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
        default: return 0; // includes 7 and invalid
    }
}

int effective_vp(const Board::BoardState* board, PlayerId pid) {
    const auto p = board->packedPlayers[static_cast<uint8_t>(pid)];
    int vp = static_cast<int>(Player::unpackVictoryPoints(p));
    if (Player::unpackLargestArmyFlag(p)) vp += 2;
    if (Player::unpackLongestRoadFlag(p)) vp += 2;
    return vp;
}

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

bool is_deterministic_action(Action::PackedAction a) {
    switch (Action::unpackType(a)) {
        case ActionType::BuildCity:
        case ActionType::BuildSettlement:
        case ActionType::BuildRoad:
        case ActionType::TradeBank:
        case ActionType::EndTurn:
            return true;
        default:
            // Avoid actions with RNG side-effects (e.g., BuyDevCard, StealResource)
            return false;
    }
}

} // namespace PlayerHelpers
