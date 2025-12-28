#pragma once

#include <utility>
#include "game_simulation/board.hpp"
#include "game_simulation/actions.hpp"

struct IPlayer {
    Board::BoardState* boardState;
    IPlayer();
    virtual ~IPlayer() = default;

    virtual std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() = 0;
    virtual std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement() = 0;
    virtual Action::PackedAction getDevAction() = 0;
    virtual Action::PackedAction getDiscardAction() = 0;
    virtual Action::PackedAction getMoveRobber() = 0;
    virtual Action::PackedAction getTurnAction() = 0;
};
