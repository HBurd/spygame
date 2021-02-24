.DEFAULT_GOAL := game

IMGUI_PATH = lib/imgui-1.80
GLEW_PATH = lib/glew-2.2.0
FAST_OBJ_PATH = lib/fast_obj-1.0
STB_IMAGE_PATH = lib/stb_image-2.26
SRC_PATH = src

CPPSRC = $(wildcard $(SRC_PATH)/*.cpp) $(wildcard $(IMGUI_PATH)/*.cpp) $(IMGUI_PATH)/backends/imgui_impl_sdl.cpp $(IMGUI_PATH)/backends/imgui_impl_opengl3.cpp

CSRC = $(GLEW_PATH)/src/glew.c $(FAST_OBJ_PATH)/fast_obj.c $(STB_IMAGE_PATH)/stb_image.c

OBJECTS = $(CPPSRC:.cpp=.o) $(CSRC:.c=.o)

CXXFLAGS = -MMD -g -Iinclude -I$(IMGUI_PATH) -Iinclude/SDL2 -I$(SRC_PATH) -I$(FAST_OBJ_PATH) -I$(STB_IMAGE_PATH) -I$(GLEW_PATH)/include -Wall -Wextra
CFLAGS = $(CXXFLAGS)
CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW

game: $(OBJECTS)
	g++ $(OBJECTS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -lGL -l:libSDL2.a

clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) game

-include $(OBJECTS:.o=.d)
