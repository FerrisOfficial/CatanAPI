# Tests CMakeLists.txt

# Create test executables
add_executable(
  player_tests
  test_player.cpp
)

# Link against gtest_main and the game_simulation library
target_link_libraries(
  player_tests
  gtest_main
  game_simulation
)

# Include GoogleTest's CMake functions
include(GoogleTest)

# Discover tests
gtest_discover_tests(player_tests)