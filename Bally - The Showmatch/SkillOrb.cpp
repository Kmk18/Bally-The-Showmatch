#include "SkillOrb.h"
#include "Player.h"
#include "Renderer.h"
#include <cmath>
#include <iostream>

SkillOrb::SkillOrb(const Vector2& position, SkillType skillType, int spawnTurn)
    : m_position(position), m_radius(DEFAULT_RADIUS), m_skillType(skillType),
    m_collected(false), m_spawnTurn(spawnTurn), m_animTime(0.0f),
    m_bobOffset(0.0f), m_bobSpeed(BOB_SPEED), m_texture(nullptr) {
}

SkillOrb::~SkillOrb() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

void SkillOrb::Update(float deltaTime) {
    if (m_collected) return;

    m_animTime += deltaTime;
    UpdateAnimation(deltaTime);
}

bool SkillOrb::LoadTexture(Renderer* renderer) {
    if (!renderer) return false;

    std::string texturePath = GetTexturePath();
    SDL_Surface* surface = IMG_Load(texturePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load skill orb texture: " << texturePath << " - " << SDL_GetError() << std::endl;
        return false;
    }

    m_texture = SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), surface);
    SDL_DestroySurface(surface);

    if (!m_texture) {
        std::cerr << "Failed to create skill orb texture: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void SkillOrb::Draw(Renderer* renderer) const {
    if (m_collected) return;

    // Draw orb with bobbing animation
    Vector2 drawPos = m_position + Vector2(0, m_bobOffset);

    // Outer glow/bubble (keep this)
    Color glowColor(255, 255, 255, 100);
    renderer->SetDrawColor(glowColor);
    renderer->DrawCircle(drawPos, m_radius + 5, glowColor);

    // Draw texture if loaded (50x50 texture scaled to 30x30 to match radius 15)
    if (m_texture) {
        Vector2 cameraOffset = renderer->GetCameraOffset();
        float textureSize = m_radius * 2.0f; // 30x30 (diameter = 2 * radius)
        SDL_FRect destRect = {
            drawPos.x - textureSize / 2.0f - cameraOffset.x,
            drawPos.y - textureSize / 2.0f - cameraOffset.y,
            textureSize,
            textureSize
        };
        SDL_RenderTexture(renderer->GetSDLRenderer(), m_texture, nullptr, &destRect);
    }
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

std::string SkillOrb::GetTexturePath() const {
    switch (m_skillType) {
    case SkillType::SPLIT_THROW:
        return "../assets/skill_orbs/orb_split.png";
    case SkillType::ENHANCED_DAMAGE:
        return "../assets/skill_orbs/orb_damage.png";
    case SkillType::ENHANCED_EXPLOSIVE:
        return "../assets/skill_orbs/orb_explosive.png";
    case SkillType::TELEPORT:
        return "../assets/skill_orbs/orb_teleport.png";
    case SkillType::HEAL:
        return "../assets/skill_orbs/orb_heal.png";
    default:
        return "";
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
