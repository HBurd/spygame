#include "SDL2/SDL.h"
#include "input.h"
#include "util.h"
#include "game.h"
#include "rendering.h"
#include "entity.h"

#include "GL/glew.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

using render::init_rendering;
using render::present_screen;

struct UpdateArgs
{
    SDL_Window* window;
    bool running;
};

void update(void* args)
{
    clear_input_events();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            static_cast<UpdateArgs*>(args)->running = false;
            return;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);

        handle_input_event(&event);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(static_cast<UpdateArgs*>(args)->window);
    ImGui::NewFrame();

    if (get_key_state(SDL_SCANCODE_ESCAPE).down)
    {
        static_cast<UpdateArgs*>(args)->running = false;
        return;
    }

    // TODO: frame timing
    update_game(1.0f / 60.0f);


    render_game();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    present_screen(static_cast<UpdateArgs*>(args)->window);
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    // These must be called before creating the window
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window* window = SDL_CreateWindow(
            "...",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            INITIAL_SCREEN_WIDTH,
            INITIAL_SCREEN_HEIGHT,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow returned null.\nMore info: %s", SDL_GetError());
        return 1;
    }

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_GLContext gl_ctxt = SDL_GL_CreateContext(window);
    if (!gl_ctxt)
    {
        fprintf(stderr, "SDL_GL_CreateContext returned null.\nMore info: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_ctxt);
    SDL_GL_SetSwapInterval(1);

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        fprintf(stderr, "glewInit did not return GLEW_OK.\nMore info: %s", glewGetErrorString(glew_status));
        return 1;
    }

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_ctxt);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    {
        ImGuiIO& io = ImGui::GetIO();
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 16);

        ImGui::StyleColorsDark();
        ImGui::GetStyle().WindowRounding = 4.0f;
        ImGui::GetStyle().FrameRounding = 4.0f;
    }

    init_rendering(window);

    init_entities();

    init_game();

    UpdateArgs args;
    args.window = window;
    args.running = true;

#ifndef __EMSCRIPTEN__

    while (args.running)
    {
        update(&args);
    }
#else
    emscripten_set_main_loop_arg(update, &args, 0, 1);
#endif

    SDL_Quit();
    return 0;
}
