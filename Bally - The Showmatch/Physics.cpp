#include "Physics.h"
#include "Player.h"
#include "SkillOrb.h"
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
    m_lifetime(0.0f), m_maxLifetime(MAX_LIFETIME) {
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
    }

    renderer->SetDrawColor(projectileColor);
    renderer->DrawCircle(m_position, m_radius, projectileColor);
}

float Projectile::GetDamage() const {
    switch (m_type) {
    case ProjectileType::NORMAL:
    case ProjectileType::ENHANCED_EXPLOSIVE:
    case ProjectileType::TELEPORT:
        return 25.0f;
    case ProjectileType::SPLIT:
        return 15.0f;
    case ProjectileType::ENHANCED_DAMAGE:
        return 50.0f;
    default:
        return 25.0f;
    }
}

float Projectile::GetExplosionRadius() const {
    switch (m_type) {
    case ProjectileType::NORMAL:
    case ProjectileType::ENHANCED_DAMAGE:
    case ProjectileType::SPLIT:
        return 80.0f;
    case ProjectileType::ENHANCED_EXPLOSIVE:
        return 150.0f;
    case ProjectileType::TELEPORT:
        return 0.0f; // No explosion
    default:
        return 80.0f;
    }
}

float Projectile::GetExplosionForce() const {
    switch (m_type) {
    case ProjectileType::NORMAL:
    case ProjectileType::SPLIT:
    case ProjectileType::TELEPORT:
        return 500.0f;
    case ProjectileType::ENHANCED_DAMAGE:
        return 300.0f;
    case ProjectileType::ENHANCED_EXPLOSIVE:
        return 800.0f;
    default:
        return 500.0f;
    }
}

Physics::Physics() : m_platformWidth(PLATFORM_WIDTH), m_platformHeight(PLATFORM_HEIGHT),
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

void Physics::AddProjectile(std::unique_ptr<Projectile> projectile) {
    m_projectiles.push_back(std::move(projectile));
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
    CheckPlayerPlatformCollisions(players);
    CheckSkillOrbCollisions(players, skillOrbs);
}

void Physics::CheckProjectileCollisions(std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<SkillOrb>>& skillOrbs) {
    for (auto& projectile : m_projectiles) {
        if (!projectile->IsActive()) continue;

        // Check collision with platform
        CollisionInfo platformCollision = CheckCirclePlatformCollision(projectile->GetPosition(), projectile->GetRadius());
        if (platformCollision.hasCollision) {
            // Handle platform collision
            if (projectile->GetType() == ProjectileType::TELEPORT) {
                // Teleport the owner to this location
                for (auto& player : players) {
                    if (player->GetId() == projectile->GetOwnerId()) {
                        player->SetPosition(projectile->GetPosition());
                        break;
                    }
                }
            }
            else {
                // Create explosion
                ApplyExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(),
                    projectile->GetExplosionForce(), players);
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
                if (projectile->GetType() == ProjectileType::TELEPORT) {
                    // Teleport the owner to this location
                    for (auto& owner : players) {
                        if (owner->GetId() == projectile->GetOwnerId()) {
                            owner->SetPosition(player->GetPosition());
                            break;
                        }
                    }
                }
                else {
                    // Damage player and create explosion
                    player->TakeDamage(projectile->GetDamage());
                    ApplyExplosion(projectile->GetPosition(), projectile->GetExplosionRadius(),
                        projectile->GetExplosionForce(), players);
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

        CollisionInfo collision = CheckCirclePlatformCollision(player->GetPosition(), player->GetRadius());
        if (collision.hasCollision) {
            // Push player out of platform
            Vector2 correction = collision.normal * collision.penetration;
            player->SetPosition(player->GetPosition() + correction);

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
