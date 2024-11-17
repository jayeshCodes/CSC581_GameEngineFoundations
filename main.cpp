#include <memory>
#include <thread>

#include "main.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/enum/enum.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/helpers/constants.hpp"
#include "lib/helpers/random.hpp"
#include "lib/model/components.hpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/client.hpp"
#include "lib/systems/collision.hpp"
#include "lib/systems/destroy.hpp"
#include "lib/systems/jump.hpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/receiver.hpp"
#include "lib/systems/render.cpp"
#include <csignal>

#include "lib/strategy/send_strategy.hpp"
#include "lib/strategy/strategy_selector.hpp"
#include "lib/systems/bubble_grid.hpp"
#include "lib/systems/bubble_shooter.hpp"
#include "lib/systems/bubble_movement.hpp"
#include "lib/systems/event_system.hpp"
#include "lib/systems/collision_handler.hpp"
#include "lib/systems/entity_created_handler.hpp"
#include "lib/systems/position_update_handler.hpp"
#include "lib/systems/score_display.hpp"

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
        auto message = strategy->get_message(entity, Message::DELETE);
        std::string entity_id = gCoordinator.getEntityKey(entity);
        NetworkHelper::sendMessageClient(client_socket, entity_id, message);
    }
}

void collision_update(Timeline &timeline, CollisionSystem *collisionSystem) {
    Timeline collisionTimeline(&timeline, 1);
    int64_t lastTime = collisionTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        try {
            int64_t currentTime = collisionTimeline.getElapsedTime();
            float dT = (currentTime - lastTime) / 1000.f;
            lastTime = currentTime;

            collisionSystem->update();

            // Add a small sleep to reduce CPU usage and give other threads a chance
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 fps
        } catch (const std::exception &e) {
            std::cout << "Error in collision thread: " << e.what() << std::endl;
        }
    }
    std::cout << "Kill collision thread" << std::endl;
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
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<RigidBody>();
    gCoordinator.registerComponent<Bubble>();
    gCoordinator.registerComponent<BubbleShooter>();
    gCoordinator.registerComponent<BubbleProjectile>();
    gCoordinator.registerComponent<BubbleGridManager>();
    // Debug component registration
    gCoordinator.registerComponent<Score>();
    std::cout << "Score component registered with type ID: " << gCoordinator.getComponentType<Score>() << std::endl;


    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();
    auto collisonHandlerSystem = gCoordinator.registerSystem<CollisionHandlerSystem>();
    auto eventSystem = gCoordinator.registerSystem<EventSystem>();
    auto entityCreatedSystem = gCoordinator.registerSystem<EntityCreatedHandler>();
    auto positionUpdateHandler = gCoordinator.registerSystem<PositionUpdateHandler>();
    auto bubbleShooterSystem = gCoordinator.registerSystem<BubbleShooterSystem>();
    auto bubbleMovementSystem = gCoordinator.registerSystem<BubbleMovementSystem>();
    auto bubbleGridSystem = gCoordinator.registerSystem<BubbleGridSystem>();
    auto scoreDisplaySystem = gCoordinator.registerSystem<ScoreDisplaySystem>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

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
    collisionSignature.set(gCoordinator.getComponentType<CKinematic>());
    collisionSignature.set(gCoordinator.getComponentType<RigidBody>());
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    Signature bubbleShooterSignature;
    bubbleShooterSignature.set(gCoordinator.getComponentType<BubbleShooter>());
    bubbleShooterSignature.set(gCoordinator.getComponentType<Transform>());
    bubbleShooterSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<BubbleShooterSystem>(bubbleShooterSignature);

    Signature bubbleMovementSignature;
    bubbleMovementSignature.set(gCoordinator.getComponentType<BubbleProjectile>());
    bubbleMovementSignature.set(gCoordinator.getComponentType<Transform>());
    bubbleMovementSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<BubbleMovementSystem>(bubbleMovementSignature);

    Signature bubbleGridSignature;
    bubbleGridSignature.set(gCoordinator.getComponentType<Transform>());
    bubbleGridSignature.set(gCoordinator.getComponentType<Color>());
    bubbleGridSignature.set(gCoordinator.getComponentType<Score>());
    gCoordinator.setSystemSignature<BubbleGridSystem>(bubbleGridSignature);

    Signature scoreDisplaySignature;
    scoreDisplaySignature.set(gCoordinator.getComponentType<Score>());
    gCoordinator.setSystemSignature<ScoreDisplaySystem>(scoreDisplaySignature);

    zmq::socket_t reply_socket(context, ZMQ_DEALER);
    std::string id = identity + "R";
    reply_socket.set(zmq::sockopt::routing_id, id);
    reply_socket.connect("tcp://localhost:5570");

    std::thread t1([receiverSystem, &reply_socket, &strategy]() {
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(reply_socket, strategy.get());
        }
    });

    Entity shooter = gCoordinator.createEntity();
    gCoordinator.addComponent(shooter, Transform{SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f, 32.f, 32.f, 0});
    gCoordinator.addComponent(shooter, BubbleShooter{
                                  2.0f, // rotationSpeed
                                  225.0f, // minAngle (pointing up-left)
                                  315.0f, // maxAngle (pointing up-right)
                                  270.0f, // currentAngle (pointing straight up)
                                  400.f, // shootForce
                                  true, // canShoot
                                  0.5f, // reloadTime
                                  0.0f // currentReloadTime
                              });
    gCoordinator.addComponent(shooter, Color{shade_color::White});
    gCoordinator.addComponent(shooter, ClientEntity{});
    gCoordinator.addComponent(shooter, CKinematic{{0, 0}, 0, {0, 0}, 0}); // Add with initial values
    gCoordinator.addComponent(shooter, Destroy{});

    const float GRID_SIZE = 32.0f;
    const float GRID_COLS = 12;
    const float GRID_OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * GRID_SIZE)) / 2.0f;
    const float BOUNDARY_WIDTH = 10.0f; // Width of invisible boundary
    const float BOUNDARY_HEIGHT = SCREEN_HEIGHT; // Full height boundary

    // Create left boundary
    Entity leftBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(leftBoundary, Transform{
                                  GRID_OFFSET_X - BOUNDARY_WIDTH, // Position just left of grid
                                  0, // Top of screen
                                  BOUNDARY_HEIGHT, // Height
                                  BOUNDARY_WIDTH, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(leftBoundary, Color{shade_color::White});
    gCoordinator.addComponent(leftBoundary, RigidBody{-1.0f}); // Immovable object
    gCoordinator.addComponent(leftBoundary, Collision{true, false, CollisionLayer::OTHER});
    gCoordinator.addComponent(leftBoundary, CKinematic{}); // Required for collision system

    // Create right boundary
    Entity rightBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(rightBoundary, Transform{
                                  GRID_OFFSET_X + (GRID_COLS * GRID_SIZE), // Position just right of grid
                                  0, // Top of screen
                                  BOUNDARY_HEIGHT, // Height
                                  BOUNDARY_WIDTH, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(rightBoundary, Color{shade_color::White}); // Invisible boundary
    gCoordinator.addComponent(rightBoundary, RigidBody{-1.0f}); // Immovable object
    gCoordinator.addComponent(rightBoundary, Collision{true, false, CollisionLayer::OTHER});
    gCoordinator.addComponent(rightBoundary, CKinematic{}); // Required for collision system


    Entity scoreEntity = gCoordinator.createEntity();
    std::cout << "Created score entity: " << scoreEntity << std::endl;

    Score initialScore{};  // Initialize with default values
    initialScore.value = 0;
    initialScore.multiplier = 1;
    initialScore.bubbleScore = 100;
    initialScore.comboBonus = 50;

    try {
        gCoordinator.addComponent(scoreEntity, initialScore);
        std::cout << "Successfully added Score component to entity " << scoreEntity << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Score component: " << e.what() << std::endl;
    }

    // Verify the component was added
    if (gCoordinator.hasComponent<Score>(scoreEntity)) {
        std::cout << "Score component verified on entity " << scoreEntity << std::endl;
    } else {
        std::cerr << "Failed to verify Score component on entity " << scoreEntity << std::endl;
    }


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{0, 0, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT});


    mainCharID = gCoordinator.getEntityKey(shooter);

    Event entityCreatedEvent{eventTypeToString(MainCharCreated), {}};
    entityCreatedEvent.data = MainCharCreatedData{shooter, strategy->get_message(shooter, Message::CREATE)};
    eventCoordinator.emitServer(client_socket, std::make_shared<Event>(entityCreatedEvent));
    gCoordinator.getComponent<ClientEntity>(shooter).synced = true;


    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();


    // Start the message sending thread
    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    // Collision thread
    std::thread collision_thread([&collisionSystem]() {
        collision_update(gameTimeline, collisionSystem.get());
    });

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, engine_constants::FRAME_RATE); // Cap the maximum dt to 60fps

        try {

            kinematicSystem->update(dt);
            // collisionSystem->update();
            bubbleMovementSystem->update(  dt);
            destroySystem->update();
            cameraSystem->update(shooter);
            renderSystem->update(mainCamera);

            eventSystem->update();
            bubbleShooterSystem->update(dt);
            scoreDisplaySystem->update();
        } catch (const std::exception& e) {
            std::cerr << "Error in game loop: " << e.what() << std::endl;
        }


        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }

        presentScene();
    }
    collision_thread.join();
    /**
     * This is the cleanup code. The order is very important here since otherwise the program will crash.
     */
    send_delete_signal(client_socket, shooter, strategy.get());
    t1.join();
    t2.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
