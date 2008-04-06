/*
 * petui.cc - PET-specific user interface.
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
#include "constants.h"
#include "petui.h"
#include "ui.h"
#include "ui_pet.h"
}

ui_menu_toggle  pet_ui_menu_toggles[]={
    { "CrtcDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "CrtcDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "CrtcVideoCache", MENU_TOGGLE_VIDEOCACHE },
    { NULL, 0 }
};

void pet_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
      case MENU_PET_SETTINGS:
        ui_pet();
        break;
      default: ;
    }
}


int pet_ui_init(void)
{
    ui_register_machine_specific(pet_ui_specific);
    ui_register_menu_toggles(pet_ui_menu_toggles);
    ui_update_menus();
    return 0;
}

