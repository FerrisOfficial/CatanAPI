"""CatanAPI replay log viewer (JSONL).

Reads a Dumper-generated `logs/game_*.jsonl` file and renders a simple board.
- Slider lets you scrub between turns (and initial state).
- Draws hex tiles (resource + dice number), robber position.
- Draws roads (edges) and settlements/cities (nodes) based on decoded state.

Usage:
  python utils/replay_viewer.py logs/game_20260109_150148_309.jsonl
    python utils/replay_viewer.py -lr

Flags:
    -lr / --last-replay   Automatically open the newest *.jsonl under logs/ (or build/logs/) and auto-play it.

If no path is provided, a file picker opens.

Notes:
- Uses only stdlib (tkinter/json/math).
- Assumes the hex indexing/layout as defined in `game_simulation/board.hpp` (radius-2 hexagon).
"""

from __future__ import annotations

import argparse
import json
import math
import os
import sys
from pathlib import Path
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

import tkinter as tk
from tkinter import filedialog


# --- Layout: 19 hexes as radius-2 axial coordinates ---
# Axial coords (q, r) with constraint |q|<=2, |r|<=2, |q+r|<=2.
# We order by rows (r=-2..2) to match the indexing used by BoardState init.
HEX_AXIAL: List[Tuple[int, int]] = []
for r in (-2, -1, 0, 1, 2):
    q_min = max(-2, -r - 2)
    q_max = min(2, -r + 2)
    for q in range(q_min, q_max + 1):
        HEX_AXIAL.append((q, r))

assert len(HEX_AXIAL) == 19, f"expected 19 hex coords, got {len(HEX_AXIAL)}"


RESOURCE_COLORS = {
    "Brick": "#c1442e",
    "Lumber": "#2f7d32",
    "Wool": "#7bc96f",
    "Grain": "#f2d16b",
    "Ore": "#9ea3a8",
    "NoResource": "#e6d3a3",  # desert
}

# Colors for player resource table text (slightly darker than tile fills for readability).
RESOURCE_TEXT_COLORS = {
    "brick": "#c1442e",
    "lumber": "#2f7d32",
    "wool": "#2f855a",
    "grain": "#b7791f",
    "ore": "#4a5568",
}

PLAYER_COLORS = {
    0: "#2b6cb0",  # blue
    1: "#c53030",  # red
    2: "#4a5568",  # NoPlayer
}


def _safe_get(dct: Dict[str, Any], key: str, default: Any) -> Any:
    v = dct.get(key, default)
    return default if v is None else v


@dataclass(frozen=True)
class Frame:
    seq: int
    label: str
    turn: Optional[int]
    current_player: Optional[int]
    state: Dict[str, Any]


class ReplayLog:
    def __init__(self, path: str):
        self.path = path
        self.frames: List[Frame] = []
        self.frame_events: List[List[str]] = []
        self.frame_end_state: List[Optional[Dict[str, Any]]] = []
        self._load()

    def _load(self) -> None:
        frames: List[Frame] = []
        events: List[Tuple[int, Dict[str, Any]]] = []

        def _event_seq(obj: Dict[str, Any], fallback: int) -> int:
            s = obj.get("seq")
            return int(s) if isinstance(s, int) else fallback

        def _fmt_resources(r: Any) -> str:
            if not isinstance(r, dict):
                return ""
            parts: List[str] = []
            for k in ("brick", "lumber", "wool", "grain", "ore"):
                v = r.get(k)
                if isinstance(v, int) and v:
                    parts.append(f"{k}={v}")
            return (" (" + ", ".join(parts) + ")") if parts else ""

        def _fmt_event(obj: Dict[str, Any]) -> Optional[str]:
            ev = obj.get("event")
            seq = obj.get("seq")
            ts = obj.get("timestamp")
            prefix = f"[{seq}] " if isinstance(seq, int) else ""
            ts_s = f"{ts} " if isinstance(ts, str) else ""

            if ev == "start":
                return f"{prefix}{ts_s}start"

            if ev == "initial_state":
                st = obj.get("state")
                if isinstance(st, dict):
                    t = st.get("turn")
                    cp = st.get("current_player_name") or st.get("current_player")
                    return f"{prefix}{ts_s}initial_state (turn={t}, current_player={cp})"
                return f"{prefix}{ts_s}initial_state"

            if ev == "turn_start":
                st = obj.get("state")
                if isinstance(st, dict):
                    t = st.get("turn")
                    cp = st.get("current_player_name") or st.get("current_player")
                    return f"{prefix}{ts_s}turn_start (turn={t}, current_player={cp})"
                return f"{prefix}{ts_s}turn_start"

            if ev == "turn_end":
                t = obj.get("turn")
                cp = obj.get("current_player")
                return f"{prefix}{ts_s}turn_end (turn={t}, next_player={cp})"

            if ev == "dice":
                dice = obj.get("dice")
                if isinstance(dice, int):
                    return f"{prefix}{ts_s}dice: {dice}"
                d1 = obj.get("d1")
                d2 = obj.get("d2")
                total = obj.get("sum")
                if isinstance(d1, int) and isinstance(d2, int):
                    tot = total if isinstance(total, int) else (d1 + d2)
                    return f"{prefix}{ts_s}dice: {d1}+{d2}={tot}"
                return f"{prefix}{ts_s}dice"

            if ev == "action_applied":
                phase = obj.get("phase")
                a = obj.get("action")
                if isinstance(a, dict):
                    tname = a.get("type_name") or a.get("type")
                    player = a.get("player")
                    arg1 = a.get("arg1")
                    arg2 = a.get("arg2")
                    arg3 = a.get("arg3")
                    args: List[str] = []
                    if isinstance(arg1, int) and arg1:
                        args.append(f"arg1={arg1}")
                    if isinstance(arg2, int) and arg2:
                        args.append(f"arg2={arg2}")
                    if isinstance(arg3, int) and arg3:
                        args.append(f"arg3={arg3}")

                    res_s = _fmt_resources(a.get("resources"))
                    args_s = (" " + ", ".join(args)) if args else ""
                    phase_s = f" phase={phase}" if isinstance(phase, str) else ""
                    return f"{prefix}{ts_s}action: {tname} (player={player}){args_s}{res_s}{phase_s}"
                return f"{prefix}{ts_s}action_applied"

            if ev == "game_end":
                winner = obj.get("winner")
                return f"{prefix}{ts_s}game_end (winner={winner})"

            # Unknown/other events: show the type.
            if isinstance(ev, str):
                return f"{prefix}{ts_s}{ev}"
            return None

        with open(self.path, "r", encoding="utf-8") as f:
            for i, line in enumerate(f):
                line = line.strip()
                if not line:
                    continue
                try:
                    obj = json.loads(line)
                except json.JSONDecodeError:
                    # Some log viewers wrap long lines; ignore malformed lines.
                    continue

                if isinstance(obj, dict):
                    events.append((_event_seq(obj, i), obj))

                event = obj.get("event")
                if event == "initial_state" and isinstance(obj.get("state"), dict):
                    st = obj["state"]
                    frames.append(
                        Frame(
                            seq=_event_seq(obj, i),
                            label="Initial State",
                            turn=_safe_get(st, "turn", None),
                            current_player=_safe_get(st, "current_player", None),
                            state=st,
                        )
                    )

                if event == "turn_start" and isinstance(obj.get("state"), dict):
                    st = obj["state"]
                    t = _safe_get(st, "turn", None)
                    cp = _safe_get(st, "current_player", None)
                    frames.append(
                        Frame(
                            seq=_event_seq(obj, i),
                            label=f"Turn {t} start" if t is not None else "Turn start",
                            turn=t,
                            current_player=cp,
                            state=st,
                        )
                    )

        # Fallback: if no turn_start events exist, build frames by turn changes
        if not frames:
            last_turn = None
            with open(self.path, "r", encoding="utf-8") as f:
                for i, line in enumerate(f):
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        obj = json.loads(line)
                    except json.JSONDecodeError:
                        continue
                    st = obj.get("state")
                    if not isinstance(st, dict):
                        continue
                    t = st.get("turn")
                    if t != last_turn:
                        frames.append(
                            Frame(
                                seq=_event_seq(obj, i),
                                label=f"Turn {t}",
                                turn=t,
                                current_player=st.get("current_player"),
                                state=st,
                            )
                        )
                        last_turn = t

        self.frames = frames

        # Build per-frame event summaries using seq ranges.
        self.frame_events = [[] for _ in self.frames]
        self.frame_end_state = [None for _ in self.frames]
        if not self.frames or not events:
            return

        events.sort(key=lambda t: t[0])
        # Frames are in encounter order already, but sort defensively by seq.
        self.frames.sort(key=lambda fr: fr.seq)
        self.frame_events = [[] for _ in self.frames]
        self.frame_end_state = [None for _ in self.frames]

        fi = 0
        for seq, obj in events:
            while fi + 1 < len(self.frames) and seq >= self.frames[fi + 1].seq:
                fi += 1
            msg = _fmt_event(obj)
            if msg:
                self.frame_events[fi].append(msg)

            # Track end-of-turn snapshot (prefer explicit turn_end, else last seen state).
            st = obj.get("state")
            if isinstance(st, dict):
                if obj.get("event") == "turn_end":
                    self.frame_end_state[fi] = st
                elif self.frame_end_state[fi] is None:
                    # Keep a fallback only if we haven't seen a proper turn_end state.
                    self.frame_end_state[fi] = st


class BoardRenderer:
    def __init__(self, canvas: tk.Canvas):
        self.canvas = canvas
        self.size = 45  # hex radius in pixels
        self.margin = 30

        # computed each draw based on canvas size
        self.hex_centers: List[Tuple[float, float]] = []
        self.hex_corners: List[List[Tuple[float, float]]] = []
        self.node_pos: Dict[int, Tuple[float, float]] = {}

        # map: tuple(sorted(hex_ids)) -> list[(x, y)]
        self._node_pattern_lookup: Dict[Tuple[int, ...], List[Tuple[float, float]]] = {}

        # Updated on each draw(); used by UI for quick sanity checks.
        self.last_stats: Dict[str, int] = {}

    def _axial_to_pixel(self, q: int, r: int, cx: float, cy: float) -> Tuple[float, float]:
        # Pointy-top hex axial->pixel (renders r=-2..2 as horizontal rows: 3-4-5-4-3)
        x = self.size * (math.sqrt(3) * (q + r / 2.0))
        y = self.size * (1.5 * r)
        return (cx + x, cy + y)

    def _hex_polygon(self, center: Tuple[float, float]) -> List[Tuple[float, float]]:
        cx, cy = center
        pts: List[Tuple[float, float]] = []
        # Pointy-top: vertices start at the top (-90 degrees)
        for i in range(6):
            ang = math.radians(-90 + 60 * i)
            pts.append((cx + self.size * math.cos(ang), cy + self.size * math.sin(ang)))
        return pts

    def _recompute_geometry(self) -> None:
        w = max(1, int(self.canvas.winfo_width()))
        h = max(1, int(self.canvas.winfo_height()))

        # Center of board
        cx = w / 2.0
        cy = h / 2.0

        self.hex_centers = [self._axial_to_pixel(q, r, cx, cy) for (q, r) in HEX_AXIAL]
        self.hex_corners = [self._hex_polygon(c) for c in self.hex_centers]

        # corner -> set(hex_id), potem (sorted hex_ids) -> lista rogów
        corner_to_hexes: Dict[Tuple[int, int], set[int]] = {}
        for hid, poly in enumerate(self.hex_corners):
            for p in poly:
                pt = self._rounded_point(p)  # (int, int)
                corner_to_hexes.setdefault(pt, set()).add(hid)

        pattern_lookup: Dict[Tuple[int, ...], List[Tuple[float, float]]] = {}
        for (x, y), hexes in corner_to_hexes.items():
            key = tuple(sorted(hexes))  # np. (3,), (4,9), (5,8,12)
            pattern_lookup.setdefault(key, []).append((float(x), float(y)))

        self._node_pattern_lookup = pattern_lookup

    def _rounded_point(self, p: Tuple[float, float]) -> Tuple[int, int]:
        return (int(round(p[0])), int(round(p[1])))

    def _compute_node_positions(self, state: Dict[str, Any]) -> None:
        nodes = state.get("nodes")
        if not isinstance(nodes, list):
            self.node_pos = {}
            return

        if not self._node_pattern_lookup:
            self.node_pos = {}
            return

        remaining: Dict[Tuple[int, ...], List[Tuple[float, float]]] = {
            key: coords.copy() for key, coords in self._node_pattern_lookup.items()
        }

        pos: Dict[int, Tuple[float, float]] = {}

        for nid, n in enumerate(nodes):
            if not isinstance(n, dict):
                continue
            adj = n.get("adjacent_hexes")
            if not isinstance(adj, list):
                continue

            hex_ids = sorted({a for a in adj if isinstance(a, int) and 0 <= a < len(self.hex_corners)})
            if not hex_ids:
                continue

            key = tuple(hex_ids)
            coords = remaining.get(key)
            if not coords:
                continue

            coord = coords.pop(0)
            pos[nid] = coord

        def _swap_positions(mapping: Dict[int, Tuple[float, float]], a: int, b: int) -> None:
            if a in mapping and b in mapping:
                mapping[a], mapping[b] = mapping[b], mapping[a]

        _swap_positions(pos, 0, 1)
        _swap_positions(pos, 16, 27)
        _swap_positions(pos, 47, 48)
        _swap_positions(pos, 52, 53)

        self.node_pos = pos

    def draw(self, frame: Frame) -> None:
        self.canvas.delete("all")
        self._recompute_geometry()

        state = frame.state

        # Draw hexes
        hexes = state.get("hexes")
        robber = state.get("robber")
        if not isinstance(hexes, list):
            hexes = []

        for hid in range(min(19, len(hexes))):
            h = hexes[hid]
            if not isinstance(h, dict):
                continue
            res_name = h.get("resource_name", "NoResource")
            number = h.get("catan_number", "")
            fill = RESOURCE_COLORS.get(res_name, "#ddd")

            poly = self.hex_corners[hid]
            flat = [v for xy in poly for v in xy]
            self.canvas.create_polygon(*flat, fill=fill, outline="#333", width=2)

            cx, cy = self.hex_centers[hid]

            # --- Resource name (top) ---
            if res_name != "NoResource":
                self.canvas.create_text(
                    cx,
                    cy - 18,
                    text=res_name,
                    font=("Segoe UI", 9, "bold"),
                    fill="#222",
                )

            # --- Number token (classic Catan style) ---
            if isinstance(number, int) and res_name != "NoResource":
                is_hot = number in (6, 8)
                token_r = 14

                self.canvas.create_oval(
                    cx - token_r,
                    cy - token_r,
                    cx + token_r,
                    cy + token_r,
                    fill="#f5f5dc",
                    outline="#111",
                    width=2,
                )

                self.canvas.create_text(
                    cx,
                    cy,
                    text=str(number),
                    font=("Segoe UI", 14, "bold"),
                    fill="#c53030" if is_hot else "#111",
                )

            # Robber marker
            if isinstance(robber, int) and hid == robber:
                self.canvas.create_oval(
                    cx - 9,
                    cy - 9,
                    cx + 9,
                    cy + 9,
                    fill="#111",
                    outline="#fff",
                    width=2,
                )

        # Compute node positions from this state
        self._compute_node_positions(state)

        nodes = state.get("nodes")
        nodes_total = len(nodes) if isinstance(nodes, list) else 0
        nodes_resolved = len(self.node_pos)

        # Draw edges (roads)
        edges = state.get("edges")
        if isinstance(edges, list):
            roads_total = 0
            roads_drawn = 0
            roads_skipped_missing = 0
            roads_skipped_too_long = 0
            for e in edges:
                if not isinstance(e, dict):
                    continue
                has_road = bool(e.get("has_road", False))
                if has_road:
                    roads_total += 1
                adj = e.get("adjacent_nodes")
                if not (isinstance(adj, list) and len(adj) == 2 and all(isinstance(x, int) for x in adj)):
                    continue
                a, b = adj
                if a not in self.node_pos or b not in self.node_pos:
                    if has_road:
                        roads_skipped_missing += 1
                    continue
                ax, ay = self.node_pos[a]
                bx, by = self.node_pos[b]

                # Defensive: if geometry placement went wrong for a node, skip absurdly long segments.
                if math.hypot(ax - bx, ay - by) > 3.0 * self.size:
                    if has_road:
                        roads_skipped_too_long += 1
                    continue
                owner = int(e.get("owner", 2)) if isinstance(e.get("owner"), int) else 2

                if has_road:
                    color = PLAYER_COLORS.get(owner, "#111")
                    self.canvas.create_line(ax, ay, bx, by, fill=color, width=6, capstyle=tk.ROUND)
                    roads_drawn += 1
                # Don't draw the full edge graph (it looks like random thin lines). Only render actual roads.

            self.last_stats = {
                "nodes_total": nodes_total,
                "nodes_pos_resolved": nodes_resolved,
                "roads_total": roads_total,
                "roads_drawn": roads_drawn,
                "roads_skipped_missing_node_pos": roads_skipped_missing,
                "roads_skipped_too_long": roads_skipped_too_long,
            }
        else:
            self.last_stats = {
                "nodes_total": nodes_total,
                "nodes_pos_resolved": nodes_resolved,
                "roads_total": 0,
                "roads_drawn": 0,
                "roads_skipped_missing_node_pos": 0,
                "roads_skipped_too_long": 0,
            }

        # Draw nodes (settlements/cities)
        if isinstance(nodes, list):
            for nid, n in enumerate(nodes):
                if nid not in self.node_pos:
                    continue
                if not isinstance(n, dict):
                    continue
                structure = n.get("structure_name")
                owner = n.get("owner")
                if not isinstance(owner, int):
                    owner = 2

                x, y = self.node_pos[nid]

                if structure == "Settlement":
                    color = PLAYER_COLORS.get(owner, "#111")
                    self.canvas.create_oval(x - 6, y - 6, x + 6, y + 6, fill=color, outline="#fff", width=1)
                elif structure == "City":
                    color = PLAYER_COLORS.get(owner, "#111")
                    self.canvas.create_rectangle(x - 7, y - 7, x + 7, y + 7, fill=color, outline="#fff", width=1)

        # Draw node IDs at all node positions
        for nid, (x, y) in self.node_pos.items():
            self.canvas.create_text(
                x,
                y,
                text=str(nid),
                font=("Segoe UI", 8, "bold"),
                fill="cyan",
            )


class App(tk.Tk):
    def __init__(self, log: ReplayLog, *, autoplay: bool = False, autoplay_ms: int = 1000):
        super().__init__()
        self.title(f"Catan Replay Viewer - {os.path.basename(log.path)}")
        self.geometry("1100x850")

        self.log = log

        self.header = tk.Label(self, text="", anchor="w", font=("Segoe UI", 11, "bold"))
        self.header.pack(fill=tk.X, padx=10, pady=(10, 0))

        body = tk.Frame(self)
        body.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        body.columnconfigure(0, weight=3)
        body.columnconfigure(1, weight=1)
        body.rowconfigure(0, weight=1)

        # Left panel: player names above the board.
        board_panel = tk.Frame(body)
        board_panel.grid(row=0, column=0, sticky="nsew", padx=(0, 10))
        board_panel.columnconfigure(0, weight=1)
        board_panel.rowconfigure(1, weight=1)

        self.players_bar = tk.Frame(board_panel)
        self.players_bar.grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self.players_bar.columnconfigure(0, weight=0)
        tk.Label(self.players_bar, text="Players:", anchor="w", font=("Segoe UI", 10, "bold")).grid(
            row=0, column=0, sticky="w"
        )

        # Up to 4 players (Catan). Keep widgets stable; only update text each redraw.
        self._player_name_labels: List[tk.Label] = []
        for pid in range(4):
            lbl = tk.Label(self.players_bar, text="", anchor="w", font=("Segoe UI", 10, "bold"))
            lbl.grid(row=0, column=pid + 1, sticky="w", padx=(10, 0))
            self._player_name_labels.append(lbl)

        self.canvas = tk.Canvas(board_panel, bg="#f7fafc", highlightthickness=0)
        self.canvas.grid(row=1, column=0, sticky="nsew")

        # Right-side panel (resources + event log)
        log_frame = tk.Frame(body)
        log_frame.grid(row=0, column=1, sticky="nsew")
        # Give the turn log a bit more space than before.
        log_frame.rowconfigure(0, weight=2)
        log_frame.rowconfigure(1, weight=2)
        log_frame.rowconfigure(2, weight=0)
        log_frame.rowconfigure(3, weight=3)
        log_frame.columnconfigure(0, weight=1)

        players_start = tk.LabelFrame(log_frame, text="Players (start)")
        players_start.grid(row=0, column=0, columnspan=2, sticky="nsew", pady=(0, 8))
        players_start.columnconfigure(0, weight=1)
        self.players_start_text = tk.Text(players_start, height=6, wrap=tk.NONE, font=("Consolas", 9))
        self.players_start_text.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)
        self.players_start_text.configure(state=tk.DISABLED)

        players_end = tk.LabelFrame(log_frame, text="Players (end)")
        players_end.grid(row=1, column=0, columnspan=2, sticky="nsew", pady=(0, 10))
        players_end.columnconfigure(0, weight=1)
        self.players_end_text = tk.Text(players_end, height=6, wrap=tk.NONE, font=("Consolas", 9))
        self.players_end_text.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)
        self.players_end_text.configure(state=tk.DISABLED)

        tk.Label(log_frame, text="Turn log", anchor="w", font=("Segoe UI", 10, "bold")).grid(
            row=2, column=0, sticky="ew", pady=(0, 6)
        )
        self.log_text = tk.Text(log_frame, wrap=tk.WORD, height=6, font=("Consolas", 8))
        self.log_text.grid(row=3, column=0, sticky="nsew")
        sb = tk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        sb.grid(row=3, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=sb.set)
        self.log_text.configure(state=tk.DISABLED)

        self.scale = tk.Scale(
            self,
            from_=0,
            to=max(0, len(self.log.frames) - 1),
            orient=tk.HORIZONTAL,
            showvalue=True,
            command=self._on_scale,
            length=900,
        )
        self.scale.pack(fill=tk.X, padx=10, pady=(0, 10))

        self.renderer = BoardRenderer(self.canvas)

        self._autoplay_enabled = bool(autoplay)
        self._autoplay_ms = int(autoplay_ms)
        self._autoplay_running = False
        self._autoplay_tick_scheduled = False
        self._autoplay_internal_scale_change = False

        self.bind("<Left>", lambda _e: self._step(-1))
        self.bind("<Right>", lambda _e: self._step(1))
        self.bind("<space>", lambda _e: self._toggle_autoplay())
        self.bind("<Configure>", lambda _e: self._redraw())

        if not self.log.frames:
            self.header.config(text="No frames found in log (missing state dumps).")
        else:
            self.scale.set(0)
            self._redraw()
            if self._autoplay_enabled:
                self._start_autoplay()

    def _update_players_bar(self, state: Optional[Dict[str, Any]]) -> None:
        names: Dict[int, str] = {}
        if isinstance(state, dict):
            players = state.get("players")
            if isinstance(players, list):
                for p in players:
                    if not isinstance(p, dict):
                        continue
                    pid = p.get("id")
                    if not isinstance(pid, int):
                        continue
                    nm = p.get("name") if isinstance(p.get("name"), str) else f"Player{pid}"
                    names[pid] = nm

        for pid, lbl in enumerate(self._player_name_labels):
            if pid in names:
                lbl.config(text=names[pid], fg=PLAYER_COLORS.get(pid, "#111"))
            else:
                lbl.config(text="")

    def _toggle_autoplay(self) -> None:
        if self._autoplay_running:
            self._stop_autoplay()
        else:
            self._start_autoplay()

    def _start_autoplay(self) -> None:
        if not self.log.frames:
            return
        self._autoplay_running = True
        self._schedule_autoplay_tick()

    def _stop_autoplay(self) -> None:
        self._autoplay_running = False
        # Note: we don't cancel scheduled callbacks; the tick checks _autoplay_running.

    def _schedule_autoplay_tick(self) -> None:
        if self._autoplay_tick_scheduled:
            return
        self._autoplay_tick_scheduled = True
        self.after(self._autoplay_ms, self._autoplay_tick)

    def _autoplay_tick(self) -> None:
        self._autoplay_tick_scheduled = False
        if not self._autoplay_running or not self.log.frames:
            return

        cur = int(self.scale.get())
        if cur >= len(self.log.frames) - 1:
            self._stop_autoplay()
            return

        self._autoplay_internal_scale_change = True
        try:
            self.scale.set(cur + 1)
        finally:
            self._autoplay_internal_scale_change = False

        self._redraw()
        self._schedule_autoplay_tick()

    def _render_players_panel(self, text_widget: tk.Text, state: Optional[Dict[str, Any]]) -> None:
        # Avoid destroying/creating many Tk widgets on every redraw (can crash Tk on Windows).
        lines: List[str] = []
        dev_section_lines: List[str] = []
        awards_section_lines: List[str] = []
        player_row_lines: List[Tuple[int, int]] = []  # (1-based line_no in final text, pid)

        # Column widths for a stable monospace table.
        col_player = 8
        col_vp = 3
        col_brick = 5
        col_lumber = 6
        col_wool = 5
        col_grain = 5
        col_ore = 4
        col_total = 6

        def fmt_row(
            player: str,
            vp: str,
            brick: str,
            lumber: str,
            wool: str,
            grain: str,
            ore: str,
            total: str,
        ) -> str:
            return (
                f"{player:<{col_player}}"
                f"{vp:>{col_vp}} "
                f"{brick:>{col_brick}} "
                f"{lumber:>{col_lumber}} "
                f"{wool:>{col_wool}} "
                f"{grain:>{col_grain}} "
                f"{ore:>{col_ore}} "
                f"{total:>{col_total}}"
            )

        dev_order = [
            ("knight", "Knight"),
            ("road_building", "RoadBuilding"),
            ("year_of_plenty", "YearOfPlenty"),
            ("monopoly", "Monopoly"),
            ("victory_point", "VictoryPoint"),
        ]

        def dev_cards_breakdown(dev: Any) -> Tuple[int, List[Tuple[str, int]]]:
            if not isinstance(dev, dict):
                return (0, [(label, 0) for _key, label in dev_order])

            breakdown: List[Tuple[str, int]] = []
            computed_total = 0
            for key, label in dev_order:
                v = dev.get(key)
                v_i = int(v) if isinstance(v, int) else 0
                computed_total += v_i
                breakdown.append((label, v_i))

            total = dev.get("total")
            total_i = int(total) if isinstance(total, int) else computed_total
            return (total_i, breakdown)

        if not isinstance(state, dict):
            lines = ["(no state)"]
        else:
            players = state.get("players")
            if not isinstance(players, list) or not players:
                lines = ["(no players)"]
            else:
                lines.append(fmt_row("Player", "VP", "Brick", "Lumber", "Wool", "Grain", "Ore", "Total"))

                dev_section_lines.append("Dev Cards")

                # Collect award-related info while iterating.
                award_rows: List[Tuple[int, str, int, bool, int, bool]] = []
                # (pid, name, used_knights, largest_army_flag, longest_road_len, longest_road_flag)
                for p in players:
                    if not isinstance(p, dict):
                        continue
                    pid = p.get("id")
                    pid_i = int(pid) if isinstance(pid, int) else 2
                    name = p.get("name") if isinstance(p.get("name"), str) else f"Player{pid_i}"
                    vp = p.get("victory_points")
                    vp_i = int(vp) if isinstance(vp, int) else 0

                    used_knights = p.get("used_knights")
                    used_knights_i = int(used_knights) if isinstance(used_knights, int) else 0
                    la_flag = bool(p.get("largest_army_flag", False))
                    lr_len = p.get("longest_road_length")
                    lr_len_i = int(lr_len) if isinstance(lr_len, int) else 0
                    lr_flag = bool(p.get("longest_road_flag", False))
                    award_rows.append((pid_i, name, used_knights_i, la_flag, lr_len_i, lr_flag))

                    res = p.get("resources") if isinstance(p.get("resources"), dict) else {}
                    b = int(res.get("brick", 0)) if isinstance(res.get("brick"), int) else 0
                    l = int(res.get("lumber", 0)) if isinstance(res.get("lumber"), int) else 0
                    w = int(res.get("wool", 0)) if isinstance(res.get("wool"), int) else 0
                    g = int(res.get("grain", 0)) if isinstance(res.get("grain"), int) else 0
                    o = int(res.get("ore", 0)) if isinstance(res.get("ore"), int) else 0
                    total = b + l + w + g + o

                    _dev_total, dev_breakdown = dev_cards_breakdown(p.get("dev_cards"))
                    dev_parts = ", ".join([f"{label}={cnt}" for (label, cnt) in dev_breakdown])
                    dev_section_lines.append(f"{name}: {dev_parts}")

                    # Keep it compact and aligned.
                    lines.append(
                        fmt_row(
                            name[:col_player],
                            str(vp_i),
                            str(b),
                            str(l),
                            str(w),
                            str(g),
                            str(o),
                            str(total),
                        )
                    )
                    # Record the line number (1-based) where this player's row was appended.
                    player_row_lines.append((len(lines), pid_i))

                # Build award section (largest army / longest road / used knights / current LR length)
                if award_rows:
                    longest_road_current = max((r[4] for r in award_rows), default=0)
                    lr_holders = [r[1] for r in award_rows if r[5]]
                    la_holders = [r[1] for r in award_rows if r[3]]

                    awards_section_lines.append("Awards")
                    if lr_holders:
                        awards_section_lines.append(
                            f"Current Longest Road: len={longest_road_current} holder={', '.join(lr_holders)}"
                        )
                    else:
                        awards_section_lines.append(
                            f"Current Longest Road: len={longest_road_current} holder=None"
                        )

                    if la_holders:
                        awards_section_lines.append(
                            f"Largest Army holder={', '.join(la_holders)}"
                        )
                    else:
                        awards_section_lines.append("Largest Army holder=None")

                    for pid_i, name, used_k, la, lr_len_i, lr in award_rows:
                        awards_section_lines.append(
                            f"{name}: used_knights={used_k} | largest_army={'yes' if la else 'no'} | "
                            f"longest_road_len={lr_len_i} | longest_road={'yes' if lr else 'no'}"
                        )

                # Add a blank line + separate dev cards section.
                if dev_section_lines:
                    lines.append("")
                    lines.extend(dev_section_lines)

                if awards_section_lines:
                    lines.append("")
                    lines.extend(awards_section_lines)

        text_widget.configure(state=tk.NORMAL)
        text_widget.delete("1.0", tk.END)
        text_widget.insert(tk.END, "\n".join(lines))

        # Colorize player names (row header column) so it's easy to track red/blue.
        text_widget.tag_configure("player0", foreground=PLAYER_COLORS[0], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("player1", foreground=PLAYER_COLORS[1], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("player2", foreground=PLAYER_COLORS[2], font=("Consolas", 9, "bold"))

        # Colorize resource columns using tags.
        text_widget.tag_configure("res_brick", foreground=RESOURCE_TEXT_COLORS["brick"], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("res_lumber", foreground=RESOURCE_TEXT_COLORS["lumber"], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("res_wool", foreground=RESOURCE_TEXT_COLORS["wool"], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("res_grain", foreground=RESOURCE_TEXT_COLORS["grain"], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("res_ore", foreground=RESOURCE_TEXT_COLORS["ore"], font=("Consolas", 9, "bold"))
        text_widget.tag_configure("res_total", foreground="#111", font=("Consolas", 9, "bold"))

        def tag_slice(line_no: int, start_col: int, width: int, tag: str) -> None:
            text_widget.tag_add(tag, f"{line_no}.{start_col}", f"{line_no}.{start_col + width}")

        def player_tag_for(pid: int) -> str:
            return "player0" if pid == 0 else ("player1" if pid == 1 else "player2")

        # Compute column starts (based on fmt_row layout).
        # player | vp<space> | brick<space> | lumber<space> | wool<space> | grain<space> | ore<space> | total
        start_player = 0
        start_vp = start_player + col_player
        start_brick = start_vp + col_vp + 1
        start_lumber = start_brick + col_brick + 1
        start_wool = start_lumber + col_lumber + 1
        start_grain = start_wool + col_wool + 1
        start_ore = start_grain + col_grain + 1
        start_total = start_ore + col_ore + 1

        # Apply tags to header + each player line.
        for line_no, line in enumerate(lines, start=1):
            if len(line) < start_total + col_total:
                continue
            tag_slice(line_no, start_brick, col_brick, "res_brick")
            tag_slice(line_no, start_lumber, col_lumber, "res_lumber")
            tag_slice(line_no, start_wool, col_wool, "res_wool")
            tag_slice(line_no, start_grain, col_grain, "res_grain")
            tag_slice(line_no, start_ore, col_ore, "res_ore")
            tag_slice(line_no, start_total, col_total, "res_total")

        # Tag the player-name column for each player row.
        for line_no, pid_i in player_row_lines:
            tag_slice(line_no, 0, col_player, player_tag_for(pid_i))

        # Tag player names inside the dev-cards section (e.g., "Player0: ...").
        for line_no, line in enumerate(lines, start=1):
            if ":" not in line:
                continue
            prefix = line.split(":", 1)[0]
            if prefix.startswith("Player") and prefix[6:].isdigit():
                pid_i = int(prefix[6:])
                tag_slice(line_no, 0, min(len(prefix), col_player), player_tag_for(pid_i))

        text_widget.configure(state=tk.DISABLED)

    def _step(self, delta: int) -> None:
        if not self.log.frames:
            return
        # Manual navigation pauses autoplay.
        self._stop_autoplay()
        cur = int(self.scale.get())
        nxt = max(0, min(len(self.log.frames) - 1, cur + delta))
        self.scale.set(nxt)
        self._redraw()

    def _on_scale(self, _value: str) -> None:
        # If user drags the slider, pause autoplay. (Internal tick sets a guard.)
        if not self._autoplay_internal_scale_change:
            self._stop_autoplay()
        self._redraw()

    def _redraw(self) -> None:
        if not self.log.frames:
            return
        idx = int(self.scale.get())
        idx = max(0, min(len(self.log.frames) - 1, idx))
        fr = self.log.frames[idx]

        cp = fr.current_player
        cp_name = {0: "Player0", 1: "Player1", 2: "NoPlayer"}.get(cp, "?")
        ev_count = len(self.log.frame_events[idx]) if idx < len(self.log.frame_events) else 0
        self.header.config(
            text=(
                f"{fr.label} | current_player={cp_name} | frames={len(self.log.frames)} | events={ev_count}"
            )
        )

        self._update_players_bar(fr.state)
        self.renderer.draw(fr)

        stats = getattr(self.renderer, "last_stats", {}) or {}
        roads_total = int(stats.get("roads_total", 0))
        roads_drawn = int(stats.get("roads_drawn", 0))
        nodes_total = int(stats.get("nodes_total", 0))
        nodes_resolved = int(stats.get("nodes_pos_resolved", 0))
        if roads_total or nodes_total:
            missing = int(stats.get("roads_skipped_missing_node_pos", 0))
            too_long = int(stats.get("roads_skipped_too_long", 0))
            diag = f" | roads={roads_drawn}/{roads_total} (skip_missing={missing}, skip_long={too_long}) | nodes={nodes_resolved}/{nodes_total}"
            self.header.config(text=self.header.cget("text") + diag)

        # Player resources panels
        self._render_players_panel(self.players_start_text, fr.state)
        end_state = None
        if idx < len(self.log.frame_end_state):
            end_state = self.log.frame_end_state[idx]
        self._render_players_panel(self.players_end_text, end_state or fr.state)

        # Update right-side log
        lines = self.log.frame_events[idx] if idx < len(self.log.frame_events) else []
        self.log_text.configure(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        if lines:
            self.log_text.insert(tk.END, "\n".join(lines))
        else:
            self.log_text.insert(tk.END, "(no events captured for this frame)")
        self.log_text.configure(state=tk.DISABLED)


def _pick_file() -> Optional[str]:
    root = tk.Tk()
    root.withdraw()
    path = filedialog.askopenfilename(
        title="Select a Dumper JSONL log",
        filetypes=[("JSONL logs", "*.jsonl"), ("All files", "*.*")],
        initialdir=os.path.join(os.getcwd(), "logs"),
    )
    root.destroy()
    return path or None


def _find_last_replay() -> Optional[str]:
    """Find the newest JSONL replay log in common locations."""
    repo_root = Path(__file__).resolve().parent.parent
    candidates: List[Path] = []

    for folder in (repo_root / "logs", repo_root / "build" / "logs"):
        if folder.is_dir():
            candidates.extend([p for p in folder.glob("*.jsonl") if p.is_file()])

    if not candidates:
        return None

    newest = max(candidates, key=lambda p: p.stat().st_mtime)
    return str(newest)


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(description="CatanAPI replay log viewer (JSONL)")
    parser.add_argument("path", nargs="?", help="Path to a Dumper JSONL log")
    parser.add_argument(
        "-lr",
        "--last-replay",
        action="store_true",
        help="Open newest *.jsonl from logs/ (or build/logs/) and auto-play it",
    )
    args = parser.parse_args(argv[1:])

    path: Optional[str] = args.path
    autoplay = False
    if args.last_replay:
        autoplay = True
        path = _find_last_replay()
        if not path:
            print("No replay logs found under logs/ or build/logs/.")
            return 2

    if not path:
        path = _pick_file()
    if not path:
        print("No log selected.")
        return 2

    log = ReplayLog(path)
    app = App(log, autoplay=autoplay)
    app.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
