//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef SOUND_EVENT_HANDLER_HPP
#define SOUND_EVENT_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../ECS/coordinator.hpp"
#include "../model/components.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "sound.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class SoundEventHandler : public System {
private:
    std::shared_ptr<SoundSystem> soundSystem;
    bool musicStarted = false;

    // Handler for bubble shooting events
    EventHandler shootHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::BubbleShot)) {
            soundSystem->playSound("shoot", 0.7f);
        }
    };

    // Handler for bubble collision/matching events
    EventHandler matchHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::CheckFloatingBubbles)) {
            soundSystem->playSound("match", 0.8f);
        }
    };

    // Handler for bubble movement events
    EventHandler bounceHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::BubbleCollision)) {
            soundSystem->playSound("bounce", 0.5f);
        }
    };

    // Handler for game over event
    EventHandler gameOverHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::GameOver)) {
            soundSystem->stopMusic();
            soundSystem->playSound("game_over", 1.0f);
        }
    };

    // Handler for grid movement events
    EventHandler gridHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::DisableShooter)) {
            soundSystem->playSound("grid_move", 0.6f);
        }
    };

public:
    SoundEventHandler(std::shared_ptr<SoundSystem> ss) : soundSystem(ss) {
        // Load all sound effects
        soundSystem->loadSound("shoot", "assets/sounds/sfx/shoot.wav");
        soundSystem->loadSound("pop", "assets/sounds/sfx/shoot.wav");
        soundSystem->loadSound("match", "assets/sounds/sfx/pop.wav");
        soundSystem->loadSound("bounce", "assets/sounds/sfx/pop.wav");
        soundSystem->loadSound("level_complete", "assets/sounds/sfx/pop.wav");
        soundSystem->loadSound("game_over", "assets/sounds/sfx/pop.wav");
        soundSystem->loadSound("grid_move", "assets/sounds/sfx/pop.wav");

        // Load music
        soundSystem->loadMusic("game", "assets/sounds/music/game_theme.mp3");

        // Set initial volumes
        soundSystem->setMasterVolume(0.8f);
        soundSystem->setMusicVolume(0.5f);
        soundSystem->setSFXVolume(0.7f);

        // Subscribe to all relevant events
        eventCoordinator.subscribe(shootHandler, eventTypeToString(EventType::BubbleShot));
        eventCoordinator.subscribe(matchHandler, eventTypeToString(EventType::CheckFloatingBubbles));
        eventCoordinator.subscribe(bounceHandler, eventTypeToString(EventType::BubbleCollision));
        eventCoordinator.subscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
        eventCoordinator.subscribe(gridHandler, eventTypeToString(EventType::DisableShooter));
    }

    ~SoundEventHandler() {
        // Unsubscribe from all events
        eventCoordinator.unsubscribe(shootHandler, eventTypeToString(EventType::BubbleShot));
        eventCoordinator.unsubscribe(matchHandler, eventTypeToString(EventType::CheckFloatingBubbles));
        eventCoordinator.unsubscribe(bounceHandler, eventTypeToString(EventType::BubbleCollision));
        eventCoordinator.unsubscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
        eventCoordinator.unsubscribe(gridHandler, eventTypeToString(EventType::DisableShooter));
    }

    void update() {
        // Start background music if it hasn't started yet
        if (!musicStarted) {
            soundSystem->playMusic("game", 0.5f, true);
            musicStarted = true;
        }

        // Handle any volume changes or other continuous audio updates here
        // You could add volume control based on game state or user settings
    }
};

#endif //SOUND_EVENT_HANDLER_HPP
