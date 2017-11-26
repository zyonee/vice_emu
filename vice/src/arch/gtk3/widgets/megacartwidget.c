/** \file   src/arch/gtk3/widgets/megacartwidget.c
 * \brief   VIC-20 Mega Cart widget
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  MegaCartNvRAMfilename (xvic)
 *  MegaCartNvRAMWriteBack (xvic)
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

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "openfiledialog.h"

#include "megacartwidget.h"



/** \brief  Handler for the "clicked" event of the browse button
 *
 * Activates a file-open dialog and stores the file name in the GtkEntry passed
 * in \a user_data if valid, triggering a resource update.
 *
 * \param[in]   widget      button
 * \param[in]   user_data   entry to store filename in
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = ui_open_file_dialog(widget, "Open NvRAM image file", NULL, NULL,
            NULL);
    if (filename != NULL) {
        GtkEntry *entry = GTK_ENTRY(user_data);
        debug_gtk3("setting MegaCartNvRAMfilename to '%s'\n", filename);
        gtk_entry_set_text(entry, filename);
        g_free(filename);
    }
}


/** \brief  Create widget to control IEEE-488 adapter
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *mega_cart_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *write_back;
    GtkWidget *label;
    GtkWidget *entry = NULL;
    GtkWidget *browse = NULL;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("NvRAM image file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    entry = resource_entry_create("MegaCartNvRAMfilename");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 0, 1, 1);

    write_back = resource_check_button_create("MegaCartNvRAMWriteBack",
            "Enable Mega Cart NvRAM write back");
    gtk_grid_attach(GTK_GRID(grid), write_back, 0, 1, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}
