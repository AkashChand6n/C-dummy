cmake_minimum_required(VERSION 3.10)
project(CasinoNumberGuessingGame)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(casino_game src/main.cpp src/game.cpp src/player.cpp)

add_executable(test_game tests/test_game.cpp src/game.cpp src/player.cpp)
