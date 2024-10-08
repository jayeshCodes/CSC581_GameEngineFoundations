//
// Created by Utsav Lal on 10/7/24.
//

#include <thread>
#include <memory>

#include "main.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/components/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/systems/server.hpp"

// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);
Timeline gameTimeline(&anchorTimeline, 1);

Coordinator gCoordinator;

void connect_clients(ServerSystem<float> &serverSystem, zmq::context_t &context) {
    while (GameManager::getInstance()->gameRunning) {
        serverSystem.connect_client(context);
    }

}

void send_msg(ServerSystem<float> &serverSystem) {
    while (GameManager::getInstance()->gameRunning) {
        serverSystem.send_message();
    }
}


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing server" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    GameManager::getInstance()->gameRunning = true;

    anchorTimeline.start();
    gameTimeline.start();

    gCoordinator.init();
    gCoordinator.registerComponent<Transform>();
    gCoordinator.registerComponent<Color>();
    gCoordinator.registerComponent<CKinematic>();
    gCoordinator.registerComponent<Camera>();
    gCoordinator.registerComponent<Gravity>();
    gCoordinator.registerComponent<KeyboardMovement>();
    gCoordinator.registerComponent<Server>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto serverSystem = gCoordinator.registerSystem<ServerSystem<float> >();

    Signature serverSignature;
    serverSignature.set(gCoordinator.getComponentType<Server>());
    gCoordinator.setSystemSignature<ServerSystem<float> >(serverSignature);

    zmq::context_t context(1);
    Entity server = gCoordinator.createEntity();
    gCoordinator.addComponent(server, Server{7000, 7001});

    auto last_time = gameTimeline.getElapsedTime();

    serverSystem->initialize(context);
    std::thread connect_thread([&serverSystem, &context]() { connect_clients(*serverSystem, context); });
    std::thread send_msg_thread([&serverSystem]() { send_msg(*serverSystem); });

    while (GameManager::getInstance()->gameRunning) {
        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;
    }

    // Create 4 Rectangle instances
    connect_thread.join();
    send_msg_thread.join();

    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
