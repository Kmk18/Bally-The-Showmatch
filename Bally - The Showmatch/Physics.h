#pragma once
#include <vector>
#include <memory>
#include "Vector2.h"

class Player;
class Projectile;
class SkillOrb;
class Terrain;
class ExplosionAnimation;

struct CollisionInfo {
    bool hasCollision;
    Vector2 point;
    Vector2 normal;
    float penetration;
};

enum class ProjectileType {
    NORMAL,
    SPLIT,
    ENHANCED_DAMAGE,
    ENHANCED_EXPLOSIVE,
    TELEPORT,
    HEAL
};

class Projectile {
public:
    Projectile(const Vector2& position, const Vector2& velocity, ProjectileType type, int ownerId);
    Projectile(const Vector2& position, const Vector2& velocity, const std::vector<int>& skillTypes, int ownerId);

    void Update(float deltaTime);
    void Draw(class Renderer* renderer) const;

    const Vector2& GetPosition() const { return m_position; }
    const Vector2& GetVelocity() const { return m_velocity; }
    float GetRadius() const { return m_radius; }
    bool IsActive() const { return m_active; }
    ProjectileType GetType() const { return m_type; }
    int GetOwnerId() const { return m_ownerId; }
    float GetDamage() const;
    float GetExplosionRadius() const;
    float GetExplosionForce() const;
    bool HasSplit() const { return m_hasSplit; }
    bool HasPowerBall() const { return m_hasPowerBall; }
    bool HasExplosiveBall() const { return m_hasExplosiveBall; }
    bool HasTeleportBall() const { return m_hasTeleportBall; }
    bool HasHeal() const { return m_hasHeal; }
    bool DamagesTerrain() const;

    void SetActive(bool active) { m_active = active; }
    void SetPosition(const Vector2& position) { m_position = position; }
    void SetVelocity(const Vector2& velocity) { m_velocity = velocity; }

private:
    Vector2 m_position;
    Vector2 m_velocity;
    Vector2 m_acceleration;
    float m_radius;
    float m_mass;
    ProjectileType m_type;
    int m_ownerId;
    bool m_active;
    float m_lifetime;
    float m_maxLifetime;

    // Skill mixing flags
    bool m_hasSplit;
    bool m_hasPowerBall;
    bool m_hasExplosiveBall;
    bool m_hasTeleportBall;
    bool m_hasHeal;

    // Physics constants
    static constexpr float GRAVITY = 980.0f;
    static constexpr float AIR_RESISTANCE = 0.98f;
    static constexpr float DEFAULT_RADIUS = 8.0f;
    static constexpr float DEFAULT_MASS = 0.5f;
    static constexpr float MAX_LIFETIME = 10.0f;
};

class Physics {
public:
    Physics();
    ~Physics();

    void Update(float deltaTime);
    void Draw(class Renderer* renderer);
    void AddProjectile(std::unique_ptr<Projectile> projectile);
    void AddProjectileWithSkills(const Vector2& position, const Vector2& velocity, const std::vector<int>& skills, int ownerId);
    void RemoveProjectile(Projectile* projectile);
    bool HasActiveProjectiles() const { return !m_projectiles.empty(); }
    const std::vector<std::unique_ptr<Projectile>>& GetProjectiles() const { return m_projectiles; }

    void CheckCollisions(std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<SkillOrb>>& skillOrbs);

    bool IsPointInBounds(const Vector2& point, const Vector2& boundsMin, const Vector2& boundsMax) const;
    Vector2 GetPlatformBounds() const { return Vector2(m_platformWidth, m_platformHeight); }

    CollisionInfo CheckCircleCollision(const Vector2& pos1, float radius1,
        const Vector2& pos2, float radius2) const;
    CollisionInfo CheckCirclePlatformCollision(const Vector2& pos, float radius) const;
    CollisionInfo CheckCircleTerrainCollision(const Vector2& pos, float radius, Terrain* terrain) const;

    void ApplyExplosion(const Vector2& center, float radius, float force,
        std::vector<std::unique_ptr<Player>>& players);
    void ApplyHealing(const Vector2& center, float radius, int ownerId,
        std::vector<std::unique_ptr<Player>>& players);

    // Set the terrain for collision detection
    void SetTerrain(Terrain* terrain) { m_terrain = terrain; }

    // Set renderer for explosion animations (needed to load sprites)
    void SetRenderer(class Renderer* renderer) { m_renderer = renderer; }

    // Debug visualization
    void SetDebugDrawContours(bool enable) { m_debugDrawContours = enable; }
    bool GetDebugDrawContours() const { return m_debugDrawContours; }

private:
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
    std::vector<std::unique_ptr<ExplosionAnimation>> m_explosions;
    Terrain* m_terrain; // Reference to terrain for collision detection
    class Renderer* m_renderer; // Reference to renderer for explosion sprite loading

    // Create explosion animation at position
    void CreateExplosion(const Vector2& position, float radius, bool isBigExplosion);

    // Debug visualization
    bool m_debugDrawContours;
    struct DebugContourData {
        Vector2 playerPos;
        float playerRadius;
        std::vector<Vector2> samplePoints;
        std::vector<Vector2> groundPoints;
        int groundY;
    };
    std::vector<DebugContourData> m_debugContourData;

    // World bounds
    float m_platformWidth;
    float m_platformHeight;
    Vector2 m_platformPosition;

    // Collision detection
    void CheckProjectileCollisions(std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<SkillOrb>>& skillOrbs);
    void CheckPlayerTerrainCollisions(std::vector<std::unique_ptr<Player>>& players);
    void CheckPlayerPlatformCollisions(std::vector<std::unique_ptr<Player>>& players);
    void CheckSkillOrbCollisions(std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<SkillOrb>>& skillOrbs);

    // Constants
    static constexpr float PLATFORM_WIDTH = 800.0f;
    static constexpr float PLATFORM_HEIGHT = 50.0f;
    static constexpr float WORLD_WIDTH = 1200.0f;
    static constexpr float WORLD_HEIGHT = 800.0f;
};
