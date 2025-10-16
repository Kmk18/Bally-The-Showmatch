#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include "Player.h"
#include "Renderer.h"
#include "InputManager.h"
#include "Physics.h"
#include "UI.h"
#include "SkillOrb.h"
#include "Menu.h"
#include "Map.h"

// Forward declarations
class Projectile;

class Game {
public:
    Game();
    ~Game();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    void Update(float deltaTime);
    void Render();
    void HandleEvents();

    void ProcessTurn();
    void ProcessCurrentPlayerInput();
    void SpawnSkillOrbs();
    void CheckWinConditions();
    void ResetGame();
    void CreatePlayers();
    void SetupPlayerInputs();
    void StartGame();
    void ReturnToMenu();

    SDL_Window* m_window;
    bool m_running;
    float m_lastFrameTime;

    // Game systems
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<Physics> m_physics;
    std::unique_ptr<UI> m_ui;
    std::unique_ptr<Menu> m_menu;

    // Map system
    std::vector<MapInfo> m_availableMaps;
    std::unique_ptr<Map> m_currentMap;

    // Game state
    GameState m_gameState;
    GameMode m_gameMode;
    int m_numPlayers;
    std::vector<std::unique_ptr<Player>> m_players;
    std::vector<std::unique_ptr<SkillOrb>> m_skillOrbs;
    std::vector<std::unique_ptr<Projectile>> m_projectiles;

    int m_currentPlayerIndex;
    float m_turnTimer;
    static constexpr float TURN_DURATION = 20.0f;
    bool m_gameStarted;
    bool m_gameEnded;
    int m_winnerId;

    // Game constants
    static constexpr int MAX_PLAYERS = 4;
    static constexpr int MIN_PLAYERS = 2;
};
