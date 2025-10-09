#include "Vector2.h"
#include <cmath>

float Vector2::Length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2::LengthSquared() const {
    return x * x + y * y;
}

Vector2 Vector2::Normalized() const {
    float len = Length();
    if (len == 0) return Vector2::Zero();
    return Vector2(x / len, y / len);
}

void Vector2::Normalize() {
    float len = Length();
    if (len != 0) {
        x /= len;
        y /= len;
    }
}

float Vector2::Dot(const Vector2& other) const {
    return x * other.x + y * other.y;
}

float Vector2::Cross(const Vector2& other) const {
    return x * other.y - y * other.x;
}
