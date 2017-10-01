/*
 * petui.c - Native GTK3 PET UI.
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
#include "petmodel.h"
#include "machinemodelwidget.h"
#include "petkeyboardtypewidget.h"

#include "petui.h"


static const char *pet_model_list[] = {
    "PET 2001-8N", "PET 3008", "PET 3016", "PET 3032", "PET 3032B", "PET 4016",
    "PET 4032", "PET 4032B", "PET 8032", "PET 8096", "PET 8296", "SuperPET",
    NULL
};


int petui_init(void)
{
    /* Some of the work here is done by video.c now, and would need to
     * be shifted over */

    machine_model_widget_getter(petmodel_get);
    machine_model_widget_setter(petmodel_set);
    machine_model_widget_set_models(pet_model_list);

    pet_keyboard_type_widget_set_keyboard_num_get(machine_get_num_keyboard_types);
    pet_keyboard_type_widget_set_keyboard_list_get(machine_get_keyboard_info_list);

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}

void petui_shutdown(void)
{
    INCOMPLETE_IMPLEMENTATION();
}

