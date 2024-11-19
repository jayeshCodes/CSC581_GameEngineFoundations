cmake_minimum_required(VERSION 3.29)
project(shade_engine)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        lib/core/draw.cpp
        lib/core/init.cpp
        lib/core/input.cpp
        lib/core/structs.cpp
        lib/core/timer.hpp
        lib/core/timer.cpp
        lib/generic/singleton.cpp
        lib/generic/singleton.hpp
        lib/game/GameManager.cpp
        lib/game/GameManager.hpp
        lib/helpers/random.hpp
        lib/helpers/network_constants.hpp
        lib/core/timeline.hpp
        lib/core/timeline.cpp
        lib/enum/enum.hpp
        lib/helpers/colors.hpp
        lib/helpers/constants.hpp
        lib/ECS/types.hpp
        lib/ECS/entity_manager.hpp
        lib/ECS/component_manager.hpp
        lib/ECS/component_array.hpp
        lib/ECS/system.hpp
        lib/ECS/system_manager.hpp
        lib/ECS/coordinator.hpp
        lib/systems/gravity.cpp
        lib/model/components.hpp
        lib/systems/render.cpp
        lib/systems/kinematic.cpp
        lib/systems/camera.cpp
        lib/systems/keyboard_movement.cpp
        lib/systems/camera.cpp
        lib/systems/move_between_2_point_system.hpp
        lib/systems/destroy.hpp
        lib/systems/collision.hpp
        lib/systems/jump.hpp
        lib/systems/death.hpp
        lib/systems/death.hpp
        lib/server/worker.hpp
        lib/systems/client.hpp
        lib/systems/receiver.hpp
        lib/strategy/send_strategy.hpp
        lib/strategy/strategy_selector.hpp
        lib/EMS/event_manager.hpp
        lib/EMS/event_coordinator.hpp
        lib/data_structures/ThreadSafePriorityQueue.hpp
        lib/systems/keyboard.hpp
        lib/systems/collision_handler.hpp
        lib/systems/vertical_boost_handler.hpp
        lib/systems/event_system.hpp
        lib/systems/respawn.hpp
        lib/helpers/network_helper.hpp
        lib/model/data_model.hpp
        lib/systems/entity_created_handler.hpp
        lib/systems/position_update_handler.hpp
        lib/systems/dash.hpp
        lib/systems/combo_event_handler.hpp
        lib/systems/replay_handler.hpp
        lib/systems/position.hpp
        lib/EMS/types.hpp
        lib/model/event.hpp)

set(BRICK_BREAKER_SOURCE games/brick_breaker/systems/keyboard_movement.hpp
        games/brick_breaker/handlers/keyboard_handler.hpp
        games/brick_breaker/model/component.hpp
        games/brick_breaker/model/events.hpp
        games/brick_breaker/handlers/movement_handler.hpp
        games/brick_breaker/handlers/launch_handler.hpp
        games/brick_breaker/systems/out_of_bound_detector.hpp
        games/brick_breaker/strategy/send_strategy.hpp
        games/brick_breaker/model/data_model.hpp
        games/brick_breaker/systems/receiver.hpp
        games/brick_breaker/handlers/collision_handler.hpp
        games/brick_breaker/handlers/out_of_bound_handler.hpp
        games/brick_breaker/systems/client.hpp
        games/brick_breaker/handlers/powerup_handler.hpp
        games/brick_breaker/systems/game_state_checker.hpp
        games/brick_breaker/handlers/game_state_handler.hpp
)

set (SNAKE_SOURCE
        games/snake/systems/keyboard_movement.hpp
        games/snake/model/event.hpp
        games/snake/systems/receiver.hpp
        games/snake/systems/client.hpp
        games/snake/handlers/keyboard_handler.hpp
        games/snake/model/components.hpp
        games/snake/systems/move.hpp
        games/snake/systems/move.hpp
        games/snake/helpers/quantizer.hpp
        games/snake/handlers/food_handler.hpp
        games/snake/handlers/collision_handler.hpp
        games/snake/systems/oob_detector.hpp
        games/snake/handlers/game_state_handler.hpp
        games/snake/helpers/text_helper.hpp
        games/snake/systems/score.hpp
)


add_executable(shade_engine ${SOURCES} main.cpp)
add_executable(shade_engine_server ${SOURCES} server.cpp)

add_executable(brick_breaker ${SOURCES} ${BRICK_BREAKER_SOURCE} games/brick_breaker/game1.cpp
        games/snake/systems/text_renderer.hpp)
add_executable(brick_breaker_server ${SOURCES} ${BRICK_BREAKER_SOURCE} games/brick_breaker/server.cpp)

add_executable(snake ${SOURCES} ${SNAKE_SOURCE} games/snake/snake.cpp)
add_executable(snake_server ${SOURCES} ${SNAKE_SOURCE} games/snake/server.cpp)

find_package(SDL2 REQUIRED)
find_package(cppzmq REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(shade_engine cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)
target_link_libraries(shade_engine_server cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)

target_link_libraries(brick_breaker cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)
target_link_libraries(brick_breaker_server cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)

target_link_libraries(snake cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json SDL2_ttf)
target_link_libraries(snake_server cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json SDL2_ttf)