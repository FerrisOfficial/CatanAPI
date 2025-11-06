#pragma once

#include "game_simulation/board.hpp"

class Display {
public:
    void renderBoard(const Board::BoardState& boardState);
};