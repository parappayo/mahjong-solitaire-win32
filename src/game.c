#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "game.h"
#include "tile_bounds.h"
#include "turtle_layout.h"

_Static_assert(GAME_TILE_COUNT == 144, "GAME_TILE_COUNT");
_Static_assert(TURTLE_TILE_COUNT == 144u, "TURTLE_TILE_COUNT");

enum {
    kTileFootprint = 2,
    kLayoutUnitPxX = 24,
    kLayoutUnitPxY = 48,
    kLayerLiftPx = 8,
    kTileDrawW = 48,
    kTileDrawH = 96,
    kBoardMargin = 32,
};

static void game_slot_to_screen(int idx, int *out_x, int *out_y) {
    const TurtleSlot *s = &kTurtleLayout[idx];
    *out_x = kBoardMargin + s->x * kLayoutUnitPxX;
    *out_y = kBoardMargin + s->y * kLayoutUnitPxY - s->z * kLayerLiftPx;
}

void game_tile_screen_rect(int idx, int *out_x, int *out_y, int *out_w, int *out_h) {
    int x = 0;
    int y = 0;
    if (idx >= 0 && (unsigned)idx < GAME_TILE_COUNT) {
        game_slot_to_screen(idx, &x, &y);
    }
    if (out_x) {
        *out_x = x;
    }
    if (out_y) {
        *out_y = y;
    }
    if (out_w) {
        *out_w = kTileDrawW;
    }
    if (out_h) {
        *out_h = kTileDrawH;
    }
}

void game_board_client_size(int *out_w, int *out_h) {
    int max_r = 0;
    int max_b = 0;
    for (unsigned i = 0; i < GAME_TILE_COUNT; i++) {
        int x = 0;
        int y = 0;
        game_slot_to_screen((int)i, &x, &y);
        int r = x + kTileDrawW;
        int b = y + kTileDrawH;
        if (r > max_r) {
            max_r = r;
        }
        if (b > max_b) {
            max_b = b;
        }
    }
    if (out_w) {
        *out_w = max_r + kBoardMargin;
    }
    if (out_h) {
        *out_h = max_b + kBoardMargin;
    }
}

static int game_xy_overlap_slots(const TurtleSlot *a, const TurtleSlot *b) {
    return a->x < b->x + kTileFootprint && a->x + kTileFootprint > b->x && a->y < b->y + kTileFootprint &&
           a->y + kTileFootprint > b->y;
}

static int game_y_overlap_slots(const TurtleSlot *a, const TurtleSlot *b) {
    return a->y < b->y + kTileFootprint && a->y + kTileFootprint > b->y;
}

static int game_covered_from_above(const GameState *g, int i) {
    const TurtleSlot *ti = &kTurtleLayout[i];
        for (unsigned j = 0; j < GAME_TILE_COUNT; j++) {
        if (j == (unsigned)i || g->removed[j]) {
            continue;
        }
        const TurtleSlot *tj = &kTurtleLayout[j];
        if (tj->z > ti->z && game_xy_overlap_slots(ti, tj)) {
            return 1;
        }
    }
    return 0;
}

static int game_left_blocked(const GameState *g, int i) {
    const TurtleSlot *ti = &kTurtleLayout[i];
        for (unsigned j = 0; j < GAME_TILE_COUNT; j++) {
        if (j == (unsigned)i || g->removed[j]) {
            continue;
        }
        const TurtleSlot *tj = &kTurtleLayout[j];
        if (tj->z < ti->z) {
            continue;
        }
        if (!game_y_overlap_slots(ti, tj)) {
            continue;
        }
        if (tj->x + kTileFootprint >= ti->x && tj->x < ti->x) {
            return 1;
        }
    }
    return 0;
}

static int game_right_blocked(const GameState *g, int i) {
    const TurtleSlot *ti = &kTurtleLayout[i];
        for (unsigned j = 0; j < GAME_TILE_COUNT; j++) {
        if (j == (unsigned)i || g->removed[j]) {
            continue;
        }
        const TurtleSlot *tj = &kTurtleLayout[j];
        if (tj->z < ti->z) {
            continue;
        }
        if (!game_y_overlap_slots(ti, tj)) {
            continue;
        }
        if (tj->x <= ti->x + kTileFootprint && tj->x + kTileFootprint > ti->x + kTileFootprint) {
            return 1;
        }
    }
    return 0;
}

int game_tile_is_free(const GameState *g, int idx) {
    if (!g || idx < 0 || (unsigned)idx >= GAME_TILE_COUNT || g->removed[idx]) {
        return 0;
    }
    if (game_covered_from_above(g, idx)) {
        return 0;
    }
    return !game_left_blocked(g, idx) || !game_right_blocked(g, idx);
}

int game_hit_test(const GameState *g, int x, int y) {
    if (!g) {
        return -1;
    }
    for (int z = 4; z >= 0; z--) {
        for (unsigned i = 0; i < GAME_TILE_COUNT; i++) {
            if (g->removed[i]) {
                continue;
            }
            if (kTurtleLayout[i].z != z) {
                continue;
            }
            int tx = 0;
            int ty = 0;
            game_slot_to_screen((int)i, &tx, &ty);
            if (x >= tx && x < tx + kTileDrawW && y >= ty && y < ty + kTileDrawH) {
                return (int)i;
            }
        }
    }
    return -1;
}

static int cmp_draw_order(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    const TurtleSlot *sa = &kTurtleLayout[ia];
    const TurtleSlot *sb = &kTurtleLayout[ib];
    if (sa->z != sb->z) {
        return (sa->z > sb->z) - (sa->z < sb->z);
    }
    if (sa->y != sb->y) {
        return (sa->y > sb->y) - (sa->y < sb->y);
    }
    if (sa->x != sb->x) {
        return (sa->x > sb->x) - (sa->x < sb->x);
    }
    return (ia > ib) - (ia < ib);
}

int game_draw_order(const GameState *g, int *out_order, int max_order) {
    int n = 0;
    if (!g || !out_order || max_order <= 0) {
        return 0;
    }
    for (unsigned i = 0; i < GAME_TILE_COUNT; i++) {
        if (!g->removed[i]) {
            if (n >= max_order) {
                break;
            }
            out_order[n++] = (int)i;
        }
    }
    qsort(out_order, (size_t)n, sizeof(out_order[0]), cmp_draw_order);
    return n;
}

static unsigned game_rng_mix(unsigned *state) {
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

void game_new(GameState *g) {
    if (!g) {
        return;
    }
    memset(g, 0, sizeof(*g));
    g->selected = -1;

    unsigned char deck[GAME_TILE_COUNT];
    int k = 0;
    for (int copy = 0; copy < 4; copy++) {
        for (unsigned f = 0; f < 27u; f++) {
            deck[k++] = (unsigned char)f;
        }
    }
    for (int copy = 0; copy < 4; copy++) {
        for (unsigned f = 27u; f < TILE_BOUNDS_COUNT; f++) {
            deck[k++] = (unsigned char)f;
        }
    }
    while (k < (int)GAME_TILE_COUNT) {
        unsigned extra = (unsigned)(k - 136);
        deck[k++] = (unsigned char)(27u + extra % (TILE_BOUNDS_COUNT - 27u));
    }

    unsigned rng = (unsigned)GetTickCount64() ^ 0x9E3779B9u;
    for (int i = (int)GAME_TILE_COUNT - 1; i > 0; i--) {
        unsigned r = game_rng_mix(&rng);
        int j = (int)(r % (unsigned)(i + 1));
        unsigned char t = deck[i];
        deck[i] = deck[j];
        deck[j] = t;
    }

    memcpy(g->face, deck, sizeof(g->face));
}

int game_on_tile_click(GameState *g, int idx) {
    if (!g || idx < 0 || (unsigned)idx >= GAME_TILE_COUNT || g->removed[idx]) {
        return 0;
    }
    if (!game_tile_is_free(g, idx)) {
        return 0;
    }

    if (g->selected < 0) {
        g->selected = idx;
        return 0;
    }

    if (g->selected == idx) {
        g->selected = -1;
        return 0;
    }

    int a = g->selected;
    if (g->face[a] == g->face[idx] && game_tile_is_free(g, a) && game_tile_is_free(g, idx)) {
        g->removed[a] = 1;
        g->removed[idx] = 1;
        g->selected = -1;
        return 1;
    }

    g->selected = idx;
    return 0;
}
