.DEFAULT_GOAL := game

IMGUI_PATH = lib/imgui-1.80
GLEW_PATH = lib/glew-2.2.0

CPPSRC = $(wildcard *.cpp) $(wildcard $(IMGUI_PATH)/*.cpp) $(IMGUI_PATH)/backends/imgui_impl_sdl.cpp $(IMGUI_PATH)/backends/imgui_impl_opengl3.cpp

CSRC = $(GLEW_PATH)/src/glew.c

OBJECTS = $(CPPSRC:.cpp=.o) $(CSRC:.c=.o)

CXXFLAGS = -MMD -g -Iinclude -I$(IMGUI_PATH) -Iinclude/SDL2
CFLAGS = $(CXXFLAGS)
CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW

game: $(OBJECTS)
	g++ $(OBJECTS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -lGL -l:libSDL2.a

clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) game

-include *.d
