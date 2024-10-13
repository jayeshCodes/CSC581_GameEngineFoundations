//
// Created by Utsav Lal on 9/22/24.
//

#ifndef MESSAGE_TYPE_HPP
#define MESSAGE_TYPE_HPP

enum MessageType {
    CHAR_1,
    CHAR_2,
    CHAR_3,
    PLATFORM
};

enum P2PMessageType {
    P2P_CHARACTER_1,
    P2P_CHARACTER_2,
    P2P_CHARACTER_3,
    P2P_PLATFORM
};

enum Message {
    CONNECT,
    DISCONNECT,
    CREATE,
    DESTROY,
    ATTACH,
    POSITION,
    UPDATE,
    END,
    CONNECTED,
    DISCONNECTED
};

#endif //MESSAGE_TYPE_HPP
