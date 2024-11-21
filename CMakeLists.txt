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
        lib/systems/event_system.hpp
        lib/systems/respawn.hpp
        lib/helpers/network_helper.hpp
        lib/model/data_model.hpp
        lib/systems/entity_created_handler.hpp
        lib/systems/position_update_handler.hpp
        lib/systems/dash.hpp
        lib/systems/replay_handler.hpp
        lib/systems/position.hpp
        lib/helpers/font_helper.cpp)

add_executable(shade_engine ${SOURCES} main.cpp
        lib/systems/bubble_event_handler.hpp
        lib/systems/shooter_reset_handler.hpp
        lib/systems/shooter_disable_handler.hpp
        lib/systems/shooter_verification.hpp
        lib/systems/sound.hpp
        lib/systems/sound_event_handler.hpp
        lib/systems/pop_animation.hpp
        lib/systems/grid_generator.hpp
        lib/systems/bubble_grid.hpp
        lib/systems/bubble_match_handler.hpp
        lib/systems/pause_handler.hpp
        lib/systems/instruction_render.hpp
        lib/systems/bubble_shooter.hpp
        lib/systems/game_over_handler.hpp
        lib/systems/reset_handler.hpp)
add_executable(shade_engine_server ${SOURCES} server.cpp
        lib/helpers/font_helper.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(cppzmq REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Combine all libraries for shade_engine in a single command
target_link_libraries(shade_engine PRIVATE
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        SDL2::SDL2
        cppzmq
        nlohmann_json::nlohmann_json
)

# Link libraries for shade_engine_server
target_link_libraries(shade_engine_server PRIVATE
        SDL2::SDL2
        cppzmq
        nlohmann_json::nlohmann_json
)

