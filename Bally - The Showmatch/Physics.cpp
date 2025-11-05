#include "Physics.h"
#include "Player.h"
#include "SkillOrb.h"
#include "Terrain.h"
#include "UI.h"
#include "ExplosionAnimation.h"
#include <cmath>
#include <algorithm>


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
        return 80.0f; // Heal AOE radius
    }

    float baseRadius = 30.0f;

    // Teleport ball has no explosion
    if (m_hasTeleportBall && !m_hasExplosiveBall && !m_hasPowerBall) {
        return 0.0f;
    }

    // Explosive ball increases explosion radius
    if (m_hasExplosiveBall) {
        baseRadius = 70.0f;
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

Physics::Physics() : m_terrain(nullptr), m_renderer(nullptr), m_platformWidth(PLATFORM_WIDTH), m_platformHeight(PLATFORM_HEIGHT),
m_platformPosition(200.0f, 650.0f), m_debugDrawContours(false) {
}

Physics::~Physics() {
    m_projectiles.clear();
    m_explosions.clear();
}

void Physics::Update(float deltaTime) {
    // Get map dimensions for bounds checking
    float mapWidth = m_terrain ? static_cast<float>(m_terrain->GetWidth()) : 1200.0f;
    float mapHeight = m_terrain ? static_cast<float>(m_terrain->GetHeight()) : 800.0f;
    
    // Update projectiles
    for (auto& projectile : m_projectiles) {
        projectile->Update(deltaTime);
        
        // Deactivate projectiles that go outside map bounds (with some buffer)
        Vector2 pos = projectile->GetPosition();
        float buffer = 100.0f; // Allow some buffer outside map before deactivating
        if (pos.x < -buffer || pos.x > mapWidth + buffer || pos.y > mapHeight + buffer) {
            projectile->SetActive(false);
        }
    }

    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) { return !p->IsActive(); }),
        m_projectiles.end()
    );

    // Update explosions
    for (auto& explosion : m_explosions) {
        explosion->Update(deltaTime);
    }

    // Remove finished explosions
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const std::unique_ptr<ExplosionAnimation>& e) { return e->IsFinished(); }),
        m_explosions.end()
    );
}

void Physics::Draw(class Renderer* renderer) {
    // Draw all active projectiles
    for (const auto& projectile : m_projectiles) {
        if (projectile->IsActive()) {
            projectile->Draw(renderer);
        }
    }

    // Draw all active explosions
    for (const auto& explosion : m_explosions) {
        if (!explosion->IsFinished()) {
            explosion->Draw(renderer);
        }
    }

    // Draw debug contour visualization
    if (m_debugDrawContours) {
        for (const auto& data : m_debugContourData) {
            // Draw sample points (vertical lines from player bottom)
            for (const auto& samplePoint : data.samplePoints) {
                renderer->DrawLine(samplePoint,
                    Vector2(samplePoint.x, samplePoint.y - data.playerRadius * 2.5f),
                    Color(255, 255, 0, 128)); // Yellow semi-transparent
            }

            // Draw ground detection points (where terrain was found)
            for (const auto& groundPoint : data.groundPoints) {
                renderer->DrawCircle(groundPoint, 3.0f, Color(0, 255, 0, 255)); // Green circles
            }

            // Draw the highest ground point (the one actually used)
            if (data.groundY >= 0) {
                renderer->DrawCircle(Vector2(data.playerPos.x, (float)data.groundY),
                    5.0f, Color(255, 0, 0, 255)); // Red circle for active ground point

                // Draw line from player bottom to ground point
                renderer->DrawLine(
                    Vector2(data.playerPos.x, data.playerPos.y + data.playerRadius),
                    Vector2(data.playerPos.x, (float)data.groundY),
                    Color(255, 0, 0, 255)); // Red line
            }

            // Draw player bounding circle
            renderer->DrawCircle(data.playerPos, data.playerRadius, Color(0, 255, 255, 128)); // Cyan
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
        const float angleOffset = 5.0f; // Reduced from 10.0f for tighter spread
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
    // Clear debug data from previous frame
    m_debugContourData.clear();

    CheckProjectileCollisions(players, skillOrbs);

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
                for (auto& player : players) {
                    if (player->GetId() == projectile->GetOwnerId()) {
                        orb->OnCollected(player.get());
                        break;
                    }
                }
            }
        }

        bool hitTerrain = false;
        if (m_terrain && m_terrain->IsCircleSolid(projectile->GetPosition(), projectile->GetRadius())) {
            hitTerrain = true;
        }

        if (hitTerrain) {
            // Handle terrain/platform collision
            if (projectile->HasHeal()) {
                // Apply healing to all allies in AOE
                ApplyHealing(projectile->GetPosition(), projectile->GetExplosionRadius(),
                    projectile->GetOwnerId(), players);
                CreateHealAnimation(projectile->GetPosition(), projectile->GetExplosionRadius());
            }
            else if (projectile->HasTeleportBall()) {
                // Teleport the owner to this location (unless it's void)
                Vector2 teleportPos = projectile->GetPosition();
                // Make sure teleport position is within map bounds
                float mapHeight = m_terrain ? static_cast<float>(m_terrain->GetHeight()) : 800.0f;
                CreateTeleportAnimation(projectile->GetPosition(), std::max(projectile->GetExplosionRadius(), 50.0f));
                
                if (teleportPos.y >= 0 && teleportPos.y < mapHeight) {
                    for (auto& player : players) {
                        if (player->GetId() == projectile->GetOwnerId()) {
                            int teleportX = (int)teleportPos.x;
                            int groundY = m_terrain ? m_terrain->FindTopSolidPixel(teleportX, (int)teleportPos.y) : -1;
                            
                            if (groundY >= 0) {
                                float playerRadius = player->GetRadius();
                                float buffer = 5.0f;
                                teleportPos.y = (float)groundY - playerRadius - buffer;
                            }
                            else {
                                // If no ground found, add height to prevent falling through
                                float playerRadius = player->GetRadius();
                                teleportPos.y -= playerRadius * 2.0f;
                            }
                            
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

                    // Create explosion animation (use big explosion if explosive buff, otherwise small)
                    bool isBigExplosion = projectile->HasExplosiveBall();
                    CreateExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(), isBigExplosion);

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
                    CreateHealAnimation(projectile->GetPosition(), projectile->GetExplosionRadius());
                }
                else if (projectile->HasTeleportBall()) {
                    CreateTeleportAnimation(projectile->GetPosition(), std::max(projectile->GetExplosionRadius(), 50.0f));
                    
                    // Teleport the owner to this location
                    for (auto& owner : players) {
                        if (owner->GetId() == projectile->GetOwnerId()) {
                            Vector2 teleportPos = player->GetPosition();
                            
                            // Find ground surface at teleport X position to ensure safe placement
                            int teleportX = (int)teleportPos.x;
                            int groundY = m_terrain ? m_terrain->FindTopSolidPixel(teleportX, (int)teleportPos.y) : -1;
                            
                            if (groundY >= 0) {
                                // Place player above ground with player radius + buffer
                                float playerRadius = owner->GetRadius();
                                float buffer = 5.0f;
                                teleportPos.y = (float)groundY - playerRadius - buffer;
                            }
                            else {
                                // If no ground found, add height to prevent falling through
                                float playerRadius = owner->GetRadius();
                                teleportPos.y -= playerRadius * 2.0f;
                            }
                            
                            owner->SetPosition(teleportPos);
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

                        bool isBigExplosion = projectile->HasExplosiveBall();
                        CreateExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(), isBigExplosion);

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


void Physics::ApplyExplosion(const Vector2& center, float radius, float force,
    std::vector<std::unique_ptr<Player>>& players) {
    for (auto& player : players) {
        if (!player->IsAlive()) continue;

        Vector2 distance = player->GetPosition() - center;
        float distanceLength = distance.Length();

        if (distanceLength < radius && distanceLength > 0) {
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
        Vector2 velocity = player->GetVelocity();
        float radius = player->GetRadius();

        // Check if player fell into the void (below map)
        // Use terrain height if available, otherwise fallback to default
        float mapHeight = static_cast<float>(m_terrain->GetHeight());
        float deathThreshold = mapHeight + 50.0f;
        if (pos.y > deathThreshold) {
            player->TakeDamage(player->GetMaxHealth());
            continue;
        }

        // High-accuracy ground following system
        bool onGround = false;
        int groundY = -1;

        // Sample many points across player's width for maximum accuracy
        const int sampleCount = 11; // Increased to 11 for very accurate terrain detection
        const float sampleWidthMultiplier = 0.8f; // Adjust this to match character sprite width (0.5-1.5)
        const float maxUpwardSearch = radius * 0.5f; // Only search slightly above player (prevent ceiling detection)
        int samples[sampleCount];
        for (int i = 0; i < sampleCount; i++) {
            float t = (i / (float)(sampleCount - 1)) - 0.5f;
            int sampleX = (int)(pos.x + t * radius * sampleWidthMultiplier);
            int startY = (int)(pos.y + radius);
            int searchStartY = std::max(0, (int)(startY - maxUpwardSearch));
            samples[i] = m_terrain->FindTopSolidPixel(sampleX, searchStartY);
        }

        // Find the highest (lowest Y value) ground point that's not too far above player
        for (int i = 0; i < sampleCount; i++) {
            if (samples[i] >= 0) {
                float currentBottom = pos.y + radius;
                float distanceToSample = samples[i] - currentBottom;

                // Only accept ground that's below or very slightly above player
                // This prevents snapping to ceiling in C-shaped terrain
                if (distanceToSample >= -maxUpwardSearch) {
                    if (groundY < 0 || samples[i] < groundY) {
                        groundY = samples[i];
                    }
                }
            }
        }

        // Store debug visualization data
        if (m_debugDrawContours) {
            DebugContourData debugData;
            debugData.playerPos = pos;
            debugData.playerRadius = radius;
            debugData.groundY = groundY;

            // Store sample points and ground points
            for (int i = 0; i < sampleCount; i++) {
                float t = (i / (float)(sampleCount - 1)) - 0.5f;
                int sampleX = (int)(pos.x + t * radius * sampleWidthMultiplier);
                int startY = (int)(pos.y + radius);
                debugData.samplePoints.push_back(Vector2((float)sampleX, (float)startY));

                if (samples[i] >= 0) {
                    debugData.groundPoints.push_back(Vector2((float)sampleX, (float)samples[i]));
                }
            }

            m_debugContourData.push_back(debugData);
        }

        // Ground following logic
        if (groundY >= 0) {
            float targetY = groundY - radius; // Target position (feet on ground)
            float currentBottom = pos.y + radius;
            float distanceToGround = groundY - currentBottom;

            // Case 1: Player is embedded in terrain (negative distance)
            if (distanceToGround < -2.0f) {
                // Push player up to surface
                onGround = true;
                pos.y = targetY;
                if (velocity.y > 0) {
                    velocity.y = 0;
                }
            }
            // Case 2: Player is on or very close to ground - snap for pixel-perfect contour
            else if (distanceToGround >= -2.0f && distanceToGround <= 3.0f) {
                onGround = true;
                pos.y = targetY; // Direct snap - no smoothing for accuracy
                if (velocity.y > 0) {
                    velocity.y = 0;
                }
            }
            // Case 3: Player is falling and close to ground - smooth landing
            else if (velocity.y > 0 && distanceToGround > 3.0f && distanceToGround <= 15.0f) {
                float smoothFactor = 0.5f;
                pos.y = pos.y + distanceToGround * smoothFactor;

                // Check if we're now close enough to snap
                float newDistanceToGround = groundY - (pos.y + radius);
                if (newDistanceToGround <= 3.0f) {
                    onGround = true;
                    pos.y = targetY;
                    velocity.y = 0;
                }
            }
            // Case 4: Player is far above ground (>15px) - let gravity work, but check collision
            else if (distanceToGround > 15.0f && velocity.y > 0) {
            }
            // Case 5: Player falling and would pass through ground this frame
            else if (velocity.y > 0 && distanceToGround > 0) {
                // Check if velocity would make player pass through
                float nextBottom = currentBottom + velocity.y;
                if (nextBottom >= groundY) {
                    // Would pass through - land on ground instead
                    onGround = true;
                    pos.y = targetY;
                    velocity.y = 0;
                }
            }
        }

        // Handle steep slope traversal (Gunny-style climbing)
        if (onGround && std::abs(velocity.x) > 0.1f) {
            float lookAheadDist = radius * 2.0f;
            int checkX = (int)(pos.x + (velocity.x > 0 ? lookAheadDist : -lookAheadDist));
            int checkStartY = (int)(pos.y + radius);
            int groundAhead = m_terrain->FindTopSolidPixel(checkX, checkStartY - (int)(radius * 6));

            if (groundAhead >= 0) {
                float heightDiff = groundAhead - groundY;

                // Allow climbing extremely steep slopes
                // Further increased to allow near-vertical terrain traversal
                if (heightDiff < radius * 8.0f && heightDiff > -radius * 4) {
                    // Smoothly adjust Y to climb slope - faster interpolation for responsive climbing
                    float targetClimbY = groundAhead - radius;
                    pos.y += (targetClimbY - pos.y) * 0.3f;
                }
                // If slope is too steep, stop horizontal movement
                // Increased threshold to 7x radius to match climbing capability
                else if (heightDiff >= radius * 8.0f) {
                    velocity.x *= 0.5f;
                }
            }
        }

        // Handle collision with terrain when embedded (pushed into walls)
        bool embedded = false;
        Vector2 pushOut(0, 0);
        int embedCount = 0;

        // Check if player center is inside terrain
        for (int angle = 0; angle < 360; angle += 45) {
            float rad = angle * 3.14159265f / 180.0f;
            int checkX = (int)(pos.x + std::cos(rad) * radius * 0.8f);
            int checkY = (int)(pos.y + std::sin(rad) * radius * 0.8f);

            if (m_terrain->IsPixelSolid(checkX, checkY)) {
                // Calculate push direction (away from solid)
                Vector2 pushDir(-std::cos(rad), -std::sin(rad));
                pushOut = pushOut + pushDir;
                embedCount++;
                embedded = true;
            }
        }

        // Apply push out correction if embedded
        if (embedded && embedCount > 0) {
            pushOut = pushOut * (1.0f / embedCount);
            pos = pos + pushOut * 2.0f;

            // Reduce velocity when hitting walls
            if (std::abs(pushOut.x) > 0.1f) {
                velocity.x *= 0.3f;
            }
        }

        // Update player position and velocity
        player->SetPosition(pos);
        player->SetVelocity(velocity);
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

        // Heal all players within radius
        if (distanceLength < radius) {
            // Heal for 30% of max health
            float healAmount = player->GetMaxHealth() * 0.3f;
            player->Heal(healAmount);
        }
    }
}

void Physics::CreateExplosion(const Vector2& position, float radius, bool isBigExplosion) {
    if (!m_renderer) return;

    auto explosion = std::make_unique<ExplosionAnimation>(position, radius, isBigExplosion);
    if (explosion->Load(m_renderer)) {
        m_explosions.push_back(std::move(explosion));
    }
}

void Physics::CreateTeleportAnimation(const Vector2& position, float radius) {
    if (!m_renderer) return;

    float animRadius = std::max(radius, 50.0f);
    auto animation = std::make_unique<ExplosionAnimation>(position, animRadius, ExplosionAnimationType::TELEPORT);
    if (animation->Load(m_renderer)) {
        m_explosions.push_back(std::move(animation));
    }
}

void Physics::CreateHealAnimation(const Vector2& position, float radius) {
    if (!m_renderer) return;

    float animRadius = std::max(radius, 50.0f);
    auto animation = std::make_unique<ExplosionAnimation>(position, animRadius, ExplosionAnimationType::HEAL);
    if (animation->Load(m_renderer)) {
        m_explosions.push_back(std::move(animation));
    }
}

