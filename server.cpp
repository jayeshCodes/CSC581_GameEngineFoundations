//
// Created by Utsav Lal on 10/7/24.
//

#include <thread>
#include <memory>
#include <csignal>
#include <zmq.hpp>

#include "main.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/model/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/helpers/constants.hpp"
#include "lib/helpers/random.hpp"
#include "lib/server/worker.hpp"
#include "lib/strategy/strategy_selector.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/client.hpp"
#include "lib/systems/collision.hpp"
#include "lib/systems/destroy.hpp"
#include "lib/systems/jump.hpp"
#include "lib/systems/keyboard_movement.cpp"

#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/receiver.hpp"

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
        auto elapsed_time = platformTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - currentTime); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }
    }

    std::cout << "Kill platform thread" << std::endl;
}

void server_run(zmq::context_t &context, zmq::socket_ref frontend, zmq::socket_ref backend, Send_Strategy *send_strategy) {
    int max_threads = 5;

    std::vector<std::unique_ptr<Worker> > workers;
    std::vector<std::unique_ptr<std::thread> > threads;

    for (int i = 0; i < max_threads; i++) {
        workers.push_back(std::make_unique<Worker>(context, ZMQ_DEALER, "WORKER" + std::to_string(i)));
        threads.push_back(std::make_unique<std::thread>(&Worker::work, workers[i].get(), send_strategy));
        threads[i]->detach();
    }

    try {
        zmq::proxy(zmq::socket_ref(frontend), zmq::socket_ref(backend), nullptr);
    } catch (std::exception &e) {
        std::cout << "Server error: " << e.what() << std::endl;
    }
    std::cout << "Kill server thread" << std::endl;
}

void catch_signals() {
    std::signal(SIGINT, [](int signal) {
        std::cout << "Caught SIGINT" << std::endl;
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGTERM, [](int signal) {
        std::cout << "Caught SIGTERM" << std::endl;
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGSEGV, [](int signal) {
        std::cout << "Caught SIGSEGV" << std::endl;
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGABRT, [](int signal) {
        std::cout << "Caught SIGABRT" << std::endl;
        GameManager::getInstance()->gameRunning = false;
    });
}


int main(int argc, char *argv[]) {
    // Register signals so that we can gracefully shutdown the server
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing server" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    GameManager::getInstance()->gameRunning = true;
    anchorTimeline.start();
    std::unique_ptr<Send_Strategy> strategy = nullptr;
    if(argv[1] != nullptr) {
        strategy = Strategy::select_message_strategy(argv[1]);
    } else {
        strategy = Strategy::select_message_strategy("float");
    }

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
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<RigidBody>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();

    Signature clientEntitySignature;
    clientEntitySignature.set(gCoordinator.getComponentType<ClientEntity>());
    clientEntitySignature.set(gCoordinator.getComponentType<Transform>());
    clientEntitySignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<ClientSystem>(clientEntitySignature);

    Signature clientSignature;
    clientSignature.set(gCoordinator.getComponentType<Receiver>());
    gCoordinator.setSystemSignature<ReceiverSystem>(clientSignature);

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

    Signature serverSig;
    serverSig.set(gCoordinator.getComponentType<Server>());
    serverSig.set(gCoordinator.getComponentType<Destroy>());

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


    // create a platform
    auto ground = gCoordinator.createEntity();
    gCoordinator.addComponent(ground, Transform{0, SCREEN_HEIGHT - 100.f, 500.f, 300.f, 0});
    gCoordinator.addComponent(ground, Color{shade_color::Blue});
    gCoordinator.addComponent(ground, ClientEntity{});
    gCoordinator.addComponent(ground, RigidBody{-1.f});
    gCoordinator.addComponent(ground, Collision{true, false, CollisionLayer::OTHER});
    gCoordinator.addComponent(ground, CKinematic{});

    auto ground2 = gCoordinator.createEntity();
    gCoordinator.addComponent(ground2, Transform{800, SCREEN_HEIGHT - 100.f, 500.f, 1000.f, 0});
    gCoordinator.addComponent(ground2, Color{shade_color::Green});
    gCoordinator.addComponent(ground2, ClientEntity{});
    gCoordinator.addComponent(ground2, RigidBody{-1.f});
    gCoordinator.addComponent(ground2, Collision{true, false, CollisionLayer::OTHER});
    gCoordinator.addComponent(ground2, CKinematic{});

    Entity platform = gCoordinator.createEntity();
    gCoordinator.addComponent(platform, Transform{300, SCREEN_HEIGHT - 100.f, 50, 200});
    gCoordinator.addComponent(platform, Color{shade_color::Red});
    gCoordinator.addComponent(platform, CKinematic{0, 0, 0, 0});
    gCoordinator.addComponent(platform, MovingPlatform{300, 800 - 200, TO, 2, HORIZONTAL});
    gCoordinator.addComponent(platform, Destroy{});
    gCoordinator.addComponent(platform, ClientEntity{true});
    gCoordinator.addComponent(platform, RigidBody{-1.f});
    gCoordinator.addComponent(platform, Collision{true, false, CollisionLayer::MOVING_PLATFORM});

    // Entity platform2 = gCoordinator.createEntity();
    // gCoordinator.addComponent(platform2, Transform{300, 500, 100, 400});
    // gCoordinator.addComponent(platform2, Color{shade_color::Brown});
    // gCoordinator.addComponent(platform2, CKinematic{0, 0, 0, 0});
    // gCoordinator.addComponent(platform2, Destroy{});
    // gCoordinator.addComponent(platform2, ClientEntity{true});
    // gCoordinator.addComponent(platform2, Collision{true, false, CollisionLayer::OTHER});
    // gCoordinator.addComponent(platform2, RigidBody{-1});

    // Entity platform2 = gCoordinator.createEntity();
    // gCoordinator.addComponent(platform2, Transform{300, 300, 100, 100});
    // gCoordinator.addComponent(platform2, Color{255, 255, 0, 255});
    // gCoordinator.addComponent(platform2, CKinematic{0, 0, 0, 0});
    // gCoordinator.addComponent(platform2, MovingPlatform{200, 800, RIGHT, 2});
    // gCoordinator.addComponent(platform2, ClientEntity{true});
    // gCoordinator.addComponent(platform2, Destroy{});

    zmq::context_t context(1);
    zmq::socket_t frontend(context, ZMQ_ROUTER);
    zmq::socket_t backend(context, ZMQ_DEALER);
    frontend.bind("tcp://*:5570");
    backend.bind("tcp://*:5571");
    std::thread server_thread(server_run, std::ref(context), zmq::socket_ref(frontend), zmq::socket_ref(backend), strategy.get());

    std::string identity = Random::generateRandomID(10);
    std::cout << "Identity: " << identity << std::endl;

    zmq::socket_t client_socket(context, ZMQ_DEALER);

    client_socket.set(zmq::sockopt::routing_id, identity);
    client_socket.connect("tcp://localhost:5570");

    Entity server = gCoordinator.createEntity();
    gCoordinator.addComponent(server, Server{7000, 7001});

    auto last_time = gameTimeline.getElapsedTime();

    zmq::socket_t socket(context, ZMQ_PUB);
    socket.bind("tcp://*:" + std::to_string(SERVERPORT));

    zmq::socket_t connect_socket(context, ZMQ_REP);
    connect_socket.bind("tcp://*:" + std::to_string(engine_constants::SERVER_CONNECT_PORT));


    std::thread platform_thread([&gameTimeline, &moveBetween2PointsSystem]() {
        platform_movement(gameTimeline, *moveBetween2PointsSystem);
    });

    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    std::thread t1([receiverSystem, &context, &identity, &strategy]() {
        zmq::socket_t socket(context, ZMQ_DEALER);
        std::string id = identity + "R";
        socket.set(zmq::sockopt::routing_id, id);
        socket.connect("tcp://localhost:5570");
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(socket, strategy.get());
        }
    });


    while (GameManager::getInstance()->gameRunning) {
        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;
        dt = std::max(dt, 1 / 60.f);

        kinematicSystem->update(dt);
        destroySystem->update();

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }
    }

    // Create 4 Rectangle instances
    platform_thread.join();
    t2.join();
    server_thread.join();

    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
