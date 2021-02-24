#include "math2d.h"
#include <cmath>

namespace math
{

///////////////////////
// Vector operations //
///////////////////////

Vec2::Vec2(float x_, float y_)
    : x(x_), y(y_)
{}

Vec2::Vec2(float* data)
    : x(data[0]), y(data[1])
{}

float Vec2::square_magnitude() const
{
    return x * x + y * y;
}

// A static_assert in the header file ensures that the components of a Vec2
// can be accessed as an array
float* Vec2::array()
{
    return &x;
}

const float* Vec2::array() const
{
    return &x;
}

float Vec2::magnitude() const
{
    return sqrtf(square_magnitude());
}

Vec2 Vec2::normalize() const
{
    return (*this) / magnitude();
}

Vec2& Vec2::operator+=(const Vec2& rhs)
{
    return *this = *this + rhs;
}

Vec2& Vec2::operator-=(const Vec2& rhs)
{
    return *this = *this - rhs;
}

Vec2& Vec2::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Vec2& Vec2::operator/=(float rhs)
{
    return *this = *this / rhs;
}

float dot(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

Vec2 operator*(float lhs, const Vec2& rhs)
{
    return Vec2(lhs * rhs.x, lhs * rhs.y);
}

Vec2 operator*(const Vec2& lhs, float rhs)
{
    return rhs * lhs;
}

Vec2 operator/(const Vec2& lhs, float rhs)
{
    return (1.0f / rhs) * lhs;
}

Vec2 operator-(const Vec2& operand)
{
    return Vec2(-operand.x, -operand.y);
}

///////////////////////
// Matrix operations //
///////////////////////

Mat2::Mat2(Vec2 left_column, Vec2 right_column)
    : data{left_column.x, right_column.x, left_column.y, right_column.y}
{}

Mat2::Mat2(float m00, float m01, float m10, float m11)
    : data{m00, m01, m10, m11}
{}

Mat2 Mat2::Diagonal(const Vec2& diag)
{
    return Mat2(diag.x, 0.0f, 0.0f, diag.y);
}

Mat2 Mat2::Rotation(float angle)
{
    float cosine = cosf(angle);
    float sine = sinf(angle);
    return Mat2(cosine, -sine, sine, cosine);
}

Mat2 Mat2::transpose() const
{
    Mat2 result;

    // Set diagonal
    for (int i = 0; i < 2; ++i)
    {
        result.data[3 * i] = data[3 * i];
    }

    // For each row
    for (int i = 0; i < 2; ++i)
    {
        // For each element of row, starting 1 past diagonal
        for (int j = i + 1; j < 2; ++j)
        {
            result.data[2*i + j] = data[2*j + i];
            result.data[2*j + i] = data[2*i + j];
        }
    }

    return result;
}

Mat2& Mat2::operator*=(float rhs)
{
    return *this = *this * rhs;
}

Mat2& Mat2::operator*=(const Mat2& rhs)
{
    return *this = *this * rhs;
}

Mat2& Mat2::operator+=(const Mat2& rhs)
{
    return *this = *this + rhs;
}

Mat2& Mat2::operator-=(const Mat2& rhs)
{
    return *this = *this - rhs;
}

Vec2 operator*(const Mat2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.data[0] * rhs.x + lhs.data[1] * rhs.y, lhs.data[2] * rhs.x + lhs.data[3] * rhs.y);
}

Mat2 operator*(const Mat2& lhs, const Mat2& rhs)
{
    Vec2 left_column  = lhs * Vec2(rhs.data[0], rhs.data[2]);
    Vec2 right_column = lhs * Vec2(rhs.data[1], rhs.data[3]);

    return Mat2(left_column, right_column);
}

Mat2 operator*(float lhs, const Mat2& rhs)
{
    return Mat2(lhs * rhs.data[0], lhs * rhs.data[1],
                lhs * rhs.data[2], lhs * rhs.data[3]);
}

Mat2 operator*(const Mat2& lhs, float rhs)
{
    return rhs * lhs;
}

Mat2 operator+(const Mat2& lhs, const Mat2& rhs)
{
    return Mat2(lhs.data[0] + rhs.data[0], lhs.data[1] + rhs.data[1],
                lhs.data[0] + rhs.data[0], lhs.data[1] + rhs.data[1]);
}

Mat2 operator-(const Mat2& lhs, const Mat2& rhs)
{
    return lhs + (-rhs);
}

Mat2 operator-(const Mat2& operand)
{
    return Mat2(-operand.data[0], -operand.data[1],
                -operand.data[2], -operand.data[3]);
}

}
