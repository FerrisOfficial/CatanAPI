# Agents

Every agent implements `IPlayer` ([`player.hpp`](player.hpp)) and is selected at
runtime by a short flag registered in [`runs/run.cpp`](../runs/run.cpp). The
engine drives agents through callbacks; agents never advance the game themselves.

- [The interface](#the-interface)
- [Reading the board](#reading-the-board)
- [Generating legal actions](#generating-legal-actions)
- [Helpers](#helpers)
- [Simulating ahead](#simulating-ahead)
- [Implemented agents](#implemented-agents)
- [Adding an agent](#adding-an-agent)

## The interface

```cpp
struct IPlayer {
    Board::BoardState* boardState;   // set by Game; valid for the agent's lifetime

    virtual std::pair<Action::PackedAction, Action::PackedAction>
        getInitialPlacement()  = 0;  // {settlement, road} — first placement
    virtual std::pair<Action::PackedAction, Action::PackedAction>
        get2InitialPlacement() = 0;  // {settlement, road} — second placement

    virtual Action::PackedAction getTurnAction()    = 0;  // build / trade / buy / end turn
    virtual Action::PackedAction getDevAction()     = 0;  // play a dev card, or empty
    virtual Action::PackedAction getDiscardAction() = 0;  // resources to discard on a 7
    virtual Action::PackedAction getMoveRobber()    = 0;  // where to send the robber
};
```

Every method returns `Action::PackedAction` (`uint64_t`). Return
`Action::getEmptyAction()` to decline where declining is legal — `getDevAction`
is called every turn whether or not there is a card worth playing.

`getTurnAction` is called repeatedly within a turn until it returns an `EndTurn`
action, so an agent that never ends its turn will hang the game. Returning
`EndTurn` is always legal.

> **A callback must leave the board exactly as it found it.** `Game` snapshots
> `BoardState` before every callback and deep-compares it afterwards; on a
> mismatch it throws `std::runtime_error` naming the agent, the method, and the
> first field that differs. Speculating *is* allowed — you just have to undo it
> before returning, which is what [Simulating ahead](#simulating-ahead) covers.
> The check compiles out with `-DCATAN_VALIDATE_PLAYERS=0`, so develop with it on.

## Reading the board

`Board::BoardState` ([`board.hpp`](../game_simulation/board.hpp)) is a plain
struct of packed integers, read through free `unpack*` functions rather than
accessors:

```cpp
const PlayerId me   = boardState->currentPlayer;
const auto     hand = boardState->packedPlayers[static_cast<uint8_t>(me)];

const uint8_t ore = Player::unpackResource(hand, Resource::Ore);
const uint8_t vp  = Player::unpackVictoryPoints(hand);
const bool    army = Player::unpackLargestArmyFlag(hand);
```

| Field | Meaning |
|---|---|
| `currentPlayer`, `currentTurn` | Whose turn it is, and the turn counter |
| `packedPlayers[2]` | Both players' packed state, indexed by `PlayerId` — **the opponent's hand is visible**, so an agent that should respect hidden information has to restrain itself |
| `hexes[19]` | Number, resource, per-player adjacency (`Board::Hex::unpack*`) |
| `nodes[54]` | Structure, owner, adjacent hexes/edges, port type (`Board::Node::unpack*`) |
| `edges[72]` | Road flag, owner, endpoints (`Board::Edge::unpack*`) |
| `robberPosition` | Current robber `HexId` |
| `packedBank` | Bank resource pool and dev-card deck (`Bank::unpack*`) |
| `actionQueue` | Full ordered history of applied actions |

Per-type accessors live with their layout comments: `Player::` in
[`player.hpp`](../game_simulation/player.hpp), `Action::` in
[`actions.hpp`](../game_simulation/actions.hpp), `Board::Hex/Node/Edge::` and
`Bank::` in [`board.hpp`](../game_simulation/board.hpp) and
[`packedBank.hpp`](../game_simulation/packedBank.hpp).

## Generating legal actions

Never construct an action by hand and assume it is legal — ask the board.

```cpp
std::vector<Action::PackedAction> getLegalActions(PlayerId);
```

The one call an agent usually needs in `getTurnAction`. Returns every build,
bank trade, 2:1 and 3:1 port trade, and dev-card purchase available right now,
plus `EndTurn` last.

For narrower questions, or to filter by type without scanning:

| Phase | Generators |
|---|---|
| Setup | `generatePlaceInitialStructures`, `generatePlace2InitialStructures` |
| Building | `generateBuildRoadActions`, `generateBuildSettlementActions`, `generateBuildCityActions` |
| Trading | `generateBankTradeActions`, `generateTwoToOnePortTradeActions`, `generateThreeToOnePortTradeActions` |
| Dev cards | `generateBuyDevCardActions`, `generatePlayDevCardActions`, and per-card `generatePlayDevCard{Knight,RoadBuilding,YearOfPlenty,Monopoly}Actions` |
| Robber | `generateMoveRobberActions` |

All take a `PlayerId` and return `std::vector<Action::PackedAction>`. The
setup generators return actions that encode both the settlement (`Arg1`, a
`NodeId`) and the road (`Arg2`, an `EdgeId`), which is why the two setup
callbacks return a pair.

Each build/trade/dev generator also has an `append*(PlayerId, std::vector&)`
form that writes into a caller-owned buffer. Prefer these on hot paths — one
reserved vector serves a whole decision instead of one allocation per generator.

Decode results with `Action::unpackType`, `unpackArg1..3`, and
`unpackResource`; the argument meaning per action type is documented on the
`ActionType` enum in [`consts.hpp`](../game_simulation/consts.hpp).

## Helpers

[`playerHelpers.hpp`](playerHelpers.hpp) holds the evaluation primitives shared
across agents, so scoring logic isn't reimplemented per bot:

| Function | Purpose |
|---|---|
| `effective_vp(board, pid)` | VP including the +2 for longest road and largest army — the number the win condition actually tests |
| `unpack_resources(packed)` | Hand as `std::array<uint8_t, 5>` |
| `hand_count(have)` | Total cards, for the 9-card discard threshold |
| `cost_for(BuyableType)` | Cost vector for road / settlement / city / dev card |
| `deficit(have, need)` | How far a hand is from affording something |
| `dice_pips(n)` | Ways to roll `n` — the weight of a number token |
| `node_production_score(board, node)` | Pips-weighted production at one node |
| `production_score_for_player(board, pid)` | Same summed over owned nodes |
| `settlement_potential_score(board, pid)` | Value of positions still reachable |
| `node_distance_rule_ok(board, node)` | Distance rule check |
| `node_is_adjacent_to_own_road(board, pid, node)` | Connectivity check |
| `evaluate_position(board, pid)` | Full position score; what `It5` maximises greedily and `AlphaBetaPlayer` searches over |
| `is_deterministic_action(a)` | Whether applying `a` has no random outcome — search can only recurse through these |

Randomness goes through [`RandomDevice`](../utils/randomDevice.hpp) rather than
a local `mt19937`, so `run --seed` reproduces a game end to end:
`uniform_u32`, `uniform_u32_range`, `get_rng()` for `std::shuffle`, and
`RandomDevice::ScopedState`, an RAII guard that restores the stream on scope
exit for agents that consume randomness while simulating.

## Simulating ahead

The cheap way to score a candidate is in place — apply it, evaluate, undo it:

```cpp
boardState->applyAction(candidate);
const int score = PlayerHelpers::evaluate_position(boardState, me);
boardState->undoLastAction();   // must run on every path out, including throws
```

Every action handler has a `handleUndo*` counterpart and the round trip is
covered by `test_board_undo_actions.cpp`, so the board comes back bit-identical —
which is what keeps the post-callback comparison happy. `It5Player`, `DevPlayer`,
`OneResourcePlayer`, `RoadPlayer`, and `ParaPlayer` all score candidates this way.

Copying works too, and is what you want if a line has to diverge from the real
board rather than rewind onto it:

```cpp
Board::BoardState sim = *boardState;
sim.applyAction(candidate);
const int score = PlayerHelpers::evaluate_position(&sim, me);
```

It is the more expensive option, though, and not by a small margin: the board
arrays are only ~800 bytes, but `actionQueue` reserves 16k actions, so the copy
drags the whole game history with it. `AlphaBetaPlayer` copies per node and
clears `actionQueue` on each child to blunt the cost — converting it to
apply/undo is an open improvement.

Two constraints either way:

- Only simulate through `is_deterministic_action` — builds, bank trades, and
  `EndTurn`. It excludes `BuyDevCard` and `StealResource` as well as dice, since
  their outcome is drawn at apply time and isn't a property of the action.
  `AlphaBetaPlayer` handles rolls instead by enumerating 2–12 through
  `handleRollDice` and weighting each outcome by its probability.
- If you copy, simulate on the copy, never on `*boardState` without undoing.

## Implemented agents

| Flag | Class | File | Notes |
|---|---|---|---|
| `rp` | `RandomPlayer` | [`randomPlayer.cpp`](randomPlayer.cpp) | Uniform over legal actions. The base class the ladder builds on, and the control in every experiment. |
| `it1` | `It1Player` | [`itPlayers/it1Player.cpp`](itPlayers/it1Player.cpp) | Action-type priority: city > settlement > road > dev card > other > end turn, random within a tier. |
| `it2` | `It2Player` | [`itPlayers/it2Player.cpp`](itPlayers/it2Player.cpp) | Detects owned ports, picks the cheapest reachable target by `deficit`, trades toward it at the best ratio it has. |
| `it3` | `It3Player` | [`itPlayers/it3Player.cpp`](itPlayers/it3Player.cpp) | Production-scored initial placement with resource diversity, plus robber placement that targets opponent production. The largest single win-rate jump on the ladder. |
| `it4` | `It4Player` | [`itPlayers/it4Player.cpp`](itPlayers/it4Player.cpp) | Dev-card play timing and knight sequencing toward largest army. |
| `it5` | `It5Player` | [`itPlayers/it5Player.cpp`](itPlayers/it5Player.cpp) | Discard selection that protects build targets, and hand management around the 9-card threshold. Strongest handwritten agent. |
| `ab` | `AlphaBetaPlayer` | [`oneTacticPlayers/alphaBetaPlayer.cpp`](oneTacticPlayers/alphaBetaPlayer.cpp) | Subclasses `It5Player`, overriding only `getTurnAction` with depth-3 alpha-beta over `evaluate_position`, stage detection, and probability-weighted roll outcomes. By a wide margin the slowest agent per decision. |
| `cr` | `CityRushPlayer` | [`oneTacticPlayers/cityRushPlayer.cpp`](oneTacticPlayers/cityRushPlayer.cpp) | Grain/ore priority, earliest possible city upgrades. |
| `dev` | `DevPlayer` | [`oneTacticPlayers/devPlayer.cpp`](oneTacticPlayers/devPlayer.cpp) | Dev cards over buildings whenever affordable. |
| `or` | `OneResourcePlayer` | [`oneTacticPlayers/oneResourcePlayer.cpp`](oneTacticPlayers/oneResourcePlayer.cpp) | Corners one resource and trades out of it. |
| `road` | `RoadPlayer` | [`roadPlayer.cpp`](roadPlayer.cpp) | Maximises longest continuous road. |
| `para` | `ParaPlayer` | [`parametricPlayers/paraPlayer.cpp`](parametricPlayers/paraPlayer.cpp) | 73 weights from [`paraPlayer.cfg`](parametricPlayers/paraPlayer.cfg) (override with `CATAN_PARA_CFG`), fitted by [`train_para_player.py`](../utils/train_para_player.py). |
| `psit5` | `ParaSetIt5Player` | [`parametricPlayers/paraSetit5Player.cpp`](parametricPlayers/paraSetit5Player.cpp) | `It5`'s midgame with a tuned initial-placement policy from [`paraSetit5Player.cfg`](parametricPlayers/paraSetit5Player.cfg) (`CATAN_PARA_SETIT5_CFG`). |

Measured win rates for all of these are in the [top-level README](../README.md#results).

## Adding an agent

Subclass whichever agent is closest and override only what you're changing —
that is how the `it*` ladder attributes a win-rate delta to one mechanism. Start
from `RandomPlayer` for a fresh strategy, since it supplies working
implementations of all six callbacks.

```cpp
// players/myPlayer.hpp
#pragma once
#include "players/randomPlayer.hpp"

struct MyPlayer : public RandomPlayer {
    MyPlayer() : RandomPlayer() {}
    virtual ~MyPlayer() = default;
    Action::PackedAction getTurnAction() override;
};
```

```cpp
// players/myPlayer.cpp
#include "myPlayer.hpp"

#include <limits>

#include "playerHelpers.hpp"

// Greedy one-ply: score every legal action in place and keep the best.
Action::PackedAction MyPlayer::getTurnAction() {
    const PlayerId me = boardState->currentPlayer;
    const auto actions = boardState->getLegalActions(me);
    if (actions.empty()) return Action::getEmptyAction();

    Action::PackedAction best = actions.front();
    int bestScore = std::numeric_limits<int>::min();

    for (const auto a : actions) {
        if (!PlayerHelpers::is_deterministic_action(a)) continue;

        boardState->applyAction(a);
        const int score = PlayerHelpers::evaluate_position(boardState, me);
        boardState->undoLastAction();

        if (score > bestScore) {
            bestScore = score;
            best = a;
        }
    }
    return best;
}
```

`getLegalActions` always includes `EndTurn`, and `is_deterministic_action` admits
it, so this always returns something and always terminates the turn eventually.

Then:

1. Add `myPlayer.cpp` to the source list in [`CMakeLists.txt`](CMakeLists.txt).
2. Register the flag in `make_player_from_flag` **and** `display_name_from_flag`
   in [`runs/run.cpp`](../runs/run.cpp) — the display name is what the metrics
   output and replay logs are labelled with.
3. Benchmark it against the baseline and the strongest agent, with seats
   alternated:

```bash
build/runs/run mine rp -n 1000 --switch --no-dump
```

```bash
build/runs/run mine it5 -n 1000 --switch --no-dump
```

A win rate against `rp` alone is close to meaningless above ~95%, where it
saturates and stops discriminating — `it3` outscores `it4` against `rp` and
still loses to it head-to-head. Report both.
