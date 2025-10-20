#include "game.hpp"

Game::Game(IPlayer& player1, IPlayer& player2) {
    player1.boardState = &boardState;
    player2.boardState = &boardState;
}