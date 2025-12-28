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
    void turnLoop();
    PlayerId runGame();
    
    // Breakdown helpers
    bool processDevPhase(IPlayer& currentPlayer);
    void applyDiceRoll(uint8_t diceNumber);
    void discardResourcesForSeven(PlayerId currentPlayerId);
    void handleRobberPhase(IPlayer& currentPlayer, PlayerId currentPlayerId);
    void processPlayerTurn(IPlayer& currentPlayer);
};