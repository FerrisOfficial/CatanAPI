#include "game_simulation/game.hpp"
#include "players/randomPlayer.hpp"
#include "utils/logger.hpp"

int main() {
    RandomPlayer player1;
    RandomPlayer player2;
    Game game(player1, player2);
    
    Logger logger;
    logger.clean_log();
    logger.log("Starting a new game between two RandomPlayers.");

    auto result = game.runGame();
    return static_cast<int>(result);
}