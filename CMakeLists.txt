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
        lib/helpers/ecs_helpers.hpp
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
        lib/EMS/event.hpp
        lib/EMS/event_manager.hpp
        lib/EMS/event_coordinator.hpp
        lib/data_structures/ThreadSafePriorityQueue.hpp
        lib/systems/keyboard.hpp
        lib/systems/event_system.hpp
)

add_executable(shade_engine ${SOURCES} main.cpp
        lib/systems/respawn.hpp
        lib/systems/keyboard.hpp)
add_executable(shade_engine_server ${SOURCES} server.cpp
        lib/systems/keyboard.hpp)

find_package(SDL2 REQUIRED)
find_package(cppzmq REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

target_link_libraries(shade_engine cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)
target_link_libraries(shade_engine_server cppzmq SDL2::SDL2 nlohmann_json::nlohmann_json)
