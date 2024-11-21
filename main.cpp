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
#include "lib/systems/bubble_event_handler.hpp"
#include "lib/systems/bubble_grid.hpp"
#include "lib/systems/bubble_shooter.hpp"
#include "lib/systems/bubble_movement.hpp"
#include "lib/systems/event_system.hpp"
#include "lib/systems/collision_handler.hpp"
#include "lib/systems/entity_created_handler.hpp"
#include "lib/systems/position_update_handler.hpp"
#include "lib/systems/score_display.hpp"
#include "lib/systems/bubble_grid_movement.hpp"
#include "lib/systems/bubble_match_handler.hpp"
#include "lib/systems/game_over_handler.hpp"
#include "lib/systems/grid_generator.hpp"
#include "lib/systems/instruction_render.hpp"
#include "lib/systems/keyboard.hpp"
#include "lib/systems/pause_handler.hpp"
#include "lib/systems/pop_animation.hpp"
#include "lib/systems/reset_handler.hpp"
#include "lib/systems/shooter_disable_handler.hpp"
#include "lib/systems/shooter_reset_handler.hpp"
#include "lib/systems/shooter_verification.hpp"
#include "lib/systems/sound.hpp"
#include "lib/systems/sound_event_handler.hpp"

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

void gameSystemsUpdate(
    std::shared_ptr<KinematicSystem> kinematicSystem,
    std::shared_ptr<BubbleMovementSystem> bubbleMovementSystem,
    std::shared_ptr<DestroySystem> destroySystem,
    std::shared_ptr<EventSystem> eventSystem,
    std::shared_ptr<BubbleShooterSystem> bubbleShooterSystem,
    std::shared_ptr<BubbleGridGeneratorSystem> gridGeneratorSystem,
    std::shared_ptr<BubbleGridMovementSystem> gridMovementSystem,
    std::shared_ptr<PopAnimationSystem> popAnimationSystem,
    std::shared_ptr<KeyboardMovementSystem> keyboardMovementSystem,
    std::shared_ptr<PauseHandlerSystem> pauseHandlerSystem
) {
    Timeline systemTimeline(&gameTimeline, 1);
    auto last_time = systemTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        auto current_time = systemTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Convert to seconds

        last_time = current_time;
        dt = std::max(dt, engine_constants::FRAME_RATE);

        if (!gameTimeline.isPaused()) {
            try {
                // Event and state systems
                eventSystem->update();

                // Game logic systems
                kinematicSystem->update(dt);
                bubbleMovementSystem->update(dt);

                // Animation systems
                popAnimationSystem->update(dt);

                // shooterVerificationSystem->update();
                bubbleShooterSystem->update(dt);

                // grid systems
                gridGeneratorSystem->update();
                gridMovementSystem->update(dt);
            } catch (const std::exception &e) {
                std::cerr << "Error in game systems thread: " << e.what() << std::endl;
            }
        }
        keyboardMovementSystem->update();

        // Maintain consistent update rate
        auto elapsed_time = systemTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time);
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
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
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<RigidBody>();
    gCoordinator.registerComponent<Bubble>();
    gCoordinator.registerComponent<BubbleShooter>();
    gCoordinator.registerComponent<BubbleProjectile>();
    gCoordinator.registerComponent<BubbleGridManager>();
    gCoordinator.registerComponent<Score>();
    gCoordinator.registerComponent<GridMovement>();
    gCoordinator.registerComponent<GridGenerator>();
    gCoordinator.registerComponent<Sound>();
    gCoordinator.registerComponent<Music>();
    gCoordinator.registerComponent<PopAnimation>();
    gCoordinator.registerComponent<KeyboardMovement>();


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
    auto gridMovementSystem = gCoordinator.registerSystem<BubbleGridMovementSystem>();
    auto gridGeneratorSystem = gCoordinator.registerSystem<BubbleGridGeneratorSystem>();
    auto bubbleEventHandlerSystem = gCoordinator.registerSystem<BubbleEventHandlerSystem>();
    auto shooterHandlerSystem = gCoordinator.registerSystem<ShooterHandlerSystem>();
    auto shooterDisableHandlerSystem = gCoordinator.registerSystem<ShooterDisableHandler>();
    auto shooterVerificationSystem = gCoordinator.registerSystem<ShooterVerificationSystem>();
    auto popAnimationSystem = gCoordinator.registerSystem<PopAnimationSystem>();
    auto bubbleMatchHandlerSystem = gCoordinator.registerSystem<BubbleMatchHandler>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto keyboardHandlerSystem = gCoordinator.registerSystem<KeyboardSystem>();
    auto pauseHandlerSystem = gCoordinator.registerSystem<PauseHandlerSystem>();
    auto instructionDisplaySystem = gCoordinator.registerSystem<InstructionDisplaySystem>();
    auto gameOverHandlerSystem = gCoordinator.registerSystem<GameOverHandlerSystem>();
    auto resetHandlerSystem = gCoordinator.registerSystem<ResetHandlerSystem>();

    auto soundSystem = std::make_shared<SoundSystem>();
    auto soundEventHandler = std::make_shared<SoundEventHandler>(soundSystem);


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

    Signature gridMovementSignature;
    gridMovementSignature.set(gCoordinator.getComponentType<GridMovement>());
    gCoordinator.setSystemSignature<BubbleGridMovementSystem>(gridMovementSignature);

    Signature gridGeneratorSignature;
    gridGeneratorSignature.set(gCoordinator.getComponentType<GridGenerator>());
    gCoordinator.setSystemSignature<BubbleGridGeneratorSystem>(gridGeneratorSignature);

    Signature shooterHandlerSignature;
    shooterHandlerSignature.set(gCoordinator.getComponentType<BubbleShooter>());
    gCoordinator.setSystemSignature<ShooterHandlerSystem>(shooterHandlerSignature);

    Signature shooterDisableHandlerSignature;
    shooterDisableHandlerSignature.set(gCoordinator.getComponentType<BubbleShooter>());
    gCoordinator.setSystemSignature<ShooterDisableHandler>(shooterDisableHandlerSignature);

    Signature verificationSignature;
    verificationSignature.set(gCoordinator.getComponentType<BubbleShooter>());
    gCoordinator.setSystemSignature<ShooterVerificationSystem>(verificationSignature);

    Signature popAnimationSignature;
    popAnimationSignature.set(gCoordinator.getComponentType<PopAnimation>());
    popAnimationSignature.set(gCoordinator.getComponentType<Transform>());
    popAnimationSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<PopAnimationSystem>(popAnimationSignature);

    Signature matchHandlerSignature;
    matchHandlerSignature.set(gCoordinator.getComponentType<Bubble>());
    gCoordinator.setSystemSignature<BubbleMatchHandler>(matchHandlerSignature);

    Signature keyboardMovementSignature;
    keyboardMovementSignature.set(gCoordinator.getComponentType<KeyboardMovement>());
    gCoordinator.setSystemSignature<KeyboardMovementSystem>(keyboardMovementSignature);

    zmq::socket_t reply_socket(context, ZMQ_DEALER);
    std::string id = identity + "R";
    reply_socket.set(zmq::sockopt::routing_id, id);
    reply_socket.connect("tcp://localhost:5570");

    // std::thread t1([receiverSystem, &reply_socket, &strategy]() {
    //     while (GameManager::getInstance()->gameRunning) {
    //         receiverSystem->update(reply_socket, strategy.get());
    //     }
    // });

    // init sounds
    soundSystem->loadSound("shoot", "assets/sounds/sfx/shoot.wav");
    soundSystem->loadSound("pop", "assets/sounds/sfx/pop.wav");
    soundSystem->loadSound("match", "assets/sounds/sfx/pop.wav");

    Entity gridGenerator = gCoordinator.createEntity();
    gCoordinator.addComponent(gridGenerator, GridGenerator{
                                  4, // initialRows
                                  1, // rowsPerDrop
                                  16.0f, // bubbleRadius
                                  0.0f, // startX (will be calculated)
                                  32.0f, // startY
                                  8, // maxColumns (reduced for better playability)
                                  {
                                      // availableColors
                                      shade_color::Red,
                                      shade_color::Blue,
                                      shade_color::Green,
                                      shade_color::Yellow,
                                      shade_color::Purple
                                  },
                                  false // needsNewRow
                              });

    Entity gridMovement = gCoordinator.createEntity();
    gCoordinator.addComponent(gridMovement, GridMovement{
                                  15.0f, // dropInterval
                                  0.0f, // currentTime
                                  32.0f, // dropDistance
                                  3.0f, // warningTime
                                  false, // isDropping
                                  64.0f, // dropSpeed
                                  0.0f, // currentDropAmount
                                  false, // showWarning
                                  0.0f // lastWarningToggle
                              });

    const float GRID_SIZE = 32.0f;
    const float GRID_COLS = 15;
    const float GRID_OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * GRID_SIZE)) / 2.0f;
    const float BOUNDARY_WIDTH = 10.0f; // Width of invisible boundary
    const float BOUNDARY_HEIGHT = SCREEN_HEIGHT; // Full height boundary
    // Calculate the effective play area width
    const float PLAY_AREA_LEFT = GRID_OFFSET_X - BOUNDARY_WIDTH - 17.f; // Left boundary x position
    const float PLAY_AREA_RIGHT = GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - 10.f; // Right boundary x position
    const float PLAY_AREA_WIDTH = PLAY_AREA_RIGHT - PLAY_AREA_LEFT;


    Entity shooter = gCoordinator.createEntity();
    const float SHOOTER_CENTER_X = PLAY_AREA_LEFT + (PLAY_AREA_WIDTH / 2.f);
    gCoordinator.addComponent(shooter, Transform{
                                  SHOOTER_CENTER_X, // x position (centered between boundaries)
                                  SCREEN_HEIGHT - 50.f,
                                  32.f,
                                  32.f,
                                  0
                              });
    gCoordinator.addComponent(shooter, BubbleShooter{
                                  2.0f, // rotationSpeed
                                  200.0f, // minAngle (pointing up-left)
                                  340.0f, // maxAngle (pointing up-right)
                                  270.0f, // currentAngle (pointing straight up)
                                  400.f, // shootForce
                                  true, // canShoot
                                  0.5f, // reloadTime
                                  0.0f, // currentReloadTime
                                  false
                              });
    gCoordinator.addComponent(shooter, Color{shade_color::White});
    gCoordinator.addComponent(shooter, ClientEntity{});
    gCoordinator.addComponent(shooter, CKinematic{{0, 0}, 0, {0, 0}, 0}); // Add with initial values
    gCoordinator.addComponent(shooter, Destroy{});
    gCoordinator.addComponent(shooter, KeyboardMovement{});


    // Create left boundary
    Entity leftBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(leftBoundary, Transform{
                                  GRID_OFFSET_X - BOUNDARY_WIDTH - 17.f, // Position just left of grid
                                  10.f, // Top of screen
                                  BOUNDARY_HEIGHT - 20.f, // Height
                                  BOUNDARY_WIDTH, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(leftBoundary, Color{shade_color::White});

    // Create right boundary
    Entity rightBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(rightBoundary, Transform{
                                  GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - 10.f, // Position just right of grid
                                  10.f, // Top of screen
                                  BOUNDARY_HEIGHT - 20.f, // Height
                                  BOUNDARY_WIDTH, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(rightBoundary, Color{shade_color::White}); // Invisible boundary

    Entity topBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(topBoundary, Transform{
                                  GRID_OFFSET_X - BOUNDARY_WIDTH - 17.f, // Position just left of grid
                                  10.f, // Top of screen
                                  BOUNDARY_WIDTH, // Height
                                  GRID_COLS * GRID_SIZE + 2 * BOUNDARY_WIDTH, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(topBoundary, Color{shade_color::White});

    Entity bottomBoundary = gCoordinator.createEntity();
    gCoordinator.addComponent(bottomBoundary, Transform{
                                  GRID_OFFSET_X - BOUNDARY_WIDTH - 17.f, // Position just left of grid
                                  SCREEN_HEIGHT - 10.f, // Bottom of screen
                                  BOUNDARY_WIDTH, // Height
                                  GRID_COLS * GRID_SIZE + 2 * BOUNDARY_WIDTH + 7.f, // Width
                                  0 // No rotation
                              });
    gCoordinator.addComponent(bottomBoundary, Color{shade_color::White});

    gridGeneratorSystem->initializeGrid(gridGenerator);

    Entity scoreEntity = gCoordinator.createEntity();

    Score initialScore{}; // Initialize with default values
    initialScore.value = 0;
    initialScore.multiplier = 1;
    initialScore.bubbleScore = 100;
    initialScore.comboBonus = 50;

    try {
        gCoordinator.addComponent(scoreEntity, initialScore);
        std::cout << "Successfully added Score component to entity " << scoreEntity << std::endl;
    } catch (const std::exception &e) {
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
    // std::thread t2([&client_socket, &clientSystem, &strategy] {
    //     while (GameManager::getInstance()->gameRunning) {
    //         clientSystem->update(client_socket, strategy.get());
    //     }
    // });

    // Start game systems thread
    std::thread gameSystemsThread(gameSystemsUpdate,
                                  kinematicSystem,
                                  bubbleMovementSystem,
                                  destroySystem,
                                  eventSystem,
                                  bubbleShooterSystem,
                                  gridGeneratorSystem,
                                  gridMovementSystem,
                                  popAnimationSystem,
                                  keyboardMovementSystem,
                                  pauseHandlerSystem
    );


    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, engine_constants::FRAME_RATE); // Cap the maximum dt to 60fps

        try {
            // Keep only render-related systems in main thread

            // Destroy system
            destroySystem->update();
            cameraSystem->update(shooter);
            renderSystem->update(mainCamera);
            scoreDisplaySystem->update();
            pauseHandlerSystem->update();
            instructionDisplaySystem->update();
            gameOverHandlerSystem->update();
        } catch (const std::exception &e) {
            std::cerr << "Error in render loop: " << e.what() << std::endl;
        }


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
    send_delete_signal(client_socket, shooter, strategy.get());
    // t1.join();
    // t2.join();

    gameSystemsThread.join(); // Wait for game systems thread to finish
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
