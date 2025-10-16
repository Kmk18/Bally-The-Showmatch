#include "Game.h"
#include "InputManager.h"
#include "Physics.h"
#include "UI.h"
#include <iostream>
#include <random>
#include <algorithm>

Game::Game() : m_window(nullptr), m_running(false), m_lastFrameTime(0.0f),
m_gameState(GameState::MAIN_MENU), m_gameMode(GameMode::FREE_FOR_ALL), m_numPlayers(4),
m_currentPlayerIndex(0), m_turnTimer(TURN_DURATION),
m_gameStarted(false), m_gameEnded(false), m_winnerId(-1) {
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    m_window = SDL_CreateWindow("Bally - The Showdown", 1200, 800, SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize systems
    m_renderer = std::make_unique<Renderer>(m_window);
    if (!m_renderer->Initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    m_inputManager = std::make_unique<InputManager>();
    m_physics = std::make_unique<Physics>();
    m_ui = std::make_unique<UI>(m_renderer.get());
    m_menu = std::make_unique<Menu>(m_renderer.get());

    // Scan for available maps
    // Path relative to the project source directory
    m_availableMaps = Map::ScanAvailableMaps("../maps");

    // Populate menu with map names
    std::vector<std::string> mapNames;
    for (const auto& mapInfo : m_availableMaps) {
        mapNames.push_back(mapInfo.name);
    }
    m_menu->SetAvailableMaps(mapNames);

    std::cout << "Found " << m_availableMaps.size() << " maps" << std::endl;

    // Set up menu callbacks
    m_menu->SetOnStartGame([this]() { StartGame(); });
    m_menu->SetOnExit([this]() { m_running = false; });

    m_running = true;
    return true;
}

void Game::CreatePlayers() {
    m_players.clear();

    std::vector<Color> playerColors = {
        Color(255, 100, 100, 255), // Red
        Color(100, 100, 255, 255), // Blue
        Color(100, 255, 100, 255), // Green
        Color(255, 255, 100, 255)  // Yellow
    };

    // Create players based on menu selection
    int numPlayers = m_numPlayers;
    float platformWidth = 800.0f;
    float platformHeight = 50.0f;
    float startY = 600.0f; // Above platform

    for (int i = 0; i < numPlayers; ++i) {
        float spacing = platformWidth / (numPlayers + 1);
        float x = 200.0f + spacing * (i + 1);
        Vector2 position(x, startY);

        auto player = std::make_unique<Player>(i, position, playerColors[i]);
        m_players.push_back(std::move(player));
    }

    // Set up input mappings for different players
    SetupPlayerInputs();
}

void Game::SetupPlayerInputs() {
    // All players use the same keybinds (Arrow keys + Space) since it's turn-based
    // No need for different keys as only one player moves at a time
    for (int i = 0; i < m_numPlayers; ++i) {
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::MOVE_LEFT, SDL_SCANCODE_LEFT);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::MOVE_RIGHT, SDL_SCANCODE_RIGHT);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::AIM_UP, SDL_SCANCODE_UP);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::AIM_DOWN, SDL_SCANCODE_DOWN);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::ADJUST_POWER, SDL_SCANCODE_SPACE);
    }
}

void Game::Run() {
    while (m_running) {
        float currentTime = SDL_GetTicks() / 1000.0f;
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;

        deltaTime = std::min(deltaTime, 1.0f / 30.0f);

        HandleEvents();
        Update(deltaTime);
        Render();

        SDL_Delay(16);
    }
}

void Game::Update(float deltaTime) {
    m_inputManager->Update();

    // Update menu if in menu state
    if (m_gameState == GameState::MAIN_MENU || m_gameState == GameState::GAME_MODE_SELECTION ||
        m_gameState == GameState::PLAYER_COUNT_SELECTION || m_gameState == GameState::MAP_SELECTION ||
        m_gameState == GameState::SETTINGS ||
        m_gameState == GameState::SOUND_SETTINGS || m_gameState == GameState::KEYBIND_SETTINGS ||
        m_gameState == GameState::PAUSED) {
        // Get mouse position and click state for menu
        Vector2 mousePos = m_inputManager->GetMousePosition();
        bool mouseClicked = m_inputManager->IsMouseButtonJustPressed(0); // Left mouse button
        m_menu->Update(deltaTime, mousePos, mouseClicked);

        // Sync game state with menu state (for pause menu navigation)
        if (m_gameState == GameState::PAUSED) {
            GameState menuState = m_menu->GetState();
            if (menuState == GameState::IN_GAME) {
                // Resume button was clicked
                m_gameState = GameState::IN_GAME;
            } else if (menuState == GameState::MAIN_MENU) {
                // Exit to Main Menu was clicked
                ReturnToMenu();
            } else if (menuState == GameState::SETTINGS ||
                      menuState == GameState::SOUND_SETTINGS ||
                      menuState == GameState::KEYBIND_SETTINGS) {
                // Settings navigation from pause menu
                m_gameState = menuState;
            }
            return;
        }
        return;
    }

    // Update game systems only when in game
    if (m_gameState == GameState::IN_GAME) {
        // Update physics
        m_physics->Update(deltaTime);

        // Check collisions
        m_physics->CheckCollisions(m_players, m_skillOrbs);

        // Update players
        for (auto& player : m_players) {
            player->Update(deltaTime);
        }

        // Update skill orbs
        for (auto& orb : m_skillOrbs) {
            orb->Update(deltaTime);
        }

        // Update UI
        m_ui->Update(deltaTime);

        // Process current player input
        if (m_gameStarted && !m_gameEnded && m_currentPlayerIndex < m_players.size()) {
            ProcessCurrentPlayerInput();

            // Handle throw action: spawn projectile and advance turn
            Player* currentPlayer = m_players[m_currentPlayerIndex].get();
            if (currentPlayer->GetState() == PlayerState::THROWING) {
                const float radians = currentPlayer->GetAngle() * (3.14159265358979323846f / 180.0f);
                const float powerRatio = currentPlayer->GetPower() / 100.0f;
                Vector2 velocity(std::cos(radians), std::sin(radians));
                if (!currentPlayer->IsFacingRight()) {
                    velocity.x = -velocity.x;
                }
                velocity = velocity * (powerRatio * 800.0f);

                Vector2 spawnPos = currentPlayer->GetPosition();
                m_physics->AddProjectile(std::make_unique<Projectile>(spawnPos, velocity, ProjectileType::NORMAL, currentPlayer->GetId()));

                currentPlayer->SetPower(0.0f);
                currentPlayer->SetState(PlayerState::IDLE);
                // End turn after throwing
                m_turnTimer = 0.0f;
            }
        }

        // Process turn
        ProcessTurn();

        // Check win conditions
        CheckWinConditions();

        // Spawn skill orbs
        if (m_turnTimer > TURN_DURATION - 5.0f && m_turnTimer < TURN_DURATION - 4.9f) {
            SpawnSkillOrbs();
        }
    }
}

void Game::ProcessCurrentPlayerInput() {
    Player* currentPlayer = m_players[m_currentPlayerIndex].get();

    // Check if space key (ADJUST_POWER) was just released to shoot
    if (currentPlayer->GetState() == PlayerState::AIMING) {
        bool spaceReleased = m_inputManager->IsPlayerInputJustReleased(
            m_currentPlayerIndex, InputManager::PlayerInput::ADJUST_POWER);

        if (spaceReleased && currentPlayer->GetPower() > 0.0f) {
            // Player released space, trigger shot
            currentPlayer->SetState(PlayerState::THROWING);
        }
    }

    // Process continuous input
    for (int input = 0; input < static_cast<int>(InputManager::PlayerInput::NONE); ++input) {
        InputManager::PlayerInput playerInput = static_cast<InputManager::PlayerInput>(input);
        bool pressed = m_inputManager->IsPlayerInputPressed(m_currentPlayerIndex, playerInput);
        currentPlayer->HandleInput(static_cast<int>(playerInput), pressed);
    }
}

void Game::ProcessTurn() {
    if (!m_gameStarted) {
        m_gameStarted = true;
        m_turnTimer = TURN_DURATION;
        if (m_currentPlayerIndex < m_players.size()) {
            m_players[m_currentPlayerIndex]->StartTurn();
        }
        m_ui->ShowMessage("Game Started! Player " + std::to_string(m_currentPlayerIndex + 1) + "'s turn");
        return;
    }

    if (m_gameEnded) return;

    m_turnTimer -= 1.0f / 60.0f;

    if (m_turnTimer <= 0.0f) {
        // End current player's turn
        m_players[m_currentPlayerIndex]->EndTurn();

        // Move to next alive player
        do {
            m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
        } while (!m_players[m_currentPlayerIndex]->IsAlive() && !m_gameEnded);

        // Start next player's turn
        m_turnTimer = TURN_DURATION;
        m_players[m_currentPlayerIndex]->StartTurn();

        m_ui->ShowMessage("Player " + std::to_string(m_currentPlayerIndex + 1) + "'s turn");
    }
}

void Game::SpawnSkillOrbs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(100.0f, 1100.0f);
    std::uniform_real_distribution<float> yDist(400.0f, 500.0f);
    std::uniform_int_distribution<int> skillDist(0, static_cast<int>(SkillType::COUNT) - 1);

    // Spawn 1-3 skill orbs
    int numOrbs = 1 + (gen() % 3);
    for (int i = 0; i < numOrbs; ++i) {
        Vector2 position(xDist(gen), yDist(gen));
        SkillType skillType = static_cast<SkillType>(skillDist(gen));
        m_skillOrbs.push_back(std::make_unique<SkillOrb>(position, skillType));
    }

    m_ui->ShowMessage("Skill orbs spawned!");
}

void Game::CheckWinConditions() {
    int alivePlayers = 0;
    int lastAlivePlayer = -1;

    for (size_t i = 0; i < m_players.size(); ++i) {
        if (m_players[i]->IsAlive()) {
            alivePlayers++;
            lastAlivePlayer = static_cast<int>(i);
        }
    }

    if (alivePlayers <= 1 && !m_gameEnded) {
        m_gameEnded = true;
        m_winnerId = lastAlivePlayer;
        m_ui->ShowGameOver(m_winnerId);
    }
}

void Game::ResetGame() {
    m_currentPlayerIndex = 0;
    m_turnTimer = TURN_DURATION;
    m_gameStarted = false;
    m_gameEnded = false;
    m_winnerId = -1;

    // Reset all players
    for (auto& player : m_players) {
        player->ResetForNewGame();
    }

    // Clear projectiles and skill orbs
    m_skillOrbs.clear();

    m_ui->ClearMessages();
}

void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                if (m_gameState == GameState::IN_GAME) {
                    // Pause the game
                    m_gameState = GameState::PAUSED;
                    m_menu->SetState(GameState::PAUSED);
                } else if (m_gameState == GameState::PAUSED) {
                    // Resume the game
                    m_gameState = GameState::IN_GAME;
                }
                // ESC key no longer closes the game
            }
            else if (event.key.scancode == SDL_SCANCODE_R && m_gameEnded) {
                ResetGame();
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            // Handle window resize if needed
            break;
        }
    }
}

void Game::Render() {
    m_renderer->BeginFrame();

    // Check if we're in a menu-only state
    bool isMenuOnlyState = (m_gameState == GameState::MAIN_MENU ||
                            m_gameState == GameState::GAME_MODE_SELECTION ||
                            m_gameState == GameState::PLAYER_COUNT_SELECTION ||
                            m_gameState == GameState::MAP_SELECTION);

    // Render menu-only states (no game background)
    if (isMenuOnlyState) {
        m_menu->Render();
        m_renderer->EndFrame();
        return;
    }

    // Render game with overlays (settings/pause accessed during gameplay)
    if (m_gameState == GameState::IN_GAME || m_gameState == GameState::PAUSED ||
        m_gameState == GameState::SETTINGS || m_gameState == GameState::SOUND_SETTINGS ||
        m_gameState == GameState::KEYBIND_SETTINGS) {

        // Draw map background and terrain (always visible during gameplay)
        if (m_currentMap) {
            m_currentMap->DrawBackground(m_renderer.get());
            m_currentMap->DrawTerrain(m_renderer.get());
        }

        // Draw skill orbs
        for (const auto& orb : m_skillOrbs) {
            orb->Draw(m_renderer.get());
        }

        // Draw projectiles
        m_physics->Draw(m_renderer.get());

        // Draw players
        for (const auto& player : m_players) {
            if (player->IsAlive()) {
                m_renderer->SetDrawColor(player->GetColor());
                m_renderer->DrawCircle(player->GetPosition(), player->GetRadius(), player->GetColor());

                // Draw health bar
                Vector2 healthBarPos = player->GetPosition() + Vector2(0, -40);
                m_renderer->DrawHealthBar(healthBarPos, player->GetHealth(), player->GetMaxHealth(), 40, 8);
            }
        }

        // Draw UI
        m_ui->Render(m_players, m_currentPlayerIndex, m_turnTimer, Vector2(0, 0));

        // Draw menu overlay if paused or in settings
        if (m_gameState == GameState::PAUSED || m_gameState == GameState::SETTINGS ||
            m_gameState == GameState::SOUND_SETTINGS || m_gameState == GameState::KEYBIND_SETTINGS) {
            m_menu->Render();
        }
    }

    m_renderer->EndFrame();
}

void Game::StartGame() {
    // Get game configuration from menu
    m_gameMode = m_menu->GetGameMode();
    m_numPlayers = m_menu->GetPlayerCount();

    // Load the selected map
    int mapIndex = m_menu->GetSelectedMapIndex();

    m_currentMap = std::make_unique<Map>();

    if (mapIndex >= 0 && mapIndex < static_cast<int>(m_availableMaps.size())) {
        // Load selected map
        if (!m_currentMap->LoadFromFolder(m_availableMaps[mapIndex].folderPath)) {
            std::cerr << "Failed to load map, using default" << std::endl;
            m_currentMap->GetTerrain()->CreateDefaultTerrain(1200, 800);
        }
    } else {
        // No map selected or invalid index, use default
        std::cout << "Using default terrain" << std::endl;
        m_currentMap->GetTerrain()->CreateDefaultTerrain(1200, 800);
    }

    // Set terrain in physics system
    m_physics->SetTerrain(m_currentMap->GetTerrain());

    // Create players based on selection
    CreatePlayers();

    // Reset game state
    m_currentPlayerIndex = 0;
    m_turnTimer = TURN_DURATION;
    m_gameStarted = false;
    m_gameEnded = false;
    m_winnerId = -1;

    // Clear any existing projectiles and skill orbs
    m_skillOrbs.clear();
    m_ui->ClearMessages();

    // Switch to game state
    m_gameState = GameState::IN_GAME;
}

void Game::ReturnToMenu() {
    m_gameState = GameState::MAIN_MENU;
    m_menu->SetState(GameState::MAIN_MENU);
}

void Game::Shutdown() {
    m_players.clear();
    m_skillOrbs.clear();
    m_ui.reset();
    m_menu.reset();
    m_physics.reset();
    m_inputManager.reset();
    m_renderer.reset();

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
