SRC = $(wildcard src/*.c)

gcc:
	windres config.rc -O coff -o config.res
	gcc -Iinclude -Llib $(SRC) -lSDL2 -O2 -lopengl32 -D MAINLOOP_WINDOWS -D MAINLOOP_GL -lshlwapi config.res -o "gameboy chan.exe"
	del config.res

debug:
	windres config.rc -O2 coff -o config.res
	gcc -D DEBUG -Iinclude -Llib $(SRC) -lSDL2 -O2 -lopengl32 -D MAINLOOP_WINDOWS -D MAINLOOP_GL -lshlwapi config.res -o "gameboy chan.exe"
	del config.res