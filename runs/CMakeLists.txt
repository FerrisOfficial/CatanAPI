cmake_minimum_required(VERSION 3.10)
project(run_random)

add_executable(run_random run_random.cpp)

target_include_directories(run_random PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/game_simulation
    ${CMAKE_SOURCE_DIR}/players
    ${CMAKE_SOURCE_DIR}/utils
)

target_link_libraries(run_random PRIVATE game_simulation)
