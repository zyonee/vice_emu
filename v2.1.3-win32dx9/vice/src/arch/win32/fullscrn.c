/*
 * fullscrn.c - Common fullscreen related support functions for Win32
 *
 * Written by
 *  Tibor Biczo <crown@matavnet.hu>
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

#include <windows.h>

#include "fullscrn.h"
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "videoarch.h"
#include "winmain.h"
#include "statusbar.h"



DirectDrawDeviceList *devices = NULL;
DirectDrawModeList *modes = NULL;


void fullscreen_set_res_from_current_display(void)
{
    int bitdepth, width, height, refreshrate;

    if (video_dx9_enabled()) {
        fullscreen_get_current_display_dx9(
                &bitdepth, &width, &height, &refreshrate);
    } else {
        fullscreen_get_current_display_ddraw(
                &bitdepth, &width, &height, &refreshrate);
    }

    resources_set_int("FullscreenBitdepth", bitdepth);
    resources_set_int("FullscreenWidth", width);
    resources_set_int("FullscreenHeight", height);
    resources_set_int("FullscreenRefreshRate", refreshrate);
}


/* check if the fullscreen resource values are valid */
static int fullscrn_res_valid(void)
{
    int width, height, bitdepth, refreshrate;

    GetCurrentModeParameters(&width, &height, &bitdepth,&refreshrate);
    
    /* FIXME: May use modelist to check if combination is valid */
    if (width <= 0 || height <= 0 || bitdepth <= 0 || refreshrate < 0)
        return -1;

    return 0;
}


void fullscreen_getmodes(void)
{
    if (video_dx9_enabled()) {
        fullscreen_getmodes_dx9();
    } else {
        fullscreen_getmodes_ddraw();
    }
}


void ui_fullscreen_init(void)
{
    fullscreen_getmodes();

    /* Use current display parameters if resources are not valid */
    if (fullscrn_res_valid() < 0) {
        fullscreen_set_res_from_current_display();
    }
}


void ui_fullscreen_shutdown(void)
{
    DirectDrawModeList *m1, *m2;
    DirectDrawDeviceList *d1, *d2;

    m1 = modes;
    while (m1 != NULL) {
        m2 = m1->next;
        lib_free(m1);
        m1 = m2;
    }

    d1 = devices;
    while (d1 != NULL) {
        d2 = d1->next;
        lib_free(d1->desc);
        lib_free(d1);
        d1 = d2;
    }
}


void GetCurrentModeParameters(int *width, int *height, int *bitdepth,
                              int *refreshrate)
{
    resources_get_int("FullscreenBitdepth", bitdepth);
    resources_get_int("FullscreenWidth", width);
    resources_get_int("FullscreenHeight", height);
    resources_get_int("FullscreenRefreshRate", refreshrate);
}


int IsFullscreenEnabled(void)
{
    int b;

    resources_get_int("FullscreenEnabled", &b);

    return b;
}


void SwitchToFullscreenMode(HWND hwnd)
{
    if (video_dx9_enabled()) {
        SwitchToFullscreenModeDx9(hwnd);
    } else {
        SwitchToFullscreenModeDDraw(hwnd);
    }
}


void SwitchToWindowedMode(HWND hwnd)
{
    if (video_dx9_enabled()) {
        SwitchToWindowedModeDx9(hwnd);
    } else {
        SwitchToWindowedModeDDraw(hwnd);
    }
}

