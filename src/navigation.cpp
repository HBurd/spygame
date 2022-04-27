#include "navigation.h"
#include "entity.h"
#include "shapes.h"

#include <cassert>

using hbmath::Vec2;
using hbmath::Mat2;

MAKE_ARRAY(nav_polys, NavPoly, 1024);
MAKE_ARRAY(nav_connections, u32, 1024);
MAKE_ARRAY(nav_vertices, Vec2, 1024);

// Note to my future self: I am sorry. There are a lot of different cases here.
// I recommend you draw each of them out to understand what is happening.
static void subdivide_nav_poly(Array<Vec2> poly, Array<Vec2> wall)
{
    // The vertices of wall outside of poly have no effect. The edge intersections
    // between wall and poly become vertices of the subdivided polygons, as do the
    // vertices of wall inside poly.

    for (uint wall_idx = 0; wall_idx < wall.size; ++wall_idx)
    {
        Vec2 w1 = wall[wall_idx];
        Vec2 w2 = wall[(wall_idx + 1) % wall.size];

        NavPoly new_poly;
        new_poly.offset = nav_vertices.size;
        new_poly.count = 0;

        // After the first point of the new polygon is added (i.e. the last point of wall),
        // poly_idx should be the index of the next point of poly to add to the new polygon.
        uint poly_idx = 0;

        if (point_in_poly(poly, w1))
        {
            ++new_poly.count;

            nav_vertices.push(w1);

            // Find the first vertex of poly to add to the new polygon.
            // This vertex has to be the same as the final poly vertex of the
            // previous wall edge.
            
            // The previous wall edge is w3->w1
            Vec2 w3 = wall[(wall_idx + wall.size - 1) % wall.size];

            // Increment poly_idx until poly[(poly_idx + 1) % poly.size] is
            // on the outside of w3->w1.
            while (!cw_from_vector(w1 - w3, poly[(poly_idx + 1) % poly.size] - w3))
            {
                ++poly_idx;
            }

            // Increment poly_idx until poly[(poly_idx + 1) % poly.size] is
            // on the inside of w3->w1.
            while (cw_from_vector(w1 - w3, poly[(poly_idx + 1) % poly.size] - w3))
            {
                ++poly_idx;
            }

            // Now poly[poly_idx % poly_size] is the final poly vertex on the outside
            // of w3->w1. If a poly was added for the final vertex of the last wall,
            // the poly_idx still needs to be advanced until the end of this poly.
            while (!cw_from_vector(w2 - w1, poly[poly_idx % poly.size] - w1))
            {
                // TODO: Potential optimization: I think it can be shown that this
                // loop will execute at most once.
                ++poly_idx;
            }
        }
        else
        {
            bool found_intersection = false;

            // Check if the edge w1->w2 intersects poly
            for (poly_idx = 0; poly_idx < poly.size; ++poly_idx)
            {
                Vec2 p1 = poly[poly_idx];
                Vec2 p2 = poly[(poly_idx + 1) % poly.size];

                Vec2 intersection;

                // If w1 is outside poly, then p2 must be on the outside of
                // w1->w2 for the intersection to be the first point of the new polygon.
                // This is important in the case where w1->w2 intersects poly twice, as
                // this check only allows the correct intersection for the first point of
                // the new polyogn.
                // TODO: Prove/verify this!
                
                if (cw_from_vector(w2 - w1, p2 - w1) && edge_intersect(w1, w2, p1, p2, &intersection))
                {
                    found_intersection = true;

                    ++new_poly.count;
                    nav_vertices.push(intersection);

                    // The next vertex of new_poly will be p2, aka poly[(poly_idx + 1) % poly.size]
                    ++poly_idx;
                    break;
                }
            }

            if (!found_intersection)
            {
                continue;
            }
        }

        // Add all the subsequent points of poly that are on the outside of w1->w2
        Vec2 p1, p2;
        while (cw_from_vector(w2 - w1, poly[poly_idx % poly.size] - w1))
        {
            p1 = poly[poly_idx % poly.size];
            p2 = poly[(poly_idx + 1) % poly.size];

            nav_vertices.push(p1);
            ++new_poly.count;
            ++poly_idx;
        }

        // Check if the new polygon ends with an intersection between wall and poly
        Vec2 intersection;
        if (edge_intersect(w1, w2, p1, p2, &intersection))
        {
            nav_vertices.push(intersection);
            ++new_poly.count;
            nav_polys.push(new_poly);
        }
        else
        {
            nav_vertices.push(w2);
            ++new_poly.count;
            nav_polys.push(new_poly);

            // Check if there must be a polygon with vertex w2.
            Vec2 w3 = wall[(wall_idx + 2) % wall.size];
            if (!cw_from_vector(w3 - w2, p1 - w2))
            {
                // TODO: I think it should always be possible to add a new
                // vertex and avoid creating a new polygon, which will
                // result in a better nav mesh.

                new_poly.offset = nav_vertices.size;
                new_poly.count = 0;

                nav_vertices.push(w2);
                ++new_poly.count;

                nav_vertices.push(p1);
                ++new_poly.count;

                while (!cw_from_vector(w3 - w2, poly[poly_idx % poly.size] - w2))
                {
                    // TODO: Same potential optimization as above comment
                    nav_vertices.push(poly[poly_idx % poly.size]);
                    ++new_poly.count;
                    ++poly_idx;
                }

                nav_vertices.push(poly[poly_idx % poly.size]);
                ++new_poly.count;

                nav_polys.push(new_poly);
            }
        }
    }
}

// Updates the nav mesh for a convex polygon obstacle defined by vertices
// TODO: This is horribly inefficient
static void add_obstacle(Array<Vec2> wall)
{
    // Find which nav polys the obstacle intersects
    // Save the count here because we'll be adding more nav polys with each iteration.
    uint nav_poly_count = nav_polys.size;
    for (uint i = 0; i < nav_poly_count; ++i)
    {
        NavPoly& p = nav_polys[i];
        if (p.occupied)
        {
            Array<Vec2> poly(&nav_vertices[p.offset], p.count);

            if (poly_intersect(poly, wall))
            {
                p.occupied = false;

                subdivide_nav_poly(poly, wall);
            }
        }
    }

    // TODO: Now connect all the polygons which were just created
}

void build_nav_mesh(float left, float right, float bottom, float top)
{
    nav_vertices.clear();
    nav_polys.clear();

    nav_vertices.push(Vec2(left,  top));
    nav_vertices.push(Vec2(left,  bottom));
    nav_vertices.push(Vec2(right, bottom));
    nav_vertices.push(Vec2(right, top));

    nav_connections.push(INVALID_INDEX);
    nav_connections.push(INVALID_INDEX);
    nav_connections.push(INVALID_INDEX);
    nav_connections.push(INVALID_INDEX);

    NavPoly poly;
    poly.offset = 0;
    poly.count = 4;

    nav_polys.push(poly);

    for (const auto& entity : entities)
    {
        Mat2 rotation = Mat2::Rotation(entity.transform.rotation);

        Vec2 vertices[4];
        vertices[0] = 0.5f * rotation * entity.transform.scale + entity.transform.pos;
        vertices[1] = 0.5f * rotation * Vec2(-entity.transform.scale.x, entity.transform.scale.y) + entity.transform.pos;
        vertices[2] = 0.5f * rotation * Vec2(-entity.transform.scale.x, -entity.transform.scale.y) + entity.transform.pos;
        vertices[3] = 0.5f * rotation * Vec2(entity.transform.scale.x, -entity.transform.scale.y) + entity.transform.pos;

        add_obstacle(&vertices);
    }
}
