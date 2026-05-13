#ifndef COBJMACROS
#define COBJMACROS
#endif

#include <string.h>

#include <windows.h>
#include <ole2.h>
#include <objidl.h>
#include <gdiplus/gdiplus.h>

#include "load_tiles.h"
#include "resource.h"

#ifndef PropertyTagFrameDelay
#define PropertyTagFrameDelay 0x5100
#endif

static const GUID kFrameDimensionTime = {
    0x6aedbd6d,
    0x3fb5,
    0x418a,
    {0x83, 0xa6, 0x7f, 0x45, 0x22, 0x9d, 0xc8, 0x72},
};

struct Tiles {
    GpBitmap *bitmap;
    UINT frameCount;
    ULONG *frameDelayCs;
};

static ULONG_PTR s_gdiplusToken;
static int s_oleBalanced;

int tiles_runtime_begin(void) {
    HRESULT hrOle = OleInitialize(NULL);
    if (FAILED(hrOle) && hrOle != RPC_E_CHANGED_MODE) {
        return TILES_ERR_OLE;
    }
    s_oleBalanced = (hrOle == S_OK || hrOle == S_FALSE);

    GdiplusStartupInput input = {1, NULL, FALSE, FALSE};
    if (GdiplusStartup(&s_gdiplusToken, &input, NULL) != Ok) {
        if (s_oleBalanced) {
            OleUninitialize();
        }
        s_oleBalanced = 0;
        return TILES_ERR_GDIPLUS;
    }

    return 0;
}

void tiles_runtime_end(void) {
    GdiplusShutdown(s_gdiplusToken);
    if (s_oleBalanced) {
        OleUninitialize();
    }
    s_oleBalanced = 0;
}

void tiles_free(Tiles *t) {
    if (!t) {
        return;
    }
    if (t->bitmap) {
        GdipDisposeImage((GpImage *)t->bitmap);
        t->bitmap = NULL;
    }
    HeapFree(GetProcessHeap(), 0, t->frameDelayCs);
    t->frameDelayCs = NULL;
    t->frameCount = 0;
    HeapFree(GetProcessHeap(), 0, t);
}

UINT tiles_frame_count(const Tiles *t) {
    return t ? t->frameCount : 0;
}

UINT tiles_frame_delay_ms(const Tiles *t, UINT frame_index) {
    ULONG cs = 10;
    if (t && t->frameDelayCs && frame_index < t->frameCount) {
        cs = t->frameDelayCs[frame_index];
    }
    if (cs == 0) {
        cs = 10;
    }
    if (cs > 60000) {
        cs = 60000;
    }
    return (UINT)(cs * 10u);
}

int tiles_set_frame(Tiles *t, UINT frame_index) {
    if (!t || !t->bitmap || t->frameCount == 0 || frame_index >= t->frameCount) {
        return 0;
    }
    return GdipImageSelectActiveFrame((GpImage *)t->bitmap, &kFrameDimensionTime, frame_index) == Ok;
}

void tiles_pixel_size(const Tiles *t, UINT *out_width, UINT *out_height) {
    UINT w = 0, h = 0;
    if (t && t->bitmap) {
        GdipGetImageWidth((GpImage *)t->bitmap, &w);
        GdipGetImageHeight((GpImage *)t->bitmap, &h);
    }
    if (out_width) {
        *out_width = w;
    }
    if (out_height) {
        *out_height = h;
    }
}

void tiles_draw_stretched(HDC hdc, const Tiles *t, int x, int y, int width, int height) {
    if (!hdc || !t || !t->bitmap || width <= 0 || height <= 0) {
        return;
    }

    GpGraphics *gfx = NULL;
    if (GdipCreateFromHDC(hdc, &gfx) != Ok) {
        return;
    }
    GdipGraphicsClear(gfx, 0xFF404040);
    GdipDrawImageRectI(gfx, t->bitmap, x, y, width, height);
    GdipDeleteGraphics(gfx);
}

Tiles *tiles_load_embedded_gif(HINSTANCE inst) {
    HRSRC res = FindResourceA(inst, MAKEINTRESOURCEA(IDR_MAHJONG_TILES_GIF), RT_RCDATA);
    if (!res) {
        return NULL;
    }

    DWORD size = SizeofResource(inst, res);
    HGLOBAL hglob = LoadResource(inst, res);
    if (!hglob || size == 0) {
        return NULL;
    }

    void *bits = LockResource(hglob);
    if (!bits) {
        return NULL;
    }

    HGLOBAL copy = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!copy) {
        return NULL;
    }

    void *locked = GlobalLock(copy);
    if (!locked) {
        GlobalFree(copy);
        return NULL;
    }
    memcpy(locked, bits, size);
    GlobalUnlock(copy);

    IStream *stream = NULL;
    if (CreateStreamOnHGlobal(copy, TRUE, &stream) != S_OK) {
        GlobalFree(copy);
        return NULL;
    }

    GpBitmap *bitmap = NULL;
    if (GdipCreateBitmapFromStream(stream, &bitmap) != Ok) {
        IStream_Release(stream);
        return NULL;
    }
    IStream_Release(stream);

    UINT frameCount = 0;
    if (GdipImageGetFrameCount((GpImage *)bitmap, &kFrameDimensionTime, &frameCount) != Ok || frameCount == 0) {
        GdipDisposeImage((GpImage *)bitmap);
        return NULL;
    }

    Tiles *t = (Tiles *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Tiles));
    if (!t) {
        GdipDisposeImage((GpImage *)bitmap);
        return NULL;
    }

    ULONG *delays = (ULONG *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (SIZE_T)frameCount * sizeof(ULONG));
    if (!delays) {
        GdipDisposeImage((GpImage *)bitmap);
        HeapFree(GetProcessHeap(), 0, t);
        return NULL;
    }

    UINT propSize = 0;
    if (GdipGetPropertyItemSize((GpImage *)bitmap, PropertyTagFrameDelay, &propSize) == Ok && propSize > 0) {
        PropertyItem *prop = (PropertyItem *)HeapAlloc(GetProcessHeap(), 0, propSize);
        if (prop && GdipGetPropertyItem((GpImage *)bitmap, PropertyTagFrameDelay, propSize, prop) == Ok) {
            if (prop->type == PropertyTagTypeLong && prop->length >= sizeof(LONG) && prop->value) {
                UINT n = prop->length / (UINT)sizeof(LONG);
                LONG *v = (LONG *)prop->value;
                for (UINT i = 0; i < frameCount; i++) {
                    delays[i] = (ULONG)(i < n ? v[i] : 10);
                }
            }
        }
        HeapFree(GetProcessHeap(), 0, prop);
    }

    t->bitmap = bitmap;
    t->frameCount = frameCount;
    t->frameDelayCs = delays;

    if (GdipImageSelectActiveFrame((GpImage *)bitmap, &kFrameDimensionTime, 0) != Ok) {
        tiles_free(t);
        return NULL;
    }

    return t;
}
