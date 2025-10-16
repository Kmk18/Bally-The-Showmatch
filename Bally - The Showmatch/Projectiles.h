#pragma once

#include "Vector2.h"
#include "Physics.h"
#include <memory>
#include <vector>

class Renderer;

// Forward declarations
class Projectile;
enum class ProjectileType;

namespace ProjectileUtils {
    // Utility function to get projectile type name
    inline const char* GetProjectileTypeName(ProjectileType type) {
        switch (type) {
        case ProjectileType::NORMAL: return "Normal";
        case ProjectileType::SPLIT: return "Split";
        case ProjectileType::ENHANCED_DAMAGE: return "Enhanced Damage";
        case ProjectileType::ENHANCED_EXPLOSIVE: return "Enhanced Explosive";
        case ProjectileType::TELEPORT: return "Teleport";
        default: return "Unknown";
        }
    }

    // Utility function to simulate trajectory (for UI preview)
    inline std::vector<Vector2> SimulateTrajectory(const Vector2& startPos, const Vector2& velocity,
                                                     float timeStep = 0.1f, int maxSteps = 50) {
        std::vector<Vector2> points;
        Vector2 pos = startPos;
        Vector2 vel = velocity;
        const float gravity = 980.0f;
        const float airResistance = 0.98f;

        for (int i = 0; i < maxSteps; ++i) {
            points.push_back(pos);

            // Apply gravity
            vel.y += gravity * timeStep;

            // Apply air resistance
            vel = vel * airResistance;

            // Update position
            pos = pos + vel * timeStep;

            // Stop if off screen or hit ground
            if (pos.y > 900.0f || pos.x < -100.0f || pos.x > 1300.0f) {
                break;
            }
        }

        return points;
    }
}
