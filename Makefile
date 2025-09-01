SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c, obj/%.o, $(SRC))
DEP := $(OBJ:.o=.d)
EMCC_SRC = $(filter-out src/p2p.c src/escapi.c, $(SRC))

CC := gcc
EXE := emuboy.exe
CFLAGS := -Iinclude -flto -O3 -DMAINLOOP_WINDOWS -DMAINLOOP_GL
DEBUG_FLAGS := -pg -no-pie
LIBS := -Llib -lSDL2 -lSDL2_net -lopengl32 -lshlwapi

all: $(EXE)

$(EXE): $(OBJ)
	windres config.rc -O coff -o config.res
	$(CC) $(OBJ) $(CFLAGS) $(LIBS) -o $(EXE) config.res
	rm config.res

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -MMD -MP $(CFLAGS) $< -o $@

emcc:
	emcc -Iinclude $(EMCC_SRC) -O3 -flto=full \
	-sUSE_SDL=2 \
	-sINVOKE_RUN=0 \
	-sEXPORTED_FUNCTIONS=[_main,_emscripten_loadRom,_switchCompatibilityMode] \
	-o website/emulator.js

emuboy.zip: $(EXE)
	tar -c -a -f emuboy.zip $(EXE) data/config.ini data/logo.bmp *.dll

clean:
	rm -rf obj $(EXE)

.PHONY: emuboy.zip clean

-include $(DEP)