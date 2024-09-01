//
// Created by Utsav Lal on 8/28/24.
//

#include "object.hpp"

#include "../game/GameManager.hpp"
#include "shapes/rectangle.hpp"

Object::Object(SDL_FRect rect, SDL_Color color, bool rigid, float mass, float restitution) {
    this->rect = rect;
    this->color = color;
    this->velocity = {0, 0};
    this->acceleration = {0, 0};
    this->orientation = 0;
    this->rotation = 0;
    this->angular_acceleration = 0;
    this->is_rigidbody = rigid;
    this->mass = mass;
    this->restitution = restitution;
}

void Object::update(float dt) {
    rotation += angular_acceleration * dt;

    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;

    rect.x += velocity.x * dt;
    rect.y += velocity.y * dt;

    orientation += rotation * dt;

    if (rect.y < 0) {
        rect.y = 0;
        velocity.y = 0; // Stop vertical movement if at the top
    } else if (rect.y + rect.h > SCREEN_HEIGHT) {
        // `rect.h` is the height of the object
        rect.y = SCREEN_HEIGHT - rect.h;
        velocity.y = 0; // Stop vertical movement if at the bottom
    }
    if (rect.x < 0) {
        rect.x = 0;
        velocity.x = 0;
    } else if (rect.x + rect.w > SCREEN_WIDTH) {
        rect.x = SCREEN_WIDTH - rect.w;
        velocity.x = 0;
    }
}

void Object::scale(SDL_FRect &r) const {
    if (GameManager::getInstance()->scaleWithScreenSize) {
        int cwWidth, cwHeight;
        SDL_GetWindowSizeInPixels(app->window, &cwWidth, &cwHeight);
        SDL_FPoint scalingFactor = {
            static_cast<float>(cwWidth) / SCREEN_WIDTH, static_cast<float>(cwHeight) / SCREEN_HEIGHT
        };
        float newX = r.x * scalingFactor.x;
        float newY = r.y * scalingFactor.y;
        float newWidth = r.w * scalingFactor.x;
        float newHeight = r.h * scalingFactor.y;
        r = {newX, newY, newWidth, newHeight};
    }
}


Object::~Object() = default;
