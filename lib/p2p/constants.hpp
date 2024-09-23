//
// Created by Jayesh Gajbhar on 9/22/24.
//

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

// Existing message types
#define SYNC "SYNC"
#define SYNC_ACK "SYNC_ACK"
#define DESYNC "DESYNC"
#define DESYNC_ACK "DESYNC_ACK"
#define ACK "ACK"
#define RETRY "RETRY"
#define MESSAGE "message"

// Existing socket ports
#define PUSH_PULL_SOCKET 5555
#define PUB_SUB_SOCKET 5556

// New socket ports for peer-to-peer networking
#define PEER_PUB_PORT 5557
#define PEER_PUB_PORT_2 5559
#define REP_REQ_SOCKET 5558

// New message types for peer-to-peer communication
// #define PEER_DISCOVERY "PEER_DISCOVERY"
// #define PEER_LIST "PEER_LIST"
#define PLAYER_POSITION "PLAYER_POSITION"
#define PLATFORM_POSITION "PLATFORM_POSITION"

// Frame rate constant (if not already defined elsewhere)
const float FRAME_RATE = 1000.0f / 60.0f;

// Enum for message types (if not already defined elsewhere)
enum MessageType {
    CHARACTER,
    PLATFORM,
    PEER_DISCOVERY,
    PEER_LIST
};

#endif //CONSTANTS_HPP
