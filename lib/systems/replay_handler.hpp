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
    std::unordered_map<std::string, std::queue<Transform> > replayTransforms;
    std::queue<EntityTime> creationOrder;
    std::queue<EntityTime> deletionOrder;
    bool recording = false;
    bool replaying = false;
    Timeline replayTimeline = Timeline(&gameTimeline, 1);

    StateSerializer serializer = [](nlohmann::json &snapshot, Entity &entity) {
        snapshot["c"] = nlohmann::json::array();
        if (gCoordinator.hasComponent<Transform>(entity)) {
            nlohmann::json j;
            j["type"] = "Transform";
            j["v"] = gCoordinator.getComponent<Transform>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Color>(entity)) {
            nlohmann::json j;
            j["type"] = "Color";
            j["v"] = gCoordinator.getComponent<Color>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<RigidBody>(entity)) {
            nlohmann::json j;
            j["type"] = "RigidBody";
            j["v"] = gCoordinator.getComponent<RigidBody>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Gravity>(entity)) {
            nlohmann::json j;
            j["type"] = "Gravity";
            j["v"] = gCoordinator.getComponent<Gravity>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<CKinematic>(entity)) {
            nlohmann::json j;
            j["type"] = "CKinematic";
            j["v"] = gCoordinator.getComponent<CKinematic>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Jump>(entity)) {
            nlohmann::json j;
            j["type"] = "Jump";
            j["v"] = gCoordinator.getComponent<Jump>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Respawnable>(entity)) {
            nlohmann::json j;
            j["type"] = "Respawnable";
            j["v"] = gCoordinator.getComponent<Respawnable>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Camera>(entity)) {
            nlohmann::json j;
            j["type"] = "Camera";
            j["v"] = gCoordinator.getComponent<Camera>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<VerticalBoost>(entity)) {
            nlohmann::json j;
            j["type"] = "VerticalBoost";
            j["v"] = gCoordinator.getComponent<VerticalBoost>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Stomp>(entity)) {
            nlohmann::json j;
            j["type"] = "Stomp";
            j["v"] = gCoordinator.getComponent<Stomp>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Dash>(entity)) {
            nlohmann::json j;
            j["type"] = "Dash";
            j["v"] = gCoordinator.getComponent<Dash>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Destroy>(entity)) {
            nlohmann::json j;
            j["type"] = "Destroy";
            j["v"] = gCoordinator.getComponent<Destroy>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<MovingPlatform>(entity)) {
            nlohmann::json j;
            j["type"] = "MovingPlatform";
            j["v"] = gCoordinator.getComponent<MovingPlatform>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Collision>(entity)) {
            nlohmann::json j;
            j["type"] = "Collision";
            j["v"] = gCoordinator.getComponent<Collision>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Receiver>(entity)) {
            nlohmann::json j;
            j["type"] = "Receiver";
            j["v"] = gCoordinator.getComponent<Receiver>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<ClientEntity>(entity)) {
            nlohmann::json j;
            j["type"] = "ClientEntity";
            j["v"] = gCoordinator.getComponent<ClientEntity>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
            nlohmann::json j;
            j["type"] = "KeyboardMovement";
            j["v"] = gCoordinator.getComponent<KeyboardMovement>(entity);
            snapshot["c"].emplace_back(j);
        }
    };

    StateDeserializer deserializer = [](nlohmann::json &entitySnap, Entity &entity) {
        for (auto &component: entitySnap["c"]) {
            if (component["type"] == "Transform") {
                Transform t = component["v"];
                gCoordinator.addComponent(entity, t);
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                transform = t;
            }
            if (component["type"] == "Color") {
                Color c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &color = gCoordinator.getComponent<Color>(entity);
                color = c;
            }
            if (component["type"] == "RigidBody") {
                RigidBody r = component["v"];
                gCoordinator.addComponent(entity, r);
                auto &rigidBody = gCoordinator.getComponent<RigidBody>(entity);
                rigidBody = r;
            }
            if (component["type"] == "Gravity") {
                Gravity g = component["v"];
                gCoordinator.addComponent(entity, g);
                auto &gravity = gCoordinator.getComponent<Gravity>(entity);
                gravity = g;
            }
            if (component["type"] == "CKinematic") {
                CKinematic k = component["v"];
                gCoordinator.addComponent(entity, k);
                auto &cKinematic = gCoordinator.getComponent<CKinematic>(entity);
                cKinematic = k;
            }
            if (component["type"] == "Jump") {
                Jump j = component["v"];
                gCoordinator.addComponent(entity, j);
                auto &jump = gCoordinator.getComponent<Jump>(entity);
                jump = j;
            }
            if (component["type"] == "Respawnable") {
                Respawnable r = component["v"];
                gCoordinator.addComponent(entity, r);
                auto &respawnable = gCoordinator.getComponent<Respawnable>(entity);
                respawnable = r;
            }
            if (component["type"] == "Camera") {
                Camera c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &camera = gCoordinator.getComponent<Camera>(entity);
                camera = c;
            }
            if (component["type"] == "VerticalBoost") {
                VerticalBoost v = component["v"];
                gCoordinator.addComponent(entity, v);
                auto &verticalBoost = gCoordinator.getComponent<VerticalBoost>(entity);
                verticalBoost = v;
            }
            if (component["type"] == "Stomp") {
                Stomp s = component["v"];
                gCoordinator.addComponent(entity, s);
                auto &stomp = gCoordinator.getComponent<Stomp>(entity);
                stomp = s;
            }
            if (component["type"] == "Dash") {
                Dash d = component["v"];
                gCoordinator.addComponent(entity, d);
                auto &dash = gCoordinator.getComponent<Dash>(entity);
                dash = d;
            }
            if (component["type"] == "Destroy") {
                Destroy d = component["v"];
                gCoordinator.addComponent(entity, d);
                auto &destroy = gCoordinator.getComponent<Destroy>(entity);
                destroy = d;
            }
            if (component["type"] == "MovingPlatform") {
                MovingPlatform m = component["v"];
                gCoordinator.addComponent(entity, m);
                auto &movingPlatform = gCoordinator.getComponent<MovingPlatform>(entity);
                movingPlatform = m;
            }
            if (component["type"] == "Collision") {
                Collision c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &collision = gCoordinator.getComponent<Collision>(entity);
                collision = c;
            }
            if (component["type"] == "ClientEntity") {
                ClientEntity ce = component["v"];
                gCoordinator.addComponent(entity, ce);
                auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
                clientEntity = ce;
            }
            if (component["type"] == "KeyboardMovement") {
                KeyboardMovement k = component["v"];
                gCoordinator.addComponent(entity, k);
                auto &keyboardMovement = gCoordinator.getComponent<KeyboardMovement>(entity);
                keyboardMovement = k;
            }
            if (component["type"] == "Receiver") {
                Receiver r = component["v"];
                gCoordinator.addComponent(entity, r);
                auto &receiver = gCoordinator.getComponent<Receiver>(entity);
                receiver = r;
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
            gCoordinator.restore(deserializer);
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
            gCoordinator.destroyEntity(to_string(entitySnap["eId"]));
            deletionOrder.pop();
            deleteItem = deletionOrder.front();
        }
    }

    void createEntities(int64_t currentTime) {
        if (creationOrder.empty()) return;
        auto createItem = creationOrder.front();
        while (!creationOrder.empty() && std::get<int64_t>(createItem) <= currentTime) {
            auto [entitySnap, time] = createItem;
            gCoordinator.restoreEntity(entitySnap, deserializer);
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
                    if (replayTransforms[id].size() >= maxReplaySize) {
                        std::cout << "We have reached limit, stopping recording" << std::endl;
                        recording = false;
                    }
                    replayTransforms[id].push(transform);
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
                    if (replayTransforms[id].empty()) {
                        replaying = false;
                        std::cout << "Replay finished" << std::endl;
                        Event stopReplayEvent{eventTypeToString(EventType::StopReplaying), {}};
                        eventCoordinator.emit(std::make_shared<Event>(stopReplayEvent));
                        return;
                    }
                    transform = replayTransforms[id].front();
                    replayTransforms[id].pop();
                }
            }
        }
    }
};
