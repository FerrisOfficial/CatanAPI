"""Train ParaPlayer parameters by win-rate.

This script writes candidate config files and runs the C++ runner to estimate win-rate.
It uses a simple evolutionary search (elitism + Gaussian mutations).

Example (after building):
  python utils/train_para_player.py --run-exe build/runs/run.exe --games 200 --generations 30 --opponent rp

Notes:
- The runner is invoked with --switch to reduce seat bias.
- ParaPlayer reads config from ./players/parametricPlayers/paraPlayer.cfg.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import random
import re
import subprocess
import sys
import threading
import time
import math
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List, Tuple


BYBOT_RE = re.compile(r"^ByBot:\s*(.*)$")
PAIR_RE = re.compile(r"([A-Za-z0-9_\-]+)=(\d+)")

PARA_CFG_ENV = "CATAN_PARA_CFG"


def find_default_run_exe(repo_root: Path) -> Path | None:
    candidates = [
        repo_root / "build" / "runs" / "run.exe",
        repo_root / "build" / "bin" / "run.exe",
        repo_root / "build-asan" / "runs" / "run.exe",
        repo_root / "build-asan" / "bin" / "run.exe",
    ]
    for c in candidates:
        if c.exists():
            return c
    return None


def read_cfg(path: Path) -> Dict[str, float]:
    d: Dict[str, float] = {}
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or line.startswith("//"):
            continue
        if "=" not in line:
            continue
        k, v = line.split("=", 1)
        k = k.strip()
        v = v.strip()
        try:
            d[k] = float(v)
        except ValueError:
            continue
    return d


def write_cfg(path: Path, params: Dict[str, float], header: str | None = None) -> None:
    lines: List[str] = []
    if header:
        lines.append(header.rstrip())
    for k in sorted(params.keys()):
        # keep ints readable
        v = params[k]
        if abs(v - round(v)) < 1e-9:
            lines.append(f"{k} = {int(round(v))}")
        else:
            lines.append(f"{k} = {v:.6g}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_bybot(output: str) -> Dict[str, int]:
    for line in output.splitlines():
        m = BYBOT_RE.match(line.strip())
        if not m:
            continue
        rest = m.group(1)
        out: Dict[str, int] = {}
        for m2 in PAIR_RE.finditer(rest):
            out[m2.group(1)] = int(m2.group(2))
        return out
    return {}


def run_eval(
    run_exe: Path,
    games: int,
    para_cfg: Path,
    opponent: str,
    seed: int,
    timeout_s: int = 10_000,
    *,
    live_runner_output: bool = False,
    heartbeat_s: float | None = None,
    label: str | None = None,
    retries: int = 0,
) -> Tuple[int, int]:
    attempt = 0
    while True:
        try:
            return _run_eval_once(
                run_exe,
                games,
                para_cfg,
                opponent,
                seed,
                timeout_s,
                live_runner_output=live_runner_output,
                heartbeat_s=heartbeat_s,
                label=label,
            )
        except (RuntimeError, TimeoutError) as e:
            if attempt >= retries:
                raise
            attempt += 1
            # Small backoff: runner sometimes flakes (rare) on long runs.
            time.sleep(min(2.0, 0.2 * attempt))


def _run_eval_once(
    run_exe: Path,
    games: int,
    para_cfg: Path,
    opponent: str,
    seed: int,
    timeout_s: int = 10_000,
    *,
    live_runner_output: bool = False,
    heartbeat_s: float | None = None,
    label: str | None = None,
) -> Tuple[int, int]:
    env = os.environ.copy()
    env["CATAN_SEED"] = str(seed)
    # Allow ParaPlayer to load a per-process config file (required for parallel training).
    env[PARA_CFG_ENV] = str(para_cfg.resolve())

    cmd = [
        str(run_exe),
        "--no-dump",
        "--switch",
        "--games",
        str(games),
        "para",
        opponent,
    ]

    runner_cwd = (
        str(run_exe.parent.parent.parent)
        if run_exe.parts[-3:] == ("build", "runs", "run.exe")
        else None
    )

    # Use Popen so we can provide progress (gate runs can be long).
    p = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=env,
        cwd=runner_cwd,
        bufsize=1,
    )

    stdout_lines: List[str] = []
    stderr_lines: List[str] = []
    last_activity = time.monotonic()
    started = time.monotonic()

    def reader(stream, sink: List[str], prefix: str) -> None:
        nonlocal last_activity
        try:
            for line in iter(stream.readline, ""):
                sink.append(line)
                last_activity = time.monotonic()
                if live_runner_output:
                    # keep runner output visible (helps confirm it isn't stuck)
                    print(f"{prefix}{line.rstrip()}")
        finally:
            try:
                stream.close()
            except Exception:
                pass

    t_out = threading.Thread(target=reader, args=(p.stdout, stdout_lines, "[run] "), daemon=True)
    t_err = threading.Thread(target=reader, args=(p.stderr, stderr_lines, "[run-err] "), daemon=True)
    t_out.start()
    t_err.start()

    # Heartbeat: print periodically even if runner is quiet.
    next_hb = time.monotonic() + (heartbeat_s or 0)

    try:
        while True:
            rc = p.poll()
            now = time.monotonic()
            if rc is not None:
                break

            if heartbeat_s is not None and now >= next_hb:
                elapsed = int(now - started)
                since = int(now - last_activity)
                tag = label or f"{opponent} games={games}"
                print(f"... still running ({tag}) elapsed={elapsed}s idle={since}s")
                next_hb = now + heartbeat_s

            if timeout_s is not None and (now - started) > float(timeout_s):
                p.kill()
                raise TimeoutError(f"runner timeout after {timeout_s}s ({label or opponent})")

            time.sleep(0.25)
    finally:
        # Ensure threads terminate.
        try:
            p.stdout and p.stdout.close()
        except Exception:
            pass
        try:
            p.stderr and p.stderr.close()
        except Exception:
            pass

    t_out.join(timeout=1.0)
    t_err.join(timeout=1.0)

    stdout = "".join(stdout_lines)
    stderr = "".join(stderr_lines)

    if p.returncode != 0:
        raise RuntimeError(f"runner failed ({p.returncode})\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}")

    wins = parse_bybot(stdout)
    return wins.get("para", 0), games


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


class JsonlLogger:
    def __init__(self, path: Path | None) -> None:
        self._path = path

    @property
    def path(self) -> Path | None:
        return self._path

    def log(self, record: Dict[str, Any]) -> None:
        if self._path is None:
            return
        self._path.parent.mkdir(parents=True, exist_ok=True)
        with self._path.open("a", encoding="utf-8") as f:
            f.write(json.dumps(record, ensure_ascii=False, sort_keys=True) + "\n")
            f.flush()


@dataclass
class SearchConfig:
    games: int
    gate_games: int
    max_generations: int
    population: int
    sigma: float
    eval_schedule: List[int]
    survivors_frac: float
    best_verify_games: int
    retries: int
    workers: int
    curriculum: List[str]
    seed: int
    target_winrate: float
    runner_live: str
    gate_heartbeat_s: float


def _safe_name(s: str) -> str:
    out = []
    for ch in str(s):
        if ch.isalnum() or ch in ("_", "-", "."):
            out.append(ch)
        else:
            out.append("_")
    return "".join(out) or "x"


def _write_population_cfgs(stage_tmp_dir: Path, pop: List[Dict[str, float]], *, stage_idx: int, opponent: str, gen: int) -> List[Path]:
    stage_tmp_dir.mkdir(parents=True, exist_ok=True)
    cfgs: List[Path] = []
    for i, params in enumerate(pop):
        p = stage_tmp_dir / f"cand_{i:03d}.cfg"
        write_cfg(p, params, header=f"# generated by train_para_player.py (stage={stage_idx} vs {opponent} gen={gen} cand={i})")
        cfgs.append(p)
    return cfgs


def _eval_worker(
    run_exe: str,
    games: int,
    para_cfg: str,
    opponent: str,
    seed: int,
    retries: int,
) -> Tuple[int, int, int]:
    wins, total = run_eval(Path(run_exe), int(games), Path(para_cfg), str(opponent), int(seed), retries=int(retries))
    return int(wins), int(total), int(seed)


def _run_tasks_parallel(
    tasks: List[Tuple[int, int, Path, str, int]],
    *,
    run_exe: Path,
    retries: int,
    workers: int,
) -> Dict[int, Tuple[int, int, int]]:
    """Run tasks in parallel.

    tasks: list of (cand_idx, games, cfg_path, opponent, seed)
    returns: cand_idx -> (wins, total, seed)
    """
    out: Dict[int, Tuple[int, int, int]] = {}
    if workers <= 1 or len(tasks) <= 1:
        for cand_idx, games, cfg_path, opponent, seed in tasks:
            wins, total = run_eval(run_exe, games, cfg_path, opponent, seed, retries=retries)
            out[int(cand_idx)] = (int(wins), int(total), int(seed))
        return out

    ex: concurrent.futures.ProcessPoolExecutor | None = None
    aborted = False
    futures: Dict[concurrent.futures.Future[Tuple[int, int, int]], int] = {}
    try:
        ex = concurrent.futures.ProcessPoolExecutor(max_workers=int(workers))
        for cand_idx, games, cfg_path, opponent, seed in tasks:
            fut = ex.submit(
                _eval_worker,
                str(run_exe),
                int(games),
                str(cfg_path),
                str(opponent),
                int(seed),
                int(retries),
            )
            futures[fut] = int(cand_idx)

        for fut in concurrent.futures.as_completed(futures):
            cand_idx = futures[fut]
            wins, total, seed = fut.result()
            out[int(cand_idx)] = (int(wins), int(total), int(seed))
        return out
    except KeyboardInterrupt:
        aborted = True
        if ex is not None:
            ex.shutdown(wait=False, cancel_futures=True)
        raise
    finally:
        if ex is not None:
            ex.shutdown(wait=(not aborted), cancel_futures=aborted)


def clamp_param(key: str, val: float) -> float:
    # Lightweight safety: keep weights within sane bounds.
    if key.startswith("policy."):
        if key == "policy.temperature":
            return max(0.0, min(val, 50000.0))
        if key == "policy.epsilon":
            return max(0.0, min(val, 0.5))
        if key == "policy.top_k":
            return float(max(1, min(int(round(val)), 50)))
        if key == "policy.lookahead_top_m":
            return float(max(0, min(int(round(val)), 50)))

    if key.startswith("eval.w_"):
        return max(-200000.0, min(val, 200000.0))

    if key.startswith("prod."):
        return max(-1000.0, min(val, 1000.0))

    if key.startswith("action.") or key.startswith("robber.") or key.startswith("discard."):
        return max(-100000.0, min(val, 100000.0))

    return val


def mutate(base: Dict[str, float], keys: Iterable[str], sigma: float) -> Dict[str, float]:
    out = dict(base)
    for k in keys:
        v = out.get(k, 0.0)
        v2 = v + random.gauss(0.0, sigma) * (abs(v) + 1.0)
        out[k] = clamp_param(k, v2)
    return out


def parse_int_list(csv: str) -> List[int]:
    parts = [p.strip() for p in str(csv).split(",") if p.strip()]
    out: List[int] = []
    for p in parts:
        out.append(int(p))
    return out


def successive_halving(
    run_exe: Path,
    candidate_cfgs: List[Path],
    opponent: str,
    stage_idx: int,
    gen: int,
    pop: List[Dict[str, float]],
    eval_schedule: List[int],
    survivors_frac: float,
    *,
    seed_base: int,
    retries: int,
    workers: int,
    logger: "JsonlLogger",
) -> Tuple[float, Dict[str, float]]:
    alive: List[int] = list(range(len(pop)))
    last_fit: Dict[int, float] = {}

    for round_idx, games in enumerate(eval_schedule):
        results: List[Tuple[float, int, int, int]] = []
        tasks: List[Tuple[int, int, Path, str, int]] = []
        for i in alive:
            seed = seed_base + round_idx * 10_000 + i
            tasks.append((i, int(games), candidate_cfgs[i], opponent, int(seed)))

        by_i = _run_tasks_parallel(tasks, run_exe=run_exe, retries=retries, workers=workers)

        for i in alive:
            wins, total, seed = by_i[i]
            fit = wins / float(total)
            last_fit[i] = fit
            results.append((fit, i, wins, total))

            logger.log(
                {
                    "ts": utc_now_iso(),
                    "event": "candidate_round",
                    "stage": stage_idx,
                    "opponent": opponent,
                    "gen": gen,
                    "round": round_idx,
                    "games": games,
                    "cand": i,
                    "seed": seed,
                    "wins": wins,
                    "total": total,
                    "winrate": fit,
                    "winrate_pct": fit * 100.0,
                    "sigma": None,
                }
            )

            print(
                f"stage={stage_idx:02d}({opponent}) gen={gen:03d} round={round_idx} cand={i:02d} winrate={fit * 100.0:6.2f}% (wins={wins}/{total})"
            )

        results.sort(key=lambda x: x[0], reverse=True)
        keep = max(1, int(math.ceil(len(results) * float(survivors_frac))))
        alive = [idx for (_, idx, _, _) in results[:keep]]

    best_i = max(alive, key=lambda i: last_fit.get(i, -1.0))
    return float(last_fit[best_i]), pop[best_i]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--run-exe", type=Path, default=None)
    ap.add_argument(
        "--base-cfg",
        type=Path,
        default=None,
        help="(Deprecated) Ignored. ParaPlayer uses only ./players/parametricPlayers/paraPlayer.cfg.",
    )
    ap.add_argument(
        "--out",
        type=Path,
        default=Path("players/parametricPlayers/paraPlayer.cfg"),
        help="(Deprecated) Ignored. ParaPlayer uses only ./players/parametricPlayers/paraPlayer.cfg.",
    )
    ap.add_argument(
        "--log-file",
        type=Path,
        default=None,
        help="Append JSONL training log to this file (use empty string to disable). Default: logs/paraPlayer_train_<timestamp>.jsonl",
    )

    ap.add_argument(
        "--opponent",
        type=str,
        default=None,
        help="(Deprecated) Single opponent flag. Prefer --curriculum.",
    )
    ap.add_argument(
        "--curriculum",
        type=str,
        default="rp,it1,it2,it3,it4,it5",
        help="Comma-separated opponent list; train sequentially until target is reached for each.",
    )
    ap.add_argument("--games", type=int, default=200)
    ap.add_argument(
        "--eval-schedule",
        type=str,
        default="",
        help="Comma-separated games for successive-halving evaluation (e.g. '60,150,400'). Empty => use --games only.",
    )
    ap.add_argument(
        "--survivors-frac",
        type=float,
        default=0.5,
        help="Fraction of candidates kept after each eval-schedule round (0<frac<=1).",
    )
    ap.add_argument(
        "--best-verify-games",
        type=int,
        default=2000,
        help="Extra verification games for a candidate before accepting it as new best (0 disables).",
    )
    ap.add_argument(
        "--retries",
        type=int,
        default=2,
        help="Retry runner calls this many times on rare failures/timeouts.",
    )
    ap.add_argument(
        "--gate-games",
        type=int,
        default=2000,
        help="Number of games for stage gate-check (decides whether to skip a stage)",
    )
    ap.add_argument(
        "--generations",
        type=int,
        default=200,
        help="Maximum number of generations per stage (training stops earlier if target is reached). Use 0 for unlimited.",
    )
    ap.add_argument("--population", type=int, default=16)
    ap.add_argument("--sigma", type=float, default=0.12)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument(
        "--target-winrate",
        type=float,
        default=0.90,
        help="Stop early when best candidate reaches this winrate vs opponent",
    )
    ap.add_argument(
        "--runner-live",
        type=str,
        default="gate",
        choices=["off", "gate", "all"],
        help="Show runner stdout/stderr live: off|gate|all",
    )
    ap.add_argument(
        "--gate-heartbeat",
        type=float,
        default=5.0,
        help="Seconds between heartbeat prints during gate checks (set 0 to disable)",
    )
    ap.add_argument(
        "--workers",
        type=int,
        default=1,
        help="Parallel worker processes for candidate evaluation (1 disables parallelism).",
    )

    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[1]

    canonical_cfg = repo_root / "players" / "parametricPlayers" / "paraPlayer.cfg"

    run_exe = args.run_exe
    if run_exe is None:
        run_exe = find_default_run_exe(repo_root)
    if run_exe is None or not run_exe.exists():
        print("Cannot find run.exe; pass --run-exe", file=sys.stderr)
        return 2

    if args.base_cfg is not None and args.base_cfg.resolve() != canonical_cfg.resolve():
        print(f"Warning: ignoring --base-cfg={args.base_cfg}; using {canonical_cfg}")
    if args.out is not None and args.out.resolve() != canonical_cfg.resolve():
        print(f"Warning: ignoring --out={args.out}; using {canonical_cfg}")

    base_cfg = canonical_cfg
    if not base_cfg.exists():
        print(f"ParaPlayer cfg not found: {base_cfg}", file=sys.stderr)
        return 2

    random.seed(args.seed)

    base_params = read_cfg(base_cfg)

    # Select trainable keys (skip top_k: it is discrete, keep fixed by default).
    train_keys = [k for k in base_params.keys() if k not in ("policy.top_k", "policy.lookahead_top_m")]

    curriculum: List[str]
    if args.opponent is not None:
        curriculum = [args.opponent]
    else:
        curriculum = [s.strip() for s in str(args.curriculum).split(",") if s.strip()]
    if not curriculum:
        print("Empty curriculum; pass --curriculum or --opponent", file=sys.stderr)
        return 2

    schedule = parse_int_list(args.eval_schedule) if str(args.eval_schedule).strip() else [int(args.games)]
    if any(g <= 0 for g in schedule):
        print(f"Invalid --eval-schedule={args.eval_schedule} (all must be >0)", file=sys.stderr)
        return 2
    if not (0.0 < float(args.survivors_frac) <= 1.0):
        print(f"Invalid --survivors-frac={args.survivors_frac} (must be in (0,1])", file=sys.stderr)
        return 2
    if int(args.workers) <= 0:
        print(f"Invalid --workers={args.workers} (must be >=1)", file=sys.stderr)
        return 2

    cfg = SearchConfig(
        games=args.games,
        gate_games=args.gate_games,
        max_generations=args.generations,
        population=args.population,
        sigma=args.sigma,
        eval_schedule=schedule,
        survivors_frac=float(args.survivors_frac),
        best_verify_games=int(args.best_verify_games),
        retries=int(args.retries),
        workers=int(args.workers),
        curriculum=curriculum,
        seed=args.seed,
        target_winrate=args.target_winrate,
        runner_live=args.runner_live,
        gate_heartbeat_s=float(args.gate_heartbeat),
    )

    run_ts = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")

    log_file: Path | None
    if args.log_file is None:
        log_file = Path("logs") / f"paraPlayer_train_{run_ts}.jsonl"
    elif str(args.log_file).strip() == "":
        log_file = None
    else:
        log_file = args.log_file
    logger = JsonlLogger(log_file)

    best_params = dict(base_params)
    best_wins = -1
    best_winrate: float | None = None
    last_stage_completed: int = -1
    last_opponent: str | None = None

    print(f"Runner: {run_exe}")
    print(f"Base cfg: {base_cfg}")
    print(f"Curriculum: {', '.join(cfg.curriculum)}")
    if len(cfg.eval_schedule) == 1 and cfg.eval_schedule[0] == cfg.games:
        print(f"Eval: {cfg.games} games per candidate (switch seats)")
    else:
        print(f"Eval schedule: {cfg.eval_schedule} (survivors frac {cfg.survivors_frac})")
    print(f"Gate: {cfg.gate_games} games per stage (switch seats)")
    print(f"Target: {cfg.target_winrate * 100.0:.1f}% winrate")
    print(f"Runner live: {cfg.runner_live} (gate heartbeat {cfg.gate_heartbeat_s:.1f}s)")
    print(f"Runner retries: {cfg.retries}")
    print(f"Workers: {cfg.workers}")
    if cfg.best_verify_games > 0:
        print(f"New-best verify: {cfg.best_verify_games} games")
    if logger.path is not None:
        print(f"Log file: {logger.path}")

    logger.log(
        {
            "ts": utc_now_iso(),
            "event": "run_start",
            "run_exe": str(run_exe),
            "runner_cmd": [str(run_exe), "--no-dump", "--switch", "--games", str(cfg.games), "para", "<opponent>"],
            "runner_cwd": (
                str(run_exe.parent.parent.parent)
                if run_exe.parts[-3:] == ("build", "runs", "run.exe")
                else None
            ),
            "base_cfg": str(base_cfg),
            "out_cfg": str(base_cfg),
            "curriculum": cfg.curriculum,
            "games": cfg.games,
            "gate_games": cfg.gate_games,
            "population": cfg.population,
            "sigma": cfg.sigma,
            "eval_schedule": cfg.eval_schedule,
            "survivors_frac": cfg.survivors_frac,
            "best_verify_games": cfg.best_verify_games,
            "retries": cfg.retries,
            "workers": cfg.workers,
            "seed": cfg.seed,
            "max_generations": cfg.max_generations,
            "target_winrate": cfg.target_winrate,
            "runner_live": cfg.runner_live,
            "gate_heartbeat_s": cfg.gate_heartbeat_s,
            "python": sys.version,
        }
    )
    tmp_root = Path("logs") / f"tmp_para_cfg_{run_ts}"

    interrupted = False
    try:
        # Train sequentially vs curriculum opponents.
        for stage_idx, opponent in enumerate(cfg.curriculum):
            last_opponent = opponent
            logger.log(
                {
                    "ts": utc_now_iso(),
                    "event": "stage_start",
                    "stage": stage_idx,
                    "opponent": opponent,
                    "target_winrate": cfg.target_winrate,
                    "games": cfg.games,
                }
            )
            print(f"\n=== STAGE {stage_idx}: opponent={opponent} target={cfg.target_winrate * 100.0:.1f}% ===")

            # Quick gate check: if current best already reaches target, skip stage.
            write_cfg(base_cfg, best_params, header="# gate eval (train_para_player.py)")
            gate_seed = cfg.seed + stage_idx * 1_000_000
            gate_wins, gate_total = run_eval(
                run_exe,
                cfg.gate_games,
                base_cfg,
                opponent,
                gate_seed,
                live_runner_output=(cfg.runner_live in ("gate", "all")),
                heartbeat_s=(cfg.gate_heartbeat_s if cfg.gate_heartbeat_s > 0 else None),
                label=f"gate stage={stage_idx} vs {opponent}",
                retries=cfg.retries,
            )
            gate_wr = gate_wins / float(gate_total)
            print(f"gate winrate vs {opponent}: {gate_wr * 100.0:.2f}% (wins={gate_wins}/{gate_total})")
            logger.log(
                {
                    "ts": utc_now_iso(),
                    "event": "stage_gate",
                    "stage": stage_idx,
                    "opponent": opponent,
                    "seed": gate_seed,
                    "wins": gate_wins,
                    "total": gate_total,
                    "winrate": gate_wr,
                    "winrate_pct": gate_wr * 100.0,
                    "gate_games": cfg.gate_games,
                }
            )
            if gate_wr >= cfg.target_winrate:
                print(f"stage already beaten (>= {cfg.target_winrate * 100.0:.1f}%), moving on")
                logger.log(
                    {
                        "ts": utc_now_iso(),
                        "event": "stage_skip",
                        "stage": stage_idx,
                        "opponent": opponent,
                        "reason": "already_beaten",
                        "gate_winrate": gate_wr,
                        "gate_winrate_pct": gate_wr * 100.0,
                        "gate_wins": gate_wins,
                        "gate_total": gate_total,
                    }
                )
                last_stage_completed = stage_idx
                continue

            # Reset annealing per stage.
            stage_sigma = cfg.sigma

            no_improve_streak = 0

            stage_tmp_dir = tmp_root / f"stage{stage_idx:02d}_{_safe_name(opponent)}"

            gen = 0
            while True:
                if cfg.max_generations > 0 and gen >= cfg.max_generations:
                    # exhausted generations for this stage
                    logger.log(
                        {
                            "ts": utc_now_iso(),
                            "event": "stage_exhausted",
                            "stage": stage_idx,
                            "opponent": opponent,
                            "max_generations": cfg.max_generations,
                        }
                    )
                    print(f"stage exhausted: opponent={opponent} (max generations reached)")
                    # If generations are bounded, do not proceed further because later opponents will be harder.
                    # If unlimited, we never reach this branch.
                    break

                candidates: List[Tuple[float, Dict[str, float]]] = []

                # Always include current best.
                pop = [best_params] + [mutate(best_params, train_keys, stage_sigma) for _ in range(cfg.population - 1)]

                # Write per-candidate cfg files (safe for parallel eval).
                cand_cfgs = _write_population_cfgs(stage_tmp_dir, pop, stage_idx=stage_idx, opponent=opponent, gen=gen)

                # Evaluate population. If an eval schedule was provided, use successive-halving to save time and reduce noise.
                seed_base = cfg.seed + stage_idx * 1_000_000 + gen * 1000
                if len(cfg.eval_schedule) > 1:
                    top_fit, top_params = successive_halving(
                        run_exe,
                        cand_cfgs,
                        opponent,
                        stage_idx,
                        gen,
                        pop,
                        cfg.eval_schedule,
                        cfg.survivors_frac,
                        seed_base=seed_base,
                        retries=cfg.retries,
                        workers=cfg.workers,
                        logger=logger,
                    )
                else:
                    tasks: List[Tuple[int, int, Path, str, int]] = []
                    for i in range(len(pop)):
                        seed = seed_base + i
                        tasks.append((i, int(cfg.games), cand_cfgs[i], opponent, int(seed)))

                    try:
                        by_i = _run_tasks_parallel(tasks, run_exe=run_exe, retries=cfg.retries, workers=cfg.workers)
                    except Exception as e:  # noqa: BLE001
                        logger.log(
                            {
                                "ts": utc_now_iso(),
                                "event": "candidate_error",
                                "stage": stage_idx,
                                "opponent": opponent,
                                "gen": gen,
                                "sigma": stage_sigma,
                                "games": cfg.games,
                                "error": repr(e),
                            }
                        )
                        raise

                    for i, params in enumerate(pop):
                        wins, total, seed = by_i[i]
                        fitness = wins / float(total)
                        candidates.append((fitness, params))

                        logger.log(
                            {
                                "ts": utc_now_iso(),
                                "event": "candidate",
                                "stage": stage_idx,
                                "opponent": opponent,
                                "gen": gen,
                                "cand": i,
                                "seed": seed,
                                "sigma": stage_sigma,
                                "wins": wins,
                                "total": total,
                                "winrate": fitness,
                                "winrate_pct": fitness * 100.0,
                                "games": cfg.games,
                                "runner_cmd": [
                                    str(run_exe),
                                    "--no-dump",
                                    "--switch",
                                    "--games",
                                    str(cfg.games),
                                    "para",
                                    opponent,
                                ],
                                "runner_cwd": (
                                    str(run_exe.parent.parent.parent)
                                    if run_exe.parts[-3:] == ("build", "runs", "run.exe")
                                    else None
                                ),
                                "cfg_path": str(cand_cfgs[i]),
                                "duration_s": None,
                                "params": params,
                            }
                        )

                        print(
                            f"stage={stage_idx:02d}({opponent}) gen={gen:03d} cand={i:02d} winrate={fitness * 100.0:6.2f}% (wins={wins}/{total})"
                        )

                    candidates.sort(key=lambda x: x[0], reverse=True)
                    top_fit, top_params = candidates[0]

                logger.log(
                    {
                        "ts": utc_now_iso(),
                        "event": "generation_end",
                        "stage": stage_idx,
                        "opponent": opponent,
                        "gen": gen,
                        "sigma": stage_sigma,
                        "best_candidate_winrate": top_fit,
                        "best_candidate_winrate_pct": top_fit * 100.0,
                        "best_candidate_cfg": str(base_cfg),
                        "target_winrate": cfg.target_winrate,
                    }
                )

                improved = (best_winrate is None) or (top_fit > best_winrate)
                if improved:
                    accepted_fit = float(top_fit)
                    accepted_params = dict(top_params)

                    # Optional verification pass to reduce overfitting to small eval samples.
                    if cfg.best_verify_games > 0:
                        write_cfg(base_cfg, accepted_params, header="# verify eval (train_para_player.py)")
                        verify_seed = cfg.seed + stage_idx * 1_000_000 + gen * 1000 + 99_000
                        vwins, vtotal = run_eval(
                            run_exe,
                            cfg.best_verify_games,
                            base_cfg,
                            opponent,
                            verify_seed,
                            retries=cfg.retries,
                        )
                        vfit = vwins / float(vtotal)
                        logger.log(
                            {
                                "ts": utc_now_iso(),
                                "event": "new_best_verify",
                                "stage": stage_idx,
                                "opponent": opponent,
                                "gen": gen,
                                "seed": verify_seed,
                                "wins": vwins,
                                "total": vtotal,
                                "winrate": vfit,
                                "winrate_pct": vfit * 100.0,
                                "eval_winrate": top_fit,
                                "eval_winrate_pct": top_fit * 100.0,
                            }
                        )
                        accepted_fit = float(vfit)

                    if best_winrate is None or accepted_fit > best_winrate:
                        best_winrate = float(accepted_fit)
                        best_wins = int(round(best_winrate * float(cfg.games)))
                        best_params = dict(accepted_params)
                        print(
                            f"NEW BEST: stage={stage_idx:02d}({opponent}) gen={gen:03d} winrate={best_winrate * 100.0:.2f}%"
                        )

                        write_cfg(base_cfg, best_params, header="# best cfg (autogenerated)")

                    logger.log(
                        {
                            "ts": utc_now_iso(),
                            "event": "new_best",
                            "stage": stage_idx,
                            "opponent": opponent,
                            "gen": gen,
                            "winrate": best_winrate,
                            "winrate_pct": best_winrate * 100.0,
                            "wins": best_wins,
                            "total": cfg.games,
                            "out_cfg": str(base_cfg),
                            "params": best_params,
                        }
                    )

                    if best_winrate is not None and best_winrate >= cfg.target_winrate:
                        print(
                            f"STAGE TARGET REACHED: stage={stage_idx:02d}({opponent}) gen={gen:03d} winrate={best_winrate * 100.0:.2f}% >= {cfg.target_winrate * 100.0:.2f}%"
                        )
                        write_cfg(
                            base_cfg,
                            dict(best_params),
                            header=f"# best cfg (autogenerated; stage {stage_idx} target reached vs {opponent})",
                        )
                        logger.log(
                            {
                                "ts": utc_now_iso(),
                                "event": "stage_target_reached",
                                "stage": stage_idx,
                                "opponent": opponent,
                                "gen": gen,
                                "winrate": best_winrate,
                                "winrate_pct": best_winrate * 100.0,
                                "wins": None,
                                "total": None,
                                "out_cfg": str(base_cfg),
                                "params": dict(best_params),
                            }
                        )
                        last_stage_completed = stage_idx
                        break

                    no_improve_streak = 0
                    # Mild annealing within stage on improvement.
                    stage_sigma = max(0.02, stage_sigma * 0.98)
                else:
                    no_improve_streak += 1
                    # If we aren't improving, very gently increase sigma to escape local optima.
                    stage_sigma = min(cfg.sigma * 1.5, max(0.02, stage_sigma * 1.01))

                gen += 1

                # If we hit the generation limit and exhausted the stage, stop the curriculum.
                if cfg.max_generations > 0 and gen >= cfg.max_generations and last_stage_completed != stage_idx:
                    break
    except KeyboardInterrupt:
        interrupted = True
        print("\nInterrupted (Ctrl+C). Saving best and exiting...")
    finally:
        # Always persist the best known parameters on exit.
        try:
            if best_params:
                write_cfg(base_cfg, dict(best_params), header="# best cfg (autogenerated; train_para_player.py exit)")
        except Exception:
            pass

        print(f"Saved best to: {base_cfg}")
        logger.log(
            {
                "ts": utc_now_iso(),
                "event": "run_end",
                "interrupted": interrupted,
                "best_winrate": best_winrate,
                "best_winrate_pct": (best_winrate * 100.0) if best_winrate is not None else None,
                "best_out": str(base_cfg),
                "last_stage_completed": last_stage_completed,
                "last_opponent": last_opponent,
            }
        )

    return 130 if interrupted else 0


if __name__ == "__main__":
    raise SystemExit(main())
