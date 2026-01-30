#include "it2Player.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
Action::PackedAction pick_random(
    const std::vector<Action::PackedAction>& pool) {
    if (pool.empty()) {
        return Action::getEmptyAction();
    }
    int idx = RandomDevice::uniform_u32_range(
        0, static_cast<uint32_t>(pool.size()) - 1);
    return pool[idx];
}

struct Ports {
    bool threeForOne = false;
    std::array<bool, 5> twoForOneByResource{false, false, false, false, false};
};

bool is_player_port_node(const Board::BoardState* board, PlayerId playerId,
                         NodeId nodeId) {
    const auto structure = Board::Node::unpackStructure(board->nodes[nodeId]);
    const auto owner = Board::Node::unpackOwner(board->nodes[nodeId]);
    return owner == playerId && (structure == StructureType::Settlement ||
                                 structure == StructureType::City);
}

Ports detect_ports(const Board::BoardState* board, PlayerId playerId) {
    Ports p;

    // 2:1 ports (resource-specific)
    for (NodeId nodeId : brickPortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.twoForOneByResource[static_cast<size_t>(Resource::Brick)] = true;
            break;
        }
    }
    for (NodeId nodeId : lumberPortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.twoForOneByResource[static_cast<size_t>(Resource::Lumber)] = true;
            break;
        }
    }
    for (NodeId nodeId : woolPortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.twoForOneByResource[static_cast<size_t>(Resource::Wool)] = true;
            break;
        }
    }
    for (NodeId nodeId : grainPortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.twoForOneByResource[static_cast<size_t>(Resource::Grain)] = true;
            break;
        }
    }
    for (NodeId nodeId : orePortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.twoForOneByResource[static_cast<size_t>(Resource::Ore)] = true;
            break;
        }
    }

    // 3:1 port (generic)
    for (NodeId nodeId : threeForOnePortsNodes) {
        if (is_player_port_node(board, playerId, nodeId)) {
            p.threeForOne = true;
            break;
        }
    }

    return p;
}

Action::PackedAction build_trade(PlayerId playerId, Resource give,
                                 Resource receive, uint8_t ratio) {
    auto a = Action::getEmptyAction();
    a = Action::packType(a, ActionType::TradeBank);
    a = Action::packPlayerID(a, playerId);
    a = Action::packArg1(a, static_cast<uint8_t>(give));
    a = Action::packArg2(a, static_cast<uint8_t>(receive));
    a = Action::packArg3(a, ratio);
    return a;
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

std::array<uint8_t, 5> cost_for(BuyableType b) {
    const auto& c = StructureCost[static_cast<size_t>(b)];
    return {c[0], c[1], c[2], c[3], c[4]};
}

uint16_t total_deficit(const std::array<uint8_t, 5>& have,
                       const std::array<uint8_t, 5>& need) {
    uint16_t sum = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i])
            sum = static_cast<uint16_t>(sum + (need[i] - have[i]));
    }
    return sum;
}

struct Closeness {
    uint16_t remainingMissing =
        0;  // cards still missing after best-case trading
    uint16_t maxTradeableUnits =
        0;  // how many missing cards we could potentially acquire via trades
    uint16_t rawMissing = 0;  // cards missing with no trades
};

uint8_t best_ratio_for_give(const Ports& ports, size_t giveIdx) {
    if (ports.twoForOneByResource[giveIdx]) return 2;
    if (ports.threeForOne) return 3;
    return 4;
}

Closeness evaluate_closeness(const std::array<uint8_t, 5>& have,
                             const std::array<uint8_t, 5>& need,
                             const Ports& ports) {
    Closeness c;

    uint16_t missing = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i])
            missing = static_cast<uint16_t>(missing + (need[i] - have[i]));
    }
    c.rawMissing = missing;

    uint16_t tradeable = 0;
    for (size_t i = 0; i < 5; ++i) {
        const uint8_t surplus =
            have[i] > need[i] ? static_cast<uint8_t>(have[i] - need[i]) : 0;
        if (surplus == 0) continue;
        const uint8_t ratio = best_ratio_for_give(ports, i);
        tradeable = static_cast<uint16_t>(tradeable + (surplus / ratio));
    }
    c.maxTradeableUnits = tradeable;

    c.remainingMissing =
        (missing > tradeable) ? static_cast<uint16_t>(missing - tradeable) : 0;
    return c;
}

int priority_index(BuyableType b) {
    // Value tie-break for "equally close" targets.
    // Keep consistent with the original higher-level preference (City >
    // Settlement > Road > Dev).
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

BuyableType choose_closest_target(const std::array<uint8_t, 5>& have,
                                  const Ports& ports) {
    BuyableType bestType = BuyableType::Road;
    Closeness best = {std::numeric_limits<uint16_t>::max(), 0,
                      std::numeric_limits<uint16_t>::max()};

    for (BuyableType t : {BuyableType::City, BuyableType::Settlement,
                          BuyableType::Road, BuyableType::DevCard}) {
        const auto need = cost_for(t);
        const auto c = evaluate_closeness(have, need, ports);

        // Primary: minimal remaining missing after best-case trades.
        if (c.remainingMissing < best.remainingMissing) {
            best = c;
            bestType = t;
            continue;
        }
        if (c.remainingMissing > best.remainingMissing) continue;

        // Secondary: if equally reachable after trades, prefer higher-value
        // buyables.
        if (priority_index(t) < priority_index(bestType)) {
            best = c;
            bestType = t;
            continue;
        }
        if (priority_index(t) > priority_index(bestType)) continue;

        // Tertiary: fewer raw missing cards (minor stability).
        if (c.rawMissing < best.rawMissing) {
            best = c;
            bestType = t;
        }
    }

    return bestType;
}

Action::PackedAction choose_trade_for_target(
    const std::array<uint8_t, 5>& have, const std::array<uint8_t, 5>& need,
    const Ports& ports, PlayerId playerId,
    const std::vector<Action::PackedAction>& legalTrades) {
    // Only trade if we're missing something.
    const uint16_t beforeDef = total_deficit(have, need);
    if (beforeDef == 0) {
        return Action::getEmptyAction();
    }

    auto best = Action::getEmptyAction();
    uint16_t bestAfterDef = std::numeric_limits<uint16_t>::max();
    uint16_t bestImprovement = 0;

    const auto try_ratio = [&](uint8_t ratio,
                               bool (*ratio_allowed)(const Ports&, size_t)) {
        for (size_t recv = 0; recv < 5; ++recv) {
            if (have[recv] >= need[recv])
                continue;  // not missing this resource

            for (size_t give = 0; give < 5; ++give) {
                if (give == recv) continue;
                if (!ratio_allowed(ports, give)) continue;

                // Simple rule: only spend from surplus relative to the target
                // cost.
                const uint8_t surplus =
                    have[give] > need[give]
                        ? static_cast<uint8_t>(have[give] - need[give])
                        : 0;
                if (surplus < ratio) continue;

                auto after = have;
                after[give] = static_cast<uint8_t>(after[give] - ratio);
                after[recv] = static_cast<uint8_t>(after[recv] + 1);
                const uint16_t afterDef = total_deficit(after, need);
                if (afterDef >= beforeDef) continue;

                const uint16_t improvement =
                    static_cast<uint16_t>(beforeDef - afterDef);
                // Prefer the trade that gets us closest to affording the
                // target.
                if (afterDef < bestAfterDef ||
                    (afterDef == bestAfterDef &&
                     improvement > bestImprovement)) {
                    bestAfterDef = afterDef;
                    bestImprovement = improvement;
                    best = build_trade(playerId, static_cast<Resource>(give),
                                       static_cast<Resource>(recv), ratio);
                }
            }
        }
    };

    // Priority: 2:1 (specific) -> 3:1 (generic) -> 4:1 (bank)
    try_ratio(2, [](const Ports& p, size_t give) {
        return p.twoForOneByResource[give];
    });
    if (bestImprovement > 0) {
        if (std::find(legalTrades.begin(), legalTrades.end(), best) ==
            legalTrades.end()) {
            throw std::runtime_error(
                "It2Player chose a 2:1 trade that is not in legal actions");
        }
        return best;
    }
    try_ratio(3, [](const Ports& p, size_t /*give*/) { return p.threeForOne; });
    if (bestImprovement > 0) {
        if (std::find(legalTrades.begin(), legalTrades.end(), best) ==
            legalTrades.end()) {
            throw std::runtime_error(
                "It2Player chose a 3:1 trade that is not in legal actions");
        }
        return best;
    }
    try_ratio(4, [](const Ports& /*p*/, size_t /*give*/) { return true; });
    if (bestImprovement > 0) {
        if (std::find(legalTrades.begin(), legalTrades.end(), best) ==
            legalTrades.end()) {
            throw std::runtime_error(
                "It2Player chose a 4:1 trade that is not in legal actions");
        }
    }
    return best;
}
}  // namespace

Action::PackedAction It2Player::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }

    std::vector<Action::PackedAction> cityActions;
    std::vector<Action::PackedAction> settlementActions;
    std::vector<Action::PackedAction> roadActions;
    std::vector<Action::PackedAction> devActions;
    std::vector<Action::PackedAction> tradeActions;
    std::vector<Action::PackedAction> otherActions;
    std::vector<Action::PackedAction> endTurnActions;

    cityActions.reserve(actions.size());
    settlementActions.reserve(actions.size());
    roadActions.reserve(actions.size());
    devActions.reserve(actions.size());
    tradeActions.reserve(actions.size());
    otherActions.reserve(actions.size());
    endTurnActions.reserve(actions.size());

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
                devActions.push_back(a);
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

    // Always take city / settlement immediately when available.
    if (!cityActions.empty()) return pick_random(cityActions);
    if (!settlementActions.empty()) return pick_random(settlementActions);

    // Try to trade towards the closest purchase (after best-case trading), even
    // if we could currently build a lower-priority thing (road/dev).
    if (!tradeActions.empty()) {
        const auto playerId = boardState->currentPlayer;
        const auto ports = detect_ports(boardState, playerId);
        const auto packed =
            boardState->packedPlayers[static_cast<uint8_t>(playerId)];
        const auto have = unpack_resources(packed);

        // Choose what we're trying to buy based on how close we are (after
        // best-case trading).
        const BuyableType target = choose_closest_target(have, ports);
        const auto need = cost_for(target);

        auto trade =
            choose_trade_for_target(have, need, ports, playerId, tradeActions);
        if (Action::unpackType(trade) == ActionType::TradeBank) {
            return trade;
        }
    }

    // No helpful trade selected: follow It1-style build preference for
    // remaining types.
    if (!roadActions.empty()) return pick_random(roadActions);
    if (!devActions.empty()) return pick_random(devActions);

    // Otherwise keep behaving like Random: random among non-EndTurn actions
    // first, then EndTurn.
    if (!tradeActions.empty()) return pick_random(tradeActions);
    if (!otherActions.empty()) return pick_random(otherActions);
    if (!endTurnActions.empty()) return pick_random(endTurnActions);

    return pick_random(actions);
}
