/** \file   src/arch/gtk3/widgets/driveidlemethodwidget.c
 * \brief   Drive expansions widget
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  Drive[8-11]IdleMethod
 *
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

#include <gtk/gtk.h>

#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "drive.h"
#include "drive-check.h"

#include "driveidlemethodwidget.h"



/** \brief  Idle method (name,id) tuples
 */
static ui_combo_entry_int_t idle_methods[] = {
    { "None", 0 },
    { "Skip cycles", 1 },
    { "Trap idle", 2 },
    { NULL, -1 }
};


/** \brief  Create widget to set the drive idle method
 *
 * \param[in]   unit    current drive unit number
 *
 * \return  GtkGrid
 */
GtkWidget *drive_idle_method_widget_create(int unit)
{
    GtkWidget *grid;
    GtkWidget *combo;

    grid = uihelpers_create_grid_with_label("Idle method", 1);
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));

    combo = resource_combo_box_int_create_sprintf("Drive%dIdleMethod",
            idle_methods, unit);
    gtk_widget_set_hexpand(combo, TRUE);
    g_object_set(combo, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
