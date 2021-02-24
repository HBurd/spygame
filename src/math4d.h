#pragma once

#include "math3d.h"

#include <cstddef>

namespace math {

struct Vec4
{
    float data[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    Vec4() = default;
    Vec4(float d0, float d1, float d2, float d3);

    // Takes an array of four floats
    Vec4(float* data_);

    // Sets the fourth coordinate to 1.0;
    Vec4(const Vec3& vec3);

    float& operator[](size_t index);
    const float& operator[](size_t index) const;

    float square_magnitude() const;
    float magnitude() const;
    Vec4 normalize() const;

    Vec4& operator+=(const Vec4& rhs);
    Vec4& operator-=(const Vec4& rhs);
    Vec4& operator*=(float rhs);
    Vec4& operator/=(float rhs);
};

struct Mat4
{
    float data[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f};

    Mat4() = default;
    Mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33);

    Mat4(const Mat3& mat3);

    static Mat4 FromColumns(const Vec4& c0, const Vec4& c1, const Vec4& c2, const Vec4& c3);
    static Mat4 FromRows(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3);
    static Mat4 Diagonal(const Vec4& diag);

    static Mat4 Perspective(float near, float far, float fov, float aspect_ratio);
    static Mat4 Orthographic(float near, float far, float width, float height);
    static Mat4 Translate(Vec3 translation);

    Mat4 transpose() const;

    Vec4 column(size_t index) const;
    Vec4 row(size_t index) const;

    Mat4& operator*=(float rhs);
    Mat4& operator*=(const Mat4& rhs);
    Mat4& operator+=(const Mat4& rhs);
    Mat4& operator-=(const Mat4& rhs);
};

// Vector operations
float dot(const Vec4& lhs, const Vec4& rhs);

Vec4 operator+(const Vec4& lhs, const Vec4& rhs);
Vec4 operator-(const Vec4& lhs, const Vec4& rhs);
Vec4 operator*(float lhs, const Vec4& rhs);
Vec4 operator*(const Vec4& lhs, float rhs);
Vec4 operator/(const Vec4& lhs, float rhs);
Vec4 operator-(const Vec4& operand);

// Matrix operations
Vec4 operator*(const Mat4& lhs, const Vec4& rhs);
Mat4 operator*(const Mat4& lhs, const Mat4& rhs);
Mat4 operator*(float lhs, const Mat4& rhs);
Mat4 operator*(const Mat4& lhs, float rhs);
Mat4 operator+(const Mat4& lhs, const Mat4& rhs);
Mat4 operator-(const Mat4& lhs, const Mat4& rhs);
Mat4 operator-(const Mat4& operand);

}
