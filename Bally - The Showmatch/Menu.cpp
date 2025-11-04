#include "Menu.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>

Menu::Menu(Renderer* renderer)
    : m_renderer(renderer), m_currentState(GameState::MAIN_MENU),
      m_previousState(GameState::MAIN_MENU),
      m_gameMode(GameMode::FREE_FOR_ALL), m_playerCount(4), m_selectedMapIndex(0),
      m_masterVolume(1.0f), m_sfxVolume(1.0f), m_musicVolume(1.0f),
      m_mousePosition(0, 0), m_mouseClicked(false),
      m_backgroundTexture(nullptr), m_backgroundLoaded(false) {
    LoadBackground();
    CreateMainMenu();
}

Menu::~Menu() {
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
}

void Menu::Update(float deltaTime, const Vector2& mousePosition, bool mouseClicked) {
    m_mousePosition = mousePosition;
    m_mouseClicked = mouseClicked;
    
    UpdateButtonHover(mousePosition);
    
    if (mouseClicked) {
        std::cout << "Mouse clicked at: " << mousePosition.x << ", " << mousePosition.y << std::endl;
        HandleButtonClicks();
    }
}

void Menu::Render() {
    DrawBackground();
    DrawButtons();
}

void Menu::SetState(GameState state) {
    // Track previous state for back navigation
    m_previousState = m_currentState;
    m_currentState = state;
    ClearButtons();

    switch (state) {
        case GameState::MAIN_MENU:
            CreateMainMenu();
            break;
        case GameState::GAME_MODE_SELECTION:
            CreateGameModeMenu();
            break;
        case GameState::PLAYER_COUNT_SELECTION:
            CreatePlayerCountMenu();
            break;
        case GameState::MAP_SELECTION:
            CreateMapSelectionMenu();
            break;
        case GameState::PAUSED:
            CreatePauseMenu();
            break;
        case GameState::SETTINGS:
            CreateSettingsMenu();
            break;
        case GameState::SOUND_SETTINGS:
            CreateSoundSettingsMenu();
            break;
        case GameState::KEYBIND_SETTINGS:
            CreateKeybindSettingsMenu();
            break;
        default:
            break;
    }
}

void Menu::CreateMainMenu() {
    // Center buttons horizontally (screen width is 1200, so center is 600)
    // Position buttons in lower half of screen starting at 600px
    float screenCenterX = 600.0f;
    float startY = 400.0f;
    
    // Start Game button - centered horizontally
    AddButton("Start Game", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { SetState(GameState::GAME_MODE_SELECTION); });
    
    // Multiplayer button (disabled for now) - centered horizontally
    AddButton("Multiplayer (Coming Soon)", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Disabled */ });
    
    // Settings button - centered horizontally
    AddButton("Settings", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::SETTINGS); });
    
    // Exit button - centered horizontally
    AddButton("Exit", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 3), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(200, 0, 0, 255), Color(255, 0, 0, 255),
              [this]() { if (m_onExit) m_onExit(); });
}

void Menu::CreateGameModeMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 60.0f;

    // Team 2v2 button - centered horizontally
    AddButton("Team Mode (2v2)", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() {
                  m_gameMode = GameMode::TEAM_2V2;
                  m_playerCount = 4;
                  SetState(GameState::MAP_SELECTION);
              });
    
    // Free for All button - centered horizontally
    AddButton("Free for All", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { SetState(GameState::PLAYER_COUNT_SELECTION); });
    
    // Back button - centered horizontally
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::MAIN_MENU); });
}

void Menu::CreatePlayerCountMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 90.0f;

    // Player count buttons - centered horizontally
    for (int i = 1; i <= 4; ++i) {
        std::string buttonText = std::to_string(i) + " Player" + (i > 1 ? "s" : "");
        AddButton(buttonText, Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + (i-1) * BUTTON_SPACING),
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  Color(0, 150, 0, 255), Color(0, 200, 0, 255),
                  [this, i]() {
                      m_gameMode = GameMode::FREE_FOR_ALL;
                      m_playerCount = i;
                      SetState(GameState::MAP_SELECTION);
                  });
    }

    // Back button - centered horizontally
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + 4 * BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::GAME_MODE_SELECTION); });
}

void Menu::CreateMapSelectionMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 120.0f;

    // If no maps available, show error message
    if (m_availableMaps.empty()) {
        AddButton("No maps found!", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  Color(200, 0, 0, 255), Color(200, 0, 0, 255),
                  []() { /* Error */ });

        AddButton("Using default map", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  Color(100, 100, 100, 255), Color(100, 100, 100, 255),
                  []() { /* Info */ });

        AddButton("Start with Default", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  Color(0, 150, 0, 255), Color(0, 200, 0, 255),
                  [this]() {
                      m_selectedMapIndex = -1; // Default map
                      if (m_onStartGame) m_onStartGame();
                  });
    }
    else {
        // Display available maps - centered horizontally
        for (size_t i = 0; i < m_availableMaps.size() && i < 5; ++i) {
            AddButton(m_availableMaps[i], Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + i * BUTTON_SPACING),
                      Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                      Color(0, 150, 0, 255), Color(0, 200, 0, 255),
                      [this, i]() {
                          m_selectedMapIndex = static_cast<int>(i);
                          if (m_onStartGame) m_onStartGame();
                      });
        }
    }

    // Back button - centered horizontally
    float backButtonOffset = m_availableMaps.empty() ? 3.0f : static_cast<float>(std::min(m_availableMaps.size() + 1, static_cast<size_t>(6)));
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + backButtonOffset * BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::PLAYER_COUNT_SELECTION); });
}

void Menu::CreatePauseMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 90.0f;

    // Resume button - centered horizontally
    AddButton("Resume", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { SetState(GameState::IN_GAME); });

    // Settings button - centered horizontally
    AddButton("Settings", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::SETTINGS); });

    // Exit to Main Menu button - centered horizontally
    AddButton("Exit to Main Menu", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(200, 0, 0, 255), Color(255, 0, 0, 255),
              [this]() { SetState(GameState::MAIN_MENU); });
}

void Menu::CreateSettingsMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 60.0f;

    // Sound Settings button - centered horizontally
    AddButton("Sound Settings", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::SOUND_SETTINGS); });

    // Keybind Settings button - centered horizontally
    AddButton("Keybind Settings", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::KEYBIND_SETTINGS); });

    // Back button - returns to previous state (MAIN_MENU or PAUSED) - centered horizontally
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() {
                  // Return to previous state (could be MAIN_MENU or PAUSED)
                  if (m_previousState == GameState::PAUSED) {
                      SetState(GameState::PAUSED);
                  } else {
                      SetState(GameState::MAIN_MENU);
                  }
              });
}

void Menu::CreateSoundSettingsMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 90.0f;
    
    // Volume sliders (simplified as buttons for now) - centered horizontally
    AddButton("Master Volume: " + std::to_string(static_cast<int>(m_masterVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_masterVolume = std::fmod(m_masterVolume + 0.1f, 1.1f);
                  if (m_masterVolume > 1.0f) m_masterVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("SFX Volume: " + std::to_string(static_cast<int>(m_sfxVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_sfxVolume = std::fmod(m_sfxVolume + 0.1f, 1.1f);
                  if (m_sfxVolume > 1.0f) m_sfxVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("Music Volume: " + std::to_string(static_cast<int>(m_musicVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_musicVolume = std::fmod(m_musicVolume + 0.1f, 1.1f);
                  if (m_musicVolume > 1.0f) m_musicVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    // Back button - centered horizontally
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 3), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::SETTINGS); });
}

void Menu::CreateKeybindSettingsMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f - 60.0f;
    
    // Keybind info (since all players use same keybinds) - centered horizontally
    AddButton("All players use same keybinds", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Info only */ });
    
    AddButton("(Players take turns)", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + 30), 
              Vector2(BUTTON_WIDTH, 30),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Info only */ });
    
    // Back button - centered horizontally
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::SETTINGS); });
}

void Menu::ClearButtons() {
    m_buttons.clear();
}

void Menu::AddButton(const std::string& text, const Vector2& position, const Vector2& size,
                    const Color& color, const Color& hoverColor, std::function<void()> onClick) {
    MenuButton button;
    button.text = text;
    button.position = position;
    button.size = size;
    button.color = color;
    button.hoverColor = hoverColor;
    button.onClick = onClick;
    button.isHovered = false;
    button.isVisible = true;
    m_buttons.push_back(button);
}

void Menu::UpdateButtonHover(const Vector2& mousePosition) {
    for (auto& button : m_buttons) {
        if (button.isVisible) {
            bool wasHovered = button.isHovered;
            button.isHovered = (mousePosition.x >= button.position.x && 
                               mousePosition.x <= button.position.x + button.size.x &&
                               mousePosition.y >= button.position.y && 
                               mousePosition.y <= button.position.y + button.size.y);
        }
    }
}

void Menu::HandleButtonClicks() {
    for (auto& button : m_buttons) {
        if (button.isVisible && button.isHovered && button.onClick) {
            // Debug: Print which button was clicked
            std::cout << "Button clicked: " << button.text << std::endl;
            button.onClick();
            break; // Only handle one click per frame
        }
    }
}

void Menu::LoadBackground() {
    if (m_backgroundLoaded) return;
    
    std::string backgroundPath = "../assets/main_screen_background.png";
    SDL_Surface* surface = IMG_Load(backgroundPath.c_str());
    if (!surface) {
        std::cerr << "Failed to load menu background: " << backgroundPath << " - " << SDL_GetError() << std::endl;
        m_backgroundLoaded = false;
        return;
    }
    
    m_backgroundTexture = SDL_CreateTextureFromSurface(m_renderer->GetSDLRenderer(), surface);
    if (!m_backgroundTexture) {
        std::cerr << "Failed to create background texture: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        m_backgroundLoaded = false;
        return;
    }
    
    SDL_DestroySurface(surface);
    m_backgroundLoaded = true;
    std::cout << "Menu background loaded successfully" << std::endl;
}

void Menu::DrawBackground() {
    // Draw background image if loaded
    if (m_backgroundTexture && m_backgroundLoaded) {
        SDL_FRect destRect = { 0.0f, 0.0f, 1200.0f, 800.0f };
        SDL_RenderTexture(m_renderer->GetSDLRenderer(), m_backgroundTexture, nullptr, &destRect);
    } else {
        // Fallback: Draw a semi-transparent dark background
        m_renderer->DrawRect(Vector2::Zero(), 1200, 800, Color(0, 0, 0, 180));
    }
}

void Menu::DrawButtons() {
    for (const auto& button : m_buttons) {
        if (!button.isVisible) continue;
        
        Color currentColor = button.isHovered ? button.hoverColor : button.color;
        
        // Draw button background
        m_renderer->DrawRect(button.position, button.size.x, button.size.y, currentColor);
        
        // Draw button outline
        m_renderer->DrawRect(button.position, button.size.x, button.size.y, Color(255, 255, 255, 255), false);
        
        // Draw button text (centered)
        Vector2 textPos = button.position + Vector2(10, button.size.y / 2 - 5);
        m_renderer->DrawText(textPos, button.text.c_str(), Color(255, 255, 255, 255));
    }
    
    // Debug: Draw mouse position
    std::string mouseDebug = "Mouse: " + std::to_string(static_cast<int>(m_mousePosition.x)) + 
                           ", " + std::to_string(static_cast<int>(m_mousePosition.y));
    m_renderer->DrawText(Vector2(10, 10), mouseDebug.c_str(), Color(255, 255, 255, 255));
}
