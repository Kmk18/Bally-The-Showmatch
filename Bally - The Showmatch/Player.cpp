#include "Player.h"
#include "InputManager.h"
#include <cmath>
#include <algorithm>

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Player::Player(int id, const Vector2& position, const Color& color, const std::string& characterName)
    : m_id(id), m_position(position), m_velocity(Vector2::Zero()), m_angle(-45.0f), m_power(0.0f),
    m_state(PlayerState::IDLE), m_health(DEFAULT_HEALTH), m_maxHealth(DEFAULT_HEALTH),
    m_mass(DEFAULT_MASS), m_radius(DEFAULT_RADIUS), m_acceleration(Vector2::Zero()),
    m_color(color), m_facingRight(true), m_characterName(characterName),
    m_hurtAnimationTimer(0.0f), m_lastHealth(DEFAULT_HEALTH),
    m_leftPressed(false), m_rightPressed(false),
    m_upPressed(false), m_downPressed(false), m_spacePressed(false), m_powerIncreasing(true),
    m_team(0) {

    // Create character animation if character name is provided
    if (!m_characterName.empty()) {
        m_animation = std::make_unique<CharacterAnimation>(m_characterName);
    }
}

void Player::Update(float deltaTime) {
    // Check if player just took damage
    if (m_health < m_lastHealth && m_health > 0) {
        m_hurtAnimationTimer = 0.4f; // Play hurt animation for 0.4 seconds (4 frames at 0.1s each)
    }
    m_lastHealth = m_health;

    // Handle death
    if (m_state == PlayerState::DEAD) {
        if (m_animation) {
            m_animation->SetAnimation(AnimationType::DIE);
            m_animation->Update(deltaTime);
        }
        return;
    }

    UpdatePhysics(deltaTime);

    // Update hurt animation timer
    if (m_hurtAnimationTimer > 0.0f) {
        m_hurtAnimationTimer -= deltaTime;
    }

    // Update character animation
    if (m_animation) {
        // Priority 1: Hurt animation (if player just took damage)
        if (m_hurtAnimationTimer > 0.0f) {
            m_animation->SetAnimation(AnimationType::HURT);
        }
        // Priority 2: Throwing animation (when space is released - play frames 2-3)
        else if (m_state == PlayerState::THROWING) {
            m_animation->SetAnimation(AnimationType::THROW);
            // Resume animation to play frames 2 and 3
            m_animation->ResumeAnimation();
        }
        // Priority 3: Charging throw animation (while holding space - play frames 0-1, pause at 1)
        else if (m_state == PlayerState::AIMING && m_spacePressed) {
            m_animation->SetAnimation(AnimationType::THROW);

            // Allow animation to advance to frame 1, then pause
            if (m_animation->GetCurrentFrame() >= 1) {
                m_animation->PauseAtFrame(1);
            } else {
                m_animation->ResumeAnimation(); // Let it advance from 0 to 1
            }
        }
        // Priority 4: Walking animation (ONLY when actively pressing movement keys)
        else if (m_state == PlayerState::AIMING && (m_leftPressed || m_rightPressed)) {
            m_animation->SetAnimation(AnimationType::WALK);
        }
        // Priority 5: Idle animation (default - no user input)
        else {
            m_animation->SetAnimation(AnimationType::IDLE);
        }

        m_animation->Update(deltaTime);
    }

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
        // Prevent movement while charging (holding space)
        if (!m_spacePressed) {
            if (m_leftPressed) {
                m_position.x -= MOVE_SPEED * (1.0f / 60.0f); // Assuming 60 FPS
            }
            if (m_rightPressed) {
                m_position.x += MOVE_SPEED * (1.0f / 60.0f);
            }
        }

        // Angle controls: up aims upward, down aims downward (can still aim while charging)
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

    // Keep player within horizontal bounds (removed - handled by map/terrain bounds)
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

bool Player::ShouldBeRemoved() const {
    // Remove player if dead and death animation has finished
    if (!IsAlive()) {
        if (m_animation) {
            return m_animation->IsAnimationFinished();
        }
        return true; // If no animation, remove immediately
    }
    return false;
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
        // Check if trying to select TELEPORT or HEAL (cannot be combined with others)
        if (skillType == 3 || skillType == 4) { // SkillType::TELEPORT or HEAL
            // TELEPORT and HEAL cannot be combined with other skills
            if (!m_selectedSkills.empty()) {
                // Already have other skills selected, clear them first
                m_selectedSkills.clear();
            }
            m_selectedSkills.push_back(skillType);
        }
        // Check if trying to select another skill while TELEPORT or HEAL is selected
        else if (!m_selectedSkills.empty() && (m_selectedSkills[0] == 3 || m_selectedSkills[0] == 4)) {
            // TELEPORT or HEAL is already selected, replace it with the new skill
            m_selectedSkills.clear();
            m_selectedSkills.push_back(skillType);
        }
        else {
            // Normal skill selection (can combine split, power, explosive)
            m_selectedSkills.push_back(skillType);
        }
    }
}
