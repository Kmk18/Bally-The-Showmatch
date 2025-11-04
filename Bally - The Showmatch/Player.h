#pragma once

#include "Vector2.h"
#include "Renderer.h"
#include "CharacterAnimation.h"
#include <SDL3/SDL.h>
#include <memory>

enum class PlayerState {
    IDLE,
    AIMING,
    THROWING,
    DEAD
};

class Player {
public:
    Player(int id, const Vector2& position, const Color& color, const std::string& characterName = "");

    void Update(float deltaTime);
    void HandleInput(int input, bool pressed);
    void TakeDamage(float damage);
    void Heal(float amount);

    // Getters
    int GetId() const { return m_id; }
    const Vector2& GetPosition() const { return m_position; }
    const Vector2& GetVelocity() const { return m_velocity; }
    float GetHealth() const { return m_health; }
    float GetMaxHealth() const { return m_maxHealth; }
    PlayerState GetState() const { return m_state; }
    const Color& GetColor() const { return m_color; }
    float GetAngle() const { return m_angle; }
    float GetPower() const { return m_power; }
    float GetRadius() const { return m_radius; }
    bool IsAlive() const { return m_health > 0; }
    bool IsFacingRight() const { return m_facingRight; }
    CharacterAnimation* GetAnimation() { return m_animation.get(); }
    bool ShouldBeRemoved() const; // Returns true if dead and death animation finished

    // Setters
    void SetPosition(const Vector2& position) { m_position = position; }
    void SetVelocity(const Vector2& velocity) { m_velocity = velocity; }
    void SetState(PlayerState state) { m_state = state; }
    void SetAngle(float angle) { m_angle = angle; }
    void SetPower(float power) { m_power = power; }
    void SetFacingRight(bool facingRight) { m_facingRight = facingRight; }

    // Physics
    void ApplyForce(const Vector2& force);
    void UpdatePhysics(float deltaTime);

    // Game logic
    void StartTurn();
    void EndTurn();
    void ResetForNewGame();

    // Skills and Inventory
    bool HasSkill(int skillType) const;
    void UseSkill(int skillType);
    void AddSkill(int skillType);
    bool AddSkillToInventory(int skillType); // Returns false if inventory is full
    bool UseInventorySlot(int slot); // slot 0-3, returns false if empty
    const std::vector<int>& GetInventory() const { return m_inventory; }
    std::vector<int>& GetInventory() { return m_inventory; }
    int GetInventorySlot(int slot) const; // Returns -1 if slot is empty or invalid
    bool IsInventoryFull() const { return m_inventory.size() >= MAX_INVENTORY_SIZE; }
    void ToggleSkillSelection(int slot); // Toggle skill selection for next shot
    const std::vector<int>& GetSelectedSkills() const { return m_selectedSkills; }
    void ClearSelectedSkills() { m_selectedSkills.clear(); }

private:
    int m_id;
    Vector2 m_position;
    Vector2 m_velocity;
    float m_angle;
    float m_power;
    PlayerState m_state;

    // Health system
    float m_health;
    float m_maxHealth;

    // Physics
    float m_mass;
    float m_radius;
    Vector2 m_acceleration;

    // Visual
    Color m_color;
    bool m_facingRight;
    std::unique_ptr<CharacterAnimation> m_animation;
    std::string m_characterName;
    float m_hurtAnimationTimer;
    float m_lastHealth;

    // Skills
    std::vector<int> m_availableSkills;

    // Inventory system (4 slots)
    std::vector<int> m_inventory; // Stack of up to 4 skill types
    std::vector<int> m_selectedSkills; // Skills selected for next shot
    static constexpr int MAX_INVENTORY_SIZE = 4;

    // Input handling
    bool m_leftPressed;
    bool m_rightPressed;
    bool m_upPressed;
    bool m_downPressed;
    bool m_spacePressed;
    bool m_powerIncreasing;  // Track if power is increasing or decreasing

    // Constants
    static constexpr float MOVE_SPEED = 5.0f;
    static constexpr float ANGLE_SPEED = 5.0f;
    static constexpr float POWER_SPEED = 35.0f;  // Reduced from 50.0f for slower power bar charging
    static constexpr float MAX_POWER = 100.0f;
    static constexpr float MAX_ANGLE = 90.0f;
    static constexpr float MIN_ANGLE = -90.0f;
    static constexpr float DEFAULT_HEALTH = 200.0f;
    static constexpr float DEFAULT_RADIUS = 20.0f;
    static constexpr float DEFAULT_MASS = 1.0f;
};
