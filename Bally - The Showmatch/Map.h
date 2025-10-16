#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Terrain.h"

class Renderer;

struct MapInfo {
    std::string name;           // Map display name
    std::string folderPath;     // Path to map folder
    std::string terrainPath;    // Path to terrain.png
    std::string backgroundPath; // Path to background.png
};

class Map {
public:
    Map();
    ~Map();

    // Load a map from a folder
    bool LoadFromFolder(const std::string& folderPath);

    // Get map info
    const std::string& GetName() const { return m_name; }
    const std::string& GetFolderPath() const { return m_folderPath; }

    // Get terrain
    Terrain* GetTerrain() { return m_terrain.get(); }
    const Terrain* GetTerrain() const { return m_terrain.get(); }

    // Drawing
    void DrawBackground(Renderer* renderer);
    void DrawTerrain(Renderer* renderer);

    // Check if map is valid
    bool IsValid() const { return m_terrain && m_terrain->GetSurface() != nullptr; }

    // Static helper: Scan for all available maps in maps directory
    static std::vector<MapInfo> ScanAvailableMaps(const std::string& mapsDirectory = "maps");

private:
    std::string m_name;
    std::string m_folderPath;

    std::unique_ptr<Terrain> m_terrain;
    SDL_Surface* m_backgroundSurface;
    SDL_Texture* m_backgroundTexture;
    bool m_needsBackgroundUpdate;

    void LoadBackground(const std::string& backgroundPath);
    void UpdateBackgroundTexture(Renderer* renderer);
};
