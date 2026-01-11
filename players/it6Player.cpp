#include "it6Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

namespace {

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

int effective_vp(const Board::BoardState* board, PlayerId pid) {
    const auto p = board->packedPlayers[static_cast<uint8_t>(pid)];
    int vp = static_cast<int>(Player::unpackVictoryPoints(p));
    if (Player::unpackLargestArmyFlag(p)) vp += 2;
    if (Player::unpackLongestRoadFlag(p)) vp += 2;
    return vp;
}

std::array<double, 5> resource_scarcity_weights(const Board::BoardState* board) {
    // Higher value => scarcer on this board.
    std::array<double, 5> totalPips = {0, 0, 0, 0, 0};

    for (HexId h = 0; h < HEX_COUNT; ++h) {
        const auto hex = board->hexes[h];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;
        const uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
        if (p == 0) continue;
        totalPips[static_cast<size_t>(r)] += static_cast<double>(p);
    }

    // Base importance: grain/ore/brick higher for Cities + expansion.
    const std::array<double, 5> base = {
        1.15, // Brick
        1.05, // Lumber
        0.90, // Wool
        1.35, // Grain
        1.45, // Ore
    };

    std::array<double, 5> w{};
    for (size_t i = 0; i < 5; ++i) {
        const double denom = std::max(1.0, totalPips[i]);
        // Normalize around typical pip totals; the exact scale isn't critical, only ratios.
        w[i] = base[i] * (12.0 / denom);
    }
    return w;
}

struct NodeFeatures {
    int pipSum = 0;
    std::array<int, 5> pipByRes = {0, 0, 0, 0, 0};
    int uniqueRes = 0;
    PortType portType = PortType::NoPort;
};

NodeFeatures node_features(const Board::BoardState* board, NodeId nodeId) {
    NodeFeatures f;
    if (nodeId >= NODE_COUNT) return f;

    const auto node = board->nodes[nodeId];
    f.portType = Board::Node::unpackPortType(node);

    std::array<bool, 5> has = {false, false, false, false, false};
    for (int i = 0; i < 3; ++i) {
        const HexId h = Board::Node::unpackAdjacentHex(node, i);
        if (h == HexIdNone || h >= HEX_COUNT) continue;
        const auto hex = board->hexes[h];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;
        const uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
        f.pipSum += static_cast<int>(p);
        f.pipByRes[static_cast<size_t>(r)] += static_cast<int>(p);
        has[static_cast<size_t>(r)] = true;
    }

    for (bool b : has) if (b) f.uniqueRes += 1;
    return f;
}

int node_production_score(const Board::BoardState* board, NodeId nodeId, const std::array<double, 5>& scarcityW) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();

    const auto nf = node_features(board, nodeId);

    // Production weighted by scarcity.
    double prod = 0.0;
    for (size_t i = 0; i < 5; ++i) {
        prod += static_cast<double>(nf.pipByRes[i]) * scarcityW[i];
    }

    // City potential: ore+grain clusters.
    const double city = 2.5 * static_cast<double>(nf.pipByRes[static_cast<size_t>(Resource::Ore)]) +
                        2.0 * static_cast<double>(nf.pipByRes[static_cast<size_t>(Resource::Grain)]);

    // Scarcity-driven expansion needs: brick and lumber still matter.
    const double expand = 1.6 * static_cast<double>(nf.pipByRes[static_cast<size_t>(Resource::Brick)]) +
                          1.4 * static_cast<double>(nf.pipByRes[static_cast<size_t>(Resource::Lumber)]);

    // Diversity: 5 ideal; 4 OK; 3 risky.
    const double diversity = 70.0 * static_cast<double>(nf.uniqueRes);

    // Ports only if they compensate for missing resources.
    double portBonus = 0.0;
    if (nf.portType != PortType::NoPort) {
        if (nf.uniqueRes <= 3) {
            portBonus += (nf.portType == PortType::ThreeForOne) ? 120.0 : 180.0;
        } else {
            portBonus += (nf.portType == PortType::ThreeForOne) ? 35.0 : 60.0;
        }
    }

    const double total = 140.0 * prod + 55.0 * city + 25.0 * expand + diversity + portBonus;
    return static_cast<int>(total);
}

int edge_seed_score(const Board::BoardState* board, EdgeId edgeId, const std::array<double, 5>& scarcityW) {
    // For initial road: prefer pointing towards high-quality, legal future settlement nodes.
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return std::numeric_limits<int>::min();

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);

    auto candidate = [&](NodeId n) {
        if (n >= NODE_COUNT) return std::numeric_limits<int>::min();
        // Must be empty and distance-rule legal.
        const auto node = board->nodes[n];
        if (Board::Node::unpackStructure(node) != StructureType::NoStructure) return std::numeric_limits<int>::min();
        for (int i = 0; i < 3; ++i) {
            const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
            if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
            const auto edge = board->edges[e];
            const NodeId a = Board::Edge::unpackAdjacentNode(edge, 0);
            const NodeId b = Board::Edge::unpackAdjacentNode(edge, 1);
            for (NodeId adj : {a, b}) {
                if (adj == n || adj >= NODE_COUNT) continue;
                const auto st = Board::Node::unpackStructure(board->nodes[adj]);
                if (st == StructureType::Settlement || st == StructureType::City) return std::numeric_limits<int>::min();
            }
        }
        return node_production_score(board, n, scarcityW);
    };

    const int s0 = candidate(n0);
    const int s1 = candidate(n1);

    // Road endpoints nearer to high scoring nodes.
    return std::max(s0, s1);
}

struct PlacementPick {
    Action::PackedAction placement;
    int score;
};

PlacementPick pick_best_initial(const Board::BoardState* board,
                               PlayerId selfId,
                               const std::vector<Action::PackedAction>& placements,
                               const std::array<double, 5>& scarcityW,
                               double denialWeight) {
    PlacementPick best{Action::getEmptyAction(), std::numeric_limits<int>::min()};
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    // This function expects the caller to apply/undo; we keep it pure by not mutating here.
    (void)enemyId;
    (void)denialWeight;

    // Placeholder; actual scoring is done in the mutating wrapper in the It6 methods.
    // (Kept for structure only.)
    if (!placements.empty()) {
        best.placement = placements.front();
        best.score = 0;
    }

    (void)board;
    (void)scarcityW;

    return best;
}

int evaluate_position(const Board::BoardState* board, PlayerId selfId) {
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked = board->packedPlayers[static_cast<uint8_t>(enemyId)];

    const int selfVP = effective_vp(board, selfId);
    const int enemyVP = effective_vp(board, enemyId);

    const auto selfHave = unpack_resources(selfPacked);
    const auto enemyHave = unpack_resources(enemyPacked);

    const int selfHand = static_cast<int>(hand_count(selfHave));
    const int enemyHand = static_cast<int>(hand_count(enemyHave));

    const int cityDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::City)));
    const int settleDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::Settlement)));
    const int devDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::DevCard)));

    const int overLimit = std::max(0, selfHand - 9);

    const int devTerm = 220 * (static_cast<int>(Player::totalDevCards(selfPacked)) - static_cast<int>(Player::totalDevCards(enemyPacked)));
    const int roadTerm = 160 * (static_cast<int>(Player::unpackLongestRoadLength(selfPacked)) - static_cast<int>(Player::unpackLongestRoadLength(enemyPacked)));

    // Big emphasis on VP race (15 VP win).
    int score = 50000 * (selfVP - enemyVP);

    // Plan quality (multi-turn): prefer being closer to City/Settlement/Dev.
    score += -2200 * cityDef - 1500 * settleDef - 650 * devDef;

    // Flex: slight preference for larger hand, but penalize over-limit risk.
    score += 55 * (selfHand - enemyHand) - 220 * overLimit;

    score += devTerm + roadTerm;

    return score;
}

bool is_deterministic_turn_action(Action::PackedAction a) {
    switch (Action::unpackType(a)) {
        case ActionType::BuildCity:
        case ActionType::BuildSettlement:
        case ActionType::BuildRoad:
        case ActionType::TradeBank:
            return true;
        default:
            return false;
    }
}

struct MctsNode {
    int parent = -1;
    Action::PackedAction action = Action::getEmptyAction();
    std::vector<int> children;
    std::vector<Action::PackedAction> untried;
    int visits = 0;
    double valueSum = 0.0;
    int depth = 0;
};

int pick_child_ucb(const std::vector<MctsNode>& nodes, int idx, double c) {
    const auto& n = nodes[idx];
    const double logN = std::log(std::max(1, n.visits));

    int bestChild = -1;
    double best = -1e100;

    for (int ci : n.children) {
        const auto& ch = nodes[ci];
        if (ch.visits == 0) return ci;
        const double mean = ch.valueSum / static_cast<double>(ch.visits);
        const double ucb = mean + c * std::sqrt(logN / static_cast<double>(ch.visits));
        if (ucb > best) {
            best = ucb;
            bestChild = ci;
        }
    }
    return bestChild;
}

Action::PackedAction greedy_rollout_action(const Board::BoardState* board, PlayerId selfId, const std::vector<Action::PackedAction>& legal) {
    // Deterministic greedy: take city/settlement if available, else pick the best-looking road/trade.
    Action::PackedAction best = Action::getEmptyAction();
    int bestS = std::numeric_limits<int>::min();

    for (const auto a : legal) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::EndTurn) continue;
        if (!is_deterministic_turn_action(a)) continue;

        // Quick shallow score: apply/undo outside.
        (void)board;
        (void)selfId;
        int bonus = 0;
        if (t == ActionType::BuildCity) bonus += 8000;
        if (t == ActionType::BuildSettlement) bonus += 5000;
        if (t == ActionType::TradeBank) bonus += 1500;
        if (t == ActionType::BuildRoad) bonus += 900;

        if (bonus > bestS) {
            bestS = bonus;
            best = a;
        }
    }

    return best;
}

Action::PackedAction mcts_pick_action(Board::BoardState* board, PlayerId selfId, int iterations, int maxDepth) {
    auto rootLegal = board->getLegalActions(selfId);

    // Filter for deterministic actions; MCTS plans sequences of these inside a single turn.
    std::vector<Action::PackedAction> rootDet;
    Action::PackedAction endTurn = Action::getEmptyAction();
    Action::PackedAction buyDev = Action::getEmptyAction();

    for (const auto a : rootLegal) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::EndTurn) {
            endTurn = a;
            continue;
        }
        if (t == ActionType::BuyDevCard) {
            buyDev = a;
            continue;
        }
        if (is_deterministic_turn_action(a)) rootDet.push_back(a);
    }

    if (rootDet.empty()) {
        // If only dev-buy exists, take it; otherwise end.
        if (Action::unpackType(buyDev) == ActionType::BuyDevCard) return buyDev;
        return Action::unpackType(endTurn) == ActionType::EndTurn ? endTurn : Action::getEmptyAction();
    }

    const int base = evaluate_position(board, selfId);

    // Heuristic dev-buy choice at root (cannot simulate due to RNG).
    int devBuyHeuristic = std::numeric_limits<int>::min();
    if (Action::unpackType(buyDev) == ActionType::BuyDevCard) {
        const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
        const int selfVP = effective_vp(board, selfId);
        const int enemyVP = effective_vp(board, enemyId);

        // Buying dev reduces hand by 3 (always), which is good if we're near the 9 limit.
        const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
        const auto have = unpack_resources(selfPacked);
        const int hand = static_cast<int>(hand_count(have));
        const int handRelief = std::max(0, hand - 9);

        devBuyHeuristic = base + 2500 + 350 * handRelief;
        if (enemyVP > selfVP) devBuyHeuristic += 2000;
        if (selfVP >= 9) devBuyHeuristic -= 1200;
    }

    std::vector<MctsNode> nodes;
    nodes.reserve(static_cast<size_t>(iterations) * 4);
    nodes.push_back(MctsNode{});
    nodes[0].parent = -1;
    nodes[0].depth = 0;
    nodes[0].untried = rootDet;

    std::mt19937 rng(0xC0FFEEu);
    std::uniform_int_distribution<int> pickUntried;

    const double c = 1.2;

    for (int it = 0; it < iterations; ++it) {
        int nodeIdx = 0;
        int applied = 0;

        // Selection
        while (nodes[nodeIdx].untried.empty() && !nodes[nodeIdx].children.empty() && nodes[nodeIdx].depth < maxDepth) {
            const int child = pick_child_ucb(nodes, nodeIdx, c);
            if (child < 0) break;
            const auto a = nodes[child].action;
            board->applyAction(a);
            applied++;
            nodeIdx = child;
        }

        // Expansion
        if (!nodes[nodeIdx].untried.empty() && nodes[nodeIdx].depth < maxDepth) {
            pickUntried = std::uniform_int_distribution<int>(0, static_cast<int>(nodes[nodeIdx].untried.size()) - 1);
            const int k = pickUntried(rng);
            const auto a = nodes[nodeIdx].untried[static_cast<size_t>(k)];
            nodes[nodeIdx].untried.erase(nodes[nodeIdx].untried.begin() + k);

            board->applyAction(a);
            applied++;

            MctsNode childNode;
            childNode.parent = nodeIdx;
            childNode.action = a;
            childNode.depth = nodes[nodeIdx].depth + 1;

            auto legal = board->getLegalActions(selfId);
            for (const auto la : legal) {
                if (Action::unpackType(la) == ActionType::EndTurn) continue;
                if (is_deterministic_turn_action(la)) childNode.untried.push_back(la);
            }

            const int newIdx = static_cast<int>(nodes.size());
            nodes.push_back(std::move(childNode));
            nodes[nodeIdx].children.push_back(newIdx);
            nodeIdx = newIdx;
        }

        // No rollout: leaf evaluation only (keeps this fast enough to run per action).
        const int score = evaluate_position(board, selfId);
        const double reward = static_cast<double>(score - base);

        // Undo rollout + path
        for (int i = 0; i < applied; ++i) {
            board->undoLastAction();
        }

        // Backprop
        int cur = nodeIdx;
        while (cur >= 0) {
            nodes[cur].visits += 1;
            nodes[cur].valueSum += reward;
            cur = nodes[cur].parent;
        }
    }

    // Choose best child by visit count.
    int bestChild = -1;
    int bestVisits = -1;
    double bestMean = -1e100;

    for (int ci : nodes[0].children) {
        const auto& ch = nodes[ci];
        if (ch.visits > bestVisits) {
            bestVisits = ch.visits;
            bestMean = ch.valueSum / std::max(1.0, static_cast<double>(ch.visits));
            bestChild = ci;
        } else if (ch.visits == bestVisits && ch.visits > 0) {
            const double mean = ch.valueSum / static_cast<double>(ch.visits);
            if (mean > bestMean) {
                bestMean = mean;
                bestChild = ci;
            }
        }
    }

    Action::PackedAction best = (bestChild >= 0) ? nodes[bestChild].action : rootDet.front();

    // Compare against dev-buy heuristic at root.
    if (devBuyHeuristic > base + static_cast<int>(bestMean)) {
        return buyDev;
    }

    return best;
}

int robber_hex_score(const Board::BoardState* board, HexId h, PlayerId selfId) {
    const auto hex = board->hexes[h];
    const Resource r = Board::Hex::unpackResource(hex);
    if (r == Resource::NoResource) return std::numeric_limits<int>::min();

    const uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
    if (p == 0) return std::numeric_limits<int>::min();

    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const int enemyVal = Board::Hex::unpackPlayerValue(hex, enemyId);
    const int selfVal = Board::Hex::unpackPlayerValue(hex, selfId);

    // Friendly robber: steal only when enemy has >=3 VP. Still worth blocking earlier.
    const int enemyVP = effective_vp(board, enemyId);

    int resW = 0;
    switch (r) {
        case Resource::Grain: resW = 140; break;
        case Resource::Ore: resW = 120; break;
        case Resource::Brick: resW = 95; break;
        case Resource::Lumber: resW = 80; break;
        case Resource::Wool: resW = 65; break;
        default: resW = 0; break;
    }

    int score = 0;
    score += (enemyVal * static_cast<int>(p)) * resW;
    score -= (selfVal * static_cast<int>(p)) * (resW + 40);

    // Prefer hexes where enemy has presence and we don't.
    if (enemyVal > 0 && selfVal == 0) score += 150;

    // If steal is active, prioritize higher enemy presence.
    if (enemyVP >= 3) score += 60 * enemyVal;

    return score;
}

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> It6Player::getInitialPlacement() {
    const PlayerId selfId = boardState->currentPlayer;
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto scarcityW = resource_scarcity_weights(boardState);

    auto placements = boardState->generatePlaceInitialStructures(selfId);
    if (placements.empty()) {
        return It5Player::getInitialPlacement();
    }

    Action::PackedAction best = placements.front();
    int bestScore = std::numeric_limits<int>::min();

    for (const auto a : placements) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);

        boardState->applyAction(a);

        const int selfNode = node_production_score(boardState, nodeId, scarcityW);
        const int roadSeed = edge_seed_score(boardState, edgeId, scarcityW);

        // Denial: compute best enemy reply after this placement.
        int enemyBest = 0;
        auto enemyPlacements = boardState->generatePlaceInitialStructures(enemyId);
        if (!enemyPlacements.empty()) {
            int eb = std::numeric_limits<int>::min();
            for (const auto ea : enemyPlacements) {
                const NodeId en = Action::unpackArg1(ea);
                eb = std::max(eb, node_production_score(boardState, en, scarcityW));
            }
            enemyBest = (eb == std::numeric_limits<int>::min()) ? 0 : eb;
        }

        boardState->undoLastAction();

        const int score = selfNode + (roadSeed / 4) - static_cast<int>(0.70 * static_cast<double>(enemyBest));
        if (score > bestScore) {
            bestScore = score;
            best = a;
        }
    }

    // NOTE: Game::initialPhase() currently applies placement.second.
    // Keep the same convention as the existing bots (return the placement action in both slots).
    return {best, best};
}

std::pair<Action::PackedAction, Action::PackedAction> It6Player::get2InitialPlacement() {
    const PlayerId selfId = boardState->currentPlayer;
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto scarcityW = resource_scarcity_weights(boardState);

    auto placements = boardState->generatePlace2InitialStructures(selfId);
    if (placements.empty()) {
        return It5Player::get2InitialPlacement();
    }

    // Compute what our first settlement already provides.
    std::array<int, 5> already = {0, 0, 0, 0, 0};
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = boardState->nodes[n];
        if (Board::Node::unpackOwner(node) != selfId) continue;
        if (Board::Node::unpackStructure(node) != StructureType::Settlement) continue;
        const auto nf = node_features(boardState, n);
        for (size_t i = 0; i < 5; ++i) already[i] += nf.pipByRes[i];
    }

    Action::PackedAction best = placements.front();
    int bestScore = std::numeric_limits<int>::min();

    for (const auto a : placements) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);

        boardState->applyAction(a);

        const auto nf = node_features(boardState, nodeId);
        int coverBonus = 0;
        for (size_t i = 0; i < 5; ++i) {
            if (already[i] == 0 && nf.pipByRes[i] > 0) {
                // Cover missing resources, especially grain/ore/brick.
                int w = 100;
                if (i == static_cast<size_t>(Resource::Grain)) w = 170;
                if (i == static_cast<size_t>(Resource::Ore)) w = 160;
                if (i == static_cast<size_t>(Resource::Brick)) w = 145;
                coverBonus += w;
            }
        }

        const int selfNode = node_production_score(boardState, nodeId, scarcityW) + coverBonus;
        const int roadSeed = edge_seed_score(boardState, edgeId, scarcityW);

        // Denial: best enemy second-placement quality.
        int enemyBest = 0;
        auto enemyPlacements = boardState->generatePlace2InitialStructures(enemyId);
        if (!enemyPlacements.empty()) {
            int eb = std::numeric_limits<int>::min();
            for (const auto ea : enemyPlacements) {
                const NodeId en = Action::unpackArg1(ea);
                eb = std::max(eb, node_production_score(boardState, en, scarcityW));
            }
            enemyBest = (eb == std::numeric_limits<int>::min()) ? 0 : eb;
        }

        boardState->undoLastAction();

        const int score = selfNode + (roadSeed / 4) - static_cast<int>(0.65 * static_cast<double>(enemyBest));
        if (score > bestScore) {
            bestScore = score;
            best = a;
        }
    }

    // NOTE: Game::initialPhase() currently applies placement.second.
    return {best, best};
}

Action::PackedAction It6Player::getMoveRobber() {
    const PlayerId selfId = boardState->currentPlayer;

    HexId bestHex = boardState->robberPosition;
    int bestScore = std::numeric_limits<int>::min();

    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (h == boardState->robberPosition) continue;
        const int s = robber_hex_score(boardState, h, selfId);
        if (s > bestScore) {
            bestScore = s;
            bestHex = h;
        }
    }

    Action::PackedAction move = Action::packType(Action::getEmptyAction(), ActionType::MoveRobber);
    move = Action::packPlayerID(move, selfId);
    move = Action::packArg1(move, bestHex);
    return move;
}

Action::PackedAction It6Player::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    // Use MCTS only when there is a real choice; keep it fast.
    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    int nonEnd = 0;
    for (const auto a : actions) if (Action::unpackType(a) != ActionType::EndTurn) nonEnd++;
    if (nonEnd <= 1) {
        // Delegate to It5/It4 behavior in trivial cases.
        return It5Player::getTurnAction();
    }

    // Iterations tuned for speed (this runs once per chosen action within a turn).
    constexpr int kIters = 70;
    constexpr int kDepth = 4;

    Action::PackedAction chosen = mcts_pick_action(boardState, selfId, kIters, kDepth);
    if (Action::unpackType(chosen) == ActionType::NoAction) {
        return It5Player::getTurnAction();
    }

    return chosen;
}
