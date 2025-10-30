#include "game.hpp"

Game::Game(IPlayer& p1, IPlayer& p2)
    : player1(p1)
    , player2(p2)
{
    this->player1.boardState = &boardState;
    this->player2.boardState = &boardState;
}

void Game::initialPhase() {
    auto p1InitialSettlement = player1.getInitialSettlement();
    auto p1InitialRoad = player1.getInitialRoad();
    this->boardState.applyAction(p1InitialSettlement);
    this->boardState.applyAction(p1InitialRoad);

    auto p2InitialSettlement = player2.getInitialSettlement();
    auto p2InitialRoad = player2.getInitialRoad();
    this->boardState.applyAction(p2InitialSettlement);
    this->boardState.applyAction(p2InitialRoad);

    auto p2InitialSettlement = player2.get2InitialSettlement();
    auto p2InitialRoad = player2.get2InitialRoad();
    this->boardState.applyAction(p2InitialSettlement);
    this->boardState.applyAction(p2InitialRoad);

    auto p1InitialSettlement = player1.get2InitialSettlement();
    auto p1InitialRoad = player1.get2InitialRoad();
    this->boardState.applyAction(p1InitialSettlement);
    this->boardState.applyAction(p1InitialRoad);
}