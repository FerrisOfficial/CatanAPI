#include <gtest/gtest.h>

#include "game_simulation/board.hpp"
#include "game_simulation/game.hpp"
#include "game_simulation/player.hpp"
#include "players/randomPlayer.hpp"

// Test RandomPlayer as a concrete IPlayer implementation
TEST(PlayerTest, RandomPlayerInitialization) {
    RandomPlayer player;
    // Test that boardState pointer can be set
    player.boardState = nullptr;
    EXPECT_EQ(player.boardState, nullptr);
}

TEST(PlayerTest, RandomPlayerBoardStateAssignment) {
    RandomPlayer player;
    Board::BoardState testBoard;
    player.boardState = &testBoard;
    EXPECT_NE(player.boardState, nullptr);
    EXPECT_EQ(player.boardState, &testBoard);
}

// Test RandomPlayer implementation
TEST(RandomPlayerTest, RandomPlayerCreation) {
    RandomPlayer player;

    // Test that RandomPlayer inherits from IPlayer correctly
    EXPECT_NO_THROW({
        IPlayer* basePtr = &player;
        (void)basePtr;  // Suppress unused variable warning
    });
}

TEST(RandomPlayerTest, RandomPlayerBoardStateAccess) {
    RandomPlayer player;
    Board::BoardState testBoard;

    player.boardState = &testBoard;
    EXPECT_NE(player.boardState, nullptr);
    EXPECT_EQ(player.boardState, &testBoard);
}

// Test Game structure
TEST(GameTest, GameInitialization) {
    RandomPlayer player1;
    RandomPlayer player2;

    EXPECT_NO_THROW({ Game game(player1, player2); });
}

TEST(GameTest, GameAssignsBoardStateToPlayers) {
    RandomPlayer player1;
    RandomPlayer player2;

    // Initially, players should have null or uninitialized boardState
    player1.boardState = nullptr;
    player2.boardState = nullptr;

    Game game(player1, player2);

    // After Game construction, both players should have the same board
    EXPECT_NE(player1.boardState, nullptr);
    EXPECT_NE(player2.boardState, nullptr);
    EXPECT_EQ(player1.boardState, player2.boardState);
}

TEST(GameTest, GameBoardStateIsValid) {
    RandomPlayer player1;
    RandomPlayer player2;

    Game game(player1, player2);

    // Verify that the board state is properly initialized
    ASSERT_NE(player1.boardState, nullptr);

    // Check that the board has the correct initial values
    EXPECT_EQ(player1.boardState->robberPosition, 0);
    EXPECT_EQ(player1.boardState->currentPlayer, PlayerId::Player1);
    EXPECT_EQ(player1.boardState->currentTurn, 0);
    EXPECT_EQ(player1.boardState->actionQueue.size(), 0u);
}

TEST(GameTest, GameBoardHasCorrectNodeCount) {
    RandomPlayer player1;
    RandomPlayer player2;

    Game game(player1, player2);

    ASSERT_NE(player1.boardState, nullptr);

    // Verify that all nodes are initialized (check a few)
    // Node 0 should connect to hexes: HexIdNone, 0, HexIdNone
    EXPECT_EQ(Board::Node::unpackAdjacentHex(player1.boardState->nodes[0], 1),
              0);

    // Node 10 should connect to hexes: 0, 1, 4
    EXPECT_EQ(Board::Node::unpackAdjacentHex(player1.boardState->nodes[10], 0),
              0);
    EXPECT_EQ(Board::Node::unpackAdjacentHex(player1.boardState->nodes[10], 1),
              1);
    EXPECT_EQ(Board::Node::unpackAdjacentHex(player1.boardState->nodes[10], 2),
              4);
}

TEST(GameTest, GameBoardHasCorrectEdgeCount) {
    RandomPlayer player1;
    RandomPlayer player2;

    Game game(player1, player2);

    ASSERT_NE(player1.boardState, nullptr);

    // Verify that edges are initialized (check a few)
    // Edge 0 connects nodes 0 and 1
    EXPECT_EQ(Board::Edge::unpackAdjacentNode(player1.boardState->edges[0], 0),
              0);
    EXPECT_EQ(Board::Edge::unpackAdjacentNode(player1.boardState->edges[0], 1),
              1);

    // Edge 12 connects nodes 9 and 10
    EXPECT_EQ(Board::Edge::unpackAdjacentNode(player1.boardState->edges[12], 0),
              9);
    EXPECT_EQ(Board::Edge::unpackAdjacentNode(player1.boardState->edges[12], 1),
              10);
}

TEST(GameTest, GameBoardPlayersInitialized) {
    RandomPlayer player1;
    RandomPlayer player2;

    Game game(player1, player2);

    ASSERT_NE(player1.boardState, nullptr);

    // Check that packed players are initialized
    auto& packedPlayer0 = player1.boardState->packedPlayers[0];
    auto& packedPlayer1 = player1.boardState->packedPlayers[1];

    // New players should start with no resources
    EXPECT_EQ(Player::unpackResource(packedPlayer0, Resource::Brick), 0);
    EXPECT_EQ(Player::unpackResource(packedPlayer0, Resource::Lumber), 0);
    EXPECT_EQ(Player::unpackResource(packedPlayer0, Resource::Wool), 0);
    EXPECT_EQ(Player::unpackResource(packedPlayer0, Resource::Grain), 0);
    EXPECT_EQ(Player::unpackResource(packedPlayer0, Resource::Ore), 0);

    // Victory points should be 0
    EXPECT_EQ(Player::unpackVictoryPoints(packedPlayer0), 0);
    EXPECT_EQ(Player::unpackVictoryPoints(packedPlayer1), 0);
}

// Test that multiple games can be created independently
TEST(GameTest, MultipleGamesAreIndependent) {
    RandomPlayer player1a, player2a;
    RandomPlayer player1b, player2b;

    Game game1(player1a, player2a);
    Game game2(player1b, player2b);

    // Each game should have its own board
    EXPECT_NE(player1a.boardState, nullptr);
    EXPECT_NE(player1b.boardState, nullptr);
    EXPECT_NE(player1a.boardState, player1b.boardState);
}
