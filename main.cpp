#include <thread>
#include <memory>

#include "main.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/components/components.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/render.cpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/camera.cpp"

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

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();

    Signature signature;
    signature.set(gCoordinator.getComponentType<Transform>());
    signature.set(gCoordinator.getComponentType<Color>());
    signature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<RenderSystem>(signature);
    gCoordinator.setSystemSignature<KinematicSystem>(signature);
    gCoordinator.setSystemSignature<GravitySystem>(signature);

    // Set up the signature for the camera system
    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

    std::vector<Entity> entities(1);

    for (auto &entity: entities) {
        entity = gCoordinator.createEntity();
        gCoordinator.addComponent(entity, Transform{SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 32, 32, 0});
        gCoordinator.addComponent(entity, Color{255, 0, 0, 255});
        gCoordinator.addComponent(entity, CKinematic{});
    }

    Entity mainCamera = gCoordinator.createEntity();
    gCoordinator.addComponent(mainCamera, Camera{SCREEN_WIDTH/2.f, SCREEN_HEIGHT/2.f, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT});
    // temporary values for viewport width and height

    auto last_time = gameTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f;
        last_time = current_time;

        gravitySystem->update(dt);
        kinematicSystem->update(dt);
        cameraSystem->update(dt);

        Camera *mainCam = cameraSystem->getMainCamera();
        if (mainCam) {
            renderSystem->update(*mainCam);
        } else {
            renderSystem->update();
        }

        presentScene();
    }

    // Create 4 Rectangle instances


    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
