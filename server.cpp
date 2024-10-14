//
// Created by Utsav Lal on 10/7/24.
//

#include "lib/systems/server.hpp"

#include <thread>
#include <memory>
#include <csignal>

#include "main.hpp"
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
#include "lib/systems/collision.hpp"
#include "lib/systems/destroy.hpp"
#include "lib/systems/jump.hpp"
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
void handleSignal(int signal) {
    GameManager::getInstance()->gameRunning = false;
}



int main(int argc, char *argv[]) {
    // Register signals so that we can gracefully shutdown the server
    std::signal(SIGTERM, handleSignal);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGILL, handleSignal);
    std::signal(SIGABRT, handleSignal);
    std::signal(SIGFPE, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGTERM, handleSignal);

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
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Jump>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto severEntitySystem = gCoordinator.registerSystem<ServerEntitySystem>();
    auto serverSystem = gCoordinator.registerSystem<ServerSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();

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
    serverSig.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<ServerSystem>(serverSig);

    Signature destroySig;
    destroySig.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<DestroySystem>(destroySig);

    Signature collisionSignature;
    collisionSignature.set(gCoordinator.getComponentType<Transform>());
    collisionSignature.set(gCoordinator.getComponentType<Collision>());
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    Signature jumpSignature;
    jumpSignature.set(gCoordinator.getComponentType<Transform>());
    jumpSignature.set(gCoordinator.getComponentType<CKinematic>());
    jumpSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<JumpSystem>(jumpSignature);

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
    gCoordinator.addComponent(platform, Destroy{});
    gCoordinator.addComponent(platform, ServerEntity{});

    Entity platform2 = gCoordinator.createEntity();
    gCoordinator.addComponent(platform2, Transform{300, 300, 100, 100});
    gCoordinator.addComponent(platform2, Color{255, 255, 0, 255});
    gCoordinator.addComponent(platform2, CKinematic{0, 0, 0, 0});
    gCoordinator.addComponent(platform2, MovingPlatform{200, 800, RIGHT, 2});
    gCoordinator.addComponent(platform2, Destroy{});
    gCoordinator.addComponent(platform2, ServerEntity{});

    auto entity2 = gCoordinator.createEntity();
    gCoordinator.addComponent(entity2, Transform{300, SCREEN_HEIGHT, 32, SCREEN_WIDTH * 5, 0});
    gCoordinator.addComponent(entity2, Color{shade_color::Black});
    gCoordinator.addComponent(entity2, Destroy{});
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

    std::thread delete_thread([&destroySystem]() {
        while (GameManager::getInstance()->gameRunning) {
            destroySystem->update();
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
    delete_thread.join();
    server_thread.join();

    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
