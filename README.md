# CatanAPI

A bit-packed C++20 engine for two-player Catan, built as a testbed for game-AI
research: 13 agents ranging from a random baseline to depth-3 alpha-beta search,
a JSONL replay format with a GUI viewer, and an evolutionary tuner that fits
agent weights by self-play.

[![CI](https://github.com/FerrisOfficial/CatanAPI/actions/workflows/ci.yml/badge.svg)](https://github.com/FerrisOfficial/CatanAPI/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

Written for a BSc thesis at the University of Wrocław ([`Thesis.pdf`](Thesis.pdf),
in Polish) by Maciej Stempniak and Yaryna Rachkevych.

![Replay viewer](docs/example_game1.png)

## Why the engine matters more than the agents

Catan resists the techniques that solve chess. Dice make it stochastic, the
development-card deck makes it partially observable, and the branching factor at
a single decision point is dominated by trade and build combinations rather than
piece moves. So agent quality can't be argued from first principles — it has to
be measured, and measuring a ~1% win-rate difference needs tens of thousands of
games.

That makes throughput a research constraint rather than a nice-to-have, and most
of the engineering here is in service of it: every game object is bit-packed into
a single integer, every action has a tested inverse so candidates can be scored
in place instead of on a copy, and action generation avoids per-decision
allocation. A single-threaded release build runs **250–300 games/s** for `rp` vs
`rp` and **~145 games/s** for `it5` vs `it5` (Clang 22, `-O3`); the pass
described in [Profile-driven optimisation](#profile-driven-optimisation) cut
per-game cost by 59%. Throughput varies with the agents, the machine, and what
else is running — `run` prints games/s at the end of every batch, so measure
yours rather than trusting these.

## Scope

Deliberately a 1v1 research variant, not a faithful reimplementation of the
retail game. What's implemented:

- Standard 19-hex board, 54 nodes, 72 edges, randomised resource and number
  layout, ports (2:1 and 3:1)
- Settlements, cities, roads, the distance rule, longest road, largest army
- All five development-card types, the robber, the 7-roll discard at >9 cards
- Bank and port trading

Non-goals, and their consequences:

- **Two players only.** `PlayerId` is 2 bits and `BoardState` holds exactly two
  packed players; 3–4 player games would need the seating and robber-steal logic
  generalised.
- **Win condition is 15 VP,** not 10, to keep two-player games long enough that
  strategic differences separate the agents rather than the opening dice.
- **No player-to-player trading.** Negotiation is the hardest part of Catan to
  model and would dominate any comparison between agents, so agents trade only
  with the bank and ports.
- **Games are capped at 1000 turns** and report a draw (`NoPlayer`) if neither
  player reaches 15 VP. Weak agents hit this cap often — see the win rates below
  that don't sum to 100%.

## Quick start

Requires a C++20 compiler, CMake 3.14+, and Python 3.7+ for the tooling
(the replay viewer needs `tkinter`; the plotting scripts in `research/` need
`matplotlib`).

```bash
cmake -S . -B build
cmake --build build
```

Run a single game with a full replay log, then watch it:

```bash
build/runs/run it5 rp
```

```bash
python utils/replay_viewer.py --last-replay
```

Run a benchmark. `--switch` alternates seats so the result measures agent
strength rather than first-player advantage, and `--no-dump` skips writing 1000
log files:

```bash
build/runs/run it5 ab -n 1000 --switch --no-dump
```

Progress lines report running win rates, average and maximum turns, throughput,
and an ETA; the run ends with a per-agent summary (abridged here):

```
ByBot: it5=422, ab=578, NP=0
LossVP(avg VP when bot lost): it5=9.94, ab=8.40
Metrics: it5, LR%=22.3, LA%=67.8, avgDevCards=2.8, avgProdScore=953.9, ...
         ab,  LR%=77.3, LA%=32.2, avgDevCards=2.1, avgProdScore=1063.6, ...
```

`LossVP` is the average victory points an agent held in the games it lost — it
separates narrow losses from being run over, which a win rate alone hides.
`LR%`/`LA%` are how often the agent finished holding longest road and largest
army, and `avgProdScore` is its pips-weighted expected production per turn.

`--seed S` re-seeds per game as `S + gameIndex`, so any individual game in a
batch can be reproduced on its own. `build/runs/run --help` lists every option.

> Paths above assume a single-config generator. Multi-config generators put the
> binary in `build/runs/Release/run.exe`, and MinGW appends `.exe`.

## Agents

| Flag | Class | Approach |
|---|---|---|
| `rp` | `RandomPlayer` | Uniform choice over legal actions. Baseline. |
| `it1` | `It1Player` | Buys by action-type priority — city > settlement > road > dev card — random within a tier. |
| `it2` | `It2Player` | Adds goal-directed trading: picks the cheapest reachable target by resource deficit and trades toward it at the best port ratio it owns. |
| `it3` | `It3Player` | Adds production-scored initial placement and deliberate robber placement. |
| `it4` | `It4Player` | Adds development-card play timing and knight sequencing. |
| `it5` | `It5Player` | Adds discard selection and hand-size risk management around the 9-card robber threshold. |
| `ab` | `AlphaBetaPlayer` | Depth-3 alpha-beta over `It5`'s evaluation, with dice outcomes weighted by probability at chance nodes. |
| `cr` | `CityRushPlayer` | Grain/ore priority, upgrades to cities as early as possible. |
| `dev` | `DevPlayer` | Buys development cards over building whenever affordable. |
| `or` | `OneResourcePlayer` | Corners production of a single resource and trades from it. |
| `road` | `RoadPlayer` | Maximises longest continuous road. |
| `para` | `ParaPlayer` | 73 weights over evaluation, discard, robber, and placement, loaded from [`paraPlayer.cfg`](players/parametricPlayers/paraPlayer.cfg). Tuned by self-play. |
| `psit5` | `ParaSetIt5Player` | `It5`'s midgame with a tuned initial-placement policy. |

Each rung of the `it1`…`it5` ladder subclasses the one below it and overrides
only the methods it changes, so a win-rate delta between two rungs is
attributable to exactly the mechanism named above. `cr`, `dev`, `or`, and `road`
are single-tactic agents used to test whether specialisation beats balance.

## Results

All figures are 1000 games with seats alternated. Full analysis, confidence
intervals, and plots are in [`docs/`](docs) and the thesis.

### The heuristic ladder against the random baseline

| Agent | Win rate | Avg turns | Longest road | Largest army | Avg dev cards | Production score |
|---|---|---|---|---|---|---|
| `it1` | 56.0% | 448.5 | 51.2% | 58.9% | 3.0 | 730.4 |
| `it2` | 73.3% | 381.0 | 64.4% | 62.0% | 3.0 | 864.3 |
| `it3` | 98.7% | 180.3 | 83.7% | 91.9% | 3.2 | 1003.2 |
| `it4` | 96.9% | 192.6 | 94.6% | 86.7% | 3.0 | 1190.3 |
| `it5` | **100.0%** | 115.3 | 71.5% | 98.5% | 3.7 | 1096.4 |
| `psit5` | **100.0%** | 111.0 | 68.3% | 98.8% | 3.7 | 1111.5 |
| `road` | 100.0% | 132.7 | 94.5% | 98.6% | 3.6 | 1051.0 |
| `para` | 99.5% | 125.1 | 90.2% | 25.9% | 9.9 | 1267.8 |
| `or` | 99.8% | 159.9 | 82.0% | 92.4% | 2.9 | 1117.0 |
| `dev` | 99.8% | 166.1 | 52.8% | 99.7% | 4.6 | 1064.8 |

The interesting result is the discontinuity at `it3`: 73.3% → 98.7% from adding
scored initial placement and active robber use. Nothing else on the ladder buys
that much. Average game length collapses alongside it (381 → 180 turns), and
`it1`–`it3` still hit the 1000-turn cap where `it5` never exceeds 258 — stronger
agents don't just win more, they convert a position instead of stalling.

`para` reaches a comparable win rate by an entirely different route: 9.9 dev
cards per game against everyone else's ~3, and 25.9% largest army against
98.5% for `it5`. Tuning found a development-card economy strategy that no
handwritten heuristic here encodes.

### Consecutive iterations head-to-head

| Matchup | Win rate | Avg turns |
|---|---|---|
| `it1` vs `it2` | 35.8% – 61.9% | 339.8 |
| `it2` vs `it3` | 5.8% – **93.6%** | 181.9 |
| `it3` vs `it4` | 32.4% – 66.0% | 184.4 |
| `it4` vs `it5` | 25.7% – 74.2% | 134.2 |
| `it1` vs `it5` | 0.1% – 99.9% | 114.9 |

Each iteration beats its predecessor directly, which is the point of building the
ladder this way. Note the non-transitivity: `it3` scores higher than `it4`
against the random baseline (98.7% vs 96.9%), yet loses to it 34%–66%. Win rate
against a weak fixed opponent saturates and stops discriminating — head-to-head
is the measurement that survives.

### Search against handcrafted heuristics

Depth-3 alpha-beta, using the same evaluation function `it5` acts on greedily:

| Opponent | `ab` win rate | Avg turns |
|---|---|---|
| `it1` | 99.9% | 105.0 |
| `it2` | 99.5% | 107.7 |
| `it3` | 94.6% | 118.8 |
| `it4` | 80.6% | 127.4 |
| `it5` | **57.8%** | 113.0 |

Search is worth a lot against a weak opponent and little against a strong one
sharing its evaluation function. Three plies of lookahead over a stochastic
branching factor this large largely re-derives what the evaluation already says,
and `ab` is by a wide margin the slowest agent to run. The reading here is that
in this domain the evaluation function is the bottleneck, not the search depth —
which is also why `para`, a tuned evaluation with no lookahead at all, is
competitive.

## Design

### Bit-packed state

Every game object is a single unsigned integer with a documented bit layout:

| Type | Width | Contents |
|---|---|---|
| `Action::PackedAction` | 64 | type, player, 5 resource counts, 3 argument bytes |
| `Player::PackedPlayer` | 64 | 5 resources, 5 dev-card types, used knights, road length, both award flags, remaining pieces, VP |
| `Node::PackedNode` | 64 | structure, owner, 3 adjacent hexes, 3 adjacent edges, port type |
| `Edge::PackedEdge` | 32 | road flag, owner, 2 endpoints |
| `Hex::PackedHex` | 16 | number, resource, per-player adjacency counts |
| `Bank::PackedBank` | 64 | resource pool and dev-card deck |

Pack and unpack are `constexpr` free functions rather than methods, so the
layout stays visible at the call site instead of hiding behind accessors, and
the whole rules layer works on integers with no pointer chasing.

The fixed part of `BoardState` — every hex, node, edge, both players, and the
bank — is under 800 bytes of flat arrays. The exception is `actionQueue`, the
full game history, which reserves 16k actions up front: useful for replay and
undo, but it means copying a `BoardState` is *not* cheap, which turns out to
drive most of the performance work below.

### Apply/undo instead of copy

`BoardState::applyAction` has a matching `undoLastAction`, and each of the 17
action handlers has a `handleUndo*` counterpart. Actions carry enough
information to invert themselves — including resolved outcomes of stochastic
events, so undoing a robber steal returns the specific resource that was taken.
`test_board_undo_actions.cpp` asserts the round trip by applying an action,
undoing it, and comparing the entire `BoardState` for equality.

This is what lets the greedy agents (`it5`, `dev`, `or`, `road`, `para`) evaluate
a candidate in place: apply, score, undo, with no copy at all.

`AlphaBetaPlayer` does not yet use it — it copies the board per node and calls
`actionQueue.clear()` plus `shrink_to_fit()` on each child to stop the reserved
history from being copied down the tree. That works, but it is a workaround for a
problem undo already solves, and it is the obvious next optimisation: the search
is the one place where per-node copying costs the most and the tested undo path
is already there.

### Allocation discipline in action generation

`getLegalActions` originally built seven vectors and copied them into an eighth.
Each generator now has an `append*(playerId, out)` form writing into one
caller-owned buffer reserved once, with the `generate*` forms kept as thin
wrappers so existing callers and tests are untouched. Similarly,
`Node::getAdjacentEdges` returns `std::array<EdgeId, 3>` — the count is a
property of the board geometry and known at compile time, so a heap allocation
in the innermost geometry loop was pure overhead.

### Profile-driven optimisation

The 59% cut in per-game cost (measured at the time as 114 → 181 g/s) came from
profiling rather than guessing, and the hot spots were not the ones the shape of
the code suggests:

- `dfsLongestFromEdge` took `BoardState` **by value** while only reading it.
  Since it recurses, every level deep-copied the board including
  `actionQueue`'s 16k-action reserved capacity — 128 KB per level. One function,
  49% of total runtime, fixed by a `const&`.
- `callPlayerGuarded` snapshots the board and deep-compares it after every agent
  callback, to catch an agent mutating state it isn't allowed to touch. Correct
  and worth keeping while developing agents, but `operator==` was 19.6% of
  runtime. It stays on by default and compiles out with
  `-DCATAN_VALIDATE_PLAYERS=0` for throughput builds.

Win rates across every benchmark were unchanged afterwards, which is the
property that makes a performance change safe to keep.

### Fitting agent weights

`ParaPlayer` exposes 73 named weights — evaluation terms, discard preferences,
robber placement, initial placement, policy temperature — read from a `.cfg`
file at construction and overridable via `CATAN_PARA_CFG`. `utils/train_para_player.py`
runs a staged evolutionary search over them: Gaussian mutation of the current
best, a population evaluated in parallel against a fixed opponent, promotion to
a harder opponent once a win-rate target is met. Fitness is the win rate the
engine itself reports, so the tuner treats `run` as a black box and needs no
bindings.

## Tooling

**Replay viewer.** Every game can be dumped as JSONL — one line per event, with
the full board state at each turn boundary — and stepped through in a Tk viewer
(screenshot above): board, both hands, dev cards, award holders, and the action
log, scrubbable turn by turn. It exists because aggregate win rates tell you an
agent is worse without telling you why; watching the turn where a position was
thrown away does.

```bash
python utils/replay_viewer.py --last-replay
```

Dumping is per-game and off by default in batch runs. `--dump-game N` and
`--dump-range A B` capture only the games worth looking at, which matters when
the interesting game is number 8,412.

**Seat-order study.** `run_fise` plays a full round robin — all 91 pairs of the
13 agents, including each against itself — in both seat orders, reporting each
agent's win rate as first and second player separately. That isolates
first-player advantage from agent strength, and is what justifies `--switch` on
every number in [Results](#results).

```bash
build/runs/run_fise --games 1000
```

At the default 1000 games per order that is 182,000 games in one invocation,
which is the concrete reason throughput is treated as a feature here. The report
it writes to `logs/` is in Polish, matching the thesis.

## Repository layout

```
game_simulation/   Board state, rules, action apply/undo, action generation
players/           IPlayer interface and all 13 agents  (see players/README.md)
runs/              run: benchmark CLI;  run_fise: seat-order study
tests/             GoogleTest suites, 6 binaries
utils/             JSONL dumper, RNG, replay viewer, training scripts
research/          Plotting scripts for the thesis figures
docs/              Thesis sources, experiment analysis, figures
```

## Testing

111 `TEST`/`TEST_P` definitions expand to 242 cases across 6 binaries, covering
bit-layout round trips, per-action legality, apply/undo symmetry, full-game
invariants, and the RNG.

```bash
ctest --test-dir build --output-on-failure
```

[CI](.github/workflows/ci.yml) runs the suite on GCC, MSVC, and Clang, plus an
AddressSanitizer build (`-DENABLE_ASAN=ON`), a `clang-format --Werror` check, and
a seeded end-to-end run of the CLI.

## Adding an agent

Subclass `IPlayer`, implement its six methods, register the flag in
`runs/run.cpp`, and add the source to `players/CMakeLists.txt`.
[`players/README.md`](players/README.md) documents the interface, the
`BoardState` API available to agents, and the helpers in `playerHelpers.hpp`.

## License

MIT — see [LICENSE](LICENSE).
