#pragma once

#include <utility>
#include "game_simulation/board.hpp"
#include "game_simulation/actions.hpp"

struct IPlayer {
    Board::BoardState* boardState;
    IPlayer();

    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement();
    std::pair<Action::PackedAction, Action::PackedAction> get2InitialPlacement();
    Action::PackedAction getDevAction();
    Action::PackedAction getDiscardAction();
    Action::PackedAction getMoveRobber();
    Action::PackedAction getTurnAction();
};
