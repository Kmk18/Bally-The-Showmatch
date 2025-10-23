#pragma once
#include "Vector2.h"

class Camera {
public:
    Camera(float viewportWidth, float viewportHeight);

    // Update camera position
    void Update(float deltaTime);

    // Set the target position for the camera to follow
    void SetTarget(const Vector2& target);

    // Get camera position (top-left corner of viewport)
    Vector2 GetPosition() const { return m_position; }

    // Get viewport size
    float GetViewportWidth() const { return m_viewportWidth; }
    float GetViewportHeight() const { return m_viewportHeight; }

    // Set map boundaries so camera doesn't go outside
    void SetMapBounds(float mapWidth, float mapHeight);

    // Convert world coordinates to screen coordinates
    Vector2 WorldToScreen(const Vector2& worldPos) const;

    // Convert screen coordinates to world coordinates
    Vector2 ScreenToWorld(const Vector2& screenPos) const;

    // Check if a point is visible in the camera view
    bool IsVisible(const Vector2& worldPos, float radius = 0.0f) const;

    // Camera settings
    void SetFollowSpeed(float speed) { m_followSpeed = speed; }
    float GetFollowSpeed() const { return m_followSpeed; }

    // Snap camera to target immediately (no smooth follow)
    void SnapToTarget();

    // Manual camera control
    void MoveCamera(const Vector2& direction, float deltaTime);
    void SetCameraPosition(const Vector2& position); // Set camera position directly

    // Manual control mode
    void SetManualControl(bool manual) { m_manualControl = manual; }
    bool IsManualControl() const { return m_manualControl; }

private:
    Vector2 m_position;          // Camera position (top-left of viewport in world space)
    Vector2 m_target;            // Target position to follow (center of viewport)
    float m_viewportWidth;       // Viewport width (usually 1200)
    float m_viewportHeight;      // Viewport height (usually 800)
    float m_followSpeed;         // How fast camera follows target (pixels per second)

    // Map boundaries
    float m_mapWidth;
    float m_mapHeight;

    // Manual control
    bool m_manualControl;
    static constexpr float MANUAL_MOVE_SPEED = 800.0f; // Pixels per second for WASD

    // Clamp camera position to map bounds
    void ClampToMapBounds();
};
