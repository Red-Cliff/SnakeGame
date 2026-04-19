cmake_minimum_required(VERSION 3.10)

project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

add_executable(SnakeGame
    main.cpp
    core/GameLoop.cpp
    core/StateMachine.cpp
    systems/InputSystem.cpp
    systems/RenderSystem.cpp
    systems/TimeSystem.cpp
    gameplay/Snake.cpp
    gameplay/Food.cpp
    persistence/SaveSystem.cpp
)

target_include_directories(SnakeGame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
