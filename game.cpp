#include "game.h"
#include "util.h"
#include "math2d.h"
#include "rendering.h"
#include "input.h"
#include "shapes.h"

#include "imgui.h"
#include <cmath>

using namespace math;

CameraView camera;

Rectangle player;

Vec2 player_velocity;

MAKE_ARRAY(all_walls, Rectangle, 1024);

void init_game()
{
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

bool rectangle_contains_point(Rectangle rect, Vec2 point)
{
    float cosine = cosf(rect.rotation);
    float sine   = sinf(rect.rotation);

    Vec2 basis1(cosine, sine);
    Vec2 basis2(-sine, cosine);

    float projection1 = dot(basis1, point - rect.pos);
    float projection2 = dot(basis2, point - rect.pos);

    return fabs(projection1) <= 0.5f * rect.scale.x && fabs(projection2) <= 0.5f * rect.scale.y;
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

bool editor_enabled = false;
Rectangle* selected_object = nullptr;
Vec2 selection_offset;

bool show_imgui_demo = false;

void update_game(float dt)
{
    if (get_key_state(SDL_SCANCODE_GRAVE).down)
    {
        editor_enabled = !editor_enabled;
    }

    if (editor_enabled)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Wall"))
                {
                    Rectangle new_wall;
                    selected_object = all_walls.push(new_wall);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("ImGui demo window", nullptr, &show_imgui_demo);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Handle object selection with mouse
        {
            MouseState mouse = get_mouse_state();
            Vec2 mouse_pos;// = renderer->pixels_to_world(mouse.x, mouse.y);
            if (mouse.left.down)
            {
                selected_object = nullptr;
                for (auto& object : all_walls)
                {
                    if (rectangle_contains_point(object, mouse_pos))
                    {
                        selected_object = &object;
                    }
                }

                if (selected_object)
                {
                    selection_offset = selected_object->pos - mouse_pos;
                }
            }

            if (mouse.left.held && selected_object)
            {
                float rotation_change = 0.05f * mouse.wheel;
                selection_offset = math::Mat2::Rotation(rotation_change) * selection_offset;
                selected_object->pos = mouse_pos + selection_offset;
                selected_object->rotation += rotation_change;
            }
        }

        if (selected_object)
        {
            if (ImGui::Begin("Selected Object"))
            {
                ImGui::InputFloat2("Position", selected_object->pos.array());
                ImGui::InputFloat2("Size", selected_object->scale.array());
                ImGui::SliderFloat("Rotation", &selected_object->rotation, 0.0f, 2.0f * M_PI);
            }
            ImGui::End();
        }

        if (show_imgui_demo)
        {
            ImGui::ShowDemoWindow(&show_imgui_demo);
        }
    }

    {
        MouseState mouse = get_mouse_state();
        if (mouse.right.held)
        {
            camera.yaw += 0.01f * mouse.xrel;
            camera.pitch += 0.01f * mouse.yrel;

            if (camera.yaw > M_PI)
            {
                camera.yaw -= 2.0f * M_PI;
            }
            else if (camera.yaw < -M_PI)
            {
                camera.yaw += 2.0f * M_PI;
            }

            if (camera.pitch > 0.0f)
            {
                camera.pitch = 0.0f;
            }
            else if (camera.pitch < -0.5f * M_PI)
            {
                camera.pitch = -0.5f * M_PI;
            }
        }
    }

    if (get_key_state(SDL_SCANCODE_LEFT).held)
    {
        camera.target += dt * Vec3(-1.0f, 0.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_RIGHT).held)
    {
        camera.target += dt * Vec3(1.0f, 0.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_UP).held)
    {
        camera.target += dt * Vec3(0.0f, 1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_DOWN).held)
    {
        camera.target += dt * Vec3(0.0f, -1.0f, 0.0f);
    }

    player_velocity = Vec2();

    if (get_key_state(SDL_SCANCODE_A).held)
    {
        player_velocity += Vec2(-1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_D).held)
    {
        player_velocity += Vec2(1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_W).held)
    {
        player_velocity += Vec2(0.0f, 1.0f);
    }
    if (get_key_state(SDL_SCANCODE_S).held)
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

void render_game(Renderer* renderer)
{
    renderer->prepare(camera);

    for (auto& wall : all_walls)
    {
        if (&wall == selected_object)
        {
            renderer->debug_draw_rectangle(wall, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            renderer->debug_draw_rectangle(wall, 0.0f, 1.0f, 0.0f);
        }
    }

    renderer->debug_draw_rectangle(player, 0.0f, 0.0f, 1.0f);
}
