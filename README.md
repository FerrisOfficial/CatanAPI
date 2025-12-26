run tests:
cmake -S . -B build -DENABLE_DISPLAY=OFF;
cmake -S . -B build -DENABLE_DISPLAY=ON; 

cmake --build build; ctest --test-dir build  -V