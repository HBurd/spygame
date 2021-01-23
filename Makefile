
OBJECTS = game.o keyboard.o main.o math2d.o rendering.o shapes.o imgui_impl_sdl.o

CXXFLAGS = -MMD -g -Iinclude -Iinclude/imgui -Iinclude/SDL2

game: $(OBJECTS)
	g++ $(OBJECTS) -o game $(CXXFLAGS) -Llib -ldl -lpthread -l:libSDL2.a -l:imgui.a

clean:
	rm *.o *.d game

-include *.d
