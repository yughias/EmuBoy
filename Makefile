SRC = $(wildcard src/*.c)

all:
	windres config.rc -O coff -o config.res
	gcc -Iinclude -Llib $(SRC) -flto=auto -O3 -lSDL2 -lSDL2_net -lopengl32 -D MAINLOOP_WINDOWS -D MAINLOOP_GL -lshlwapi config.res -o "emuboy.exe"
	del config.res