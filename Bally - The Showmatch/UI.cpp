#include "UI.h"
#include "SkillOrb.h"
#include "Player.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

UI::UI(Renderer* renderer) : m_renderer(renderer), m_turnTimer(20.0f), m_currentPlayerIndex(0) {
}

UI::~UI() {
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

    // Draw power indicator
    Vector2 powerIndicatorPos = playerPos + Vector2(0, -60);
    DrawPowerIndicator(powerIndicatorPos, player.GetPower(), 100.0f);

    // Draw trajectory preview using the same velocity
    m_renderer->DrawProjectileTrajectory(playerPos, velocity * (powerRatio * 1200.0f), 980.0f, 60);

    // Draw controls help
    Vector2 helpPos(10, 400);
    m_renderer->DrawText(helpPos, "Controls:", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 15), "Left/Right: Move", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 30), "Up/Down: Aim", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 45), "Space: Power (Hold)", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 60), "Enter: Throw", Color(255, 255, 255, 255));
    m_renderer->DrawText(helpPos + Vector2(0, 75), "1/2/3/4: Select Skills", Color(255, 255, 255, 255));

    // Draw inventory
    Vector2 inventoryPos(900, 50);
    DrawInventory(player, inventoryPos);
}

void UI::DrawInventory(const Player& player, const Vector2& position) {
    // Draw inventory title
    m_renderer->DrawText(position, "Inventory (1-4)", Color(255, 255, 255, 255));

    const std::vector<int>& inventory = player.GetInventory();
    const std::vector<int>& selectedSkills = player.GetSelectedSkills();

    // Draw inventory slots
    for (int i = 0; i < 4; ++i) {
        Vector2 slotPos = position + Vector2(i * 60, 20);

        // Draw slot background
        Color slotColor = Color(50, 50, 50, 255);
        if (i < inventory.size()) {
            // Check if this skill is selected
            int skillType = inventory[i];
            bool isSelected = std::find(selectedSkills.begin(), selectedSkills.end(), skillType) != selectedSkills.end();

            if (isSelected) {
                slotColor = Color(255, 255, 0, 180); // Yellow highlight for selected
            }
            else {
                slotColor = Color(70, 70, 70, 255);
            }
        }

        m_renderer->DrawRect(slotPos, 50, 50, slotColor);
        m_renderer->DrawRect(slotPos, 50, 50, Color(255, 255, 255, 255), false);

        // Draw key number
        std::string keyText = std::to_string(i + 1);
        m_renderer->DrawText(slotPos + Vector2(5, 5), keyText.c_str(), Color(200, 200, 200, 255));

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
