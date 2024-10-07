#include <thread>
#include <memory>

#include "main.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/components/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/thread_pool/thread_pool.hpp"

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
    ThreadPool threadPool(std::thread::hardware_concurrency());

    anchorTimeline.start();
    gameTimeline.start();

    gCoordinator.init();
    gCoordinator.registerComponent<Transform>();
    gCoordinator.registerComponent<Color>();
    gCoordinator.registerComponent<CKinematic>();
    gCoordinator.registerComponent<Camera>();
    gCoordinator.registerComponent<Gravity>();
    gCoordinator.registerComponent<KeyboardMovement>();

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();

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
    gCoordinator.setSystemSignature<KeyboardMovementSystem>(keyboardMovementSignature);


    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{
                                  SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT
                              });
    // temporary values for viewport width and height

    std::vector<Entity> entities(1);
    for (auto &entity: entities) {
        entity = gCoordinator.createEntity();
        gCoordinator.addComponent(entity, Transform{300, 200, 32, 32, 0});
        gCoordinator.addComponent(entity, Color{shade_color::Red});
        gCoordinator.addComponent(entity, CKinematic{});
        gCoordinator.addComponent(entity, Gravity{});
    }

    auto mainChar = gCoordinator.createEntity();
    gCoordinator.addComponent(mainChar, Transform{SCREEN_WIDTH / 2.f, SCREEN_HEIGHT * 3 / 4.f, 32, 32, 0});
    gCoordinator.addComponent(mainChar, Color{shade_color::Blue});
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, KeyboardMovement{300.f});

    auto entity2 = gCoordinator.createEntity();
    gCoordinator.addComponent(entity2, Transform{300, SCREEN_HEIGHT, 32, SCREEN_WIDTH * 5, 0});
    gCoordinator.addComponent(entity2, Color{shade_color::Black});

    auto last_time = gameTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;

        threadPool.enqueue([&] { gravitySystem->update(dt); });
        threadPool.enqueue([&] { kinematicSystem->update(dt); });
        threadPool.enqueue([&] { keyboardMovementSystem->update(dt); });
        cameraSystem->update(dt);

        auto main_camera = cameraSystem->getMainCamera();
        auto transform = gCoordinator.getComponent<Transform>(mainChar);
        renderSystem->update(*main_camera, transform.x, transform.y);

        presentScene();
    }

    // Create 4 Rectangle instances


    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
