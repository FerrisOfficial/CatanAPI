#pragma once
#include "player.hpp"
#include "game_simulation/board.hpp"
#include "utils/randomDevice.hpp"
#include "utils/logger.hpp"

struct RandomPlayer : public IPlayer {
    RandomPlayer() : IPlayer() {}
    virtual ~RandomPlayer() = default;
    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() override;
    Action::PackedAction getDevAction() override;
    Action::PackedAction getDiscardAction() override;
    Action::PackedAction getMoveRobber() override;
    Action::PackedAction getTurnAction() override;

    Logger logger;
};
