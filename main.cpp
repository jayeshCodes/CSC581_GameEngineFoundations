#include <thread>
#include <memory>

#include "main.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/components/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/client.hpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/collision.cpp"
#include "lib/systems/jump.cpp"

// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);
Timeline gameTimeline(&anchorTimeline, 1);

Coordinator gCoordinator;


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
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
    gCoordinator.registerComponent<Client>();
    gCoordinator.registerComponent<MovingPlatform>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Jump>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();

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

    Signature collisionSignature;
    collisionSignature.set(gCoordinator.getComponentType<Transform>());
    collisionSignature.set(gCoordinator.getComponentType<Collision>());
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    Signature jumpSignature;
    jumpSignature.set(gCoordinator.getComponentType<Transform>());
    jumpSignature.set(gCoordinator.getComponentType<CKinematic>());
    jumpSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<JumpSystem>(jumpSignature);

    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

    Signature keyboardMovementSignature;
    keyboardMovementSignature.set(gCoordinator.getComponentType<Transform>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<CKinematic>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<KeyboardMovement>());
    gCoordinator.setSystemSignature<KeyboardMovementSystem>(keyboardMovementSignature);

    Signature clientSignature;
    clientSignature.set(gCoordinator.getComponentType<Client>());
    gCoordinator.setSystemSignature<ClientSystem>(clientSignature);

    Signature movingPlatformSignature;
    movingPlatformSignature.set(gCoordinator.getComponentType<Transform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<MoveBetween2PointsSystem>(movingPlatformSignature);


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{
                                  SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT
                              });
    // temporary values for viewport width and height

    std::vector<Entity> entities(1);
    // for (auto &entity: entities) {
    //     entity = gCoordinator.createEntity();
    //     gCoordinator.addComponent(entity, Transform{300, 200, 32, 32, 0});
    //     gCoordinator.addComponent(entity, Color{shade_color::Red});
    //     gCoordinator.addComponent(entity, CKinematic{});
    //     gCoordinator.addComponent(entity, Gravity{});
    //     gCoordinator.addComponent(entity, Collision{true, false, CollisionShape::RECTANGLE, 32, 32});
    // }

    auto mainChar = gCoordinator.createEntity();
    gCoordinator.addComponent(mainChar, Transform{0, SCREEN_HEIGHT * 3 / 4.f, 32, 32, 0});
    gCoordinator.addComponent(mainChar, Color{shade_color::Blue});
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, KeyboardMovement{300.f});
    gCoordinator.addComponent(mainChar, Collision{true, false, CollisionShape::RECTANGLE, 32, 32});
    gCoordinator.addComponent(mainChar, Gravity{});
    gCoordinator.addComponent(mainChar, Jump{0.5f, 1.f, 0.3f, false, 0.0f, true, 0.2f});

    auto entity2 = gCoordinator.createEntity();
    gCoordinator.addComponent(entity2, Transform{-500.f, SCREEN_HEIGHT, SCREEN_HEIGHT, SCREEN_WIDTH * 5, 0});
    gCoordinator.addComponent(entity2, Color{shade_color::Brown});
    // gCoordinator.addComponent(entity2, Collision{true, false, CollisionShape::RECTANGLE, SCREEN_WIDTH * 5, 32});

    auto bigblackblock = gCoordinator.createEntity();
    gCoordinator.addComponent(bigblackblock, Transform{
                                  0, SCREEN_HEIGHT * 3 / 4.f + 100.f, SCREEN_WIDTH / 100.f, SCREEN_WIDTH, 0
                              });
    gCoordinator.addComponent(bigblackblock, Color{shade_color::Black});
    gCoordinator.addComponent(bigblackblock, CKinematic{});
    gCoordinator.addComponent(bigblackblock, Collision{true, false, CollisionShape::RECTANGLE, SCREEN_WIDTH, SCREEN_WIDTH / 100.f});

    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Client{7000, 7001});

    zmq::context_t context(1);
    clientSystem->initialize(context);

    std::thread send_msg_thread([&clientSystem]() {
        while (GameManager::getInstance()->gameRunning) { clientSystem->receive_message(); }
    });

    auto last_time = gameTimeline.getElapsedTime();
    for (int i = 0; i < argc; ++i) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    }

    clientSystem->connect_server(std::stof(argv[1]));

    auto movingPlatformEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(movingPlatformEntity, Transform{100, 100, 64, 64, 0});
    gCoordinator.addComponent(movingPlatformEntity, MovingPlatform{100, 500, MovementState::LEFT, 2});
    gCoordinator.addComponent(movingPlatformEntity, CKinematic{});
    gCoordinator.addComponent(movingPlatformEntity, Color{shade_color::Brown});
    gCoordinator.addComponent(movingPlatformEntity, Collision{true, false, CollisionShape::RECTANGLE, 64, 64});

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;

        gravitySystem->update(dt);
        kinematicSystem->update(dt);
        keyboardMovementSystem->update(dt);
        cameraSystem->update(dt);
        moveBetween2PointsSystem->update(dt, gameTimeline);
        collisionSystem->update(dt);
        jumpSystem->update(dt);

        auto main_camera = cameraSystem->getMainCamera();
        auto transform = gCoordinator.getComponent<Transform>(mainChar);
        renderSystem->update(*main_camera, transform.x, transform.y);

        presentScene();
    }

    send_msg_thread.join();

    // Create 4 Rectangle instances
    clientSystem->disconnect();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
