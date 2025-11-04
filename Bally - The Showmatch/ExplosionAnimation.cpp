#include "ExplosionAnimation.h"
#include "Renderer.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>

ExplosionAnimation::ExplosionAnimation(const Vector2& position, float radius, bool isBigExplosion)
    : m_position(position), m_explosionRadius(radius), m_isBigExplosion(isBigExplosion),
      m_texture(nullptr), m_frameCount(isBigExplosion ? 8 : 7), m_currentFrame(0), m_animationTimer(0.0f),
      m_frameDuration(0.08f), m_frameWidth(0), m_frameHeight(0), m_finished(false) {
}

ExplosionAnimation::~ExplosionAnimation() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

bool ExplosionAnimation::Load(Renderer* renderer) {
    std::string filename;
    if (m_isBigExplosion) {
        filename = "../assets/common/big_explosion.png";
    } else {
        filename = "../assets/common/small_explosion.png";
    }
    return LoadSpriteSheet(renderer, filename);
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
    m_frameHeight = surface->h;
    m_frameWidth = surface->w / m_frameCount;

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
    destRect.x = m_position.x - animationSize * 0.5f - cameraOffset.x;
    destRect.y = m_position.y - animationSize * 0.5f - cameraOffset.y;
    destRect.w = animationSize;
    destRect.h = animationSize;

    // Draw the frame
    SDL_RenderTexture(renderer->GetSDLRenderer(), m_texture, &srcRect, &destRect);
}

