#pragma once
#include "player.hpp"
#include "game_simulation/board.hpp"

struct SettlePrioGreedyPlayer : public IPlayer {
    SettlePrioGreedyPlayer() : IPlayer() {}
    virtual ~SettlePrioGreedyPlayer() = default;

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getDevAction() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getMoveRobber() override;
    Action::PackedAction getTurnAction() override;
};
