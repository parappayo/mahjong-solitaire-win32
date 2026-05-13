#ifndef GAME_H
#define GAME_H

#define GAME_TILE_COUNT (144)

typedef struct GameState {
    unsigned char face[GAME_TILE_COUNT];
    unsigned char removed[GAME_TILE_COUNT];
    int selected;
} GameState;

void game_new(GameState *g);

void game_board_client_size(int *out_w, int *out_h);

int game_hit_test(const GameState *g, int x, int y);

int game_tile_is_free(const GameState *g, int idx);

/** Returns 1 if a matching pair was removed. */
int game_on_tile_click(GameState *g, int idx);

void game_tile_screen_rect(int idx, int *out_x, int *out_y, int *out_w, int *out_h);

int game_draw_order(const GameState *g, int *out_order, int max_order);

#endif /* GAME_H */
