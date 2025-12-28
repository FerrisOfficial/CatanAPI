run tests:
cmake -S . -B build -G "MinGW Makefiles"
cmake -S . -B build
cmake -B build -DLOGGING=ON

cmake --build build; ctest --test-dir build  -V --output-on-failure --stop-on-failure

cmake --build build; build\runs\run_random.exe
