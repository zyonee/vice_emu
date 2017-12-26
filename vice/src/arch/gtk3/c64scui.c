/** \file   src/arch/gtk3/c64scui.c
 * \brief   Native GTK3 C64SC UI
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Bas Wassink <b.wassink@ziggo.nl>
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
#include "widgethelpers.h"
#include "c64model.h"
#include "vicii.h"
#include "sampler.h"
#include "uisamplersettings.h"
#include "machinemodelwidget.h"
#include "videomodelwidget.h"
#include "clockportdevicewidget.h"
#include "clockport.h"

#include "cartridge.h"
#include "georamwidget.h"
#include "georam.h"
#include "reu.h"
#include "reuwidget.h"
#include "ramcartwidget.h"
#include "dqbbwidget.h"
#include "expertwidget.h"
#include "isepicwidget.h"
#include "gmod2widget.h"
#include "mmcrwidget.h"
#include "mmc64widget.h"
#include "retroreplaywidget.h"
#include "easyflashwidget.h"
#include "rrnetmk3widget.h"
#include "uicart.h"
#include "carthelpers.h"

#include "c64ui.h"


/** \brief  List of C64 models
 *
 * Used in the machine-model widget
 */
static const char *c64_model_list[] = {
    "C64 PAL", "C64C PAL", "C64 old PAL",
    "C64 NTSC", "C64C NTSC", "C64 old NTSC",
    "Drean",
    "C64 SX PAL", "C64 SX NTSC",
    "Japanese", "C64 GS",
    "PET64 PAL", "PET64 NTSC",
    "Ultimax", NULL };


/** \brief  List of VIC-II models
 *
 * Used in the VIC-II model widget
 */
static ui_radiogroup_entry_t c64sc_vicii_models[] = {
    { "6569 (PAL)",             VICII_MODEL_6569 },
    { "8565 (PAL)",             VICII_MODEL_8565 },
    { "6569R1 (old PAL)",       VICII_MODEL_6569R1 },
    { "6567 (NTSC)",            VICII_MODEL_6567 },
    { "8562 (NTSC)",            VICII_MODEL_8562 },
    { "6567R56A (old NTSC)",    VICII_MODEL_6567R56A },
    { "6572 (PAL-N)",           VICII_MODEL_6572 },
    { NULL, -1 }
};


int c64scui_init_early(void)
{
    INCOMPLETE_IMPLEMENTATION();
    return 0;
}


/** \brief  Initialize the UI
 *
 * \return  0 on success, -1 on failure
 */
int c64scui_init(void)
{
    /* Some of the work here is done by video.c now, and would need to
     * be shifted over */

    machine_model_widget_getter(c64model_get);
    machine_model_widget_setter(c64model_set);
    machine_model_widget_set_models(c64_model_list);

    video_model_widget_set_title("VIC-II model");
    video_model_widget_set_resource("VICIIModel");
    video_model_widget_set_models(c64sc_vicii_models);

    uisamplersettings_set_devices_getter(sampler_get_devices);
    /* work around VSID again */
    clockport_device_widget_set_devices((void *)clockport_supported_devices);

    /* I/O extension function pointers */
    carthelpers_set_functions(
            cartridge_save_image,
            cartridge_flush_image,
            cartridge_type_enabled);

    /* uicart_set_detect_func(cartridge_detect); only cbm2/plus4 */
    uicart_set_list_func(cartridge_get_info_list);
    uicart_set_attach_func(cartridge_attach_image);
    uicart_set_freeze_func(cartridge_trigger_freeze);
    uicart_set_detach_func(cartridge_detach_image);
    INCOMPLETE_IMPLEMENTATION();
    return 0;
}


/** \brief  Shut down the UI
 */
void c64scui_shutdown(void)
{
    INCOMPLETE_IMPLEMENTATION();
}
