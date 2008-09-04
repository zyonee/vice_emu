/*
 * uimenu.h - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef _UIMENU_H
#define _UIMENU_H

#include "vice.h"
#include "types.h"

extern int sdl_menu_state;

typedef void* ui_callback_data_t;
typedef const char *(*ui_callback_t)(int activated, ui_callback_data_t callback_param);

typedef enum {
    MENU_ENTRY_NONE,
    MENU_ENTRY_SEPARATOR,
    MENU_ENTRY_RESOURCE_TOGGLE,
    MENU_ENTRY_RESOURCE_RADIO,
    MENU_ENTRY_RESOURCE_INT,
    MENU_ENTRY_RESOURCE_STRING,
    MENU_ENTRY_SUBMENU,
    MENU_ENTRY_DIALOG,
    MENU_ENTRY_OTHER
} ui_menu_entry_type_t;

typedef struct ui_menu_entry_s {
    char *string;
    ui_menu_entry_type_t type;
    ui_callback_t callback;
    ui_callback_data_t callback_data;
    struct ui_menu_entry_s *sub_menu;
} ui_menu_entry_t;

extern void sdl_register_vcachename(char *vcache_name);
extern void sdl_ui_set_main_menu(ui_menu_entry_t *menu);
extern void sdl_ui_set_menu_font(BYTE *font, int w, int h);
extern void sdl_ui_set_menu_colors(int front, int back);

extern void sdl_ui_activate(void);
extern int sdl_ui_menu_item_activate(ui_menu_entry_t *item);

typedef enum {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_UP,
    MENU_ACTION_DOWN,
    MENU_ACTION_SELECT,
    MENU_ACTION_CANCEL,
    MENU_ACTION_EXIT,
    MENU_ACTION_MAP,
    MENU_ACTION_NUM
} ui_menu_action_t;

#define UI_MENU_CALLBACK(name)                            \
    const char *name(int activated, ui_callback_data_t param)

#define UI_MENU_DEFINE_TOGGLE(resource)                              \
    static UI_MENU_CALLBACK(toggle_##resource##_callback)            \
    {                                                                \
        return sdl_ui_menu_toggle_helper(activated, #resource);      \
    }

#define UI_MENU_DEFINE_RADIO(resource)                                \
    static UI_MENU_CALLBACK(radio_##resource##_callback)              \
    {                                                                 \
        return sdl_ui_menu_radio_helper(activated, param, #resource); \
    }

extern const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name);
extern const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name);

#endif

