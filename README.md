# Shade Engine

Built by: Utsav Lal and Jayesh Gajbhar \
Library: SDL2 and C++ 20

# Introduction

The engine was started from the template shared by Dr. Alexander Card in moodle
and grew from the same template. Current capabilities include:

1. A rectangle generator
2. Keyboard movement using WASD
3. Constant pixel scaling mode and Window Size Scaling mode
4. Physics with customizable gravity in x and y directions
5. Collisions between objects with conservation of kinetic energy
6. A simple state machine animation controller
7. Fully multithreaded engine
8. Custom timeline class with ability to pause, play and increase decrease speed.
9. Asynchronous networking capabilities using ZMQ for client server and peer to peer between 3 clients.

# Instructions to build the code

## Since we used CLion CMake is required to build the project

1. If there is an existing `build` folder run `rm -r build` to clear and remove the folder
2. Run `mkdir build && cd build` from the root to create a new build folder and change directory to it
3. Run `cmake ..` to generate required make files in build folder
4. Run `make all` to compile the binaries
5. Run `./shade_engine` to start the game engine

PS: Another way to build the project would be to simply open it in CLion IDE.

# Clean Up

To clean up

1. Run `cd ..` to come back to parent directory
2. Run `rm -r build` to remove build folder

# Project Structure

- `main.cpp`: This is the main entry point to the code
- `CMakeLists.cpp`: This is the build file which helps run Cmake and build the project
- `lib/`: This contains all the source code for the project.
    - `animation/`: Contains the code for pattern movement
    - `core/`: The core elements of the game engine like setting up screen, physics, drawing, etc
    - `enum/`: Contains all the enums used in the project
    - `game/`: This contains game specific code. One such is a `GameManager` which is a singleton containing variables
      shared between the engine
    - `generic/`: This contains generic classes which might be used by us in the future
    - `helpers/`: This contains helper functions like random, constants, etc
    - `network/`: This contains the networking code
    - `objects/`: This contains the code to generate in game objects like a rectangle in our case
