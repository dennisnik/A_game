CC = clang
CFLAGS = -Wall -I/opt/homebrew/opt/raylib/include
LDFLAGS = -L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

all: game

game: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o game

clean:
	rm -f game