#include "ExplosionAnimation.h"
#include "Renderer.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>

ExplosionAnimation::ExplosionAnimation(const Vector2& position, float radius, bool isBigExplosion)
    : m_position(position), m_explosionRadius(radius), 
      m_type(isBigExplosion ? ExplosionAnimationType::BIG_EXPLOSION : ExplosionAnimationType::SMALL_EXPLOSION),
      m_texture(nullptr), m_frameCount(isBigExplosion ? 8 : 7), m_currentFrame(0), m_animationTimer(0.0f),
      m_frameDuration(0.08f), m_frameWidth(0), m_frameHeight(0), m_finished(false) {
}

ExplosionAnimation::ExplosionAnimation(const Vector2& position, float radius, ExplosionAnimationType type)
    : m_position(position), m_explosionRadius(radius), m_type(type),
      m_texture(nullptr), m_frameCount(12), m_currentFrame(0), m_animationTimer(0.0f),
      m_frameDuration(0.08f), m_frameWidth(0), m_frameHeight(0), m_finished(false) {
}

ExplosionAnimation::~ExplosionAnimation() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

bool ExplosionAnimation::Load(Renderer* renderer) {
    std::string filename = GetSpriteFilename();
    return LoadSpriteSheet(renderer, filename);
}

std::string ExplosionAnimation::GetSpriteFilename() const {
    switch (m_type) {
        case ExplosionAnimationType::SMALL_EXPLOSION:
            return "../assets/common/small_explosion.png";
        case ExplosionAnimationType::BIG_EXPLOSION:
            return "../assets/common/big_explosion.png";
        case ExplosionAnimationType::TELEPORT:
            return "../assets/common/teleport.png";
        case ExplosionAnimationType::HEAL:
            return "../assets/common/heal.png";
        case ExplosionAnimationType::COLLECT:
            return "../assets/common/collect.png";
        default:
            return "../assets/common/small_explosion.png";
    }
}

bool ExplosionAnimation::LoadSpriteSheet(Renderer* renderer, const std::string& filename) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cerr << "Failed to load explosion sprite: " << filename << " - " << SDL_GetError() << std::endl;
        return false;
    }

    m_texture = SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), surface);
    if (!m_texture) {
        std::cerr << "Failed to create texture from: " << filename << " - " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        return false;
    }

    // Store frame dimensions
    if (m_type == ExplosionAnimationType::TELEPORT || m_type == ExplosionAnimationType::HEAL || m_type == ExplosionAnimationType::COLLECT) {
        m_frameWidth = 64;
        m_frameHeight = 48;
    } else {
        m_frameHeight = surface->h;
        m_frameWidth = surface->w / m_frameCount;
    }

    SDL_DestroySurface(surface);
    return true;
}

void ExplosionAnimation::Update(float deltaTime) {
    if (m_finished || !m_texture) return;

    m_animationTimer += deltaTime;

    if (m_animationTimer >= m_frameDuration) {
        m_animationTimer = 0.0f;
        m_currentFrame++;

        if (m_currentFrame >= m_frameCount) {
            m_finished = true;
        }
    }
}

void ExplosionAnimation::Draw(Renderer* renderer) {
    if (m_finished || !m_texture) return;

    // Calculate size based on explosion radius
    // Explosion radius is the actual damage/effect radius, so animation should be 2x radius (diameter)
    float animationSize = m_explosionRadius * 2.0f;

    // Source rectangle (current frame) - SDL3 uses SDL_FRect for both source and destination
    SDL_FRect srcRect;
    srcRect.x = static_cast<float>(m_currentFrame * m_frameWidth);
    srcRect.y = 0.0f;
    srcRect.w = static_cast<float>(m_frameWidth);
    srcRect.h = static_cast<float>(m_frameHeight);

    // Destination rectangle (scaled to match explosion radius)
    Vector2 cameraOffset = renderer->GetCameraOffset();
    SDL_FRect destRect;
    
    if (m_type == ExplosionAnimationType::TELEPORT || m_type == ExplosionAnimationType::HEAL || m_type == ExplosionAnimationType::COLLECT) {
        // Maintain 4:3 aspect ratio (64:48)
        float aspectRatio = static_cast<float>(m_frameWidth) / static_cast<float>(m_frameHeight);
        float destWidth = animationSize;
        float destHeight = animationSize / aspectRatio;
        destRect.x = m_position.x - destWidth * 0.5f - cameraOffset.x;
        destRect.y = m_position.y - destHeight * 0.5f - cameraOffset.y;
        destRect.w = destWidth;
        destRect.h = destHeight;
    } else {
        // Explosion animations use 1:1 ratio
        destRect.x = m_position.x - animationSize * 0.5f - cameraOffset.x;
        destRect.y = m_position.y - animationSize * 0.5f - cameraOffset.y;
        destRect.w = animationSize;
        destRect.h = animationSize;
    }

    // Draw the frame
    SDL_RenderTexture(renderer->GetSDLRenderer(), m_texture, &srcRect, &destRect);
}

