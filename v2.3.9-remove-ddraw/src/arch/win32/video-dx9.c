/*
 * video-dx9.c - Video using DirectX9 for Win32
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include "fullscrn.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "raster.h"
#include "resources.h"
#include "ui.h"
#include "video.h"
#include "video-dx9.h"
#include "video-render.h"
#include "videoarch.h"
#include "viewport.h"

#ifdef HAVE_D3D9_H

#include <d3d9.h>

typedef IDirect3D9 *(WINAPI* Direct3DCreate9_t)(UINT SDKVersion);

static Direct3DCreate9_t DynDirect3DCreate9;
static HINSTANCE d3d9dll = NULL;

static D3DTEXTUREFILTERTYPE d3dpreffilter;

LPDIRECT3D9 d3d;

int video_setup_dx9(void)
{
    d3d9dll = LoadLibrary("d3d9.dll");
    if (d3d9dll == NULL) {
        return -1;
    }
    DynDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(d3d9dll, "Direct3DCreate9");
    if (!DynDirect3DCreate9) {
        return -1;
    }
    d3d = DynDirect3DCreate9(D3D_SDK_VERSION);

    return 0;
}

void video_shutdown_dx9(void)
{
    if (d3d != NULL) {
        IDirect3D9_Release(d3d);
        d3d = NULL;
    }
}

int video_device_create_dx9(raster_t *raster, int fullscreen)
{
    int device = D3DADAPTER_DEFAULT;

    ZeroMemory(&raster->canvas->d3dpp, sizeof(raster->canvas->d3dpp));
    raster->canvas->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    raster->canvas->d3dpp.BackBufferCount = 1;
    raster->canvas->d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    raster->canvas->d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    raster->canvas->d3dpp.Flags = 0;
    raster->canvas->d3dpp.EnableAutoDepthStencil = FALSE;
    raster->canvas->d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    if (dx_primary_surface_rendering) {
        raster->canvas->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    } else {
        raster->canvas->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }

    if (fullscreen) {
        int width, height, bitdepth, refreshrate;
        int keep_aspect_ratio, true_aspect_ratio, aspect_ratio;
        int shrinked_width, shrinked_height;
        double canvas_aspect_ratio;
  
        GetCurrentModeParameters(&device, &width, &height, &bitdepth, &refreshrate);

        resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
        if (keep_aspect_ratio == 0) {
            raster->canvas->dest_rect_ptr = NULL;
        } else {
            raster->canvas->dest_rect_ptr = &raster->canvas->dest_rect;
            resources_get_int("TrueAspectRatio", &true_aspect_ratio);
            if (true_aspect_ratio) {
                aspect_ratio = (int)(raster->geometry->pixel_aspect_ratio * 1000);
            } else {
                resources_get_int("AspectRatio", &aspect_ratio);
            }
            canvas_aspect_ratio = aspect_ratio / 1000.0 * raster->canvas->width / raster->canvas->height;
            canvas_aspect_ratio = aspect_ratio / 1000.0 
                                    * raster->canvas->width / raster->canvas->height;
            if (canvas_aspect_ratio < (double) width / height) {
                shrinked_width = (int)(height * canvas_aspect_ratio);
                raster->canvas->dest_rect.top = 0;
                raster->canvas->dest_rect.bottom = height - 1;
                raster->canvas->dest_rect.left = (width - shrinked_width) / 2;
                raster->canvas->dest_rect.right = raster->canvas->dest_rect.left + shrinked_width - 1;
            } else {
                shrinked_height = (int)(width / canvas_aspect_ratio);
                raster->canvas->dest_rect.left = 0;
                raster->canvas->dest_rect.right = width - 1;
                raster->canvas->dest_rect.top = (height - shrinked_height) / 2;
                raster->canvas->dest_rect.bottom = raster->canvas->dest_rect.top + shrinked_height - 1;
            }
        }
        raster->canvas->d3dpp.Windowed = FALSE;
        raster->canvas->d3dpp.BackBufferWidth = width;
        raster->canvas->d3dpp.BackBufferHeight = height;
    } else {
        raster->canvas->dest_rect_ptr = NULL;
        raster->canvas->d3dpp.Windowed = TRUE;
        raster->canvas->d3dpp.BackBufferWidth = raster->canvas->width;
        raster->canvas->d3dpp.BackBufferHeight = raster->canvas->height;
    }

    if (S_OK != IDirect3D9_CreateDevice(d3d, device, D3DDEVTYPE_HAL, raster->canvas->render_hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &raster->canvas->d3dpp, &raster->canvas->d3ddev)) {
        log_debug("video_dx9: Failed to create the DirectX9 device!");
        return -1;
    }


    if (S_OK != IDirect3DDevice9_CreateOffscreenPlainSurface(raster->canvas->d3ddev, raster->canvas->width, raster->canvas->height, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &raster->canvas->d3dsurface, NULL)) {
        log_debug("video_dx9: Failed to create the offscreen surface!");
        return -1;
    }

    d3dpreffilter = D3DTEXF_LINEAR;

    return 0;
}

video_canvas_t *video_canvas_create_dx9(raster_t *raster, unsigned int *width, unsigned int *height)
{
    ui_make_resizable(raster->canvas, 1);

    raster->canvas->depth = 32;

    if  (video_device_create_dx9(raster, 0) != 0) {
        return NULL;
    }

    if (video_set_physical_colors(raster->canvas, raster->palette, &raster->videoconfig->color_tables) < 0) {
        return NULL;
    }

    video_canvas_add(raster);

    return raster->canvas;
}

void video_device_release_dx9(video_canvas_t *canvas)
{
    if (canvas->d3dsurface) {
        IDirect3DSurface9_Release(canvas->d3dsurface);
        canvas->d3dsurface = NULL;
    }
    if (canvas->d3ddev) { 
        IDirect3DDevice9_Release(canvas->d3ddev);
        canvas->d3ddev = NULL;
    }
}

HRESULT video_canvas_reset_dx9(video_canvas_t *canvas)
{
    LPDIRECT3DSWAPCHAIN9 d3dsc;
    HRESULT ddresult;

    if (canvas->d3ddev == NULL) {
        log_debug("video_dx9: Cannot reset canvas. Invalid D3D objects.");
        return -1;
    }

    if ((canvas->d3dsurface != NULL) 
            && (S_OK != IDirect3DSurface9_Release(canvas->d3dsurface))
        || S_OK != IDirect3DDevice9_GetSwapChain(canvas->d3ddev, 0, &d3dsc)
        || S_OK != IDirect3DSwapChain9_Release(d3dsc))
    {
        log_debug("video_dx9: Failed to release the DirectX9 device resources!");
    }
    
    canvas->d3dsurface = NULL;

    if (canvas->d3dpp.Windowed == 0) {
        int device, width, height, bitdepth, refreshrate;

        GetCurrentModeParameters(&device, &width, &height, &bitdepth, &refreshrate);
        canvas->d3dpp.BackBufferWidth = width;
        canvas->d3dpp.BackBufferHeight = height;
    } else {
        canvas->d3dpp.BackBufferWidth = canvas->width;
        canvas->d3dpp.BackBufferHeight = canvas->height;
    }

    if (dx_primary_surface_rendering) {
        canvas->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    } else {
        canvas->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }

    if (S_OK != (ddresult = IDirect3DDevice9_Reset(
                                canvas->d3ddev,
                                &canvas->d3dpp)))
    {
        log_debug("video_dx9: Failed to reset the DirectX9 device!");
    }
    
    if (S_OK != (ddresult = IDirect3DDevice9_CreateOffscreenPlainSurface(
                                canvas->d3ddev, canvas->width, canvas->height,
                                D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                                &canvas->d3dsurface, NULL)))
    {
        log_debug("video_dx9: Failed to create new offscreen surface!");
        return ddresult;
    }

    return IDirect3DDevice9_TestCooperativeLevel(canvas->d3ddev);
}

static HRESULT video_canvas_prepare_for_update(video_canvas_t *canvas)
{
    HRESULT coopresult;

    if (canvas->d3ddev == NULL || canvas->d3dsurface == NULL) {
        return -1;
    }

    coopresult = IDirect3DDevice9_TestCooperativeLevel(canvas->d3ddev);
    if (coopresult == D3DERR_DEVICENOTRESET) {
        coopresult = video_canvas_reset_dx9(canvas);
    }
    return coopresult;
}


int video_canvas_refresh_dx9(raster_t *raster, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    HRESULT stretchresult;
    LPDIRECT3DSURFACE9 d3dbackbuffer = NULL;
    D3DLOCKED_RECT lockedrect;

    if (raster->videoconfig->doublesizex) {
        xi *= 2;
        w *= 2;
    }

    if (raster->videoconfig->doublesizey) {
        yi *= 2;
        h *= 2;
    }
    
    if (S_OK != video_canvas_prepare_for_update(raster->canvas)) {
        return -1;
    }

    if (S_OK != IDirect3DDevice9_Clear(raster->canvas->d3ddev, 0, NULL, D3DCLEAR_TARGET, 0, 0, 0) ||
        S_OK != IDirect3DDevice9_BeginScene(raster->canvas->d3ddev) ||
        S_OK != IDirect3DDevice9_GetBackBuffer(raster->canvas->d3ddev, 0, 0, D3DBACKBUFFER_TYPE_MONO, &d3dbackbuffer) ||
        S_OK != IDirect3DSurface9_LockRect(raster->canvas->d3dsurface, &lockedrect, NULL, 0)) {
        log_debug("video_dx9: Failed to prepare for rendering!");
        return -1;
    }

    video_render_main(raster->videoconfig, raster->draw_buffer->draw_buffer,
      lockedrect.pBits, w, h, xs, ys, xi, yi,
      raster->draw_buffer->draw_buffer_width, lockedrect.Pitch, 32,
      raster->viewport);

    if (S_OK != IDirect3DSurface9_UnlockRect(raster->canvas->d3dsurface)) {
        log_debug("video_dx9: Failed to unlock surface!");
        return -1;
    }

    do {
        stretchresult = IDirect3DDevice9_StretchRect(raster->canvas->d3ddev, raster->canvas->d3dsurface, NULL, d3dbackbuffer, raster->canvas->dest_rect_ptr, d3dpreffilter);

        if (d3dpreffilter == D3DTEXF_NONE) {
            break;
        }

        if (stretchresult != S_OK) {
            /* Some adapters don't support filtering */
            d3dpreffilter = D3DTEXF_NONE;
            log_debug("video_dx9: Disabled StretchRect filtering!");
        }

    } while (stretchresult != S_OK);
    if (stretchresult != S_OK) {
        log_debug("video_dx9: StretchRect failed even without filtering!");
    }

    if (S_OK != IDirect3DSurface9_Release(d3dbackbuffer) 
        || S_OK != IDirect3DDevice9_EndScene(raster->canvas->d3ddev))
    {
        log_debug("video_dx9: EndScene failed!");
        return -1;
    }

    if (S_OK != IDirect3DDevice9_Present(raster->canvas->d3ddev, NULL, NULL, NULL, NULL)) {
        log_debug("video_dx9: Refresh failed to present the scene!");
        return -1;
    }
    return 0;
}

void video_canvas_update_dx9(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h)
{
    raster_t *raster = video_canvas_for_hwnd(hwnd);
    if (raster == NULL) {
        return;
    }
    
    /* Just refresh the whole canvas */
    raster_refresh_all(raster);
}

#endif
