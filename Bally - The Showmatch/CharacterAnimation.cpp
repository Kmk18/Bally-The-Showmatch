#include "CharacterAnimation.h"
#include "Renderer.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

CharacterAnimation::CharacterAnimation(const std::string& characterName)
    : m_characterName(characterName), m_currentAnimation(AnimationType::IDLE),
      m_currentFrame(0), m_animationTimer(0.0f), m_animationFinished(false),
      m_paused(false), m_pauseFrame(-1) {
    // Initialize all animation data
    for (int i = 0; i < 6; i++) {
        m_animations[i].texture = nullptr;
        m_animations[i].frameCount = 0;
        m_animations[i].frameWidth = 0;
        m_animations[i].frameHeight = 0;
        m_animations[i].frameDuration = 0.1f; // Default 100ms per frame
    }
}

CharacterAnimation::~CharacterAnimation() {
    // Clean up all textures
    for (int i = 0; i < 6; i++) {
        if (m_animations[i].texture) {
            SDL_DestroyTexture(m_animations[i].texture);
            m_animations[i].texture = nullptr;
        }
    }
}

bool CharacterAnimation::LoadCharacter(Renderer* renderer) {
    std::string basePath = "..\\characters\\" + m_characterName + "\\";

    // Load each animation sprite sheet
    if (!LoadSpriteSheet(renderer, basePath + "idle.png", m_animations[(int)AnimationType::IDLE], 4)) return false;

    // Try walk.png first, fallback to move.png for Turt
    if (!LoadSpriteSheet(renderer, basePath + "walk.png", m_animations[(int)AnimationType::WALK], 6)) {
        if (!LoadSpriteSheet(renderer, basePath + "move.png", m_animations[(int)AnimationType::WALK], 6)) {
            return false;
        }
    }

    if (!LoadSpriteSheet(renderer, basePath + "throw.png", m_animations[(int)AnimationType::THROW], 4)) return false;
    if (!LoadSpriteSheet(renderer, basePath + "hurt.png", m_animations[(int)AnimationType::HURT], 4)) return false;
    if (!LoadSpriteSheet(renderer, basePath + "die.png", m_animations[(int)AnimationType::DIE], 8)) return false;
    if (!LoadSpriteSheet(renderer, basePath + "push.png", m_animations[(int)AnimationType::PUSH], 6)) return false;

    std::cout << "Character '" << m_characterName << "' loaded successfully" << std::endl;
    return true;
}

bool CharacterAnimation::LoadSpriteSheet(Renderer* renderer, const std::string& filename,
                                         AnimationData& data, int frameCount) {
    // Load the image
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cerr << "Failed to load sprite sheet: " << filename << " - " << SDL_GetError() << std::endl;
        return false;
    }

    // Create texture from surface
    data.texture = SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), surface);
    if (!data.texture) {
        std::cerr << "Failed to create texture from: " << filename << " - " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        return false;
    }

    // Store animation data
    data.frameCount = frameCount;
    data.frameHeight = surface->h;
    data.frameWidth = surface->w / frameCount; // Assuming horizontal sprite sheet
    data.frameDuration = 0.1f; // 100ms per frame

    SDL_DestroySurface(surface);
    return true;
}

void CharacterAnimation::Update(float deltaTime) {
    AnimationData& currentAnim = m_animations[(int)m_currentAnimation];

    if (currentAnim.frameCount == 0) return;

    // Don't update if paused
    if (m_paused) {
        if (m_pauseFrame >= 0 && m_pauseFrame < currentAnim.frameCount) {
            m_currentFrame = m_pauseFrame;
        }
        return;
    }

    m_animationTimer += deltaTime;

    if (m_animationTimer >= currentAnim.frameDuration) {
        m_animationTimer = 0.0f;
        m_currentFrame++;

        if (m_currentFrame >= currentAnim.frameCount) {
            // Check if this is a one-shot animation (die, hurt)
            if (m_currentAnimation == AnimationType::DIE ||
                m_currentAnimation == AnimationType::HURT) {
                m_currentFrame = currentAnim.frameCount - 1; // Stay on last frame
                m_animationFinished = true;
            }
            else {
                m_currentFrame = 0; // Loop animation (idle, walk, throw, push)
            }
        }
    }
}

void CharacterAnimation::Draw(Renderer* renderer, const Vector2& position, float radius, bool facingRight) {
    AnimationData& currentAnim = m_animations[(int)m_currentAnimation];

    if (!currentAnim.texture) return;

    // Source rectangle (which frame to draw)
    SDL_FRect srcRect;
    srcRect.x = m_currentFrame * currentAnim.frameWidth;
    srcRect.y = 0;
    srcRect.w = currentAnim.frameWidth;
    srcRect.h = currentAnim.frameHeight;

    // Destination rectangle (where to draw on screen)
    // Scale sprite to match player radius (diameter = 2 * radius)
    float scale = (radius * 2.5f) / currentAnim.frameHeight; // Slightly larger than player circle
    float drawWidth = currentAnim.frameWidth * scale;
    float drawHeight = currentAnim.frameHeight * scale;

    SDL_FRect destRect;
    destRect.x = position.x - drawWidth / 2.0f;
    // Position sprite so bottom aligns with player's bottom (position.y + radius is the bottom)
    destRect.y = (position.y + radius) - drawHeight;
    destRect.w = drawWidth;
    destRect.h = drawHeight;

    // Apply camera offset
    Vector2 cameraOffset = renderer->GetCameraOffset();
    destRect.x -= cameraOffset.x;
    destRect.y -= cameraOffset.y;

    // Flip sprite based on facing direction
    SDL_FlipMode flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderTextureRotated(renderer->GetSDLRenderer(), currentAnim.texture,
                            &srcRect, &destRect, 0.0, nullptr, flip);
}

void CharacterAnimation::SetAnimation(AnimationType type) {
    if (m_currentAnimation != type) {
        m_currentAnimation = type;
        m_currentFrame = 0;
        m_animationTimer = 0.0f;
        m_animationFinished = false;
        m_paused = false;
        m_pauseFrame = -1;
    }
}

void CharacterAnimation::ResetAnimation() {
    m_currentFrame = 0;
    m_animationTimer = 0.0f;
    m_animationFinished = false;
    m_paused = false;
    m_pauseFrame = -1;
}

bool CharacterAnimation::IsAnimationFinished() const {
    return m_animationFinished;
}

void CharacterAnimation::PauseAtFrame(int frame) {
    m_paused = true;
    m_pauseFrame = frame;
}

void CharacterAnimation::ResumeAnimation() {
    m_paused = false;
}
