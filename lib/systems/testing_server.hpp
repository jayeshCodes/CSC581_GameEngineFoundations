//
// Created by Utsav Lal on 10/16/24.
//

#pragma once
#include <zmq.hpp>

#include "../ECS/system.hpp"
#include "../strategy/send_strategy.hpp"

class TestingServerSystem : public System {
public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        for (auto entity: entities) {
            auto &testbench = gCoordinator.getComponent<TestServer>(entity);
            if(testbench.start == false) {
                continue;
            }
            auto emptyTransform = Transform{};
            auto emptyColor = Color{};
            auto emptyCollision = Collision{false, false};
            auto emptyRigidBody = RigidBody{0};
            auto message = send_strategy->get_message(entity, emptyTransform, emptyColor, Message::TEST, emptyRigidBody, emptyCollision);

            for (int i = 0; i < 4500; i++) {
                const Entity id = gCoordinator.createEntity();
                auto x = static_cast<float>(Random::generateRandomInt(0, SCREEN_WIDTH));
                Transform transform{static_cast<float>(Random::generateRandomInt(0, SCREEN_WIDTH)), static_cast<float>(Random::generateRandomFloat(0, SCREEN_WIDTH)), 32, 32};
                Color color{
                    SDL_Color{
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)),
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)),
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)), 255
                    }
                };
                gCoordinator.addComponent(id, transform);
                gCoordinator.addComponent(id, color);
                gCoordinator.addComponent(id, CKinematic{static_cast<float>(Random::generateRandomFloat(1,2)), static_cast<float>(Random::generateRandomFloat(1,2))});
                gCoordinator.addComponent(id, ClientEntity{});
                gCoordinator.addComponent(id, Destroy{});

            }


            // Send 5 times just to be sure
            for (int i = 0; i < 10; i++) {
                std::string entity_id = socket.get(zmq::sockopt::routing_id) + std::to_string(-123);
                socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
                if (std::holds_alternative<std::string>(message)) {
                    const auto str = std::get<std::string>(message);
                    socket.send(zmq::buffer(str), zmq::send_flags::none);
                } else {
                    auto vec = std::get<std::vector<float> >(message);
                    socket.send(zmq::buffer(vec), zmq::send_flags::none);
                }
            }
            testbench.start = false;
            std::cout << "Test started" << std::endl;
        }
    }
};
