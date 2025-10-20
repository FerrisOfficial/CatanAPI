#pragma once

#include "game_simulation/board.hpp"

struct IPlayer {
    Board::BoardState* boardState;
    IPlayer();
};
