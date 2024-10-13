#pragma once
#include "lib/core/structs.hpp"
#include "lib/core/init.hpp"
#include "lib/core/draw.hpp"
#include "lib/core/input.hpp"
#include <memory>

#include "lib/ECS/coordinator.hpp"

// SDL render and window context
extern App *app;

std::atomic<bool> gameRunning{false};
Coordinator gCoordinator;
constexpr int SERVERPORT = 8000;

int main(int argc, char *argv[]);
