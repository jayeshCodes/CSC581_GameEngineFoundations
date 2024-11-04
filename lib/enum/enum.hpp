//
// Created by Utsav Lal on 9/22/24.
//

#ifndef MESSAGE_TYPE_HPP
#define MESSAGE_TYPE_HPP


enum Message {
    UPDATE,
    DESTROY,
    SYNC,
    DELETE,
    CREATE
};

enum MovementState {
    TO,
    FRO,
    STOP
};

enum MovementType {
    HORIZONTAL,
    VERTICAL
};

#endif //MESSAGE_TYPE_HPP
