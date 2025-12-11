#include "game.hpp"
#include "randomDevice.hpp"

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

    auto p2SecondInitialSettlement = player2.get2InitialSettlement();
    auto p2SecondInitialRoad = player2.get2InitialRoad();
    this->boardState.applyAction(p2SecondInitialSettlement);
    this->boardState.applyAction(p2SecondInitialRoad);

    auto p1SecondInitialSettlement = player1.get2InitialSettlement();
    auto p1SecondInitialRoad = player1.get2InitialRoad();
    this->boardState.applyAction(p1SecondInitialSettlement);
    this->boardState.applyAction(p1SecondInitialRoad);
}

void Game::processDevPhase(IPlayer& currentPlayer) {
    auto devAction = currentPlayer.getDevAction();
    this->boardState.applyAction(devAction);
}

void Game::applyDiceRoll(uint8_t diceNumber) {
    auto rollDiceAction = Action::packType(Action::getEmptyAction(), ActionType::RollDice);
    rollDiceAction = Action::packArg1(rollDiceAction, diceNumber);
    this->boardState.applyAction(rollDiceAction);
}

void Game::discardResourcesForSeven(PlayerId currentPlayerId) {
    PlayerId enemyPlayerId = (currentPlayerId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    auto currentPacked = this->boardState.packedPlayers[static_cast<uint8_t>(currentPlayerId)];
    auto enemyPlayerPacked = this->boardState.packedPlayers[static_cast<uint8_t>(enemyPlayerId)];

    uint8_t totalResoucesCurrentPlayer = Player::totalResources(currentPacked);
    uint8_t totalResourcesEnemyPlayer = Player::totalResources(enemyPlayerPacked);

    if (totalResoucesCurrentPlayer > 9) {
        auto discardAction = this->player1.getDiscardAction();
        this->boardState.applyAction(discardAction);
    }
    
    if (totalResourcesEnemyPlayer > 9) {
        auto discardAction = this->player2.getDiscardAction();
        this->boardState.applyAction(discardAction);
    }
}

void Game::handleRobberPhase(IPlayer& currentPlayer, PlayerId currentPlayerId) {
    auto moveRobberAction = currentPlayer.getMoveRobber();
    this->boardState.applyAction(moveRobberAction);

    HexId robberPos = this->boardState.robberPosition;
    Player::PackedPlayer enemyPlayer = (currentPlayerId == PlayerId::Player0) ? 
        this->boardState.packedPlayers[1] : this->boardState.packedPlayers[0];
    auto enemyPlayerValue = (currentPlayerId == PlayerId::Player0) ? 
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player1) :
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player0);

    if (enemyPlayerValue && Player::unpackVictoryPoints(enemyPlayer) >= 3 && Player::totalResources(enemyPlayer) > 0) {
        auto stealResourceAction = Action::packType(Action::getEmptyAction(), ActionType::StealResource);
        this->boardState.applyAction(stealResourceAction);
    }
}

void Game::processPlayerTurn(IPlayer& currentPlayer) {
    auto action = Action::getEmptyAction();
    do {
        action = currentPlayer.getTurnAction();
        this->boardState.applyAction(action);
    } while (Action::unpackType(action) != ActionType::EndTurn);
}

void Game::turnLoop() {
    PlayerId currentPlayerId = this->boardState.currentPlayer;
    IPlayer& currentPlayer = (currentPlayerId == PlayerId::Player0) ? player1 : player2;

    processDevPhase(currentPlayer);

    auto rolledDiceNumber = RandomDevice::rollDices();
    if (rolledDiceNumber != 7) {
        applyDiceRoll(rolledDiceNumber);
    } else {
        discardResourcesForSeven(currentPlayerId);
        handleRobberPhase(currentPlayer, currentPlayerId);
    }

    processPlayerTurn(currentPlayer);
}

PlayerId Game::runGame() {
    this->boardState.generateRandomBoard();
    this->initialPhase();
    
    auto actualTurn = this->boardState.currentTurn;
    auto vpP0 = Player::unpackVictoryPoints(this->boardState.packedPlayers[0]);
    auto vpP1 = Player::unpackVictoryPoints(this->boardState.packedPlayers[1]);

    while (vpP0 < 15 && vpP1 < 15 && actualTurn < 150) 
    {
        this->turnLoop();
        vpP0 = Player::unpackVictoryPoints(this->boardState.packedPlayers[0]);
        vpP1 = Player::unpackVictoryPoints(this->boardState.packedPlayers[1]);
        actualTurn = this->boardState.currentTurn;
    }

    if (vpP0 >= 15) 
    {
        return PlayerId::Player0;
    } 
    if (vpP1 >= 15) 
    {
        return PlayerId::Player1;
    } 
        
    return PlayerId::NoPlayer;
}
