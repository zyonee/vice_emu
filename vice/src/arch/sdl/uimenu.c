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

#include "archdep.h"
#include "charset.h"
#include "interrupt.h"
#include "ioutil.h"
#include "joy.h"
#include "lib.h"
#include "menu_common.h"
#include "raster.h"
#include "resources.h"
#include "sound.h"
#include "types.h"
#include "ui.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsync.h"

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sdl_menu_state = 0;

static ui_menu_entry_t *main_menu = NULL;

static WORD sdl_default_translation[256];

static BYTE *draw_buffer_backup = NULL;

static menufont_t menufont = { NULL, sdl_default_translation, 0, 0 };

static menu_draw_t menu_draw = { 0, 0, 40, 25, 0, 0, 1, 0 };

void (*sdl_ui_set_menu_params)(int index, menu_draw_t *menu_draw);

/* ------------------------------------------------------------------ */
/* static functions */

static ui_menu_retval_t sdl_ui_menu_item_activate(ui_menu_entry_t *item);

static void sdl_ui_putchar(BYTE c, int pos_x, int pos_y)
{
    int x, y;
    BYTE fontchar;
    BYTE *font_pos;
    BYTE *draw_pos;

    font_pos = &(menufont.font[menufont.translate[(int)c]]);
    draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * menufont.w + pos_y * menufont.h * menu_draw.pitch]);

    draw_pos += menu_draw.offset;

    for (y=0; y < menufont.h; ++y) {
        fontchar = *font_pos;
        for (x=0; x < menufont.w; ++x) {
            draw_pos[x] = (fontchar & (0x80 >> x))?menu_draw.color_front:menu_draw.color_back;
        }
        ++font_pos;
        draw_pos += menu_draw.pitch;
    }
}

static int sdl_ui_print_wrap(const char *text, int pos_x, int pos_y)
{
    int i = 0;
    BYTE c;

    if (text == NULL) {
        return 0;
    }

    while (pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    while ((c = text[i]) != 0) {
        if (pos_x == menu_draw.max_text_x) {
            ++pos_y;
            pos_x = 0;
        }

        if (pos_y == menu_draw.max_text_y) {
            sdl_ui_scroll_screen_up();
            --pos_y;
        }

        sdl_ui_putchar(c, pos_x++, pos_y);
        ++i;
    }

    return i;
}

static int *sdl_ui_menu_get_offsets(ui_menu_entry_t *menu, int num_items)
{
    int i, j, len, max_len;
    ui_menu_entry_type_t block_type;
    int *offsets = NULL;

    offsets = (int *)lib_malloc(num_items * sizeof(int));

    for (i = 0; i < num_items; ++i) {
        block_type = menu[i].type;

        switch (block_type) {
            case MENU_ENTRY_SUBMENU:
            case MENU_ENTRY_TEXT:
                offsets[i] = 1;
                break;
            default:
                max_len = 0;
                j = i;

                while (menu[j].type == block_type) {
                    len = strlen(menu[j].string);
                    offsets[j] = len;
                    if (len > max_len) {
                        max_len = len;
                    }
                    ++j;
                }

                while (i < j) {
                    len = offsets[i];
                    offsets[i] = max_len - len + 2;
                    ++i;
                }
                --i;
                break;
        }
    }

    return offsets;
}


static void sdl_ui_display_item(ui_menu_entry_t *item, int y_pos, int value_offset)
{
    int i;

    if (item->string == NULL) {
        return;
    }

    if ((item->type == MENU_ENTRY_TEXT)&&(vice_ptr_to_int(item->data) == 1)) {
        sdl_ui_reverse_colors();
    }

    i = sdl_ui_print(item->string, MENU_FIRST_X, y_pos+MENU_FIRST_Y);

    if ((item->type == MENU_ENTRY_TEXT)&&(vice_ptr_to_int(item->data) == 1)) {
        sdl_ui_reverse_colors();
    }

    sdl_ui_print(item->callback(0, item->data), MENU_FIRST_X+i+value_offset, y_pos+MENU_FIRST_Y);
}

static void sdl_ui_menu_redraw(ui_menu_entry_t *menu, const char *title, int offset, int *value_offsets)
{
    int i = 0;

    sdl_ui_init_draw_params();
    sdl_ui_clear();
    sdl_ui_display_title(title);

    while ((menu[i + offset].string != NULL) && (i <= (menu_draw.max_text_y - MENU_FIRST_Y))) {
        sdl_ui_display_item(&(menu[i + offset]), i, value_offsets[i + offset]);
        ++i;
    }
}

static ui_menu_retval_t sdl_ui_menu_display(ui_menu_entry_t *menu, const char *title)
{
    int num_items = 0, cur = 0, cur_old = -1, cur_offset = 0, in_menu = 1, redraw = 1;
    int *value_offsets = NULL;
    ui_menu_retval_t menu_retval = MENU_RETVAL_DEFAULT;

    while (menu[num_items].string != NULL) {
        ++num_items;
    }

    if (num_items == 0) {
        return 0;
    }

    value_offsets = sdl_ui_menu_get_offsets(menu, num_items);

    while (in_menu) {
        if (redraw) {
            sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets);
            cur_old = -1;
            redraw = 0;
        }
        sdl_ui_display_cursor(cur, cur_old);
        sdl_ui_refresh();

        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_UP:
                cur_old = cur;
                if (cur > 0) {
                    --cur;
                } else {
                    if (cur_offset > 0) {
                        --cur_offset;
                    } else {
                        cur_offset = num_items - (menu_draw.max_text_y - MENU_FIRST_Y);
                        cur = (menu_draw.max_text_y - MENU_FIRST_Y) - 1;
                        if (cur_offset < 0) {
                            cur += cur_offset;
                            cur_offset = 0;
                        }
                    }
                    redraw = 1;
                }
                break;
            case MENU_ACTION_DOWN:
                cur_old = cur;
                if ((cur + cur_offset) < (num_items - 1)) {
                    if (++cur == (menu_draw.max_text_y - MENU_FIRST_Y)) {
                        --cur;
                        ++cur_offset;
                        redraw = 1;
                    }
                } else {
                    cur = cur_offset = 0;
                    redraw = 1;
                }
                break;
            case MENU_ACTION_RIGHT:
                if (menu[cur + cur_offset].type != MENU_ENTRY_SUBMENU) {
                    break;
                }
                /* fall through */
            case MENU_ACTION_SELECT:
                if (sdl_ui_menu_item_activate(&(menu[cur + cur_offset])) == MENU_RETVAL_EXIT_UI) {
                    in_menu = 0;
                    menu_retval = MENU_RETVAL_EXIT_UI;
                } else {
                    sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets);
                }
                break;
            case MENU_ACTION_EXIT:
                menu_retval = MENU_RETVAL_EXIT_UI;
                /* fall through */
            case MENU_ACTION_LEFT:
            case MENU_ACTION_CANCEL:
                in_menu = 0;
                break;
            case MENU_ACTION_MAP:
                if (sdl_ui_hotkey_map(&(menu[cur + cur_offset]))) {
                    sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets);
                }
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }

    lib_free(value_offsets);
    return menu_retval;
}

static ui_menu_retval_t sdl_ui_menu_item_activate(ui_menu_entry_t *item)
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
            if (p == sdl_menu_text_exit_ui) {
                return MENU_RETVAL_EXIT_UI;
            }
            break;
        case MENU_ENTRY_SUBMENU:
            return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string);
            break;
        default:
            break;
    }
    return MENU_RETVAL_DEFAULT;
}

static void sdl_ui_trap(WORD addr, void *data)
{
    unsigned int width;
    unsigned int height;

    width = sdl_active_canvas->draw_buffer->draw_buffer_width;
    height = sdl_active_canvas->draw_buffer->draw_buffer_height;
    draw_buffer_backup = (BYTE *)lib_malloc(width * height);
    memcpy(draw_buffer_backup, sdl_active_canvas->draw_buffer->draw_buffer, width * height);

    sdl_ui_activate_pre_action();

    if (data == NULL) {
        sdl_ui_menu_display(main_menu, "VICE main menu");
    } else {
        sdl_ui_init_draw_params();
        sdl_ui_menu_item_activate((ui_menu_entry_t *)data);
    }

    if (ui_emulation_is_paused() && (width == sdl_active_canvas->draw_buffer->draw_buffer_width) && (height == sdl_active_canvas->draw_buffer->draw_buffer_height)) {
        memcpy(sdl_active_canvas->draw_buffer->draw_buffer, draw_buffer_backup, width * height);
        sdl_ui_refresh();
    }

    sdl_ui_activate_post_action();

    lib_free(draw_buffer_backup);
}

/* ------------------------------------------------------------------ */
/* External UI interface */

ui_menu_retval_t sdl_ui_external_menu_activate(ui_menu_entry_t *item)
{
    if (item && (item->type == MENU_ENTRY_SUBMENU)) {
        return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string);
    }

    return MENU_RETVAL_DEFAULT;
}

BYTE *sdl_ui_get_draw_buffer(void)
{
    return draw_buffer_backup;
}

menu_draw_t *sdl_ui_get_menu_param(void)
{
    return &menu_draw;
}

menufont_t *sdl_ui_get_menu_font(void)
{
    return &menufont;
}

void sdl_ui_activate_pre_action(void)
{
    vsync_suspend_speed_eval();
    sound_suspend();

    if (sdl_vkbd_state) {
        sdl_vkbd_close();
    }

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    sdl_menu_state = 1;
    ui_check_mouse_cursor();
}

void sdl_ui_activate_post_action(void)
{
    int warp_state;

    sdl_menu_state = 0;
    ui_check_mouse_cursor();
    SDL_EnableKeyRepeat(0, 0);

    /* Do not resume sound if in warp mode */
    resources_get_int("WarpMode", &warp_state);
    if (warp_state == 0) {
        sound_resume();
    }

    /* Force a video refresh */
    raster_force_repaint(sdl_active_canvas->parent_raster);
}

void sdl_ui_init_draw_params(void)
{
    if (sdl_ui_set_menu_params != NULL) {
        sdl_ui_set_menu_params(sdl_active_canvas->index, &menu_draw);
    }

    menu_draw.pitch = sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    menu_draw.offset = sdl_active_canvas->geometry->gfx_position.x + menu_draw.extra_x
                     + (sdl_active_canvas->geometry->gfx_position.y + menu_draw.extra_y) * menu_draw.pitch
                     + sdl_active_canvas->geometry->extra_offscreen_border_left;
}

void sdl_ui_reverse_colors(void)
{
    BYTE color;

    color = menu_draw.color_front;
    menu_draw.color_front = menu_draw.color_back;
    menu_draw.color_back = color;
}

ui_menu_action_t sdl_ui_menu_poll_input(void)
{
    ui_menu_action_t retval = MENU_ACTION_NONE;

    do {
        SDL_Delay(20);
        retval = ui_dispatch_events();
        if (retval == MENU_ACTION_NONE || retval == MENU_ACTION_NONE_RELEASE) {
            retval = sdljoy_autorepeat();
        }
    } while (retval == MENU_ACTION_NONE || retval == MENU_ACTION_NONE_RELEASE);
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

    if (text == NULL) {
        return 0;
    }

    if ((pos_x >= menu_draw.max_text_x)||(pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while (((c = text[i]) != 0)&&((pos_x + i) < menu_draw.max_text_x)) {
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

    if (text == NULL) {
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

    if ((pos_x >= menu_draw.max_text_x)||(pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while (((c = text[i]) != 0)&&((pos_x + i) < menu_draw.max_text_x)) {
        sdl_ui_putchar(c, pos_x+i, pos_y);
        ++i;
    }

    return i;
}

int sdl_ui_display_title(const char *title)
{
    return sdl_ui_print_wrap(title, 0, 0);
}

void sdl_ui_invert_char(int pos_x, int pos_y)
{
    int x, y;
    BYTE *draw_pos;

    while(pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * menufont.w + pos_y * menufont.h * menu_draw.pitch]);

    draw_pos += menu_draw.offset;

    for (y=0; y < menufont.h; ++y) {
        for (x=0; x < menufont.w; ++x) {
            if (draw_pos[x] == menu_draw.color_front) {
                draw_pos[x] = menu_draw.color_back;
            } else {
                draw_pos[x] = menu_draw.color_front;
            }
        }
        draw_pos += menu_draw.pitch;
    }
}

void sdl_ui_activate(void)
{
    if (ui_emulation_is_paused()) {
        ui_pause_emulation(0);
    }
    interrupt_maincpu_trigger_trap(sdl_ui_trap, NULL);
}

void sdl_ui_clear(void)
{
    int x, y;
    const char c = ' ';

    for (y = 0; y < menu_draw.max_text_y; ++y) {
        for (x = 0; x < menu_draw.max_text_x; ++x) {
            sdl_ui_putchar(c, x, y);
        }
    }
}

int sdl_ui_hotkey(ui_menu_entry_t *item)
{
    if (item == NULL) {
        return 0;
    }

    switch (item->type) {
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

char* sdl_ui_readline(const char* previous, int pos_x, int pos_y, int escaped_is_null)
{
#define SDL_UI_STRING_LEN_MAX 1024
    int i = 0, prev = -1, done = 0, got_key = 0, string_changed = 0, screen_dirty = 1, escaped = 0;
    size_t size = 0, max;
    char *new_string = NULL;
    SDL_Event e;
    SDLKey key;
    SDLMod mod;
    Uint16 c_uni;
    char c;

    if (previous) {
        new_string = lib_stralloc(previous);
        size = max = strlen(new_string) + 1;
        if (max < SDL_UI_STRING_LEN_MAX) {
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
        if (i != prev) {
            sdl_ui_invert_char(pos_x + i, pos_y);
            if (prev >= 0) {
                sdl_ui_invert_char(pos_x + prev, pos_y);
            }
            prev = i;
            screen_dirty = 1;
        }

        if (screen_dirty) {
            sdl_ui_refresh();
            screen_dirty = 0;
        }

        got_key = 0;
        do {
            SDL_WaitEvent(&e);
            switch (e.type) {
                case SDL_KEYDOWN:
                    key = e.key.keysym.sym;
                    mod = e.key.keysym.mod;
                    c_uni = e.key.keysym.unicode;
                    got_key = 1;
                    break;
                default:
                    ui_handle_misc_sdl_event(e);
                    break;
            }
        } while(!got_key);

        switch(key) {
            case SDLK_LEFT:
                if (i>0) {
                    --i;
                }
                break;
            case SDLK_RIGHT:
                if (i < (int)size) {
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
                if (i>0) {
                    memmove(new_string+i-1, new_string+i, size - i + 1);
                    --size;
                    new_string[size] = ' ';
                    sdl_ui_print_wrap(new_string+i-1, pos_x+i-1, pos_y);
                    new_string[size] = 0;
                    --i;
                    if (i != (int)size) {
                        prev = -1;
                    }
                    string_changed = 1;
                }
                break;
            case SDLK_ESCAPE:
                string_changed = 0;
                escaped = 1;
                /* fall through */
            case SDLK_RETURN:
                sdl_ui_invert_char(pos_x + i, pos_y);
                done = 1;
                break;
            default:
                got_key = 0; /* got unicode value */
                break;
        }

        if (!got_key && (size < max) && ((c_uni & 0xff80) == 0) && ((c_uni & 0x7f) != 0)) {
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

    if ((!string_changed && previous) || (escaped && escaped_is_null)) {
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
    return sdl_ui_readline(previous, 0, i+MENU_FIRST_Y, 0);
}

ui_menu_entry_t *sdl_ui_get_main_menu(void)
{
    return main_menu;
}

void sdl_ui_refresh(void)
{
    video_canvas_refresh_all(sdl_active_canvas);
}

void sdl_ui_scroll_screen_up(void)
{
    int i, j;
    BYTE *draw_pos = sdl_active_canvas->draw_buffer->draw_buffer + menu_draw.offset;

    for (i = 0; i < menu_draw.max_text_y-1; ++i) {
        for (j = 0; j < menufont.h; ++j) {
            memmove(draw_pos + (i * menufont.h + j) * menu_draw.pitch, draw_pos + (((i+1) * menufont.h) + j) * menu_draw.pitch, menu_draw.max_text_x * menufont.w);
        }
    }

    for (j = 0; j < menufont.h; ++j) {
        memset(draw_pos + (i * menufont.h + j) * menu_draw.pitch, (char)menu_draw.color_back, menu_draw.max_text_x * menufont.w);
    }
}

/* ------------------------------------------------------------------ */
/* Initialization/setting */

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

    for (i=0; i<256; ++i) {
        menufont.translate[i] = h*charset_petcii_to_screencode(charset_p_topetcii((char)i), 0);
    }
}

