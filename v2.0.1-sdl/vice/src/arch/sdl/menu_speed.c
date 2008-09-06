/*
 * menu_speed.c - Implementation of the speed settings menu for the SDL UI.
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

#include <stdio.h>
#include <stdlib.h>

#include "lib.h"
#include "menu_speed.h"
#include "resources.h"
#include "uimenu.h"

/* prototypes of the custom input functions */
static const char *custom_RefreshRate_callback(int activated, ui_callback_data_t param);
static const char *custom_Speed_callback(int activated, ui_callback_data_t param);

UI_MENU_DEFINE_RADIO(RefreshRate)

static ui_menu_entry_t refresh_rate_menu[] = {
    { "Auto",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)0,
      NULL },
    { "1/1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)1,
      NULL },
    { "1/2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)2,
      NULL },
    { "1/3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)3,
      NULL },
    { "1/4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)4,
      NULL },
    { "1/5",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)5,
      NULL },
    { "1/6",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)6,
      NULL },
    { "1/7",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)7,
      NULL },
    { "1/8",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)8,
      NULL },
    { "1/9",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)9,
      NULL },
    { "1/10",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RefreshRate_callback,
      (ui_callback_data_t)10,
      NULL },
    { "Custom",
      MENU_ENTRY_DIALOG,
      custom_RefreshRate_callback,
      NULL,
      NULL },
    { NULL }
};

UI_MENU_DEFINE_RADIO(Speed)

static ui_menu_entry_t maximum_speed_menu[] = {
    { "10%",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)10,
      NULL },
    { "25%",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)25,
      NULL },
    { "50%",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)50,
      NULL },
    { "100%",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)100,
      NULL },
    { "200%",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)200,
      NULL },
    { "No limit",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Speed_callback,
      (ui_callback_data_t)0,
      NULL },
    { "Custom",
      MENU_ENTRY_DIALOG,
      custom_Speed_callback,
      NULL,
      NULL },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(WarpMode)

ui_menu_entry_t speed_menu[] = {
    { "Refresh rate",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      refresh_rate_menu },
    { "Maximum speed",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      maximum_speed_menu },
    { "Warp mode",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_WarpMode_callback,
      NULL,
      NULL },
    { NULL }
};

static const char *custom_RefreshRate_callback(int activated, ui_callback_data_t param)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    resources_get_int("RefreshRate", &previous);

    if (activated)
    {
        sprintf(buf, "%i", previous);
        value = sdl_ui_text_input_dialog("Enter custom refresh rate", buf);
        if (value)
        {
            new_value = strtol(value, NULL, 0);
            if (new_value != previous)
            {
                resources_set_int("RefreshRate", new_value);
            }
            lib_free(value);
        }
    }
    else
    {
        if(previous > 10)
        {
            return "*";
        }
        else
        {
            return " ";
        }
    }
    return NULL;
}

static const char *custom_Speed_callback(int activated, ui_callback_data_t param)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    resources_get_int("Speed", &previous);

    if (activated)
    {
        sprintf(buf, "%i", previous);
        value = sdl_ui_text_input_dialog("Enter custom maximum speed", buf);
        if (value)
        {
            new_value = strtol(value, NULL, 0);
            if (new_value != previous)
            {
                resources_set_int("Speed", new_value);
            }
            lib_free(value);
        }
    }
    else
    {
        if (previous != 0 && previous != 10 && previous != 25 &&
            previous != 50 && previous != 100 && previous != 200)
        {
            return "*";
        }
        else
        {
            return " ";
        }
    }
    return NULL;
}

