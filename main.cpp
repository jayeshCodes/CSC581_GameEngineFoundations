#include <memory>
#include <thread>

#include "main.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/enum/enum.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/helpers/constants.hpp"
#include "lib/helpers/ecs_helpers.hpp"
#include "lib/helpers/random.hpp"
#include "lib/model/components.hpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/client.hpp"
#include "lib/systems/collision.hpp"
#include "lib/systems/destroy.hpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/jump.hpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/death.hpp"
#include "lib/systems/receiver.hpp"
#include "lib/systems/render.cpp"
#include <csignal>

#include "lib/strategy/send_strategy.hpp"
#include "lib/strategy/strategy_selector.hpp"
#include "lib/systems/event_system.hpp"
#include "lib/systems/keyboard.hpp"
#include "lib/systems/respawn.hpp"
#include "lib/systems/collision_handler.hpp"
#include "lib/systems/trigger_handler.hpp"

class ReceiverSystem;
// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds


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
        Collision empty_collision{0, 0};
        RigidBody empty_rigid_body{0};
        auto message = strategy->get_message(entity, empty_transform, empty_color, Message::DELETE, empty_rigid_body,
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


    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();
    auto deathSystem = gCoordinator.registerSystem<DeathSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();
    auto respawnSystem = gCoordinator.registerSystem<RespawnSystem>();
    auto keyboardSystem = gCoordinator.registerSystem<KeyboardSystem>();
    auto collisonHandlerSystem = gCoordinator.registerSystem<CollisionHandlerSystem>();
    auto triggerHandlerSystem = gCoordinator.registerSystem<TriggerHandlerSystem>();
    auto eventSystem = gCoordinator.registerSystem<EventSystem>();

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
    respawnSignature.set(gCoordinator.getComponentType<Transform>());
    respawnSignature.set(gCoordinator.getComponentType<Collision>());
    gCoordinator.setSystemSignature<DeathSystem>(respawnSignature);


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{.x=0, 0, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT});

    auto mainChar = gCoordinator.createEntity();
    gCoordinator.addComponent(mainChar, Transform{.x=0.f, SCREEN_HEIGHT - 200.f, 32, 32, 0});
    gCoordinator.addComponent(mainChar, Color{.color=shade_color::generateRandomSolidColor()});
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, KeyboardMovement{.speed=150.f});
    gCoordinator.addComponent(mainChar, ClientEntity{});
    gCoordinator.addComponent(mainChar, Destroy{});
    gCoordinator.addComponent(mainChar, Jump{.maxJumpHeight=50.f, 1.f, false, 0.0f, true, 120.f});
    gCoordinator.addComponent(mainChar, Gravity{.gravX=0, 100});
    gCoordinator.addComponent(mainChar, Respawnable{.lastSafePosition={.x=0, SCREEN_HEIGHT - 200.f, 32, 32, 0, 1}, false});
    gCoordinator.addComponent(mainChar, RigidBody{.mass=1.f});
    gCoordinator.addComponent(mainChar, Collision{.isCollider=true, false, CollisionLayer::PLAYER});

    auto trigger = gCoordinator.createEntity();
    gCoordinator.addComponent(trigger, Transform{.x=100.f, SCREEN_HEIGHT - 150.f, 32, 32, 0});
    gCoordinator.addComponent(trigger, Color{.color=shade_color::Black});
    gCoordinator.addComponent(trigger, CKinematic{});
    gCoordinator.addComponent(trigger, ClientEntity{});
    gCoordinator.addComponent(trigger, Destroy{});
    gCoordinator.addComponent(trigger, RigidBody{.mass=-1.f});
    gCoordinator.addComponent(trigger, Collision{false, true, CollisionLayer::OTHER});


    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();

    // Start the message receiver thread
    std::thread t1([receiverSystem, &context, &identity, &strategy]() {
        zmq::socket_t socket(context, ZMQ_DEALER);
        std::string id = identity + "R";
        socket.set(zmq::sockopt::routing_id, id);
        socket.connect("tcp://localhost:5570");
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(socket, strategy.get());
        }
    });

    // Start the message sending thread
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
        jumpSystem->update(dt);
        gravitySystem->update(dt);
        keyboardMovementSystem->update();
        collisionSystem->update();
        deathSystem->update();
        destroySystem->update();
        cameraSystem->update(mainChar);
        renderSystem->update(mainCamera);
        eventSystem->update();

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }

        presentScene();
    }

    /**
     * This is the cleanup code. The order is very important here since otherwise the program will crash.
     */
    send_delete_signal(client_socket, mainChar, strategy.get());
    t1.join();
    t2.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
