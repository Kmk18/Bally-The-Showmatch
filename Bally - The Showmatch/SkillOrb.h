#pragma once

#include "Vector2.h"
#include "UI.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Player;
class Renderer;

class SkillOrb {
public:
    SkillOrb(const Vector2& position, SkillType skillType, int spawnTurn);
    ~SkillOrb();

    void Update(float deltaTime);
    void Draw(Renderer* renderer) const;
    void OnCollected(Player* player);
    bool IsExpired(int currentTurn, int playerCount) const { return currentTurn >= m_spawnTurn + (playerCount - 1); }

    const Vector2& GetPosition() const { return m_position; }
    float GetRadius() const { return m_radius; }
    SkillType GetSkillType() const { return m_skillType; }
    bool IsCollected() const { return m_collected; }
    bool IsActive() const { return !m_collected; }

    void SetPosition(const Vector2& position) { m_position = position; }
    void SetCollected(bool collected) { m_collected = collected; }

    // Texture loading
    bool LoadTexture(Renderer* renderer);

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
    int m_spawnTurn; // Turn number when this orb was spawned
    float m_animTime; // Animation timer
    float m_bobOffset;
    float m_bobSpeed;
    SDL_Texture* m_texture; // Texture for the skill orb

    // Animation
    void UpdateAnimation(float deltaTime);
    std::string GetTexturePath() const;

    // Constants
    static constexpr float DEFAULT_RADIUS = 15.0f;
    static constexpr float MAX_LIFETIME = 30.0f;
    static constexpr float BOB_SPEED = 3.0f;
    static constexpr float BOB_AMPLITUDE = 5.0f;
};
