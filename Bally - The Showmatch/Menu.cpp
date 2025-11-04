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
      m_backgroundTexture(nullptr), m_backgroundLoaded(false),
      m_buttonTextureIndex(0) {
    // Initialize button textures to nullptr
    for (int i = 0; i < 4; ++i) {
        m_buttonTextures[i] = nullptr;
        m_smallButtonTextures[i] = nullptr;
        m_buttonTextureWidths[i] = 0;
        m_buttonTextureHeights[i] = 0;
        m_smallButtonTextureWidths[i] = 0;
        m_smallButtonTextureHeights[i] = 0;
    }
    LoadBackground();
    LoadButtonTextures();
    CreateMainMenu();
}

Menu::~Menu() {
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
    
    // Clean up button textures
    for (int i = 0; i < 4; ++i) {
        if (m_buttonTextures[i]) {
            SDL_DestroyTexture(m_buttonTextures[i]);
            m_buttonTextures[i] = nullptr;
        }
        if (m_smallButtonTextures[i]) {
            SDL_DestroyTexture(m_smallButtonTextures[i]);
            m_smallButtonTextures[i] = nullptr;
        }
    }
}

void Menu::Update(float deltaTime, const Vector2& mousePosition, bool mouseClicked) {
    m_mousePosition = mousePosition;
    m_mouseClicked = mouseClicked;
    
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
              [this]() { SetState(GameState::GAME_MODE_SELECTION); });
    
    // Settings button - centered horizontally
    AddButton("Settings", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::SETTINGS); });
    
    // Exit button - centered horizontally (pink)
    AddButton("Exit", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { if (m_onExit) m_onExit(); }, 2);  // 2 = pink
}

void Menu::CreateGameModeMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;

    // Team 2v2 button - centered horizontally
    AddButton("Team Mode (2v2)", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() {
                  m_gameMode = GameMode::TEAM_2V2;
                  m_playerCount = 4;
                  SetState(GameState::MAP_SELECTION);
              });
    
    // Free for All button - centered horizontally
    AddButton("Free for All", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::PLAYER_COUNT_SELECTION); });
    
    // Back button - centered horizontally (pink)
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::MAIN_MENU); }, 2);  // 2 = pink
}

void Menu::CreatePlayerCountMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;

    // Player count buttons - centered horizontally (orange) - 2, 3, 4 players
    int playerOptions[] = {2, 3, 4};
    for (int idx = 0; idx < 3; ++idx) {
        int i = playerOptions[idx];
        std::string buttonText = std::to_string(i) + " Player" + (i > 1 ? "s" : "");
        AddButton(buttonText, Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + idx * BUTTON_SPACING),
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  [this, i]() {
                      m_gameMode = GameMode::FREE_FOR_ALL;
                      m_playerCount = i;
                      SetState(GameState::MAP_SELECTION);
                  }, 1);  // 1 = orange
    }

    // Back button - centered horizontally (pink)
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + 3 * BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::GAME_MODE_SELECTION); }, 2);  // 2 = pink
}

void Menu::CreateMapSelectionMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;

    // If no maps available, show error message
    if (m_availableMaps.empty()) {
        AddButton("No maps found!", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  []() { /* Error */ });

        AddButton("Using default map", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  []() { /* Info */ });

        AddButton("Start with Default", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  [this]() {
                      m_selectedMapIndex = -1; // Default map
                      if (m_onStartGame) m_onStartGame();
                  });
        
        // Back button below error messages (pink)
        float backButtonY = startY + BUTTON_SPACING * 3 + 20.0f;
        AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, backButtonY),
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  [this]() { SetState(m_previousState); }, 2);  // 2 = pink, go back to previous state
    }
    else {
        // Display available maps in columns (5 maps per column)
        const int MAPS_PER_COLUMN = 5;
        int totalMaps = static_cast<int>(m_availableMaps.size());
        int numColumns = (totalMaps + MAPS_PER_COLUMN - 1) / MAPS_PER_COLUMN;  // Ceiling division
        
        // Calculate column width (button width + spacing between columns)
        float columnSpacing = 50.0f;
        float totalColumnsWidth = (BUTTON_WIDTH * numColumns) + (columnSpacing * (numColumns - 1));
        float columnsStartX = screenCenterX - totalColumnsWidth / 2.0f + BUTTON_WIDTH / 2.0f;
        
        // Map buttons start at 400px (startY)
        float mapStartY = startY;
        
        // Display maps in columns (orange)
        int maxRow = 0;  // Track the bottommost row to position back button
        for (size_t i = 0; i < m_availableMaps.size(); ++i) {
            int column = static_cast<int>(i) / MAPS_PER_COLUMN;
            int row = static_cast<int>(i) % MAPS_PER_COLUMN;
            
            if (row > maxRow) maxRow = row;
            
            float x = columnsStartX + column * (BUTTON_WIDTH + columnSpacing);
            float y = mapStartY + row * BUTTON_SPACING;
            
            AddButton(m_availableMaps[i], Vector2(x - BUTTON_WIDTH / 2.0f, y),
                      Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                      [this, i]() {
                          m_selectedMapIndex = static_cast<int>(i);
                          if (m_onStartGame) m_onStartGame();
                      }, 1);  // 1 = orange
        }
        
        // Back button - positioned below the maps (pink)
        float backButtonY = mapStartY + (maxRow + 1) * BUTTON_SPACING + 20.0f;  // Position below map buttons with spacing
        AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, backButtonY),
                  Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
                  [this]() { SetState(m_previousState); }, 2);  // 2 = pink, go back to previous state
    }
}

void Menu::CreatePauseMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;

    // Resume button - centered horizontally
    AddButton("Resume", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::IN_GAME); });

    // Settings button - centered horizontally
    AddButton("Settings", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::SETTINGS); });

    // Exit to Main Menu button - centered horizontally (pink)
    AddButton("Exit to Main Menu", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::MAIN_MENU); }, 2);  // 2 = pink
}

void Menu::CreateSettingsMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;

    // Sound Settings button - centered horizontally
    AddButton("Sound Settings", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::SOUND_SETTINGS); });

    // Back button - returns to previous state (MAIN_MENU or PAUSED) - centered horizontally (pink)
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING),
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() {
                  // Return to previous state (could be MAIN_MENU or PAUSED)
                  if (m_previousState == GameState::PAUSED) {
                      SetState(GameState::PAUSED);
                  } else {
                      SetState(GameState::MAIN_MENU);
                  }
              }, 2);  // 2 = pink
}

void Menu::CreateSoundSettingsMenu() {
    float screenCenterX = 600.0f;
    float startY = 400.0f;
    
    // Volume sliders (simplified as buttons for now) - centered horizontally
    AddButton("Master Volume: " + std::to_string(static_cast<int>(m_masterVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { 
                  m_masterVolume = std::fmod(m_masterVolume + 0.1f, 1.1f);
                  if (m_masterVolume > 1.0f) m_masterVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("SFX Volume: " + std::to_string(static_cast<int>(m_sfxVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { 
                  m_sfxVolume = std::fmod(m_sfxVolume + 0.1f, 1.1f);
                  if (m_sfxVolume > 1.0f) m_sfxVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    AddButton("Music Volume: " + std::to_string(static_cast<int>(m_musicVolume * 100)) + "%", 
              Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 2), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { 
                  m_musicVolume = std::fmod(m_musicVolume + 0.1f, 1.1f);
                  if (m_musicVolume > 1.0f) m_musicVolume = 0.0f;
                  SetState(GameState::SOUND_SETTINGS); // Refresh display
              });
    
    // Back button - centered horizontally (pink)
    AddButton("Back", Vector2(screenCenterX - BUTTON_WIDTH / 2.0f, startY + BUTTON_SPACING * 3), 
              Vector2(BUTTON_WIDTH, BUTTON_HEIGHT),
              [this]() { SetState(GameState::SETTINGS); }, 2);  // 2 = pink
}

void Menu::ClearButtons() {
    m_buttons.clear();
    m_buttonTextureIndex = 0;  // Reset texture index for new menu
}

void Menu::AddButton(const std::string& text, const Vector2& position, const Vector2& size,
                    std::function<void()> onClick,
                    int textureIndex) {
    MenuButton button;
    button.text = text;
    button.position = position;
    button.size = size;
    button.onClick = onClick;
    button.isVisible = true;
    
    // Assign button texture based on size (use small variants for smaller buttons)
    bool useSmall = (size.x < BUTTON_WIDTH * 0.8f || size.y < BUTTON_HEIGHT * 0.8f);
    
    // Use specified texture index, or auto-assign by cycling between aqua (0) and purple (3)
    if (textureIndex >= 0 && textureIndex < 4) {
        button.textureIndex = textureIndex;
    } else {
        // Auto-assign: cycle between aqua (0) and purple (3)
        button.textureIndex = (m_buttonTextureIndex % 2 == 0) ? 0 : 3;
        m_buttonTextureIndex = (m_buttonTextureIndex + 1) % 2;  // Toggle between 0 and 1 for index selection
    }
    
    button.usesSmallTexture = useSmall;
    button.texture = useSmall ? m_smallButtonTextures[button.textureIndex] : m_buttonTextures[button.textureIndex];
    
    m_buttons.push_back(button);
}

//void Menu::UpdateButtonHover(const Vector2& mousePosition) {
//    for (auto& button : m_buttons) {
//        if (button.isVisible) {
//            bool wasHovered = button.isHovered;
//            button.isHovered = (mousePosition.x >= button.position.x && 
//                               mousePosition.x <= button.position.x + button.size.x &&
//                               mousePosition.y >= button.position.y && 
//                               mousePosition.y <= button.position.y + button.size.y);
//        }
//    }
//}

void Menu::HandleButtonClicks() {
    for (auto& button : m_buttons) {
        if (button.isVisible && button.onClick) {
            // Check if mouse is over button
            bool mouseOverButton = (m_mousePosition.x >= button.position.x && 
                                   m_mousePosition.x <= button.position.x + button.size.x &&
                                   m_mousePosition.y >= button.position.y && 
                                   m_mousePosition.y <= button.position.y + button.size.y);
            
            if (mouseOverButton) {
                // Debug: Print which button was clicked
                std::cout << "Button clicked: " << button.text << std::endl;
                button.onClick();
                break; // Only handle one click per frame
            }
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

void Menu::LoadButtonTextures() {
    // Button texture paths - order: aqua, orange, pink, purple
    const char* buttonPaths[] = {
        "../assets/buttons/aqua_button.png",
        "../assets/buttons/orange_button.png",
        "../assets/buttons/pink_button.png",
        "../assets/buttons/purple_button.png"
    };
    
    const char* smallButtonPaths[] = {
        "../assets/buttons/small_aqua_button.png",
        "../assets/buttons/small_orange_button.png",
        "../assets/buttons/small_pink_button.png",
        "../assets/buttons/small_purple_button.png"
    };
    
    // Load regular button textures
    for (int i = 0; i < 4; ++i) {
        SDL_Surface* surface = IMG_Load(buttonPaths[i]);
        if (surface) {
            m_buttonTextureWidths[i] = surface->w;
            m_buttonTextureHeights[i] = surface->h;
            m_buttonTextures[i] = SDL_CreateTextureFromSurface(m_renderer->GetSDLRenderer(), surface);
            SDL_DestroySurface(surface);
            if (!m_buttonTextures[i]) {
                std::cerr << "Failed to create button texture " << i << ": " << SDL_GetError() << std::endl;
            }
        } else {
            std::cerr << "Failed to load button texture: " << buttonPaths[i] << " - " << SDL_GetError() << std::endl;
        }
    }
    
    // Load small button textures
    for (int i = 0; i < 4; ++i) {
        SDL_Surface* surface = IMG_Load(smallButtonPaths[i]);
        if (surface) {
            m_smallButtonTextureWidths[i] = surface->w;
            m_smallButtonTextureHeights[i] = surface->h;
            m_smallButtonTextures[i] = SDL_CreateTextureFromSurface(m_renderer->GetSDLRenderer(), surface);
            SDL_DestroySurface(surface);
            if (!m_smallButtonTextures[i]) {
                std::cerr << "Failed to create small button texture " << i << ": " << SDL_GetError() << std::endl;
            }
        } else {
            std::cerr << "Failed to load small button texture: " << smallButtonPaths[i] << " - " << SDL_GetError() << std::endl;
        }
    }
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
        
        // Draw button texture if available
        if (button.texture) {
            // Get texture dimensions to maintain aspect ratio
            int texWidth = button.usesSmallTexture ? 
                m_smallButtonTextureWidths[button.textureIndex] : 
                m_buttonTextureWidths[button.textureIndex];
            int texHeight = button.usesSmallTexture ? 
                m_smallButtonTextureHeights[button.textureIndex] : 
                m_buttonTextureHeights[button.textureIndex];
            
            if (texWidth > 0 && texHeight > 0) {
                // Calculate scale to maintain aspect ratio while fitting in button size
                float scaleX = button.size.x / static_cast<float>(texWidth);
                float scaleY = button.size.y / static_cast<float>(texHeight);
                float scale = std::min(scaleX, scaleY);  // Use smaller scale to fit both dimensions
                
                // Calculate centered destination rectangle maintaining aspect ratio
                float scaledWidth = static_cast<float>(texWidth) * scale;
                float scaledHeight = static_cast<float>(texHeight) * scale;
                float offsetX = (button.size.x - scaledWidth) * 0.5f;
                float offsetY = (button.size.y - scaledHeight) * 0.5f;
                
                SDL_FRect destRect = { 
                    button.position.x + offsetX, 
                    button.position.y + offsetY, 
                    scaledWidth, 
                    scaledHeight 
                };
                SDL_RenderTexture(m_renderer->GetSDLRenderer(), button.texture, nullptr, &destRect);
            } else {
                // Fallback: stretch to button size if dimensions unknown
                SDL_FRect destRect = { button.position.x, button.position.y, button.size.x, button.size.y };
                SDL_RenderTexture(m_renderer->GetSDLRenderer(), button.texture, nullptr, &destRect);
            }
        } else {
            // Fallback to gray rectangle if texture not loaded
            m_renderer->DrawRect(button.position, button.size.x, button.size.y, Color(150, 150, 150, 255));
            m_renderer->DrawRect(button.position, button.size.x, button.size.y, Color(255, 255, 255, 255), false);
        }
        
        // Draw button text (centered horizontally and vertically) in black
        // Get text dimensions to center it properly
        int textWidth = 0, textHeight = 0;
        m_renderer->GetTextSize(button.text.c_str(), &textWidth, &textHeight);
        
        if (textWidth > 0 && textHeight > 0) {
            // Center text both horizontally and vertically
            float textX = button.position.x + (button.size.x - static_cast<float>(textWidth)) * 0.5f;
            float textY = button.position.y + (button.size.y - static_cast<float>(textHeight)) * 0.5f;
            m_renderer->DrawText(Vector2(textX, textY), button.text.c_str(), Color(0, 0, 0, 255));
        } else {
            // Fallback: approximate center position
            Vector2 textPos = button.position + Vector2(
                (button.size.x - button.text.length() * 6.0f) * 0.5f,  // Approximate width: 6px per character
                (button.size.y - 16.0f) * 0.5f  // Approximate height: 16px
            );
            m_renderer->DrawText(textPos, button.text.c_str(), Color(0, 0, 0, 255));
        }
    }
    
    // Debug: Draw mouse position
    std::string mouseDebug = "Mouse: " + std::to_string(static_cast<int>(m_mousePosition.x)) + 
                           ", " + std::to_string(static_cast<int>(m_mousePosition.y));
    m_renderer->DrawText(Vector2(10, 10), mouseDebug.c_str(), Color(255, 255, 255, 255));
}
