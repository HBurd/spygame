#include "game.h"
#include "util.h"
#include "math2d.h"
#include "rendering.h"
#include "input.h"
#include "shapes.h"
#include "entity.h"
#include "save_load.h"
#include "navigation.h"

#include "imgui.h"
#include <cmath>

using namespace math;
using namespace render;

struct CameraView
{
    Vec3 pos;

    float yaw = 0.0f;
    float pitch = 0.1f;
    Quaternion compute_orientation() const;
};

Quaternion CameraView::compute_orientation() const
{
    return Quaternion::RotateZ(yaw) * Quaternion::RotateX(pitch);
}

GameState game_state;

Camera camera;
CameraView camera_view;

LightSource light_source;
u32 skybox;

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
        RenderObjectIndex building = load_obj("building.obj");

        // Create a random scene to start with
        create_entity(Transform2d({2.0f, 0.0f}, {0.5f, 5.0f}, 0.1f));
        EntityRef building_entity = create_entity(Transform2d({-1.7f, 0.1f}, {1.0f, 1.0f}, -1.0f));
        lookup_entity(building_entity)->render_object = building;

        game_state.player = create_entity(Transform2d());
    }

    skybox = render::load_texture("cubemap.png");

    Entity* player = lookup_entity(game_state.player);
    assert(player);

    player->render_object = render::load_obj("bettermug.obj");

    build_nav_mesh(-10.0f, 10.0f, -10.0f, 10.0f);
}

bool editor_enabled = true;
EntityRef selected_object;
Vec2 selection_offset;

bool show_imgui_demo = false;
bool show_light_window = false;
bool show_camera_window = false;
bool show_navigation_window = false;

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
                ImGui::MenuItem("Navigation", nullptr, &show_navigation_window);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        {
            Entity* selected_entity = lookup_entity(selected_object);

            // Handle object selection with mouse
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

            if (selected_entity)
            {
                if (ImGui::Begin("Selected Object"))
                {
                    ImGui::InputFloat2("Position", selected_entity->transform.pos.array());
                    ImGui::InputFloat2("Size", selected_entity->transform.scale.array());
                    ImGui::SliderFloat("Rotation", &selected_entity->transform.rotation, 0.0f, 2.0f * M_PI);

                    if (ImGui::Button("Duplicate"))
                    {
                        selected_object = create_entity(*selected_entity);
                        selected_entity = lookup_entity(selected_object);
                    }

                    // NOTE: It's important that this is last since it will invalidate selected_entity
                    if (ImGui::Button("Delete"))
                    {
                        delete_entity(selected_object);
                        selected_object = EntityRef();
                    }
                }
                ImGui::End();
            }
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

        if (show_navigation_window)
        {
            if (ImGui::Begin("Navigation", &show_navigation_window))
            {
                // TODO: Is this too hacky?
                static float bounds[4] = {-10.0f, 10.0f, -10.0f, 10.0f};

                ImGui::InputFloat4("left/right/bottom/top bounds", bounds);

                if (ImGui::Button("(Re)generate nav mesh"))
                {
                    build_nav_mesh(bounds[0], bounds[1], bounds[2], bounds[3]);
                }
            }
            ImGui::End();
        }
    }

    {
        MouseState mouse = get_mouse_state();
        if (mouse.right.held)
        {
            camera_view.yaw -= 0.005f * mouse.xrel;
            camera_view.pitch -= 0.005f * mouse.yrel;

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

    {
        float camera_speed = 4.0f;
        Vec3 camera_delta;

        if (get_key_state(SDL_SCANCODE_A).held)
        {
            camera_delta += dt * Vec3(-1.0f, 0.0f, 0.0f);
        }
        if (get_key_state(SDL_SCANCODE_D).held)
        {
            camera_delta += dt * Vec3(1.0f, 0.0f, 0.0f);
        }
        if (get_key_state(SDL_SCANCODE_W).held)
        {
            camera_delta += dt * Vec3(0.0f, 0.0f, -1.0f);
        }
        if (get_key_state(SDL_SCANCODE_S).held)
        {
            camera_delta += dt * Vec3(0.0f, 0.0f, 1.0f);
        }
        if (get_key_state(SDL_SCANCODE_Q).held)
        {
            camera_delta += dt * Vec3(0.0f, -1.0f, 0.0f);
        }
        if (get_key_state(SDL_SCANCODE_E).held)
        {
            camera_delta += dt * Vec3(0.0f, 1.0f, 0.0f);
        }

        camera_view.pos += camera_speed * camera_view.compute_orientation().apply_rotation(camera_delta);
    }

    player_velocity = Vec2();

    if (get_key_state(SDL_SCANCODE_LEFT).held)
    {
        player_velocity += Vec2(-1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_RIGHT).held)
    {
        player_velocity += Vec2(1.0f, 0.0f);
    }
    if (get_key_state(SDL_SCANCODE_UP).held)
    {
        player_velocity += Vec2(0.0f, 1.0f);
    }
    if (get_key_state(SDL_SCANCODE_DOWN).held)
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
    draw_skybox(skybox, camera_view.pos);
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

    camera.pos = camera_view.pos;
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

        for (auto p : nav_polys)
        {
            if (p.occupied)
            {
                debug_draw_poly(&nav_vertices[p.offset], p.count, 0.0f, 0.0f, 1.0f);
            }
        }
    }
}
