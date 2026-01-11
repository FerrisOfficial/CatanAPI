cmake_minimum_required(VERSION 3.10)
project(runs)

add_executable(run 
    run.cpp
    ${CMAKE_SOURCE_DIR}/players/player.cpp
    ${CMAKE_SOURCE_DIR}/players/randomPlayer.cpp
    ${CMAKE_SOURCE_DIR}/players/it1Player.cpp
    ${CMAKE_SOURCE_DIR}/players/it2Player.cpp
    ${CMAKE_SOURCE_DIR}/players/it3Player.cpp
    ${CMAKE_SOURCE_DIR}/players/it4Player.cpp
    ${CMAKE_SOURCE_DIR}/players/it5Player.cpp
    ${CMAKE_SOURCE_DIR}/players/it6Player.cpp
)

target_include_directories(run PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/game_simulation
    ${CMAKE_SOURCE_DIR}/players
    ${CMAKE_SOURCE_DIR}/utils
)

target_link_libraries(run PRIVATE game_simulation)

# Configure-time player flags (set via: cmake -S . -B build -DRUN_P0=rp -DRUN_P1=rp)
set(RUN_P0 "rp" CACHE STRING "Player0 flag for runs/run (e.g., rp)")
set(RUN_P1 "rp" CACHE STRING "Player1 flag for runs/run (e.g., rp)")

# Convenience target: builds and runs the simulation with configured flags.
# Working directory is the repo root so logs land in ./logs rather than build/logs.
add_custom_target(play
    COMMAND $<TARGET_FILE:run> ${RUN_P0} ${RUN_P1}
    DEPENDS run
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
