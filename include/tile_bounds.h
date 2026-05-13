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
    // character tiles
    { 24, 24, 24, 48 },
    { 48, 24, 24, 48 },
    { 72, 24, 24, 48 },
    { 96, 24, 24, 48 },
    { 120, 24, 24, 48 },
    { 144, 24, 24, 48 },
    { 168, 24, 24, 48 },
    { 192, 24, 24, 48 },
    { 216, 24, 24, 48 },

    // dot tiles
    { 24, 72, 24, 48 },
    { 48, 72, 24, 48 },
    { 72, 72, 24, 48 },
    { 96, 72, 24, 48 },
    { 120, 72, 24, 48 },
    { 144, 72, 24, 48 },
    { 168, 72, 24, 48 },
    { 192, 72, 24, 48 },
    { 216, 72, 24, 48 },

    // stalks tiles
    { 24, 120, 24, 48 },
    { 48, 120, 24, 48 },
    { 72, 120, 24, 48 },
    { 96, 120, 24, 48 },
    { 120, 120, 24, 48 },
    { 144, 120, 24, 48 },
    { 168, 120, 24, 48 },
    { 192, 120, 24, 48 },
    { 216, 120, 24, 48 },

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
