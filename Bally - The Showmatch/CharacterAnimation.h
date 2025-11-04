#pragma once
#include "Vector2.h"
#include <string>
#include <SDL3/SDL.h>

class Renderer;

enum class AnimationType {
    IDLE,
    WALK,
    THROW,
    HURT,
    DIE,
    PUSH
};

class CharacterAnimation {
public:
    CharacterAnimation(const std::string& characterName);
    ~CharacterAnimation();

    bool LoadCharacter(Renderer* renderer);
    void Update(float deltaTime);
    void Draw(Renderer* renderer, const Vector2& position, float radius, bool facingRight);

    void SetAnimation(AnimationType type);
    AnimationType GetCurrentAnimation() const { return m_currentAnimation; }

    void ResetAnimation();
    bool IsAnimationFinished() const;

    // Special controls for throw animation
    void PauseAtFrame(int frame);
    void ResumeAnimation();
    int GetCurrentFrame() const { return m_currentFrame; }

private:
    struct AnimationData {
        SDL_Texture* texture;
        int frameCount;
        int frameWidth;
        int frameHeight;
        float frameDuration;
    };

    std::string m_characterName;
    AnimationData m_animations[6]; // One for each AnimationType
    AnimationType m_currentAnimation;
    int m_currentFrame;
    float m_animationTimer;
    bool m_animationFinished;
    bool m_paused;
    int m_pauseFrame;

    bool LoadSpriteSheet(Renderer* renderer, const std::string& filename, AnimationData& data, int frameCount);
};
