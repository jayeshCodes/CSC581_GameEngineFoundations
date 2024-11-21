//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef BUBBLE_MATCH_HANDLER_HPP
#define BUBBLE_MATCH_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include <algorithm>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleMatchHandler : public System {
private:
    const int BASE_SCORE = 100;
    const int COMBO_MULTIPLIER = 50;
    int consecutiveMatches = 0;
    int64_t lastMatchTime = 0;
    static constexpr int64_t COMBO_WINDOW = 1000;

    void updateScore(size_t matchSize) {
        auto scoreEntities = gCoordinator.getEntitiesWithComponent<Score>();
        if (scoreEntities.empty()) return;

        Entity scoreEntity = scoreEntities[0];
        auto& score = gCoordinator.getComponent<Score>(scoreEntity);

        auto currentTime = eventTimeline.getElapsedTime();
        if (currentTime - lastMatchTime < COMBO_WINDOW) {
            consecutiveMatches++;
            score.multiplier = std::min(consecutiveMatches, 5);
        } else {
            consecutiveMatches = 1;
            score.multiplier = 1;
        }
        lastMatchTime = currentTime;

        int matchPoints = matchSize * BASE_SCORE;
        int comboBonus = (consecutiveMatches > 1) ? COMBO_MULTIPLIER * (consecutiveMatches - 1) : 0;
        score.value += (matchPoints + comboBonus) * score.multiplier;
    }

    EventHandler matchHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type != eventTypeToString(EventType::BubbleMatch)) return;

        std::cout << "Processing bubble match event" << std::endl;
        const BubbleMatchData& data = event->data;

        // Verify all matched bubbles still exist
        std::vector<Entity> validMatches;
        for (Entity bubble : data.matches) {
            if (gCoordinator.hasComponent<Transform>(bubble) &&
                gCoordinator.hasComponent<Color>(bubble) &&
                gCoordinator.hasComponent<Bubble>(bubble)) {
                validMatches.push_back(bubble);
            }
        }

        if (validMatches.empty()) return;

        std::cout << "Found " << validMatches.size() << " valid matches to destroy" << std::endl;
        updateScore(validMatches.size());

        // Mark bubbles for destruction
        for (Entity bubble : validMatches) {
            if (!gCoordinator.hasComponent<Destroy>(bubble)) {
                gCoordinator.addComponent(bubble, Destroy{});
            }
            auto& destroy = gCoordinator.getComponent<Destroy>(bubble);
            destroy.destroy = true;

            // Clear grid position
            if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                auto& bubbleComp = gCoordinator.getComponent<Bubble>(bubble);
                gridSystem->clearGridPosition(bubbleComp.row, bubbleComp.col);
            }
        }

        // Emit events
        Event floatingEvent{eventTypeToString(EventType::CheckFloatingBubbles), {}};
        eventCoordinator.emit(std::make_shared<Event>(floatingEvent));

        // Event soundEvent{eventTypeToString(EventType::BubblePopped),
        //     PopSoundData{static_cast<int>(validMatches.size())}};
        // eventCoordinator.emit(std::make_shared<Event>(soundEvent));
    };

public:
    BubbleMatchHandler() {
        eventCoordinator.subscribe(matchHandler, eventTypeToString(EventType::BubbleMatch));
    }

    ~BubbleMatchHandler() {
        eventCoordinator.unsubscribe(matchHandler, eventTypeToString(EventType::BubbleMatch));
    }
};

#endif //BUBBLE_MATCH_HANDLER_HPP