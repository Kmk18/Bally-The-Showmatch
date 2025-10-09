#pragma once

#include "Vector2.h"
#include "UI.h"

class Player;
class Renderer;

class SkillOrb {
public:
    SkillOrb(const Vector2& position, SkillType skillType);

    void Update(float deltaTime);
    void Draw(Renderer* renderer) const;
    void OnCollected(Player* player);

    const Vector2& GetPosition() const { return m_position; }
    float GetRadius() const { return m_radius; }
    SkillType GetSkillType() const { return m_skillType; }
    bool IsCollected() const { return m_collected; }
    bool IsActive() const { return !m_collected && m_lifetime > 0; }

    void SetPosition(const Vector2& position) { m_position = position; }
    void SetCollected(bool collected) { m_collected = collected; }

    // Skill effects
    static void ApplySplitThrowSkill(Player* player);
    static void ApplyEnhancedDamageSkill(Player* player);
    static void ApplyEnhancedExplosiveSkill(Player* player);
    static void ApplyTeleportSkill(Player* player);

private:
    Vector2 m_position;
    float m_radius;
    SkillType m_skillType;
    bool m_collected;
    float m_lifetime;
    float m_maxLifetime;
    float m_bobOffset;
    float m_bobSpeed;
    Color m_color;

    // Animation
    void UpdateAnimation(float deltaTime);
    Color GetSkillColor() const;
    std::string GetSkillName() const;

    // Constants
    static constexpr float DEFAULT_RADIUS = 15.0f;
    static constexpr float MAX_LIFETIME = 30.0f;
    static constexpr float BOB_SPEED = 3.0f;
    static constexpr float BOB_AMPLITUDE = 5.0f;
};
