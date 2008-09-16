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

#include "archdep.h"
#include "charset.h"
#include "interrupt.h"
#include "ioutil.h"
#include "lib.h"
#include "menu_common.h"
#include "resources.h"
#include "ui.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
#include "vsync.h"

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MENU_FIRST_Y 2
#define MENU_FIRST_X 1

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

static const char *vcache_name = NULL;

static menu_draw_t menu_draw;

/* ------------------------------------------------------------------ */
/* static functions */

static int sdl_ui_menu_item_activate(ui_menu_entry_t *item);

static void sdl_ui_scroll_screen_up(void)
{
    int i, j;
    BYTE *draw_pos = sdl_active_canvas->draw_buffer->draw_buffer + menu_draw.offset;

    for(i = 0; i < menu_draw.max_text_y-1; ++i) {
        for(j = 0; j < menufont.h; ++j) {
            memmove(draw_pos + (i * menufont.h + j) * menu_draw.pitch, draw_pos + (((i+1) * menufont.h) + j) * menu_draw.pitch, menu_draw.max_text_x * menufont.w);
        }
    }

    for(j = 0; j < menufont.h; ++j) {
        memset(draw_pos + (i * menufont.h + j) * menu_draw.pitch, (char)menu_draw.color_back, menu_draw.max_text_x * menufont.w);
    }
}

static void sdl_ui_putchar(char c, int pos_x, int pos_y)
{
    int x, y;
    BYTE fontchar;
    BYTE *font_pos;
    BYTE *draw_pos;

    font_pos = &(menufont.font[menufont.translate[(int)c]]);
    draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * menufont.w + pos_y * menufont.h * menu_draw.pitch]);

    draw_pos += menu_draw.offset;

    for(y=0; y < menufont.h; ++y) {
        fontchar = *font_pos;
        for(x=0; x < menufont.w; ++x) {
            draw_pos[x] = (fontchar & (0x80 >> x))?menu_draw.color_front:menu_draw.color_back;
        }
        ++font_pos;
        draw_pos += sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    }
}

static void sdl_ui_invert_char(int pos_x, int pos_y)
{
    int x, y;
    BYTE *draw_pos;

    while(pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * menufont.w + pos_y * menufont.h * menu_draw.pitch]);

    draw_pos += menu_draw.offset;

    for(y=0; y < menufont.h; ++y) {
        for(x=0; x < menufont.w; ++x) {
            if(draw_pos[x] == menu_draw.color_front) {
                draw_pos[x] = menu_draw.color_back;
            } else {
                draw_pos[x] = menu_draw.color_front;
            }
        }
        draw_pos += sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    }
}

static int sdl_ui_print_wrap(const char *text, int pos_x, int pos_y)
{
    int i = 0;
    BYTE c;

    if(text == NULL) {
        return 0;
    }

    while(pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    while((c = text[i]) != 0) {
        if(pos_x == menu_draw.max_text_x) {
            ++pos_y;
            pos_x = 0;
        }

        if(pos_y == menu_draw.max_text_y) {
            sdl_ui_scroll_screen_up();
            --pos_y;
        }

        sdl_ui_putchar(c, pos_x++, pos_y);
        ++i;
    }

    return i;
}

static void sdl_ui_reverse_colors(void)
{
    BYTE color;

    color = menu_draw.color_front;
    menu_draw.color_front = menu_draw.color_back;
    menu_draw.color_back = color;
}

static void sdl_ui_display_item(ui_menu_entry_t *item, int y_pos)
{
    int i;

    if(item->string == NULL) {
        return;
    }

    if((item->type == MENU_ENTRY_TEXT)&&((int)item->data == 1)) {
        sdl_ui_reverse_colors();
    }

    i = sdl_ui_print(item->string, MENU_FIRST_X, y_pos+MENU_FIRST_Y);

    if((item->type == MENU_ENTRY_TEXT)&&((int)item->data == 1)) {
        sdl_ui_reverse_colors();
    }

    switch(item->type) {
        case MENU_ENTRY_RESOURCE_STRING:
        case MENU_ENTRY_RESOURCE_INT:
            i += 3;
            /* fall through */
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_OTHER:
            sdl_ui_print(item->callback(0, item->data), MENU_FIRST_X+i+1, y_pos+MENU_FIRST_Y);
            break;
        case MENU_ENTRY_SUBMENU:
            sdl_ui_print("->", MENU_FIRST_X+i, y_pos+MENU_FIRST_Y);
            break;
        default:
            break;
    }
}

static void sdl_ui_init_draw_params(void)
{
    menu_draw.max_text_x = sdl_active_canvas->geometry->text_size.width * (menu_draw.max_text_x_double);
    menu_draw.max_text_y = sdl_active_canvas->geometry->text_size.height;
    menu_draw.pitch = sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    menu_draw.offset = sdl_active_canvas->geometry->gfx_position.x + menu_draw.extra_x
                     + (sdl_active_canvas->geometry->gfx_position.y + menu_draw.extra_y) * menu_draw.pitch
                     + sdl_active_canvas->geometry->extra_offscreen_border_left;
    menu_draw.first_x = MENU_FIRST_X;
    menu_draw.first_y = MENU_FIRST_Y;
}

static void sdl_ui_menu_redraw(ui_menu_entry_t *menu, const char *title, int num_items)
{
    int i;

    sdl_ui_init_draw_params();
    sdl_ui_clear();
    sdl_ui_display_title(title);

    for(i=0; i<num_items; ++i) {
        if(num_items == (menu_draw.max_text_y - MENU_FIRST_Y)) {
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
        sdl_ui_refresh();

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
                if(sdl_ui_menu_item_activate(&(menu[cur])) == 2) {
                    return 2;
                }
                sdl_ui_menu_redraw(menu, title, num_items);
                break;
            case MENU_ACTION_CANCEL:
                in_menu = 0;
                break;
            case MENU_ACTION_EXIT:
                return 2;
                break;
            case MENU_ACTION_MAP:
                if(sdl_ui_hotkey_map(&(menu[cur]))) {
                    sdl_ui_menu_redraw(menu, title, num_items);
                }
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }

    return 0;
}

static int sdl_ui_menu_item_activate(ui_menu_entry_t *item)
{
    const char *p = NULL;

    switch(item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
            p = item->callback(1, item->data);
            if(p == sdl_menu_text_exit_ui) {
                return 2;
            }
            return 1;
            break;
        case MENU_ENTRY_SUBMENU:
            return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string);
            break;
        default:
            break;
    }
    return 0;
}

static void sdl_ui_trap(WORD addr, void *data)
{
    int vcache_state;

    vsync_suspend_speed_eval();

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    sdl_menu_state = 1;
    if(data == NULL) {
        sdl_ui_menu_display(main_menu, "VICE main menu");
    } else {
        sdl_ui_init_draw_params();     
        sdl_ui_menu_item_activate((ui_menu_entry_t *)data);
    }
    sdl_menu_state = 0;
    SDL_EnableKeyRepeat(0, 0);

    /* Force a video refresh by temprorarily disabling vcache */
    resources_get_int(vcache_name, &vcache_state);

    if (vcache_state != 0) {
        resources_set_int(vcache_name, 0);
    }

    video_canvas_refresh_all(sdl_active_canvas);

    if (vcache_state != 0) {
        resources_set_int(vcache_name, vcache_state);
    }
}

/* ------------------------------------------------------------------ */
/* External UI interface */

ui_menu_action_t sdl_ui_menu_poll_input(void)
{
    ui_menu_action_t retval = MENU_ACTION_NONE;
    do {
        SDL_Delay(20);
        retval = ui_dispatch_events();
    } while (retval == MENU_ACTION_NONE);
    return retval;
}

void sdl_ui_display_cursor(int pos, int old_pos)
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

int sdl_ui_print(const char *text, int pos_x, int pos_y)
{
    int i = 0;
    BYTE c;

    if(text == NULL) {
        return 0;
    }

    if((pos_x >= menu_draw.max_text_x)||(pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while(((c = text[i]) != 0)&&((pos_x + i) < menu_draw.max_text_x)) {
        sdl_ui_putchar(c, pos_x+i, pos_y);
        ++i;
    }

    return i;
}

int sdl_ui_print_center(const char *text, int pos_y)
{
    int len, pos_x;
    int i = 0;
    BYTE c;

    if(text == NULL) {
        return 0;
    }

    len = strlen(text);

    if (len == 0) {
        return 0;
    }

    pos_x = (menu_draw.max_text_x - len) / 2;
    if (pos_x < 0) {
        return -1;
    }

    if((pos_x >= menu_draw.max_text_x)||(pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while(((c = text[i]) != 0)&&((pos_x + i) < menu_draw.max_text_x)) {
        sdl_ui_putchar(c, pos_x+i, pos_y);
        ++i;
    }

    return i;
}

int sdl_ui_display_title(const char *title)
{
    return sdl_ui_print_wrap(title, 0, 0);
}

void sdl_ui_clear(void)
{
    unsigned int x, y;
    const char c = ' ';

    for(y=0; y < menu_draw.max_text_y; ++y) {
        for(x=0; x < menu_draw.max_text_x; ++x) {
            sdl_ui_putchar(c, x, y);
        }
    }
}

void sdl_ui_activate(void)
{
    interrupt_maincpu_trigger_trap(sdl_ui_trap, NULL);
}

int sdl_ui_hotkey(ui_menu_entry_t *item)
{
    if(item == NULL) {
        return 0;
    }

    switch(item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
            return sdl_ui_menu_item_activate(item);
            break;
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_SUBMENU:
            interrupt_maincpu_trigger_trap(sdl_ui_trap, (void *)item);
        default:
            break;
    }
    return 0;
}

char* sdl_ui_readline(const char* previous, int pos_x, int pos_y)
{
#define SDL_UI_STRING_LEN_MAX 1024
    int i = 0, prev = -1, done = 0, got_key = 0, string_changed = 0, screen_dirty = 1;
    size_t size = 0, max;
    char *new_string = NULL;
    SDL_Event e;
    SDLKey key;
    SDLMod mod;
    Uint16 c_uni;
    char c;

    if(previous) {
        new_string = lib_stralloc(previous);
        size = max = strlen(new_string) + 1;
        if(max < SDL_UI_STRING_LEN_MAX) {
            new_string = lib_realloc(new_string, SDL_UI_STRING_LEN_MAX);
            max = SDL_UI_STRING_LEN_MAX;
        }
    } else {
        max = SDL_UI_STRING_LEN_MAX;
        new_string = lib_malloc(max);
        new_string[0] = 0;
    }

    size = i = sdl_ui_print_wrap(new_string, pos_x, pos_y);

    SDL_EnableUNICODE(1);

    do {
        if(i != prev) {
            sdl_ui_invert_char(pos_x + i, pos_y);
            if(prev >= 0) {
                sdl_ui_invert_char(pos_x + prev, pos_y);
            }
            prev = i;
            screen_dirty = 1;
        }

        if(screen_dirty) {
            sdl_ui_refresh();
            screen_dirty = 0;
        }

        got_key = 0;
        do {
            SDL_WaitEvent(&e);
            switch(e.type) {
                case SDL_QUIT:
                    exit(0);
                    break;
                case SDL_KEYDOWN:
                    key = e.key.keysym.sym;
                    mod = e.key.keysym.mod;
                    c_uni = e.key.keysym.unicode;
                    got_key = 1;
                break;
            default:
/*fprintf(stderr,"%s: %i\n",__func__,e.type);*/
                break;
            }
        } while(!got_key);

        switch(key) {
            case SDLK_LEFT:
                if(i>0) {
                    --i;
                }
                break;
            case SDLK_RIGHT:
                if(i<size) {
                    ++i;
                }
                break;
            case SDLK_HOME:
                i = 0;
                break;
            case SDLK_END:
                i = size;
                break;
            case SDLK_BACKSPACE:
                if(i>0) {
                    memmove(new_string+i-1, new_string+i, size - i + 1);
                    --size;
                    new_string[size] = ' ';
                    sdl_ui_print_wrap(new_string+i-1, pos_x+i-1, pos_y);
                    new_string[size] = 0;
                    --i;
                    if(i != size) {
                        prev = -1;
                    }
                    string_changed = 1;
                }
                break;
            case SDLK_ESCAPE:
                string_changed = 0;
                /* fall through */
            case SDLK_RETURN:
                done = 1;
                break;
            default:
                got_key = 0; /* got unicode value */
                break;
        }

        if(!got_key && (size < max) && ((c_uni & 0xff80) == 0) && ((c_uni & 0x7f) != 0)) {
            c = c_uni & 0x7f;
            memmove(new_string+i+1 , new_string+i, size - i);
            new_string[i] = c;
            ++size;
            new_string[size] = 0;
            sdl_ui_print_wrap(new_string+i, pos_x+i, pos_y);
            ++i;
            prev = -1;
            string_changed = 1;
        }

    } while(!done);

    SDL_EnableUNICODE(0);

    if(!string_changed && previous) {
        lib_free(new_string);
        new_string = NULL;
    }
    return new_string;
}

char* sdl_ui_text_input_dialog(const char* title, const char* previous)
{
    int i;

    sdl_ui_clear();
    i = sdl_ui_display_title(title) / menu_draw.max_text_x;
    return sdl_ui_readline(previous, 0, i+MENU_FIRST_Y);
}

ui_menu_entry_t *sdl_ui_get_main_menu(void)
{
    return main_menu;
}

void sdl_ui_refresh(void)
{
    video_canvas_refresh_all(sdl_active_canvas);
}

/* ------------------------------------------------------------------ */
/* Initialization/setting */

void sdl_ui_set_vcachename(const char *vcache)
{
    vcache_name = vcache;
}

void sdl_ui_set_menu_borders(int x, int y)
{
    menu_draw.extra_x = x;
    menu_draw.extra_y = y;
}

void sdl_ui_set_main_menu(const ui_menu_entry_t *menu)
{
    main_menu = (ui_menu_entry_t *)menu;
}

void sdl_ui_set_menu_font(BYTE *font, int w, int h)
{
    int i;

    menufont.font = font;
    menufont.w = w;
    menufont.h = h;

    for(i=0; i<256; ++i) {
        menufont.translate[i] = h*charset_petcii_to_screencode(charset_p_topetcii((char)i), 0);
    }
}

void sdl_ui_set_menu_colors(int front, int back)
{
    if(front >= 0) {
        menu_draw.color_front = (BYTE)(front & 0xff);
    }

    if(back >= 0) {
        menu_draw.color_back = (BYTE)(back & 0xff);
    }
}

void sdl_ui_set_double_x(int value)
{
    menu_draw.max_text_x_double = (value) ? 2 : 1;
}

menu_draw_t *sdl_ui_get_menu_param(void)
{
  return &menu_draw;
}

