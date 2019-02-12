CC = gcc

CFLAGS = $(shell pkg-config sdl2 --cflags) 
LDFLAGS = $(shell pkg-config sdl2 --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lopengl32

all: breakout.exe

cp_functions.o: cp_functions.c
	$(CC) $(CFLAGS) -c cp_functions.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

breakout.exe: cp_functions.o main.o
	$(CC) -o breakout.exe cp_functions.o main.o $(LDFLAGS)

clean: 
	rm -f *.o breakout.exe
