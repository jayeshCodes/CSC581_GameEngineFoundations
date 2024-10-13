////
//// Created by Jayesh Gajbhar on 8/29/24.
////
//
//#include "collision.hpp"
//#include <cmath>
//
//void Collision::calculate(Object &character, std::vector<std::unique_ptr<Rectangle> > &objects) {
//    for (auto &obj: objects) {
//        if (&character != obj.get() && obj != nullptr && character.is_rigidbody && obj->is_rigidbody &&
//            SDL_HasIntersectionF(&obj->rect, &character.rect)) {
//            // Calculate collision normal
//            SDL_FPoint normal = {obj->rect.x - character.rect.x, obj->rect.y - character.rect.y};
//            float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
//            normal.x /= length;
//            normal.y /= length;
//
//            // Calculate penetration depth
//            float overlapX = (character.rect.x + character.rect.w) - obj->rect.x;
//            float overlapY = (character.rect.y + character.rect.h) - obj->rect.y;
//            float penetrationDepth = std::min(overlapX, overlapY);
//
//            // Adjust positions to resolve penetration
//            if (penetrationDepth > 0) {
//                character.rect.x -= normal.x * penetrationDepth / 2;
//                character.rect.y -= normal.y * penetrationDepth / 2;
//                obj->rect.x += normal.x * penetrationDepth / 2;
//                obj->rect.y += normal.y * penetrationDepth / 2;
//            }
//
//            // Relative velocity
//            SDL_FPoint relativeVelocity = {
//                obj->velocity.x - character.velocity.x, obj->velocity.y - character.velocity.y
//            };
//            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
//
//            // Coefficient of restitution
//            float e = std::min(character.restitution, obj->restitution);
//
//            // Calculate impulse scalar
//            float j = -(1 + e) * velocityAlongNormal;
//            j /= (1 / character.mass + 1 / obj->mass);
//
//            // Apply impulse to both objects
//            SDL_FPoint impulse = {j * normal.x, j * normal.y};
//            character.velocity.x -= impulse.x / character.mass;
//            character.velocity.y -= impulse.y / character.mass;
//            obj->velocity.x += impulse.x / obj->mass;
//            obj->velocity.y += impulse.y / obj->mass;
//
//            // Apply damping
//            const float damping = 0.7f;
//            character.velocity.x *= damping;
//            character.velocity.y *= damping;
//            obj->velocity.x *= damping;
//            obj->velocity.y *= damping;
//        }
//    }
//}
