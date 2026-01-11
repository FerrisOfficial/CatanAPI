#pragma once

#include <cstdint>
#include "../game_simulation/board.hpp"

// Forward declarations
enum class PlayerId : uint8_t;

namespace PlayerHelpers {

uint8_t dice_pips(uint8_t diceNumber);

int effective_vp(const Board::BoardState* board, PlayerId pid);

inline int effective_vp(const Board::BoardState& board, PlayerId pid) {
    return effective_vp(&board, pid);
}

} // namespace PlayerHelpers
