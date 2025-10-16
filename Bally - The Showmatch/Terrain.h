#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Vector2.h"

class Renderer;

class Terrain {
public:
    Terrain();
    ~Terrain();

    // Load terrain from PNG image
    bool LoadFromImage(const std::string& filepath);

    // Create a simple default terrain (for testing)
    void CreateDefaultTerrain(int width, int height);

    // Render the terrain
    void Draw(Renderer* renderer);

    // Collision detection - check if a point is solid
    bool IsPixelSolid(int x, int y) const;
    bool IsCircleSolid(const Vector2& center, float radius) const;

    // Terrain destruction - remove pixels in a circular area
    void DestroyCircle(const Vector2& center, float radius);

    // Get terrain dimensions
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // Get the terrain surface for rendering
    SDL_Surface* GetSurface() const { return m_surface; }

    // Find the highest solid pixel at a given x position (for standing on terrain)
    int FindTopSolidPixel(int x, int startY) const;

private:
    SDL_Surface* m_surface;
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
    bool m_needsTextureUpdate;

    // Helper to check if coordinates are in bounds
    bool IsInBounds(int x, int y) const;

    // Get pixel alpha value at position
    Uint8 GetPixelAlpha(int x, int y) const;

    // Set pixel to transparent
    void SetPixelTransparent(int x, int y);

    // Update texture from surface after destruction
    void UpdateTexture(Renderer* renderer);
};
