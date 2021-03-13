#pragma once

#include "math2d.h"
#include "util.h"

struct NavPoly
{
    u32 offset;
    u32 count;
    bool occupied = true;
};

extern Array<NavPoly> nav_polys;
extern Array<u32> nav_connections;
extern Array<math::Vec2> nav_vertices;

void build_nav_mesh(float left, float right, float bottom, float top);
