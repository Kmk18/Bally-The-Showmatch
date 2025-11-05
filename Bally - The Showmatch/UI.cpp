#include "UI.h"
#include "SkillOrb.h"
#include "Player.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

UI::UI(Renderer* renderer) : m_renderer(renderer), m_turnTimer(20.0f), m_currentPlayerIndex(0),
    m_inventorySlotTexture(nullptr), m_selectedInventorySlotTexture(nullptr), m_inventorySlotWidth(0), m_inventorySlotHeight(0) {
    LoadInventorySlotTexture();
}

UI::~UI() {
    if (m_inventorySlotTexture) {
        SDL_DestroyTexture(m_inventorySlotTexture);
        m_inventorySlotTexture = nullptr;
    }
    if (m_selectedInventorySlotTexture) {
        SDL_DestroyTexture(m_selectedInventorySlotTexture);
        m_selectedInventorySlotTexture = nullptr;
    }
}

void UI::Update(float deltaTime) {
    // Update messages
    for (auto it = m_messages.begin(); it != m_messages.end();) {
        it->remainingTime -= deltaTime;
        if (it->remainingTime <= 0) {
            it = m_messages.erase(it);
        }
        else {
            ++it;
        }
    }
}

void UI::Render(const std::vector<std::unique_ptr<Player>>& players,
    int currentPlayerIndex, float turnTimer,
    const Vector2& mousePosition) {
    m_currentPlayerIndex = currentPlayerIndex;
    m_turnTimer = turnTimer;

    DrawHUD(players);
    DrawTurnTimer(turnTimer);
    DrawCurrentPlayerIndicator(currentPlayerIndex);

    // Draw aiming UI for current player
    if (currentPlayerIndex < players.size() && players[currentPlayerIndex]->IsAlive()) {
        const Player& currentPlayer = *players[currentPlayerIndex];
        if (currentPlayer.GetState() == PlayerState::AIMING) {
            DrawAimingUI(currentPlayer, mousePosition);
        }
    }

    DrawMessages();
}

void UI::RenderWorldSpace(const std::vector<std::unique_ptr<Player>>& players,
    int currentPlayerIndex, const Vector2& mousePosition) {
    // Draw aiming UI for current player (world-space: angle, power, trajectory)
    if (currentPlayerIndex < players.size() && players[currentPlayerIndex]->IsAlive()) {
        const Player& currentPlayer = *players[currentPlayerIndex];
        if (currentPlayer.GetState() == PlayerState::AIMING) {
            DrawAimingUI(currentPlayer, mousePosition);
        }
    }
}

void UI::RenderScreenSpace(const std::vector<std::unique_ptr<Player>>& players,
    int currentPlayerIndex, float turnTimer,
    const Vector2& cameraPos, float mapWidth, float mapHeight) {
    m_currentPlayerIndex = currentPlayerIndex;
    m_turnTimer = turnTimer;

    // Draw HUD elements (screen-space)
    DrawHUD(players);
    DrawTurnTimer(turnTimer);
    DrawCurrentPlayerIndicator(currentPlayerIndex);
    DrawMessages();

    // Draw inventory (screen-space, always visible for current player)
    if (currentPlayerIndex < players.size() && players[currentPlayerIndex]->IsAlive()) {
        Vector2 inventoryPos(900, 50); // Fixed screen position
        DrawInventory(*players[currentPlayerIndex], inventoryPos);
    }

    // Draw minimap
    DrawMinimap(cameraPos, mapWidth, mapHeight, players);

    // Draw power bar ruler at bottom of screen (only for current player)
    if (currentPlayerIndex < players.size() && players[currentPlayerIndex]->IsAlive()) {
        DrawPowerBarRuler(*players[currentPlayerIndex]);
    }
}

void UI::DrawHUD(const std::vector<std::unique_ptr<Player>>& players) {
    // Draw player info panels
    Vector2 startPos(10, 10);
    float panelWidth = 200;
    float panelHeight = 80;
    float spacing = 10;

    for (size_t i = 0; i < players.size(); ++i) {
        Vector2 panelPos = startPos + Vector2(0, i * (panelHeight + spacing));
        DrawPlayerInfo(*players[i], static_cast<int>(i), panelPos);
    }
}

void UI::DrawPlayerInfo(const Player& player, int index, const Vector2& position) {
    // Panel background
    Color panelColor = player.GetColor();
    panelColor.a = 128; // Semi-transparent
    m_renderer->DrawRect(position, 200, 80, panelColor);

    // Panel outline
    m_renderer->DrawRect(position, 200, 80, Color(255, 255, 255, 255), false);

    // Player name
    std::string playerName = "Player " + std::to_string(index + 1);
    m_renderer->DrawText(position + Vector2(5, 5), playerName.c_str(), Color(255, 255, 255, 255));

    // Health bar
    Vector2 healthBarPos = position + Vector2(5, 25);
    m_renderer->DrawHealthBar(healthBarPos, player.GetHealth(), player.GetMaxHealth(),
        HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);

    // Status text
    std::string status;
    if (!player.IsAlive()) {
        status = "DEAD";
    }
    else if (index == m_currentPlayerIndex) {
        status = "TURN";
    }
    else {
        status = "WAITING";
    }

    m_renderer->DrawText(position + Vector2(5, 45), status.c_str(), Color(255, 255, 255, 255));

    // Skills
    DrawPlayerSkills(player, position + Vector2(5, 60));
}

void UI::DrawTurnTimer(float timer) {
    Vector2 timerPos(500, 10);
    float normalizedTimer = timer / 20.0f; // Assuming 20 second turns

    // Timer background
    m_renderer->DrawRect(timerPos, TURN_TIMER_WIDTH, TURN_TIMER_HEIGHT, Color(50, 50, 50, 255));

    // Timer bar
    Color timerColor;
    if (normalizedTimer > 0.5f) {
        timerColor = Color(0, 255, 0, 255); // Green
    }
    else if (normalizedTimer > 0.25f) {
        timerColor = Color(255, 255, 0, 255); // Yellow
    }
    else {
        timerColor = Color(255, 0, 0, 255); // Red
    }

    m_renderer->DrawRect(timerPos, TURN_TIMER_WIDTH * normalizedTimer, TURN_TIMER_HEIGHT, timerColor);
    m_renderer->DrawRect(timerPos, TURN_TIMER_WIDTH, TURN_TIMER_HEIGHT, Color(255, 255, 255, 255), false);

    // Timer text
    std::stringstream ss;
    ss << "Turn: " << std::fixed << std::setprecision(1) << timer << "s";
    m_renderer->DrawText(timerPos + Vector2(0, 25), ss.str().c_str(), Color(255, 255, 255, 255));
}

void UI::DrawCurrentPlayerIndicator(int playerIndex) {
    std::string text = "Current Player: " + std::to_string(playerIndex + 1);
    m_renderer->DrawText(Vector2(500, 50), text.c_str(), Color(255, 255, 0, 255));
}

void UI::DrawAimingUI(const Player& player, const Vector2& mousePosition) {
    Vector2 playerPos = player.GetPosition();

    // Calculate velocity for both angle display and trajectory
    float radians = player.GetAngle() * M_PI / 180.0f;
    float powerRatio = player.GetPower() / 100.0f;
    Vector2 velocity = Vector2(std::cos(radians), std::sin(radians));

    // Mirror velocity when facing left
    if (!player.IsFacingRight()) {
        velocity.x = -velocity.x;
    }

    // Draw angle indicator using actual velocity direction
    float displayAngle = std::atan2(velocity.y, velocity.x) * 180.0f / M_PI;
    DrawAngleIndicator(playerPos, displayAngle, ANGLE_INDICATOR_LENGTH);

    // Draw trajectory preview using the same velocity (matching Game.cpp projectile speed)
    m_renderer->DrawProjectileTrajectory(playerPos, velocity * (powerRatio * 1800.0f), 980.0f, 60);

    // Draw controls help
    Vector2 helpPos(10, 400);
    m_renderer->DrawText(helpPos, "Controls:", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 15), "Left/Right: Move", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 30), "Up/Down: Aim", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 45), "Space: Power (Hold)", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 60), "Enter: Throw", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 75), "1/2/3/4: Select Skills", Color(255, 255, 255, 255));
}

void UI::DrawInventory(const Player& player, const Vector2& position) {
    // Draw inventory title
    m_renderer->DrawText(position, "Inventory (1-4)", Color(255, 255, 255, 255));

    const std::vector<int>& inventory = player.GetInventory();
    const std::vector<int>& selectedSkills = player.GetSelectedSkills();

    // Draw inventory slots
    for (int i = 0; i < 4; ++i) {
        Vector2 slotPos = position + Vector2(i * 60, 20);

        // Check if this skill is selected
        bool isSelected = false;
        if (i < inventory.size()) {
            int skillType = inventory[i];
            isSelected = std::find(selectedSkills.begin(), selectedSkills.end(), skillType) != selectedSkills.end();
        }

        // Draw inventory slot texture if loaded (use selected texture if skill is selected)
        SDL_Texture* textureToUse = isSelected && m_selectedInventorySlotTexture ? 
            m_selectedInventorySlotTexture : m_inventorySlotTexture;
        
        if (textureToUse && m_inventorySlotWidth > 0 && m_inventorySlotHeight > 0) {
            // Draw slot texture at fixed size (50x50)
            SDL_FRect destRect = { slotPos.x, slotPos.y, 50.0f, 50.0f };
            SDL_RenderTexture(m_renderer->GetSDLRenderer(), textureToUse, nullptr, &destRect);
        } else {
            // Fallback: Draw slot background if texture not loaded
            Color slotColor = Color(50, 50, 50, 255);
            if (isSelected) {
                slotColor = Color(255, 255, 0, 180); // Yellow highlight for selected
            }
            else if (i < inventory.size()) {
                slotColor = Color(70, 70, 70, 255);
            }
            m_renderer->DrawRect(slotPos, 50, 50, slotColor);
            m_renderer->DrawRect(slotPos, 50, 50, Color(255, 255, 255, 255), false);
        }

        // Draw key number (bottom left of slot)
        std::string keyText = std::to_string(i + 1);
        int textWidth = 0, textHeight = 0;
        m_renderer->GetTextSize(keyText.c_str(), &textWidth, &textHeight);
        float offset = 5.0f; // Padding from edge
        Vector2 textPos = slotPos + Vector2(offset, 53.0f - textHeight - offset);
        m_renderer->DrawText(textPos, keyText.c_str(), Color(255, 255, 255, 255));

        // Draw skill icon if slot is occupied
        if (i < inventory.size()) {
            int skillType = inventory[i];
            Color skillColor = GetSkillColor(static_cast<SkillType>(skillType));

            // Draw skill circle
            Vector2 skillCenter = slotPos + Vector2(25, 30);
            m_renderer->DrawCircle(skillCenter, 12, skillColor);

            // Draw skill name
            std::string skillName = GetSkillName(static_cast<SkillType>(skillType));
            m_renderer->DrawText(slotPos + Vector2(5, 40), skillName.substr(0, 4).c_str(), Color(255, 255, 255, 255));
        }
    }
}

void UI::DrawPowerIndicator(const Vector2& position, float power, float maxPower) {
    float normalizedPower = clamp(power / maxPower, 0.0f, 1.0f);

    // Background
    m_renderer->DrawRect(position, 100, 10, Color(50, 50, 50, 255));

    // Power bar
    Color powerColor;
    if (normalizedPower < 0.3f) {
        powerColor = Color(0, 255, 0, 255); // Green
    }
    else if (normalizedPower < 0.7f) {
        powerColor = Color(255, 255, 0, 255); // Yellow
    }
    else {
        powerColor = Color(255, 0, 0, 255); // Red
    }

    m_renderer->DrawRect(position, 100 * normalizedPower, 10, powerColor);

    // Outline
    m_renderer->DrawRect(position, 100, 10, Color(255, 255, 255, 255), false);
}

void UI::DrawPowerBarRuler(const Player& player) {
    const float barWidth = 600.0f;
    const float barHeight = 30.0f;
    const float barX = 100.0f; // Center it: (1200 - 1000) / 2 = 100
    const float barY = 800.0f - barHeight - 20.0f; // 20px from bottom
    const float maxPower = 100.0f;
    
    float power = player.GetPower();
    float normalizedPower = clamp(power / maxPower, 0.0f, 1.0f);
    
    // Background bar
    m_renderer->DrawRect(Vector2(barX, barY), barWidth, barHeight, Color(30, 30, 30, 30));
    
    // Power fill bar
    Color powerColor;
    if (normalizedPower < 0.3f) {
        powerColor = Color(0, 255, 0, 255); // Green
    }
    else if (normalizedPower < 0.7f) {
        powerColor = Color(255, 255, 0, 255); // Yellow
    }
    else {
        powerColor = Color(255, 0, 0, 255); // Red
    }
    m_renderer->DrawRect(Vector2(barX, barY), barWidth * normalizedPower, barHeight, powerColor);
    
    // Draw ruler marks and numbers (0-100, interval 5)
    const int interval = 5;
    const int maxValue = 100;
    Color tickColor = Color(255, 255, 255, 255);
    Color textColor = Color(255, 255, 255, 255);
    
    for (int value = 0; value <= maxValue; value += interval) {
        float x = barX + (barWidth * value / maxValue);
        
        // Draw tick mark inside the ruler - longer ticks for multiples of 10, shorter for multiples of 5
        float tickHeight = (value % 10 == 0) ? barHeight * 0.8f : barHeight * 0.5f;
        float tickStartY = barY + barHeight; // Start from bottom of bar
        float tickEndY = tickStartY - tickHeight; // Draw upward inside the bar
        m_renderer->DrawLine(Vector2(x, tickStartY), Vector2(x, tickEndY), tickColor, 2.0f);
        
        // Draw number label above the ruler
        std::string label = std::to_string(value);
        int textWidth = 0, textHeight = 0;
        m_renderer->GetTextSize(label.c_str(), &textWidth, &textHeight);
        Vector2 textPos(x - textWidth / 2.0f, barY - textHeight - 5.0f);
        m_renderer->DrawText(textPos, label.c_str(), textColor);
    }
    
    // Draw current power value indicator (vertical line)
    float powerX = barX + (barWidth * normalizedPower);
    m_renderer->DrawLine(Vector2(powerX, barY), Vector2(powerX, barY + barHeight), Color(255, 255, 255, 255), 3.0f);
    
    // Outline
    m_renderer->DrawRect(Vector2(barX, barY), barWidth, barHeight, Color(255, 255, 255, 255), false);
}

void UI::DrawAngleIndicator(const Vector2& position, float angle, float length) {
    float radians = angle * M_PI / 180.0f;
    Vector2 end = position + Vector2(std::cos(radians), std::sin(radians)) * length;

    // Draw angle indicator line
    m_renderer->DrawLine(position, end, Color(255, 255, 255, 255), 3.0f);

    // Draw angle arc
    for (float a = -M_PI / 4; a <= M_PI / 4; a += 0.1f) {
        Vector2 arcStart = position + Vector2(std::cos(a), std::sin(a)) * (length * 0.5f);
        Vector2 arcEnd = position + Vector2(std::cos(a + 0.1f), std::sin(a + 0.1f)) * (length * 0.5f);
        m_renderer->DrawLine(arcStart, arcEnd, Color(255, 255, 255, 128), 1.0f);
    }
}

void UI::DrawMessages() {
    Vector2 messagePos(500, 100);

    for (const auto& message : m_messages) {
        m_renderer->DrawText(messagePos, message.text.c_str(), message.color);
        messagePos.y += 20;
    }
}

void UI::DrawGameOverScreen(int winnerId) {
    // Semi-transparent overlay
    m_renderer->DrawRect(Vector2::Zero(), 1200, 800, Color(0, 0, 0, 128));

    // Game over text
    Vector2 centerPos(600, 300);
    m_renderer->DrawText(centerPos + Vector2(-50, 0), "GAME OVER", Color(255, 0, 0, 255));

    if (winnerId >= 0) {
        std::string winnerText = "Player " + std::to_string(winnerId + 1) + " Wins!";
        m_renderer->DrawText(centerPos + Vector2(-30, 30), winnerText.c_str(), Color(255, 255, 0, 255));
    }

    m_renderer->DrawText(centerPos + Vector2(-40, 60), "Press R to Restart", Color(255, 255, 255, 255));
}

void UI::DrawPlayerSkills(const Player& player, const Vector2& position) {
    // Draw skill icons (simple colored circles for now)
    for (int i = 0; i < 4; ++i) { // Max 4 skills
        if (player.HasSkill(i)) {
            Vector2 skillPos = position + Vector2(i * 15, 0);
            Color skillColor = GetSkillColor(static_cast<SkillType>(i));
            m_renderer->DrawCircle(skillPos, 6, skillColor);
        }
    }
}

std::string UI::GetSkillName(SkillType skillType) const {
    switch (skillType) {
    case SkillType::SPLIT_THROW:
        return "Split";
    case SkillType::ENHANCED_DAMAGE:
        return "Damage+";
    case SkillType::ENHANCED_EXPLOSIVE:
        return "Explosive+";
    case SkillType::TELEPORT:
        return "Teleport";
    case SkillType::HEAL:
        return "Heal";
    default:
        return "Unknown";
    }
}

Color UI::GetSkillColor(SkillType skillType) const {
    switch (skillType) {
    case SkillType::SPLIT_THROW:
        return Color(255, 165, 0, 255); // Orange
    case SkillType::ENHANCED_DAMAGE:
        return Color(255, 0, 0, 255); // Red
    case SkillType::ENHANCED_EXPLOSIVE:
        return Color(255, 0, 255, 255); // Magenta
    case SkillType::TELEPORT:
        return Color(0, 255, 255, 255); // Cyan
    case SkillType::HEAL:
        return Color(0, 255, 0, 255); // Green
    default:
        return Color(255, 255, 255, 255); // White
    }
}

void UI::ShowMessage(const std::string& message, float duration) {
    Message msg;
    msg.text = message;
    msg.remainingTime = duration;
    msg.color = Color(255, 255, 255, 255);
    m_messages.push_back(msg);
}

void UI::ShowGameOver(int winnerId) {
    ShowMessage("Game Over!", 10.0f);
    if (winnerId >= 0) {
        ShowMessage("Player " + std::to_string(winnerId + 1) + " wins!", 10.0f);
    }
}

void UI::ClearMessages() {
    m_messages.clear();
}

void UI::DrawMinimap(const Vector2& cameraPos, float mapWidth, float mapHeight,
    const std::vector<std::unique_ptr<Player>>& players) {
    // Minimap position (bottom-right corner)
    Vector2 minimapPos(1200 - MINIMAP_WIDTH - 10, 800 - MINIMAP_HEIGHT - 10);

    // Draw minimap background
    m_renderer->DrawRect(minimapPos, MINIMAP_WIDTH, MINIMAP_HEIGHT, Color(30, 30, 30, 200));
    m_renderer->DrawRect(minimapPos, MINIMAP_WIDTH, MINIMAP_HEIGHT, Color(255, 255, 255, 255), false);

    // Calculate scale factors
    float scaleX = MINIMAP_WIDTH / mapWidth;
    float scaleY = MINIMAP_HEIGHT / mapHeight;

    // Draw players on minimap
    for (const auto& player : players) {
        if (player->IsAlive()) {
            Vector2 playerWorldPos = player->GetPosition();
            Vector2 playerMinimapPos(
                minimapPos.x + playerWorldPos.x * scaleX,
                minimapPos.y + playerWorldPos.y * scaleY
            );
            m_renderer->DrawCircle(playerMinimapPos, 3, player->GetColor());
        }
    }

    // Draw camera viewport rectangle
    float viewportWidth = 1200.0f; // Camera viewport width
    float viewportHeight = 800.0f; // Camera viewport height

    Vector2 cameraRectPos(
        minimapPos.x + cameraPos.x * scaleX,
        minimapPos.y + cameraPos.y * scaleY
    );
    Vector2 cameraRectSize(
        viewportWidth * scaleX,
        viewportHeight * scaleY
    );

    // Draw camera viewport outline
    m_renderer->DrawRect(cameraRectPos, cameraRectSize.x, cameraRectSize.y,
        Color(255, 255, 0, 150), false);
}

bool UI::HandleMinimapClick(const Vector2& mousePos, float mapWidth, float mapHeight, Vector2& outWorldPos) {
    // Minimap position (bottom-right corner)
    Vector2 minimapPos(1200 - MINIMAP_WIDTH - 10, 800 - MINIMAP_HEIGHT - 10);

    // Check if mouse is within minimap bounds
    if (mousePos.x >= minimapPos.x && mousePos.x <= minimapPos.x + MINIMAP_WIDTH &&
        mousePos.y >= minimapPos.y && mousePos.y <= minimapPos.y + MINIMAP_HEIGHT) {

        // Calculate scale factors
        float scaleX = MINIMAP_WIDTH / mapWidth;
        float scaleY = MINIMAP_HEIGHT / mapHeight;

        // Convert minimap position to world position (centered on viewport)
        float clickX = (mousePos.x - minimapPos.x) / scaleX;
        float clickY = (mousePos.y - minimapPos.y) / scaleY;

        // Center the camera on the clicked position (subtract half viewport to get top-left position)
        outWorldPos.x = clickX - 600.0f; // 1200 / 2
        outWorldPos.y = clickY - 400.0f; // 800 / 2

        return true;
    }

    return false;
}

void UI::LoadInventorySlotTexture() {
    // Load regular inventory slot texture
    std::string texturePath = "../assets/inventory_slot.png";
    SDL_Surface* surface = IMG_Load(texturePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load inventory slot texture: " << texturePath << " - " << SDL_GetError() << std::endl;
    } else {
        m_inventorySlotWidth = surface->w;
        m_inventorySlotHeight = surface->h;
        m_inventorySlotTexture = SDL_CreateTextureFromSurface(m_renderer->GetSDLRenderer(), surface);
        SDL_DestroySurface(surface);

        if (!m_inventorySlotTexture) {
            std::cerr << "Failed to create inventory slot texture: " << SDL_GetError() << std::endl;
            m_inventorySlotWidth = 0;
            m_inventorySlotHeight = 0;
        } else {
            std::cout << "Inventory slot texture loaded successfully (" << m_inventorySlotWidth << "x" << m_inventorySlotHeight << ")" << std::endl;
        }
    }

    // Load selected inventory slot texture
    std::string selectedTexturePath = "../assets/selected_inventory_slot.png";
    SDL_Surface* selectedSurface = IMG_Load(selectedTexturePath.c_str());
    if (!selectedSurface) {
        std::cerr << "Failed to load selected inventory slot texture: " << selectedTexturePath << " - " << SDL_GetError() << std::endl;
    } else {
        // Verify dimensions match (should be same size as regular texture)
        if (m_inventorySlotWidth > 0 && m_inventorySlotHeight > 0) {
            if (selectedSurface->w != m_inventorySlotWidth || selectedSurface->h != m_inventorySlotHeight) {
                std::cerr << "Warning: Selected inventory slot texture size (" << selectedSurface->w << "x" << selectedSurface->h 
                          << ") doesn't match regular texture size (" << m_inventorySlotWidth << "x" << m_inventorySlotHeight << ")" << std::endl;
            }
        } else {
            // If regular texture failed, use selected texture dimensions
            m_inventorySlotWidth = selectedSurface->w;
            m_inventorySlotHeight = selectedSurface->h;
        }

        m_selectedInventorySlotTexture = SDL_CreateTextureFromSurface(m_renderer->GetSDLRenderer(), selectedSurface);
        SDL_DestroySurface(selectedSurface);

        if (!m_selectedInventorySlotTexture) {
            std::cerr << "Failed to create selected inventory slot texture: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Selected inventory slot texture loaded successfully" << std::endl;
        }
    }
}
