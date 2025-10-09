#include "SkillOrb.h"
#include "Player.h"
#include <cmath>

SkillOrb::SkillOrb(const Vector2& position, SkillType skillType)
    : m_position(position), m_radius(DEFAULT_RADIUS), m_skillType(skillType),
    m_collected(false), m_lifetime(MAX_LIFETIME), m_maxLifetime(MAX_LIFETIME),
    m_bobOffset(0.0f), m_bobSpeed(BOB_SPEED) {
    m_color = GetSkillColor();
}

void SkillOrb::Update(float deltaTime) {
    if (m_collected) return;

    m_lifetime -= deltaTime;
    if (m_lifetime <= 0) {
        // Orb expires
        return;
    }

    UpdateAnimation(deltaTime);
}

void SkillOrb::Draw(Renderer* renderer) const {
    if (m_collected || m_lifetime <= 0) return;

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

    m_collected = true;
    player->AddSkill(static_cast<int>(m_skillType));

    // Apply immediate effect based on skill type
    switch (static_cast<int>(m_skillType)) {
    case static_cast<int>(SkillType::SPLIT_THROW):
        ApplySplitThrowSkill(player);
        break;
    case static_cast<int>(SkillType::ENHANCED_DAMAGE):
        ApplyEnhancedDamageSkill(player);
        break;
    case static_cast<int>(SkillType::ENHANCED_EXPLOSIVE):
        ApplyEnhancedExplosiveSkill(player);
        break;
    case static_cast<int>(SkillType::TELEPORT):
        ApplyTeleportSkill(player);
        break;
    }
}

void SkillOrb::UpdateAnimation(float deltaTime) {
    m_bobOffset = std::sin(m_bobSpeed * m_lifetime) * BOB_AMPLITUDE;
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
    default:
        return "Unknown";
    }
}

// Static skill effect methods
void SkillOrb::ApplySplitThrowSkill(Player* player) {
    if (!player) return;

    // Split throw creates multiple projectiles with reduced damage
    // This is handled in the projectile creation logic
    // For now, just give the player the skill
}

void SkillOrb::ApplyEnhancedDamageSkill(Player* player) {
    if (!player) return;

    // Enhanced damage increases projectile damage
    // This is handled in the projectile creation logic
    // For now, just give the player the skill
}

void SkillOrb::ApplyEnhancedExplosiveSkill(Player* player) {
    if (!player) return;

    // Enhanced explosive increases explosion radius and force
    // This is handled in the projectile creation logic
    // For now, just give the player the skill
}

void SkillOrb::ApplyTeleportSkill(Player* player) {
    if (!player) return;

    // Teleport allows the player to teleport to projectile impact location
    // This is handled in the projectile collision logic
    // For now, just give the player the skill
}
