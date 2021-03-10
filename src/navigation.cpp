#include "navigation.h"
#include "entity.h"
#include "shapes.h"

#include <cassert>

using math::Vec2;
using math::Mat2;

MAKE_ARRAY(nav_polys, NavPoly, 1024);
MAKE_ARRAY(nav_vertices, Vec2, 1024);

// Note to my future self: I am sorry. There are a lot of different cases here.
// I recommend you draw each of them out to understand what is happening.
void subdivide_nav_poly(Array<Vec2> poly, Array<Vec2> wall)
{
    // The vertices of wall outside of poly have no effect. The edge intersections
    // between wall and poly become vertices of the subdivided polygons, as do the
    // vertices of wall inside poly.

    for (uint wall_idx = 0; wall_idx < wall.size; ++wall_idx)
    {
        Vec2 w1 = wall[wall_idx];
        Vec2 w2 = wall[(wall_idx + 1) % wall.size];

        if (point_in_poly(poly, w1))
        {
            NavPoly new_poly;
            new_poly.offset = nav_vertices.size;
            new_poly.count = 1;

            nav_vertices.push(w1);

            // Find the first vertex of poly to add to the new polygon.
            // This vertex has to be the same as the final poly vertex of the
            // previous wall edge.
            
            // The previous wall edge is w3->w1
            Vec2 w3 = wall[(wall_idx + wall.size - 1) % wall.size];

            uint poly_idx = 0;

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
                ++poly_idx;
            }

            Vec2 p1, p2;
            while (cw_from_vector(w2 - w1, poly[poly_idx % poly.size] - w1))
            {
                p1 = poly[poly_idx % poly.size];
                p2 = poly[(poly_idx + 1) % poly.size];

                nav_vertices.push(p1);
                ++new_poly.count;
                ++poly_idx;
            }

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

                // Now check if there must be a polygon with vertex w2.
                Vec2 w3 = wall[(wall_idx + 2) % wall.size];
                if (!cw_from_vector(w3 - w2, p1 - w2))
                {
                    new_poly.offset = nav_vertices.size;
                    new_poly.count = 1;

                    nav_vertices.push(p1);

                    while (!cw_from_vector(w3 - w2, poly[poly_idx % poly.size] - w2))
                    {
                        nav_vertices.push(poly[poly_idx % poly.size]);
                        ++new_poly.count;
                        ++poly_idx;
                    }

                    nav_vertices.push(poly[poly_idx % poly.size]);
                    ++new_poly.count;

                    nav_vertices.push(w2);
                    ++new_poly.count;

                    nav_polys.push(new_poly);
                }
            }
        }
        else
        {
            // Check if the edge w1->w2 intersects poly
            for (uint poly_idx = 0; poly_idx < poly.size; ++poly_idx)
            {
                Vec2 p1 = poly[poly_idx];
                Vec2 p2 = poly[(poly_idx + 1) % poly.size];

                Vec2 intersection;
                if (edge_intersect(w1, w2, p1, p2, &intersection))
                {
                    if (point_in_poly(poly, w2))
                    {
                        // This edge of wall intersects poly only once
                        NavPoly new_poly;
                        new_poly.offset = nav_vertices.size;
                        new_poly.count = 2;

                        nav_vertices.push(w2);
                        nav_vertices.push(intersection);

                        // The next vertex of new_poly will be p2, aka poly[(poly_idx + 1) % poly.size]
                        ++poly_idx;

                        while (cw_from_vector(w2 - intersection, poly[poly_idx % poly.size] - intersection))
                        {
                            nav_vertices.push(poly[poly_idx % poly.size]);
                            ++new_poly.count;
                            ++poly_idx;
                        }

                        nav_polys.push(new_poly);

                        // Now check if there must be a polygon with vertex w2.
                        Vec2 w3 = wall[(wall_idx + 2) % wall.size];
                        if (!cw_from_vector(w3 - w2, *nav_vertices.back() - w2))
                        {
                            // We are reusing the last vertex of the polygon we just added.
                            new_poly.offset = nav_vertices.size - 1;
                            new_poly.count = 1;

                            while (!cw_from_vector(w3 - w2, poly[poly_idx % poly.size]))
                            {
                                nav_vertices.push(poly[poly_idx % poly.size]);
                                ++new_poly.count;
                                ++poly_idx;
                            }

                            nav_vertices.push(poly[poly_idx % poly.size]);
                            ++new_poly.count;

                            nav_vertices.push(w2);
                            ++new_poly.count;

                            nav_polys.push(new_poly);
                        }
                    }
                    else
                    {
                        // Both w1 and w2 are outside poly, so there must be a second intersection
                        bool found_intersection2 = false;
                        Vec2 intersection2;
                        uint old_poly_idx = poly_idx;
                        for (++poly_idx; poly_idx < poly.size; ++poly_idx)
                        {
                            Vec2 p3 = poly[poly_idx % poly.size];
                            Vec2 p4 = poly[(poly_idx + 1) % poly.size];

                            if (edge_intersect(w1, w2, p3, p4, &intersection2))
                            {
                                found_intersection2 = true;
                                break;
                            }
                        }
                        assert(found_intersection2);

                        // Now construct the new polygon.
                        // intersection and intersection2 need to have the correct winding for the
                        // new polygon. The start of the edge should be intersection and the end of
                        // the edge should be intersection2. Depending on the order that the edge
                        // intersections were detected, intersection and intersection2 may need to
                        // be swapped.
                        
                        // Index of the first vertex and one-past the last vertex of the new polygon.
                        // These also depend on the order that the edges were detected in.
                        uint poly_start_idx = poly_idx + 1;
                        uint poly_end_idx = old_poly_idx + 1;

                        // Make sure intersection and intersection2 will have the correct winding
                        // when used as an for the new polygon. The start of the edge will be intersection,
                        // and the end of the edge will be intersection2.
                        if ((w1 - intersection).square_magnitude() < (w1 - intersection2).square_magnitude())
                        {
                            // The first intersection along w1->w2 is intersection, the second is intersection2.
                            // The intersections must be swapped to have the correct winding.
                            Vec2 tmp = intersection;
                            intersection = intersection2;
                            intersection2 = tmp;

                            poly_start_idx = old_poly_idx + 1;
                            poly_end_idx = poly_idx + 1;
                        }

                        NavPoly new_poly;
                        new_poly.offset = nav_vertices.size;
                        new_poly.count = 2;

                        nav_vertices.push(intersection);
                        nav_vertices.push(intersection2);

                        for (int i = poly_start_idx; i % poly.size != poly_end_idx % poly.size; ++i)
                        {
                            nav_vertices.push(poly[i % poly.size]);
                            ++new_poly.count;
                        }

                        nav_polys.push(new_poly);
                    }
                }
            }
        }
    }
}

// Updates the nav mesh for a convex polygon obstacle defined by vertices
// TODO: This is horribly inefficient
void add_obstacle(Array<Vec2> wall)
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
}

void build_nav_mesh(float left, float right, float bottom, float top)
{
    nav_vertices.push(Vec2(left,  top));
    nav_vertices.push(Vec2(left,  bottom));
    nav_vertices.push(Vec2(right, bottom));
    nav_vertices.push(Vec2(right, top));

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
