/* Source image: D:\repos\mahjong-solitaire-win32\mahjong_tiles.gif */
#ifndef TILE_BOUNDS_H
#define TILE_BOUNDS_H

#include <stddef.h>

typedef struct TileBounds {
    int x;
    int y;
    int w;
    int h;
} TileBounds;

static const TileBounds kTileBounds[] = {
    // row of numbers at the top
    { 24, 7, 24, 48 },
    { 48, 7, 24, 48 },
    { 72, 7, 24, 48 },
    { 96, 7, 24, 48 },
    { 120, 7, 24, 48 },
    { 164, 7, 24, 48 },
    { 198, 7, 24, 48 },
    { 222, 7, 24, 48 },
    { 246, 7, 24, 48 },

    // character tiles
    { 24, 24, 24, 48 },
    { 48, 24, 24, 48 },
    { 72, 24, 24, 48 },
    { 96, 24, 24, 48 },
    { 120, 24, 24, 48 },
    { 164, 24, 24, 48 },
    { 198, 24, 24, 48 },
    { 222, 24, 24, 48 },
    { 246, 24, 24, 48 },

    // dot tiles
    { 24, 72, 24, 48 },
    { 48, 72, 24, 48 },
    { 72, 72, 24, 48 },
    { 96, 72, 24, 48 },
    { 120, 72, 24, 48 },
    { 164, 72, 24, 48 },
    { 198, 72, 24, 48 },
    { 222, 72, 24, 48 },
    { 246, 72, 24, 48 },

    // stalks tiles
    { 24, 120, 24, 48 },
    { 48, 120, 24, 48 },
    { 72, 120, 24, 48 },
    { 96, 120, 24, 48 },
    { 120, 120, 24, 48 },
    { 164, 120, 24, 48 },
    { 198, 120, 24, 48 },
    { 222, 120, 24, 48 },
    { 246, 120, 24, 48 },

    // others
    { 24, 184, 24, 48 },
    { 56, 184, 24, 48 },
    { 88, 184, 24, 48 },
    { 120, 184, 24, 48 },
    { 168, 184, 24, 48 },
    { 192, 184, 24, 48 },
    { 216, 184, 24, 48 },
};

#define TILE_BOUNDS_COUNT ((unsigned)(sizeof(kTileBounds) / sizeof((kTileBounds)[0])))

#endif /* TILE_BOUNDS_H */
