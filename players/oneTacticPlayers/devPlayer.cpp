#include "devPlayer.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

#include "playerHelpers.hpp"

namespace {

using PlayerHelpers::cost_for;
using PlayerHelpers::deficit;
using PlayerHelpers::dice_pips;
using PlayerHelpers::effective_vp;
using PlayerHelpers::evaluate_position;
using PlayerHelpers::hand_count;
using PlayerHelpers::is_deterministic_action;
using PlayerHelpers::unpack_resources;

uint8_t resource_weight_dev_early(Resource r) {
    // Dev-card strategy wants Ore/Grain/Wool early.
    switch (r) {
        case Resource::Ore:
            return 9;
        case Resource::Grain:
            return 8;
        case Resource::Wool:
            return 7;
        case Resource::Brick:
            return 2;
        case Resource::Lumber:
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

int score_settlement_node_dev(const Board::BoardState* board, NodeId nodeId,
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
                           static_cast<int>(resource_weight_dev_early(r));

        if (static_cast<uint8_t>(r) < 5) {
            resourceCounts[static_cast<size_t>(r)]++;
        }
    }

    // Diversity still matters, but less than hitting ore/grain/wool.
    int unique = 0;
    for (size_t i = 0; i < 5; ++i) unique += resourceCounts[i] > 0 ? 1 : 0;
    int diversityBonus = unique * 18;

    // Penalize duplicates on the same node (double resources are less
    // flexible).
    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (resourceCounts[i] > 1) {
            duplicatePenalty += 20 * static_cast<int>(resourceCounts[i] - 1);
        }
    }

    // Port bonus: dev-buy strategy likes 3:1 and ore/grain/wool ports.
    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += secondPlacement ? 30 : 15;
        if (port == PortType::ThreeForOne) portBonus += 12;
        if (port == PortType::OrePort) portBonus += 18;
        if (port == PortType::GrainPort) portBonus += 14;
        if (port == PortType::WoolPort) portBonus += 12;
    }

    // Second placement: strongly complement missing dev resources.
    int complementBonus = 0;
    if (secondPlacement) {
        auto add_if_missing = [&](Resource r, int bonus) {
            const size_t idx = static_cast<size_t>(r);
            if (resourceCounts[idx] > 0 && !firstRes[idx])
                complementBonus += bonus;
        };

        add_if_missing(Resource::Ore, 30);
        add_if_missing(Resource::Grain, 26);
        add_if_missing(Resource::Wool, 22);

        // Still mildly prefer getting at least one of {Brick,Lumber} if totally
        // missing.
        add_if_missing(Resource::Brick, 8);
        add_if_missing(Resource::Lumber, 8);
    }

    return productionScore + diversityBonus + portBonus + complementBonus -
           duplicatePenalty;
}

PlacementScore pick_best_dev_placement(
    const Board::BoardState* board,
    const std::vector<Action::PackedAction>& actions, bool secondPlacement,
    const std::array<bool, 5>& firstRes) {
    PlacementScore best;

    for (const auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);
        if (nodeId >= NODE_COUNT) continue;
        if (edgeId == EdgeIdNone) continue;

        int s =
            score_settlement_node_dev(board, nodeId, secondPlacement, firstRes);

        // Tiny road target degree bonus (more future options).
        NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
        NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
        const NodeId other = (n0 == nodeId) ? n1 : n0;
        if (other < NODE_COUNT) {
            const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
            int deg = 0;
            for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
            s += deg;  // 1..3
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

std::array<int, 5> dev_keep_weights() {
    // Strongly keep Ore/Grain/Wool to enable dev buys.
    return {
        6,   // Brick
        6,   // Lumber
        20,  // Wool
        22,  // Grain
        24,  // Ore
    };
}

}  // namespace

std::pair<Action::PackedAction, Action::PackedAction>
DevPlayer::getInitialPlacement() {
    auto actions =
        boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    firstPlacementResources = {false, false, false, false, false};
    const auto best = pick_best_dev_placement(boardState, actions, false,
                                              firstPlacementResources);
    const NodeId nodeId = Action::unpackArg1(best.action);
    firstPlacementResources = resources_at_node(boardState, nodeId);
    return {best.action, best.action};
}

std::pair<Action::PackedAction, Action::PackedAction>
DevPlayer::get2InitialPlacement() {
    auto actions =
        boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const auto best = pick_best_dev_placement(boardState, actions, true,
                                              firstPlacementResources);
    return {best.action, best.action};
}

Action::PackedAction DevPlayer::getDiscardAction() {
    const PlayerId selfId = boardState->currentPlayer;
    const auto packed = boardState->packedPlayers[static_cast<uint8_t>(selfId)];

    auto have = unpack_resources(packed);
    const uint8_t total = hand_count(have);
    if (total <= 9) return Action::getEmptyAction();

    const uint8_t toDiscard = static_cast<uint8_t>(total / 2);

    const auto need = cost_for(BuyableType::DevCard);
    const auto baseW = dev_keep_weights();

    Action::PackedAction action = Action::getEmptyAction();

    uint8_t remaining = toDiscard;
    while (remaining > 0) {
        int best = std::numeric_limits<int>::max();
        int bestIdx = -1;

        for (int i = 0; i < 5; ++i) {
            if (have[static_cast<size_t>(i)] == 0) continue;

            const bool neededForDev =
                have[static_cast<size_t>(i)] <= need[static_cast<size_t>(i)];
            int discardCost = baseW[static_cast<size_t>(i)];
            if (neededForDev)
                discardCost +=
                    800;  // strongly avoid discarding dev ingredients

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

Action::PackedAction DevPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    const int baseScore = evaluate_position(boardState, selfId);

    // If we can win immediately by building (or are at 9+ VP), prefer
    // deterministic builds.
    const int selfVP = effective_vp(boardState, selfId);

    Action::PackedAction devBuy = Action::getEmptyAction();
    Action::PackedAction cityBuild = Action::getEmptyAction();
    Action::PackedAction settleBuild = Action::getEmptyAction();

    for (const auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::BuyDevCard) devBuy = a;
        if (t == ActionType::BuildCity) cityBuild = a;
        if (t == ActionType::BuildSettlement) settleBuild = a;
    }

    if (Action::unpackType(devBuy) == ActionType::BuyDevCard) {
        if (selfVP >= 9) {
            if (Action::unpackType(cityBuild) == ActionType::BuildCity)
                return cityBuild;
            if (Action::unpackType(settleBuild) == ActionType::BuildSettlement)
                return settleBuild;
        }
        // Heavy dev-card priority.
        return devBuy;
    }

    Action::PackedAction best = Action::getEmptyAction();
    int bestScore = std::numeric_limits<int>::min();

    auto consider = [&](Action::PackedAction a, int extraBonus = 0) {
        if (!is_deterministic_action(a)) return;

        boardState->applyAction(a);
        int s = evaluate_position(boardState, selfId) + extraBonus;

        // Strongly reward actions that enable buying a dev card next.
        if (Action::unpackType(a) == ActionType::TradeBank ||
            Action::unpackType(a) == ActionType::BuildRoad) {
            auto next = boardState->getLegalActions(selfId);
            bool canDev = false;
            for (const auto na : next) {
                if (Action::unpackType(na) == ActionType::BuyDevCard) {
                    canDev = true;
                    break;
                }
            }
            if (canDev) s += 6000;
        }

        boardState->undoLastAction();

        if (s > bestScore) {
            bestScore = s;
            best = a;
        }
    };

    // Still allow high-value deterministic builds.
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

    // Otherwise, prefer actions that set up dev-buy.
    for (const auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::TradeBank || t == ActionType::BuildRoad ||
            t == ActionType::EndTurn) {
            consider(a);
        }
    }

    // Fall back to It5 if we didn't pick something useful.
    if (Action::unpackType(best) == ActionType::NoAction) {
        return It5Player::getTurnAction();
    }

    // Avoid choosing EndTurn when it doesn't improve the evaluation.
    if (Action::unpackType(best) == ActionType::EndTurn &&
        bestScore < baseScore) {
        return It5Player::getTurnAction();
    }

    return best;
}
