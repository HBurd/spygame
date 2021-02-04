#pragma once

#include <cstddef>

namespace math {

struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_);

    float* array();
    const float* array() const;

    Vec3& operator+=(const Vec3& rhs);
    Vec3& operator-=(const Vec3& rhs);
    Vec3& operator*=(float rhs);
    Vec3& operator/=(float rhs);
};

// This ensures that the memory layout of a Vec2 is the same as an array of its components.
// This is a requirement of various parts of the code, e.g. imgui.
static_assert(sizeof(Vec3) == 3 * sizeof(float), "The Vec3 type cannot be padded.");

struct Mat3
{
    float data[9] = {1.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f,
                     0.0f, 0.0f, 1.0f};

    Mat3() = default;
    Mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);

    static Mat3 FromColumns(const Vec3& c0, const Vec3& c1, const Vec3& c2);
    static Mat3 FromRows(const Vec3& r0, const Vec3& r1, const Vec3& r2);

    static Mat3 RotateX(float angle);
    static Mat3 RotateY(float angle);
    static Mat3 RotateZ(float angle);

    Mat3& transpose();

    Vec3 column(size_t index) const;
    Vec3 row(size_t index) const;

    Mat3& operator*=(float rhs);
    Mat3& operator*=(const Mat3& rhs);
    Mat3& operator+=(const Mat3& rhs);
    Mat3& operator-=(const Mat3& rhs);
};

// Vector operations
float dot(const Vec3& lhs, const Vec3& rhs);

Vec3 operator+(const Vec3& lhs, const Vec3& rhs);
Vec3 operator-(const Vec3& lhs, const Vec3& rhs);
Vec3 operator*(float lhs, const Vec3& rhs);
Vec3 operator*(const Vec3& lhs, float rhs);
Vec3 operator/(const Vec3& lhs, float rhs);
Vec3 operator-(const Vec3& operand);

// Matrix operations
Vec3 operator*(const Mat3& lhs, const Vec3& rhs);
Mat3 operator*(const Mat3& lhs, const Mat3& rhs);
Mat3 operator*(float lhs, const Mat3& rhs);
Mat3 operator*(const Mat3& lhs, float rhs);
Mat3 operator+(const Mat3& lhs, const Mat3& rhs);
Mat3 operator-(const Mat3& lhs, const Mat3& rhs);
Mat3 operator-(const Mat3& operand);

}
