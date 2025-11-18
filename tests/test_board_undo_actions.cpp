#include "gtest/gtest.h"
#include "game_simulation/board.hpp"
#include "game_simulation/actions.hpp"
#include "game_simulation/player.hpp"
#include "game_simulation/packedBank.hpp"

using namespace Board;

class UndoActionTest : public testing::TestWithParam<ActionType> {
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

TEST_P(UndoActionTest, ApplyThenUndoActions) {
    ActionType type = GetParam();

    Action::PackedAction action = 0;
    Action::PackedAction undoAction = 0;
    PlayerId pid = PlayerId::Player0;
    PlayerId victim_pid = PlayerId::Player1;

    // Prepare action and initial board state depending on the ActionType
    switch (type) {
    case ActionType::MoveRobber: {
        boardState.robberPosition = 0;

        boardState.packedPlayers[static_cast<uint8_t>(victim_pid)] =
            Player::packResource(boardState.packedPlayers[static_cast<uint8_t>(victim_pid)], Resource::Wool, 5);

        action = Action::packType(action, ActionType::MoveRobber);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 9);
        break;
    }
    case ActionType::StealResource: {
        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packResource(boardState.packedPlayers[static_cast<uint8_t>(pid)], Resource::Brick, 5);
        boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player1)] =
            Player::packResource(boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player1)], Resource::Brick, 2);

        action = Action::packType(action, ActionType::StealResource);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, static_cast<uint8_t>(Resource::Brick));
        break;
    }
    case ActionType::DiscardResources: {
        action = Action::packType(action, ActionType::DiscardResources);
        action = Action::packPlayerID(action, pid);
        action = Action::packResource(action, Resource::Brick, 1);
        action = Action::packResource(action, Resource::Lumber, 2);
        break;
    }
    case ActionType::BuildRoad: {
        setBankResourcesTen();
        setPlayersResourcesSeven();

        action = Action::packType(action, ActionType::BuildRoad);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 10); // EdgeId 10
        break;
    }
    case ActionType::BuildSettlement: {
        setBankResourcesTen();
        setPlayersResourcesSeven();

        action = Action::packType(action, ActionType::BuildSettlement);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 5); // NodeId 5
        break;
    }
    case ActionType::BuildCity: {
        setBankResourcesTen();
        setPlayersResourcesSeven();
        NodeId cityNodeId = 15;

        boardState.packedPlayers[static_cast<size_t>(pid)] = Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<size_t>(pid)],
            StructureType::Settlement,
            3
        );
        boardState.packedPlayers[static_cast<size_t>(pid)] = Player::packVictoryPoints(
            boardState.packedPlayers[static_cast<size_t>(pid)],
            1
        );

        Node::PackedNode& cityNode = boardState.nodes[cityNodeId];
        Node::PackedNode& targetNode = boardState.nodes[cityNodeId];
        targetNode = Node::packStructure(targetNode, StructureType::Settlement);
        targetNode = Node::packOwner(targetNode, pid);

        HexId adjHex[3] = {
            Node::unpackAdjacentHex(cityNode, 0),
            Node::unpackAdjacentHex(cityNode, 1),
            Node::unpackAdjacentHex(cityNode, 2)
        };

        for (HexId h : adjHex) {
            if (h != HexIdNone) {
                boardState.hexes[h] = Hex::packPlayerValue(
                    boardState.hexes[h],
                    pid,
                    1
                );
            }
        }

        action = Action::packType(action, ActionType::BuildCity);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 15); // NodeId 15
        break;
    }
    case ActionType::BuyDevCard: {
        setBankResourcesTen();
        setPlayersResourcesSeven();

        boardState.packedBank = Bank::packDevCard(boardState.packedBank, DevType::Knight, 0);
        boardState.packedBank = Bank::packDevCard(boardState.packedBank, DevType::RoadBuilding, 0);
        boardState.packedBank = Bank::packDevCard(boardState.packedBank, DevType::YearOfPlenty, 0);
        boardState.packedBank = Bank::packDevCard(boardState.packedBank, DevType::Monopoly, 0);
        boardState.packedBank = Bank::packDevCard(boardState.packedBank, DevType::VictoryPoint, 5);
        boardState.packedBank = Bank::packTotalDevCount(boardState.packedBank, 5);

        action = Action::packType(action, ActionType::BuyDevCard);
        action = Action::packPlayerID(action, pid);
        break;
    }
    case ActionType::PlayDevCardKnight: {
        boardState.robberPosition = 1;
        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packDevCard(boardState.packedPlayers[static_cast<uint8_t>(pid)], DevType::Knight, 1);

        boardState.packedPlayers[static_cast<uint8_t>(victim_pid)] =
            Player::packResource(boardState.packedPlayers[static_cast<uint8_t>(victim_pid)], Resource::Wool, 5);

        action = Action::packType(action, ActionType::PlayDevCardKnight);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 12); // New robber HexId
        break;
    }
    case ActionType::PlayDevCardRoadBuilding: {
        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packDevCard(boardState.packedPlayers[static_cast<uint8_t>(pid)], DevType::RoadBuilding, 1);

        action = Action::packType(action, ActionType::PlayDevCardRoadBuilding);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, 20); // First road EdgeId
        action = Action::packArg2(action, 28); // Second road EdgeId
        break;
    }
    case ActionType::PlayDevCardYearOfPlenty: {
        setBankResourcesTen();
        setPlayersResourcesSeven();

        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packDevCard(boardState.packedPlayers[static_cast<uint8_t>(pid)], DevType::YearOfPlenty, 1);

        action = Action::packType(action, ActionType::PlayDevCardYearOfPlenty);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, static_cast<uint8_t>(Resource::Wool));
        action = Action::packArg2(action, static_cast<uint8_t>(Resource::Ore));
        break;
    }
    case ActionType::PlayDevCardMonopoly: {
        setPlayersResourcesSeven();
        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packDevCard(boardState.packedPlayers[static_cast<uint8_t>(pid)], DevType::Monopoly, 1);

        action = Action::packType(action, ActionType::PlayDevCardMonopoly);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, static_cast<uint8_t>(Resource::Grain));
        // record stolen amount so undo can return it
        action = Action::packArg2(action, 4);
        break;
    }
    case ActionType::ReceiveResources: {
        setBankResourcesTen();
        boardState.packedPlayers[static_cast<uint8_t>(pid)] =
            Player::packResource(boardState.packedPlayers[static_cast<uint8_t>(pid)], Resource::Grain, 5);
        action = Action::packType(action, ActionType::ReceiveResources);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, static_cast<uint8_t>(Resource::Grain));
        action = Action::packArg2(action, 3);
        break;
    }
    case ActionType::TradeBank: {
        setBankResourcesTen();
        setPlayersResourcesSeven();

        action = Action::packType(action, ActionType::TradeBank);
        action = Action::packPlayerID(action, pid);
        action = Action::packArg1(action, static_cast<uint8_t>(Resource::Brick));
        action = Action::packArg2(action, static_cast<uint8_t>(Resource::Grain));
        action = Action::packArg3(action, 4); // 4:1 trade
        break;
    }
    default:
        GTEST_SKIP() << "Test not set up for action type";
    }

    // Save copies of the relevant board fields
    auto beforeBank = boardState.packedBank;
    auto beforePlayers = boardState.packedPlayers;
    auto beforeNodes = boardState.nodes;
    auto beforeEdges = boardState.edges;
    auto beforeRobber = boardState.robberPosition;

    boardState.applyAction(action);

    // Build undo action corresponding to the action we applied
    switch (type) {
    case ActionType::MoveRobber:
        undoAction = Action::packType(undoAction, ActionType::UndoMoveRobber);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, 0);
        undoAction = Action::packArg2(undoAction, Resource::Wool);
        break;
    case ActionType::StealResource:
        undoAction = Action::packType(undoAction, ActionType::UndoStealResource);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        break;
    case ActionType::DiscardResources:
        undoAction = Action::packType(undoAction, ActionType::UndoDiscardResources);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packResource(undoAction, Resource::Brick, Action::unpackResource(action, Resource::Brick));
        undoAction = Action::packResource(undoAction, Resource::Lumber, Action::unpackResource(action, Resource::Lumber));
        break;
    case ActionType::BuildRoad:
        undoAction = Action::packType(undoAction, ActionType::UndoBuildRoad);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        break;
    case ActionType::BuildSettlement:
        undoAction = Action::packType(undoAction, ActionType::UndoBuildSettlement);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        break;
    case ActionType::BuildCity:
        undoAction = Action::packType(undoAction, ActionType::UndoBuildCity);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        break;
    case ActionType::BuyDevCard:
        undoAction = Action::packType(undoAction, ActionType::UndoBuyDevCard);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, static_cast<uint8_t>(DevType::VictoryPoint));
        break;
    case ActionType::PlayDevCardKnight:
        undoAction = Action::packType(undoAction, ActionType::UndoPlayDevCardKnight);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, 1);
        undoAction = Action::packArg2(undoAction, Resource::Wool);
        break;
    case ActionType::PlayDevCardRoadBuilding:
        undoAction = Action::packType(undoAction, ActionType::UndoPlayDevCardRoadBuilding);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        undoAction = Action::packArg2(undoAction, Action::unpackArg2(action));
        break;
    case ActionType::PlayDevCardYearOfPlenty:
        undoAction = Action::packType(undoAction, ActionType::UndoPlayDevCardYearOfPlenty);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        undoAction = Action::packArg2(undoAction, Action::unpackArg2(action));
        break;
    case ActionType::PlayDevCardMonopoly:
        undoAction = Action::packType(undoAction, ActionType::UndoPlayDevCardMonopoly);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        // stolen amount is stored in Arg2 above
        undoAction = Action::packArg2(undoAction, Action::unpackArg2(action));
        break;
    case ActionType::TradeBank:
        undoAction = Action::packType(undoAction, ActionType::UndoTradeBank);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        undoAction = Action::packArg2(undoAction, Action::unpackArg2(action));
        undoAction = Action::packArg3(undoAction, Action::unpackArg3(action));
        break;
    case ActionType::ReceiveResources:
        undoAction = Action::packType(undoAction, ActionType::UndoReceiveResources);
        undoAction = Action::packPlayerID(undoAction, pid);
        undoAction = Action::packArg1(undoAction, Action::unpackArg1(action));
        undoAction = Action::packArg2(undoAction, Action::unpackArg2(action));
        break;
    default:
        break;
    }

    std::cout << "Typ undo Action to" << static_cast<int>(Action::unpackType(undoAction)) << std::endl;
    boardState.applyUndoAction(undoAction);

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

    EXPECT_EQ(boardState.robberPosition, beforeRobber);
}

INSTANTIATE_TEST_SUITE_P(
    UndoActions,
    UndoActionTest,
    testing::Values(
        ActionType::MoveRobber,
        ActionType::StealResource,
        ActionType::DiscardResources,
        ActionType::BuildRoad,
        ActionType::BuildSettlement,
        ActionType::BuildCity,
        ActionType::BuyDevCard,
        ActionType::PlayDevCardKnight,
        ActionType::PlayDevCardRoadBuilding,
        ActionType::PlayDevCardYearOfPlenty,
        ActionType::PlayDevCardMonopoly,
        ActionType::TradeBank,
        ActionType::ReceiveResources
    )
);
