#include "actions.hpp"
#include "consts.hpp"
#include "board.hpp"   
#include "player.hpp" 
#include "players/player.hpp"

#include <cstdint>

struct Game {
    Board::BoardState boardState;
    IPlayer& player1;
    IPlayer& player2;

    Game(IPlayer& player1, IPlayer& player2);
    void initialPhase();

};