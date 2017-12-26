/*
 * vsidui.c - Native GTK3 VSID UI.
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

#include "not_implemented.h"

#include "machine.h"
#include "psid.h"
#include "ui.h"
#include "uisidattach.h"
#include "vicii.h"
#include "vsidui.h"


void vsid_ui_close(void)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_author(const char *author)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_copyright(const char *copyright)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_irqtype(const char *irq)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_name(const char *name)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_nr_of_tunes(int count)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_sid_model(int model)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_display_sync(int sync)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

static int count = 0;

void vsid_ui_display_time(unsigned int sec)
{
    NOT_IMPLEMENTED_WARN_X_TIMES(count, 3);
}

void vsid_ui_display_tune_nr(int nr)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

int vsid_ui_init(void)
{
    video_canvas_t *canvas = vicii_get_canvas();

    /* Some of the work here is done by video.c now, and would need to
     * be shifted over */

    ui_create_toplevel_window(canvas);
    ui_display_toplevel_window(canvas->window_index);

    uisidattach_set_psid_init_func(psid_init_driver);
    uisidattach_set_psid_play_func(machine_play_psid);

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}

void vsid_ui_setdrv(char *driver_info_text)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void vsid_ui_set_default_tune(int nr)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

