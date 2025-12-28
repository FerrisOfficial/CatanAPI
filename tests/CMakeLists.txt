# ----------------------------
# 🧪 Create test executables
# ----------------------------
add_executable(player_tests test_player.cpp)
add_executable(utils_tests test_random_device.cpp)
add_executable(actions_tests test_actions.cpp)
add_executable(board_tests
    test_board.cpp
    test_board_actions.cpp
    test_board_undo_actions.cpp
    test_generate_all_actions.cpp
)
add_executable(game_tests test_game.cpp)
add_executable(bank_tests test_bank.cpp)

# ----------------------------
# 🔗 Link libraries
# ----------------------------
foreach(test_target IN ITEMS player_tests actions_tests board_tests game_tests bank_tests utils_tests)
    target_link_libraries(${test_target} gtest_main gmock game_simulation)
endforeach()

# ----------------------------
# 🔍 Discover tests for CTest
# ----------------------------
include(GoogleTest)

foreach(test_target IN ITEMS player_tests actions_tests board_tests game_tests bank_tests utils_tests)
    gtest_discover_tests(${test_target}
        PROPERTIES
            LABELS "unit"
            TIMEOUT 30
    )
endforeach()
