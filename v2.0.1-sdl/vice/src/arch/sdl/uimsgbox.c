/*
 * uimsgbox.c - Common SDL message box functions.
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

#include <string.h>
#include <SDL/SDL.h>

#include "lib.h"
#include "ui.h"
#include "uimenu.h"

static int make_28_cols(char *text)
{
    int i = 28;
    int j = 1;

    while (i < (strlen(text) - 1))
    {
        while (text[i] != ' ')
        {
            i--;
        }
        text[i] = 0;
        i += 28;
        j++;
    }
    return j;
}

void message_box_ok(const char *title, char *message)
{
    char *text;
    char *pos;
    char template[40];
    int lines, before;
    int active = 1;
    int i, j;

    text = lib_stralloc(message);
    pos = text;
    lines = make_28_cols(text);
    sdl_ui_clear();
    sdl_ui_print_center("\260\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\256", 2);
    sprintf(template,"\335                            \335");
    before = (28 - strlen(title))/2;
    for (i = 0; i < strlen(title); i++)
    {
        template[i + before + 1] = title[i];
    }
    sdl_ui_print_center(template, 3);
    sdl_ui_print_center("\253\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\263", 4);
    for (j = 0; j < lines; j++)
    {
        sprintf(template,"\335                            \335");
        before = (28 - strlen(pos))/2;
        for (i = 0; i < strlen(pos); i++)
        {
            template[i + before + 1] = pos[i];
        }
        sdl_ui_print_center(template, j + 5);

        pos += strlen(pos) + 1;
    }
    sdl_ui_print_center("\335                            \335", j + 5);
    sdl_ui_print_center("\335            \260\300\300\256            \335", j + 6);
    sdl_ui_print_center("\335            \335OK\335            \335", j + 7);
    sdl_ui_print_center("\335            \255\300\300\275            \335", j + 8);
    sdl_ui_print_center("\255\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\300\275", j + 9);
    sdl_ui_reverse_colors();
    sdl_ui_print_center("OK", j + 7);
    sdl_ui_reverse_colors();
    sdl_ui_refresh();
    while (active)
    {
        switch(sdl_ui_menu_poll_input())
        {
            case MENU_ACTION_CANCEL:
            case MENU_ACTION_EXIT:
            case MENU_ACTION_SELECT:
                active = 0;
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }
}
