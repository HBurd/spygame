#pragma once

#include "math2d.h"
#include "math3d.h"

struct Transform2d
{
    math::Vec2 pos;
    math::Vec2 scale = math::Vec2(1.0f, 1.0f);
    float rotation = 0.0f;

    Transform2d() = default;
    Transform2d(math::Vec2 pos_, math::Vec2 scale_, float rotation_);
};

struct Transform3d
{
    math::Vec3 pos;
    math::Vec3 scale = math::Vec3(1.0f, 1.0f, 1.0f);
    float rotation = 0.0f;

    Transform3d() = default;
    Transform3d(math::Vec3 pos_, math::Vec3 scale_, float rotation_);
};
