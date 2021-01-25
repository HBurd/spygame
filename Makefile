
OBJECTS = game.o keyboard.o main.o math2d.o rendering.o shapes.o imgui_impl_sdl.o

CXXFLAGS = -MMD -g -Iinclude -Iinclude/imgui -Iinclude/SDL2

CPPFLAGS = -DGLEW_STATIC -DGLEW_NO_GLU

game: $(OBJECTS)
	g++ $(OBJECTS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -lGL -l:libSDL2.a -l:imgui.a -l:glew.a

clean:
	rm *.o *.d game

-include *.d
