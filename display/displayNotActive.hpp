#pragma once

#include "game_simulation/board.hpp"
#include <iostream>

class Display {
public:
    void renderBoard(const Board::BoardState& /*boardState*/) {
        std::cout << "displayNotActive" << std::endl;
    }
};