cmake_minimum_required(VERSION 3.29)
project(shade_engine)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES
        lib/core/draw.cpp
        lib/core/init.cpp
        lib/core/input.cpp
        lib/core/structs.cpp
        lib/objects/shapes/rectangle.cpp
        lib/objects/shapes/rectangle.hpp
        lib/objects/object.cpp
        lib/objects/object.hpp
        lib/objects/factory.cpp
        lib/objects/factory.hpp
        lib/core/timer.hpp
        lib/core/timer.cpp
        lib/core/kinematic.hpp
        lib/core/physics/physics.hpp
        lib/core/physics/physics.cpp
        lib/core/physics/gravity.hpp
        lib/core/physics/gravity.cpp
        lib/core/physics/keyMovement.cpp
        lib/core/physics/keyMovement.hpp
        lib/core/physics/collision.hpp
        lib/core/physics/collision.cpp
        lib/generic/singleton.cpp
        lib/generic/singleton.hpp
        lib/game/GameManager.cpp
        lib/game/GameManager.hpp
        lib/animation/controller/moveBetween2Points.cpp
        lib/animation/controller/moveBetween2Points.hpp
        lib/helpers/random.hpp
        lib/helpers/network_constants.hpp
        lib/core/timeline.hpp
        lib/core/timeline.cpp
        lib/generic/safe_queue.hpp
        lib/enum/message_type.hpp
        lib/helpers/colors.hpp
        lib/helpers/constants.hpp
        lib/network/p2p/host.cpp
        lib/network/p2p/host.hpp
        lib/network/client_server/client.cpp
        lib/network/client_server/client.hpp
        lib/network/client_server/server.cpp
        lib/network/client_server/server.hpp
)

add_executable(shade_engine ${SOURCES} main.cpp)
add_executable(zmq_server ${SOURCES} lib/client_server/server.cpp)
add_executable(zmq_client ${SOURCES} lib/client_server/client_1.cpp)
add_executable(zmq_listener_1 ${SOURCES} lib/client_server/client_2.cpp)
add_executable(zmq_listener_2 ${SOURCES} lib/client_server/client_3.cpp)
add_executable(p2p_client_1 ${SOURCES} lib/p2p/client1.cpp)
add_executable(p2p_client_2 ${SOURCES} lib/p2p/client2.cpp)
add_executable(p2p_client_3 ${SOURCES} lib/p2p/client3.cpp)


find_package(SDL2 REQUIRED)
find_package(cppzmq REQUIRED)

target_link_libraries(shade_engine cppzmq SDL2::SDL2)
target_link_libraries(zmq_server cppzmq SDL2::SDL2)
target_link_libraries(zmq_client cppzmq SDL2::SDL2)
target_link_libraries(zmq_listener_1 cppzmq SDL2::SDL2)
target_link_libraries(zmq_listener_2 cppzmq SDL2::SDL2)
target_link_libraries(p2p_client_1 cppzmq SDL2::SDL2)
target_link_libraries(p2p_client_2 cppzmq SDL2::SDL2)
target_link_libraries(p2p_client_3 cppzmq SDL2::SDL2)



