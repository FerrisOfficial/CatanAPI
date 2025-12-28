#include "game.hpp"
#include "utils/randomDevice.hpp"
#include "utils/logger.hpp"

Game::Game(IPlayer& p1, IPlayer& p2)
    : player1(p1)
    , player2(p2)
{
    this->player1.boardState = &boardState;
    this->player2.boardState = &boardState;
}

void Game::initialPhase() {
    Logger logger;

    logger.log_players(this->boardState);
    auto p1InitialPlacement = player1.getInitialPlacement();
    logger.log("Player 1 id of initial settlement: ", std::to_string(static_cast<int>(Action::unpackArg1(p1InitialPlacement.second))));
    this->boardState.applyAction(p1InitialPlacement.second);
    this->boardState.applyAction(Action::packType(Action::getEmptyAction(), ActionType::EndTurn));


    logger.log_players(this->boardState);
    auto p2InitialPlacement = player2.getInitialPlacement();
    logger.log("Player 2 id of initial settlement: ", std::to_string(static_cast<int>(Action::unpackArg1(p2InitialPlacement.second))));
    this->boardState.applyAction(p2InitialPlacement.second);
    this->boardState.applyAction(Action::packType(Action::getEmptyAction(), ActionType::EndTurn));

    logger.log_players(this->boardState);
    auto p2SecondInitialPlacement = player2.get2InitialPlacement();
    logger.log("Player 2 id of second initial settlement: ", std::to_string(static_cast<int>(Action::unpackArg1(p2SecondInitialPlacement.second))));
    this->boardState.applyAction(p2SecondInitialPlacement.second);
    this->boardState.applyAction(Action::packType(Action::getEmptyAction(), ActionType::EndTurn));

    logger.log_players(this->boardState);
    auto p1SecondInitialPlacement = player1.get2InitialPlacement();
    logger.log("Player 1 id of second initial settlement: ", std::to_string(static_cast<int>(Action::unpackArg1(p1SecondInitialPlacement.second))));
    this->boardState.applyAction(p1SecondInitialPlacement.second);
    this->boardState.applyAction(Action::packType(Action::getEmptyAction(), ActionType::EndTurn));
}

bool Game::processDevPhase(IPlayer& currentPlayer) {
    Logger logger;
    auto devAction = currentPlayer.getDevAction();
    if (Action::unpackType(devAction) == ActionType::NoAction) {
        return false;
    }
    logger.log("Dev action played: ", actionTypeName(Action::unpackType(devAction)));
    this->boardState.applyAction(devAction);
    return true;
}

void Game::applyDiceRoll(uint8_t diceNumber) {
    auto rollDiceAction = Action::packType(Action::getEmptyAction(), ActionType::RollDice);
    rollDiceAction = Action::packArg1(rollDiceAction, diceNumber);
    this->boardState.applyAction(rollDiceAction);
}

void Game::discardResourcesForSeven(PlayerId currentPlayerId) {
    Logger logger;
    PlayerId enemyPlayerId = (currentPlayerId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    auto currentPacked = this->boardState.packedPlayers[static_cast<uint8_t>(currentPlayerId)];
    auto enemyPlayerPacked = this->boardState.packedPlayers[static_cast<uint8_t>(enemyPlayerId)];

    uint8_t totalResoucesCurrentPlayer = Player::totalResources(currentPacked);
    uint8_t totalResourcesEnemyPlayer = Player::totalResources(enemyPlayerPacked);

    if (totalResoucesCurrentPlayer > 9) {
        auto discardAction = this->player1.getDiscardAction();
        logger.log("Player 1 discards resources: ", std::to_string(static_cast<int>(Action::unpackType(discardAction))));
        this->boardState.applyAction(discardAction);
    }
    if (totalResourcesEnemyPlayer > 9) {
        auto discardAction = this->player2.getDiscardAction();
        logger.log("Player 2 discards resources: ", std::to_string(static_cast<int>(Action::unpackType(discardAction))));
        this->boardState.applyAction(discardAction);
    }
}

void Game::handleRobberPhase(IPlayer& currentPlayer, PlayerId currentPlayerId) {
    Logger logger;
    auto moveRobberAction = currentPlayer.getMoveRobber();
    logger.log("Robber moved: ", actionTypeName(Action::unpackType(moveRobberAction)));
    this->boardState.applyAction(moveRobberAction);

    HexId robberPos = this->boardState.robberPosition;
    Player::PackedPlayer enemyPlayer = (currentPlayerId == PlayerId::Player0) ? 
        this->boardState.packedPlayers[1] : this->boardState.packedPlayers[0];
    auto enemyPlayerValue = (currentPlayerId == PlayerId::Player0) ? 
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player1) :
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player0);

    if (enemyPlayerValue && Player::unpackVictoryPoints(enemyPlayer) >= 3 && Player::totalResources(enemyPlayer) > 0) {
        auto stealResourceAction = Action::packType(Action::getEmptyAction(), ActionType::StealResource);
        logger.log("Resource stolen: ", std::to_string(static_cast<int>(Action::unpackType(stealResourceAction))));
        this->boardState.applyAction(stealResourceAction);
    }
}

void Game::processPlayerTurn(IPlayer& currentPlayer) {
    Logger logger;

    auto action = Action::getEmptyAction();
    do {
        action = currentPlayer.getTurnAction();

        logger.log("NEW ACTION:");
        logger.log_players(this->boardState);
        logger.log("Turn action played: ", actionTypeName(Action::unpackType(action)));
        logger.log_players(this->boardState);
        logger.log("");


        this->boardState.applyAction(action);
    } while (Action::unpackType(action) != ActionType::EndTurn);
}

void Game::turnLoop() {
    PlayerId currentPlayerId = this->boardState.currentPlayer;
    IPlayer& currentPlayer = (currentPlayerId == PlayerId::Player0) ? player1 : player2;

    bool didUseDev = processDevPhase(currentPlayer);

    auto rolledDiceNumber = RandomDevice::rollDices();

    Logger logger;
    logger.log("Dice rolled: ", std::to_string(rolledDiceNumber));

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

    Logger logger;
    
    while (vpP0 < 15 && vpP1 < 15 && actualTurn < 150) 
    {
        logger.log_players(this->boardState);
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
