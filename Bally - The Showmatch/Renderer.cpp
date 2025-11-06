#include "Renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Renderer::Renderer(SDL_Window* window) : m_renderer(nullptr), m_window(window), m_windowWidth(1200), m_windowHeight(800), m_cameraOffset(0, 0) {
    SDL_GetWindowSize(window, &m_windowWidth, &m_windowHeight);
}

Renderer::~Renderer() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    if (TTF_WasInit()) {
        TTF_Quit();
    }
}

bool Renderer::Initialize() {
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    if (!TTF_Init()) {
        return false;
    }

    if (!m_font) {
        const char* candidateFonts[] = {
            "../fonts/PixelifySans-Regular.ttf",
            "fonts/PixelifySans-Regular.ttf",    
            "../fonts/PixelifySans-Medium.ttf",
            "fonts/PixelifySans-Medium.ttf",
            "../fonts/PixelifySans-SemiBold.ttf",
            "fonts/PixelifySans-SemiBold.ttf",
            "../fonts/PixelifySans-Bold.ttf",
            "fonts/PixelifySans-Bold.ttf",
            // Fallback to Windows fonts if Pixelify Sans not found
            "C:\\Windows\\Fonts\\segoeui.ttf",
            "C:\\Windows\\Fonts\\arial.ttf",
            "C:\\Windows\\Fonts\\tahoma.ttf"
        };
        for (const char* path : candidateFonts) {
            if (LoadFont(path, 16)) {
                std::cout << "Successfully loaded font: " << path << std::endl;
                break;
            }
        }
        if (!m_font) {
            std::cerr << "Warning: Failed to load any font! Text rendering will not work." << std::endl;
        }
    }
    return true;
}

void Renderer::BeginFrame() {
    Clear();
}

void Renderer::EndFrame() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::SetDrawColor(const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
}

void Renderer::Clear(const Color& color) {
    SetDrawColor(color);
    SDL_RenderClear(m_renderer);
}

void Renderer::DrawCircle(const Vector2& center, float radius, const Color& color) {
    SetDrawColor(color);

    // Apply camera offset
    int centerX = static_cast<int>(center.x - m_cameraOffset.x);
    int centerY = static_cast<int>(center.y - m_cameraOffset.y);
    int r = static_cast<int>(radius);

    for (int y = -r; y <= r; ++y) {
        for (int x = -r; x <= r; ++x) {
            if (x * x + y * y <= r * r) {
                SDL_RenderPoint(m_renderer, centerX + x, centerY + y);
            }
        }
    }

    // Draw outline
    Color outlineColor(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, color.a);
    SetDrawColor(outlineColor);

    // Draw circle outline
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        SDL_RenderPoint(m_renderer, centerX + x, centerY + y);
        SDL_RenderPoint(m_renderer, centerX + y, centerY + x);
        SDL_RenderPoint(m_renderer, centerX - y, centerY + x);
        SDL_RenderPoint(m_renderer, centerX - x, centerY + y);
        SDL_RenderPoint(m_renderer, centerX - x, centerY - y);
        SDL_RenderPoint(m_renderer, centerX - y, centerY - x);
        SDL_RenderPoint(m_renderer, centerX + y, centerY - x);
        SDL_RenderPoint(m_renderer, centerX + x, centerY - y);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void Renderer::DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness) {
    SetDrawColor(color);

    // Apply camera offset
    Vector2 adjustedStart = start - m_cameraOffset;
    Vector2 adjustedEnd = end - m_cameraOffset;

    // Simple line drawing with thickness
    Vector2 direction = adjustedEnd - adjustedStart;
    float length = direction.Length();
    if (length == 0) return;

    direction = direction * (1.0f / length);
    Vector2 perpendicular(-direction.y, direction.x);

    for (float t = 0; t <= length; t += 0.5f) {
        Vector2 point = adjustedStart + direction * t;

        // Draw thickness
        for (float offset = -thickness / 2; offset <= thickness / 2; offset += 0.5f) {
            Vector2 thickPoint = point + perpendicular * offset;
            SDL_RenderPoint(m_renderer, static_cast<int>(thickPoint.x), static_cast<int>(thickPoint.y));
        }
    }
}

void Renderer::DrawRect(const Vector2& position, float width, float height, const Color& color, bool filled) {
    SetDrawColor(color);

    // Apply camera offset
    SDL_FRect rect = {
        position.x - m_cameraOffset.x, position.y - m_cameraOffset.y, width, height
    };

    if (filled) {
        SDL_RenderFillRect(m_renderer, &rect);
    }
    else {
        SDL_RenderRect(m_renderer, &rect);
    }
}

void Renderer::DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) {
    SetDrawColor(color);

    // Draw triangle outline
    DrawLine(p1, p2, color, 2.0f);
    DrawLine(p2, p3, color, 2.0f);
    DrawLine(p3, p1, color, 2.0f);
}

void Renderer::DrawPowerIndicator(const Vector2& position, float power, float maxPower) {
    float normalizedPower = clamp(power / maxPower, 0.0f, 1.0f);

    // Background
    DrawRect(position, 100, 10, Color(50, 50, 50, 255));

    // Power bar
    Color powerColor;
    if (normalizedPower < 0.3f) {
        powerColor = Color(0, 255, 0, 255); // Green
    }
    else if (normalizedPower < 0.7f) {
        powerColor = Color(255, 255, 0, 255); // Yellow
    }
    else {
        powerColor = Color(255, 0, 0, 255); // Red
    }

    DrawRect(position, 100 * normalizedPower, 10, powerColor);

    // Outline
    DrawRect(position, 100, 10, Color(255, 255, 255, 255), false);
}

void Renderer::DrawHealthBar(const Vector2& position, float health, float maxHealth, float width, float height, const Color& teamHealthColor) {
    float normalizedHealth = clamp(health / maxHealth, 0.0f, 1.0f);

    // Background
    DrawRect(position, width, height, Color(100, 0, 0, 255));

    // Health bar color
    Color healthColor;
    if (teamHealthColor.r != 255 || teamHealthColor.g != 255 || teamHealthColor.b != 255 || teamHealthColor.a != 255) {
        // Use team color if provided (valid team color)
        healthColor = teamHealthColor;
    } else {
        // Default: color based on health percentage
        if (normalizedHealth > 0.6f) {
            healthColor = Color(0, 255, 0, 255); // Green
        }
        else if (normalizedHealth > 0.3f) {
            healthColor = Color(255, 255, 0, 255); // Yellow
        }
        else {
            healthColor = Color(255, 0, 0, 255); // Red
        }
    }

    DrawRect(position, width * normalizedHealth, height, healthColor);

    // Outline
    DrawRect(position, width, height, Color(255, 255, 255, 255), false);
}

void Renderer::DrawText(const Vector2& position, const char* text, const Color& color) {
    if (!text || !*text) return;
    if (!m_font) return; // font must be loaded via LoadFont

    SDL_Color sdlColor{ color.r, color.g, color.b, color.a };
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(
        m_font,
        text,
        static_cast<size_t>(SDL_strlen(text)),
        sdlColor,
        0
    );
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }

    // Apply camera offset
    SDL_FRect dst{ position.x - m_cameraOffset.x, position.y - m_cameraOffset.y, 
                   static_cast<float>(surface->w), static_cast<float>(surface->h) };
    SDL_RenderTexture(m_renderer, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void Renderer::GetTextSize(const char* text, int* width, int* height) const {
    if (!text || !*text || !m_font) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    // Render to surface to get dimensions (SDL3 compatible approach)
    SDL_Color sdlColor{ 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(
        m_font,
        text,
        static_cast<size_t>(SDL_strlen(text)),
        sdlColor,
        0
    );
    if (surface) {
        if (width) *width = surface->w;
        if (height) *height = surface->h;
        SDL_DestroySurface(surface);
    } else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

bool Renderer::LoadFont(const char* fontPath, int pointSize) {
    if (!TTF_WasInit()) {
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    m_font = TTF_OpenFont(fontPath, pointSize);
    if (!m_font) {
        std::cerr << "Failed to load font: " << fontPath << " - " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void Renderer::DrawPlatform(const Vector2& position, float width, float height) {
    // Platform base
    DrawRect(position, width, height, Color(139, 69, 19, 255)); // Brown

    // Platform top
    DrawRect(position, width, height * 0.2f, Color(160, 82, 45, 255)); // Lighter brown

    // Platform shadow
    DrawRect(Vector2(position.x + 5, position.y + height), width, height * 0.1f, Color(0, 0, 0, 100));
}
