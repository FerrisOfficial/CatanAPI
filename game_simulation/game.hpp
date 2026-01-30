#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "actions.hpp"
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "players/player.hpp"
#include "utils/dumper.hpp"

struct Game {
    Board::BoardState boardState;
    IPlayer& player1;
    IPlayer& player2;

    std::array<std::string, 2> playerDisplayNames{{"Player0", "Player1"}};

    std::unique_ptr<Dumper> dumper;
    bool dumpEnabled = true;

    Game(IPlayer& player1, IPlayer& player2);
    void setDumpEnabled(bool enabled);
    void setPlayerDisplayNames(std::string player0Name,
                               std::string player1Name);
    void initialPhase();
    void turnLoop();
    PlayerId runGame();

    void applyActionLogged(Action::PackedAction action, const char* phase);

    // Breakdown helpers
    bool processDevPhase(IPlayer& currentPlayer);
    void applyDiceRoll(uint8_t diceNumber);
    void discardResourcesForSeven(PlayerId currentPlayerId);
    void handleRobberPhase(IPlayer& currentPlayer, PlayerId currentPlayerId);
    void processPlayerTurn(IPlayer& currentPlayer);
};