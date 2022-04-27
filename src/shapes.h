#pragma once

#include "hbmath.h"
#include "util.h"

struct Transform2d
{
    hbmath::Vec2 pos;
    hbmath::Vec2 scale = hbmath::Vec2(1.0f, 1.0f);
    float rotation = 0.0f;

    Transform2d() = default;
    Transform2d(hbmath::Vec2 pos_, hbmath::Vec2 scale_, float rotation_);
};

struct Transform3d
{
    hbmath::Vec3 pos;
    hbmath::Vec3 scale = hbmath::Vec3(1.0f, 1.0f, 1.0f);
    float rotation = 0.0f;

    Transform3d() = default;
    Transform3d(hbmath::Vec3 pos_, hbmath::Vec3 scale_, float rotation_);
};

hbmath::Vec2 generic_support(Array<hbmath::Vec2> points, hbmath::Vec2 d);

bool rectangle_contains_point(Transform2d rect, hbmath::Vec2 point);

bool point_in_poly(Array<hbmath::Vec2> poly, hbmath::Vec2 point);

bool poly_intersect(Array<hbmath::Vec2> p1, Array<hbmath::Vec2> p2);

bool edge_intersect(hbmath::Vec2 a1, hbmath::Vec2 a2, hbmath::Vec2 b1, hbmath::Vec2 b2, hbmath::Vec2* intersection);

// Returns whether p is cw from v
bool cw_from_vector(hbmath::Vec2 v, hbmath::Vec2 p);

// Find where the line along dir passing through p intersects with a particular z plane
hbmath::Vec2 z_intersect(hbmath::Vec3 p, hbmath::Vec3 dir, float z);

// Penetration_vector is optional, points out of r2
bool intersect(Transform2d r1, Transform2d r2, hbmath::Vec2* penetration_vector);
