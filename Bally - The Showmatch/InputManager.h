#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include "Vector2.h"

class InputManager {
public:
    InputManager();
    ~InputManager();

    void Update();
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyJustPressed(SDL_Scancode key);
    bool IsKeyJustReleased(SDL_Scancode key);

    enum class PlayerInput {
        MOVE_LEFT,
        MOVE_RIGHT,
        AIM_UP,
        AIM_DOWN,
        ADJUST_POWER,
        THROW,
        USE_SLOT_1,
        USE_SLOT_2,
        USE_SLOT_3,
        USE_SLOT_4,
        NONE
    };

    PlayerInput GetPlayerInput(int playerId, SDL_Scancode key) const;
    bool IsPlayerInputPressed(int playerId, PlayerInput input) const;
    bool IsPlayerInputJustPressed(int playerId, PlayerInput input);
    bool IsPlayerInputJustReleased(int playerId, PlayerInput input);

    void SetKeyMapping(int playerId, PlayerInput input, SDL_Scancode key);

    Vector2 GetMousePosition() const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonJustPressed(int button);

private:
    std::unordered_map<SDL_Scancode, bool> m_currentKeys;
    std::unordered_map<SDL_Scancode, bool> m_previousKeys;

    // Player input mappings
    struct PlayerKeyMappings {
        SDL_Scancode moveLeft = SDL_SCANCODE_LEFT;
        SDL_Scancode moveRight = SDL_SCANCODE_RIGHT;
        SDL_Scancode aimUp = SDL_SCANCODE_UP;
        SDL_Scancode aimDown = SDL_SCANCODE_DOWN;
        SDL_Scancode adjustPower = SDL_SCANCODE_SPACE;
        SDL_Scancode useSlot1 = SDL_SCANCODE_1;
        SDL_Scancode useSlot2 = SDL_SCANCODE_2;
        SDL_Scancode useSlot3 = SDL_SCANCODE_3;
        SDL_Scancode useSlot4 = SDL_SCANCODE_4;
    };

    std::unordered_map<int, PlayerKeyMappings> m_playerMappings;

    // Global mapping used for all players (defaults to arrow keys + space)
    PlayerKeyMappings m_globalMappings;

    // Mouse state
    Vector2 m_mousePosition;
    bool m_mouseButtons[3]; // Left, Middle, Right
    bool m_previousMouseButtons[3];

    void UpdateKeyboard();
    void UpdateMouse();
};
