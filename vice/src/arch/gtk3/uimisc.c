/** \file   src/arch/gtk3/ui_misc.c
 * \brief   Widget to control resources that are hard to place properly
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  VirtualDevices  - enable virtual devices/device traps
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"

#include "htmlbrowserwidget.h"
#include "cwdwidget.h"

#include "uimisc.h"



GtkWidget *uimisc_create_central_widget(GtkWidget *widget)
{
    GtkWidget *grid;
    GtkWidget *browser = html_browser_widget_create();
    GtkWidget *cwd_widget = cwd_widget_create();
    GtkWidget *vdev_widget = uihelpers_create_resource_checkbox(
            "Enable virtual devices", "VirtualDevices");

    grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), browser, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), cwd_widget, 0, 1, 1, 1);
    g_object_set(vdev_widget, "margin-left",8, NULL);
    gtk_grid_attach(GTK_GRID(grid), vdev_widget, 0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



