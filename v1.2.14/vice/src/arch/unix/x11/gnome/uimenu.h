/*
 * uimenu.h - Simple and ugly cascaded pop-up menu implementation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * Support for multiple visuals and depths by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include "ui.h"
#include "uihotkey.h"

typedef struct _ui_menu_entry {
    const char *string;
    ui_callback_t callback;
    ui_callback_data_t callback_data;
    struct _ui_menu_entry *sub_menu;
    KeySym hotkey_keysym;
    ui_hotkey_modifier_t hotkey_modifier;
} ui_menu_entry_t;

typedef enum { 
  CB_NORMAL , 
  CB_REFRESH 
} ui_menu_cb_status;

typedef struct {
    void* value; 
    ui_menu_cb_status status;
} ui_menu_cb_obj;


extern ui_menu_entry_t ui_menu_separator[];

extern int ui_menu_init();
extern void ui_menu_set_sensitive(GtkWidget *w, int flag);
extern void ui_menu_set_tick(GtkWidget *w, int flag);
extern int ui_menu_any_open(void);
extern void ui_menu_update_all(void);
extern void ui_menu_update_all_GTK(void);
#ifndef GNOME_MENUS
extern GtkWidget* ui_menu_create(const char *menu_name, ...);
#else
extern GnomeUIInfo* ui_menu_create(const char *menu_name, ...);
#endif
extern int refresh_dummy;

/* ------------------------------------------------------------------------- */

/* These are the callbacks for the toggle and radio menus (the ones with a
   checkmark on the left).  If call_data is NULL, they simply set/unset the
   checkmark according to the value of the corrisponding resource.  If not
   NULL, they set the value of the corresponding resource before doing so.
   For this reason, to update the checkmarks, we simply have to call all the
   callbacks with a NULL `call_data' parameter.  */

#define UI_MENU_DEFINE_TOGGLE(resource)                                 \
    static void toggle_##resource(GtkWidget *w, ui_callback_data_t event_data) \
    {                                                                   \
        _ui_menu_toggle_helper(w, event_data, #resource);   \
    }

#define UI_MENU_DEFINE_RADIO(resource)                                  \
    static void radio_##resource(GtkWidget *w, ui_callback_data_t event_data) \
    {                                                                   \
        _ui_menu_radio_helper(w, event_data, #resource);    \
    }

#define UI_MENU_DEFINE_STRING_RADIO(resource)                               \
    static void radio_##resource(GtkWidget *w, ui_callback_data_t event_data) \
    {                                                                       \
        _ui_menu_string_radio_helper(w, event_data, #resource); \
    }

/* Private helper functions for toggle and radio menu items.  */
extern void _ui_menu_toggle_helper(GtkWidget *w,
                                   ui_callback_data_t event_data,
                                   const char *resource_name);
extern void _ui_menu_radio_helper(GtkWidget *w,
                                  ui_callback_data_t event_data,
                                  const char *resource_name);
extern void _ui_menu_string_radio_helper(GtkWidget *w,
                                         ui_callback_data_t event_data,
                                         const char *resource_name);


/*#define CHECK_MENUS (event_data != &refresh_dummy)*/

#define CHECK_MENUS (((ui_menu_cb_obj*)event_data)->status != CB_NORMAL)
#define UI_MENU_CB_PARAM (((ui_menu_cb_obj*)event_data)->value) 


#endif /* _UIMENU_H */
