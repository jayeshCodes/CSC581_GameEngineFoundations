//
// Created by Utsav Lal on 11/10/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ReplayHandler : public System {
    const int maxReplaySize = 60 * 30; // 30 seconds of replay assuming 60FPS
    std::unordered_map<Entity, std::queue<Transform> > replayTransforms;
    bool recording = false;
    bool replaying = false;

    EventHandler startReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::StartRecording) {
            std::cout << "Recording started" << std::endl;
            replayTransforms.clear();
            gCoordinator.backup();
            recording = true;
        }
    };

    EventHandler stopReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::StopRecording) {
            std::cout << "Recording stopped" << std::endl;
            recording = false;
        }
    };

    EventHandler replayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::StartReplaying) {
            std::cout << "Starting Replay" << std::endl;
            gCoordinator.restore();
            replaying = true;
        }
    };

public:
    ReplayHandler() {
        eventCoordinator.subscribe(startReplayHandler, EventType::StartRecording);
        eventCoordinator.subscribe(stopReplayHandler, EventType::StopRecording);
        eventCoordinator.subscribe(replayHandler, EventType::StartReplaying);
    }

    ~ReplayHandler() {
        eventCoordinator.unsubscribe(startReplayHandler, EventType::StartRecording);
        eventCoordinator.unsubscribe(stopReplayHandler, EventType::StopRecording);
        eventCoordinator.unsubscribe(replayHandler, EventType::StartReplaying);
    }

    void update() {
        if (!recording && !replaying) return;

        if (recording) {
            for (auto &[id, entity]: gCoordinator.getEntityIds()) {
                if (gCoordinator.hasComponent<Transform>(entity)) {
                    auto &transform = gCoordinator.getComponent<Transform>(entity);
                    if(replayTransforms[entity].size() >= maxReplaySize) {
                        std::cout << "We have reached limit, stopping recording" << std::endl;
                        recording = false;
                    }
                    replayTransforms[entity].push(transform);
                }
            }
        }

        if (replaying) {
            for (auto &[id, entity]: gCoordinator.getEntityIds()) {
                if (gCoordinator.hasComponent<Transform>(entity)) {
                    auto &transform = gCoordinator.getComponent<Transform>(entity);
                    if (replayTransforms[entity].empty()) {
                        replaying = false;
                        std::cout << "Replay finished" << std::endl;
                        Event stopReplayEvent{EventType::StopReplaying, {}};
                        eventCoordinator.emit(std::make_shared<Event>(stopReplayEvent));
                        return;
                    }
                    transform = replayTransforms[entity].front();
                    replayTransforms[entity].pop();
                }
            }
        }
    }
};
