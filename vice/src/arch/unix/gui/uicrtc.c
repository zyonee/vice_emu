/*
 * uicrtc.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#include "fullscreenarch.h"
#include "uicrtc.h"
#include "uimenu.h"
#include "uipalette.h"
#include "resources.h"
#include "openGL_sync.h"

#include "uifullscreen-menu.h"

UI_FULLSCREEN(CRTC, KEYSYM_f)

UI_MENU_DEFINE_STRING_RADIO(CrtcPaletteFile)

static ui_menu_entry_t crtc_palette_submenu[] = {
    { N_("*Default (Green)"), (ui_callback_t)radio_CrtcPaletteFile,
      (ui_callback_data_t)"green", NULL },
    { N_("*Amber"), (ui_callback_t)radio_CrtcPaletteFile,
      (ui_callback_data_t)"amber", NULL },
    { N_("*White"), (ui_callback_t)radio_CrtcPaletteFile,
      (ui_callback_data_t)"white", NULL },
    { "--" },
    { N_("Load custom"), (ui_callback_t)ui_load_palette,
      (ui_callback_data_t)"CrtcPaletteFile", NULL },
    { NULL }
};

#define NOTHING(x) x

UI_MENU_DEFINE_TOGGLE(CrtcDoubleSize)
UI_MENU_DEFINE_TOGGLE(CrtcDoubleScan)
UI_MENU_DEFINE_TOGGLE(CrtcVideoCache)
#ifdef HAVE_HWSCALE
UI_MENU_DEFINE_TOGGLE_COND(CrtcHwScale, HwScalePossible, NOTHING)
#endif
UI_MENU_DEFINE_TOGGLE(CrtcScale2x)

#ifdef HAVE_OPENGL_SYNC
UI_MENU_DEFINE_TOGGLE_COND(openGL_sync, openGL_no_sync, openGL_available)
#endif

#ifndef USE_GNOMEUI
UI_MENU_DEFINE_TOGGLE(UseXSync)
#endif

ui_menu_entry_t crtc_submenu[] = {
    { N_("*Double size"),
      (ui_callback_t)toggle_CrtcDoubleSize, NULL, NULL },
    { N_("*Double scan"),
      (ui_callback_t)toggle_CrtcDoubleScan, NULL, NULL },
    { N_("*Video cache"),
      (ui_callback_t)toggle_CrtcVideoCache, NULL, NULL },
    { "--" },
    { N_("*Scale 2x render"),
      (ui_callback_t)toggle_CrtcScale2x, NULL, NULL },
#ifdef HAVE_HWSCALE
    { N_("*Hardware scaling"),
      (ui_callback_t)toggle_CrtcHwScale, NULL, NULL },
#endif
    { "--" },
    { N_("*CRTC Screen color"),
      NULL, NULL, crtc_palette_submenu },
    { "--" },
#ifdef HAVE_OPENGL_SYNC
    { N_("*OpenGL Rastersynchronization"),
      (ui_callback_t)toggle_openGL_sync, NULL, NULL },
#endif
#ifdef HAVE_FULLSCREEN
    { N_("*Fullscreen settings"), NULL, NULL, fullscreen_menuCRTC },
#endif    
#ifndef USE_GNOMEUI
    { N_("*Use XSync()"),
      (ui_callback_t)toggle_UseXSync, NULL, NULL },
#endif
    { NULL }
};

void uicrtc_menu_create(void)
{
    UI_FULLSCREEN_MENU_CREATE(CRTC)
}

void uicrtc_menu_shutdown(void)
{
    UI_FULLSCREEN_MENU_SHUTDOWN(CRTC)
}
