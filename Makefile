SRC = $(wildcard src/*.c)

all:
	windres config.rc -O coff -o config.res
	gcc -Iinclude -Llib $(SRC) -lSDL2 -lSDL2_net -O2 -flto -lopengl32 -D MAINLOOP_WINDOWS -D MAINLOOP_GL -lshlwapi config.res -o "gameboy chan.exe"
	del config.res