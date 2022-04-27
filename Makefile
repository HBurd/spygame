.DEFAULT_GOAL := game

IMGUI_PATH = lib/imgui-1.80
GLEW_PATH = lib/glew-2.2.0
FAST_OBJ_PATH = lib/fast_obj-1.0
STB_IMAGE_PATH = lib/stb_image-2.26
HBMATH_PATH = lib/hbmath
SRC_PATH = src

SRC = $(wildcard $(SRC_PATH)/*.cpp)


CPPLIBS = $(wildcard $(IMGUI_PATH)/*.cpp) $(IMGUI_PATH)/backends/imgui_impl_sdl.cpp $(IMGUI_PATH)/backends/imgui_impl_opengl3.cpp $(HBMATH_PATH)/hbmath.cpp

CLIBS = $(GLEW_PATH)/src/glew.c $(FAST_OBJ_PATH)/fast_obj.c $(STB_IMAGE_PATH)/stb_image.c

LIBS = $(CPPLIBS:.cpp=.o) $(CLIBS:.c=.o)

CXXFLAGS = -g -Iinclude -I$(IMGUI_PATH) -Iinclude/SDL2 -I$(SRC_PATH) -I$(FAST_OBJ_PATH) -I$(STB_IMAGE_PATH) -I$(GLEW_PATH)/include -I$(HBMATH_PATH) -Wall -Wextra
CFLAGS = $(CXXFLAGS)
CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW

game: $(LIBS)
	g++ $(SRC) $(LIBS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -lGL -l:libSDL2.a

compile_flags.txt:
	echo $(CXXFLAGS) | sed -e "s/ /\n/g" > compile_flags.txt

clean:
	rm -f $(LIBS) compile_flags.txt game
