#include "Terrain.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Terrain::Terrain() : m_surface(nullptr), m_texture(nullptr), m_width(0), m_height(0), m_needsTextureUpdate(false) {
}

Terrain::~Terrain() {
    if (m_surface) {
        SDL_DestroySurface(m_surface);
        m_surface = nullptr;
    }
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

bool Terrain::LoadFromImage(const std::string& filepath) {
    // Load image using SDL_image
    SDL_Surface* loadedSurface = IMG_Load(filepath.c_str());
    if (!loadedSurface) {
        std::cerr << "Failed to load terrain image: " << filepath.c_str() << " - " << SDL_GetError() << std::endl;
        return false;
    }

    // Convert to RGBA format for easier pixel manipulation
    m_surface = SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loadedSurface);

    if (!m_surface) {
        std::cerr << "Failed to convert terrain surface to RGBA32" << std::endl;
        return false;
    }

    m_width = m_surface->w;
    m_height = m_surface->h;
    m_needsTextureUpdate = true;

    std::cout << "Terrain loaded: " << filepath.c_str() << " (" << m_width << "x" << m_height << ")" << std::endl;
    return true;
}

void Terrain::CreateDefaultTerrain(int width, int height) {
    // Create a default terrain surface with RGBA32 format
    m_width = width;
    m_height = height;

    m_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!m_surface) {
        std::cerr << "Failed to create default terrain surface" << std::endl;
        return;
    }

    // Fill with a simple terrain pattern
    SDL_LockSurface(m_surface);

    Uint32* pixels = (Uint32*)m_surface->pixels;
    int pitch = m_surface->pitch / 4; // Convert byte pitch to pixel pitch

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Uint32 color = 0x00000000; // Transparent by default

            // Create a simple ground at the bottom with some hills
            float terrainHeight = height * 0.7f + std::sin(x * 0.05f) * 30.0f;

            if (y >= terrainHeight) {
                // Solid ground - brown/green color
                if (y < terrainHeight + 20) {
                    color = 0x8B7355FF; // Grass brown
                } else {
                    color = 0x654321FF; // Dirt brown
                }
            }

            pixels[y * pitch + x] = color;
        }
    }

    SDL_UnlockSurface(m_surface);
    m_needsTextureUpdate = true;

    std::cout << "Default terrain created (" << width << "x" << height << ")" << std::endl;
}

void Terrain::Draw(Renderer* renderer) {
    if (!m_surface) return;

    // Update texture if needed (after destruction)
    if (m_needsTextureUpdate) {
        UpdateTexture(renderer);
        m_needsTextureUpdate = false;
    }

    if (m_texture) {
        // Get camera offset from renderer and apply it
        Vector2 cameraOffset = renderer->GetCameraOffset();
        SDL_FRect destRect = { -cameraOffset.x, -cameraOffset.y, (float)m_width, (float)m_height };
        SDL_RenderTexture(renderer->GetSDLRenderer(), m_texture, nullptr, &destRect);
    }
}

bool Terrain::IsInBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

Uint8 Terrain::GetPixelAlpha(int x, int y) const {
    if (!m_surface || !IsInBounds(x, y)) return 0;

    SDL_LockSurface(m_surface);

    Uint32* pixels = (Uint32*)m_surface->pixels;
    int pitch = m_surface->pitch / 4;
    Uint32 pixel = pixels[y * pitch + x];

    SDL_UnlockSurface(m_surface);

    // Extract alpha channel (assuming RGBA32 format)
    Uint8 alpha = (pixel >> 24) & 0xFF;
    return alpha;
}

void Terrain::SetPixelTransparent(int x, int y) {
    if (!m_surface || !IsInBounds(x, y)) return;

    SDL_LockSurface(m_surface);

    Uint32* pixels = (Uint32*)m_surface->pixels;
    int pitch = m_surface->pitch / 4;
    pixels[y * pitch + x] = 0x00000000; // Fully transparent

    SDL_UnlockSurface(m_surface);
}

bool Terrain::IsPixelSolid(int x, int y) const {
    // A pixel is solid if its alpha is greater than a threshold
    const Uint8 ALPHA_THRESHOLD = 128;
    return GetPixelAlpha(x, y) > ALPHA_THRESHOLD;
}

bool Terrain::IsCircleSolid(const Vector2& center, float radius) const {
    // Check if any pixel in the circle is solid
    int minX = std::max(0, (int)(center.x - radius));
    int maxX = std::min(m_width - 1, (int)(center.x + radius));
    int minY = std::max(0, (int)(center.y - radius));
    int maxY = std::min(m_height - 1, (int)(center.y + radius));

    float radiusSq = radius * radius;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dx = x - center.x;
            float dy = y - center.y;
            float distSq = dx * dx + dy * dy;

            if (distSq <= radiusSq && IsPixelSolid(x, y)) {
                return true;
            }
        }
    }

    return false;
}

void Terrain::DestroyCircle(const Vector2& center, float radius) {
    if (!m_surface) return;

    int minX = std::max(0, (int)(center.x - radius));
    int maxX = std::min(m_width - 1, (int)(center.x + radius));
    int minY = std::max(0, (int)(center.y - radius));
    int maxY = std::min(m_height - 1, (int)(center.y + radius));

    float radiusSq = radius * radius;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dx = x - center.x;
            float dy = y - center.y;
            float distSq = dx * dx + dy * dy;

            if (distSq <= radiusSq) {
                SetPixelTransparent(x, y);
            }
        }
    }

    m_needsTextureUpdate = true;
}

int Terrain::FindTopSolidPixel(int x, int startY) const {
    if (!IsInBounds(x, 0)) return -1;

    // Search downward from startY to find the first solid pixel
    for (int y = startY; y < m_height; ++y) {
        if (IsPixelSolid(x, y)) {
            return y;
        }
    }

    return -1; // No solid pixel found
}

void Terrain::UpdateTexture(Renderer* renderer) {
    if (!m_surface) return;

    // Destroy old texture
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    // Create new texture from surface
    m_texture = SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), m_surface);
    if (!m_texture) {
        std::cerr << "Failed to create terrain texture from surface" << std::endl;
    }
}
