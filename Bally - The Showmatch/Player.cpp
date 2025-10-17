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
    m_color(color), m_facingRight(true), m_leftPressed(false), m_rightPressed(false),
    m_upPressed(false), m_downPressed(false), m_spacePressed(false), m_powerIncreasing(true) {
}

void Player::Update(float deltaTime) {
    if (m_state == PlayerState::DEAD) return;

    UpdatePhysics(deltaTime);

    // Update input states
    if (m_state == PlayerState::AIMING) {
        // Handle power adjustment (while space is held)
        if (m_spacePressed) {
            if (m_powerIncreasing) {
                m_power += POWER_SPEED * deltaTime;
                if (m_power >= MAX_POWER) {
                    m_power = MAX_POWER;
                    m_powerIncreasing = false;  // Reverse direction
                }
            } else {
                m_power -= POWER_SPEED * deltaTime;
                if (m_power <= 0.0f) {
                    m_power = 0.0f;
                    m_powerIncreasing = true;  // Reverse direction
                }
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
        if (pressed) {
            m_facingRight = false;
        }
        break;
    case InputManager::PlayerInput::MOVE_RIGHT:
        m_rightPressed = pressed;
        if (pressed) {
            m_facingRight = true;
        }
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
        // THROW input is now handled in Game.cpp via space key release
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

        // Angle controls: up aims upward, down aims downward
        if (m_upPressed) {
            m_angle -= ANGLE_SPEED * (1.0f / 60.0f);
            m_angle = std::max(m_angle, MIN_ANGLE);
        }
        if (m_downPressed) {
            m_angle += ANGLE_SPEED * (1.0f / 60.0f);
            m_angle = std::min(m_angle, MAX_ANGLE);
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

    // Keep player within horizontal screen bounds
    m_position.x = clamp(m_position.x, m_radius, 1200.0f - m_radius);
}

void Player::StartTurn() {
    m_state = PlayerState::AIMING;
    m_power = 0.0f;
    m_angle = -45.0f; // Reset angle
    m_selectedSkills.clear(); // Clear selected skills for new turn
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

    // Clear skills and inventory
    m_availableSkills.clear();
    m_inventory.clear();
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

bool Player::AddSkillToInventory(int skillType) {
    // Check if inventory is full
    if (m_inventory.size() >= MAX_INVENTORY_SIZE) {
        return false;
    }

    // Add skill to inventory (can have duplicates)
    m_inventory.push_back(skillType);
    return true;
}

bool Player::UseInventorySlot(int slot) {
    // Validate slot
    if (slot < 0 || slot >= static_cast<int>(m_inventory.size())) {
        return false;
    }

    // Remove skill from that slot
    m_inventory.erase(m_inventory.begin() + slot);
    return true;
}

int Player::GetInventorySlot(int slot) const {
    if (slot < 0 || slot >= static_cast<int>(m_inventory.size())) {
        return -1;
    }
    return m_inventory[slot];
}

void Player::ToggleSkillSelection(int slot) {
    // Validate slot
    if (slot < 0 || slot >= static_cast<int>(m_inventory.size())) {
        return;
    }

    int skillType = m_inventory[slot];

    // Check if skill is already selected
    auto it = std::find(m_selectedSkills.begin(), m_selectedSkills.end(), skillType);
    if (it != m_selectedSkills.end()) {
        // Remove from selection
        m_selectedSkills.erase(it);
    }
    else {
        // Check if trying to select HEAL
        if (skillType == 4) { // SkillType::HEAL
            // HEAL cannot be combined with other skills
            if (!m_selectedSkills.empty()) {
                // Already have other skills selected, clear them first
                m_selectedSkills.clear();
            }
            m_selectedSkills.push_back(skillType);
        }
        // Check if trying to select another skill while HEAL is selected
        else if (!m_selectedSkills.empty() && m_selectedSkills[0] == 4) {
            // HEAL is already selected, replace it with the new skill
            m_selectedSkills.clear();
            m_selectedSkills.push_back(skillType);
        }
        else {
            // Normal skill selection (no heal involved)
            m_selectedSkills.push_back(skillType);
        }
    }
}
