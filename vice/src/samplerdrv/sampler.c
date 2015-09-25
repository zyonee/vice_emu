/*
 * sampler.c - audio input driver manager.
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

#include <string.h>

#include "sampler.h"

#ifdef USE_PORTAUDIO
#include "portaudio_drv.h"
#endif

#define MAX_DEVICE 10

/* stays at 0 for now, but will become configurable in the future */
static int current_sampler = 0;

static sampler_device_t devices[MAX_DEVICE];

void sampler_init(void)
{
    memset(devices, 0, sizeof(devices));

#ifdef USE_PORTAUDIO
    portaudio_init();
#endif
}

void sampler_start()
{
    if (devices[current_sampler].open) {
        devices[current_sampler].open();
    }
}

void sampler_stop(void)
{
    if (devices[current_sampler].close) {
        devices[current_sampler].close();
    }
}

BYTE sampler_get_sample(void)
{
    if (devices[current_sampler].get_sample) {
        return devices[current_sampler].get_sample();
    }
    return 0x80;
}

void sampler_device_register(sampler_device_t *device)
{
    int i;

    for (i = 0; devices[i].name; ++i) {
    }

    devices[i].name = device->name;
    devices[i].open = device->open;
    devices[i].close = device->close;
    devices[i].get_sample = device->get_sample;
}
