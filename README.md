# Moodle Jump
This game is built using Shade Engine. The game is a simple platformer where you have to jump from one platform to another avoiding the monsters.

# How to Play

1. Use `A` to move left and `D` to move right
2. Use `Space` to shoot Moodle knives
3. Do not touch the monsters

- If you fall down you will die and will have the option to restart the game.
- If you touch the monsters you will die and will have the option to restart the game.


# Shade Engine

Built by: Utsav Lal and Jayesh Gajbhar \
Library: SDL2, ZeroMQ, nlohmann/json and C++ 20

# Introduction

The engine was started from the template shared by Dr. Alexander Card in moodle
and grew from the same template. Current capabilities include:

1. A fully functional Entity Component System
2. Keyboard movement using WASD
3. Physics with customizable gravity in x and y directions
4. Collisions between objects with conservation of kinetic energy
5. A simple state machine animation controller
6. Fully multithreaded engine
7. Custom timeline class with ability to pause, play and increase decrease speed.
8. Asynchronous networking capabilities using ZMQ for client server and peer to peer between 3 clients.
9. A fully functional Event System where you can raise an event immediately or later or send it to the server.

# Instructions to build the code

## Since we used CLion CMake is required to build the project

PS: Remember to run the server first and then the clients

1. If there is an existing `build` folder run `rm -r build` to clear and remove the folder
2. Run `mkdir build && cd build` from the root to create a new build folder and change directory to it
3. Run `cmake ..` to generate required make files in build folder
4. Run `make all` to compile the binaries (this includes the game engine and game binaries)
5. Run `./shade_engine_server` to start the server for the game engine
6. Run `./shade_engine` to start the game engine or the game in this version

**Please ensure that both the server and the game are started with the same messaging system.**

PS: Another way to build the project would be to simply open it in CLion IDE and setting the env variables from the
build menu.

# Building the games

No further action is required for building the games. The engine and game are run from the same executable for this
version.

# Clean Up

To clean up

1. Run `cd ..` to come back to parent directory
2. Run `rm -r build` to remove build folder

# Project Structure

- `main.cpp`: This is the main entry point to the code
- `CMakeLists.cpp`: This is the build file which helps run Cmake and build the project
- `game/`: contains the game code (if any) for the project
- `lib/`: This contains all the source code for the project.
    - `core/`: The core elements of the game engine like setting up screen, etc
    - `data_structures/`: The custom data structures for the engine
    - `enum/`: Contains all the enumerations for the game engine
    - `ECS/`: Contains the elements of the Entity Component System
    - `EMS/`: Contains the elements of the Event Management System
    - `enum/`: Contains all the enums used in the project
    - `game/`: This contains game specific code. One such is a `GameManager` which is a singleton containing variables
      shared between the engine
    - `generic/`: This contains generic classes which might be used by us in the future
    - `helpers/`: This contains helper functions like random, constants, etc
    - `models/`: This contains the models for the game engine like the `Component` and `Data Model`
    - `server`: This contains a file called `worker.cpp` which is the main worker for the server.
    - `strategy/`: All the classes for strategy pattern (in our case we use it for selecting messaging by JSON or array
      of vectors)
    - `systems/`: All the systems for the game engine like `PhysicsSystem`, `CollisionSystem`, etc. Systems that end
      with `handler` are event handlers.


# Author
[Jayesh Bhagyesh Gajbhar](https://www.jayeshgajbhar.com)