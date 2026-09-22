tetris: main.o tetris.o
	gcc -lcsfml-system -lcsfml-window -lcsfml-graphics main.o tetris.o -o tetris
main.o: main.c texturedata.h
	gcc -c main.c
tetris.o: tetris.c
	gcc -c tetris.c
clean:
	rm -f tetris.o main.o tetris
