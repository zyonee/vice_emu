/*
 * joystick.c - Joystick support for Windows.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "cmdline.h"
#include "resources.h"

#include "joystick.h"
#include "keyboard.h"
#include "types.h"
#include "ui.h"

/* Notice that this has to be `int' to make resources work.  */
static int keyset1[9], keyset2[9];

/* ------------------------------------------------------------------------ */

/* Joystick devices.  */
static joystick_device_t joystick_device_1, joystick_device_2;

static int set_joystick_device_1(resource_value_t v)
{
    joystick_device_t dev = (joystick_device_t) v;

    joystick_device_1 = dev;
    return 0;
}

static int set_joystick_device_2(resource_value_t v)
{
    joystick_device_t dev = (joystick_device_t) v;

    joystick_device_2 = dev;
    return 0;
}

#define DEFINE_SET_KEYSET(num, dir)                             \
    static int set_keyset##num##_##dir##(resource_value_t v)    \
    {                                                           \
        keyset##num##[KEYSET_##dir##] = (int) v;                \
                                                                \
        return 0;                                               \
    }

DEFINE_SET_KEYSET(1, NW)
DEFINE_SET_KEYSET(1, N)
DEFINE_SET_KEYSET(1, NE)
DEFINE_SET_KEYSET(1, E)
DEFINE_SET_KEYSET(1, SE)
DEFINE_SET_KEYSET(1, S)
DEFINE_SET_KEYSET(1, SW)
DEFINE_SET_KEYSET(1, W)
DEFINE_SET_KEYSET(1, FIRE)

DEFINE_SET_KEYSET(2, NW)
DEFINE_SET_KEYSET(2, N)
DEFINE_SET_KEYSET(2, NE)
DEFINE_SET_KEYSET(2, E)
DEFINE_SET_KEYSET(2, SE)
DEFINE_SET_KEYSET(2, S)
DEFINE_SET_KEYSET(2, SW)
DEFINE_SET_KEYSET(2, W)
DEFINE_SET_KEYSET(2, FIRE)

static resource_t resources[] = {
    { "JoyDevice1", RES_INTEGER, (resource_value_t) JOYDEV_NONE,
      (resource_value_t *) &joystick_device_1, set_joystick_device_1 },
    { "JoyDevice2", RES_INTEGER, (resource_value_t) JOYDEV_NONE,
      (resource_value_t *) &joystick_device_2, set_joystick_device_2 },
    { "KeySet1NorthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_NW], set_keyset1_NW },
    { "KeySet1North", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_N], set_keyset1_N },
    { "KeySet1NorthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_NE], set_keyset1_NE },
    { "KeySet1East", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_E], set_keyset1_E },
    { "KeySet1SouthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_SE], set_keyset1_SE },
    { "KeySet1South", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_S], set_keyset1_S },
    { "KeySet1SouthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_SW], set_keyset1_SW },
    { "KeySet1West", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_W], set_keyset1_W },
    { "KeySet1Fire", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_FIRE], set_keyset1_FIRE },
    { "KeySet2NorthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_NW], set_keyset2_NW },
    { "KeySet2North", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_N], set_keyset2_N },
    { "KeySet2NorthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_NE], set_keyset2_NE },
    { "KeySet2East", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_E], set_keyset2_E },
    { "KeySet2SouthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_SE], set_keyset2_SE },
    { "KeySet2South", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_S], set_keyset2_S },
    { "KeySet2SouthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_SW], set_keyset2_SW },
    { "KeySet2West", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_W], set_keyset2_W },
    { "KeySet2Fire", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_FIRE], set_keyset2_FIRE },
    { NULL }
};

int joystick_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

static cmdline_option_t cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice1", NULL,
      "<number>", "Set input device for joystick #1" },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice2", NULL,
      "<number>", "Set input device for joystick #2" },
    { NULL }
};

int joystick_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

int joystick_init(void)
{
}

int joystick_close(void)
{
}

void joystick_update(void)
{
}

/* Joystick-through-keyboard.  */

int handle_keyset_mapping(joystick_device_t device, int *set,
                          kbd_code_t kcode, int pressed)
{
    return 0;
}

int joystick_handle_key(kbd_code_t kcode, int pressed)
{
	return 0;
}
