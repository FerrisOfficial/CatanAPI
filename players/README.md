# Players Directory

This directory contains all player implementations (bots) for the Catan game simulation. Each bot inherits from the `IPlayer` interface and implements strategies for playing the game.

## Table of Contents

- [Introduction](#introduction)
- [IPlayer Interface](#iplayer-interface)
- [Board Methods Available to Players](#board-methods-available-to-players)
- [Implemented Bots](#implemented-bots)
- [Creating a New Bot](#creating-a-new-bot)

## Introduction

Bots are AI players that can participate in Catan games. They implement different strategies, from simple random moves to complex algorithms like alpha-beta pruning. All bots inherit from the abstract `IPlayer` class and must implement its pure virtual methods.

## IPlayer Interface

The `IPlayer` class defines the interface that all bots must implement. It provides methods for different phases of the game:

### Implemented Methods

- `getInitialPlacement()`: Returns initial settlement and road placements (2 pairs of actions).
- `get2InitialPlacement()`: Returns second initial settlement and road placements.
- `getTurnAction()`: Returns the main action for a turn (build, trade, etc.).
- `getDevAction()`: Returns action for playing development cards.
- `getDiscardAction()`: Returns action for discarding resources during robber attacks.
- `getMoveRobber()`: Returns action for moving the robber and stealing.

All methods return `Action::PackedAction` values, which are compact representations of game actions.

## Board Methods Available to Players

Players have access to the `Board::BoardState` through the `boardState` pointer. The board provides various methods for querying and generating actions:

### Query Methods
- `getLegalActions(playerId)`: Get all legal actions for a player.
- `getCurrentPlayer()`: Get the current player ID.
- `getCurrentTurn()`: Get the current turn number.

### Action Generation Methods
- `generatePlaceInitialStructures(playerId)`: Generate initial settlement/road placements.
- `generatePlace2InitialStructures(playerId)`: Generate second initial placements.
- `generateBuildActions(playerId)`: Generate building actions (settlements, cities, roads).
- `generateTradeActions(playerId)`: Generate trading actions.
- `generatePlayDevCardActions(playerId)`: Generate development card playing actions.
- `generateMoveRobberActions(playerId)`: Generate robber movement actions.

### Helper Methods
- `getPlayerResources(playerId)`: Get player's current resources.
- `getPlayerDevCards(playerId)`: Get player's development cards.
- `getLongestRoadPlayer()`: Get player with longest road.
- `getLargestArmyPlayer()`: Get player with largest army.

## Implemented Bots

### Basic Bots

#### RandomPlayer
- **Strategy**: Makes completely random legal moves.
- **Use**: Baseline for testing, very fast.
- **Strength**: Poor, but can occasionally win by luck.

#### RoadPlayer
- **Strategy**: Prioritizes building roads to achieve longest road victory.
- **Use**: Good for testing road-building mechanics.
- **Strength**: Strong in road-focused games.

### Iterative Bots (itPlayers/)

These bots represent incremental improvements over the basic random strategy:

#### It1Player
- **Strategy**: Same as RandomPlayer (baseline).

#### It2Player
- **Strategy**: Improved initial placements + better robber management.

#### It3Player
- **Strategy**: It2 + smarter development card usage.

#### It4Player
- **Strategy**: It3 + optimized development card playing.

#### It5Player
- **Strategy**: Full strategy implementation with all improvements.

### Specialized Bots (oneTacticPlayers/)

#### AlphaBetaPlayer
- **Strategy**: Uses alpha-beta pruning algorithm for move evaluation.
- **Use**: Advanced AI with look-ahead capabilities.
- **Strength**: Very strong, but slower.

#### CityRushPlayer
- **Strategy**: Focuses on building cities as quickly as possible.
- **Use**: Good for city-focused victory conditions.
- **Strength**: Strong in resource-rich games.

#### DevPlayer
- **Strategy**: Prioritizes acquiring and playing development cards.
- **Use**: Good for development card heavy strategies.
- **Strength**: Strong in games with many dev cards.

#### OneResourcePlayer
- **Strategy**: Focuses on maximizing production of a single resource type.
- **Use**: Testing resource monopolization.
- **Strength**: Situational, very strong in specific board layouts.

### Parametric Bots (parametricPlayers/)

#### ParaPlayer
- **Strategy**: Uses configurable parameters for decision making.
- **Use**: Research and optimization of bot parameters.
- **Strength**: Variable, can be tuned for high performance.

#### ParaSetit5Player
- **Strategy**: Advanced parametric bot with more parameters.
- **Use**: High-performance configurable AI.
- **Strength**: Potentially very strong when optimized.

## Creating a New Bot

To create a new bot:

1. **Create header and implementation files** in appropriate subfolder (e.g., `myBot.hpp` and `myBot.cpp`).

2. **Inherit from IPlayer**:
   ```cpp
   class MyBot : public IPlayer {
   public:
       MyBot() : IPlayer() {}
       virtual ~MyBot() = default;

       // Implement all pure virtual methods
       std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override;
       // ... other methods
   };
   ```

3. **Implement the methods** using `boardState` to query game state and generate actions.

4. **Add to CMakeLists.txt** in the players directory to include in build.

5. **Test the bot** by running games with it.

### Tips for Implementation

- Always check `boardState->getLegalActions()` to ensure actions are valid.
- Use helper functions from `playerHelpers.hpp` for common calculations.
- Consider game phase (initial placement vs. main game) in your logic.
- Test with different board configurations and opponent types.

### Example Simple Bot

```cpp
#include "player.hpp"

class SimpleBot : public IPlayer {
public:
    std::pair<Action::PackedAction, Action::PackedAction> getInitialPlacement() override {
        auto actions = boardState->generatePlaceInitialStructures(boardState->getCurrentPlayer());
        if (actions.empty()) {
            auto noAction = Action::getEmptyAction();
            return {noAction, noAction};
        }
        // Pick first available placement
        return {actions[0], actions[0]};
    }

    Action::PackedAction getTurnAction() override {
        auto actions = boardState->getLegalActions(boardState->getCurrentPlayer());
        if (actions.empty()) return Action::getEmptyAction();
        // Pick random action
        return actions[RandomDevice::uniform_u32_range(0, actions.size() - 1)];
    }

    // Implement other methods similarly...
};
```