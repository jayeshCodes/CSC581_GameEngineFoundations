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
#include "lib/systems/combo_event_handler.hpp"
#include "lib/systems/dash.hpp"
#include "lib/systems/entity_created_handler.hpp"
#include "lib/systems/position_update_handler.hpp"
#include "lib/systems/replay_handler.hpp"
#include "lib/systems/vertical_boost_handler.hpp"
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <limits.h>  // for PATH_MAX

#include "lib/systems/monster_spawner.hpp"
#include "lib/systems/platform_spawner.hpp"
#include "lib/systems/projectile_handler.hpp"
#include "lib/systems/score.hpp"
#include "lib/systems/score_display.hpp"
#include "lib/systems/shoot.hpp"
#include "lib/systems/title_text.hpp"

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

std::atomic<bool> physicsRunning{true};

void physicsUpdate(Timeline &gameTimeline,
                   std::shared_ptr<KinematicSystem> kinematicSystem,
                   std::shared_ptr<JumpSystem> jumpSystem,
                   std::shared_ptr<GravitySystem> gravitySystem,
                   std::shared_ptr<CollisionSystem> collisionSystem,
                   std::shared_ptr<DeathSystem> deathSystem,
                   std::shared_ptr<DashSystem> dashSystem,
                   std::shared_ptr<ShooterSystem> shooterSystem,
                   std::shared_ptr<KeyboardMovementSystem> keyboardMovementSystem,
                   std::shared_ptr<ProjectileHandlerSystem> projectileHandlerSystem) {
    const float FIXED_TIMESTEP = 1.0f / 120.0f; // 120 Hz physics updates
    int64_t lastTime = gameTimeline.getElapsedTime();

    while (physicsRunning && GameManager::getInstance()->gameRunning) {
        int64_t currentTime = gameTimeline.getElapsedTime();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        dt = std::min(dt, FIXED_TIMESTEP); // Prevent large time steps

        // Update physics systems
        if (!gameTimeline.isPaused()) {
            keyboardMovementSystem->update();
            kinematicSystem->update(dt);
            jumpSystem->update(dt);
            gravitySystem->update(dt);
            collisionSystem->update();
            deathSystem->update();
            dashSystem->update(dt);
            shooterSystem->update(dt);
            projectileHandlerSystem->update(dt);
        }

        // Sleep to maintain consistent update rate
        std::this_thread::sleep_for(std::chrono::microseconds(8333)); // ~120 Hz
    }
}


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    GameManager::getInstance()->gameRunning = true;
    TextureManager::getInstance()->init(app->renderer);
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
    // monsterTimeline.start();

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
    gCoordinator.registerComponent<Respawnable>();
    gCoordinator.registerComponent<RigidBody>();
    gCoordinator.registerComponent<Dash>();
    gCoordinator.registerComponent<Stomp>();
    gCoordinator.registerComponent<VerticalBoost>();
    gCoordinator.registerComponent<Sprite>();
    gCoordinator.registerComponent<IntroScreen>();
    gCoordinator.registerComponent<PlatformSpawner>();
    gCoordinator.registerComponent<Shooter>();
    gCoordinator.registerComponent<Projectile>();
    gCoordinator.registerComponent<Score>();


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
    auto triggerHandlerSystem = gCoordinator.registerSystem<VerticalBoostHandler>();
    auto eventSystem = gCoordinator.registerSystem<EventSystem>();
    auto entityCreatedSystem = gCoordinator.registerSystem<EntityCreatedHandler>();
    auto positionUpdateHandler = gCoordinator.registerSystem<PositionUpdateHandler>();
    auto dashSystem = gCoordinator.registerSystem<DashSystem>();
    auto comboEventHandler = gCoordinator.registerSystem<ComboEventHandler>();
    auto replayHandler = gCoordinator.registerSystem<ReplayHandler>();
    auto introScreenSystem = gCoordinator.registerSystem<IntroScreenSystem>();
    auto platformSpawnerSystem = gCoordinator.registerSystem<PlatformSpawnerSystem>();
    auto shooterSystem = gCoordinator.registerSystem<ShooterSystem>();
    auto projectileHandlerSystem = gCoordinator.registerSystem<ProjectileHandlerSystem>();
    auto scoreSystem = gCoordinator.registerSystem<ScoreSystem>();
    auto scoreDisplaySystem = gCoordinator.registerSystem<ScoreDisplaySystem>();
    auto monsterSpawnerSystem = gCoordinator.registerSystem<MonsterSpawnerSystem>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>(), false);
    renderSignature.set(gCoordinator.getComponentType<Sprite>(), false);
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
    keyboardMovementSignature.set(gCoordinator.getComponentType<Jump>(), false);
    keyboardMovementSignature.set(gCoordinator.getComponentType<Dash>(), false);
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

    Signature dashSignature;
    dashSignature.set(gCoordinator.getComponentType<Dash>());
    dashSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<DashSystem>(dashSignature);

    // Set up signature for intro screen system
    Signature introScreenSignature;
    introScreenSignature.set(gCoordinator.getComponentType<IntroScreen>());
    gCoordinator.setSystemSignature<IntroScreenSystem>(introScreenSignature);

    Signature platformSpawnerSignature;
    platformSpawnerSignature.set(gCoordinator.getComponentType<PlatformSpawner>());
    gCoordinator.setSystemSignature<PlatformSpawnerSystem>(platformSpawnerSignature);

    Signature shooterSignature;
    shooterSignature.set(gCoordinator.getComponentType<Shooter>());
    shooterSignature.set(gCoordinator.getComponentType<Transform>());
    gCoordinator.setSystemSignature<ShooterSystem>(shooterSignature);

    Signature scoreSignature;
    scoreSignature.set(gCoordinator.getComponentType<Score>());
    scoreSignature.set(gCoordinator.getComponentType<Transform>());
    gCoordinator.setSystemSignature<ScoreSystem>(scoreSignature);

    Signature scoreDisplaySignature;
    scoreDisplaySignature.set(gCoordinator.getComponentType<Score>());
    gCoordinator.setSystemSignature<ScoreDisplaySystem>(scoreDisplaySignature);

    Signature monsterSpawnerSignature;
    gCoordinator.setSystemSignature<MonsterSpawnerSystem>(monsterSpawnerSignature);

    zmq::socket_t reply_socket(context, ZMQ_DEALER);
    std::string id = identity + "R";
    reply_socket.set(zmq::sockopt::routing_id, id);
    reply_socket.connect("tcp://localhost:5570");

    std::thread t1([receiverSystem, &reply_socket, &strategy]() {
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(reply_socket, strategy.get());
        }
    });

    Entity introEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(introEntity, IntroScreen{});

    Entity spawner = gCoordinator.createEntity();
    gCoordinator.addComponent(spawner, PlatformSpawner{});

    platformSpawnerSystem->init();
    monsterSpawnerSystem->init();


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{0, 0, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT});
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    std::string path = "assets/images/moodle.png";

    SDL_Texture *tempTexture = TextureManager::getInstance()->loadTexture("assets/images/moodle.png");
    int texWidth, texHeight;
    SDL_QueryTexture(tempTexture, nullptr, nullptr, &texWidth, &texHeight);
    std::cout << "Loaded texture dimensions: " << texWidth << "x" << texHeight << std::endl;

    // Calculate a good base size for the character (e.g., 1/8th of screen height)
    float desiredHeight = SCREEN_HEIGHT / 11.0f; // Adjust this ratio to make character bigger/smaller
    float scale = desiredHeight / texHeight; // Calculate scale to achieve desired height

    Sprite sprite;
    sprite.texturePath = "assets/images/moodle.png";
    sprite.srcRect = {0, 0, texWidth, texHeight};
    sprite.scale = scale;
    sprite.origin = {0, 0};
    sprite.flipX = false;
    sprite.flipY = false;

    auto mainChar = gCoordinator.createEntity();

    float aspectRatio = (float) texWidth / texHeight;
    float desiredWidth = desiredHeight * aspectRatio;

    // Set transform dimensions to these exact values
    gCoordinator.addComponent(mainChar, Transform{
                                  SCREEN_WIDTH / 2, // center horizontally
                                  SCREEN_HEIGHT - desiredHeight - 240.0f, // place above ground
                                  desiredHeight, // height
                                  desiredWidth, // width
                                  0 // orientation
                              });
    gCoordinator.addComponent(mainChar, sprite);
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, KeyboardMovement{200.f});
    gCoordinator.addComponent(mainChar, ClientEntity{0, false});
    gCoordinator.addComponent(mainChar, Destroy{});
    // gCoordinator.addComponent(mainChar, Jump{50.f, 1.f, true, 0.0f, true, 120.f});
    gCoordinator.addComponent(mainChar, Gravity{0, 1000});
    gCoordinator.addComponent(mainChar, Respawnable{
                                  {0, SCREEN_HEIGHT - 200.f, 32, 32, 0, 1}, false
                              });
    gCoordinator.addComponent(mainChar, RigidBody{1.f});
    gCoordinator.addComponent(mainChar, Collision{true, false, CollisionLayer::PLAYER});
    gCoordinator.addComponent(mainChar, Shooter{true, 0.5f, 0.0f});
    gCoordinator.addComponent(mainChar, Score{0, SCREEN_HEIGHT - desiredHeight - 240.0f});
    std::cout << "MainChar: " << gCoordinator.getEntityKey(mainChar) << std::endl;
    mainCharID = gCoordinator.getEntityKey(mainChar);

    SDL_Texture *tempTexture2 = TextureManager::getInstance()->loadTexture("assets/images/platform.png");
    SDL_QueryTexture(tempTexture2, nullptr, nullptr, &texWidth, &texHeight);


    Sprite platformSprite;
    platformSprite.texturePath = "assets/images/platform.png";
    platformSprite.srcRect = {0, 0, texWidth, texHeight};
    platformSprite.scale = 1.0f;
    platformSprite.origin = {0, 0};
    platformSprite.flipX = false;
    platformSprite.flipY = false;
    float desiredPlatformHeight = SCREEN_HEIGHT / 8.0f; // Adjust this ratio to make character bigger/smaller
    float platformScale = desiredPlatformHeight / texHeight; // Calculate scale to achieve desired height
    float aspectRatio2 = (float) texWidth / texHeight;
    float desiredPlatformWidth = desiredPlatformHeight * aspectRatio2;


    // init platform for start screen
    auto platform = gCoordinator.createEntity();
    gCoordinator.addComponent(platform, Transform{
                                  SCREEN_WIDTH / 2.f, // center horizontally
                                  SCREEN_HEIGHT - 30.f, // place above ground
                                  50.f, // height
                                  120.f, // width
                                  0 // orientation
                              });
    gCoordinator.addComponent(platform, platformSprite);
    gCoordinator.addComponent(platform, CKinematic{});
    gCoordinator.addComponent(platform, ClientEntity{0, false});
    gCoordinator.addComponent(platform, Collision{false, true, CollisionLayer::OTHER});
    gCoordinator.addComponent(platform, VerticalBoost{});
    gCoordinator.addComponent(platform, RigidBody{-1.f});
    gCoordinator.addComponent(platform, Destroy{});


    Event entityCreatedEvent{eventTypeToString(MainCharCreated), {}};
    entityCreatedEvent.data = MainCharCreatedData{mainChar, strategy->get_message(mainChar, Message::CREATE)};
    eventCoordinator.emitServer(client_socket, std::make_shared<Event>(entityCreatedEvent));
    gCoordinator.getComponent<ClientEntity>(mainChar).synced = true;

    // After creating mainChar and adding all components
    std::cout << "Is mainChar in render system: " <<
            (renderSystem->entities.find(mainChar) != renderSystem->entities.end()) << std::endl;


    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();


    // Start the message sending thread
    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    std::thread physicsThread(physicsUpdate,
                              std::ref(gameTimeline),
                              kinematicSystem,
                              jumpSystem,
                              gravitySystem,
                              collisionSystem,
                              deathSystem,
                              dashSystem,
                              shooterSystem,
                              keyboardMovementSystem,
                              projectileHandlerSystem);

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, engine_constants::FRAME_RATE); // Cap the maximum dt to 60fps

        static int updateCounter = 0;
        if (++updateCounter >= 6 && introScreenSystem->hasGameStarted()) {
            // Update every 6th frame
            platformSpawnerSystem->update(mainCamera);
            monsterSpawnerSystem->update(mainCamera);
            updateCounter = 0;
        }
        scoreSystem->update();
        scoreDisplaySystem->update();
        moveBetween2PointsSystem->update(dt, monsterTimeline);
        cameraSystem->update(mainChar);
        renderSystem->update(mainCamera);
        eventSystem->update();
        replayHandler->update();

        if (!introScreenSystem->hasGameStarted()) {
            introScreenSystem->update();
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
    send_delete_signal(client_socket, mainChar, strategy.get());
    t1.join();
    t2.join();
    physicsRunning = false;
    physicsThread.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
