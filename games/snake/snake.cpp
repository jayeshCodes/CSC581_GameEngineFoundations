#include <csignal>
#include <memory>
#include <thread>


#include "../../main.hpp"
#include "../../lib/core/timeline.hpp"
#include "../../lib/ECS/coordinator.hpp"
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
#include "../../lib/systems/render.cpp"
#include "../../lib/strategy/send_strategy.hpp"
#include "../../lib/systems/event_system.hpp"
#include "../../lib/systems/position_update_handler.hpp"
#include "handlers/food_handler.hpp"
#include "handlers/keyboard_handler.hpp"
#include "helpers/quantizer.hpp"
#include "model/components.hpp"
#include "systems/client.hpp"
#include "systems/keyboard_movement.hpp"
#include "systems/move.hpp"
#include "systems/receiver.hpp"
#include "handlers/collision_handler.hpp"

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


int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;

    screen_height = 640;
    screen_width = 640;

    initSDL(screen_width, screen_height);
    GameManager::getInstance()->gameRunning = true;
    catch_signals();

    std::unique_ptr<Send_Strategy> strategy = std::make_unique<JSON_Strategy>();

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
    gCoordinator.registerComponent<KeyboardMovement>();
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Snake>();
    gCoordinator.registerComponent<Map>();
    gCoordinator.registerComponent<Food>();

    // Systems
    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();
    auto eventSystem = gCoordinator.registerSystem<EventSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveSystem = gCoordinator.registerSystem<MoveSystem>();


    // Handlers
    auto positionUpdateHandler = gCoordinator.registerSystem<PositionUpdateHandler>();
    auto keyboardHandler = gCoordinator.registerSystem<KeyboardHandler>();
    auto foodHandler = gCoordinator.registerSystem<FoodHandler>();
    auto collisionHandler = gCoordinator.registerSystem<CollisionHandler>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature moveSig;
    moveSig.set(gCoordinator.getComponentType<Transform>());
    moveSig.set(gCoordinator.getComponentType<Snake>());
    gCoordinator.setSystemSignature<MoveSystem>(moveSig);


    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

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

    auto clientEntity = gCoordinator.createEntity();
    gCoordinator.addComponent(clientEntity, Receiver{});

    auto last_time = gameTimeline.getElapsedTime();


    // Start the message sending thread
    std::thread t2([&client_socket, &clientSystem, &strategy] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket, strategy.get());
        }
    });

    const int length = 20;
    const int rows = screen_height / length;
    const int cols = screen_width / length;

    const auto startPos = SnakeQuantizer::dequantize(rows / 2, cols / 2, length);

    Entity player = gCoordinator.createEntity();
    gCoordinator.addComponent(player, Transform{startPos[0], startPos[1], length, length, 0, 1});
    gCoordinator.addComponent(player, Color{shade_color::Black});
    gCoordinator.addComponent(player, CKinematic{SDL_FPoint{150, 0}});
    gCoordinator.addComponent(player, Collision{true, false, CollisionLayer::PLAYER});
    gCoordinator.addComponent(player, Destroy{});
    gCoordinator.addComponent(player, Snake{length});

    Event startEvent{eventTypeToString(GameEvents::EventType::GameStart), GameEvents::GameStartData{length}};
    eventCoordinator.emit(std::make_shared<Event>(startEvent));

    std::thread t3([&collisionSystem, &destroySystem] {
        auto current_time = gameTimeline.getElapsedTime();
        while (GameManager::getInstance()->gameRunning) {
            collisionSystem->update();
            destroySystem->update();

            auto elapsed_time = gameTimeline.getElapsedTime();
            auto time_to_sleep = engine_constants::FRAME_RATE - (elapsed_time - current_time); // Ensure float division
            if (time_to_sleep > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
            }
        }
    });

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene(shade_color::White);

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, 1 / 20.f); // Cap the maximum dt to 60fps

        // kinematicSystem->update(dt);

        cameraSystem->update(INVALID_ENTITY);
        renderSystem->update(INVALID_ENTITY);
        eventSystem->update();
        keyboardMovementSystem->update();
        moveSystem->update(dt);

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = engine_constants::FRAME_RATE - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }

        presentScene();
    }


    t1.join();
    t2.join();
    t3.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
