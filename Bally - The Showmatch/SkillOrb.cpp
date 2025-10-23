#include "SkillOrb.h"
#include "Player.h"
#include <cmath>

SkillOrb::SkillOrb(const Vector2& position, SkillType skillType, int spawnTurn)
    : m_position(position), m_radius(DEFAULT_RADIUS), m_skillType(skillType),
    m_collected(false), m_spawnTurn(spawnTurn), m_animTime(0.0f),
    m_bobOffset(0.0f), m_bobSpeed(BOB_SPEED) {
    m_color = GetSkillColor();
}

void SkillOrb::Update(float deltaTime) {
    if (m_collected) return;

    m_animTime += deltaTime;
    UpdateAnimation(deltaTime);
}

void SkillOrb::Draw(Renderer* renderer) const {
    if (m_collected) return;

    // Draw orb with bobbing animation
    Vector2 drawPos = m_position + Vector2(0, m_bobOffset);

    // Outer glow
    Color glowColor = m_color;
    glowColor.a = 100;
    renderer->SetDrawColor(glowColor);
    renderer->DrawCircle(drawPos, m_radius + 5, glowColor);

    // Main orb
    renderer->SetDrawColor(m_color);
    renderer->DrawCircle(drawPos, m_radius, m_color);

    // Inner highlight
    Color highlightColor = Color(255, 255, 255, 128);
    renderer->SetDrawColor(highlightColor);
    renderer->DrawCircle(drawPos + Vector2(-3, -3), m_radius * 0.3f, highlightColor);

    // Skill type indicator
    Vector2 textPos = drawPos + Vector2(0, m_radius + 15);
    renderer->DrawText(textPos, GetSkillName().c_str(), Color(255, 255, 255, 255));
}

void SkillOrb::OnCollected(Player* player) {
    if (!player || m_collected) return;

    // Try to add to inventory - only collect if there's space
    if (player->AddSkillToInventory(static_cast<int>(m_skillType))) {
        m_collected = true;
        // No immediate effect - skills are used from inventory with keybinds
    }
}

void SkillOrb::UpdateAnimation(float deltaTime) {
    m_bobOffset = std::sin(m_bobSpeed * m_animTime) * BOB_AMPLITUDE;
}

Color SkillOrb::GetSkillColor() const {
    switch (m_skillType) {
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

std::string SkillOrb::GetSkillName() const {
    switch (m_skillType) {
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

// Static skill effect methods
void SkillOrb::ApplySplitThrowSkill(Player* player) {
    if (!player) return;
}

void SkillOrb::ApplyEnhancedDamageSkill(Player* player) {
    if (!player) return;
}

void SkillOrb::ApplyEnhancedExplosiveSkill(Player* player) {
    if (!player) return;
}

void SkillOrb::ApplyTeleportSkill(Player* player) {
    if (!player) return;
}
