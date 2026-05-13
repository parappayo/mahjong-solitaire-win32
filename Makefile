# MSYS2 / MinGW-w64: install mingw-w64-clang (UCRT64 or MINGW64 shell), then `make`.
CC      := clang
CFLAGS  := -std=c17 -Wall -Wextra -O2
LDFLAGS := -mwindows

TARGET := mahjong.exe
SRC    := src/main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
