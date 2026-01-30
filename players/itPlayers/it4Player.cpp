#include "it4Player.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

#include "playerHelpers.hpp"

namespace {

using PlayerHelpers::dice_pips;
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

uint8_t total_cards(const std::array<uint8_t, 5>& have) {
    uint8_t s = 0;
    for (auto v : have) s = static_cast<uint8_t>(s + v);
    return s;
}

std::array<uint8_t, 5> cost_for(BuyableType b) {
    const auto& c = StructureCost[static_cast<size_t>(b)];
    return {c[0], c[1], c[2], c[3], c[4]};
}

uint16_t deficit(const std::array<uint8_t, 5>& have,
                 const std::array<uint8_t, 5>& need) {
    uint16_t d = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i])
            d = static_cast<uint16_t>(d + (need[i] - have[i]));
    }
    return d;
}

struct TargetEval {
    BuyableType target = BuyableType::Road;
    uint16_t def = 0;
    bool affordable = false;
};

int priority_index(BuyableType b) {
    switch (b) {
        case BuyableType::City:
            return 0;
        case BuyableType::Settlement:
            return 1;
        case BuyableType::Road:
            return 2;
        case BuyableType::DevCard:
            return 3;
        default:
            return 99;
    }
}

TargetEval best_target_for(const std::array<uint8_t, 5>& have) {
    TargetEval best;
    best.def = std::numeric_limits<uint16_t>::max();

    for (BuyableType t : {BuyableType::City, BuyableType::Settlement,
                          BuyableType::Road, BuyableType::DevCard}) {
        const auto need = cost_for(t);
        const auto d = deficit(have, need);
        const bool aff = (d == 0);

        if (d < best.def || (d == best.def &&
                             priority_index(t) < priority_index(best.target))) {
            best.target = t;
            best.def = d;
            best.affordable = aff;
        }
    }
    return best;
}

bool is_pre_roll_dev_window(const Board::BoardState* board) {
    if (board->actionQueue.empty()) return true;

    const auto last = board->actionQueue.back();
    const auto type = Action::unpackType(last);

    // Typical start-of-turn: last action is EndTurn from previous player.
    if (type == ActionType::EndTurn) return true;

    // First ever turn after setup: last action may be a placement.
    if (board->currentTurn == 0 &&
        (type == ActionType::PlaceInitialStructures ||
         type == ActionType::Place2InitialStructures)) {
        return true;
    }

    return false;
}

int robber_hex_score(const Board::BoardState* board, PlayerId selfId,
                     HexId hexId) {
    if (hexId >= HEX_COUNT) return std::numeric_limits<int>::min();

    const auto hex = board->hexes[hexId];
    const Resource r = Board::Hex::unpackResource(hex);
    if (r == Resource::NoResource) return std::numeric_limits<int>::min();

    const PlayerId enemyId =
        (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
    const uint8_t selfVal = Board::Hex::unpackPlayerValue(hex, selfId);
    const uint8_t enemyVal = Board::Hex::unpackPlayerValue(hex, enemyId);

    int s = 0;
    s += static_cast<int>(pips) * 30 * static_cast<int>(enemyVal);
    s -= static_cast<int>(pips) * 45 * static_cast<int>(selfVal);

    if (enemyVal > 0 && selfVal == 0) s += 500;
    if (enemyVal == 0) s -= 200;

    return s;
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

        // Adjacent to any of our roads?
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

    // Minor bonus if this edge is near a port node (helps future trading).
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

bool bank_can_supply(const Board::BoardState* board, Resource r,
                     uint8_t amount) {
    return Bank::unpackResource(board->packedBank, r) >= amount;
}

int node_production_score(const Board::BoardState* board, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    // Slight preference: ore/grain are more valuable mid-game (cities/devs).
    auto res_weight = [](Resource r) -> int {
        switch (r) {
            case Resource::Ore:
                return 14;
            case Resource::Grain:
                return 13;
            case Resource::Brick:
                return 12;
            case Resource::Lumber:
                return 12;
            case Resource::Wool:
                return 11;
            default:
                return 0;
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
    // Port bonus (generic small, specific larger).
    const auto pt = Board::Node::unpackPortType(node);
    if (pt != PortType::NoPort) {
        if (pt == PortType::ThreeForOne)
            s += 35;
        else
            s += 70;
    }
    return s;
}

bool node_distance_rule_ok(const Board::BoardState* board, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return false;
    const auto node = board->nodes[nodeId];

    // Node itself must be empty.
    if (Board::Node::unpackStructure(node) != StructureType::NoStructure)
        return false;

    // No adjacent settlements/cities.
    for (int i = 0; i < 3; ++i) {
        const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
        if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
        const auto edge = board->edges[e];
        const NodeId a = Board::Edge::unpackAdjacentNode(edge, 0);
        const NodeId b = Board::Edge::unpackAdjacentNode(edge, 1);
        for (NodeId adj : {a, b}) {
            if (adj == nodeId || adj >= NODE_COUNT) continue;
            const auto st = Board::Node::unpackStructure(board->nodes[adj]);
            if (st == StructureType::Settlement || st == StructureType::City)
                return false;
        }
    }
    return true;
}

int city_upgrade_score(const Board::BoardState* board, NodeId nodeId) {
    // Upgrading doubles production on adjacent hexes; so just use production
    // score. (Node already contains a settlement owned by us in legal actions.)
    return node_production_score(board, nodeId);
}

// Road scoring: prefer roads that open good settlement spots.
int road_action_score(const Board::BoardState* board, PlayerId selfId,
                      EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT)
        return std::numeric_limits<int>::min();

    int s = edge_network_score(board, selfId, edgeId);

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    for (NodeId n : {n0, n1}) {
        if (!node_distance_rule_ok(board, n)) continue;
        // This road gives adjacency, so this node is an immediately-legal
        // settlement *once we can afford it*.
        s += 3 * node_production_score(board, n);
    }

    return s;
}

}  // namespace

Action::PackedAction It4Player::getDevAction() {
    const PlayerId selfId = boardState->currentPlayer;
    const PlayerId enemyId =
        (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    auto actions = boardState->generatePlayDevCardActions(selfId);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }

    const bool preRoll = is_pre_roll_dev_window(boardState);

    const auto selfPacked =
        boardState->packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked =
        boardState->packedPlayers[static_cast<uint8_t>(enemyId)];

    const uint8_t selfVP = effective_vp(boardState, selfId);
    const uint8_t enemyVP = effective_vp(boardState, enemyId);

    const uint8_t usedKnights = Player::unpackUsedKnights(selfPacked);
    const uint8_t enemyUsedKnights = Player::unpackUsedKnights(enemyPacked);

    const auto have = unpack_resources(selfPacked);
    const uint8_t handCount = total_cards(have);
    const auto baseTarget = best_target_for(have);

    // How much the current robber hurts us (if it's on our production).
    int currentRobberHurt = 0;
    if (boardState->robberPosition < HEX_COUNT) {
        const auto hx = boardState->hexes[boardState->robberPosition];
        const uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hx));
        const uint8_t selfVal = Board::Hex::unpackPlayerValue(hx, selfId);
        currentRobberHurt =
            static_cast<int>(p) * 220 * static_cast<int>(selfVal);
    }

    int bestScore = std::numeric_limits<int>::min();
    Action::PackedAction best = Action::getEmptyAction();

    for (const auto a : actions) {
        const auto type = Action::unpackType(a);
        if (type == ActionType::NoAction) continue;

        int score = 0;

        switch (type) {
            case ActionType::PlayDevCardKnight: {
                const HexId hexId = Action::unpackArg1(a);
                score += robber_hex_score(boardState, selfId, hexId);

                // Playing a knight also frees the current robber spot (if it
                // hurts us).
                score += currentRobberHurt;

                // Prefer stealing (must place robber on an enemy-produced hex).
                const uint8_t enemyVal = Board::Hex::unpackPlayerValue(
                    boardState->hexes[hexId], enemyId);
                if (enemyVal > 0) {
                    score += 700;
                }

                // Huge if this knight would grant largest army.
                const uint8_t newUsed = static_cast<uint8_t>(usedKnights + 1);
                if (newUsed >= 3 && newUsed > enemyUsedKnights &&
                    !Player::unpackLargestArmyFlag(selfPacked)) {
                    score += 10000;
                    if (Player::unpackLargestArmyFlag(enemyPacked)) {
                        // Swing of +4 effective VP.
                        score += 2000;
                    }
                }

                // If enemy is close to winning, be more willing to block.
                if (enemyVP >= 12) score += 800;

                // Knights are generally safe pre-roll (don't increase hand
                // size), so don't punish pre-roll.
                break;
            }

            case ActionType::PlayDevCardMonopoly: {
                const Resource r = static_cast<Resource>(Action::unpackArg1(a));
                const uint8_t enemyHave =
                    Player::unpackResource(enemyPacked, r);

                // Only meaningful if it actually takes a chunk.
                if (enemyHave < 3) {
                    score = -5000;
                    break;
                }

                score += static_cast<int>(enemyHave) * 420;

                // Model the post-monopoly inventory to see if it unlocks a
                // better purchase.
                auto after = have;
                after[static_cast<size_t>(r)] =
                    static_cast<uint8_t>(std::min<int>(
                        255, static_cast<int>(after[static_cast<size_t>(r)]) +
                                 static_cast<int>(enemyHave)));
                const auto afterTarget = best_target_for(after);
                const int improvement = static_cast<int>(baseTarget.def) -
                                        static_cast<int>(afterTarget.def);
                score += improvement * 750;

                if (afterTarget.affordable) {
                    if (afterTarget.target == BuyableType::City)
                        score += 8000;
                    else if (afterTarget.target == BuyableType::Settlement)
                        score += 6000;
                    else if (afterTarget.target == BuyableType::Road)
                        score += 2500;
                    else
                        score += 1800;
                }

                // If enemy is close to winning, deny resources even more.
                if (enemyVP >= 12) score += 600;

                // Pre-roll risk: if we already have a big hand, avoid taking
                // more before we know the roll.
                if (preRoll) {
                    const int projected = static_cast<int>(handCount) +
                                          static_cast<int>(enemyHave);
                    if (projected >= 10)
                        score -= 4500;
                    else if (projected >= 8)
                        score -= 1800;
                }

                break;
            }

            case ActionType::PlayDevCardYearOfPlenty: {
                const Resource r1 =
                    static_cast<Resource>(Action::unpackArg1(a));
                const Resource r2 =
                    static_cast<Resource>(Action::unpackArg2(a));

                // Avoid underflowing the bank in edge cases.
                const uint8_t need1 = 1;
                const uint8_t need2 = (r1 == r2) ? 2 : 1;
                if (!bank_can_supply(boardState, r1, need2) ||
                    (r1 != r2 && !bank_can_supply(boardState, r2, need1))) {
                    score = -5000;
                    break;
                }

                auto after = have;
                after[static_cast<size_t>(r1)]++;
                after[static_cast<size_t>(r2)]++;

                const auto afterTarget = best_target_for(after);

                // Prefer reducing deficit to high-value targets.
                const int improvement = static_cast<int>(baseTarget.def) -
                                        static_cast<int>(afterTarget.def);
                score += improvement * 850;

                // Big bonuses if it makes an immediate purchase possible.
                if (afterTarget.affordable) {
                    if (afterTarget.target == BuyableType::City)
                        score += 9000;
                    else if (afterTarget.target == BuyableType::Settlement)
                        score += 7000;
                    else if (afterTarget.target == BuyableType::Road)
                        score += 4500;
                    else
                        score += 2500;
                }

                // Pre-roll risk: YoP increases hand size; avoid if it risks
                // discarding.
                if (preRoll) {
                    const int projected = static_cast<int>(handCount) + 2;
                    if (projected >= 10)
                        score -= 4200;
                    else if (projected >= 8 && score < 9000)
                        score -= 1400;
                }

                break;
            }

            case ActionType::PlayDevCardRoadBuilding: {
                const EdgeId e1 = Action::unpackArg1(a);
                const EdgeId e2 = Action::unpackArg2(a);

                score += road_action_score(boardState, selfId, e1);
                if (e2 != EdgeIdNone) {
                    score += road_action_score(boardState, selfId, e2);
                    score += 200;  // two-road actions are better than one-road
                                   // actions
                }

                // If we are behind, push expansion a bit.
                if (enemyVP > selfVP) score += 200;

                // Road building is also safe pre-roll (doesn't increase hand
                // size).

                break;
            }

            default:
                score = -5000;
                break;
        }

        if (score > bestScore) {
            bestScore = score;
            best = a;
        }
    }

    // If nothing looked worthwhile, don't play a dev card.
    if (bestScore < 250) {
        return Action::getEmptyAction();
    }

    return best;
}

Action::PackedAction It4Player::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;
    const PlayerId enemyId =
        (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    std::vector<Action::PackedAction> cityActions;
    std::vector<Action::PackedAction> settlementActions;
    std::vector<Action::PackedAction> roadActions;
    std::vector<Action::PackedAction> devBuyActions;
    std::vector<Action::PackedAction> tradeActions;
    std::vector<Action::PackedAction> otherActions;
    std::vector<Action::PackedAction> endTurnActions;

    for (const auto a : actions) {
        switch (Action::unpackType(a)) {
            case ActionType::BuildCity:
                cityActions.push_back(a);
                break;
            case ActionType::BuildSettlement:
                settlementActions.push_back(a);
                break;
            case ActionType::BuildRoad:
                roadActions.push_back(a);
                break;
            case ActionType::BuyDevCard:
                devBuyActions.push_back(a);
                break;
            case ActionType::TradeBank:
                tradeActions.push_back(a);
                break;
            case ActionType::EndTurn:
                endTurnActions.push_back(a);
                break;
            default:
                otherActions.push_back(a);
                break;
        }
    }

    // Deterministic best-pickers (avoid It2/It3 random among builds).
    if (!cityActions.empty()) {
        int bestS = std::numeric_limits<int>::min();
        Action::PackedAction best = cityActions.front();
        for (const auto a : cityActions) {
            const NodeId n = Action::unpackArg1(a);
            const int s = city_upgrade_score(boardState, n);
            if (s > bestS) {
                bestS = s;
                best = a;
            }
        }
        return best;
    }

    if (!settlementActions.empty()) {
        int bestS = std::numeric_limits<int>::min();
        Action::PackedAction best = settlementActions.front();
        for (const auto a : settlementActions) {
            const NodeId n = Action::unpackArg1(a);
            const int s = node_production_score(boardState, n);
            if (s > bestS) {
                bestS = s;
                best = a;
            }
        }
        return best;
    }

    // Trades: pick the trade that most improves our ability to afford best
    // target.
    if (!tradeActions.empty()) {
        const auto selfPacked =
            boardState->packedPlayers[static_cast<uint8_t>(selfId)];
        const auto have = unpack_resources(selfPacked);
        const auto beforeTarget = best_target_for(have);

        uint16_t bestDef = beforeTarget.def;
        int bestPri = priority_index(beforeTarget.target);
        Action::PackedAction best = Action::getEmptyAction();

        for (const auto t : tradeActions) {
            const Resource give = static_cast<Resource>(Action::unpackArg1(t));
            const Resource recv = static_cast<Resource>(Action::unpackArg2(t));
            const uint8_t ratio = Action::unpackArg3(t);
            if (give == recv) continue;

            auto after = have;
            const size_t gi = static_cast<size_t>(give);
            const size_t ri = static_cast<size_t>(recv);
            if (after[gi] < ratio) continue;
            after[gi] = static_cast<uint8_t>(after[gi] - ratio);
            after[ri] = static_cast<uint8_t>(after[ri] + 1);

            const auto afterTarget = best_target_for(after);
            const uint16_t d = afterTarget.def;
            const int pri = priority_index(afterTarget.target);

            // Prefer lowering deficit; tie-break by higher-value target.
            if (d < bestDef || (d == bestDef && pri < bestPri)) {
                bestDef = d;
                bestPri = pri;
                best = t;
            }
        }

        if (Action::unpackType(best) == ActionType::TradeBank) {
            return best;
        }
    }

    // Between roads and dev cards, be more dev-card heavy than It3.
    int bestRoadScore = std::numeric_limits<int>::min();
    Action::PackedAction bestRoad = Action::getEmptyAction();
    if (!roadActions.empty()) {
        for (const auto a : roadActions) {
            const EdgeId e = Action::unpackArg1(a);
            const int s = road_action_score(boardState, selfId, e);
            if (s > bestRoadScore) {
                bestRoadScore = s;
                bestRoad = a;
            }
        }
    }

    const uint8_t selfVP = effective_vp(boardState, selfId);
    const uint8_t enemyVP = effective_vp(boardState, enemyId);

    if (!devBuyActions.empty() && !roadActions.empty()) {
        // If behind or mid-game, buy dev unless a road is clearly opening a
        // strong settlement.
        if (enemyVP > selfVP || selfVP >= 6) {
            if (bestRoadScore < 3500) return devBuyActions.front();
        }
        // Otherwise, take the higher-scoring option.
        if (bestRoadScore >= 4200) return bestRoad;
        return devBuyActions.front();
    }

    if (!roadActions.empty()) return bestRoad;
    if (!devBuyActions.empty()) return devBuyActions.front();

    // Fallback.
    if (!otherActions.empty()) return otherActions.front();
    if (!endTurnActions.empty()) return endTurnActions.front();
    return actions.front();
}
