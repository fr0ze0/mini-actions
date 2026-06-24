cmake_minimum_required(VERSION 3.10)
project(mini_actions CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mini-actions
    src/main.cpp
    src/config.cpp
    src/runner.cpp
)
