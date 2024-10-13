//
// Created by Utsav Lal on 10/7/24.
//

#include "lib/systems/server.hpp"

#include <thread>
#include <memory>

#include "main.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/model/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/helpers/constants.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/keyboard_movement.cpp"

#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/server_entity_system.hpp"

// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);


void platform_movement(Timeline &timeline, MoveBetween2PointsSystem &moveBetween2PointsSystem) {
    Timeline platformTimeline(&timeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        moveBetween2PointsSystem.update(dT, platformTimeline);
    }

    std::cout << "Kill platform thread" << std::endl;
}


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing server" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    GameManager::getInstance()->gameRunning = true;
    anchorTimeline.start();

    Timeline gameTimeline(&anchorTimeline, 1);
    gameTimeline.start();

    gCoordinator.init();
    gCoordinator.registerComponent<Transform>();
    gCoordinator.registerComponent<Color>();
    gCoordinator.registerComponent<CKinematic>();
    gCoordinator.registerComponent<Camera>();
    gCoordinator.registerComponent<Gravity>();
    gCoordinator.registerComponent<KeyboardMovement>();
    gCoordinator.registerComponent<Server>();
    gCoordinator.registerComponent<MovingPlatform>();
    gCoordinator.registerComponent<ServerEntity>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto severEntitySystem = gCoordinator.registerSystem<ServerEntitySystem>();
    auto serverSystem = gCoordinator.registerSystem<ServerSystem>();

    Signature movingPlatformSignature;
    movingPlatformSignature.set(gCoordinator.getComponentType<Transform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<CKinematic>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    gCoordinator.setSystemSignature<MoveBetween2PointsSystem>(movingPlatformSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature serverEntitySignature;
    serverEntitySignature.set(gCoordinator.getComponentType<ServerEntity>());
    serverEntitySignature.set(gCoordinator.getComponentType<Transform>());
    serverEntitySignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<ServerEntitySystem>(serverEntitySignature);

    Signature serverSig;
    serverSig.set(gCoordinator.getComponentType<Server>());
    gCoordinator.setSystemSignature<ServerSystem>(serverSig);

    zmq::context_t context(1);
    Entity server = gCoordinator.createEntity();
    gCoordinator.addComponent(server, Server{7000, 7001});

    auto last_time = gameTimeline.getElapsedTime();

    zmq::socket_t socket(context, ZMQ_PUB);
    socket.bind("tcp://*:" + std::to_string(SERVERPORT));

    zmq::socket_t connect_socket(context, ZMQ_REP);
    connect_socket.bind("tcp://*:" + std::to_string(engine_constants::SERVER_CONNECT_PORT));

    Entity platform = gCoordinator.createEntity();
    gCoordinator.addComponent(platform, Transform{300, 100, 100, 100});
    gCoordinator.addComponent(platform, Color{255, 0, 0, 255});
    gCoordinator.addComponent(platform, CKinematic{0, 0, 0, 0});
    gCoordinator.addComponent(platform, MovingPlatform{200, 800, LEFT, 2});
    gCoordinator.addComponent(platform, ServerEntity{});

    Entity platform2 = gCoordinator.createEntity();
    gCoordinator.addComponent(platform2, Transform{300, 300, 100, 100});
    gCoordinator.addComponent(platform2, Color{255, 255, 0, 255});
    gCoordinator.addComponent(platform2, CKinematic{0, 0, 0, 0});
    gCoordinator.addComponent(platform2, MovingPlatform{200, 800, RIGHT, 2});
    gCoordinator.addComponent(platform2, ServerEntity{});

    auto entity2 = gCoordinator.createEntity();
    gCoordinator.addComponent(entity2, Transform{300, SCREEN_HEIGHT, 32, SCREEN_WIDTH * 5, 0});
    gCoordinator.addComponent(entity2, Color{shade_color::Black});
    gCoordinator.addComponent(entity2, ServerEntity{});


    std::thread platform_thread([&gameTimeline, &moveBetween2PointsSystem]() {
        platform_movement(gameTimeline, *moveBetween2PointsSystem);
    });

    std::thread server_thread([&serverSystem, &connect_socket, &context] {
        while (GameManager::getInstance()->gameRunning) {
            serverSystem->update(context, connect_socket);
        }
    });

    std::thread network_thread([&severEntitySystem, &socket] {
        while (GameManager::getInstance()->gameRunning) {
            severEntitySystem->update(&socket);
        }
    });


    while (GameManager::getInstance()->gameRunning) {
        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;

        kinematicSystem->update(dt);
    }

    // Create 4 Rectangle instances
    platform_thread.join();
    network_thread.join();
    socket.close();

    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
