tetris: main.o tetris.o texture_png.o
	gcc -lcsfml-system -lcsfml-window -lcsfml-graphics main.o tetris.o texture_png.o -o tetris
texture_png.c: texture.png
	xxd -i texture.png texture_png.c
texture_png.o: texture_png.c
	gcc -c texture_png.c
main.o: main.c texturedata.h
	gcc -c main.c
tetris.o: tetris.c
	gcc -c tetris.c
clean:
	rm -f texture_png.c texture_png.o tetris.o main.o tetris
