#pragma once
#include "lib/core/structs.hpp"
#include "lib/core/init.hpp"
#include "lib/core/draw.hpp"
#include "lib/core/input.hpp"
#include <memory>

#include "lib/ECS/coordinator.hpp"
#include "lib/EMS/event_coordinator.hpp"

// SDL render and window context
extern App *app;

inline Timeline anchorTimeline(nullptr, 1000);
inline Timeline gameTimeline(&anchorTimeline, 1);
inline Timeline monsterTimeline(&anchorTimeline, 0.5);
inline Timeline eventTimeline(&anchorTimeline, 1);
inline std::string mainCharID;

std::atomic<bool> gameRunning{false};
EventCoordinator eventCoordinator;
Coordinator gCoordinator;
constexpr int SERVERPORT = 8000;

int main(int argc, char *argv[]);
