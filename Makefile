SRC = $(wildcard src/*.c)
EMCC_SRC = $(filter-out src/p2p.c src/ini.c, SRC)

gcc:
	windres config.rc -O coff -o config.res
	gcc -Iinclude -Llib $(SRC) -flto=auto -O2 -lSDL2 -lSDL2_net -lopengl32 -D MAINLOOP_WINDOWS -D MAINLOOP_GL -lshlwapi config.res -o "emuboy.exe"
	del config.res

emcc:
	emcc -Iinclude $(wildcard src/*.c) -O3 -flto=full \
	-sUSE_SDL=2 \
	-sINVOKE_RUN=0 \
	-sEXPORTED_FUNCTIONS=[_main,_emscripten_loadRom] \
	-o website/emulator.js