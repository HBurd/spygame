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

// Takes an array of three floats
Vec3::Vec3(float* data)
    : x(data[0]), y(data[1]), z(data[2])
{}

float Vec3::square_magnitude() const
{
    return x*x + y*y + z*z;
}

float Vec3::magnitude() const
{
    return sqrtf(square_magnitude());
}

Vec3 Vec3::normalize() const
{
    return *this / magnitude();
}

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

Mat3 Mat3::Diagonal(const Vec3& diag)
{
    return Mat3(
        diag.x, 0.0f,   0.0f,
        0.0f,   diag.y, 0.0f,
        0.0f,   0.0f,   diag.z);
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

Mat3 Mat3::transpose() const
{
    Mat3 result;

    // Set diagonal
    for (int i = 0; i < 3; ++i)
    {
        result.data[4 * i] = data[4 * i];
    }

    // For each row
    for (int i = 0; i < 3; ++i)
    {
        // For each element of row, starting 1 past diagonal
        for (int j = i + 1; j < 3; ++j)
        {
            result.data[3*i + j] = data[3*j + i];
            result.data[3*j + i] = data[3*i + j];
        }
    }

    return result;
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

Quaternion::Quaternion(float w_, float x_, float y_, float z_)
    : w(w_), x(x_), y(y_), z(z_)
{}

Quaternion Quaternion::normalize() const
{
    Quaternion result;

    float magnitude = sqrtf(w*w + x*x + y*y + z*z);
    if (magnitude != 0.0f)
    {
        result.w = w / magnitude;
        result.x = x / magnitude;
        result.y = y / magnitude;
        result.z = z / magnitude;
    }
    else
    {
        result.w = 1.0f;
        result.x = 0.0f;
        result.y = 0.0f;
        result.z = 0.0f;
    }

    return result;
}

Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    Quaternion result;
    result.w = lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z;
    result.x = lhs.w*rhs.x + lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y;
    result.y = lhs.w*rhs.y - lhs.x*rhs.z + lhs.y*rhs.w + lhs.z*rhs.x;
    result.z = lhs.w*rhs.z + lhs.x*rhs.y - lhs.y*rhs.x + lhs.z*rhs.w;

    return result;
}

Quaternion Quaternion::inverse() const
{
    Quaternion result;
    result.w = w;
    result.x = -x;
    result.y = -y;
    result.z = -z;

    return result;
}

void Quaternion::to_matrix(float* matrix)
{
    // assuming magnitude 1
    matrix[0] = 1 - 2 * (y*y + z*z);
    matrix[1] = 2 * (x*y - z*w);
    matrix[2] = 2 * (x*z + y*w);
    matrix[3] = 2 * (x*y + z*w);
    matrix[4] = 1 - 2 * (x*x + z*z);
    matrix[5] = 2 * (y*z - x*w);
    matrix[6] = 2 * (x*z - y*w);
    matrix[7] = 2 * (y*z + x*w);
    matrix[8] = 1 - 2 * (x*x + y*y);
}

Vec3 Quaternion::apply_rotation(const Vec3& x)
{
    Quaternion x_quat;
    x_quat.w = 0.0f;
    x_quat.x = x.x;
    x_quat.y = x.y;
    x_quat.z = x.z;

    Quaternion result = (*this) * x_quat * (*this).inverse();
    return Vec3(result.x, result.y, result.z);
}

Quaternion Quaternion::RotateX(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.x = sine;

    return result;
}

Quaternion Quaternion::RotateY(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.y = sine;

    return result;
}

Quaternion Quaternion::RotateZ(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.z = sine;

    return result;
}

}
