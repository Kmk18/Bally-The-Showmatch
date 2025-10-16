#include "InputManager.h"
#include "Player.h"
#include <SDL3/SDL.h>

InputManager::InputManager() {
    // Initialize key states
    for (int i = 0; i < SDL_SCANCODE_COUNT; ++i) {
        m_currentKeys[static_cast<SDL_Scancode>(i)] = false;
        m_previousKeys[static_cast<SDL_Scancode>(i)] = false;
    }

    // Initialize mouse buttons
    for (int i = 0; i < 3; ++i) {
        m_mouseButtons[i] = false;
        m_previousMouseButtons[i] = false;
    }

    m_mousePosition = Vector2::Zero();
}

InputManager::~InputManager() {
}

void InputManager::Update() {
    UpdateKeyboard();
    UpdateMouse();
}

void InputManager::UpdateKeyboard() {
    // Copy current to previous
    m_previousKeys = m_currentKeys;

    // Update current key states
    const bool* keyboardState = SDL_GetKeyboardState(nullptr);

    for (int i = 0; i < SDL_SCANCODE_COUNT; ++i) {
        m_currentKeys[static_cast<SDL_Scancode>(i)] = keyboardState[i];
    }
}

void InputManager::UpdateMouse() {
    // Copy current to previous
    for (int i = 0; i < 3; ++i) {
        m_previousMouseButtons[i] = m_mouseButtons[i];
    }

    // Get mouse position
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    m_mousePosition = Vector2(mouseX, mouseY);

    // Get mouse button states
    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
    m_mouseButtons[0] = (mouseState & SDL_BUTTON_LMASK) != 0; // Left
    m_mouseButtons[1] = (mouseState & SDL_BUTTON_MMASK) != 0; // Middle
    m_mouseButtons[2] = (mouseState & SDL_BUTTON_RMASK) != 0; // Right
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const {
    auto it = m_currentKeys.find(key);
    return it != m_currentKeys.end() ? it->second : false;
}

bool InputManager::IsKeyJustPressed(SDL_Scancode key) {
    bool current = IsKeyPressed(key);
    bool previous = m_previousKeys[key];
    return current && !previous;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode key) {
    bool current = IsKeyPressed(key);
    bool previous = m_previousKeys[key];
    return !current && previous;
}

InputManager::PlayerInput InputManager::GetPlayerInput(int playerId, SDL_Scancode key) const {
    auto it = m_playerMappings.find(playerId);
    if (it == m_playerMappings.end()) {
        return PlayerInput::NONE;
    }

    const PlayerKeyMappings& mappings = it->second;

    if (key == mappings.moveLeft) return PlayerInput::MOVE_LEFT;
    if (key == mappings.moveRight) return PlayerInput::MOVE_RIGHT;
    if (key == mappings.aimUp) return PlayerInput::AIM_UP;
    if (key == mappings.aimDown) return PlayerInput::AIM_DOWN;
    if (key == mappings.adjustPower) return PlayerInput::ADJUST_POWER;

    return PlayerInput::NONE;
}

bool InputManager::IsPlayerInputPressed(int playerId, PlayerInput input) const {
    auto it = m_playerMappings.find(playerId);
    if (it == m_playerMappings.end()) {
        return false;
    }

    const PlayerKeyMappings& mappings = it->second;
    SDL_Scancode key = SDL_SCANCODE_UNKNOWN;

    switch (input) {
    case PlayerInput::MOVE_LEFT:
        key = mappings.moveLeft;
        break;
    case PlayerInput::MOVE_RIGHT:
        key = mappings.moveRight;
        break;
    case PlayerInput::AIM_UP:
        key = mappings.aimUp;
        break;
    case PlayerInput::AIM_DOWN:
        key = mappings.aimDown;
        break;
    case PlayerInput::ADJUST_POWER:
        key = mappings.adjustPower;
        break;
    default:
        return false;
    }

    return IsKeyPressed(key);
}

bool InputManager::IsPlayerInputJustReleased(int playerId, PlayerInput input) {
    auto it = m_playerMappings.find(playerId);
    if (it == m_playerMappings.end()) {
        return false;
    }

    const PlayerKeyMappings& mappings = it->second;
    SDL_Scancode key = SDL_SCANCODE_UNKNOWN;

    switch (input) {
    case PlayerInput::MOVE_LEFT:
        key = mappings.moveLeft;
        break;
    case PlayerInput::MOVE_RIGHT:
        key = mappings.moveRight;
        break;
    case PlayerInput::AIM_UP:
        key = mappings.aimUp;
        break;
    case PlayerInput::AIM_DOWN:
        key = mappings.aimDown;
        break;
    case PlayerInput::ADJUST_POWER:
        key = mappings.adjustPower;
        break;
    default:
        return false;
    }

    return IsKeyJustReleased(key);
}

void InputManager::SetKeyMapping(int playerId, PlayerInput input, SDL_Scancode key) {
    PlayerKeyMappings& mappings = m_playerMappings[playerId];

    switch (input) {
    case PlayerInput::MOVE_LEFT:
        mappings.moveLeft = key;
        break;
    case PlayerInput::MOVE_RIGHT:
        mappings.moveRight = key;
        break;
    case PlayerInput::AIM_UP:
        mappings.aimUp = key;
        break;
    case PlayerInput::AIM_DOWN:
        mappings.aimDown = key;
        break;
    case PlayerInput::ADJUST_POWER:
        mappings.adjustPower = key;
        break;
    default:
        break;
    }
}

Vector2 InputManager::GetMousePosition() const {
    return m_mousePosition;
}

bool InputManager::IsMouseButtonPressed(int button) const {
    if (button >= 0 && button < 3) {
        return m_mouseButtons[button];
    }
    return false;
}

bool InputManager::IsMouseButtonJustPressed(int button) {
    if (button >= 0 && button < 3) {
        return m_mouseButtons[button] && !m_previousMouseButtons[button];
    }
    return false;
}
