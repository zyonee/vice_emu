/*
 * fullscrn.c - Fullscreen related support functions for Win32
 *
 * Written by
 *  Tibor Biczo <crown@matavnet.hu>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <prsht.h>

#include "intl.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "videoarch.h"
#include "winmain.h"
#include "statusbar.h"

void SuspendFullscreenMode(HWND hwnd)
{
}

void ResumeFullscreenMode(HWND hwnd)
{
}

void SuspendFullscreenModeKeep(HWND hwnd)
{
}

void ResumeFullscreenModeKeep(HWND hwnd)
{
}

void SwitchFullscreenMode(HWND hwnd)
{
}


BOOL CALLBACK dialog_fullscreen_proc(HWND hwnd, UINT msg, WPARAM wparam,
                                     LPARAM lparam)
{
    return TRUE;
}


void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
    cap_fullscreen->device_num = 0;
}
