/*
 * joy.c - Joystick support for MS-DOS.
 *
 * Written by
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

#include <allegro.h>

#include "cmdline.h"
#include "joystick.h"
#include "resources.h"
#include "lib.h"

typedef struct joy_allegro_priv_s {
    int num;
} joy_allegro_priv_t;


/* Update the `joystick_value' variables according to the joystick status.  */
void joystick_allegro_poll(int idx, void* dev)
{
    int num = ((joy_allegro_priv_t*)dev)->num;
    int value = 0;

    poll_joystick();


        if (joy[num].stick[0].axis[0].d1)
            value |= 4;
        if (joy[num].stick[0].axis[0].d2)
            value |= 8;
        if (joy[num].stick[0].axis[1].d1)
            value |= 1;
        if (joy[num].stick[0].axis[1].d2)
            value |= 2;
        if (joy[num].button[0].b
         || joy[num].button[1].b
         || joy[num].button[2].b
         || joy[num].button[3].b
         || joy[num].button[4].b
         || joy[num].button[5].b
         || joy[num].button[6].b
         || joy[num].button[7].b)
            value |= 16;
        joystick_set_value_absolute(idx, value);
}

static joy_device_t allegro_joystick_device =
{
    joystick_allegro_poll,
    NULL,
    NULL,
    NULL
};



/* ------------------------------------------------------------------------- */

static int init_allegro_driver(int type)
{
    int i;
    
    joystick_close();
    remove_joystick();
    if(install_joystick(type)){
        return 1;
    }
    
    for (i=0; i<num_joysticks;i++)
    {
        joy_allegro_priv_t* priv = (joy_allegro_priv_t*)lib_malloc(sizeof(joy_allegro_priv_t));
        joy_data_t* dev = (joy_data_t*)lib_malloc(sizeof(joy_data_t));
        priv->num = i;
        dev->name = lib_msprintf("Hardware joystick %u (%s)", i, joystick_driver->name);
        dev->priv = priv;
        dev->device = &allegro_joystick_device;
        joystick_register_device(dev);
    }
    return 0;
}

#ifdef MSDOS
static int joystick_hw_type;

static int set_joystick_hw_type(int type, void* param)
{
    if (init_allegro_driver(type)){
        joystick_hw_type = 0;
        return 1;
    }
    joystick_hw_type = type;
    return 0;
}
#endif

int joystick_allegro_init(void)
{
#ifdef MSDOS
    static const resource_int_t resources_int[] = {
        { "HwJoyType", 0, RES_EVENT_NO, NULL,
         &joystick_hw_type, set_joystick_hw_type, NULL },
        { NULL }
    };
    return resources_register_int(resources_int);
#else
    init_allegro_driver(JOY_TYPE_AUTODETECT);
#endif
    return 0;
}
