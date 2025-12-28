run tests:
cmake -S . -B build -G "MinGW Makefiles"
cmake -S . -B build

cmake --build build; ctest --test-dir build  -V --output-on-failure --stop-on-failure
