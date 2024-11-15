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

using EntityTime = std::tuple<nlohmann::json, int64_t>;

class ReplayHandler : public System {
    const int maxReplaySize = 60 * 30; // 30 seconds of replay assuming 60FPS
    std::unordered_map<Entity, std::queue<Transform> > replayTransforms;
    std::queue<EntityTime> creationOrder;
    std::queue<EntityTime> deletionOrder;
    bool recording = false;
    bool replaying = false;
    Timeline replayTimeline = Timeline(&gameTimeline, 1);

    StateSerializer serializer = [](nlohmann::json &snapshot, Entity &entity) {
        if (gCoordinator.hasComponent<Transform>(entity)) {
            snapshot["c"]["type"] = "Transform";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Transform>(entity));
        }
        if (gCoordinator.hasComponent<Color>(entity)) {
            snapshot["c"]["type"] = "Color";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Color>(entity));
        }
        if (gCoordinator.hasComponent<RigidBody>(entity)) {
            snapshot["c"]["type"] = "RigidBody";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<RigidBody>(entity));
        }
        if (gCoordinator.hasComponent<Gravity>(entity)) {
            snapshot["c"]["type"] = "Gravity";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Gravity>(entity));
        }
        if (gCoordinator.hasComponent<CKinematic>(entity)) {
            snapshot["c"]["type"] = "CKinematic";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<CKinematic>(entity));
        }
        if (gCoordinator.hasComponent<Jump>(entity)) {
            snapshot["c"]["type"] = "Jump";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Jump>(entity));
        }
        if (gCoordinator.hasComponent<Respawnable>(entity)) {
            snapshot["c"]["type"] = "Respawnable";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Respawnable>(entity));
        }
        if (gCoordinator.hasComponent<Camera>(entity)) {
            snapshot["c"]["type"] = "Camera";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Camera>(entity));
        }
        if (gCoordinator.hasComponent<VerticalBoost>(entity)) {
            snapshot["c"]["type"] = "VerticalBoost";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<VerticalBoost>(entity));
        }
        if (gCoordinator.hasComponent<Stomp>(entity)) {
            snapshot["c"]["type"] = "Stomp";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Stomp>(entity));
        }
        if (gCoordinator.hasComponent<Dash>(entity)) {
            snapshot["c"]["type"] = "Dash";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Dash>(entity));
        }
        if (gCoordinator.hasComponent<Destroy>(entity)) {
            snapshot["c"]["type"] = "Destroy";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Destroy>(entity));
        }
        if (gCoordinator.hasComponent<MovingPlatform>(entity)) {
            snapshot["c"]["type"] = "MovingPlatform";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<MovingPlatform>(entity));
        }
        if (gCoordinator.hasComponent<Collision>(entity)) {
            snapshot["c"]["type"] = "Collision";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Collision>(entity));
        }
        if (gCoordinator.hasComponent<Receiver>(entity)) {
            snapshot["c"]["type"] = "Receiver";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Receiver>(entity));
        }
        if (gCoordinator.hasComponent<ClientEntity>(entity)) {
            snapshot["c"]["type"] = "ClientEntity";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<ClientEntity>(entity));
        }
        if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
            snapshot["c"]["type"] = "KeyboardMovement";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<KeyboardMovement>(entity));
        }
    };

    StateDeserializer deserializer = [](nlohmann::json &entitySnap, Entity &entity) {
        for (auto &component: entitySnap["c"]) {
            if (component["type"] == "Transform") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                transform = component["v"];
            }
            if (component["type"] == "Color") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &color = gCoordinator.getComponent<Color>(entity);
                color = component["v"];
            }
            if (component["type"] == "RigidBody") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &rigidBody = gCoordinator.getComponent<RigidBody>(entity);
                rigidBody = component["v"];
            }
            if (component["type"] == "Gravity") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &gravity = gCoordinator.getComponent<Gravity>(entity);
                gravity = component["v"];
            }
            if (component["type"] == "CKinematic") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &cKinematic = gCoordinator.getComponent<CKinematic>(entity);
                cKinematic = component["v"];
            }
            if (component["type"] == "Jump") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &jump = gCoordinator.getComponent<Jump>(entity);
                jump = component["v"];
            }
            if (component["type"] == "Respawnable") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &respawnable = gCoordinator.getComponent<Respawnable>(entity);
                respawnable = component["v"];
            }
            if (component["type"] == "Camera") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &camera = gCoordinator.getComponent<Camera>(entity);
                camera = component["v"];
            }
            if (component["type"] == "VerticalBoost") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &verticalBoost = gCoordinator.getComponent<VerticalBoost>(entity);
                verticalBoost = component["v"];
            }
            if (component["type"] == "Stomp") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &stomp = gCoordinator.getComponent<Stomp>(entity);
                stomp = component["v"];
            }
            if (component["type"] == "Dash") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &dash = gCoordinator.getComponent<Dash>(entity);
                dash = component["v"];
            }
            if (component["type"] == "") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &destroy = gCoordinator.getComponent<Destroy>(entity);
                destroy = component["v"];
            }
            if (component["type"] == "MovingPlatform") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &movingPlatform = gCoordinator.getComponent<MovingPlatform>(entity);
                movingPlatform = component["v"];
            }
            if (component["type"] == "Collision") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &collision = gCoordinator.getComponent<Collision>(entity);
                collision = component["v"];
            }
            if (component["type"] == "ClientEntity") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
                clientEntity = component["v"];
            }
            if (component["type"] == "KeyboardMovement") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &keyboardMovement = gCoordinator.getComponent<KeyboardMovement>(entity);
                keyboardMovement = component["v"];
            }
            if (component["type"] == "Receiver") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &receiver = gCoordinator.getComponent<Receiver>(entity);
                receiver = component["v"];
            }
        }
    };

    EventHandler startReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::StartRecording)) {
            std::cout << "Recording started" << std::endl;
            replayTransforms.clear();
            creationOrder = std::queue<EntityTime>();
            deletionOrder = std::queue<EntityTime>();
            gCoordinator.backup(serializer);
            replayTimeline.reset();
            recording = true;
        }
    };

    EventHandler stopReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::StopRecording)) {
            std::cout << "Recording stopped" << std::endl;
            recording = false;
        }
    };

    EventHandler replayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::StartReplaying)) {
            std::cout << "Starting Replay" << std::endl;
            gCoordinator.restore();
            replayTimeline.reset();
            replaying = true;
        }
    };

    EventHandler entityCreatedHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityCreated)) {
            if (!recording) return;
            const EntityCreatedData &data = event->data;
            creationOrder.emplace(gCoordinator.createSnapshot(data.entity, data.id, serializer),
                                  replayTimeline.getElapsedTime());
        }
    };

    EventHandler entityDeletedHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityDestroyed)) {
            if (!recording) return;
            const EntityDestroyedData &data = event->data;
            deletionOrder.emplace(gCoordinator.createSnapshot(data.entity, data.id, serializer),
                                  replayTimeline.getElapsedTime());
        }
    };

    void deleteEntities(int64_t currentTime) {
        if (deletionOrder.empty()) return;
        auto deleteItem = deletionOrder.front();
        while (!deletionOrder.empty() && std::get<int64_t>(deleteItem) <= currentTime) {
            auto [entitySnap, time] = deleteItem;
            gCoordinator.destroyEntity(entitySnap["entity"]);
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
        eventCoordinator.subscribe(startReplayHandler, eventTypeToString(EventType::StartRecording));
        eventCoordinator.subscribe(stopReplayHandler, eventTypeToString(EventType::StopRecording));
        eventCoordinator.subscribe(replayHandler, eventTypeToString(EventType::StartReplaying));
        eventCoordinator.subscribe(entityCreatedHandler, eventTypeToString(EventType::EntityCreated));
        eventCoordinator.subscribe(entityDeletedHandler, eventTypeToString(EventType::EntityDestroyed));
    }

    ~ReplayHandler() {
        eventCoordinator.unsubscribe(startReplayHandler, eventTypeToString(EventType::StartRecording));
        eventCoordinator.unsubscribe(stopReplayHandler, eventTypeToString(EventType::StopRecording));
        eventCoordinator.unsubscribe(replayHandler, eventTypeToString(EventType::StartReplaying));
        eventCoordinator.unsubscribe(entityCreatedHandler, eventTypeToString(EventType::EntityCreated));
        eventCoordinator.unsubscribe(entityDeletedHandler, eventTypeToString(EventType::EntityDestroyed));
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
                        Event stopReplayEvent{eventTypeToString(EventType::StopReplaying), {}};
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
