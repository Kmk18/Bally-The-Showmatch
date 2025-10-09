#include "Menu.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Menu::Menu(Renderer* renderer) 
    : m_renderer(renderer), m_currentState(GameState::MAIN_MENU), 
      m_gameMode(GameMode::FREE_FOR_ALL), m_playerCount(4),
      m_masterVolume(1.0f), m_sfxVolume(1.0f), m_musicVolume(1.0f),
      m_mousePosition(0, 0), m_mouseClicked(false) {
    CreateMainMenu();
}

Menu::~Menu() {
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
    DrawTitle();
    DrawButtons();
}

void Menu::SetState(GameState state) {
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
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 120);
    
    // Start Game button
    AddButton("Start Game", startPos, Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { SetState(GameState::GAME_MODE_SELECTION); });
    
    // Multiplayer button (disabled for now)
    AddButton("Multiplayer (Coming Soon)", startPos + Vector2(0, BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Disabled */ });
    
    // Settings button
    AddButton("Settings", startPos + Vector2(0, BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::SETTINGS); });
    
    // Exit button
    AddButton("Exit", startPos + Vector2(0, BUTTON_SPACING * 3), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(200, 0, 0, 255), Color(255, 0, 0, 255),
              [this]() { if (m_onExit) m_onExit(); });
}

void Menu::CreateGameModeMenu() {
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 60);
    
    // Team 2v2 button
    AddButton("Team Mode (2v2)", startPos, Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { 
                  m_gameMode = GameMode::TEAM_2V2; 
                  m_playerCount = 4;
                  if (m_onStartGame) m_onStartGame();
              });
    
    // Free for All button
    AddButton("Free for All", startPos + Vector2(0, BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 150, 0, 255), Color(0, 200, 0, 255),
              [this]() { SetState(GameState::PLAYER_COUNT_SELECTION); });
    
    // Back button
    AddButton("Back", startPos + Vector2(0, BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::MAIN_MENU); });
}

void Menu::CreatePlayerCountMenu() {
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 90);
    
    // Player count buttons
    for (int i = 1; i <= 4; ++i) {
        std::string buttonText = std::to_string(i) + " Player" + (i > 1 ? "s" : "");
        AddButton(buttonText, startPos + Vector2(0, (i-1) * BUTTON_SPACING), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  Color(0, 150, 0, 255), Color(0, 200, 0, 255),
                  [this, i]() { 
                      m_gameMode = GameMode::FREE_FOR_ALL; 
                      m_playerCount = i;
                      if (m_onStartGame) m_onStartGame();
                  });
    }
    
    // Back button
    AddButton("Back", startPos + Vector2(0, 4 * BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::GAME_MODE_SELECTION); });
}

void Menu::CreateSettingsMenu() {
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 60);
    
    // Sound Settings button
    AddButton("Sound Settings", startPos, Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::SOUND_SETTINGS); });
    
    // Keybind Settings button
    AddButton("Keybind Settings", startPos + Vector2(0, BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(0, 100, 200, 255), Color(0, 150, 255, 255),
              [this]() { SetState(GameState::KEYBIND_SETTINGS); });
    
    // Back button
    AddButton("Back", startPos + Vector2(0, BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::MAIN_MENU); });
}

void Menu::CreateSoundSettingsMenu() {
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 90);
    
    // Volume sliders (simplified as buttons for now)
    AddButton("Master Volume: " + std::to_string(static_cast<int>(m_masterVolume * 100)) + "%", 
              startPos, Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_masterVolume = std::fmod(m_masterVolume + 0.1f, 1.1f);
                  if (m_masterVolume > 1.0f) m_masterVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("SFX Volume: " + std::to_string(static_cast<int>(m_sfxVolume * 100)) + "%", 
              startPos + Vector2(0, BUTTON_SPACING), Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_sfxVolume = std::fmod(m_sfxVolume + 0.1f, 1.1f);
                  if (m_sfxVolume > 1.0f) m_sfxVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("Music Volume: " + std::to_string(static_cast<int>(m_musicVolume * 100)) + "%", 
              startPos + Vector2(0, BUTTON_SPACING * 2), Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(150, 150, 150, 255),
              [this]() { 
                  m_musicVolume = std::fmod(m_musicVolume + 0.1f, 1.1f);
                  if (m_musicVolume > 1.0f) m_musicVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    // Back button
    AddButton("Back", startPos + Vector2(0, BUTTON_SPACING * 3), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(150, 150, 150, 255), Color(200, 200, 200, 255),
              [this]() { SetState(GameState::SETTINGS); });
}

void Menu::CreateKeybindSettingsMenu() {
    Vector2 center(600, 400);
    Vector2 startPos = center - Vector2(0, 60);
    
    // Keybind info (since all players use same keybinds)
    AddButton("All players use same keybinds", startPos, Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Info only */ });
    
    AddButton("(Players take turns)", startPos + Vector2(0, 30), Vector2(BUTTON_WIDTH, 30),
              Color(100, 100, 100, 255), Color(100, 100, 100, 255),
              []() { /* Info only */ });
    
    // Back button
    AddButton("Back", startPos + Vector2(0, BUTTON_SPACING * 2), 
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

void Menu::DrawBackground() {
    // Draw a semi-transparent dark background
    m_renderer->DrawRect(Vector2::Zero(), 1200, 800, Color(0, 0, 0, 180));
}

void Menu::DrawTitle() {
    std::string title = "Bally - The Showmatch";
    Vector2 titlePos(600, 150);
    
    // Draw title background
    m_renderer->DrawRect(titlePos - Vector2(150, 20), 300, 40, Color(0, 0, 0, 128));
    
    // Draw title text (adjusted position)
    Vector2 textPos = titlePos - Vector2(120, 5);
    m_renderer->DrawText(textPos, title.c_str(), Color(255, 255, 0, 255));
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
