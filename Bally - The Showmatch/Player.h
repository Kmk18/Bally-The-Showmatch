#pragma once

#include "Vector2.h"
#include "Renderer.h"
#include <SDL3/SDL.h>

enum class PlayerState {
    IDLE,
    AIMING,
    THROWING,
    DEAD
};

class Player {
public:
    Player(int id, const Vector2& position, const Color& color);

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

    // Skills
    bool HasSkill(int skillType) const;
    void UseSkill(int skillType);
    void AddSkill(int skillType);

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

    // Skills
    std::vector<int> m_availableSkills;

    // Input handling
    bool m_leftPressed;
    bool m_rightPressed;
    bool m_upPressed;
    bool m_downPressed;
    bool m_spacePressed;

    // Constants
    static constexpr float MOVE_SPEED = 200.0f;
    static constexpr float ANGLE_SPEED = 90.0f;
    static constexpr float POWER_SPEED = 50.0f;
    static constexpr float MAX_POWER = 100.0f;
    static constexpr float MAX_ANGLE = 90.0f;
    static constexpr float MIN_ANGLE = -90.0f;
    static constexpr float DEFAULT_HEALTH = 100.0f;
    static constexpr float DEFAULT_RADIUS = 20.0f;
    static constexpr float DEFAULT_MASS = 1.0f;
};
