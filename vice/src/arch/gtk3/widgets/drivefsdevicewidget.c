/** \file   src/arch/gtk3/widgets/drivefsdevicewidget.c
 * \brief   Drive file system device widget
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  FileSystemDevice[8-11]
 *  FSDevice[8-11]ConvertP00
 *  FSDevice[8-11]SaveP00
 *  FSDevice[8-11]HideCBMFiles
 *  FSDevice[8-11]Dir
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
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "drive.h"
#include "drive-check.h"
#include "machine.h"
#include "attach.h"
#include "selectdirectorydialog.h"

#include "drivefsdevicewidget.h"


/** \brief  List of file system types
 */
static ui_combo_entry_int_t device_types[] = {
    { "None", ATTACH_DEVICE_NONE },
    { "File system", ATTACH_DEVICE_FS },
#ifdef HAVE_REALDEVICE
    { "Real device (OpenCBM)", ATTACH_DEVICE_REAL },
#endif
    { "Block device (RAW)", ATTACH_DEVICE_RAW },
    { NULL, -1 }
};

/*
 * TODO:    refactor the fsdir entry code into a `resourceentrywidget` in
 *          src/arch/gtk3/widgets/base
 */

/** \brief  Handler for "destroy" event of the fsdir entry widget
 *
 * Frees memory allocated for the resource name
 *
 * \param[in]   widget      fs dir entry widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_fsdir_entry_destroy(GtkWidget *widget, gpointer user_data)
{
    char *resource;

    resource = (char *)g_object_get_data(G_OBJECT(widget), "ResourceName");
    lib_free(resource);
}


/** \brief  Handler for the "changed" event of the fs dir entry widget
 *
 * \param[in]   widget      fs dir entry widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_fsdir_entry_changed(GtkWidget *widget, gpointer user_data)
{
    const char *resource;

    resource = (const char *)g_object_get_data(G_OBJECT(widget), "ResourceName");
    resources_set_string(resource, gtk_entry_get_text(GTK_ENTRY(widget)));
}


/** \brief  Handler for the "clicked" event of the fs dir browse button
 *
 * \param[in]   widget      fs dir browse button
 * \param[in]   user_data   extra event data (entry widget)
 */
static void on_fsdir_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    GtkEntry *entry = GTK_ENTRY(user_data);
    gchar *filename;

    filename = ui_select_directory_dialog(widget,
            "Select file system directory",
            NULL, TRUE, NULL);
    if (filename != NULL) {
        gtk_entry_set_text(entry, filename);
        g_free(filename);
    }
}



/** \brief  Create a combo box with device types
 *
 * \param[in]   unit    unit number
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_device_type_widget(int unit)
{
    GtkWidget *combo;
    char resource[256];

    g_snprintf(resource, 256, "FileSystemDevice%d", unit);

    combo = resource_combo_box_int_create(resource, device_types);
    gtk_widget_show(combo);
    return combo;
}


/** \brief  Create text entry for file system directory for \a unit
 *
 * \param[in]   unit    unit number
 *
 * \return  GtkEntry
 */
static GtkWidget *create_fsdir_entry_widget(int unit)
{
    GtkWidget *entry;
    char resource[256];
    const char *dir;

    g_snprintf(resource, 256, "FSDevice%dDir", unit);

    entry = gtk_entry_new();
    g_object_set_data(G_OBJECT(entry), "ResourceName",
            (gpointer)lib_stralloc(resource));
    if (resources_get_string(resource, &dir) < 0) {
        debug_gtk3("warning: couldn't retrieve value for resource '%s'\n",
                resource);
        dir = NULL;
    }
    gtk_entry_set_text(GTK_ENTRY(entry), dir);

    g_signal_connect(entry, "changed", G_CALLBACK(on_fsdir_entry_changed),
            NULL);
    g_signal_connect(entry, "destroy", G_CALLBACK(on_fsdir_entry_destroy),
            NULL);

    gtk_widget_show(entry);
    return entry;
}


/** \brief  Create widget to control P00-settings
 *
 * \param[in]   unit    unit number
 *
 * \return  GtkGrid
 */
static GtkWidget *create_p00_widget(int unit)
{
    GtkWidget *grid;
    GtkWidget *p00_convert;
    GtkWidget *p00_only;
    GtkWidget *p00_save;
    char resource[256];

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    g_snprintf(resource, 256, "FSDevice%dConvertP00", unit);
    p00_convert = resource_check_button_create(resource, "Convert P00");
    gtk_grid_attach(GTK_GRID(grid), p00_convert, 0, 0, 1, 1);

    g_snprintf(resource, 256, "FSDevice%dSaveP00", unit);
    p00_save = resource_check_button_create(resource, "Save P00");
    gtk_grid_attach(GTK_GRID(grid), p00_save, 1, 0, 1, 1);

    g_snprintf(resource, 256, "FSDevice%dHideCBMFiles", unit);
    p00_only = resource_check_button_create(resource, "Hide non-P00");
    gtk_grid_attach(GTK_GRID(grid), p00_only, 2, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control file system device settings of \a unit
 *
 * \param[in]   unit    unit number
 *
 * \return  GtkGrid
 */
GtkWidget *drive_fsdevice_widget_create(int unit)
{
    GtkWidget *grid;
    GtkWidget *combo;
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *browse;
    GtkWidget *p00;

    grid = uihelpers_create_grid_with_label("FS Device settings", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("device type");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    combo = create_device_type_widget(unit);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 1, 2, 1);

    label = gtk_label_new("directory");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    entry = create_fsdir_entry_widget(unit);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_fsdir_browse_clicked),
            (gpointer)entry);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 2, 1, 1);

    p00 = create_p00_widget(unit);
    g_object_set(p00, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), p00, 0, 3, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}
