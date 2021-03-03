#pragma once

#include "math3d.h"

struct NavNode
{
    u32 vertex_count;
    u32 vertices[MAX_NAV_POLY_VERTICES];
};

extern Array<Vec3> nav_vertices;
extern Array<NavNode> nav_nodes;

void add_nav_vertex(NavNode* node, Vec3 vertex);
