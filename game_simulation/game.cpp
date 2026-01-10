#include "game.hpp"
#include "utils/randomDevice.hpp"

#include <algorithm>

#include "utils/dumper.hpp"

Game::Game(IPlayer& p1, IPlayer& p2)
    : player1(p1)
    , player2(p2)
{
    this->player1.boardState = &boardState;
    this->player2.boardState = &boardState;
}

void Game::setPlayerDisplayNames(std::string player0Name, std::string player1Name) {
    if (!player0Name.empty()) {
        playerDisplayNames[0] = std::move(player0Name);
    }
    if (!player1Name.empty()) {
        playerDisplayNames[1] = std::move(player1Name);
    }
}

void Game::applyActionLogged(Action::PackedAction action, const char* phase) {
    this->boardState.applyAction(action);
    if (this->dumper) {
        this->dumper->recordActionApplied(action, this->boardState, phase ? phase : "");
    }
}

void Game::initialPhase() {
    auto p1InitialPlacement = player1.getInitialPlacement();
    applyActionLogged(p1InitialPlacement.second, "initialPhase");
    applyActionLogged(Action::packType(Action::getEmptyAction(), ActionType::EndTurn), "initialPhase");

    auto p2InitialPlacement = player2.getInitialPlacement();
    applyActionLogged(p2InitialPlacement.second, "initialPhase");
    applyActionLogged(Action::packType(Action::getEmptyAction(), ActionType::EndTurn), "initialPhase");

    auto p2SecondInitialPlacement = player2.get2InitialPlacement();
    applyActionLogged(p2SecondInitialPlacement.second, "initialPhase");
    applyActionLogged(Action::packType(Action::getEmptyAction(), ActionType::EndTurn), "initialPhase");

    auto p1SecondInitialPlacement = player1.get2InitialPlacement();
    applyActionLogged(p1SecondInitialPlacement.second, "initialPhase");
    applyActionLogged(Action::packType(Action::getEmptyAction(), ActionType::EndTurn), "initialPhase");
}

bool Game::processDevPhase(IPlayer& currentPlayer) {
    auto devAction = currentPlayer.getDevAction();
    if (Action::unpackType(devAction) == ActionType::NoAction) {
        return false;
    }
    applyActionLogged(devAction, "devPhase");
    return true;
}

void Game::applyDiceRoll(uint8_t diceNumber) {
    auto rollDiceAction = Action::packType(Action::getEmptyAction(), ActionType::RollDice);
    rollDiceAction = Action::packArg1(rollDiceAction, diceNumber);
    applyActionLogged(rollDiceAction, "dice");
}

void Game::discardResourcesForSeven(PlayerId currentPlayerId) {
    (void)currentPlayerId;

    auto buildDiscardAction = [&](PlayerId discardingPlayerId) {
        const auto packed = this->boardState.packedPlayers[static_cast<uint8_t>(discardingPlayerId)];
        const uint8_t totalResources = Player::totalResources(packed);
        if (totalResources <= 7) {
            return Action::getEmptyAction();
        }

        const uint8_t toDiscard = totalResources / 2;

        const PlayerId savedCurrentPlayer = this->boardState.currentPlayer;
        this->boardState.currentPlayer = discardingPlayerId;
        IPlayer& discardingPlayer = (discardingPlayerId == PlayerId::Player0) ? this->player1 : this->player2;
        const auto proposed = discardingPlayer.getDiscardAction();
        this->boardState.currentPlayer = savedCurrentPlayer;

        Action::PackedAction action = Action::getEmptyAction();
        uint8_t discarded = 0;

        for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
            const uint8_t want = Action::unpackResource(proposed, r);
            const uint8_t have = Player::unpackResource(packed, r);
            const uint8_t give = std::min<uint8_t>(std::min<uint8_t>(want, have), static_cast<uint8_t>(toDiscard - discarded));
            action = Action::packResource(action, r, give);
            discarded = static_cast<uint8_t>(discarded + give);
        }

        if (discarded < toDiscard) {
            for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
                const uint8_t have = Player::unpackResource(packed, r);
                const uint8_t current = Action::unpackResource(action, r);
                const uint8_t available = static_cast<uint8_t>(have - current);
                const uint8_t add = std::min<uint8_t>(available, static_cast<uint8_t>(toDiscard - discarded));
                action = Action::packResource(action, r, static_cast<uint8_t>(current + add));
                discarded = static_cast<uint8_t>(discarded + add);
                if (discarded >= toDiscard) {
                    break;
                }
            }
        }

        action = Action::packType(action, ActionType::DiscardResources);
        action = Action::packPlayerID(action, discardingPlayerId);
        return action;
    };

    for (PlayerId discardingPlayerId : {PlayerId::Player0, PlayerId::Player1}) {
        auto discardAction = buildDiscardAction(discardingPlayerId);
        if (Action::unpackType(discardAction) == ActionType::DiscardResources) {
            applyActionLogged(discardAction, "discardSeven");
        }
    }
}

void Game::handleRobberPhase(IPlayer& currentPlayer, PlayerId currentPlayerId) {
    auto moveRobberAction = currentPlayer.getMoveRobber();
    applyActionLogged(moveRobberAction, "robber");

    HexId robberPos = this->boardState.robberPosition;
    Player::PackedPlayer enemyPlayer = (currentPlayerId == PlayerId::Player0) ? 
        this->boardState.packedPlayers[1] : this->boardState.packedPlayers[0];
    auto enemyPlayerValue = (currentPlayerId == PlayerId::Player0) ? 
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player1) :
        Board::Hex::unpackPlayerValue(this->boardState.hexes[robberPos], PlayerId::Player0);

    if (enemyPlayerValue && Player::unpackVictoryPoints(enemyPlayer) >= 3 && Player::totalResources(enemyPlayer) > 0) {
        auto stealResourceAction = Action::packType(Action::getEmptyAction(), ActionType::StealResource);
        applyActionLogged(stealResourceAction, "robber");
    }
}

void Game::processPlayerTurn(IPlayer& currentPlayer) {
    auto action = Action::getEmptyAction();
    do {
        action = currentPlayer.getTurnAction();
        applyActionLogged(action, "turn");
    } while (Action::unpackType(action) != ActionType::EndTurn);
}

void Game::turnLoop() {
    PlayerId currentPlayerId = this->boardState.currentPlayer;
    IPlayer& currentPlayer = (currentPlayerId == PlayerId::Player0) ? player1 : player2;

    if (this->dumper) {
        this->dumper->recordTurnStart(this->boardState);
    }

    bool didUseDev = processDevPhase(currentPlayer);

    auto rolledDiceNumber = RandomDevice::rollDices();

    if (this->dumper) {
        this->dumper->recordDiceRoll(static_cast<uint8_t>(rolledDiceNumber), this->boardState);
    }

    if (rolledDiceNumber != 7) {
        applyDiceRoll(rolledDiceNumber);
    } else {
        discardResourcesForSeven(currentPlayerId);
        handleRobberPhase(currentPlayer, currentPlayerId);
    }

    if (!didUseDev)
        processDevPhase(currentPlayer);

    processPlayerTurn(currentPlayer);

    if (this->dumper) {
        this->dumper->recordTurnEnd(this->boardState);
    }
}

PlayerId Game::runGame() {
    this->dumper = std::make_unique<Dumper>("logs");

    this->dumper->setPlayerNames(playerDisplayNames[0], playerDisplayNames[1]);
    this->dumper->recordPlayersInfo();

    this->boardState.generateRandomBoard();
    this->dumper->recordInitialState(this->boardState);
    this->initialPhase();
    
    auto actualTurn = this->boardState.currentTurn;
    auto vpP0 = Player::unpackVictoryPoints(this->boardState.packedPlayers[0]);
    auto vpP1 = Player::unpackVictoryPoints(this->boardState.packedPlayers[1]);
    
    while (vpP0 < 15 && vpP1 < 15 && actualTurn < 1000) 
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
        if (this->dumper) this->dumper->recordGameEnd(PlayerId::Player0, this->boardState);
        return PlayerId::Player0;
    } 
    if (vpP1 >= 15) 
    {
        if (this->dumper) this->dumper->recordGameEnd(PlayerId::Player1, this->boardState);
        return PlayerId::Player1;
    } 

    if (this->dumper) this->dumper->recordGameEnd(PlayerId::NoPlayer, this->boardState);
    return PlayerId::NoPlayer;
}
