/*
 * c64dtvui.c - Native GTK3 C64DTV UI.
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
#include "c64dtvmodel.h"
#include "machinemodelwidget.h"
#include "videomodelwidget.h"

#include "c64ui.h"


static const char *c64dtv_model_list[] = {
    "V2 PAL", "V2 NTSC", "V3 PAL", "V3 NTSC", "Hummer (NTSC)", NULL
};


static ui_radiogroup_entry_t c64dtv_vicii_models[] = {
     { "PAL-G", MACHINE_SYNC_PAL },
     { "NTSC-M", MACHINE_SYNC_NTSC },
     { NULL, -1 }
};


int c64dtvui_init(void)
{
    /* Some of the work here is done by video.c now, and would need to
     * be shifted over */

    machine_model_widget_getter(dtvmodel_get);
    machine_model_widget_setter(dtvmodel_set);
    machine_model_widget_set_models(c64dtv_model_list);

    video_model_widget_set_title("VIC_II model");
    video_model_widget_set_resource("MachineVideoStandard");
    video_model_widget_set_models(c64dtv_vicii_models);

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}

void c64dtvui_shutdown(void)
{
    INCOMPLETE_IMPLEMENTATION();
}

