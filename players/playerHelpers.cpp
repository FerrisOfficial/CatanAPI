#include "playerHelpers.hpp"

#include "../game_simulation/board.hpp"
#include "../game_simulation/player.hpp"

namespace PlayerHelpers {

uint8_t dice_pips(uint8_t diceNumber) {
    switch (diceNumber) {
        case 2:  return 1;
        case 3:  return 2;
        case 4:  return 3;
        case 5:  return 4;
        case 6:  return 5;
        case 8:  return 5;
        case 9:  return 4;
        case 10: return 3;
        case 11: return 2;
        case 12: return 1;
        default: return 0; // includes 7 and invalid
    }
}

int effective_vp(const Board::BoardState* board, PlayerId pid) {
    const auto p = board->packedPlayers[static_cast<uint8_t>(pid)];
    int vp = static_cast<int>(Player::unpackVictoryPoints(p));
    if (Player::unpackLargestArmyFlag(p)) vp += 2;
    if (Player::unpackLongestRoadFlag(p)) vp += 2;
    return vp;
}

} // namespace PlayerHelpers
