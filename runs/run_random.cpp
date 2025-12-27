#include "game_simulation/game.hpp"
#include "players/random_player.hpp"

int main() {
    RandomPlayer player1;
    RandomPlayer player2;
    Game game(player1, player2);
    
    auto result = game.runGame();
    return result;
}