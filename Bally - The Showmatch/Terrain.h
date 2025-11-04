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
    
    // Find the ground surface (top of terrain platform) at a given x position
    // Searches from bottom up to find the top surface of terrain, avoiding ceilings
    int FindGroundSurface(int x) const;
    
    // Find the ground surface with a small area check (checks multiple x positions)
    // This ensures we find valid ground even if there's a small gap at the exact x position
    int FindGroundSurfaceArea(int x, int searchRadius = 5) const;
    
    // Check if terrain has solid ground below a position (to avoid floating platforms)
    // Returns true if there's solid terrain below the given Y position at X
    bool HasSolidGroundBelow(int x, int y, int minDepth = 20) const;
    
    // Find solid ground surface (not floating) - returns -1 if no solid ground found
    // Prefers terrain that has solid ground below it, avoiding floating platforms
    int FindSolidGroundSurface(int x, int searchRadius = 10) const;
    
    // Find a valid spawn position near target X - searches horizontally for valid ground
    // Returns true if valid position found, with spawnX and spawnY set
    bool FindValidSpawnPosition(int targetX, int searchRange, float playerRadius, int& outSpawnX, int& outSpawnY) const;

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
