#include <windows.h>

#include "load_tiles.h"

#define TIMER_GIF_FRAME 1

static const char kWindowClass[] = "MahjongSolitaireWinClass";

typedef struct {
    Tiles *tiles;
    UINT currentFrame;
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

        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)s);

        UINT imgW = 0, imgH = 0;
        tiles_pixel_size(s->tiles, &imgW, &imgH);

        RECT rc = {0, 0, (LONG)imgW, (LONG)imgH};
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

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (state && state->tiles) {
            RECT cr;
            GetClientRect(hwnd, &cr);
            int cw = cr.right - cr.left;
            int ch = cr.bottom - cr.top;
            tiles_draw_stretched(hdc, state->tiles, 0, 0, cw, ch);
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
