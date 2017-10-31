/** \file   src/arch/gtk3/widgets/reuwidget.c
 * \brief   Widget to control RAM Expansion Module resources
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  REU
 *  REUsize
 *  REUfilename
 *  REUImageWrite
 *  REUIOSwap (xvic)
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
#include "savefiledialog.h"
#include "cartridge.h"

#include "reuwidget.h"


/** \brief  List of supported RAM sizes
 */
static ui_radiogroup_entry_t ram_sizes[] = {
    { "128KB", 128 },
    { "256KB", 256 },
    { "512KB", 512 },
    { "1MB", 1024 },
    { "2MB", 2048 },
    { "4MB", 4096 },
    { "8MB", 8192 },
    { "16MB", 16384 },
    { NULL, -1 }
};


/* list of widgets, used to enable/disable depending on GEORAM resource */
static GtkWidget *reu_enable_widget = NULL;
static GtkWidget *reu_size = NULL;
static GtkWidget *reu_ioswap = NULL;
static GtkWidget *reu_image = NULL;

static int (*reu_save_func)(int, const char *) = NULL;
static int (*reu_flush_func)(int) = NULL;


/** \brief  Handler for the "toggled" event of the reu_enable widget
 *
 * \param[in]   widget      check button
 * \param[in]   user_data   unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(reu_size, state);
    if (reu_ioswap != NULL) {
        gtk_widget_set_sensitive(reu_ioswap, state);
    }
    gtk_widget_set_sensitive(reu_image, state);
}


/** \brief  Handler for the "clicked" event of the "browse" button
 *
 * Select an image file for the GEORAM extension.
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_browse_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *filename;

    filename = ui_open_file_dialog(button,
            "Open RAM Expansion Module image file", NULL, NULL, NULL);
    if (filename != NULL) {
        GtkWidget *grid = gtk_widget_get_parent(button);
        GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
        gtk_entry_set_text(GTK_ENTRY(entry), filename);
        g_free(filename);
    }
}


/** \brief  Handler for the "clicked" event of the "save" button
 *
 * Save REU image file. Uses dirname()/basename() on the REUfilename
 * resource to act as a "Save" button, but also allows changing filename/dir
 * to act as a "Save As" button.
 *
 * \param[in]   button      save button
 * \param[in]   user_data   unused
 */
static void on_save_clicked(GtkWidget *button, gpointer user_data)
{
    const char *current_filename;
    gchar *new_filename;
    gchar *fname = NULL;
    gchar *dname = NULL;

    resources_get_string("REUfilename", &current_filename);
    if (current_filename != NULL && *current_filename != '\0') {
        /* provide the current filename and path */
        fname = g_path_get_basename(current_filename);
        dname = g_path_get_dirname(current_filename);
        debug_gtk3("got dir '%s', file '%s'\n", dname, fname);
    }

    new_filename = ui_save_file_dialog(button,
            "Save RAM Expansion Module image file",
            fname, TRUE, dname);
    if (new_filename != NULL) {
        debug_gtk3("writing RAM Expansion Module file image as '%s'\n",
                new_filename);
        /* write file */
        if (reu_save_func != NULL) {
            if (reu_save_func(CARTRIDGE_REU, new_filename) < 0) {
                /* oops */
                ui_message_error(button, "I/O error",
                        "Failed to save '%s'", new_filename);
            }
        } else {
            ui_message_error(button, "Core error",
                    "RAM Expansion Module save handler not specified");
        }
        g_free(new_filename);
    }

    if (fname != NULL) {
        g_free(fname);
    }
    if (dname != NULL) {
        g_free(dname);
    }
}

/** \brief  Handler for the "clicked" event of the "Flush image" button
 *
 * \param[in]   widget      button triggering the event
 * \param[in]   user_data   unused
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (reu_flush_func != NULL) {
        if (reu_flush_func(CARTRIDGE_REU) < 0) {
            ui_message_error(widget, "I/O error", "Failed to flush image");
        }
    } else {
        ui_message_error(widget, "Core error",
                "RAM Expansion Module flush handler not specified");
    }
}


/** \brief  Create REU enable check button
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_reu_enable_widget(void)
{
    return resource_check_button_create("REU", "Enable RAM Expansion Module");
}


/** \brief  Create IO-swap check button (seems to be valid for xvic only)
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_reu_ioswap_widget(void)
{
    GtkWidget *check;

    check = resource_check_button_create("REUIOSwap", "MasC=uarade I/O swap");
    return check;
}


/** \brief  Create radio button group to determine GEORAM RAM size
 *
 * \return  GtkGrid
 */
static GtkWidget *create_reu_size_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("RAM Size", 1);
    radio_group = resource_radiogroup_create("REUsize", ram_sizes,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to load/save GEORAM image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_reu_image_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *auto_save;
    GtkWidget *save_button;
    GtkWidget *flush_button;

    grid = uihelpers_create_grid_with_label("Image file", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    entry = resource_entry_create("REUfilename");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    auto_save = resource_check_button_create("REUImageWrite",
            "Write image on image detach/emulator quit");
    g_object_set(auto_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), auto_save, 0, 2, 2, 1);

    save_button = gtk_button_new_with_label("Save ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 2, 2, 1, 1);

    flush_button = gtk_button_new_with_label("Flush image");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 3, 1, 1);


    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked), NULL);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control RAM Expansion Module resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *reu_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    reu_enable_widget = create_reu_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), reu_enable_widget, 0, 0, 1, 1);

    if (machine_class == VICE_MACHINE_VIC20) {
        reu_ioswap = create_reu_ioswap_widget();
        gtk_grid_attach(GTK_GRID(grid), reu_ioswap, 0, 1, 1, 1);
    }

    reu_size = create_reu_size_widget();
    gtk_grid_attach(GTK_GRID(grid), reu_size, 0, 1, 1, 1);

    reu_image = create_reu_image_widget();
    gtk_grid_attach(GTK_GRID(grid), reu_image, 1, 1, 1, 1);

    g_signal_connect(reu_enable_widget, "toggled", G_CALLBACK(on_enable_toggled),
            NULL);

    /* enable/disable widget based on reu-enable (dirty trick, I know) */
    on_enable_toggled(reu_enable_widget, NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set save function for the RAM Module Extension
 *
 * \param[in]   func    save function
 */
void reu_widget_set_save_handler(int (*func)(int, const char *))
{
    reu_save_func = func;
}


/** \brief  Set flush function for the RAM Module Extension
 *
 * \param[in]   func    flush function
 */
void reu_widget_set_flush_handler(int (*func)(int))
{
    reu_flush_func = func;
}

