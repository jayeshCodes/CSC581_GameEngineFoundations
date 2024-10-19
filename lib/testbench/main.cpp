#include <memory>
#include <thread>
#include <csignal>

#include "main.hpp"
#include "../core/timeline.hpp"
#include "../ECS/coordinator.hpp"
#include "../enum/message_type.hpp"
#include "../game/GameManager.hpp"
#include "../helpers/colors.hpp"
#include "../helpers/constants.hpp"
#include "../helpers/ecs_helpers.hpp"
#include "../helpers/random.hpp"
#include "../model/components.hpp"
#include "../systems/camera.cpp"
#include "../systems/client.hpp"
#include "../systems/collision.hpp"
#include "../systems/destroy.hpp"
#include "../systems/gravity.cpp"
#include "../systems/jump.hpp"
#include "../systems/keyboard_movement.cpp"
#include "../systems/kinematic.cpp"
#include "../systems/move_between_2_point_system.hpp"
#include "../systems/respawn.hpp"
#include "../systems/receiver.hpp"
#include "../systems/render.cpp"
#include "../strategy/send_strategy.hpp"
#include "../strategy/strategy_selector.hpp"
#include "../systems/testing_client.hpp"

class ReceiverSystem;
// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);
Timeline gameTimeline(&anchorTimeline, 1);

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

void catch_signals() {
    std::signal(SIGINT, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGTERM, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGSEGV, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGABRT, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
}

void send_delete_signal(zmq::socket_t &client_socket, Entity entity, Send_Strategy *strategy) {
    for (int i = 0; i < 5; i++) {
        Transform empty_transform{0, 0, 0, 0};
        Color empty_color{0, 0, 0, 0};
        Collision empty_collision{false, false};
        RigidBody empty_rigidbody{0};
        auto message = strategy->get_message(entity, empty_transform, empty_color, Message::DELETE, empty_rigidbody,
                                             empty_collision);
        std::string entity_id = gCoordinator.getEntityKey(entity);
        client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        if (std::holds_alternative<std::string>(message)) {
            auto str = std::get<std::string>(message);
            client_socket.send(zmq::buffer(str), zmq::send_flags::none);
        } else {
            auto vec = std::get<std::vector<float> >(message);
            client_socket.send(zmq::buffer(vec), zmq::send_flags::none);
        }
    }
}


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    GameManager::getInstance()->gameRunning = true;
    catch_signals();

    std::unique_ptr<Send_Strategy> strategy = nullptr;
    if (argv[1] != nullptr) {
        strategy = Strategy::select_message_strategy(argv[1]);
    } else {
        strategy = Strategy::select_message_strategy("float");
    }

    std::string identity = Random::generateRandomID(10);
    std::cout << "Identity: " << identity << std::endl;

    zmq::context_t context(1);
    zmq::socket_t client_socket(context, ZMQ_DEALER);

    client_socket.set(zmq::sockopt::routing_id, identity);
    client_socket.connect("tcp://localhost:5570");

    zmq::pollitem_t items[] = {{client_socket, 0, ZMQ_POLLIN, 0}};

    anchorTimeline.start();
    gameTimeline.start();

    std::vector<std::string> component_names{MAX_COMPONENTS};
    std::vector<std::string> entity_names{MAX_ENTITIES};

    gCoordinator.init();
    gCoordinator.registerComponent<Transform>();
    gCoordinator.registerComponent<Color>();
    gCoordinator.registerComponent<CKinematic>();
    gCoordinator.registerComponent<Camera>();
    gCoordinator.registerComponent<Gravity>();
    gCoordinator.registerComponent<KeyboardMovement>();
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<MovingPlatform>();
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Jump>();
    gCoordinator.registerComponent<Platform>();
    gCoordinator.registerComponent<Respawnable>();
    gCoordinator.registerComponent<RigidBody>();
    gCoordinator.registerComponent<TestClient>();


    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();
    auto respawnSystem = gCoordinator.registerSystem<RespawnSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();
    auto testingClientSystem = gCoordinator.registerSystem<TestingClientSystem>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature gravitySignature;
    gravitySignature.set(gCoordinator.getComponentType<Transform>());
    gravitySignature.set(gCoordinator.getComponentType<Gravity>());
    gCoordinator.setSystemSignature<GravitySystem>(gravitySignature);

    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

    Signature keyboardMovementSignature;
    keyboardMovementSignature.set(gCoordinator.getComponentType<Transform>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<CKinematic>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<KeyboardMovement>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<KeyboardMovementSystem>(keyboardMovementSignature);

    Signature clientSignature;
    clientSignature.set(gCoordinator.getComponentType<Receiver>());
    gCoordinator.setSystemSignature<ReceiverSystem>(clientSignature);

    Signature movingPlatformSignature;
    movingPlatformSignature.set(gCoordinator.getComponentType<Transform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<CKinematic>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    gCoordinator.setSystemSignature<MoveBetween2PointsSystem>(movingPlatformSignature);

    Signature clientEntitySignature;
    clientEntitySignature.set(gCoordinator.getComponentType<ClientEntity>());
    clientEntitySignature.set(gCoordinator.getComponentType<Transform>());
    clientEntitySignature.set(gCoordinator.getComponentType<Color>());
    clientEntitySignature.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<ClientSystem>(clientEntitySignature);

    Signature destroySig;
    destroySig.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<DestroySystem>(destroySig);

    Signature collisionSignature;
    collisionSignature.set(gCoordinator.getComponentType<Transform>());
    collisionSignature.set(gCoordinator.getComponentType<Collision>());
    collisionSignature.set(gCoordinator.getComponentType<CKinematic>());
    collisionSignature.set(gCoordinator.getComponentType<RigidBody>());
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    Signature jumpSignature;
    jumpSignature.set(gCoordinator.getComponentType<Transform>());
    jumpSignature.set(gCoordinator.getComponentType<CKinematic>());
    jumpSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<JumpSystem>(jumpSignature);

    Signature respawnSignature;
    respawnSignature.set(gCoordinator.getComponentType<Respawnable>());
    gCoordinator.setSystemSignature<RespawnSystem>(respawnSignature);

    Signature testingClientSignature;
    testingClientSignature.set(gCoordinator.getComponentType<TestClient>());
    gCoordinator.setSystemSignature<TestingClientSystem>(testingClientSignature);


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{
                                  SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT
                              }); // temporary values for viewport width and height


    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();


    std::thread delete_thread([&destroySystem]() {
        while (GameManager::getInstance()->gameRunning) {
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

    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, engine_constants::FRAME_RATE); // Cap the maximum dt to 60fps

        kinematicSystem->update(dt);
        gravitySystem->update(dt);
        testingClientSystem->update(gameTimeline);
        receiverSystem->update(client_socket, strategy.get());


        renderSystem->update(INVALID_ENTITY);

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }

        presentScene();
    }

    delete_thread.join();
    t1.join();
    t2.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
