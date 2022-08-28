TARGET = game
.DEFAULT_GOAL := $(TARGET)
.PHONY: www
.PHONY: clean

IMGUI_PATH = lib/imgui-1.80
GLEW_PATH = lib/glew-2.2.0
FAST_OBJ_PATH = lib/fast_obj-1.0
STB_IMAGE_PATH = lib/stb_image-2.26
HBMATH_PATH = lib/hbmath
SRC_PATH = src

SRC = $(wildcard $(SRC_PATH)/*.cpp)

NATIVE_CLIBS = $(GLEW_PATH)/src/glew.c
COMMON_CLIBS = $(FAST_OBJ_PATH)/fast_obj.c $(STB_IMAGE_PATH)/stb_image.c

COMMON_CPPLIBS = $(wildcard $(IMGUI_PATH)/*.cpp) $(IMGUI_PATH)/backends/imgui_impl_sdl.cpp $(IMGUI_PATH)/backends/imgui_impl_opengl3.cpp $(HBMATH_PATH)/hbmath.cpp


LIB_OBJECTS = $(COMMON_CPPLIBS:.cpp=.o) $(COMMON_CLIBS:.c=.o) $(NATIVE_CLIBS:.c=.o)
LIB_OBJECTS_EMSCRIPTEN = $(COMMON_CPPLIBS:.cpp=.emcxx.o) $(COMMON_CLIBS:.c=.emc.o)

CXXFLAGS = -g -Iinclude -Iinclude/SDL2 -I$(IMGUI_PATH) -I$(SRC_PATH) -I$(FAST_OBJ_PATH) -I$(STB_IMAGE_PATH) -I$(HBMATH_PATH) -Wall -Wextra
CFLAGS = $(CXXFLAGS)
CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DIMGUI_IMPL_OPENGL_ES3

EMSCRIPTEN_FLAGS = -sUSE_SDL=2 -sFULL_ES3 -sUSE_WEBGL2 -sALLOW_MEMORY_GROWTH --preload-file Roboto-Regular.ttf --preload-file shaders --preload-file test.scene --preload-file building.obj --preload-file bettermug.obj --preload-file cubemap.png --preload-file building.mtl --preload-file bettermug.mtl --preload-file uvtest.png

game: $(SRC) $(LIB_OBJECTS) lib/libSDL2.a
	g++ $^ -o $@ $(CXXFLAGS) -I$(GLEW_PATH)/include -Llib -ldl -lpthread -lGL

compile_flags.txt:
	echo $(CXXFLAGS) | sed -e "s/ /\n/g" > compile_flags.txt

clean:
	rm -f $(LIB_OBJECTS) $(LIB_OBJECTS_EMSCRIPTEN) compile_flags.txt game

%.emcxx.o: %.cpp
	em++ -o $@ -c $^ $(CXXFLAGS) $(CPPFLAGS) $(EMSCRIPTEN_FLAGS)

%.emc.o: %.c
	emcc -o $@ -c $^ $(CFLAGS) $(CPPFLAGS) $(EMSCRIPTEN_FLAGS)

www/$(TARGET): $(SRC) $(LIB_OBJECTS_EMSCRIPTEN)
	mkdir -p www
	em++ -o $@.html $^ $(CXXFLAGS) $(CPPFLAGS) $(EMSCRIPTEN_FLAGS)

www: www/$(TARGET)
