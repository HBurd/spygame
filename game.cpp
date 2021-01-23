#include "game.h"
#include "util.h"
#include "math2d.h"
#include "rendering.h"
#include "keyboard.h"
#include "shapes.h"

#include <cmath>

using namespace math;

Renderer renderer;

Rectangle player;

Vec2 player_velocity;

MAKE_ARRAY(all_walls, Rectangle, 1024);

void init_game(SDL_Renderer* sdl_renderer)
{
    renderer.init(sdl_renderer);

    all_walls.push(Rectangle({2.0f, 0.0f}, {0.5f, 5.0f}, 0.1f));
    all_walls.push(Rectangle({-1.7f, 0.1f}, {0.5f, 5.0f}, -1.0f));
}

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

// Penetration_vector is optional, points out of r2
bool intersect(Rectangle r1, Rectangle r2, Vec2* penetration_vector)
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

void update_game(float dt)
{
    player_velocity = Vec2();

    if (key_is_held(SDL_SCANCODE_LEFT))
    {
        player_velocity += Vec2(-1.0f, 0.0f);
    }
    if (key_is_held(SDL_SCANCODE_RIGHT))
    {
        player_velocity += Vec2(1.0f, 0.0f);
    }
    if (key_is_held(SDL_SCANCODE_UP))
    {
        player_velocity += Vec2(0.0f, 1.0f);
    }
    if (key_is_held(SDL_SCANCODE_DOWN))
    {
        player_velocity += Vec2(0.0f, -1.0f);
    }

    player.pos += dt * player_velocity;

    // Check for collisions
    uint num_collisions = 0;
    Vec2 penetration;
    for (auto wall : all_walls)
    {
        Vec2 collision;
        if (intersect(player, wall, &penetration))
        {
            num_collisions++;
        }
    }

    // Resolve collisions
    if (num_collisions == 1)
    {
        // In this case we can just translate by the penetration vector
        player.pos += 1.05f * penetration;
    }
    else if (num_collisions > 1)
    {
        // This case is tricky to handle, so for now just go back to the start
        // of the frame
        player.pos -= dt * player_velocity;
    }
}

void render_game()
{
    renderer.clear();

    for (auto wall : all_walls)
    {
        renderer.debug_draw_rectangle(wall);
    }

    renderer.debug_draw_rectangle(player);

    renderer.present();
}
