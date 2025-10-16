#include "Map.h"
#include "Renderer.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

Map::Map() : m_backgroundSurface(nullptr), m_backgroundTexture(nullptr), m_needsBackgroundUpdate(false) {
    m_terrain = std::make_unique<Terrain>();
}

Map::~Map() {
    if (m_backgroundSurface) {
        SDL_DestroySurface(m_backgroundSurface);
        m_backgroundSurface = nullptr;
    }
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
}

bool Map::LoadFromFolder(const std::string& folderPath) {
    m_folderPath = folderPath;

    // Extract map name from folder path
    fs::path path(folderPath);
    m_name = path.filename().string();

    std::cout << "Loading map: " << m_name.c_str() << " from " << folderPath.c_str() << std::endl;

    // Construct paths to terrain and background
    std::string terrainPath = folderPath + "/terrain.png";
    std::string backgroundPath = folderPath + "/background.png";

    // Load terrain (required)
    if (!m_terrain->LoadFromImage(terrainPath)) {
        std::cerr << "Failed to load terrain for map: " << m_name.c_str() << std::endl;
        return false;
    }

    // Load background (optional)
    LoadBackground(backgroundPath);

    std::cout << "Map loaded successfully: " << m_name.c_str() << std::endl;
    return true;
}

void Map::LoadBackground(const std::string& backgroundPath) {
    // Try to load background image
    SDL_Surface* loadedSurface = IMG_Load(backgroundPath.c_str());
    if (!loadedSurface) {
        std::cout << "No background image found for map, using solid color. (" << backgroundPath.c_str() << ")" << std::endl;
        return;
    }

    // Convert to RGBA format
    m_backgroundSurface = SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loadedSurface);

    if (!m_backgroundSurface) {
        std::cerr << "Failed to convert background surface" << std::endl;
        return;
    }

    m_needsBackgroundUpdate = true;
    std::cout << "Background loaded: " << backgroundPath.c_str() << std::endl;
}

void Map::UpdateBackgroundTexture(Renderer* renderer) {
    if (!m_backgroundSurface) return;

    // Destroy old texture
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }

    // Create new texture from surface
    m_backgroundTexture = SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), m_backgroundSurface);
    if (!m_backgroundTexture) {
        std::cerr << "Failed to create background texture" << std::endl;
    }
}

void Map::DrawBackground(Renderer* renderer) {
    if (!m_backgroundSurface) {
        // No background image, just clear with a sky blue color
        renderer->Clear(Color(135, 206, 235, 255)); // Sky blue
        return;
    }

    // Update texture if needed
    if (m_needsBackgroundUpdate) {
        UpdateBackgroundTexture(renderer);
        m_needsBackgroundUpdate = false;
    }

    if (m_backgroundTexture) {
        // Draw background scaled to window size
        Vector2 windowSize = renderer->GetWindowSize();
        SDL_FRect destRect = { 0, 0, windowSize.x, windowSize.y };
        SDL_RenderTexture(renderer->GetSDLRenderer(), m_backgroundTexture, nullptr, &destRect);
    }
}

void Map::DrawTerrain(Renderer* renderer) {
    if (m_terrain) {
        m_terrain->Draw(renderer);
    }
}

std::vector<MapInfo> Map::ScanAvailableMaps(const std::string& mapsDirectory) {
    std::vector<MapInfo> maps;

    // Get absolute path for debugging
    fs::path absolutePath = fs::absolute(mapsDirectory);
    std::cout << "Looking for maps in: " << mapsDirectory.c_str() << std::endl;
    std::cout << "Absolute path: " << absolutePath.string().c_str() << std::endl;
    std::cout << "Current working directory: " << fs::current_path().string().c_str() << std::endl;

    // Check if maps directory exists
    if (!fs::exists(mapsDirectory) || !fs::is_directory(mapsDirectory)) {
        std::cout << "Maps directory not found!" << std::endl;
        std::cout << "Please create folder at: " << absolutePath.string().c_str() << std::endl;
        return maps;
    }

    // Scan through all subdirectories in maps folder
    for (const auto& entry : fs::directory_iterator(mapsDirectory)) {
        if (entry.is_directory()) {
            std::string folderPath = entry.path().string();
            std::string terrainPath = folderPath + "/terrain.png";
            std::string backgroundPath = folderPath + "/background.png";

            // Check if terrain.png exists
            if (fs::exists(terrainPath)) {
                MapInfo info;
                info.name = entry.path().filename().string();
                info.folderPath = folderPath;
                info.terrainPath = terrainPath;
                info.backgroundPath = backgroundPath;

                maps.push_back(info);
                std::cout << "Found map: " << info.name.c_str() << " at " << folderPath.c_str() << std::endl;
            }
        }
    }

    std::cout << "Total maps found: " << maps.size() << std::endl;
    return maps;
}
