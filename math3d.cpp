#include "math3d.h"

#include <cmath>
#include <cassert>

namespace math {

///////////////////////
// Vector operations //
///////////////////////

Vec3::Vec3(float x_, float y_, float z_)
    : x(x_), y(y_), z(z_)
{}

Vec3& Vec3::operator+=(const Vec3& rhs)
{
    return *this = *this + rhs;
}

Vec3& Vec3::operator-=(const Vec3& rhs)
{
    return *this = *this - rhs;
}

Vec3& Vec3::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Vec3& Vec3::operator/=(float rhs)
{
    return *this = *this / rhs;
}

float* Vec3::array()
{
    return &x;
}

const float* Vec3::array() const
{
    return &x;
}

float dot(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x * rhs.x
         + lhs.y * rhs.y
         + lhs.z * rhs.z;
}

Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x + rhs.x,
                lhs.y + rhs.y,
                lhs.z + rhs.z);
}

Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x - rhs.x,
                lhs.y - rhs.y,
                lhs.z - rhs.z);
}

Vec3 operator*(float lhs, const Vec3& rhs)
{
    return Vec3(lhs * rhs.x,
                lhs * rhs.y,
                lhs * rhs.z);
}

Vec3 operator*(const Vec3& lhs, float rhs)
{
    return rhs * lhs;
}

Vec3 operator/(const Vec3& lhs, float rhs)
{
    return (1.0f / rhs) * lhs;
}

Vec3 operator-(const Vec3& operand)
{
    return Vec3(-operand.x,
                -operand.y,
                -operand.z);
}

///////////////////////
// Matrix operations //
///////////////////////

Mat3::Mat3(float m00, float m01, float m02,
           float m10, float m11, float m12,
           float m20, float m21, float m22)
    : data{m00, m01, m02,
           m10, m11, m12,
           m20, m21, m22}
{}

Mat3 Mat3::FromColumns(const Vec3& c0, const Vec3& c1, const Vec3& c2)
{
    return Mat3(
        c0.x, c1.x, c2.x,
        c0.y, c1.y, c2.y,
        c0.z, c1.z, c2.z
    );
}

Mat3 Mat3::FromRows(const Vec3& r0, const Vec3& r1, const Vec3& r2)
{
    return Mat3(
        r0.x, r0.y, r0.z,
        r1.x, r1.y, r1.z,
        r2.x, r2.y, r2.z
    );
}

Mat3 Mat3::RotateX(float angle)
{
    float cosine = cosf(angle);
    float sine = sinf(angle);

    return Mat3(
        1.0f,  0.0f,   0.0f,
        0.0f,  cosine, -sine,
        0.0f, sine,   cosine
    );
}

Mat3 Mat3::RotateY(float angle)
{
    float cosine = cosf(angle);
    float sine = sinf(angle);

    return Mat3(
         cosine, 0.0f, sine,
         0.0f,   1.0f, 0.0f,
        -sine,   0.0f, cosine
    );
}

Mat3 Mat3::RotateZ(float angle)
{
    float cosine = cosf(angle);
    float sine = sinf(angle);

    return Mat3(
        cosine, -sine,   0.0f,
        sine,    cosine, 0.0f,
        0.0f,    0.0f,   1.0f
    );
}

Vec3 Mat3::column(size_t index) const
{
    assert(index < 3);
    return Vec3(data[index], data[index + 3], data[index + 6]);
}

Vec3 Mat3::row(size_t index) const
{
    assert(index < 3);
    return Vec3(data[3 * index], data[3 * index + 1], data[3 * index + 2]);
}

Mat3& Mat3::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Mat3& Mat3::operator*=(const Mat3& rhs)
{
    return *this = *this * rhs;
}

Mat3& Mat3::operator+=(const Mat3& rhs)
{
    return *this = *this + rhs;
}

Mat3& Mat3::operator-=(const Mat3& rhs)
{
    return *this = *this - rhs;
}

Vec3 operator*(const Mat3& lhs, const Vec3& rhs)
{
    return Vec3(
        dot(lhs.row(0), rhs),
        dot(lhs.row(1), rhs),
        dot(lhs.row(2), rhs));
}

Mat3 operator*(const Mat3& lhs, const Mat3& rhs)
{
    return Mat3::FromColumns(
        lhs * rhs.column(0),
        lhs * rhs.column(1),
        lhs * rhs.column(2));
}

Mat3 operator*(float lhs, const Mat3& rhs)
{
    Mat3 result;

    for (int i = 0; i < 9; ++i)
    {
        result.data[i] = lhs * rhs.data[i];
    }

    return result;
}

Mat3 operator*(const Mat3& lhs, float rhs)
{
    return rhs * lhs;
}

Mat3 operator+(const Mat3& lhs, const Mat3& rhs)
{
    Mat3 result;

    for (int i = 0; i < 9; ++i)
    {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }

    return result;
}

Mat3 operator-(const Mat3& lhs, const Mat3& rhs)
{
    Mat3 result;

    for (int i = 0; i < 9; ++i)
    {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }

    return result;
}

Mat3 operator-(const Mat3& operand)
{
    Mat3 result;

    for (int i = 0; i < 9; ++i)
    {
        result.data[i] = operand.data[i];
    }

    return result;
}

}
