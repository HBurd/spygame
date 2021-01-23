#pragma once

#include "math2d.h"

struct Rectangle
{
    math::Vec2 pos;
    math::Vec2 scale = math::Vec2(1.0f, 1.0f);
    float rotation = 0.0f;

    Rectangle() = default;
    Rectangle(math::Vec2 pos_, math::Vec2 scale_, float rotation_);
};
