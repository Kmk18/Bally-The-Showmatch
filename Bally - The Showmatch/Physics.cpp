#include "Physics.h"
#include "Player.h"
#include "SkillOrb.h"
#include "Terrain.h"
#include "UI.h"
#include <cmath>
#include <algorithm>

// Custom clamp function for C++14 compatibility
template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Projectile::Projectile(const Vector2& position, const Vector2& velocity, ProjectileType type, int ownerId)
    : m_position(position), m_velocity(velocity), m_acceleration(Vector2::Zero()), m_radius(DEFAULT_RADIUS),
    m_mass(DEFAULT_MASS), m_type(type), m_ownerId(ownerId), m_active(true),
    m_lifetime(0.0f), m_maxLifetime(MAX_LIFETIME), m_hasSplit(false), m_hasPowerBall(false),
    m_hasExplosiveBall(false), m_hasTeleportBall(false), m_hasHeal(false) {
}

Projectile::Projectile(const Vector2& position, const Vector2& velocity, const std::vector<int>& skillTypes, int ownerId)
    : m_position(position), m_velocity(velocity), m_acceleration(Vector2::Zero()), m_radius(DEFAULT_RADIUS),
    m_mass(DEFAULT_MASS), m_type(ProjectileType::NORMAL), m_ownerId(ownerId), m_active(true),
    m_lifetime(0.0f), m_maxLifetime(MAX_LIFETIME), m_hasSplit(false), m_hasPowerBall(false),
    m_hasExplosiveBall(false), m_hasTeleportBall(false), m_hasHeal(false) {

    // Parse skill types and set flags
    for (int skillType : skillTypes) {
        if (skillType == static_cast<int>(SkillType::SPLIT_THROW)) {
            m_hasSplit = true;
        }
        else if (skillType == static_cast<int>(SkillType::ENHANCED_DAMAGE)) {
            m_hasPowerBall = true;
        }
        else if (skillType == static_cast<int>(SkillType::ENHANCED_EXPLOSIVE)) {
            m_hasExplosiveBall = true;
        }
        else if (skillType == static_cast<int>(SkillType::TELEPORT)) {
            m_hasTeleportBall = true;
        }
        else if (skillType == static_cast<int>(SkillType::HEAL)) {
            m_hasHeal = true;
        }
    }

    // Determine visual type based on priority (heal > teleport > explosive > power > split)
    if (m_hasHeal) {
        m_type = ProjectileType::HEAL;
    }
    else if (m_hasTeleportBall) {
        m_type = ProjectileType::TELEPORT;
    }
    else if (m_hasExplosiveBall) {
        m_type = ProjectileType::ENHANCED_EXPLOSIVE;
    }
    else if (m_hasPowerBall) {
        m_type = ProjectileType::ENHANCED_DAMAGE;
    }
    else if (m_hasSplit) {
        m_type = ProjectileType::SPLIT;
    }
}

void Projectile::Update(float deltaTime) {
    if (!m_active) return;

    m_lifetime += deltaTime;
    if (m_lifetime >= m_maxLifetime) {
        m_active = false;
        return;
    }

    // Apply gravity
    m_acceleration.y = GRAVITY;

    // Update velocity with air resistance
    m_velocity = m_velocity + m_acceleration * deltaTime;
    m_velocity = m_velocity * AIR_RESISTANCE;

    // Update position
    m_position = m_position + m_velocity * deltaTime;

    // Reset acceleration
    m_acceleration = Vector2::Zero();

    // Deactivate if it goes off screen
    if (m_position.x < -100 || m_position.x > 1300 || m_position.y > 900) {
        m_active = false;
    }
}

void Projectile::Draw(class Renderer* renderer) const {
    if (!m_active) return;

    Color projectileColor;
    switch (m_type) {
    case ProjectileType::NORMAL:
        projectileColor = Color(255, 255, 255, 255);
        break;
    case ProjectileType::SPLIT:
        projectileColor = Color(255, 165, 0, 255); // Orange
        break;
    case ProjectileType::ENHANCED_DAMAGE:
        projectileColor = Color(255, 0, 0, 255); // Red
        break;
    case ProjectileType::ENHANCED_EXPLOSIVE:
        projectileColor = Color(255, 0, 255, 255); // Magenta
        break;
    case ProjectileType::TELEPORT:
        projectileColor = Color(0, 255, 255, 255); // Cyan
        break;
    case ProjectileType::HEAL:
        projectileColor = Color(0, 255, 0, 255); // Green
        break;
    }

    renderer->SetDrawColor(projectileColor);
    renderer->DrawCircle(m_position, m_radius, projectileColor);
}

float Projectile::GetDamage() const {
    float baseDamage = 25.0f;

    // Heal ball deals no damage
    if (m_hasHeal) {
        return 0.0f;
    }

    // Teleport ball deals no damage
    if (m_hasTeleportBall) {
        return 0.0f;
    }

    // Split reduces damage
    if (m_hasSplit) {
        baseDamage *= 0.4f;
    }

    // Power ball increases damage
    if (m_hasPowerBall) {
        baseDamage *= 2.0f;
    }

    // Explosive ball decreases damage
    if (m_hasExplosiveBall) {
        baseDamage *= 0.5f;
    }

    return baseDamage;
}

float Projectile::GetExplosionRadius() const {
    // Heal ball uses heal radius instead (returned separately)
    if (m_hasHeal) {
        return 50.0f; // Heal AOE radius
    }

    float baseRadius = 20.0f;

    // Teleport ball has no explosion
    if (m_hasTeleportBall && !m_hasExplosiveBall && !m_hasPowerBall) {
        return 0.0f;
    }

    // Explosive ball increases explosion radius
    if (m_hasExplosiveBall) {
        baseRadius = 50.0f;
    }

    return baseRadius;
}

float Projectile::GetExplosionForce() const {
    // Heal ball has no force
    if (m_hasHeal) {
        return 0.0f;
    }

    float baseForce = 500.0f;

    // Teleport ball has no force
    if (m_hasTeleportBall && !m_hasExplosiveBall && !m_hasPowerBall) {
        return 0.0f;
    }

    // Power ball reduces explosion force
    if (m_hasPowerBall) {
        baseForce = 300.0f;
    }

    // Explosive ball increases explosion force
    if (m_hasExplosiveBall) {
        baseForce = 800.0f;
    }

    return baseForce;
}

bool Projectile::DamagesTerrain() const {
    // Heal ball doesn't damage terrain
    if (m_hasHeal) {
        return false;
    }

    // Power ball doesn't damage terrain
    if (m_hasPowerBall && !m_hasExplosiveBall) {
        return false;
    }

    // Teleport ball doesn't damage terrain (unless mixed with other skills)
    if (m_hasTeleportBall && !m_hasExplosiveBall && !m_hasPowerBall) {
        return false;
    }

    return true;
}

Physics::Physics() : m_terrain(nullptr), m_platformWidth(PLATFORM_WIDTH), m_platformHeight(PLATFORM_HEIGHT),
m_platformPosition(200.0f, 650.0f) {
}

Physics::~Physics() {
    m_projectiles.clear();
}

void Physics::Update(float deltaTime) {
    // Update projectiles
    for (auto& projectile : m_projectiles) {
        projectile->Update(deltaTime);
    }

    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) { return !p->IsActive(); }),
        m_projectiles.end()
    );
}

void Physics::Draw(class Renderer* renderer) {
    // Draw all active projectiles
    for (const auto& projectile : m_projectiles) {
        if (projectile->IsActive()) {
            projectile->Draw(renderer);
        }
    }
}

void Physics::AddProjectile(std::unique_ptr<Projectile> projectile) {
    m_projectiles.push_back(std::move(projectile));
}

void Physics::AddProjectileWithSkills(const Vector2& position, const Vector2& velocity, const std::vector<int>& skills, int ownerId) {
    // Check if has split skill
    bool hasSplit = false;
    for (int skill : skills) {
        if (skill == static_cast<int>(SkillType::SPLIT_THROW)) {
            hasSplit = true;
            break;
        }
    }

    if (hasSplit) {
        // Create 3 projectiles with angle offsets
        const float angleOffset = 10.0f;
        const float radianOffset = angleOffset * 3.14159265f / 180.0f;

        // Calculate angle from velocity
        float baseAngle = std::atan2(velocity.y, velocity.x);
        float speed = velocity.Length();

        // Middle projectile
        m_projectiles.push_back(std::make_unique<Projectile>(position, velocity, skills, ownerId));

        // Upper projectile (offset upward)
        float upperAngle = baseAngle - radianOffset;
        Vector2 upperVelocity(std::cos(upperAngle) * speed, std::sin(upperAngle) * speed);
        m_projectiles.push_back(std::make_unique<Projectile>(position, upperVelocity, skills, ownerId));

        // Lower projectile (offset downward)
        float lowerAngle = baseAngle + radianOffset;
        Vector2 lowerVelocity(std::cos(lowerAngle) * speed, std::sin(lowerAngle) * speed);
        m_projectiles.push_back(std::make_unique<Projectile>(position, lowerVelocity, skills, ownerId));
    }
    else {
        // Single projectile
        m_projectiles.push_back(std::make_unique<Projectile>(position, velocity, skills, ownerId));
    }
}

void Physics::RemoveProjectile(Projectile* projectile) {
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [projectile](const std::unique_ptr<Projectile>& p) { return p.get() == projectile; }),
        m_projectiles.end()
    );
}

void Physics::CheckCollisions(std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<SkillOrb>>& skillOrbs) {
    CheckProjectileCollisions(players, skillOrbs);

    // Only use terrain collision (no platform)
    if (m_terrain) {
        CheckPlayerTerrainCollisions(players);
    }

    CheckSkillOrbCollisions(players, skillOrbs);
}

void Physics::CheckProjectileCollisions(std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<SkillOrb>>& skillOrbs) {
    for (auto& projectile : m_projectiles) {
        if (!projectile->IsActive()) continue;

        // Check collision with skill orbs - projectiles collect them for their owner
        for (auto& orb : skillOrbs) {
            if (orb->IsCollected() || !orb->IsActive()) continue;

            // Check if projectile intersects with skill orb
            Vector2 distance = orb->GetPosition() - projectile->GetPosition();
            float distanceLength = distance.Length();
            float combinedRadius = orb->GetRadius() + projectile->GetRadius();

            if (distanceLength < combinedRadius) {
                // Find the owner player and give them the skill
                for (auto& player : players) {
                    if (player->GetId() == projectile->GetOwnerId()) {
                        orb->OnCollected(player.get());
                        break;
                    }
                }
            }
        }

        // Check collision with terrain only (no platform)
        bool hitTerrain = false;
        if (m_terrain) {
            // Check if projectile hit terrain
            if (m_terrain->IsCircleSolid(projectile->GetPosition(), projectile->GetRadius())) {
                hitTerrain = true;
            }
        }

        if (hitTerrain) {
            // Handle terrain/platform collision
            if (projectile->HasHeal()) {
                // Apply healing to all allies in AOE
                ApplyHealing(projectile->GetPosition(), projectile->GetExplosionRadius(),
                    projectile->GetOwnerId(), players);
            }
            else if (projectile->HasTeleportBall()) {
                // Teleport the owner to this location (unless it's void)
                Vector2 teleportPos = projectile->GetPosition();
                // Make sure teleport position is above terrain
                if (teleportPos.y < 850.0f) {
                    for (auto& player : players) {
                        if (player->GetId() == projectile->GetOwnerId()) {
                            player->SetPosition(teleportPos);
                            break;
                        }
                    }
                }
            }
            else {
                // Apply explosion effects if not pure teleport
                if (projectile->GetExplosionRadius() > 0) {
                    ApplyExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(),
                        projectile->GetExplosionForce(), players);

                    // Destroy terrain only if projectile damages terrain
                    if (m_terrain && projectile->DamagesTerrain()) {
                        m_terrain->DestroyCircle(projectile->GetPosition(), projectile->GetExplosionRadius());
                    }
                }
            }

            projectile->SetActive(false);
            continue;
        }

        // Check collision with players
        for (auto& player : players) {
            if (!player->IsAlive() || player->GetId() == projectile->GetOwnerId()) continue;

            CollisionInfo playerCollision = CheckCircleCollision(
                projectile->GetPosition(), projectile->GetRadius(),
                player->GetPosition(), player->GetRadius()
            );

            if (playerCollision.hasCollision) {
                // Handle player collision
                if (projectile->HasHeal()) {
                    // Apply healing to all allies in AOE
                    ApplyHealing(projectile->GetPosition(), projectile->GetExplosionRadius(),
                        projectile->GetOwnerId(), players);
                }
                else if (projectile->HasTeleportBall()) {
                    // Teleport the owner to this location
                    for (auto& owner : players) {
                        if (owner->GetId() == projectile->GetOwnerId()) {
                            owner->SetPosition(player->GetPosition());
                            break;
                        }
                    }
                }
                else {
                    // Damage player (if damage > 0)
                    float damage = projectile->GetDamage();
                    if (damage > 0) {
                        player->TakeDamage(damage);
                    }

                    // Apply explosion effects
                    if (projectile->GetExplosionRadius() > 0) {
                        ApplyExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(),
                            projectile->GetExplosionForce(), players);

                        // Destroy terrain only if projectile damages terrain
                        if (m_terrain && projectile->DamagesTerrain()) {
                            m_terrain->DestroyCircle(projectile->GetPosition(), projectile->GetExplosionRadius());
                        }
                    }
                }

                projectile->SetActive(false);
                break;
            }
        }
    }
}

void Physics::CheckPlayerPlatformCollisions(std::vector<std::unique_ptr<Player>>& players) {
    for (auto& player : players) {
        if (!player->IsAlive()) continue;

        Vector2 pos = player->GetPosition();

        // Check if player fell into the void (below screen)
        if (pos.y > 850.0f) {  // Screen height is 800, give some buffer
            player->TakeDamage(player->GetMaxHealth());  // Kill the player
            continue;
        }

        CollisionInfo collision = CheckCirclePlatformCollision(pos, player->GetRadius());
        if (collision.hasCollision) {
            // Push player out of platform
            Vector2 correction = collision.normal * collision.penetration;
            player->SetPosition(pos + correction);

            // Stop downward velocity
            Vector2 velocity = player->GetVelocity();
            if (velocity.y > 0) {
                player->SetVelocity(Vector2(velocity.x, 0));
            }
        }
    }
}

void Physics::CheckSkillOrbCollisions(std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<SkillOrb>>& skillOrbs) {
    for (auto& orb : skillOrbs) {
        if (orb->IsCollected()) continue;

        for (auto& player : players) {
            if (!player->IsAlive()) continue;

            CollisionInfo collision = CheckCircleCollision(
                orb->GetPosition(), orb->GetRadius(),
                player->GetPosition(), player->GetRadius()
            );

            if (collision.hasCollision) {
                orb->OnCollected(player.get());
                break;
            }
        }
    }
}

CollisionInfo Physics::CheckCircleCollision(const Vector2& pos1, float radius1,
    const Vector2& pos2, float radius2) const {
    CollisionInfo info;
    info.hasCollision = false;

    Vector2 distance = pos2 - pos1;
    float distanceLength = distance.Length();
    float combinedRadius = radius1 + radius2;

    if (distanceLength < combinedRadius) {
        info.hasCollision = true;
        info.penetration = combinedRadius - distanceLength;

        if (distanceLength > 0) {
            info.normal = distance * (1.0f / distanceLength);
            info.point = pos1 + info.normal * radius1;
        }
        else {
            info.normal = Vector2(1, 0);
            info.point = pos1;
        }
    }

    return info;
}

CollisionInfo Physics::CheckCirclePlatformCollision(const Vector2& pos, float radius) const {
    CollisionInfo info;
    info.hasCollision = false;

    // Check if circle intersects with platform rectangle
    Vector2 closestPoint;
    closestPoint.x = clamp(pos.x, m_platformPosition.x, m_platformPosition.x + m_platformWidth);
    closestPoint.y = clamp(pos.y, m_platformPosition.y, m_platformPosition.y + m_platformHeight);

    Vector2 distance = pos - closestPoint;
    float distanceLength = distance.Length();

    if (distanceLength < radius) {
        info.hasCollision = true;
        info.penetration = radius - distanceLength;

        if (distanceLength > 0) {
            info.normal = distance * (1.0f / distanceLength);
        }
        else {
            // Circle is inside platform, push up
            info.normal = Vector2(0, -1);
        }

        info.point = closestPoint;
    }

    return info;
}

void Physics::ApplyExplosion(const Vector2& center, float radius, float force,
    std::vector<std::unique_ptr<Player>>& players) {
    for (auto& player : players) {
        if (!player->IsAlive()) continue;

        Vector2 distance = player->GetPosition() - center;
        float distanceLength = distance.Length();

        if (distanceLength < radius && distanceLength > 0) {
            // Calculate explosion force based on distance
            float forceMagnitude = force * (1.0f - (distanceLength / radius));
            Vector2 explosionForce = distance * (forceMagnitude / distanceLength);

            player->ApplyForce(explosionForce);

            // Also apply damage based on distance
            float damage = 30.0f * (1.0f - (distanceLength / radius));
            player->TakeDamage(damage);
        }
    }
}

bool Physics::IsPointInBounds(const Vector2& point, const Vector2& boundsMin, const Vector2& boundsMax) const {
    return point.x >= boundsMin.x && point.x <= boundsMax.x &&
        point.y >= boundsMin.y && point.y <= boundsMax.y;
}

void Physics::CheckPlayerTerrainCollisions(std::vector<std::unique_ptr<Player>>& players) {
    if (!m_terrain) return;

    for (auto& player : players) {
        if (!player->IsAlive()) continue;

        Vector2 pos = player->GetPosition();
        float radius = player->GetRadius();

        // Check if player fell into the void (below screen)
        if (pos.y > 850.0f) { 
            player->TakeDamage(player->GetMaxHealth());  // Kill the player
            continue;
        }

        // Check if player is overlapping with solid terrain
        bool onGround = false;
        Vector2 correction(0, 0);

        // Check bottom of player (feet) - sample multiple points for stability
        int bottomY = (int)(pos.y + radius);

        // Sample 3 points across the bottom of the player for more stable ground detection
        int leftX = (int)(pos.x - radius * 0.5f);
        int centerX = (int)pos.x;
        int rightX = (int)(pos.x + radius * 0.5f);

        int groundY = -1;

        // Find highest solid pixel among the samples
        int leftGround = m_terrain->FindTopSolidPixel(leftX, bottomY - (int)radius);
        int centerGround = m_terrain->FindTopSolidPixel(centerX, bottomY - (int)radius);
        int rightGround = m_terrain->FindTopSolidPixel(rightX, bottomY - (int)radius);

        // Use the highest ground found
        if (leftGround >= 0 && (groundY < 0 || leftGround < groundY)) groundY = leftGround;
        if (centerGround >= 0 && (groundY < 0 || centerGround < groundY)) groundY = centerGround;
        if (rightGround >= 0 && (groundY < 0 || rightGround < groundY)) groundY = rightGround;

        if (groundY >= 0) {
            float distanceToGround = groundY - (pos.y + radius);

            // If player is overlapping or very close to ground
            if (distanceToGround < 3.0f) {
                onGround = true;
                // Smooth the correction to prevent jittering
                correction.y = distanceToGround * 0.5f;  // Smoother adjustment
            }
        }

        // Apply horizontal terrain collision (check multiple points around player)
        for (int angle = 0; angle < 360; angle += 30) {
            float rad = angle * 3.14159265f / 180.0f;
            int checkX = (int)(pos.x + std::cos(rad) * radius);
            int checkY = (int)(pos.y + std::sin(rad) * radius);

            if (m_terrain->IsPixelSolid(checkX, checkY)) {
                // Push player away from solid pixel
                Vector2 pushDir(std::cos(rad), std::sin(rad));
                correction = correction - pushDir * 2.0f;
            }
        }

        // Apply correction
        if (correction.Length() > 0.1f) {
            player->SetPosition(pos + correction);

            // Stop downward velocity if on ground
            if (onGround) {
                Vector2 velocity = player->GetVelocity();
                if (velocity.y > 0) {
                    player->SetVelocity(Vector2(velocity.x, 0));
                }
            }
        }
    }
}

CollisionInfo Physics::CheckCircleTerrainCollision(const Vector2& pos, float radius, Terrain* terrain) const {
    CollisionInfo info;
    info.hasCollision = false;

    if (!terrain) return info;

    // Simple check: if the circle overlaps solid terrain pixels
    if (terrain->IsCircleSolid(pos, radius)) {
        info.hasCollision = true;
        info.point = pos;
        info.normal = Vector2(0, -1);  // Default push up
        info.penetration = radius;
    }

    return info;
}

void Physics::ApplyHealing(const Vector2& center, float radius, int ownerId,
    std::vector<std::unique_ptr<Player>>& players) {
    for (auto& player : players) {
        if (!player->IsAlive()) continue;

        Vector2 distance = player->GetPosition() - center;
        float distanceLength = distance.Length();

        // Heal all players (including thrower) within radius
        if (distanceLength < radius) {
            // Heal for 30% of max health
            float healAmount = player->GetMaxHealth() * 0.3f;
            player->Heal(healAmount);
        }
    }
}
