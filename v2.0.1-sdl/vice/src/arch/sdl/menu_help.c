/*
 * menu_help.c - SDL help menu functions.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include "types.h"

#include <stdlib.h>
#include <SDL/SDL.h>

#include "info.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_help.h"
#include "ui.h"
#include "uimenu.h"
#include "video.h"
#include "videoarch.h"

static void show_text(const char *text)
{
    int next_line = 0;
    int next_page = 0;
    int current_line = 0;
    int x, y;
    int active = 1;
    int active_keys;
    char *string;
    menu_draw_t *menu_draw;

    menu_draw = sdl_ui_get_menu_param();

    string = lib_malloc(81);
    while(active)
    {
        sdl_ui_clear();
        for (y = 0; (y < menu_draw->max_text_y) && (current_line < strlen(text)); y++)
        {
            for (x = 0; text[current_line + x] != '\n'; x++)
            {
                string[x] = text[current_line + x];
            }
            if (x != 0)
            {
                string[x] = 0;
                sdl_ui_print(string, 0, y);
            }
            if (y == 0)
            {
                next_line = current_line + x + 1;
            }
            current_line += x + 1;
        }
        next_page = current_line;
        active_keys = 1;
        video_canvas_refresh_all(sdl_active_canvas);
        while (active_keys)
        {
            switch(sdl_ui_menu_poll_input())
            {
                case MENU_ACTION_CANCEL:
                case MENU_ACTION_EXIT:
                    active_keys = 0;
                    active = 0;
                    break;
                case MENU_ACTION_RIGHT:
                    active_keys = 0;
                    current_line = next_page;
                    break;
                case MENU_ACTION_DOWN:
                    active_keys = 0;
                    current_line = next_line;
                    break;
                default:
                    SDL_Delay(10);
                    break;
            }
        }
    }
    lib_free(string);
}

static UI_MENU_CALLBACK(about_callback)
{
    int active = 1;

    if (activated)
    {
        sdl_ui_clear();
        sdl_ui_print_center("VICE", 0);
        sdl_ui_print_center("Versatile Commodore Emulator", 1);
        sdl_ui_print_center("Version " VERSION, 2);
        sdl_ui_print_center("The VICE Team", 4);
        sdl_ui_print_center("(C) 1998-2008 Andreas Boose", 5);
        sdl_ui_print_center("(C) 1998-2008 Tibor Biczo", 6);
        sdl_ui_print_center("(C) 1999-2008 Andreas Matthies", 7);
        sdl_ui_print_center("(C) 1999-2008 Martin Pottendorfer", 8);
        sdl_ui_print_center("(C) 1998-2008 Dag Lem", 9);
        sdl_ui_print_center("(C) 2000-2008 Spiro Trikaliotis", 10);
        sdl_ui_print_center("(C) 2005-2008 Marco van den Heuvel", 11);
        sdl_ui_print_center("(C) 2006-2008 Christian Vogelgsang", 12);
        sdl_ui_print_center("(C) 2007-2008 Fabrizio Gennari", 13);
        sdl_ui_print_center("(C) 2007-2008 M. Kiesel", 14);
        sdl_ui_print_center("(C) 2007-2008 Hannu Nuotio", 15);
        sdl_ui_print_center("(C) 2007-2008 Daniel Kahlin", 16);
        sdl_ui_print_center("(C) 1999-2007 Andreas Dehmel", 17);
        sdl_ui_print_center("(C) 2003-2005 David Hansel", 18);
        sdl_ui_print_center("(C) 1999-2004 Thomas Bretz", 19);
        sdl_ui_print_center("(C) 2000-2004 Markus Brenner", 20);
        video_canvas_refresh_all(sdl_active_canvas);
        while(active)
        {
            switch(sdl_ui_menu_poll_input())
            {
                case MENU_ACTION_CANCEL:
                case MENU_ACTION_EXIT:
                    active = 0;
                    break;
                default:
                    SDL_Delay(10);
                    break;
            }
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(cmdline_callback)
{
    return NULL;
}

static UI_MENU_CALLBACK(contributors_callback)
{
    return NULL;
}

static UI_MENU_CALLBACK(license_callback)
{
    menu_draw_t *menu_draw;

    if (activated)
    {
        menu_draw = sdl_ui_get_menu_param();
        if (menu_draw->max_text_x_double == 2)
        {
            show_text(info_license_text);
        }
        else
        {
            show_text(info_license_text40);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(warranty_callback)
{
    menu_draw_t *menu_draw;

    if (activated)
    {
        menu_draw = sdl_ui_get_menu_param();
        if (menu_draw->max_text_x_double == 2)
        {
            show_text(info_warranty_text);
        }
        else
        {
            show_text(info_warranty_text40);
        }
    }
    return NULL;
}

const ui_menu_entry_t help_menu[] = {
    { "About",
      MENU_ENTRY_OTHER,
      about_callback,
      NULL },
    { "Command-line options (todo)",
      MENU_ENTRY_OTHER,
      cmdline_callback,
      NULL },
    { "Contributors (todo)",
      MENU_ENTRY_OTHER,
      contributors_callback,
      NULL },
    { "License",
      MENU_ENTRY_OTHER,
      license_callback,
      NULL },
    { "Warranty",
      MENU_ENTRY_OTHER,
      warranty_callback,
      NULL },
    { NULL }
};
