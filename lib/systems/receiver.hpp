//
// Created by Utsav Lal on 10/15/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/network_helper.hpp"
#include "../model/components.hpp"
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;

class ReceiverSystem : public System {
    bool testStarted = false;

public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t copy;
            std::string entity_id;
            NetworkHelper::receiveMessageClient( socket, copy, entity_id);

            try {
                auto [command, received_transform, received_color, rigidbody, collision] = send_strategy->
                        parse_message(copy);
                if (command == Message::SYNC) {
                    std::cout << "Syncing" << std::endl;
                    auto ids = gCoordinator.getEntityIds();
                    for (const auto &id: ids) {
                        if (gCoordinator.hasComponent<ClientEntity>(id.second)) {
                            gCoordinator.getComponent<ClientEntity>(id.second).noOfTimes = 5;
                        }
                    }
                }
                if (command == Message::UPDATE) {
                    const std::string eId = entity_id;
                    auto generatedId = gCoordinator.createEntity(eId);

                    gCoordinator.addComponent<Transform>(generatedId, Transform{});
                    gCoordinator.addComponent<Color>(generatedId, Color{});
                    gCoordinator.addComponent<RigidBody>(generatedId, RigidBody{-1});
                    gCoordinator.addComponent<Collision>(generatedId, Collision{false, false});
                    gCoordinator.addComponent<CKinematic>(generatedId, CKinematic{});

                    auto &transform = gCoordinator.getComponent<Transform>(generatedId);
                    auto &color = gCoordinator.getComponent<Color>(generatedId);
                    auto &rigidBody = gCoordinator.getComponent<RigidBody>(generatedId);
                    auto &collisionComponent = gCoordinator.getComponent<Collision>(generatedId);

                    transform = received_transform;
                    color.color = {
                        received_color.color.r, received_color.color.g, received_color.color.b,
                        received_color.color.a
                    };
                    rigidBody = rigidbody;
                    collisionComponent = collision;
                }
                if (command == Message::DELETE) {
                    std::cout << "Destroying" << std::endl;
                    std::string eId = entity_id;
                    auto entities = gCoordinator.getEntitiesStartsWith(eId);
                    for (auto entity: entities) {
                        auto generatedId = gCoordinator.getEntityIds()[eId];
                        gCoordinator.addComponent<Destroy>(generatedId, Destroy{});
                        gCoordinator.getComponent<Destroy>(generatedId).destroy = true;
                    }
                }
                if (command == Message::TEST) {
                    if (testStarted) {
                        return;
                    }
                    std::cout << "Starting Testing" << std::endl;
                    Entity id = gCoordinator.createEntity();
                    gCoordinator.addComponent<TestClient>(id, TestClient{});
                    gCoordinator.getComponent<TestClient>(id).testCompleted = false;
                    gCoordinator.getComponent<TestClient>(id).testStarted = false;
                    testStarted = true;
                    std::cout << "Generated test client entity" << std::endl;
                }
            } catch (std::exception &e) {
                std::string jsonString(static_cast<char *>(copy.data()), copy.size());
                std::cerr << "Error parsing message: " << jsonString << std::endl;
            }
        }
    }
};
