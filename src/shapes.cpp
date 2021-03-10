#include "shapes.h"
#include <cmath>

using math::Vec2;
using math::Vec3;

Transform2d::Transform2d(Vec2 pos_, Vec2 scale_, float rotation_)
    : pos(pos_), scale(scale_), rotation(rotation_)
{}

Transform3d::Transform3d(Vec3 pos_, Vec3 scale_, float rotation_)
    : pos(pos_), scale(scale_), rotation(rotation_)
{}

Vec2 generic_support(Array<Vec2> points, Vec2 d)
{
    float max_proj = -INFINITY;
    Vec2 result;

    for (auto p : points)
    {
        float proj = dot(p, d);
        if (proj > max_proj)
        {
            max_proj = proj;
            result = p;
        }
    }

    return result;
}

bool rectangle_contains_point(Transform2d rect, Vec2 point)
{
    float cosine = cosf(rect.rotation);
    float sine   = sinf(rect.rotation);

    Vec2 basis1(cosine, sine);
    Vec2 basis2(-sine, cosine);

    float projection1 = dot(basis1, point - rect.pos);
    float projection2 = dot(basis2, point - rect.pos);

    return fabs(projection1) <= 0.5f * rect.scale.x && fabs(projection2) <= 0.5f * rect.scale.y;
}

// intersection (optional) returns the point of intersection
bool edge_intersect(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2* intersection)
{
    // When denom != 0, t (or u) = t_num (or u_num) / denom
    float t_num = (a1.x - b1.x) * (b1.y - b2.y) - (a1.y - b1.y) * (b1.x - b2.x);
    float u_num = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);
    float denom = (a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x);

    if ((denom >= 0.0f && t_num >= 0.0f && u_num >= 0.0f && t_num <= denom && u_num <= denom)
        || (denom < 0.0f && t_num <= 0.0f && u_num <= 0.0f && t_num >= denom && u_num >= denom))
    {
        // The line segments intersect
        if (intersection)
        {
            if (denom)
            {
                *intersection = a1 + (t_num / denom) * (a2 - a1);
            }
            else
            {
                // TODO: Does this case matter?
                assert(false);
            }
        }
        
        return true;
    }

    return false;
}

// Find where the line along dir passing through p intersects with a particular z plane
Vec2 z_intersect(Vec3 p, Vec3 dir, float z)
{
    if (!dir.z)
    {
        // Return zero if the direction is parallel to the z plane
        return Vec2();
    }

    float t = (z - p.z) / dir.z;
    return Vec2(p.x, p.y) + t * Vec2(dir.x, dir.y);
}

// Penetration_vector is optional, points out of r2
bool intersect(Transform2d r1, Transform2d r2, Vec2* penetration_vector)
{
    float cos1 = cosf(r1.rotation);
    float sin1 = sinf(r1.rotation);
    float cos2 = cosf(r2.rotation);
    float sin2 = sinf(r2.rotation);

    // Normals of rectangle sides
    Vec2 r1_normals[4] = {
        Vec2( cos1,  sin1), // right
        Vec2(-sin1,  cos1), // up
        Vec2(-cos1, -sin1), // left
        Vec2( sin1, -cos1), // down
    };

    Vec2 r2_normals[4] = {
        Vec2( cos2,  sin2), // right
        Vec2(-sin2,  cos2), // up
        Vec2(-cos2, -sin2), // left
        Vec2( sin2, -cos2), // down
    };

    // Side lengths of each rectangle (corresponding to normals)
    float r1_sides[4] = {
        0.5f * r1.scale.x, // right
        0.5f * r1.scale.y, // up
        0.5f * r1.scale.x, // left
        0.5f * r1.scale.y, // down
    };

    float r2_sides[4] = {
        0.5f * r2.scale.x, // right
        0.5f * r2.scale.y, // up
        0.5f * r2.scale.x, // left
        0.5f * r2.scale.y, // down
    };

    // Positions of rectangle corners, relative to their respective centre
    Vec2 r1_corners[4] = {
        r1_sides[0] * r1_normals[0] + r1_sides[1] * r1_normals[1],
        r1_sides[1] * r1_normals[1] + r1_sides[2] * r1_normals[2],
        r1_sides[2] * r1_normals[2] + r1_sides[3] * r1_normals[3],
        r1_sides[3] * r1_normals[3] + r1_sides[0] * r1_normals[0],
    };

    Vec2 r2_corners[4] = {
        r2_sides[0] * r2_normals[0] + r2_sides[1] * r2_normals[1],
        r2_sides[1] * r2_normals[1] + r2_sides[2] * r2_normals[2],
        r2_sides[2] * r2_normals[2] + r2_sides[3] * r2_normals[3],
        r2_sides[3] * r2_normals[3] + r2_sides[0] * r2_normals[0],
    };

    Vec2 min_penetration_normal;
    float min_penetration_distance = 0.0f;

    // Test if r2 lies entirely past one of the sides of r1
    for (uint i = 0; i < 4; ++i)
    {
        Vec2 n = r1_normals[i];
        float s = r1_sides[i];
        Vec2 c = generic_support(&r2_corners, -n);

        float penetration_distance = dot(n, c + r2.pos - r1.pos) - s;

        if (penetration_distance <= 0.0f)
        {
            if (min_penetration_distance == 0.0f || penetration_distance > min_penetration_distance)
            {
                min_penetration_distance = penetration_distance;
                min_penetration_normal = n;
            }
        }
        else
        {
            return false;
        }
    }

    // Test if r1 lies entirely past one of the sides of r2
    for (uint i = 0; i < 4; ++i)
    {
        Vec2 n = r2_normals[i];
        float s = r2_sides[i];
        Vec2 c = generic_support(&r1_corners, -n);

        float penetration_distance = dot(n, c + r1.pos - r2.pos) - s;

        if (penetration_distance <= 0.0f)
        {
            if (min_penetration_distance == 0.0f || penetration_distance > min_penetration_distance)
            {
                min_penetration_distance = penetration_distance;
                min_penetration_normal = -n;
            }
        }
        else
        {
            return false;
        }
    }

    // Now we know the objects are colliding

    if (penetration_vector)
    {
        *penetration_vector = min_penetration_distance * min_penetration_normal;
    }

    return true;
}

bool cw_from_vector(Vec2 v, Vec2 p)
{
    // Rotate v clockwise by 90 degrees
    Vec2 normal(v.y, -v.x);
    return dot(normal, p) > 0;
}

bool poly_intersect(Array<Vec2> p1, Array<Vec2> p2)
{
    // TODO: This algorithm is inefficient, and I actually have no proof it's correct
    // (but I think it is). GJK would be more efficient.

    // Check if every vertex of p2 lies to one side of p1
    for (uint i = 0; i < p1.size; ++i)
    {
        Vec2 edge = p1[(i + 1) % p1.size] - p1[i];
        
        // Rotate edge 90 degrees clockwise to get normal
        Vec2 normal(edge.y, -edge.x);

        bool no_intersect = true;
        for (auto v : p2)
        {
            if (!cw_from_vector(edge, v - p1[i]))
            {
                no_intersect = false;
                break;
            }
        }

        if (no_intersect)
        {
            return false;
        }
    }

    // Check if every vertex of p1 lies to one side of p2
    for (uint i = 0; i < p2.size; ++i)
    {
        Vec2 edge = p2[(i + 1) % p2.size] - p2[i];
        
        // Rotate edge 90 degrees clockwise to get normal
        Vec2 normal(edge.y, -edge.x);

        bool no_intersect = true;
        for (auto v : p1)
        {
            if (!cw_from_vector(edge, v - p2[i]))
            {
                no_intersect = false;
                break;
            }
        }

        if (no_intersect)
        {
            return false;
        }
    }

    // No polygon lies entirely past one edge of the other, so they intersect
    return true;
}

bool point_in_poly(Array<Vec2> poly, Vec2 point)
{
    for (uint i = 0; i < poly.size; ++i)
    {
        Vec2 edge = poly[(i + 1) % poly.size] - poly[i];
        if (cw_from_vector(edge, point - poly[i]))
        {
            return false;
        }
    }

    return true;
}
