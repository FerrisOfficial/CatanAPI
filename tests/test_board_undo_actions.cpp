#include "gtest/gtest.h"
#include "game_simulation/board.hpp"
#include "game_simulation/actions.hpp"
#include "game_simulation/player.hpp"
#include "game_simulation/packedBank.hpp"
#include <array>

using namespace Board;

class UndoLastActionTest : public testing::TestWithParam<Action::PackedAction> {
protected:
    Board::BoardState boardState;

    void SetUp() override {
        boardState.generateRandomBoard();
    }

    void setBankResourcesTen() {
        for (size_t res = 0; res < 5; ++res) {
            boardState.packedBank = Bank::packResource(boardState.packedBank, static_cast<Resource>(res), 10);
        }
    }

    void setPlayersResourcesSeven() {
        for (size_t p = 0; p < 2; ++p) {
            for (size_t res = 0; res < 5; ++res) {
                boardState.packedPlayers[p] = Player::packResource(
                    boardState.packedPlayers[p],
                    static_cast<Resource>(res),
                    7
                );
            }
        }
    }
};

constexpr Action::PackedAction endTurn =
    Action::packPlayerID(
        Action::packType(0, ActionType::EndTurn),
        PlayerId::Player0);

constexpr Action::PackedAction rollDice =
    Action::packPlayerID(
        Action::packType(0, ActionType::RollDice),
        PlayerId::Player0);

constexpr Action::PackedAction moveRobber =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::MoveRobber),
            PlayerId::Player0),
        9);

constexpr Action::PackedAction stealResource =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::StealResource),
            PlayerId::Player0),
        static_cast<uint8_t>(Resource::Brick));

constexpr Action::PackedAction discardResources =
    Action::packResource(
        Action::packResource(
            Action::packPlayerID(
                Action::packType(0, ActionType::DiscardResources),
                PlayerId::Player0),
            Resource::Brick,
            1),
        Resource::Lumber,
        2);

constexpr Action::PackedAction buildRoad =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::BuildRoad),
            PlayerId::Player0),
        10);

constexpr Action::PackedAction buildSettlement =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::BuildSettlement),
            PlayerId::Player0),
        5);

constexpr Action::PackedAction buildCity =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::BuildCity),
            PlayerId::Player0),
        15);

constexpr Action::PackedAction buyDevCard =
    Action::packPlayerID(
        Action::packType(0, ActionType::BuyDevCard),
        PlayerId::Player0);

constexpr Action::PackedAction playDevCardKnight =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::PlayDevCardKnight),
            PlayerId::Player0),
        8);

constexpr Action::PackedAction playDevCardRoadBuilding =
    Action::packArg2(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::PlayDevCardRoadBuilding),
                PlayerId::Player0),
            20),
        28);

constexpr Action::PackedAction playDevCardYearOfPlenty =
    Action::packArg2(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::PlayDevCardYearOfPlenty),
                PlayerId::Player0),
            static_cast<uint8_t>(Resource::Wool)),
        static_cast<uint8_t>(Resource::Ore));

constexpr Action::PackedAction playDevCardMonopoly =
    Action::packArg1(
        Action::packPlayerID(
            Action::packType(0, ActionType::PlayDevCardMonopoly),
            PlayerId::Player0),
        static_cast<uint8_t>(Resource::Grain));

constexpr Action::PackedAction receiveResources =
    Action::packArg2(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::ReceiveResources),
                PlayerId::Player0),
            static_cast<uint8_t>(Resource::Grain)),
        3);

constexpr Action::PackedAction tradeBank =
    Action::packArg3(
        Action::packArg2(
            Action::packArg1(
                Action::packPlayerID(
                    Action::packType(0, ActionType::TradeBank),
                    PlayerId::Player0),
                static_cast<uint8_t>(Resource::Lumber)),
            static_cast<uint8_t>(Resource::Ore)),
        2); // 2:1 trade

constexpr Action::PackedAction placeInitialStructures =
    Action::packArg2(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::PlaceInitialStructures),
                PlayerId::Player0),
            30), // NodeId
        50); // EdgeId

constexpr Action::PackedAction place2InitialStructures =
    Action::packArg2(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::Place2InitialStructures),
                PlayerId::Player0),
            22), // NodeId
        29); // EdgeId

constexpr std::array<Action::PackedAction, 17> allActions = {
    endTurn,
    rollDice,
    moveRobber,
    stealResource,
    discardResources,
    buildRoad,
    buildSettlement,
    buildCity,
    buyDevCard,
    playDevCardKnight,
    playDevCardRoadBuilding,
    playDevCardYearOfPlenty,
    playDevCardMonopoly,
    receiveResources,
    tradeBank,
    placeInitialStructures,
    place2InitialStructures
};


TEST_P(UndoLastActionTest, ExpectUndoLastAction){
    Action::PackedAction action = GetParam();

    Board::BoardState beforeBoardState = boardState;
    auto beforeBank = boardState.packedBank;
    auto beforePlayers = boardState.packedPlayers;
    auto beforeNodes = boardState.nodes;
    auto beforeEdges = boardState.edges;
    auto beforeRobber = boardState.robberPosition;

    boardState.applyAction(action);
    boardState.undoLastAction();

    EXPECT_EQ(boardState.packedBank, beforeBank);
    // players
    for (size_t i = 0; i < sizeof(boardState.packedPlayers) / sizeof(boardState.packedPlayers[0]); ++i) {
        EXPECT_EQ(boardState.packedPlayers[i], beforePlayers[i]);
    }

    // nodes
    for (size_t i = 0; i < sizeof(boardState.nodes) / sizeof(boardState.nodes[0]); ++i) {
        EXPECT_EQ(boardState.nodes[i], beforeNodes[i]);
    }

    // edges
    for (size_t i = 0; i < sizeof(boardState.edges) / sizeof(boardState.edges[0]); ++i) {
        EXPECT_EQ(boardState.edges[i], beforeEdges[i]);
    }

    EXPECT_EQ(boardState.packedPlayers[0], beforePlayers[0]);
    EXPECT_EQ(boardState.packedPlayers[1], beforePlayers[1]);

    EXPECT_EQ(boardState.robberPosition, beforeRobber);
}

INSTANTIATE_TEST_SUITE_P(
    UndoLastAction,
    UndoLastActionTest,
    testing::ValuesIn(allActions)
);

TEST_F(UndoLastActionTest, ActionQueueShrinksAfterUndo) {
    for (auto a : allActions) {
        uint16_t beforeSize = boardState.actionQueueSize;
        boardState.applyAction(a);
        EXPECT_EQ(boardState.actionQueueSize, beforeSize + 1);
        boardState.undoLastAction();
        EXPECT_EQ(boardState.actionQueueSize, beforeSize);
    }
}
