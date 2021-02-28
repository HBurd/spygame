#include "game.h"
#include "util.h"
#include "math2d.h"
#include "rendering.h"
#include "input.h"
#include "shapes.h"
#include "entity.h"
#include "save_load.h"

#include "imgui.h"
#include <cmath>

using namespace math;
using namespace render;

struct CameraView
{
    float distance = 5.0f;
    float yaw = 0.0f;
    float pitch = 0.1f;

    Vec3 target;

    Vec3 compute_position() const;
    Quaternion compute_orientation() const;
};

Vec3 CameraView::compute_position() const
{
    return target + compute_orientation().apply_rotation(Vec3(0.0f, 0.0f, distance));
}

Quaternion CameraView::compute_orientation() const
{
    return Quaternion::RotateZ(yaw) * Quaternion::RotateX(pitch);
}

GameState game_state;

Camera camera;
CameraView camera_view;

LightSource light_source;
RenderObject skybox;

Vec2 player_velocity;

void init_game()
{
    camera.set_fov(0.5f * M_PI);

    light_source = make_light_source(1024);
    light_source.camera.is_ortho = true;
    light_source.camera.near_width = 20.0f;
    light_source.camera.pos = Vec3(0.0f, 0.0f, 10.0f);
    light_source.camera.orientation = Quaternion::RotateZ(1.0f) * Quaternion::RotateX(-0.25f * M_PI);

    if (load_scene("test.scene"))
    {
        // Scene loaded successfully
    }
    else
    {
        // Create a random scene to start with
        create_entity(Transform2d({2.0f, 0.0f}, {0.5f, 5.0f}, 0.1f));
        create_entity(Transform2d({-1.7f, 0.1f}, {0.5f, 5.0f}, -1.0f));

        game_state.player = create_entity(Transform2d());
    }

    skybox = render::create_skybox("cubemap.png");

    Entity* player = lookup_entity(game_state.player);
    assert(player);

    player->render_object = render::load_obj("bettermug.obj");
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

bool editor_enabled = false;
EntityRef selected_object;
Vec2 selection_offset;

bool show_imgui_demo = false;
bool show_light_window = false;
bool show_camera_window = false;

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
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save scene"))
                {
                    save_scene("test.scene");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Wall"))
                {
                    Transform2d new_wall;
                    selected_object = create_entity(new_wall);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("ImGui demo window", nullptr, &show_imgui_demo);
                ImGui::MenuItem("Edit light", nullptr, &show_light_window);
                ImGui::MenuItem("Edit camera", nullptr, &show_camera_window);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        Entity* selected_entity = lookup_entity(selected_object);

        // Handle object selection with mouse
        {
            MouseState mouse = get_mouse_state();
            Vec3 mouse_pos, mouse_dir;
            camera.pixel_ray(mouse.x, mouse.y, get_screen_width(), get_screen_height(), &mouse_pos, &mouse_dir);
            Vec2 mouse_in_plane = z_intersect(mouse_pos, mouse_dir, 0.0f);

            if (mouse.left.down)
            {
                // Clear selection
                selected_object = EntityRef();

                // Check if user clicked an entity
                for (auto& entity : entities)
                {
                    if (rectangle_contains_point(entity.transform, mouse_in_plane))
                    {
                        selected_object = entity.ref;
                        selected_entity = lookup_entity(selected_object);
                    }
                }

                if (selected_entity)
                {
                    selection_offset = selected_entity->transform.pos - mouse_in_plane;
                }
            }

            if (mouse.left.held && selected_entity)
            {
                float rotation_change = 0.05f * mouse.wheel;
                selection_offset = math::Mat2::Rotation(rotation_change) * selection_offset;
                selected_entity->transform.pos = mouse_in_plane + selection_offset;
                selected_entity->transform.rotation += rotation_change;
            }
        }

        if (selected_entity)
        {
            if (ImGui::Begin("Selected Object"))
            {
                ImGui::InputFloat2("Position", selected_entity->transform.pos.array());
                ImGui::InputFloat2("Size", selected_entity->transform.scale.array());
                ImGui::SliderFloat("Rotation", &selected_entity->transform.rotation, 0.0f, 2.0f * M_PI);
            }
            ImGui::End();
        }

        if (show_imgui_demo)
        {
            ImGui::ShowDemoWindow(&show_imgui_demo);
        }

        if (show_light_window)
        {
            if (ImGui::Begin("Edit Light", &show_light_window))
            {
                light_source.draw_gui();
            }
            ImGui::End();
        }

        if (show_camera_window)
        {
            if (ImGui::Begin("Edit Camera", &show_camera_window))
            {
                camera.draw_gui();
            }
            ImGui::End();
        }
    }

    {
        MouseState mouse = get_mouse_state();
        if (mouse.right.held)
        {
            camera_view.yaw -= 0.01f * mouse.xrel;
            camera_view.pitch -= 0.01f * mouse.yrel;

            if (camera_view.yaw > M_PI)
            {
                camera_view.yaw -= 2.0f * M_PI;
            }
            else if (camera_view.yaw < -M_PI)
            {
                camera_view.yaw += 2.0f * M_PI;
            }

            if (camera_view.pitch < 0.0f)
            {
                camera_view.pitch = 0.0f;
            }
            else if (camera_view.pitch > M_PI)
            {
                camera_view.pitch = M_PI;
            }
        }
    }

    if (get_key_state(SDL_SCANCODE_LEFT).held)
    {
        camera_view.target += dt * Vec3(-1.0f, 0.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_RIGHT).held)
    {
        camera_view.target += dt * Vec3(1.0f, 0.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_UP).held)
    {
        camera_view.target += dt * Vec3(0.0f, 1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_DOWN).held)
    {
        camera_view.target += dt * Vec3(0.0f, -1.0f, 0.0f);
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

    Entity* player_entity = lookup_entity(game_state.player);

    if (player_entity)
    {
        player_entity->transform.pos += dt * player_velocity;

        // Check for collisions
        uint num_collisions = 0;
        Vec2 penetration;
        for (auto& entity : entities)
        {
            if (&entity == player_entity)
            {
                continue;
            }

            Vec2 collision;
            if (intersect(player_entity->transform, entity.transform, &penetration))
            {
                num_collisions++;
            }
        }

        // Resolve collisions
        if (num_collisions == 1)
        {
            // In this case we can just translate by the penetration vector
            player_entity->transform.pos += 1.05f * penetration;
        }
        else if (num_collisions > 1)
        {
            // This case is tricky to handle, so for now just go back to the start
            // of the frame
            player_entity->transform.pos -= dt * player_velocity;
        }
    }
}

static void draw_scene()
{
    draw_skybox(skybox, camera_view.compute_position());
    for (auto& entity : entities)
    {
        Transform3d box_transform(Vec3(entity.transform.pos.x, entity.transform.pos.y, 0.5f), Vec3(entity.transform.scale.x, entity.transform.scale.y, 1.0f), entity.transform.rotation);
        draw_object(box_transform, entity.render_object);
    }
}

void render_game()
{
    prepare_lightmap_draw(light_source);
    draw_scene();

    camera.pos = camera_view.compute_position();
    camera.orientation = camera_view.compute_orientation();
    prepare_final_draw(camera, light_source);
    draw_scene();

    if (editor_enabled)
    {
        prepare_debug_draw(camera);
        for (auto& entity : entities)
        {
            float r, g, b;
            if (entity.ref == selected_object)
            {
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
            }
            else
            {
                r = 0.0f;
                g = 1.0f;
                b = 0.0f;
            }

            debug_draw_rectangle(entity.transform, r, g, b);
        }
    }
}
