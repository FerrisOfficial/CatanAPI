#include "cityRushPlayer.hpp"
#include "playerHelpers.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <vector>

namespace {
using PlayerHelpers::dice_pips;
using PlayerHelpers::unpack_resources;
using PlayerHelpers::cost_for;
using PlayerHelpers::deficit;
using PlayerHelpers::evaluate_position;
using PlayerHelpers::is_deterministic_action;

int city_rush_resource_weight(Resource r) {
    // Mild city bias, but keep early expansion healthy.
    switch (r) {
        case Resource::Brick: return 5;
        case Resource::Lumber: return 5;
        case Resource::Wool: return 4;
        case Resource::Grain: return 6;
        case Resource::Ore: return 6;
        default: return 0;
    }
}

std::array<bool, 5> resources_at_node(const Board::BoardState* board, NodeId nodeId) {
    std::array<bool, 5> has {false, false, false, false, false};
    if (nodeId >= NODE_COUNT) return has;
    const auto node = board->nodes[nodeId];
    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
        const Resource r = Board::Hex::unpackResource(board->hexes[hexId]);
        if (r == Resource::NoResource) continue;
        if (static_cast<uint8_t>(r) < 5) has[static_cast<size_t>(r)] = true;
    }
    return has;
}

int score_city_rush_node(const Board::BoardState* board, NodeId nodeId, bool secondPlacement,
                         const std::array<bool, 5>& firstRes) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    int productionScore = 0;
    int orePips = 0;
    int grainPips = 0;
    std::array<uint8_t, 5> resourceCounts {0, 0, 0, 0, 0};

    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
        const auto hex = board->hexes[hexId];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;

        const int pips = static_cast<int>(dice_pips(Board::Hex::unpackCatanNumber(hex)));
        productionScore += pips * city_rush_resource_weight(r);

        if (r == Resource::Ore) orePips += pips;
        if (r == Resource::Grain) grainPips += pips;
        if (static_cast<uint8_t>(r) < 5) {
            resourceCounts[static_cast<size_t>(r)]++;
        }
    }

    int unique = 0;
    for (size_t i = 0; i < 5; ++i) unique += (resourceCounts[i] > 0) ? 1 : 0;
    const int diversityBonus = unique * (secondPlacement ? 25 : 25);

    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (resourceCounts[i] > 1) duplicatePenalty += 18 * static_cast<int>(resourceCounts[i] - 1);
    }

    // City-rush tilt: keep it present but not dominant.
    const int cityBias = orePips * 6 + grainPips * 6;

    int complementBonus = 0;
    if (secondPlacement) {
        for (size_t i = 0; i < 5; ++i) {
            if (resourceCounts[i] > 0 && !firstRes[i]) complementBonus += 18;
        }
        // Strongly avoid ending setup without Brick/Lumber access.
        if (!firstRes[static_cast<size_t>(Resource::Brick)] && resourceCounts[static_cast<size_t>(Resource::Brick)] > 0) complementBonus += 14;
        if (!firstRes[static_cast<size_t>(Resource::Lumber)] && resourceCounts[static_cast<size_t>(Resource::Lumber)] > 0) complementBonus += 14;
    }

    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += secondPlacement ? 25 : 15;
        if (port == PortType::ThreeForOne) portBonus += 10;
        if (port == PortType::OrePort) portBonus += 18;
        if (port == PortType::GrainPort) portBonus += 16;
    }

    // Sanity penalties: avoid stalling without Brick/Lumber.
    int stallPenalty = 0;
    if (resourceCounts[static_cast<size_t>(Resource::Brick)] == 0 &&
        resourceCounts[static_cast<size_t>(Resource::Lumber)] == 0) {
        stallPenalty = secondPlacement ? 45 : 70;
    }
    if (secondPlacement) {
        if (!firstRes[static_cast<size_t>(Resource::Brick)] && resourceCounts[static_cast<size_t>(Resource::Brick)] == 0) stallPenalty += 35;
        if (!firstRes[static_cast<size_t>(Resource::Lumber)] && resourceCounts[static_cast<size_t>(Resource::Lumber)] == 0) stallPenalty += 35;
    }

    return productionScore + diversityBonus + cityBias + complementBonus + portBonus - duplicatePenalty - stallPenalty;
}

Action::PackedAction pick_best_city_rush_placement(const Board::BoardState* board,
                                                   const std::vector<Action::PackedAction>& actions,
                                                   bool secondPlacement,
                                                   const std::array<bool, 5>& firstRes) {
    int bestScore = std::numeric_limits<int>::min();
    Action::PackedAction best = Action::getEmptyAction();

    for (const auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);
        if (nodeId >= NODE_COUNT || edgeId == EdgeIdNone) continue;

        int s = score_city_rush_node(board, nodeId, secondPlacement, firstRes);

        const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
        const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
        const NodeId other = (n0 == nodeId) ? n1 : n0;
        if (other < NODE_COUNT) {
            const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
            int deg = 0;
            for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
            s += deg;
        }

        if (s > bestScore) {
            bestScore = s;
            best = a;
        }
    }

    if (Action::unpackType(best) == ActionType::NoAction && !actions.empty()) {
        return actions.front();
    }

    return best;
}

bool has_upgradeable_settlement(const Board::BoardState* board, PlayerId pid) {
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = board->nodes[n];
        if (Board::Node::unpackOwner(node) != pid) continue;
        if (Board::Node::unpackStructure(node) == StructureType::Settlement) return true;
    }
    return false;
}

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> CityRushPlayer::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const std::array<bool, 5> emptyFirst {false, false, false, false, false};
    const auto best = pick_best_city_rush_placement(boardState, actions, false, emptyFirst);
    const NodeId nodeId = Action::unpackArg1(best);
    firstPlacementResources = resources_at_node(boardState, nodeId);
    hasFirstPlacement = true;
    return {best, best};
}

std::pair<Action::PackedAction, Action::PackedAction> CityRushPlayer::get2InitialPlacement() {
    auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const auto best = pick_best_city_rush_placement(boardState, actions, true, hasFirstPlacement ? firstPlacementResources
                                                                                                 : std::array<bool, 5>{false, false, false, false, false});
    return {best, best};
}

Action::PackedAction CityRushPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    // Keep the "rush cities" identity by snapping to city builds when available,
    // but otherwise rely on It5's much stronger overall policy (including dev cards).
    for (const auto a : actions) {
        if (Action::unpackType(a) == ActionType::BuildCity) return a;
    }

    return It5Player::getTurnAction();
}
