#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include "Vector2.h"
#include <SDL3_ttf/SDL_ttf.h>

struct Color {
    Uint8 r, g, b, a;
    Color(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255) : r(r), g(g), b(b), a(a) {}
};

class Renderer {
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    bool Initialize();
    void BeginFrame();
    void EndFrame();

    // Drawing primitives
    void DrawCircle(const Vector2& center, float radius, const Color& color);
    void DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness = 1.0f);
    void DrawRect(const Vector2& position, float width, float height, const Color& color, bool filled = true);
    void DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color);

    // UI elements
    void DrawPowerIndicator(const Vector2& position, float power, float maxPower);
    void DrawAngleIndicator(const Vector2& position, float angle, float length);
    void DrawHealthBar(const Vector2& position, float health, float maxHealth, float width, float height, const Color& healthColor = Color(-1, -1, -1, -1));
    void DrawText(const Vector2& position, const char* text, const Color& color);
    void GetTextSize(const char* text, int* width, int* height) const;

    bool LoadFont(const char* fontPath, int pointSize);

    // Game specific
    void DrawPlatform(const Vector2& position, float width, float height);

    void SetDrawColor(const Color& color);
    void Clear(const Color& color = Color(50, 50, 50, 255));

    SDL_Renderer* GetSDLRenderer() const { return m_renderer; }
    TTF_Font* GetFont() const { return m_font; }
    Vector2 GetWindowSize() const { return Vector2(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)); }

    // Camera offset
    void SetCameraOffset(const Vector2& offset) { m_cameraOffset = offset; }
    Vector2 GetCameraOffset() const { return m_cameraOffset; }

private:
    SDL_Renderer* m_renderer;
    SDL_Window* m_window;
    int m_windowWidth;
    int m_windowHeight;
    TTF_Font* m_font = nullptr;
    Vector2 m_cameraOffset; // Camera offset for scrolling
};

