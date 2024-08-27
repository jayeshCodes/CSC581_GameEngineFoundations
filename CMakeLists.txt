cmake_minimum_required(VERSION 3.29)
project(shade_engine)

set(CMAKE_CXX_STANDARD 14)


set(SOURCES
        lib/core/draw.cpp
        lib/core/init.cpp
        lib/core/input.cpp
        lib/core/structs.cpp
        main.cpp
)


add_executable(shade_engine ${SOURCES})

find_package(SDL2 REQUIRED)
target_link_libraries(shade_engine SDL2::SDL2)
