/** \file   src/arch/gtk3/widgets/retroreplaywidget.c
 * \brief   Widget to control Retro Replay resources
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  RRFlashJumper (x64/x64sc/xscpu64/x128)
 *  RRBankJumper (x64/x64sc/xscpu64/x128)
 *  RRBiosWrite (x64/x64sc/xscpu64/x128)
 *  RRrevision (x64/x64sc/xscpu64/x128)
 *  RRClockPort (x64/x64sc/xscpu64/x128)
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
#include "cartimagewidget.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "clockportdevicewidget.h"
#include "cartridge.h"

#include "retroreplaywidget.h"


/** \brief  List of Retro Replay revisions
 */
static ui_combo_entry_int_t rr_revisions[] = {
    { CARTRIDGE_NAME_RETRO_REPLAY, 0 },
    { CARTRIDGE_NAME_NORDIC_REPLAY, 1 },
    { NULL, -1 }
};



static int (*save_func)(int, const char *) = NULL;
static int (*flush_func)(int) = NULL;


/** \brief  Handler for the "clicked" event of the "Save As" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_save_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = ui_save_file_dialog(widget, "Save image as", NULL, TRUE, NULL);
    if (filename != NULL) {
        debug_gtk3("writing RR image file as '%s'\n", filename);
        if (save_func(CARTRIDGE_RETRO_REPLAY, filename) < 0) {
            /* ui_error("Failed to save RR image '%s'", filename); */
        }
        g_free(filename);
    }
}


/** \brief  Handler for the "clicked" event of the "Flush now" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    debug_gtk3("flushing RR image\n");
    if (flush_func(CARTRIDGE_RETRO_REPLAY) < 0) {
        /* TODO: report error */
    }
}


/** \brief  Create widget to control Retro Replay resources
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *retroreplay_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *flash;
    GtkWidget *bank;
    GtkWidget *label;
    GtkWidget *rev_combo;
    GtkWidget *cp_combo;
    GtkWidget *save_button;
    GtkWidget *flush_button;
    GtkWidget *bios_write;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /* RRFlashJumper */
    flash = resource_check_button_create("RRFlashJumper",
            "Enable flash jumper");
    gtk_grid_attach(GTK_GRID(grid), flash, 0, 0, 1, 1);

    /* RRBankJumper */
    bank = resource_check_button_create("RRBankJumper",
            "Enable bank jumper");
    gtk_grid_attach(GTK_GRID(grid), bank, 0, 1, 1, 1);

    /* RRrevision */
    label = gtk_label_new("Revision");
    g_object_set(label, "margin-left", 8, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    rev_combo = resource_combo_box_int_create("RRrevision", rr_revisions);
    gtk_grid_attach(GTK_GRID(grid), rev_combo, 2, 0, 1, 1);

    /* RRClockPort */
    label = gtk_label_new("Clockport device");
    g_object_set(label, "margin-left", 8, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 1, 1, 1);
    cp_combo = clockport_device_widget_create("RRClockPort");
    gtk_grid_attach(GTK_GRID(grid), cp_combo, 2, 1, 1, 1);

    /* RRBiosWrite */
    bios_write = resource_check_button_create("RRBiosWrite",
            "Write back RR Flash ROM image automatically");
    gtk_grid_attach(GTK_GRID(grid), bios_write, 0, 2, 2, 1);

    /* Save image as... */
    save_button = gtk_button_new_with_label("Save image as ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 2, 2, 1, 1);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked),
            NULL);

    /* Flush image now */
    flush_button = gtk_button_new_with_label("Flush image now");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 3, 1, 1);
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set function to save RR image to disk
 *
 * \param[in]   func    function to save image
 */
void retroreplay_widget_set_save_func(int (*func)(int, const char *))
{
    save_func = func;
}


/** \brief  Set function to flush RR image to disk
 *
 * \param[in]   func    function to flush image
 */
void retroreplay_widget_set_flush_func(int (*func)(int))
{
    flush_func = func;
}
