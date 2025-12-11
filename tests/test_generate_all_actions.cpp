#include <gtest/gtest.h>
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "actions.hpp"

using namespace Board;

class ActionTest : public ::testing::Test {
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

TEST_F(ActionTest, ExpectTradeActionsWithWoolPort){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId tradingPlayer = PlayerId::Player0;

    // Give player a settlement on a wool port
    NodeId woolPortNodeId = 7;
    boardState.nodes[woolPortNodeId] = Node::packStructure(boardState.nodes[woolPortNodeId], StructureType::Settlement);
    boardState.nodes[woolPortNodeId] = Node::packOwner(boardState.nodes[woolPortNodeId], tradingPlayer);

    std::vector<Action::PackedAction> tradeActions;

    tradeActions = boardState.generateTwoToOnePortTradeActions(tradingPlayer);

    for (const auto& action : tradeActions) {
        EXPECT_EQ(ActionType::TradeBank, Action::unpackType(action));
        EXPECT_EQ(tradingPlayer, Action::unpackPlayerID(action));
        EXPECT_EQ(Resource::Wool, static_cast<Resource>(Action::unpackArg1(action)));
        EXPECT_EQ(2, Action::unpackArg3(action));
    }
}

TEST_F(ActionTest, ExpectBuyDevCardActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buyDevCardActions;

    buyDevCardActions = boardState.generateDevCardActions(PlayerId::Player0);

    EXPECT_EQ(7, buyDevCardActions.size());
    for (const auto& action : buyDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuyDevCard, type);
    }
}