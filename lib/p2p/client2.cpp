//
// Created by Jayesh Gajbhar on 9/23/24.
//

#include "client2.hpp"

const SDL_Color blueColor = {0, 210, 255, 255};
const SDL_Color redColor = {255, 0, 0, 255};


int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    gameRunning = true;

    // Create Rectangle instance

    // Launching basic systems of Shade Engine
    KeyMovement key_movement(500, 500);

    //  init timer
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();

    int64_t lastTime = gameTimeline.getElapsedTime();

    zmq::context_t context(1);
    zmq::socket_t server_sub_socket(context, ZMQ_SUB);
    zmq::socket_t peer_pub_socket(context, ZMQ_PUB);
    zmq::socket_t peer_sub_socket(context, ZMQ_SUB);
    zmq::socket_t discovery_req_socket(context, ZMQ_REQ);

    server_sub_socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));
    server_sub_socket.set(zmq::sockopt::subscribe, "");

    std::string own_address = "tcp://*:" + std::to_string(PEER_PUB_PORT_2);
    peer_pub_socket.bind(own_address);
    // peer_pub_socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));  // Connect to the server's PUB socket

    auto player = Factory::createRectangle(blueColor, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 3.f, 100, 100}, false,
                                           1, 1);
    auto platform = Factory::createRectangle(redColor, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100}, true,
                                             100000.f, 0.8);

    // other players if any
    std::vector<std::unique_ptr<Rectangle>> otherPlayers;

    std::thread server_receive_thread(receive_server_messages, std::ref(server_sub_socket), std::ref(platform));
    std::thread player_send_thread(send_player_position, std::ref(peer_pub_socket), std::ref(player));
    std::thread peer_receive_thread(receive_peer_messages, std::ref(peer_sub_socket), std::ref(otherPlayers));

    const float FRAME_RATE_LIMIT = 1000.f / 120.0f;


    while (gameRunning) {
        Uint32 currentTime = gameTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;

        if (deltaTime > FRAME_RATE_LIMIT) {
            deltaTime = FRAME_RATE_LIMIT;
        }

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        SDL_FPoint direction = getKeyPress();

        key_movement.calculate(*player, direction);

        // std::string message = std::to_string(movementRect->rect.x) + " " + std::to_string(movementRect->rect.y);
        std::array<float, 2> positions = {player->rect.x, player->rect.y};
        // messageQueue.enqueue({MessageType::CHARACTER, positions[0], positions[1]});

        player->draw();
        platform->draw();
        for (const auto &otherPlayer : otherPlayers) {
            otherPlayer->draw();
        }

        player->update(deltaTime);
        //Present the resulting scene
        presentScene();

        if (deltaTime < FRAME_RATE_LIMIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE_LIMIT - deltaTime)));
        }
    }
    // netThread.join();
    server_receive_thread.join();
    player_send_thread.join();
    peer_receive_thread.join();

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
