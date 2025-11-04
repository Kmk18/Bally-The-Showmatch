#include "Game.h"
#include "InputManager.h"
#include "Physics.h"
#include "UI.h"
#include <iostream>
#include <random>
#include <algorithm>

Game::Game() : m_window(nullptr), m_running(false), m_lastFrameTime(0.0f),
m_gameState(GameState::MAIN_MENU), m_gameMode(GameMode::FREE_FOR_ALL), m_numPlayers(4),
m_currentPlayerIndex(0), m_turnTimer(TURN_DURATION), m_turnCounter(0),
m_gameStarted(false), m_gameEnded(false), m_winnerId(-1), m_waitingForProjectiles(false),
m_cameraDelayTimer(0.0f), m_cameraDelayActive(false) {
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
    m_physics->SetRenderer(m_renderer.get()); // Set renderer for explosion animations
    m_ui = std::make_unique<UI>(m_renderer.get());
    m_menu = std::make_unique<Menu>(m_renderer.get());
    m_camera = std::make_unique<Camera>(1200.0f, 800.0f);

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

    // Character names for each player
    std::string characterNames[] = {
        "Meep",   // Player 1
        "Yetty",  // Player 2
        "Turt",   // Player 3
        "Meep"    // Player 4 (reuse Meep for now)
    };

    // Create players based on menu selection
    int numPlayers = m_numPlayers;
    
    // Get actual map dimensions
    float mapWidth = m_currentMap ? static_cast<float>(m_currentMap->GetWidth()) : 1200.0f;
    float mapHeight = m_currentMap ? static_cast<float>(m_currentMap->GetHeight()) : 800.0f;
    
    // Spread players across the map with padding from edges
    float padding = 100.0f; // Padding from map edges
    float spawnAreaWidth = mapWidth - (padding * 2.0f); // Available width for spawning
    float startX = padding; // Left edge of spawn area

    for (int i = 0; i < numPlayers; ++i) {
        // Distribute players evenly across the spawn area
        float spacing = (numPlayers > 1) ? spawnAreaWidth / (numPlayers - 1) : 0.0f;
        float x = startX + spacing * i;
        
        // Clamp x to map bounds
        x = std::max(padding, std::min(x, mapWidth - padding));
        
        // Find terrain height at spawn position
        float startY = mapHeight * 0.75f; // Default fallback (75% down the map)
        if (m_currentMap && m_currentMap->GetTerrain()) {
            int terrainY = m_currentMap->GetTerrain()->FindTopSolidPixel((int)x, 0);
            if (terrainY >= 0) {
                // Position player so their feet (bottom) are on the ground
                float playerRadius = 20.0f; // Default radius
                startY = (float)terrainY - playerRadius;
            }
        }
        
        Vector2 position(x, startY);

        auto player = std::make_unique<Player>(i, position, playerColors[i], characterNames[i]);

        // Load character animations
        if (player->GetAnimation()) {
            player->GetAnimation()->LoadCharacter(m_renderer.get());
        }

        // Snap player to terrain using actual radius
        if (m_currentMap && m_currentMap->GetTerrain()) {
            Vector2 playerPos = player->GetPosition();
            int terrainY = m_currentMap->GetTerrain()->FindTopSolidPixel((int)playerPos.x, 0);
            if (terrainY >= 0) {
                playerPos.y = (float)terrainY - player->GetRadius();
                player->SetPosition(playerPos);
            }
        }

        m_players.push_back(std::move(player));
    }

    // Set up input mappings for different players
    SetupPlayerInputs();
}

void Game::SetupPlayerInputs() {
    // All players use the same keybinds (Arrow keys + Space + 1-4) since it's turn-based
    for (int i = 0; i < m_numPlayers; ++i) {
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::MOVE_LEFT, SDL_SCANCODE_LEFT);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::MOVE_RIGHT, SDL_SCANCODE_RIGHT);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::AIM_UP, SDL_SCANCODE_UP);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::AIM_DOWN, SDL_SCANCODE_DOWN);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::ADJUST_POWER, SDL_SCANCODE_SPACE);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::USE_SLOT_1, SDL_SCANCODE_1);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::USE_SLOT_2, SDL_SCANCODE_2);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::USE_SLOT_3, SDL_SCANCODE_3);
        m_inputManager->SetKeyMapping(i, InputManager::PlayerInput::USE_SLOT_4, SDL_SCANCODE_4);
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
        m_gameState == GameState::SOUND_SETTINGS ||
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
                      menuState == GameState::SOUND_SETTINGS) {
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

        // Check for minimap click (left mouse button)
        Vector2 mousePos = m_inputManager->GetMousePosition();
        if (m_inputManager->IsMouseButtonPressed(0)) { // Left mouse button
            Vector2 worldPos;
            if (m_ui->HandleMinimapClick(mousePos, m_currentMap->GetWidth(), m_currentMap->GetHeight(), worldPos)) {
                m_camera->SetManualControl(true);
                m_camera->SetCameraPosition(worldPos);
            }
        }

        // Check for manual camera controls (WASD)
        Vector2 cameraMovement(0, 0);
        bool manualCameraInput = false;

        if (m_inputManager->IsKeyPressed(SDL_SCANCODE_W)) {
            cameraMovement.y -= 1.0f;
            manualCameraInput = true;
        }
        if (m_inputManager->IsKeyPressed(SDL_SCANCODE_S)) {
            cameraMovement.y += 1.0f;
            manualCameraInput = true;
        }
        if (m_inputManager->IsKeyPressed(SDL_SCANCODE_A)) {
            cameraMovement.x -= 1.0f;
            manualCameraInput = true;
        }
        if (m_inputManager->IsKeyPressed(SDL_SCANCODE_D)) {
            cameraMovement.x += 1.0f;
            manualCameraInput = true;
        }

        // Enable/disable manual control based on input
        if (manualCameraInput) {
            m_camera->SetManualControl(true);
            if (cameraMovement.Length() > 0) {
                m_camera->MoveCamera(cameraMovement.Normalized(), deltaTime);
            }
        } else if (!m_inputManager->IsMouseButtonPressed(0)) {
            // Only disable manual control if not holding mouse button
            m_camera->SetManualControl(false);
        }

        // Update camera delay timer if active
        if (m_cameraDelayActive) {
            m_cameraDelayTimer -= deltaTime;
            if (m_cameraDelayTimer <= 0.0f) {
                m_cameraDelayActive = false;
                // Force turn to end immediately after camera delay
                m_turnTimer = 0.0f;
            }
        }

        // Update camera to follow active player or projectiles (if not in manual mode)
        Vector2 cameraTarget;
        bool hasCameraTarget = false;

        // If there are active projectiles, follow them in order
        const auto& projectiles = m_physics->GetProjectiles();
        if (!projectiles.empty()) {
            // For split projectiles (3 projectiles), follow in order: middle (0), bottom (2), upper (1)
            // For other projectiles, just follow the first one
            Projectile* targetProjectile = nullptr;

            if (projectiles.size() == 3 && projectiles[0]->HasSplit()) {
                // Split projectile - follow in specific order
                if (projectiles[0]->IsActive()) {
                    targetProjectile = projectiles[0].get(); // Middle
                }
                else if (projectiles.size() > 2 && projectiles[2]->IsActive()) {
                    targetProjectile = projectiles[2].get(); // Bottom (lower)
                }
                else if (projectiles.size() > 1 && projectiles[1]->IsActive()) {
                    targetProjectile = projectiles[1].get(); // Upper
                }
            }
            else {
                // Non-split or other case - follow first active projectile
                for (const auto& proj : projectiles) {
                    if (proj->IsActive()) {
                        targetProjectile = proj.get();
                        break;
                    }
                }
            }

            if (targetProjectile) {
                cameraTarget = targetProjectile->GetPosition();
                hasCameraTarget = true;
            }
        }

        // If no projectiles and camera delay is active, keep camera at last projectile position
        // After delay expires, move camera back to current player
        if (!hasCameraTarget && !m_cameraDelayActive && m_currentPlayerIndex < m_players.size()) {
            // No projectiles and delay expired - follow the current player
            cameraTarget = m_players[m_currentPlayerIndex]->GetPosition();
            hasCameraTarget = true;
        }

        if (hasCameraTarget) {
            m_camera->SetTarget(cameraTarget);
        }
        m_camera->Update(deltaTime);

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
                velocity = velocity * (powerRatio * 1200.0f);

                Vector2 spawnPos = currentPlayer->GetPosition();

                // Check if player has selected skills
                const std::vector<int>& selectedSkills = currentPlayer->GetSelectedSkills();
                if (!selectedSkills.empty()) {
                    // Create projectile with skills
                    m_physics->AddProjectileWithSkills(spawnPos, velocity, selectedSkills, currentPlayer->GetId());

                    // Remove used skills from inventory
                    for (int skillType : selectedSkills) {
                        auto& inventory = currentPlayer->GetInventory();
                        auto it = std::find(inventory.begin(), inventory.end(), skillType);
                        if (it != inventory.end()) {
                            int slot = static_cast<int>(std::distance(inventory.begin(), it));
                            currentPlayer->UseInventorySlot(slot);
                        }
                    }

                    // Clear selected skills
                    currentPlayer->ClearSelectedSkills();
                }
                else {
                    // Normal projectile without skills
                    m_physics->AddProjectile(std::make_unique<Projectile>(spawnPos, velocity, ProjectileType::NORMAL, currentPlayer->GetId()));
                }

                currentPlayer->SetPower(0.0f);
                currentPlayer->SetState(PlayerState::IDLE);
                // Wait for projectiles to land before ending turn
                m_waitingForProjectiles = true;
            }
        }

        // Process turn
        ProcessTurn();

        // Check win conditions
        CheckWinConditions();
    }
}

void Game::ProcessCurrentPlayerInput() {
    Player* currentPlayer = m_players[m_currentPlayerIndex].get();

    // Check if inventory slots were pressed (toggle skill selection)
    if (currentPlayer->GetState() == PlayerState::AIMING) {
        // Check for skill usage (keys 1-4)
        if (m_inputManager->IsPlayerInputJustPressed(m_currentPlayerIndex, InputManager::PlayerInput::USE_SLOT_1)) {
            currentPlayer->ToggleSkillSelection(0);
        }
        if (m_inputManager->IsPlayerInputJustPressed(m_currentPlayerIndex, InputManager::PlayerInput::USE_SLOT_2)) {
            currentPlayer->ToggleSkillSelection(1);
        }
        if (m_inputManager->IsPlayerInputJustPressed(m_currentPlayerIndex, InputManager::PlayerInput::USE_SLOT_3)) {
            currentPlayer->ToggleSkillSelection(2);
        }
        if (m_inputManager->IsPlayerInputJustPressed(m_currentPlayerIndex, InputManager::PlayerInput::USE_SLOT_4)) {
            currentPlayer->ToggleSkillSelection(3);
        }
    }

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
        m_turnCounter = 0;
        if (m_currentPlayerIndex < m_players.size()) {
            m_players[m_currentPlayerIndex]->StartTurn();
        }
        // Spawn skill orbs at start of first turn
        SpawnSkillOrbs();
        m_ui->ShowMessage("Game Started! Player " + std::to_string(m_currentPlayerIndex + 1) + "'s turn");
        return;
    }

    if (m_gameEnded) return;

    // Check if current player died during their turn - skip immediately
    if (!m_players[m_currentPlayerIndex]->IsAlive()) {
        m_players[m_currentPlayerIndex]->EndTurn();
        m_waitingForProjectiles = false;

        // Move to next alive player
        do {
            m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
        } while (!m_players[m_currentPlayerIndex]->IsAlive() && !m_gameEnded);

        // Start next player's turn
        m_turnCounter++;
        m_turnTimer = TURN_DURATION;
        m_players[m_currentPlayerIndex]->StartTurn();

        // Spawn skill orbs at start of turn
        SpawnSkillOrbs();

        // Remove expired skill orbs (older than 3 turns)
        m_skillOrbs.erase(
            std::remove_if(m_skillOrbs.begin(), m_skillOrbs.end(),
                [this](const std::unique_ptr<SkillOrb>& orb) {
                    return orb->IsExpired(m_turnCounter);
                }),
            m_skillOrbs.end()
        );

        m_ui->ShowMessage("Player " + std::to_string(m_currentPlayerIndex + 1) + "'s turn");
        return;
    }

    // If waiting for projectiles, don't count down timer until all projectiles land
    if (m_waitingForProjectiles) {
        if (!m_physics->HasActiveProjectiles()) {
            // All projectiles have landed, start camera delay
            m_waitingForProjectiles = false;
            m_cameraDelayActive = true;
            m_cameraDelayTimer = CAMERA_DELAY_AFTER_IMPACT;
        }
        return; // Don't decrement timer while waiting
    }

    // If camera delay is active after projectiles landed, don't count down timer
    if (m_cameraDelayActive) {
        return; // Delay will force turn end when it expires
    }

    m_turnTimer -= 1.0f / 60.0f;

    if (m_turnTimer <= 0.0f) {
        // End current player's turn
        m_players[m_currentPlayerIndex]->EndTurn();

        // Move to next alive player
        do {
            m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
        } while (!m_players[m_currentPlayerIndex]->IsAlive() && !m_gameEnded);

        // Start next player's turn
        m_turnCounter++;
        m_turnTimer = TURN_DURATION;
        m_players[m_currentPlayerIndex]->StartTurn();

        // Spawn skill orbs at start of turn
        SpawnSkillOrbs();

        // Remove expired skill orbs (older than 3 turns)
        m_skillOrbs.erase(
            std::remove_if(m_skillOrbs.begin(), m_skillOrbs.end(),
                [this](const std::unique_ptr<SkillOrb>& orb) {
                    return orb->IsExpired(m_turnCounter);
                }),
            m_skillOrbs.end()
        );

        m_ui->ShowMessage("Player " + std::to_string(m_currentPlayerIndex + 1) + "'s turn");
    }
}

void Game::SpawnSkillOrbs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(100.0f, 1100.0f);
    std::uniform_real_distribution<float> yDist(200.0f, 500.0f);
    std::uniform_int_distribution<int> skillDist(0, static_cast<int>(SkillType::COUNT) - 1);
    std::uniform_int_distribution<int> orbCountDist(2, 5);

    // Spawn 2-5 skill orbs
    int numOrbs = orbCountDist(gen);
    for (int i = 0; i < numOrbs; ++i) {
        Vector2 position(xDist(gen), yDist(gen));
        SkillType skillType = static_cast<SkillType>(skillDist(gen));
        m_skillOrbs.push_back(std::make_unique<SkillOrb>(position, skillType, m_turnCounter));
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
    m_turnCounter = 0;
    m_gameStarted = false;
    m_gameEnded = false;
    m_winnerId = -1;
    m_waitingForProjectiles = false;

    // Get actual map dimensions for respawning
    float mapWidth = m_currentMap ? static_cast<float>(m_currentMap->GetWidth()) : 1200.0f;
    float mapHeight = m_currentMap ? static_cast<float>(m_currentMap->GetHeight()) : 800.0f;
    
    // Spread players across the map with padding from edges
    float padding = 100.0f;
    float spawnAreaWidth = mapWidth - (padding * 2.0f);
    float startX = padding;

    // Reset all players and respawn them across the map
    for (size_t i = 0; i < m_players.size(); ++i) {
        m_players[i]->ResetForNewGame();
        
        // Calculate new spawn position based on map size
        float spacing = (m_players.size() > 1) ? spawnAreaWidth / (m_players.size() - 1) : 0.0f;
        float x = startX + spacing * i;
        
        // Clamp x to map bounds
        x = std::max(padding, std::min(x, mapWidth - padding));
        
        // Find terrain height at spawn position
        float startY = mapHeight * 0.75f; // Default fallback
        if (m_currentMap && m_currentMap->GetTerrain()) {
            int terrainY = m_currentMap->GetTerrain()->FindTopSolidPixel((int)x, 0);
            if (terrainY >= 0) {
                startY = (float)terrainY - m_players[i]->GetRadius();
            }
        }
        
        // Set player position on terrain
        Vector2 playerPos(x, startY);
        m_players[i]->SetPosition(playerPos);
        
        // Ensure player is snapped to terrain
        if (m_currentMap && m_currentMap->GetTerrain()) {
            int terrainY = m_currentMap->GetTerrain()->FindTopSolidPixel((int)playerPos.x, 0);
            if (terrainY >= 0) {
                playerPos.y = (float)terrainY - m_players[i]->GetRadius();
                m_players[i]->SetPosition(playerPos);
            }
        }
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
        m_gameState == GameState::SETTINGS || m_gameState == GameState::SOUND_SETTINGS) {

        // Set camera offset for world-space rendering
        m_renderer->SetCameraOffset(m_camera->GetPosition());

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

        // Draw players (including dead ones to show death animation)
        for (const auto& player : m_players) {
            // Skip players that should be removed (death animation finished)
            if (player->ShouldBeRemoved()) continue;

            // Draw character animation if available, otherwise draw circle
            if (player->GetAnimation()) {
                player->GetAnimation()->Draw(m_renderer.get(), player->GetPosition(),
                                             player->GetRadius(), player->IsFacingRight());
            } else {
                // Fallback to circle for players without animation
                if (player->IsAlive()) {
                    m_renderer->SetDrawColor(player->GetColor());
                    m_renderer->DrawCircle(player->GetPosition(), player->GetRadius(), player->GetColor());
                }
            }

            // Draw health bar only for alive players
            if (player->IsAlive()) {
                Vector2 healthBarPos = player->GetPosition() + Vector2(0, -40);
                m_renderer->DrawHealthBar(healthBarPos, player->GetHealth(), player->GetMaxHealth(), 40, 8);
            }
        }

        // Draw world-space UI elements (angle/power indicators, trajectory)
        m_ui->RenderWorldSpace(m_players, m_currentPlayerIndex, Vector2(0, 0));

        // Reset camera offset for screen-space UI rendering
        m_renderer->SetCameraOffset(Vector2(0, 0));

        // Draw screen-space UI (HUD, timer, messages, minimap)
        m_ui->RenderScreenSpace(m_players, m_currentPlayerIndex, m_turnTimer,
            m_camera->GetPosition(), m_currentMap->GetWidth(), m_currentMap->GetHeight());

        // Draw menu overlay if paused or in settings
        if (m_gameState == GameState::PAUSED || m_gameState == GameState::SETTINGS ||
            m_gameState == GameState::SOUND_SETTINGS) {
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

    // Configure camera for the map size
    m_camera->SetMapBounds(m_currentMap->GetWidth(), m_currentMap->GetHeight());

    // Create players based on selection
    CreatePlayers();

    // Snap camera to the first player's position
    if (!m_players.empty()) {
        m_camera->SetTarget(m_players[0]->GetPosition());
        m_camera->SnapToTarget();
    }

    // Reset game state
    m_currentPlayerIndex = 0;
    m_turnTimer = TURN_DURATION;
    m_turnCounter = 0;
    m_gameStarted = false;
    m_gameEnded = false;
    m_winnerId = -1;
    m_waitingForProjectiles = false;

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
