#ifndef LOAD_TILES_H
#define LOAD_TILES_H

#include <windows.h>

typedef struct Tiles Tiles;

#define TILES_ERR_OLE 1
#define TILES_ERR_GDIPLUS 2

/** 0 = success, or TILES_ERR_* on failure. */
int tiles_runtime_begin(void);
void tiles_runtime_end(void);

Tiles *tiles_load_embedded_gif(HINSTANCE instance);
void tiles_free(Tiles *tiles);

UINT tiles_frame_count(const Tiles *tiles);
UINT tiles_frame_delay_ms(const Tiles *tiles, UINT frame_index);
int tiles_set_frame(Tiles *tiles, UINT frame_index);
void tiles_pixel_size(const Tiles *tiles, UINT *out_width, UINT *out_height);
void tiles_draw_stretched(HDC hdc, const Tiles *tiles, int x, int y, int width, int height);

#endif
