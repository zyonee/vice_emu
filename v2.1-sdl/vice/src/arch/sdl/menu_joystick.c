/*
 * menu_joystick.c - Joystick menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "joy.h"
#include "menu_common.h"
#include "menu_joystick.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(JoyDevice1)

static const ui_menu_entry_t joystick_port1_device_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_NONE },
    { "Numpad",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_NUMPAD },
    { "Keyset 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_KEYSET1 },
    { "Keyset 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_KEYSET2 },
    { "Joystick",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice1_callback,
      (ui_callback_data_t)JOYDEV_JOYSTICK },
    { NULL }
};

UI_MENU_DEFINE_RADIO(JoyDevice2)

static const ui_menu_entry_t joystick_port2_device_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_NONE },
    { "Numpad",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_NUMPAD },
    { "Keyset 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_KEYSET1 },
    { "Keyset 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_KEYSET2 },
    { "Joystick",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_JoyDevice2_callback,
      (ui_callback_data_t)JOYDEV_JOYSTICK },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(KeySetEnable)

static UI_MENU_CALLBACK(custom_swap_ports_callback)
{
    if (activated) {
        sdljoy_swap_ports();
    }
    return NULL;
}

const ui_menu_entry_t joystick_menu[] = {
    { "Joystick device in port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device in port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER,
      custom_swap_ports_callback,
      NULL },
    { NULL }
};

const ui_menu_entry_t joystick_single_menu[] = {
    { "Joystick device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { NULL }
};
