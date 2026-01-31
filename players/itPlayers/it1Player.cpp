#include "it1Player.hpp"

#include <vector>

Action::PackedAction It1Player::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);
    if (actions.empty()) {
        return Action::getEmptyAction();
    }

    std::vector<Action::PackedAction> cityActions;
    std::vector<Action::PackedAction> settlementActions;
    std::vector<Action::PackedAction> roadActions;
    std::vector<Action::PackedAction> devActions;
    std::vector<Action::PackedAction> otherActions;
    std::vector<Action::PackedAction> endTurnActions;

    cityActions.reserve(actions.size());
    settlementActions.reserve(actions.size());
    roadActions.reserve(actions.size());
    devActions.reserve(actions.size());
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
            case ActionType::EndTurn:
                endTurnActions.push_back(a);
                break;
            default:
                // Trades and any other legal actions should still be considered
                // (randomly), but only after we fail to find the
                // higher-priority build/buy options.
                otherActions.push_back(a);
                break;
        }
    }

    const auto pick_random = [](const std::vector<Action::PackedAction>& pool)
        -> Action::PackedAction {
        if (pool.empty()) {
            return Action::getEmptyAction();
        }
        int idx = RandomDevice::uniform_u32_range(
            0, static_cast<uint32_t>(pool.size()) - 1);
        return pool[idx];
    };

    if (!cityActions.empty()) return pick_random(cityActions);
    if (!settlementActions.empty()) return pick_random(settlementActions);
    if (!roadActions.empty()) return pick_random(roadActions);
    if (!devActions.empty()) return pick_random(devActions);
    if (!otherActions.empty()) return pick_random(otherActions);
    if (!endTurnActions.empty()) return pick_random(endTurnActions);

    // Fall back to pure-random among all legal actions (e.g., RollDice,
    // TradeBank, etc.).
    int idx = RandomDevice::uniform_u32_range(
        0, static_cast<uint32_t>(actions.size()) - 1);
    return actions[idx];
}
