#pragma once

struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }

    float Length() const;
    float LengthSquared() const;
    Vector2 Normalized() const;
    void Normalize();

    float Dot(const Vector2& other) const;
    float Cross(const Vector2& other) const;

    static Vector2 Zero() { return Vector2(0, 0); }
    static Vector2 One() { return Vector2(1, 1); }
    static Vector2 Up() { return Vector2(0, -1); }
    static Vector2 Down() { return Vector2(0, 1); }
    static Vector2 Left() { return Vector2(-1, 0); }
    static Vector2 Right() { return Vector2(1, 0); }
};
