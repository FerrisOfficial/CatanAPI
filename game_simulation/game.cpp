#include "game.hpp"
#include "utils/randomDevice.hpp"

Game::Game(IPlayer& p1, IPlayer& p2)
    : player1(p1)
    , player2(p2)
{
    this->player1.boardState = &boardState;
    this->player2.boardState = &boardState;
}

void Game::initialPhase() {
    auto p1InitialPlacement = player1.getInitialPlacement();
    this->boardState.applyAction(p1InitialPlacement.first);
    this->boardState.applyAction(p1InitialPlacement.second);

    auto p2InitialPlacement = player2.getInitialPlacement();
    this->boardState.applyAction(p2InitialPlacement.first);
    this->boardState.applyAction(p2InitialPlacement.second);

    auto p2SecondInitialPlacement = player2.get2InitialPlacement();
    this->boardState.applyAction(p2SecondInitialPlacement.first);
    this->boardState.applyAction(p2SecondInitialPlacement.second);

    auto p1SecondInitialPlacement = player1.get2InitialPlacement();
    this->boardState.applyAction(p1SecondInitialPlacement.first);
    this->boardState.applyAction(p1SecondInitialPlacement.second);
}

bool Game::processDevPhase(IPlayer& currentPlayer) {
    auto devAction = currentPlayer.getDevAction();

    if (Action::unpackType(devAction) == ActionType::NoAction) {
        return false;
    }

    this->boardState.applyAction(devAction);
    return true;
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

    bool didUseDev = processDevPhase(currentPlayer);

    auto rolledDiceNumber = RandomDevice::rollDices();
    if (rolledDiceNumber != 7) {
        applyDiceRoll(rolledDiceNumber);
    } else {
        discardResourcesForSeven(currentPlayerId);
        handleRobberPhase(currentPlayer, currentPlayerId);
    }

    if (!didUseDev)
        processDevPhase(currentPlayer);

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
        if (Player::unpackLargestArmyFlag(this->boardState.packedPlayers[0])) {
            vpP0 += 2;
        }
        if (Player::unpackLargestArmyFlag(this->boardState.packedPlayers[1])) {
            vpP1 += 2;
        }
        if (Player::unpackLongestRoadFlag(this->boardState.packedPlayers[0])) {
            vpP0 += 2;
        }
        if (Player::unpackLongestRoadFlag(this->boardState.packedPlayers[1])) {
            vpP1 += 2;
    }
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
