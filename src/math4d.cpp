#include "math4d.h"
#include <cassert>
#include <cmath>

namespace math {

///////////////////////
// Vector operations //
///////////////////////

Vec4::Vec4(float d0, float d1, float d2, float d3)
    : data{d0, d1, d2, d3}
{}

// Sets the fourth coordinate to 1.0
Vec4::Vec4(const Vec3& vec3)
    : data{vec3.x, vec3.y, vec3.z, 1.0f}
{}

float& Vec4::operator[](size_t index)
{
    return data[index];
}

const float& Vec4::operator[](size_t index) const
{
    return data[index];
}

float Vec4::square_magnitude() const
{
    return data[0]*data[0] + data[1]*data[1] + data[2]*data[2] + data[3]*data[3];
}

float Vec4::magnitude() const
{
    return sqrtf(square_magnitude());
}

Vec4 Vec4::normalize() const
{
    return *this / magnitude();
}

Vec4& Vec4::operator+=(const Vec4& rhs)
{
    return *this = *this + rhs;
}

Vec4& Vec4::operator-=(const Vec4& rhs)
{
    return *this = *this - rhs;
}

Vec4& Vec4::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Vec4& Vec4::operator/=(float rhs)
{
    return *this = *this / rhs;
}

float dot(const Vec4& lhs, const Vec4& rhs)
{
    return lhs[0] * rhs[0]
         + lhs[1] * rhs[1]
         + lhs[2] * rhs[2]
         + lhs[3] * rhs[3];
}

Vec4 operator+(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs[0] + rhs[0],
                lhs[1] + rhs[1],
                lhs[2] + rhs[2],
                lhs[3] + rhs[3]);
}

Vec4 operator-(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs[0] - rhs[0],
                lhs[1] - rhs[1],
                lhs[2] - rhs[2],
                lhs[3] - rhs[3]);
}

Vec4 operator*(float lhs, const Vec4& rhs)
{
    return Vec4(lhs * rhs[0],
                lhs * rhs[1],
                lhs * rhs[2],
                lhs * rhs[3]);
}

Vec4 operator*(const Vec4& lhs, float rhs)
{
    return rhs * lhs;
}

Vec4 operator/(const Vec4& lhs, float rhs)
{
    return (1.0f / rhs) * lhs;
}

Vec4 operator-(const Vec4& operand)
{
    return Vec4(-operand[0],
                -operand[1],
                -operand[2],
                -operand[3]);
}

///////////////////////
// Matrix operations //
///////////////////////

Mat4::Mat4(float m00, float m01, float m02, float m03,
     float m10, float m11, float m12, float m13,
     float m20, float m21, float m22, float m23,
     float m30, float m31, float m32, float m33)
    : data{m00, m01, m02, m03,
           m10, m11, m12, m13,
           m20, m21, m22, m23,
           m30, m31, m32, m33}
{}

Mat4::Mat4(const Mat3& mat3)
    : data{mat3.data[0], mat3.data[1], mat3.data[2], 0.0f,
           mat3.data[3], mat3.data[4], mat3.data[5], 0.0f,
           mat3.data[6], mat3.data[7], mat3.data[8], 0.0f,
           0.0f,         0.0f,         0.0f,         1.0f}
{}

Mat4 Mat4::FromColumns(const Vec4& c0, const Vec4& c1, const Vec4& c2, const Vec4& c3)
{
    return Mat4(
        c0[0], c1[0], c2[0], c3[0],
        c0[1], c1[1], c2[1], c3[1],
        c0[2], c1[2], c2[2], c3[2],
        c0[3], c1[3], c2[3], c3[3]);
}

Mat4 Mat4::FromRows(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3)
{
    return Mat4(
        r0[0], r0[1], r0[2], r0[3],
        r1[0], r1[1], r1[2], r1[3],
        r2[0], r2[1], r2[2], r2[3],
        r3[0], r3[1], r3[2], r3[3]);
}
Mat4 Mat4::Diagonal(const Vec4& diag)
{
    return Mat4(
        diag[0], 0.0f,    0.0f,    0.0f,
        0.0f,    diag[1], 0.0f,    0.0f,
        0.0f,    0.0f,    diag[2], 0.0f,
        0.0f,    0.0f,    0.0f,    diag[3]);
}

Mat4 Mat4::Perspective(float near, float far, float fov, float aspect_ratio)
{
    float cot_fov = 1.0f / tanf(fov * 0.5f);

    return Mat4(
        cot_fov, 0.0f, 0.0f, 0.0f,
        0.0f, aspect_ratio * cot_fov, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -2 * near * far / (far - near),
        0.0f, 0.0f, -1.0f, 0.0f);
}

Mat4 Mat4::Orthographic(float near, float far, float width, float height)
{
    return Mat4(
        2.0f / width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / height, 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far - near), -(far + near) / (far - near),
        0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4 Mat4::Translate(Vec3 translation)
{
    return Mat4(
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Vec4 Mat4::column(size_t index) const
{
    assert(index < 4);
    return Vec4(data[index], data[index + 4], data[index + 8], data[index + 12]);
}

Vec4 Mat4::row(size_t index) const
{
    assert(index < 4);
    return Vec4(data[4 * index], data[4 * index + 1], data[4 * index + 2], data[4 * index + 3]);
}

Mat4 Mat4::transpose() const
{
    Mat4 result;

    // Set diagonal
    for (int i = 0; i < 4; ++i)
    {
        result.data[5 * i] = data[5 * i];
    }

    // For each row
    for (int i = 0; i < 4; ++i)
    {
        // For each element of row, starting 1 past diagonal
        for (int j = i + 1; j < 4; ++j)
        {
            result.data[4*i + j] = data[4*j + i];
            result.data[4*j + i] = data[4*i + j];
        }
    }

    return result;
}

Mat4& Mat4::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Mat4& Mat4::operator*=(const Mat4& rhs)
{
    return *this = *this * rhs;
}

Mat4& Mat4::operator+=(const Mat4& rhs)
{
    return *this = *this + rhs;
}

Mat4& Mat4::operator-=(const Mat4& rhs)
{
    return *this = *this - rhs;
}

Vec4 operator*(const Mat4& lhs, const Vec4& rhs)
{
    return Vec4(
        dot(lhs.row(0), rhs),
        dot(lhs.row(1), rhs),
        dot(lhs.row(2), rhs),
        dot(lhs.row(3), rhs));
}

Mat4 operator*(const Mat4& lhs, const Mat4& rhs)
{
    return Mat4::FromColumns(
        lhs * rhs.column(0),
        lhs * rhs.column(1), 
        lhs * rhs.column(2),
        lhs * rhs.column(3));
}

Mat4 operator*(float lhs, const Mat4& rhs)
{
    Mat4 result;

    for (int i = 0; i < 16; ++i)
    {
        result.data[i] = lhs * rhs.data[i];
    }

    return result;
}

Mat4 operator*(const Mat4& lhs, float rhs)
{
    return rhs * lhs;
}

Mat4 operator+(const Mat4& lhs, const Mat4& rhs)
{
    Mat4 result;

    for (int i = 0; i < 16; ++i)
    {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }

    return result;
}

Mat4 operator-(const Mat4& lhs, const Mat4& rhs)
{
    Mat4 result;

    for (int i = 0; i < 16; ++i)
    {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }

    return result;
}

Mat4 operator-(const Mat4& operand)
{
    Mat4 result;

    for (int i = 0; i < 16; ++i)
    {
        result.data[i] = -operand.data[i];
    }

    return result;
}

}
