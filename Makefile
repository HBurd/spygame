
OBJECTS = game.o keyboard.o main.o math2d.o rendering.o shapes.o imgui_impl_sdl.o imgui_impl_opengl3.o

CXXFLAGS = -MMD -g -Iinclude -Iinclude/imgui -Iinclude/SDL2

CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW

game: $(OBJECTS)
	g++ $(OBJECTS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -lGL -l:libSDL2.a -l:imgui.a -l:glew.a

clean:
	rm -f *.o *.d game

-include *.d
