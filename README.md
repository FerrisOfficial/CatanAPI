## Build

Configure:
cmake -S . -B build -G "MinGW Makefiles"

Build:
cmake --build build

## Tests

cmake --build build; ctest --test-dir build -V --output-on-failure --stop-on-failure

## Run simulation

The runner is `run.exe` and takes 2 positional arguments: player flags for Player0 and Player1.

Options:
- `-n, --games <N>`: number of games to run (default: 1)
- `--no-dump`: disable JSONL dumper logs (recommended for batch runs)

Currently supported flags:
- `rp` = RandomPlayer
- `it1` = It1Player
- `it2` = It2Player
- `it3` = It3Player
- `it4` = It4Player
- `it5` = It5Player
- `para` = ParaPlayer (sterowany parametrami z pliku)

ParaPlayer czyta konfigurację tylko z:
- `./players/paraPlayer.cfg`

Trenowanie parametrów (prosty ewolucyjny search po win-rate):
- `python utils/train_para_player.py --run-exe build/runs/run.exe --opponent rp --games 200 --generations 30`

### Run directly

cmake --build build --target run
build\runs\run.exe rp rp

Batch run example:
build\runs\run.exe -n 1000 --no-dump rp rp

### Pass flags via CMake (configure-time)

cmake -S . -B build -DRUN_P0=rp -DRUN_P1=rp
cmake --build build --target play
