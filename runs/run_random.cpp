#include "game_simulation/game.hpp"
#include "players/randomPlayer.hpp"

int main() {
    RandomPlayer player1;
    RandomPlayer player2;
    Game game(player1, player2);

    auto result = game.runGame();
    return static_cast<int>(result);
}