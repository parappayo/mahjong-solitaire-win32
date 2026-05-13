#include <windows.h>

#include "game.h"
#include "load_tiles.h"

#define TIMER_GIF_FRAME 1

static const char kWindowClass[] = "MahjongSolitaireWinClass";

typedef struct {
    Tiles *tiles;
    UINT currentFrame;
    GameState game;
} AppState;

static HINSTANCE g_hInstance;

static void App_ScheduleNextFrame(HWND hwnd, AppState *s) {
    if (!s->tiles || tiles_frame_count(s->tiles) <= 1) {
        return;
    }
    UINT ms = tiles_frame_delay_ms(s->tiles, s->currentFrame);
    SetTimer(hwnd, TIMER_GIF_FRAME, ms, NULL);
}

static void App_OnTimer(HWND hwnd, AppState *s) {
    if (!s->tiles || tiles_frame_count(s->tiles) <= 1) {
        return;
    }

    UINT n = tiles_frame_count(s->tiles);
    s->currentFrame = (s->currentFrame + 1) % n;
    if (!tiles_set_frame(s->tiles, s->currentFrame)) {
        return;
    }

    InvalidateRect(hwnd, NULL, FALSE);
    KillTimer(hwnd, TIMER_GIF_FRAME);
    App_ScheduleNextFrame(hwnd, s);
}

static void App_DrawSelection(HWND hwnd, HDC hdc, const GameState *g) {
    (void)hwnd;
    if (!g || g->selected < 0) {
        return;
    }

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    game_tile_screen_rect(g->selected, &x, &y, &w, &h);

    HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 220, 0));
    if (!pen) {
        return;
    }
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    SetBkMode(hdc, TRANSPARENT);
    Rectangle(hdc, x, y, x + w + 1, y + h + 1);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppState *state = (AppState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        (void)lParam;

        AppState *s = (AppState *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(AppState));
        if (!s) {
            return -1;
        }

        s->tiles = tiles_load_embedded_gif(g_hInstance);
        if (!s->tiles) {
            HeapFree(GetProcessHeap(), 0, s);
            MessageBoxA(hwnd, "Could not load embedded GIF resource.", "Mahjong Solitaire", MB_OK | MB_ICONERROR);
            return -1;
        }

        game_new(&s->game);

        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)s);

        int cw = 0;
        int ch = 0;
        game_board_client_size(&cw, &ch);

        RECT rc = {0, 0, cw, ch};
        AdjustWindowRectEx(&rc, (DWORD)GetWindowLongPtrA(hwnd, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtrA(hwnd, GWL_EXSTYLE));
        int winW = rc.right - rc.left;
        int winH = rc.bottom - rc.top;
        SetWindowPos(hwnd, NULL, 0, 0, winW, winH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        App_ScheduleNextFrame(hwnd, s);
        return 0;
    }

    case WM_TIMER:
        if (wParam == TIMER_GIF_FRAME && state) {
            App_OnTimer(hwnd, state);
        }
        return 0;

    case WM_LBUTTONDOWN: {
        if (!state) {
            return 0;
        }
        int mx = (int)(short)LOWORD(lParam);
        int my = (int)(short)HIWORD(lParam);
        int idx = game_hit_test(&state->game, mx, my);
        if (idx >= 0) {
            (void)game_on_tile_click(&state->game, idx);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (state && state->tiles) {
            int order[GAME_TILE_COUNT];
            int n = game_draw_order(&state->game, order, GAME_TILE_COUNT);
            TilesDrawCmd cmds[GAME_TILE_COUNT];
            int c = 0;
            for (int i = 0; i < n; i++) {
                int idx = order[i];
                int x = 0;
                int y = 0;
                int w = 0;
                int h = 0;
                game_tile_screen_rect(idx, &x, &y, &w, &h);
                cmds[c].tile_index = state->game.face[idx];
                cmds[c].x = x;
                cmds[c].y = y;
                cmds[c].w = w;
                cmds[c].h = h;
                c++;
            }
            tiles_draw_commands(hdc, state->tiles, cmds, (size_t)c);
            App_DrawSelection(hwnd, hdc, &state->game);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_GIF_FRAME);
        if (state) {
            tiles_free(state->tiles);
            HeapFree(GetProcessHeap(), 0, state);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, 0);
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    g_hInstance = hInstance;

    int rt = tiles_runtime_begin();
    if (rt != 0) {
        if (rt == TILES_ERR_OLE) {
            MessageBoxA(NULL, "OleInitialize failed.", "Error", MB_OK | MB_ICONERROR);
        } else {
            MessageBoxA(NULL, "GdiplusStartup failed.", "Error", MB_OK | MB_ICONERROR);
        }
        return 1;
    }

    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kWindowClass;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        tiles_runtime_end();
        MessageBoxA(NULL, "RegisterClass failed.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(
        0,
        kWindowClass,
        "Mahjong Solitaire",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd) {
        tiles_runtime_end();
        MessageBoxA(NULL, "CreateWindowEx failed.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    tiles_runtime_end();
    return (int)msg.wParam;
}
