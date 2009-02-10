/*
 * fullscrn-dx9.c - Fullscreen using DirectX9 for Win32
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

#include <d3d9.h>
#include <windows.h>

#include "fullscrn.h"
#include "lib.h"
#include "statusbar.h"
#include "ui.h"
#include "videoarch.h"


static HMENU   old_menu;
static RECT    old_rect;
static DWORD   old_style;
static int     old_width;
static int     old_height;
static int     old_bitdepth;
static int     old_client_width;
static int     old_client_height;
static float   old_refreshrate;


void fullscreen_getmodes_dx9(void)
{
    int adapter, numAdapter, mode, numAdapterModes;
    D3DADAPTER_IDENTIFIER9 d3didentifier;
    D3DDISPLAYMODE displayMode;
    DirectDrawDeviceList *new_device;
    DirectDrawDeviceList *search_device;
    DirectDrawModeList *new_mode;
    DirectDrawModeList *search_mode;

    numAdapter = 0;
    while (D3D_OK == IDirect3D9_GetAdapterIdentifier(d3d, numAdapter, 0, &d3didentifier))
    {
        new_device = lib_malloc(sizeof(DirectDrawDeviceList));
        new_device->next = NULL;
        new_device->desc = lib_stralloc(d3didentifier.Description);

        if (devices == NULL) {
            devices = new_device;
        } else {
            search_device = devices;
            while (search_device->next != NULL) {
                search_device = search_device->next;
            }
            search_device->next = new_device;
        }
        numAdapter++;
    }
    
    for (adapter = 0; adapter < numAdapter; adapter++)
    {
        numAdapterModes = IDirect3D9_GetAdapterModeCount(d3d, adapter, D3DFMT_X8R8G8B8);
    
        for (mode = 0; mode < numAdapterModes; mode++) {
            if (S_OK == IDirect3D9_EnumAdapterModes(d3d, D3DADAPTER_DEFAULT, 
                                                    D3DFMT_X8R8G8B8, mode, &displayMode))
            {
                new_mode = lib_malloc(sizeof(DirectDrawModeList));
                new_mode->next = NULL;
                new_mode->devicenumber = adapter;
                new_mode->width = displayMode.Width;
                new_mode->height = displayMode.Height;
                new_mode->bitdepth = 32;
                new_mode->refreshrate = displayMode.RefreshRate;

                if (modes == NULL) {
                    modes = new_mode;
                } else {
                    search_mode = modes;
                    while (search_mode->next != NULL) {
                        search_mode = search_mode->next;
                    }
                    search_mode->next = new_mode;
                }
            }
        }
    }


#if 0
    /*  Enumerate DirectDraw Devices */
    ddresult = DirectDrawEnumerate(DDEnumCallbackFunction, NULL);

    /*  List all available modes for all available devices */
    search_device = devices;
    i = 0;
    while (search_device != NULL) {
//        log_debug("--- Video modes for device %s", search_device->desc);
//        log_debug("MODEPROBE_Create");
        if (search_device->isNullGUID) {
            ddresult = DirectDrawCreate(NULL, &DirectDrawObject, NULL);
        } else {
            ddresult = DirectDrawCreate(&search_device->guid,
                                        &DirectDrawObject, NULL);
        }
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_SetCooperativeLevel");
        ddresult = IDirectDraw_SetCooperativeLevel(DirectDrawObject,
                                                   ui_get_main_hwnd(),
                                                   DDSCL_EXCLUSIVE
                                                   | DDSCL_FULLSCREEN);
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_ObtainDirectDraw2");
        ddresult = IDirectDraw_QueryInterface(DirectDrawObject,
                                              (GUID *)&IID_IDirectDraw2,
                                              (LPVOID *)&DirectDrawObject2);
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_EnumDisplayModes");
        ddresult = IDirectDraw2_EnumDisplayModes(DirectDrawObject2,
                                                 DDEDM_REFRESHRATES, NULL, &i,
                                                 ModeCallBack);
        CHECK_DDRESULT(ddresult);
        IDirectDraw2_Release(DirectDrawObject2);
        DirectDrawObject2 = NULL;
        IDirectDraw_Release(DirectDrawObject);
        DirectDrawObject = NULL;
        search_device = search_device->next;
        i++;
    }

    /*  This is here because some Matrox G200 drivers don't leave the window
        in its previous state */
    ShowWindow(ui_get_main_hwnd(), SW_HIDE);
#endif
}


void SwitchToFullscreenModeDx9(HWND hwnd)
{
    int w, h, wnow, hnow;
    int fullscreen_width;
    int fullscreen_height;
    int bitdepth;
    video_canvas_t *c;

    fullscreen_transition = 1;

    //  Get the Canvas for this window
    c = video_canvas_for_hwnd(hwnd);

#if 0
    //  Get fullscreen parameters
    GetCurrentModeParameters(&fullscreen_width, &fullscreen_height, &bitdepth,
                             &refreshrate);
    memset(&desc2, 0, sizeof(desc2));
    desc2.dwSize = sizeof(desc2);
    ddresult = IDirectDraw2_GetDisplayMode(c->dd_object2,&desc2);
    old_width = desc2.dwWidth;
    old_height = desc2.dwHeight;
#ifdef _ANONYMOUS_UNION
    old_bitdepth = desc2.ddpfPixelFormat.dwRGBBitCount;;
#else
    old_bitdepth = desc2.ddpfPixelFormat.u1.dwRGBBitCount;;
#endif
    old_refreshrate = c->refreshrate; /* save this, because recalculating takes time */

    IDirectDrawSurface_Release(c->temporary_surface);
    IDirectDrawSurface_Release(c->primary_surface);
    IDirectDraw_Release(c->dd_object2);
    IDirectDraw_Release(c->dd_object);
#endif

    log_debug("switch to fullscreen");
    log_debug("releasing device...");
    video_device_release_dx9(c);

    statusbar_destroy();

    //  Remove Window Styles
    old_style = GetWindowLong(hwnd, GWL_STYLE);
    GetWindowRect(hwnd, &old_rect);
    SetWindowLong(hwnd, GWL_STYLE, old_style & ~WS_SYSMENU & ~WS_CAPTION);
    //  Remove Menu
    old_menu = GetMenu(hwnd);
    SetMenu(hwnd, NULL);
    //  Cover screen with window
    wnow = GetSystemMetrics(SM_CXSCREEN);
    hnow = GetSystemMetrics(SM_CYSCREEN);
    w = (fullscreen_width > wnow) ? fullscreen_width : wnow;
    h = (fullscreen_height > hnow) ? fullscreen_height : hnow;
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, w, h, SWP_NOCOPYBITS);
    ShowCursor(FALSE);

    log_debug("opening new render window...");
    ui_set_render_window(c, 1);
    log_debug("creating device...");
    video_device_create_dx9(c, 1);
    log_debug("switch to fullscreen done");
	video_canvas_refresh_all(c);

#if 0
    device_guid = GetGUIDForActualDevice();
    ddresult = DirectDrawCreate(device_guid, &c->dd_object, NULL);
    ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, c->hwnd,
                                               DDSCL_EXCLUSIVE
                                               | DDSCL_FULLSCREEN);
    ddresult = IDirectDraw_QueryInterface(c->dd_object,
                                          (GUID *)&IID_IDirectDraw2,
                                          (LPVOID *)&c->dd_object2);

    //  Set cooperative level
    ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, c->hwnd,
                                               DDSCL_EXCLUSIVE
                                               | DDSCL_FULLSCREEN);
    //  Set Mode
    ddresult = IDirectDraw2_SetDisplayMode(c->dd_object2, fullscreen_width,
                                           fullscreen_height, bitdepth,
                                           refreshrate,0);
#endif
    //  Adjust window size
    old_client_width = c->client_width;
    old_client_height = c->client_height;
    c->client_width = fullscreen_width;
    c->client_height = fullscreen_height;

#if 0
    if (fullscreen_refreshrate_buffer < 0.0f) {
        /* if no refreshrate is buffered, recalculate (1 second) */
        for (i = 0; i < 50; i++)
            IDirectDraw2_WaitForVerticalBlank(c->dd_object2,
                                              DDWAITVB_BLOCKBEGIN, 0);
        c->refreshrate = video_refresh_rate(c);
        fullscreen_refreshrate_buffer = c->refreshrate;
    } else {
        c->refreshrate = fullscreen_refreshrate_buffer;
    }

    video_create_single_surface(c, fullscreen_width, fullscreen_height);

    c->depth = bitdepth;

    /* Create palette.  */
    if (c->depth == 8) {
        PALETTEENTRY ape[256];
        HRESULT result;

        init_palette(c->palette, ape);

        result = IDirectDraw2_CreatePalette(c->dd_object2, DDPCAPS_8BIT,
                                            ape, &c->dd_palette, NULL);
        if (result != DD_OK) {
        }
    }

    video_set_palette(c);
    video_set_physical_colors(c);

    IDirectDrawSurface_GetDC(c->primary_surface, &hdc);
    video_canvas_update(c->hwnd, hdc, 0, 0, fullscreen_width,
                        fullscreen_height);
    IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);
#endif

    fullscreen_active = 1;

    fullscreen_transition = 0;
}


void SwitchToWindowedModeDx9(HWND hwnd)
{
    video_canvas_t *c;
    HRESULT ddresult;

    fullscreen_transition = 1;

    //  Get the Canvas for this window
    c = video_canvas_for_hwnd(hwnd);

#if 0
    IDirectDrawSurface_Release(c->temporary_surface);
    IDirectDrawSurface_Release(c->primary_surface);
    ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, NULL,
                                               DDSCL_NORMAL);
    IDirectDraw_RestoreDisplayMode(c->dd_object);
    IDirectDraw_Release(c->dd_object2);
    IDirectDraw_Release(c->dd_object);
#endif

    log_debug("switch to window");
    log_debug("releasing device...");
    video_device_release_dx9(c);
    log_debug("opening new render window...");
    /* Create statusbar here go get correct dimensions for client window */
    statusbar_create(hwnd);
    ui_set_render_window(c, 0);


    LockWindowUpdate(hwnd);
    SetWindowLong(hwnd, GWL_STYLE, old_style);
    //  Remove Menu
    SetMenu(hwnd,old_menu);
    SetWindowPos(hwnd, HWND_TOP, old_rect.left, old_rect.top,
                 old_rect.right - old_rect.left, old_rect.bottom - old_rect.top,
                 SWP_NOCOPYBITS);
    ShowCursor(TRUE);
    c->client_width = old_client_width;
    c->client_height = old_client_height;
    LockWindowUpdate(NULL);


    //ui_resize_canvas_window(c);

    log_debug("creating device...");
    video_device_create_dx9(c, 0);
    log_debug("switch to window done");
	video_canvas_refresh_all(c);

#if 0
    ddresult = DirectDrawCreate(NULL, &c->dd_object, NULL);
    ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, NULL,
                                               DDSCL_NORMAL);
    ddresult = IDirectDraw_QueryInterface(c->dd_object,
                                          (GUID *)&IID_IDirectDraw2,
                                          (LPVOID *)&c->dd_object2);

    memset(&desc2,0,sizeof(desc2));
    desc2.dwSize = sizeof(desc2);
    ddresult = IDirectDraw2_GetDisplayMode(c->dd_object2, &desc2);

    video_create_single_surface(c, desc2.dwWidth, desc2.dwHeight);

    c->depth = old_bitdepth;


    /* Create palette.  */
    if (c->depth == 8) {
        PALETTEENTRY ape[256];
        HRESULT result;

        init_palette(c->palette, ape);

        result = IDirectDraw2_CreatePalette(c->dd_object2, DDPCAPS_8BIT,
                                            ape, &c->dd_palette, NULL);
        if (result != DD_OK) {
        }
    }

    video_set_palette(c);
    video_set_physical_colors(c);


    IDirectDrawSurface_GetDC(c->primary_surface, &hdc);
    video_canvas_update(c->hwnd, hdc, 0, 0, c->client_width, c->client_height);
    IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);
    fullscreen_active = 0;
#endif

    fullscreen_transition = 0;

    c->refreshrate = old_refreshrate;
}


void fullscreen_get_current_display_dx9(int *bitdepth, int *width,
                                          int *height, int *refreshrate)
{
    D3DDISPLAYMODE mode;

    if (S_OK == IDirect3D9_GetAdapterDisplayMode(
                            d3d, D3DADAPTER_DEFAULT , &mode))
    {
        *bitdepth = 32;
        *width = mode.Width;
        *height = mode.Height;
        *refreshrate = mode.RefreshRate;
    } else {
        /* provide defaults if GetDisplayMode fails for some reason */
        log_debug("fullscreen_get_current_display_dx9 failed to get mode!");
        *bitdepth = 32;
        *width = 640;
        *height = 480;
        *refreshrate = 0;
    }
}
