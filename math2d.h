#pragma once

#include <cstddef>

namespace math
{

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y);

    float square_magnitude() const;
    float magnitude() const;
    Vec2 normalize() const;

    Vec2& operator+=(const Vec2& rhs);
    Vec2& operator-=(const Vec2& rhs);
    Vec2& operator*=(float rhs);
    Vec2& operator/=(float rhs);
};

struct Mat2
{
    float data[4] = {1.0f, 0.0f, 0.0f, 1.0f};

    Mat2() = default;
    Mat2(Vec2 left_column, Vec2 right_column);
    Mat2(float m00, float m01, float m10, float m11);

    Mat2& operator*=(float rhs);
    Mat2& operator*=(const Mat2& rhs);
    Mat2& operator+=(const Mat2& rhs);
    Mat2& operator-=(const Mat2& rhs);
};

float dot(const Vec2& lhs, const Vec2& rhs);

// Vector operations
Vec2 operator+(const Vec2& lhs, const Vec2& rhs);
Vec2 operator-(const Vec2& lhs, const Vec2& rhs);
Vec2 operator*(float lhs, const Vec2& rhs);
Vec2 operator*(const Vec2& lhs, float rhs);
Vec2 operator/(const Vec2& lhs, float rhs);
Vec2 operator-(const Vec2& operand);

// Matrix operations
Vec2 operator*(const Mat2& lhs, const Vec2& rhs);
Mat2 operator*(const Mat2& lhs, const Mat2& rhs);
Mat2 operator*(float lhs, const Mat2& rhs);
Mat2 operator*(const Mat2& lhs, float rhs);
Mat2 operator+(const Mat2& lhs, const Mat2& rhs);
Mat2 operator-(const Mat2& lhs, const Mat2& rhs);
Mat2 operator-(const Mat2& operand);

}
