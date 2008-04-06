/*
 * c128ui.cc - C128-specific user interface.
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

#include <Message.h>
#include <stdio.h>

extern "C" {
#include "c128ui.h"
#include "constants.h" 
#include "resources.h"
#include "ui.h"
#include "ui_vicii.h"
}

ui_menu_toggle  c128_ui_menu_toggles[]={
    { "DoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "DoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "VideoCache", MENU_TOGGLE_VIDEOCACHE },
    { "REU", MENU_TOGGLE_REU },
    { "IEEE488", MENU_TOGGLE_IEEE488 },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "SidFilters", MENU_TOGGLE_SIDFILTERS },
#ifdef HAVE_RESID
    { "SidUseResid", MENU_TOGGLE_SOUND_RESID },
#endif
    { "VDC_DoubleSize", MENU_TOGGLE_VDC_DOUBLESIZE },
    { "VDC_DoubleScan", MENU_TOGGLE_VDC_DOUBLESCAN },
    { "VDC_64KB", MENU_TOGGLE_VDC_64KB },
    { NULL, 0 }
};

static ui_res_possible_values SidType[] = {
    {0, MENU_SIDTYPE_6581},
    {1, MENU_SIDTYPE_8580},
    {-1,0}
};

#ifdef HAVE_RESID
static ui_res_possible_values SidResidSampling[] = {
    {0, MENU_RESID_SAMPLE_FAST},
    {1, MENU_RESID_SAMPLE_INTERPOLATE},
    {2, MENU_RESID_SAMPLE_RESAMPLE},
    {-1,0}
};
#endif

ui_res_value_list c128_ui_res_values[] = {
    {"SidModel", SidType},
#ifdef HAVE_RESID
    {"SidResidSampling", SidResidSampling},
#endif
    {NULL,NULL}
};


void c128_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
		case MENU_SIDTYPE_6581:
    		resources_set_value("SidModel", (resource_value_t) 0);
        	break;
		case MENU_SIDTYPE_8580:
        	resources_set_value("SidModel", (resource_value_t) 1);
        	break;
		case MENU_RESID_SAMPLE_FAST:
    		resources_set_value("SidResidSampling", (resource_value_t) 0);
        	break;
		case MENU_RESID_SAMPLE_INTERPOLATE:
    		resources_set_value("SidResidSampling", (resource_value_t) 1);
        	break;
		case MENU_RESID_SAMPLE_RESAMPLE:
    		resources_set_value("SidResidSampling", (resource_value_t) 2);
        	break;
		case MENU_VICII_SETTINGS:
        	ui_vicii();
        	break;

    	default: ;
    }
}

int c128_ui_init(void)
{
    ui_register_machine_specific(c128_ui_specific);
    ui_register_menu_toggles(c128_ui_menu_toggles);
    ui_register_res_values(c128_ui_res_values);
    ui_update_menus();
    return 0;
}
