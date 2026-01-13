
# Players

This folder contains example AI player implementations used by the game simulation.

## Random Player

The Random Player is a baseline strategy that selects its next move by sampling uniformly from the set of currently legal actions.

- It does not evaluate positions, track long-term plans, or attempt to optimize resource collection.
- It is useful as a simple opponent, a sanity check for game rules, and a benchmark when comparing stronger strategies.

## It1Player

It1Player is a small improvement over Random Player.

It inherits the same random behavior for setup, discarding, robber moves, and development-card play, but overrides turn decisions to prefer building actions when available.

Priority order in `getTurnAction()`:

- Build city
- Build settlement
- Build road
- Buy development card
- Otherwise pick randomly from other legal actions (e.g., trades)
- End turn

## It2Player

It2Player keeps It1’s build priorities, but adds a **goal-directed bank/port trading heuristic**.

High-level behavior:

- If a **city** or **settlement** build is legal, it takes it immediately (same as It1).
- Otherwise, if a bank/port **trade** is legal, it tries to trade **towards the closest affordable purchase**, where “closest” means: *minimize how many cards are still missing after best-case trading*, using the player’s current ports (2:1, 3:1, 4:1).
	- It evaluates targets in this order for tie-breaks: City > Settlement > Road > DevCard.
	- It chooses a concrete trade that reduces the “deficit” to the chosen target and prefers better ratios (2:1 first, then 3:1, then 4:1).
	- It only spends resources that are surplus relative to the chosen target’s cost.
- If no helpful trade exists, it falls back to It1-style: Road, then DevCard, then random among remaining actions.

Note: this means It2 may trade even when a road/dev purchase is currently legal, if trading moves it closer to a higher-value target.

## It3Player

It3Player inherits **It2Player’s turn logic**, but improves:

- **Setup** (`getInitialPlacement()`, `get2InitialPlacement()`): chooses initial settlement+road placements by scoring candidate settlement nodes using dice-number “pips” (6/8 highest), early-game resource preferences (Brick/Lumber slightly favored), resource diversity, and a modest port bonus. The second placement additionally prefers covering resources that the first placement didn’t provide.
- **Robber** (`getMoveRobber()`): prefers moving the robber onto high-frequency hexes that hurt the opponent’s production while avoiding blocking its own production (strongly prefers hexes where the opponent has buildings and It3 does not).

## It4Player

It4Player inherits **It3Player** (so it keeps It3’s improved setup + robber), and adds two major improvements:

- Smarter **development-card usage** via `getDevAction()`.
- Stronger, more deterministic **turn policy** via `getTurnAction()` (better build placement choices and more goal-directed trades).

High-level behavior:

- Generally avoids playing a development card **before rolling** unless it has a strong reason (e.g., securing Largest Army, a high-value Monopoly).
- **Knight**: prioritizes playing a knight if it would immediately secure (or swing) **Largest Army**; otherwise chooses a robber move that reduces opponent production while minimizing self-blocking.
- **Monopoly**: plays when the opponent holds a meaningful amount of a resource (prefers denying resources that help the opponent and that help It4’s current best purchase goal).
- **Year of Plenty**: prefers resource pairs that reduce the deficit to the best purchase target; huge preference if it makes a City/Settlement immediately affordable.
- **Road Building**: prefers road placements that connect to the player’s existing network (roads/settlements), with a bias toward building two roads when possible.

Turn policy highlights:

- If a **City** is buildable, picks the best city upgrade (production-weighted).
- Else if a **Settlement** is buildable, picks the best settlement node (production-weighted + ports).
- Else considers **bank/port trades** that reduce the deficit to the best purchase target.
- Otherwise chooses between **Road** and **BuyDevCard** with a bias toward dev cards in mid-game.

## It5Player

It5Player inherits **It4Player** and focuses on handling the “7 discard” rule better while also improving deterministic turn decisions.

- **Discarding** (`getDiscardAction()`): when the hand size is above the limit, it discards resources that least damage its closest purchase plan.
	- Note: in this project, discarding triggers when you have **10+** resource cards (hand limit **9**) and you discard `totalResources / 2`.
- **Turn action** (`getTurnAction()`): uses a 1-step lookahead for deterministic actions (builds, roads, trades) by temporarily applying an action to the board, scoring the resulting position, then undoing it.
	- It intentionally does **not** simulate RNG actions like `BuyDevCard` (because that would consume randomness and bias the simulation).

