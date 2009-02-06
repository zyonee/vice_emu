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
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "videoarch.h"
#include "winmain.h"
#include "statusbar.h"


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

