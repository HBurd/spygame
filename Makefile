
game: *.cpp *.h
	g++ *.cpp -o game -g -Iinclude -Llib -ldl -lpthread -l:libSDL2.a
