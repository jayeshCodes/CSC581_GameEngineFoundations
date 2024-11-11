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
extern Timeline gameTimeline;

using EntityTime = std::tuple<EntitySnapshot, int64_t>;

class ReplayHandler : public System {
    const int maxReplaySize = 60 * 30; // 30 seconds of replay assuming 60FPS
    std::unordered_map<Entity, std::queue<Transform> > replayTransforms;
    std::queue<EntityTime> creationOrder;
    std::queue<EntityTime> deletionOrder;
    bool recording = false;
    bool replaying = false;
    Timeline replayTimeline = Timeline(&gameTimeline, 1);

    EventHandler startReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::StartRecording) {
            std::cout << "Recording started" << std::endl;
            replayTransforms.clear();
            creationOrder = std::queue<EntityTime>();
            deletionOrder = std::queue<EntityTime>();
            gCoordinator.backup();
            replayTimeline.reset();
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
            replayTimeline.reset();
            replaying = true;
        }
    };

    EventHandler entityCreatedHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::EntityCreated) {
            if (!recording) return;
            auto [entity, id] = std::get<EntityCreatedData>(event->data);
            creationOrder.emplace(gCoordinator.createSnapshot(entity, id), replayTimeline.getElapsedTime());
        }
    };

    EventHandler entityDeletedHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::EntityDestroyed) {
            if (!recording) return;
            auto [entity, id] = std::get<EntityDestroyedData>(event->data);
            deletionOrder.emplace(gCoordinator.createSnapshot(entity, id), replayTimeline.getElapsedTime());
        }
    };

    void deleteEntities(int64_t currentTime) {
        if (deletionOrder.empty()) return;
        auto deleteItem = deletionOrder.front();
        while (!deletionOrder.empty() && std::get<int64_t>(deleteItem) <= currentTime) {
            auto [entitySnap, time] = deleteItem;
            gCoordinator.destroyEntity(entitySnap.entity);
            deletionOrder.pop();
            deleteItem = deletionOrder.front();
        }
    }

    void createEntities(int64_t currentTime) {
        if (creationOrder.empty()) return;
        auto createItem = creationOrder.front();
        while (!creationOrder.empty() && std::get<int64_t>(createItem) <= currentTime) {
            auto [entitySnap, time] = createItem;
            gCoordinator.restoreEntity(entitySnap);
            creationOrder.pop();
            createItem = creationOrder.front();
        }
    }

public:
    ReplayHandler() {
        eventCoordinator.subscribe(startReplayHandler, EventType::StartRecording);
        eventCoordinator.subscribe(stopReplayHandler, EventType::StopRecording);
        eventCoordinator.subscribe(replayHandler, EventType::StartReplaying);
        eventCoordinator.subscribe(entityCreatedHandler, EventType::EntityCreated);
        eventCoordinator.subscribe(entityDeletedHandler, EventType::EntityDestroyed);
    }

    ~ReplayHandler() {
        eventCoordinator.unsubscribe(startReplayHandler, EventType::StartRecording);
        eventCoordinator.unsubscribe(stopReplayHandler, EventType::StopRecording);
        eventCoordinator.unsubscribe(replayHandler, EventType::StartReplaying);
        eventCoordinator.unsubscribe(entityCreatedHandler, EventType::EntityCreated);
        eventCoordinator.unsubscribe(entityDeletedHandler, EventType::EntityDestroyed);
    }

    void update() {
        if (!recording && !replaying) return;

        if (recording) {
            for (auto &[id, entity]: gCoordinator.getEntityIds()) {
                if (gCoordinator.hasComponent<Transform>(entity)) {
                    auto &transform = gCoordinator.getComponent<Transform>(entity);
                    if (replayTransforms[entity].size() >= maxReplaySize) {
                        std::cout << "We have reached limit, stopping recording" << std::endl;
                        recording = false;
                    }
                    replayTransforms[entity].push(transform);
                }
            }
        }

        if (replaying) {
            int64_t currentTime = replayTimeline.getElapsedTime();
            deleteEntities(currentTime);
            createEntities(currentTime);
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
