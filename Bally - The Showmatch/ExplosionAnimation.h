#pragma once
#include "Vector2.h"
#include <SDL3/SDL.h>
#include <string>

class Renderer;

enum class ExplosionAnimationType {
    SMALL_EXPLOSION,
    BIG_EXPLOSION,
    TELEPORT,
    HEAL,
    COLLECT
};

class ExplosionAnimation {
public:
    ExplosionAnimation(const Vector2& position, float radius, bool isBigExplosion);
    ExplosionAnimation(const Vector2& position, float radius, ExplosionAnimationType type);
    ~ExplosionAnimation();

    bool Load(Renderer* renderer);
    void Update(float deltaTime);
    void Draw(Renderer* renderer);

    bool IsFinished() const { return m_finished; }
    const Vector2& GetPosition() const { return m_position; }
    float GetRadius() const { return m_explosionRadius; }

private:
    Vector2 m_position;
    float m_explosionRadius; // The actual explosion radius (used for sizing)
    ExplosionAnimationType m_type;
    SDL_Texture* m_texture;
    int m_frameCount;
    int m_currentFrame;
    float m_animationTimer;
    float m_frameDuration;
    int m_frameWidth;
    int m_frameHeight;
    bool m_finished;

    bool LoadSpriteSheet(Renderer* renderer, const std::string& filename);
    std::string GetSpriteFilename() const;
};

