#pragma once
#include "../core/structs.hpp"
#include "../core/init.hpp"
#include "../core/draw.hpp"
#include "../core/input.hpp"
#include <memory>

#include "../ECS/coordinator.hpp"

// SDL render and window context
extern App *app;

std::atomic<bool> gameRunning{false};
Coordinator gCoordinator;
constexpr int SERVERPORT = 8000;

int main(int argc, char *argv[]);
