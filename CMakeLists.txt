cmake_minimum_required(VERSION 3.29)
project(shade_engine)

set(CMAKE_CXX_STANDARD 14)


set(SOURCES
        lib/core/draw.cpp
        lib/core/init.cpp
        lib/core/input.cpp
        lib/core/structs.cpp
        main.cpp
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
)


add_executable(shade_engine ${SOURCES}
        lib/core/physics/collision.hpp
        lib/core/physics/collision.cpp)

find_package(SDL2 REQUIRED)
target_link_libraries(shade_engine SDL2::SDL2)
