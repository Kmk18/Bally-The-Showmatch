#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Vector2.h"
#include "Renderer.h"
#include "Menu.h"
#include <SDL3/SDL.h>

class Player;
class Renderer;

enum class SkillType {
    SPLIT_THROW = 0,
    ENHANCED_DAMAGE,
    ENHANCED_EXPLOSIVE,
    TELEPORT,
    HEAL,
    COUNT
};

class UI {
public:
    UI(Renderer* renderer);
    ~UI();

    void Update(float deltaTime);
    void Render(const std::vector<std::unique_ptr<Player>>& players,
        int currentPlayerIndex, float turnTimer,
        const Vector2& mousePosition);

    // Render world-space UI (call with camera offset active)
    void RenderWorldSpace(const std::vector<std::unique_ptr<Player>>& players,
        int currentPlayerIndex, const Vector2& mousePosition);

    // Render screen-space UI (call with camera offset reset to 0)
    void RenderScreenSpace(const std::vector<std::unique_ptr<Player>>& players,
        int currentPlayerIndex, float turnTimer,
        const Vector2& cameraPos, float mapWidth, float mapHeight);

    // Draw minimap
    void DrawMinimap(const Vector2& cameraPos, float mapWidth, float mapHeight,
        const std::vector<std::unique_ptr<Player>>& players);

    // Check if mouse is clicking on minimap and return world position if so
    bool HandleMinimapClick(const Vector2& mousePos, float mapWidth, float mapHeight, Vector2& outWorldPos);
    // Check if mouse is over minimap (for dragging)
    bool IsMouseOverMinimap(const Vector2& mousePos) const;

    void ShowMessage(const std::string& message, float duration = 3.0f);
    void ShowGameOver(int winnerId, GameMode gameMode);
    void ClearMessages();
    void SetGameMode(GameMode gameMode) { m_gameMode = gameMode; }
    int GetGameOverButtonClick(const Vector2& mousePos); // Returns 1 = back to menu, 2 = rematch, 0 = none

    void SetTurnTimer(float timer) { m_turnTimer = timer; }
    void SetCurrentPlayer(int playerIndex) { m_currentPlayerIndex = playerIndex; }

private:
    Renderer* m_renderer;
    float m_turnTimer;
    int m_currentPlayerIndex;

    // Inventory slot textures
    SDL_Texture* m_inventorySlotTexture;
    SDL_Texture* m_selectedInventorySlotTexture;
    int m_inventorySlotWidth;
    int m_inventorySlotHeight;

    // Skill orb textures (for inventory)
    SDL_Texture* m_skillOrbTextures[static_cast<int>(SkillType::COUNT)];
    
    // Button textures (aqua=0, orange=1, pink=2, purple=3)
    SDL_Texture* m_buttonTextures[4];
    int m_buttonTextureWidths[4];
    int m_buttonTextureHeights[4];

    // Message system
    struct Message {
        std::string text;
        float remainingTime;
        Color color;
    };
    std::vector<Message> m_messages;

    // UI Elements
    void DrawPlayerHealthBar(const Player& player, int index);
    void DrawTurnTimer(float timer);
    void DrawCurrentPlayerIndicator(int playerIndex);
    void DrawAimingUI(const Player& player, const Vector2& mousePosition);
    void DrawControlsHelp();
    void DrawPowerIndicator(const Vector2& position, float power, float maxPower);
    void DrawPowerBarRuler(const Player& player);
    void DrawAngleIndicator(const Vector2& position, float angle, float length);
    void DrawMessages();
    void DrawGameOverScreen(int winnerId);
    void DrawInventory(const Player& player, const Vector2& position);

    // Skill UI
    void DrawSkillOrbs(const std::vector<std::unique_ptr<class SkillOrb>>& skillOrbs);
    void DrawPlayerSkills(const Player& player, const Vector2& position);
    std::string GetSkillName(SkillType skillType) const;
    Color GetSkillColor(SkillType skillType) const;

    // Texture loading
    void LoadInventorySlotTexture();
    void LoadSkillOrbTextures();
    void LoadButtonTextures();
    std::string GetSkillOrbTexturePath(SkillType skillType) const;

    GameMode m_gameMode;
    
    // Game over screen state
    bool m_gameOverActive;
    int m_winnerId;
    float m_colorCycleTime;
    int m_currentColorIndex;
    
    // Constants
    static constexpr float MESSAGE_DURATION = 3.0f;
    static constexpr float HEALTH_BAR_WIDTH = 120.0f;
    static constexpr float HEALTH_BAR_HEIGHT = 15.0f;
    static constexpr float POWER_INDICATOR_LENGTH = 200.0f;
    static constexpr float ANGLE_INDICATOR_LENGTH = 80.0f;
    static constexpr float TURN_TIMER_WIDTH = 200.0f;
    static constexpr float TURN_TIMER_HEIGHT = 20.0f;
    static constexpr float MINIMAP_WIDTH = 200.0f;
    static constexpr float MINIMAP_HEIGHT = 150.0f;
};
