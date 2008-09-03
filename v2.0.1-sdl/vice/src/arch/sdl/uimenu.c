/*
 * uimenu.c - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "charset.h"
#include "interrupt.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"
#include "video.h"
#include "videoarch.h"
#include "vsync.h"

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BACK 0
#define COLOR_FRONT 1
#define MENU_FIRST_Y 2

int sdl_menu_state = 0;

static ui_menu_entry_t *main_menu = NULL;

static WORD sdl_default_translation[256];

struct menufont_s {
    BYTE *font;
    WORD *translate;
    int w;
    int h;
};
typedef struct menufont_s menufont_t;

static menufont_t menufont = { NULL, sdl_default_translation, 0, 0 };

static char *vcache_name = NULL;

static int menu_draw_pitch;
static int menu_draw_offset;
static int menu_draw_max_text_x;
static int menu_draw_max_text_y;
static BYTE menu_draw_color_front = COLOR_FRONT;
static BYTE menu_draw_color_back = COLOR_BACK;

/* ------------------------------------------------------------------ */

static void sdl_ui_putchar(char c, int pos_x, int pos_y)
{
    int x, y;
    BYTE fontchar;
    BYTE *font_pos;
    BYTE *draw_pos;

    font_pos = &(menufont.font[menufont.translate[(int)c]]);
    draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * menufont.w + pos_y * menufont.h * menu_draw_pitch]);

    draw_pos += menu_draw_offset;

    for(y=0; y < menufont.h; ++y) {
        fontchar = *font_pos;
        for(x=0; x < menufont.w; ++x) {
            draw_pos[x] = (fontchar & (0x80 >> x))?menu_draw_color_front:menu_draw_color_back;
        }
        ++font_pos;
        draw_pos += sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    }
}

static int sdl_ui_print(const char *text, int pos_x, int pos_y)
{
    int i = 0;
    BYTE c;

    while((c = text[i]) != 0) {
        sdl_ui_putchar(c, pos_x+i, pos_y);
        ++i;
    }

    return i;
}

static void sdl_ui_clear(void)
{
    unsigned int x, y;
    const char c = ' ';

    for(y=0; y < menu_draw_max_text_y; ++y) {
        for(x=0; x < menu_draw_max_text_x; ++x) {
            sdl_ui_putchar(c, x, y);
        }
    }
}

static void sdl_ui_display_title(const char *title)
{
    sdl_ui_print(title, 0, 0);
}


static void sdl_ui_display_item(ui_menu_entry_t *item, int y_pos)
{
    int i;

    if(((item->string == NULL) || (item->type == MENU_ENTRY_SEPARATOR))) {
        return;
    }

    i = sdl_ui_print(item->string, 1, y_pos+2);

    switch(item->type) {
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
            sdl_ui_print(item->callback(0, item->callback_data), 1+i+1, y_pos+MENU_FIRST_Y);
            break;
        default:
            break;
    }
}

static void sdl_ui_display_cursor(int pos, int old_pos)
{
    const char c_erase = ' ';
    const char c_cursor = '>';

    if(pos == old_pos) {
        return;
    }

    if(old_pos >= 0) {
        sdl_ui_putchar(c_erase, 0, old_pos+MENU_FIRST_Y);
    }

    sdl_ui_putchar(c_cursor, 0, pos+MENU_FIRST_Y);
}

static ui_menu_action_t sdl_ui_menu_poll_input(void)
{
    ui_menu_action_t retval = MENU_ACTION_NONE;
    do {
        SDL_Delay(20);
        retval = ui_dispatch_events();
    } while (retval == MENU_ACTION_NONE);
    return retval;
}

static void sdl_ui_menu_redraw(ui_menu_entry_t *menu, const char *title, int num_items)
{
    int i;

    menu_draw_max_text_x = sdl_active_canvas->geometry->text_size.width;
    menu_draw_max_text_y = sdl_active_canvas->geometry->text_size.height;
    menu_draw_pitch = sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    menu_draw_offset = sdl_active_canvas->geometry->gfx_position.x
                     + sdl_active_canvas->geometry->gfx_position.y * menu_draw_pitch
                     + sdl_active_canvas->geometry->extra_offscreen_border_left;

    sdl_ui_clear();
    sdl_ui_display_title(title);

    for(i=0; i<num_items; ++i) {
        if(num_items == (menu_draw_max_text_y - MENU_FIRST_Y)) {
            break;
        }
        sdl_ui_display_item(&(menu[i]), i);
    }
}

static int sdl_ui_menu_display(ui_menu_entry_t *menu, const char *title)
{
    int num_items = 0, cur = 0, cur_old = -1, cur_offset = 0, in_menu = 1;

    while(menu[num_items].string != NULL) {
        ++num_items;
    }

    sdl_ui_menu_redraw(menu, title, num_items);

    while(in_menu) {
        sdl_ui_display_cursor(cur - cur_offset, cur_old - cur_offset);
        video_canvas_refresh_all(sdl_active_canvas);

        switch(sdl_ui_menu_poll_input()) {
            case MENU_ACTION_UP:
                if(cur > 0) {
                    cur_old = cur;
                    --cur;
                }
                break;
            case MENU_ACTION_DOWN:
                if(cur < (num_items-1)) {
                    cur_old = cur;
                    ++cur;
                }
                break;
            case MENU_ACTION_SELECT:
                if(sdl_ui_menu_item_activate(&(menu[cur]))) {
                    sdl_ui_menu_redraw(menu, title, num_items);
                }
                break;
            case MENU_ACTION_CANCEL:
                return 0;
                break;
            case MENU_ACTION_EXIT:
                in_menu = 0;
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }

    return 0;
}

static void sdl_ui_trap(WORD addr, void *data)
{
    int vcache_state;

    vsync_suspend_speed_eval();
    sdl_menu_state = 1;
    sdl_ui_menu_display(main_menu, "VICE main menu");
    sdl_menu_state = 0;

    resources_get_int(vcache_name, &vcache_state);

    if (vcache_state != 0)
        resources_set_int(vcache_name, 0);

    video_canvas_refresh_all(sdl_active_canvas);

    if (vcache_state != 0)
        resources_set_int(vcache_name, vcache_state);
}

/* ------------------------------------------------------------------ */

int sdl_ui_menu_item_activate(ui_menu_entry_t *item)
{
    switch(item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
            item->callback(1, item->callback_data);
            return 1;
            break;
        case MENU_ENTRY_SUBMENU:
            sdl_ui_menu_display(item->sub_menu, item->string);
            return 1;
            break;
        default:
            break;
    }
    return 0;
}

void sdl_ui_set_main_menu(ui_menu_entry_t *menu)
{
    main_menu = menu;
}

void sdl_ui_set_menu_font(BYTE *font, WORD *translate, int offset, int w, int h)
{
    int i;

    menufont.font = font;
    menufont.w = w;
    menufont.h = h;

    if(translate) {
        menufont.translate = translate;
    } else {
        for(i=0; i<256; ++i) {
            menufont.translate[i] = offset + h*charset_petcii_to_screencode(charset_p_topetcii((char)i), 0);
        }
    }
}

void sdl_ui_set_menu_colors(int front, int back)
{
    if(front >= 0) {
        menu_draw_color_front = (BYTE)(front & 0xff);
    }

    if(back >= 0) {
        menu_draw_color_back = (BYTE)(back & 0xff);
    }
}

void sdl_ui_activate(void)
{
    interrupt_maincpu_trigger_trap(sdl_ui_trap, 0);
}

const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name)
{
    int value, r;

    if (activated) {
        r = resources_toggle(resource_name, &value);
        if (r < 0)
            r = resources_get_int(resource_name, &value);
    } else
        r = resources_get_int(resource_name, &value);

    if (r < 0)
        return "?";
    else
        return value ? "*" : " ";
}

const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    if (activated) {
        resources_set_value(resource_name, (resource_value_t)param);
    } else {
        resource_value_t v;
        resources_get_value(resource_name, (void *)&v);
        if (v == (resource_value_t)param)
            return "*";
    }
    return " ";
}

void sdl_register_vcachename(char *vcache)
{
    vcache_name = vcache;
}

