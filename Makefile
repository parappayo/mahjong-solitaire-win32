# MSYS2 / MinGW-w64: pacman -S mingw-w64-ucrt-x86_64-{clang,tools} (windres is in "tools")
CC       := clang
WINDRES  := windres
CFLAGS   := -std=c17 -Wall -Wextra -O2 -Iinclude
LDFLAGS  := -mwindows
LDLIBS   := -lgdiplus -lole32 -luuid

TARGET   := mahjong.exe
MAIN_O   := src/main.o
TILES_O  := src/load_tiles.o
RES_O    := res/app.o
RC       := res/app.rc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(MAIN_O) $(TILES_O) $(RES_O)
	$(CC) -o $@ $(MAIN_O) $(TILES_O) $(RES_O) $(LDFLAGS) $(LDLIBS)

$(MAIN_O): src/main.c include/load_tiles.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(TILES_O): src/load_tiles.c include/load_tiles.h include/resource.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(RES_O): $(RC) include/resource.h mahjong_tiles.gif
	$(WINDRES) -I include -o $@ $<

clean:
	rm -f $(TARGET) $(MAIN_O) $(TILES_O) $(RES_O)
