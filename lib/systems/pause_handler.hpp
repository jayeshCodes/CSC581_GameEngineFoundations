//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef PAUSE_HANDLER_HPP
#define PAUSE_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../helpers/font_helper.hpp"
#include <string>
#include <mutex>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern Timeline gameTimeline;
extern Timeline eventTimeline;

class PauseHandlerSystem : public System {
private:
    std::mutex pauseMutex;
    bool shouldDisplayPause = false;

    EventHandler pauseHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::Pause)) {
            std::lock_guard<std::mutex> lock(pauseMutex);
            if (!gameTimeline.isPaused()) {
                shouldDisplayPause = true;
                gameTimeline.pause();
            } else {
                gameTimeline.start();
                shouldDisplayPause = false;
            }
        }
    };
public:
    PauseHandlerSystem() {
        eventCoordinator.subscribe(pauseHandler, eventTypeToString(EventType::Pause));
    }

    ~PauseHandlerSystem() {
        eventCoordinator.unsubscribe(pauseHandler, eventTypeToString(EventType::Pause));
    }

    void update() {
        std::lock_guard<std::mutex> lock(pauseMutex);
        if (shouldDisplayPause) {
            try {
                std::string pauseText = "Paused";
                auto *fontHelper = FontHelper::getInstance();
                if (fontHelper) {
                    fontHelper->renderText(pauseText, "game_font",
                                           SDL_Color{255, 255, 255, 255},
                                           SCREEN_WIDTH * 3 / 4.f, SCREEN_HEIGHT / 2.f);
                }
            } catch (const std::exception &e) {
                std::cerr << "Error rendering pause message: " << e.what() << std::endl;
            }
        }
    }
};

#endif //PAUSE_HANDLER_HPP
