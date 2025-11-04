#pragma once

#include <vector>
#include <string>
#include <functional>
#include <SDL3/SDL.h>
#include "Vector2.h"
#include "Renderer.h"

enum class GameState {
    MAIN_MENU,
    GAME_MODE_SELECTION,
    PLAYER_COUNT_SELECTION,
    MAP_SELECTION,
    SETTINGS,
    SOUND_SETTINGS,
    IN_GAME,
    PAUSED,
    GAME_OVER
};

enum class GameMode {
    TEAM_2V2,
    FREE_FOR_ALL
};

struct MenuButton {
    std::string text;
    Vector2 position;
    Vector2 size;
    std::function<void()> onClick;

    bool isVisible;
    SDL_Texture* texture = nullptr;  // Button texture
    int textureIndex = 0;  // Index for looking up texture dimensions
    bool usesSmallTexture = false;  // Whether this button uses small variant
};

class Menu {
public:
    Menu(Renderer* renderer);
    ~Menu();

    void Update(float deltaTime, const Vector2& mousePosition, bool mouseClicked);
    void Render();

    // State management
    void SetState(GameState state);
    GameState GetState() const { return m_currentState; }
    
    // Game configuration
    GameMode GetGameMode() const { return m_gameMode; }
    int GetPlayerCount() const { return m_playerCount; }
    int GetSelectedMapIndex() const { return m_selectedMapIndex; }

    // Map management
    void SetAvailableMaps(const std::vector<std::string>& mapNames) { m_availableMaps = mapNames; }

    // Settings
    float GetMasterVolume() const { return m_masterVolume; }
    float GetSFXVolume() const { return m_sfxVolume; }
    float GetMusicVolume() const { return m_musicVolume; }
    
    // Callbacks
    void SetOnStartGame(std::function<void()> callback) { m_onStartGame = callback; }
    void SetOnExit(std::function<void()> callback) { m_onExit = callback; }

private:
    Renderer* m_renderer;
    GameState m_currentState;
    GameState m_previousState;  // Track previous state for back navigation
    GameMode m_gameMode;
    int m_playerCount;
    int m_selectedMapIndex;
    std::vector<std::string> m_availableMaps;

    // Settings
    float m_masterVolume;
    float m_sfxVolume;
    float m_musicVolume;
    
    // UI
    std::vector<MenuButton> m_buttons;
    Vector2 m_mousePosition;
    bool m_mouseClicked;
    
    // Background image
    SDL_Texture* m_backgroundTexture;
    bool m_backgroundLoaded;
    
    // Button textures
    SDL_Texture* m_buttonTextures[4];  // aqua, orange, pink, purple
    SDL_Texture* m_smallButtonTextures[4];  // small variants
    int m_buttonTextureWidths[4];  // Texture widths for aspect ratio
    int m_buttonTextureHeights[4];  // Texture heights for aspect ratio
    int m_smallButtonTextureWidths[4];  // Small texture widths
    int m_smallButtonTextureHeights[4];  // Small texture heights
    int m_buttonTextureIndex;  // Current index for cycling through colors
    
    // Callbacks
    std::function<void()> m_onStartGame;
    std::function<void()> m_onExit;
    
    // Menu creation methods
    void CreateMainMenu();
    void CreateGameModeMenu();
    void CreatePlayerCountMenu();
    void CreateMapSelectionMenu();
    void CreatePauseMenu();
    void CreateSettingsMenu();
    void CreateSoundSettingsMenu();
    
    // Button management
    void ClearButtons();
    void AddButton(const std::string& text, const Vector2& position, const Vector2& size,
                  std::function<void()> onClick,
                  int textureIndex = -1);  // -1 means auto-assign (cycle), 0=aqua, 1=orange, 2=pink, 3=purple
    void HandleButtonClicks();
    
    // Rendering
    void LoadBackground();
    void LoadButtonTextures();
    void DrawBackground();
    void DrawTitle();
    void DrawButtons();
    void DrawSlider(const Vector2& position, const Vector2& size, float value, 
                   const std::string& label, std::function<void(float)> onValueChanged);
    
    // Constants
    static constexpr float BUTTON_WIDTH = 200.0f;
    static constexpr float BUTTON_HEIGHT = 50.0f;
    static constexpr float BUTTON_SPACING = 60.0f;
    static constexpr float SLIDER_WIDTH = 200.0f;
    static constexpr float SLIDER_HEIGHT = 20.0f;
};

