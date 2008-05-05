/*
 * uijoystick2.c
 *
 * Written by
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

#include "vice.h"

#include <stdio.h>

#include "joystick.h"
#include "resources.h"
#include "uimenu.h"
#include "vsync.h"
#include "uijoystickkeys.h"


static UI_CALLBACK(set_joystick_device_1)
{
    int tmp;

    if (!CHECK_MENUS) {
        resources_set_int("JoyDevice1", (int)UI_MENU_CB_PARAM);
        ui_update_menus();
    } else {
        resources_get_int("JoyDevice1", &tmp);
        ui_menu_set_tick(w, tmp == (int)UI_MENU_CB_PARAM);
    }
}

static UI_CALLBACK(set_joystick_device_2)
{
    int tmp;

    if (!CHECK_MENUS) {
        resources_set_int("JoyDevice2", (int)UI_MENU_CB_PARAM);
        ui_update_menus();
    } else {
        resources_get_int("JoyDevice2", &tmp);
        ui_menu_set_tick(w, tmp == (int)UI_MENU_CB_PARAM);
    }
}

static UI_CALLBACK(swap_joystick_ports)
{
    int tmp1, tmp2;

    if (w != NULL)
        vsync_suspend_speed_eval();
    resources_get_int("JoyDevice1", &tmp1);
    resources_get_int("JoyDevice2", &tmp2);
    resources_set_int("JoyDevice1", tmp2);
    resources_set_int("JoyDevice2", tmp1);
    ui_update_menus();
}

static ui_menu_entry_t *devices_submenu1 = NULL;
static ui_menu_entry_t *devices_submenu2 = NULL;

static ui_menu_entry_t set_joystick_device_1_submenu[] = {
    { N_("*None"),
      (ui_callback_t)set_joystick_device_1,
      (ui_callback_data_t)JOYDEV_NONE, NULL },
    { N_("*Numpad"),
      (ui_callback_t)set_joystick_device_1,
      (ui_callback_data_t)JOYDEV_NUMPAD, NULL },
    { N_("*Keyset 1"),
      (ui_callback_t)set_joystick_device_1,
      (ui_callback_data_t)JOYDEV_KEYSET1, NULL },
    { N_("*Keyset 2"),
      (ui_callback_t)set_joystick_device_1,
      (ui_callback_data_t)JOYDEV_KEYSET2, NULL },
    { "", NULL }, /* Will be changed by uijoystick_menu_create */
                  /* Important: must be index 4 */
    { NULL }
};

static ui_menu_entry_t set_joystick_device_2_submenu[] = {
    { N_("*None"),
      (ui_callback_t)set_joystick_device_2,
      (ui_callback_data_t)JOYDEV_NONE, NULL },
    { N_("*Numpad"),
      (ui_callback_t)set_joystick_device_2,
      (ui_callback_data_t)JOYDEV_NUMPAD, NULL },
    { N_("*Keyset 1"),
      (ui_callback_t)set_joystick_device_2,
      (ui_callback_data_t)JOYDEV_KEYSET1, NULL },
    { N_("*Keyset 2"),
      (ui_callback_t)set_joystick_device_2,
      (ui_callback_data_t)JOYDEV_KEYSET2, NULL },
    { "", NULL }, /* Will be changed by uijoystick_menu_create */
                  /* Important: must be index 4 */
    { NULL }
};


ui_menu_entry_t joystick_settings_submenu[] = {
    { N_("Joystick device in port 1"),
      NULL, NULL, set_joystick_device_1_submenu },
    { N_("Joystick device in port 2"),
      NULL, NULL, set_joystick_device_2_submenu },
    { "--" },
#ifdef USE_GNOMEUI
    { N_("Define keysets"),
      (ui_callback_t)ui_keyset_dialog },
    { "--" },
#endif
    { N_("Swap joystick ports"),
      (ui_callback_t)swap_joystick_ports, NULL, NULL, KEYSYM_j, UI_HOTMOD_META },
    { NULL }
};

ui_menu_entry_t joystick_options_submenu[] = {
    { N_("Swap joystick ports"),
      (ui_callback_t)swap_joystick_ports, NULL, NULL, KEYSYM_j, UI_HOTMOD_META },
    { NULL }
};

ui_menu_entry_t joystick_settings_menu[] = {
    { N_("Joystick settings"),
      NULL, NULL, joystick_settings_submenu },
    { NULL }
};

void uijoystick_menu_create(void)
{
    unsigned int i, num;

    num = joystick_device_num();

    if (num == 0)
        return;

    devices_submenu1 = (ui_menu_entry_t *)lib_calloc((size_t)(num + 1),
                      sizeof(ui_menu_entry_t));
    devices_submenu2 = (ui_menu_entry_t *)lib_calloc((size_t)(num + 1),
                      sizeof(ui_menu_entry_t));

    for (i = 0; i < num ; i++) {
        devices_submenu1[i].string =
            (ui_callback_data_t)lib_msprintf("*%s", joystick_device_name(i));
        devices_submenu1[i].callback = (ui_callback_t)set_joystick_device_1;
        devices_submenu1[i].callback_data
            = (ui_callback_data_t)(i + JOYDEV_HW_BASE);
        devices_submenu2[i].string =
            (ui_callback_data_t)lib_msprintf("*%s", joystick_device_name(i));
        devices_submenu2[i].callback = (ui_callback_t)set_joystick_device_2;
        devices_submenu2[i].callback_data
            = (ui_callback_data_t)(i + JOYDEV_HW_BASE);
    }

    set_joystick_device_1_submenu[4].sub_menu = devices_submenu1;
    set_joystick_device_2_submenu[4].sub_menu = devices_submenu2;
}

void uijoystick_menu_shutdown(void)
{
    unsigned int i;

    set_joystick_device_1_submenu[4].sub_menu = NULL;
    set_joystick_device_2_submenu[4].sub_menu = NULL;

    i = 0;

    if (devices_submenu1 != NULL) {
        while (devices_submenu1[i].string != NULL) {
            lib_free(devices_submenu1[i++].string);
        }
    }

    i = 0;

    if (devices_submenu2 != NULL) {
        while (devices_submenu2[i].string != NULL) {
            lib_free(devices_submenu2[i++].string);
        }
    }

   lib_free(devices_submenu1);
   lib_free(devices_submenu2);
}

