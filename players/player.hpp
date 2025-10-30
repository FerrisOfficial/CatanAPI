#pragma once

#include "game_simulation/board.hpp"
#include "game_simulation/actions.hpp"

struct IPlayer {
    Board::BoardState* boardState;
    IPlayer();

    Action::PackedAction getInitialSettlement();
    Action::PackedAction getInitialRoad();
    Action::PackedAction get2InitialSettlement();
    Action::PackedAction get2InitialRoad();
};
