## Build

Configure:
cmake -S . -B build -G "MinGW Makefiles"

Build:
cmake --build build

## Tests

cmake --build build; ctest --test-dir build -V --output-on-failure --stop-on-failure

## Run simulation

The runner is `run.exe` and takes 2 arguments: player flags for Player0 and Player1.

Currently supported flags:
- `rp` = RandomPlayer

### Run directly

cmake --build build --target run
build\runs\run.exe rp rp

### Pass flags via CMake (configure-time)

cmake -S . -B build -DRUN_P0=rp -DRUN_P1=rp
cmake --build build --target play
