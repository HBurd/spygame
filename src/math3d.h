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

    // Takes an array of three floats
    Vec3(float* data);

    float* array();
    const float* array() const;

    float square_magnitude() const;
    float magnitude() const;
    Vec3 normalize() const;

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
    static Mat3 Diagonal(const Vec3& diag);

    static Mat3 RotateX(float angle);
    static Mat3 RotateY(float angle);
    static Mat3 RotateZ(float angle);

    Mat3 transpose() const;

    Vec3 column(size_t index) const;
    Vec3 row(size_t index) const;

    Mat3& operator*=(float rhs);
    Mat3& operator*=(const Mat3& rhs);
    Mat3& operator+=(const Mat3& rhs);
    Mat3& operator-=(const Mat3& rhs);
};

struct Quaternion
{
    float w = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Quaternion() = default;
    Quaternion(float w_, float x_, float y_, float z_);

    Quaternion normalize() const;

    Quaternion inverse() const;

    // TODO: use the Mat4 type
    void to_matrix(float* matrix);

    Vec3 apply_rotation(const Vec3& x);

    static Quaternion RotateX(float angle);
    static Quaternion RotateY(float angle);
    static Quaternion RotateZ(float angle);
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

// Quaternion operations
Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);

}
