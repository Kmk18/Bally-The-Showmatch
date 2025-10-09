#include "Player.h"
#include "InputManager.h"
#include <cmath>
#include <algorithm>

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Player::Player(int id, const Vector2& position, const Color& color)
    : m_id(id), m_position(position), m_velocity(Vector2::Zero()), m_angle(-45.0f), m_power(0.0f),
    m_state(PlayerState::IDLE), m_health(DEFAULT_HEALTH), m_maxHealth(DEFAULT_HEALTH),
    m_mass(DEFAULT_MASS), m_radius(DEFAULT_RADIUS), m_acceleration(Vector2::Zero()),
    m_color(color), m_leftPressed(false), m_rightPressed(false),
    m_upPressed(false), m_downPressed(false), m_spacePressed(false) {
}

void Player::Update(float deltaTime) {
    if (m_state == PlayerState::DEAD) return;

    UpdatePhysics(deltaTime);

    // Update input states
    if (m_state == PlayerState::AIMING) {
        // Handle power adjustment
        if (m_spacePressed) {
            m_power += POWER_SPEED * deltaTime;
            if (m_power > MAX_POWER) {
                m_power = 0.0f; // Cycle back to 0
            }
        }
    }
}

void Player::HandleInput(int input, bool pressed) {
    if (m_state == PlayerState::DEAD) return;

    InputManager::PlayerInput playerInput = static_cast<InputManager::PlayerInput>(input);
    switch (playerInput) {
    case InputManager::PlayerInput::MOVE_LEFT:
        m_leftPressed = pressed;
        break;
    case InputManager::PlayerInput::MOVE_RIGHT:
        m_rightPressed = pressed;
        break;
    case InputManager::PlayerInput::AIM_UP:
        m_upPressed = pressed;
        break;
    case InputManager::PlayerInput::AIM_DOWN:
        m_downPressed = pressed;
        break;
    case InputManager::PlayerInput::ADJUST_POWER:
        m_spacePressed = pressed;
        break;
    case InputManager::PlayerInput::THROW:
        if (pressed && m_state == PlayerState::AIMING) {
            // Throw projectile (handled by Game class)
            m_state = PlayerState::THROWING;
        }
        break;
    default:
        break;
    }

    // Handle continuous input
    if (m_state == PlayerState::AIMING) {
        if (m_leftPressed) {
            m_position.x -= MOVE_SPEED * (1.0f / 60.0f); // Assuming 60 FPS
        }
        if (m_rightPressed) {
            m_position.x += MOVE_SPEED * (1.0f / 60.0f);
        }
        if (m_upPressed) {
            m_angle += ANGLE_SPEED * (1.0f / 60.0f);
            m_angle = std::min(m_angle, MAX_ANGLE);
        }
        if (m_downPressed) {
            m_angle -= ANGLE_SPEED * (1.0f / 60.0f);
            m_angle = std::max(m_angle, MIN_ANGLE);
        }
    }
}

void Player::TakeDamage(float damage) {
    m_health -= damage;
    m_health = std::max(0.0f, m_health);

    if (m_health <= 0.0f) {
        m_state = PlayerState::DEAD;
    }
}

void Player::Heal(float amount) {
    m_health += amount;
    m_health = std::min(m_health, m_maxHealth);
}

void Player::ApplyForce(const Vector2& force) {
    m_acceleration = m_acceleration + force / m_mass;
}

void Player::UpdatePhysics(float deltaTime) {
    // Apply gravity
    ApplyForce(Vector2(0, 980.0f * m_mass)); // Gravity

    // Update velocity
    m_velocity = m_velocity + m_acceleration * deltaTime;

    // Update position
    m_position = m_position + m_velocity * deltaTime;

    // Reset acceleration
    m_acceleration = Vector2::Zero();

    // Apply friction
    m_velocity = m_velocity * 0.99f;

    // Keep player on platform
    float platformY = 650.0f; // Platform top
    if (m_position.y > platformY - m_radius) {
        m_position.y = platformY - m_radius;
        if (m_velocity.y > 0) {
            m_velocity.y = 0;
        }
    }

    // Keep player within screen bounds
    m_position.x = clamp(m_position.x, m_radius, 1200.0f - m_radius);
}

void Player::StartTurn() {
    m_state = PlayerState::AIMING;
    m_power = 0.0f;
    m_angle = -45.0f; // Reset angle
}

void Player::EndTurn() {
    m_state = PlayerState::IDLE;
    m_power = 0.0f;
}

void Player::ResetForNewGame() {
    m_health = m_maxHealth;
    m_state = PlayerState::IDLE;
    m_velocity = Vector2::Zero();
    m_acceleration = Vector2::Zero();
    m_power = 0.0f;
    m_angle = -45.0f;

    // Reset position based on player ID
    float platformWidth = 800.0f;
    float spacing = platformWidth / 4;
    m_position.x = 200.0f + spacing * (m_id + 1);
    m_position.y = 600.0f;

    // Clear input states
    m_leftPressed = m_rightPressed = m_upPressed = m_downPressed = m_spacePressed = false;

    // Clear skills
    m_availableSkills.clear();
}

bool Player::HasSkill(int skillType) const {
    return std::find(m_availableSkills.begin(), m_availableSkills.end(), skillType) != m_availableSkills.end();
}

void Player::UseSkill(int skillType) {
    auto it = std::find(m_availableSkills.begin(), m_availableSkills.end(), skillType);
    if (it != m_availableSkills.end()) {
        m_availableSkills.erase(it);
    }
}

void Player::AddSkill(int skillType) {
    // Check if player already has this skill
    if (!HasSkill(skillType)) {
        m_availableSkills.push_back(skillType);
    }
}
