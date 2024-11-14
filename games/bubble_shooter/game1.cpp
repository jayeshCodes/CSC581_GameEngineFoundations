#include <csignal>
#include <memory>
#include <thread>


#include "../../main.hpp"
#include "../../lib/core/timeline.hpp"
#include "../../lib/ECS/coordinator.hpp"
#include "../../lib/enum/enum.hpp"
#include "../../lib/game/GameManager.hpp"
#include "../../lib/helpers/colors.hpp"
#include "../../lib/helpers/constants.hpp"
#include "../../lib/helpers/random.hpp"
#include "../../lib/model/components.hpp"
#include "../../lib/systems/camera.cpp"
#include "../../lib/systems/collision.hpp"
#include "../../lib/systems/destroy.hpp"
#include "../../lib/systems/gravity.cpp"
#include "../../lib/systems/jump.hpp"
#include "../../lib/systems/kinematic.cpp"
#include "../../lib/systems/move_between_2_point_system.hpp"
#include "../../lib/systems/render.cpp"
#include "../../lib/strategy/send_strategy.hpp"
#include "../../lib/systems/event_system.hpp"
#include "../../lib/systems/position_update_handler.hpp"
#include "handlers/collision_handler.hpp"
#include "model/component.hpp"
#include "handlers/keyboard_handler.hpp"
#include "handlers/launch_handler.hpp"
#include "systems/keyboard_movement.hpp"
#include "handlers/movement_handler.hpp"
#include "handlers/out_of_bound_handler.hpp"
#include "strategy/send_strategy.hpp"
#include "systems/client.hpp"
#include "systems/out_of_bound_detector.hpp"
#include "systems/receiver.hpp"


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
        auto message = strategy->get_message(entity, Message::DELETE);
        std::string entity_id = gCoordinator.getEntityKey(entity);
        NetworkHelper::sendMessageClient(client_socket, entity_id, message);
    }
}


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;

    screen_height = 640;
    screen_width = 640;

    initSDL(screen_width, screen_height);
    GameManager::getInstance()->gameRunning = true;
    catch_signals();

    std::unique_ptr<Send_Strategy> strategy = std::make_unique<BrickBreakerStrategy>();

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
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Ball>();
    gCoordinator.registerComponent<Brick>();
    gCoordinator.registerComponent<Launcher>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();
    auto eventSystem = gCoordinator.registerSystem<EventSystem>();
    auto oobDetectorSystem = gCoordinator.registerSystem<OutOfBoundsDetectorSystem>();


    auto positionUpdateHandler = gCoordinator.registerSystem<PositionUpdateHandler>();
    auto movementHandler = gCoordinator.registerSystem<MovementHandler>();
    auto keyboardHandler = gCoordinator.registerSystem<KeyboardHandler>();
    auto launchHandler = gCoordinator.registerSystem<LaunchHandler>();
    auto collisionHandler = gCoordinator.registerSystem<CollisionHandler>();
    auto outOfBoundHandler = gCoordinator.registerSystem<OutOfBoundHandler>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature ooBSignature;
    ooBSignature.set(gCoordinator.getComponentType<Transform>());
    ooBSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<OutOfBoundsDetectorSystem>(ooBSignature);

    Signature launchSignature;
    launchSignature.set(gCoordinator.getComponentType<Ball>());
    launchSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<LaunchHandler>(launchSignature);

    Signature gravitySignature;
    gravitySignature.set(gCoordinator.getComponentType<Transform>());
    gravitySignature.set(gCoordinator.getComponentType<Gravity>());
    gCoordinator.setSystemSignature<GravitySystem>(gravitySignature);

    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

    Signature movementHandlerSignature;
    movementHandlerSignature.set(gCoordinator.getComponentType<Transform>());
    movementHandlerSignature.set(gCoordinator.getComponentType<CKinematic>());
    movementHandlerSignature.set(gCoordinator.getComponentType<KeyboardMovement>());
    gCoordinator.setSystemSignature<MovementHandler>(movementHandlerSignature);

    Signature clientSignature;
    clientSignature.set(gCoordinator.getComponentType<Receiver>());
    gCoordinator.setSystemSignature<ReceiverSystem>(clientSignature);

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
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    zmq::socket_t reply_socket(context, ZMQ_DEALER);
    std::string id = identity + "R";
    reply_socket.set(zmq::sockopt::routing_id, id);
    reply_socket.connect("tcp://localhost:5570");

    std::thread t1([receiverSystem, &reply_socket, &strategy]() {
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(reply_socket, strategy.get());
        }
    });


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{
                                  0, 0, 1.f, 0.f, static_cast<float>(screen_width), static_cast<float>(screen_height)
                              });


    // int rows = screen_height / 40;
    // int cols = screen_width / 40;
    // int row_offset_top = 2;
    // int row_offset_bottom = 7;
    //
    //
    // for (int i = row_offset_top; i < rows - row_offset_bottom; i++) {
    //     SDL_Color color1 = shade_color::Red;
    //     SDL_Color color2 = shade_color::Blue;
    //     for (int j = 0; j < cols; j++) {
    //         auto entity = gCoordinator.createEntity();
    //         auto newColor = shade_color::generateNonRepeatingColor(color1, color2);
    //         gCoordinator.addComponent(entity, Transform{j * 40.f, i * 40.f, 40, 40, 0});
    //         gCoordinator.addComponent(entity, Color{newColor});
    //         gCoordinator.addComponent(entity, Collision{true, false, CollisionLayer::BRICK});
    //         gCoordinator.addComponent(entity, Brick{});
    //         color1 = color2;
    //         color2 = newColor;
    //     }
    // }

    auto mainChar = gCoordinator.createEntity();
    gCoordinator.addComponent(mainChar, Transform{screen_width / 2.f, screen_height - 40.f, 40, 100, 0});
    gCoordinator.addComponent(mainChar, Color{shade_color::generateRandomSolidColor()});
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, Collision{true, false, CollisionLayer::LAUNCHER});
    gCoordinator.addComponent(mainChar, KeyboardMovement{150.f});
    gCoordinator.addComponent(mainChar, Launcher{});

    auto ball = gCoordinator.createEntity();
    gCoordinator.addComponent(ball, Transform{screen_width / 2.f + 50 - 7.5f, screen_height - 40.f - 15.f, 15, 15, 0});
    gCoordinator.addComponent(ball, Color{shade_color::Gray});
    gCoordinator.addComponent(ball, CKinematic{});
    gCoordinator.addComponent(ball, Ball{false});
    gCoordinator.addComponent(ball, Collision{true, false, CollisionLayer::BALL});
    gCoordinator.addComponent(ball, KeyboardMovement{150.f});


    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();


    // Start the message sending thread
    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    std::thread t3([&collisionSystem, &destroySystem] {
        auto current_time = gameTimeline.getElapsedTime();
        while (GameManager::getInstance()->gameRunning) {
            collisionSystem->update();
            destroySystem->update();

            auto elapsed_time = gameTimeline.getElapsedTime();
            auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
            if (time_to_sleep > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
            }
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
        keyboardMovementSystem->update();

        cameraSystem->update(mainChar);
        renderSystem->update(INVALID_ENTITY);
        eventSystem->update();
        oobDetectorSystem->update(screen_width, screen_height);

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = engine_constants::FRAME_RATE - (elapsed_time - current_time); // Ensure float division
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
