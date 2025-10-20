# Tests CMakeLists.txt

# Create test executables
add_executable(
  player_tests
  test_player.cpp
)

add_executable(
  actions_tests
  test_actions.cpp
)

add_executable(
  board_tests
  test_board.cpp
)

add_executable(
  game_tests
  test_game.cpp
)

# Link against gtest_main and the game_simulation library
target_link_libraries(
  player_tests
  gtest_main
  game_simulation
)

target_link_libraries(
  actions_tests
  gtest_main
  game_simulation
)

target_link_libraries(
  board_tests
  gtest_main
  game_simulation
)

target_link_libraries(
  game_tests
  gtest_main
  game_simulation
)

# Include GoogleTest's CMake functions
include(GoogleTest)

# Discover tests
gtest_discover_tests(player_tests)
gtest_discover_tests(actions_tests)
gtest_discover_tests(board_tests)
gtest_discover_tests(game_tests)