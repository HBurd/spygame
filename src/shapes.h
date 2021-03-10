#pragma once

#include "math2d.h"
#include "math3d.h"
#include "util.h"

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

math::Vec2 generic_support(Array<math::Vec2> points, math::Vec2 d);

bool rectangle_contains_point(Transform2d rect, math::Vec2 point);

bool point_in_poly(Array<math::Vec2> poly, math::Vec2 point);

bool poly_intersect(Array<math::Vec2> p1, Array<math::Vec2> p2);

bool edge_intersect(math::Vec2 a1, math::Vec2 a2, math::Vec2 b1, math::Vec2 b2, math::Vec2* intersection);

bool cw_from_vector(math::Vec2 v, math::Vec2 p);

// Find where the line along dir passing through p intersects with a particular z plane
math::Vec2 z_intersect(math::Vec3 p, math::Vec3 dir, float z);

// Penetration_vector is optional, points out of r2
bool intersect(Transform2d r1, Transform2d r2, math::Vec2* penetration_vector);
