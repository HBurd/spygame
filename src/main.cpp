#include "SDL2/SDL.h"
#include "input.h"
#include "util.h"
#include "game.h"
#include "rendering.h"

#include "GL/glew.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"
#include <cstdio>

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

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
        ImGui::GetStyle().WindowRounding = 4.0f;
        ImGui::GetStyle().FrameRounding = 4.0f;
    }

    init_game();

    Renderer renderer(window);

    bool running = true;

    while (running)
    {
        clear_input_events();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            ImGui_ImplSDL2_ProcessEvent(&event);

            handle_input_event(&event);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (get_key_state(SDL_SCANCODE_ESCAPE).down)
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
