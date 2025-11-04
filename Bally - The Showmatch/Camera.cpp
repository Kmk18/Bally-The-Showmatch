#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(float viewportWidth, float viewportHeight)
    : m_position(0, 0)
    , m_target(viewportWidth / 2.0f, viewportHeight / 2.0f)
    , m_viewportWidth(viewportWidth)
    , m_viewportHeight(viewportHeight)
    , m_followSpeed(2000.0f)  // 2000 pixels per second (increased for smoother camera movement)
    , m_mapWidth(viewportWidth)
    , m_mapHeight(viewportHeight)
    , m_manualControl(false)
{
}

void Camera::Update(float deltaTime) {
    // Skip automatic following if in manual control mode
    if (m_manualControl) {
        ClampToMapBounds();
        return;
    }

    // Calculate desired camera position (target centered in viewport)
    Vector2 desiredPosition(
        m_target.x - m_viewportWidth / 2.0f,
        m_target.y - m_viewportHeight / 2.0f
    );

    // Smooth follow using lerp
    Vector2 direction = desiredPosition - m_position;
    float distance = direction.Length();

    if (distance > 1.0f) {
        // Smooth interpolation
        float moveDistance = std::min(distance, m_followSpeed * deltaTime);
        m_position = m_position + direction.Normalized() * moveDistance;
    } else {
        m_position = desiredPosition;
    }

    // Clamp to map boundaries
    ClampToMapBounds();
}

void Camera::SetTarget(const Vector2& target) {
    m_target = target;
}

void Camera::SetMapBounds(float mapWidth, float mapHeight) {
    m_mapWidth = mapWidth;
    m_mapHeight = mapHeight;
    ClampToMapBounds();
}

Vector2 Camera::WorldToScreen(const Vector2& worldPos) const {
    return Vector2(worldPos.x - m_position.x, worldPos.y - m_position.y);
}

Vector2 Camera::ScreenToWorld(const Vector2& screenPos) const {
    return Vector2(screenPos.x + m_position.x, screenPos.y + m_position.y);
}

bool Camera::IsVisible(const Vector2& worldPos, float radius) const {
    // Check if object is within camera viewport (with some margin for radius)
    return worldPos.x + radius >= m_position.x &&
           worldPos.x - radius <= m_position.x + m_viewportWidth &&
           worldPos.y + radius >= m_position.y &&
           worldPos.y - radius <= m_position.y + m_viewportHeight;
}

void Camera::SnapToTarget() {
    m_position.x = m_target.x - m_viewportWidth / 2.0f;
    m_position.y = m_target.y - m_viewportHeight / 2.0f;
    ClampToMapBounds();
}

void Camera::MoveCamera(const Vector2& direction, float deltaTime) {
    m_position = m_position + direction * MANUAL_MOVE_SPEED * deltaTime;
    ClampToMapBounds();
}

void Camera::SetCameraPosition(const Vector2& position) {
    m_position = position;
    ClampToMapBounds();
}

void Camera::ClampToMapBounds() {
    // If map is smaller than viewport, center it
    if (m_mapWidth <= m_viewportWidth) {
        m_position.x = (m_mapWidth - m_viewportWidth) / 2.0f;
    } else {
        // Clamp so we don't show area outside the map
        m_position.x = std::max(0.0f, std::min(m_position.x, m_mapWidth - m_viewportWidth));
    }

    if (m_mapHeight <= m_viewportHeight) {
        m_position.y = (m_mapHeight - m_viewportHeight) / 2.0f;
    } else {
        m_position.y = std::max(0.0f, std::min(m_position.y, m_mapHeight - m_viewportHeight));
    }
}
