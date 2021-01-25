#include "SDL2/SDL.h"
#include "keyboard.h"
#include "util.h"
#include "game.h"
#include "rendering.h"

#include "GL/glew.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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
    ImGui_ImplOpenGL3_Init("#version 330");

    {
        ImGuiIO& io = ImGui::GetIO();
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 16);

        ImGui::StyleColorsDark();
    }

    init_game();

    Renderer renderer(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);

    bool running = true;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_QUIT:
                {
                   running = false;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        if (key_is_held(SDL_SCANCODE_ESCAPE))
        {
            running = false;
        }

        // TODO: frame timing
        update_game(1.0f / 60.0f);

        ImGui::Render();

        render_game(&renderer);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        renderer.present(window);
    }

    SDL_Quit();
    return 0;
}
