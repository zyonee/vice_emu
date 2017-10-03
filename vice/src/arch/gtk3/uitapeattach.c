/*
 * uitapeattach.c - GTK3 tape-attach dialog
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *  Michael C. Martin <mcmartin@gmail.com>
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

#include "attach.h"
#include "autostart.h"
#include "tape.h"
#include "debug_gtk3.h"
#include "contentpreviewwidget.h"
#include "filechooserhelpers.h"

#include "uitapeattach.h"


/** \brief  Custom response ID's for the dialog
 *
 * Negative values are reserved by Gtk to handle stock responses
 */
enum {
    RESPONSE_AUTOSTART = 1  /**< Autostart button clicked */
};


/** \brief  File type filters for the dialog
 */
static ui_file_filter_t filters[] = {
    { "All files", file_chooser_pattern_all },
    { "Tape images", file_chooser_pattern_tape },
    { NULL, NULL }
};


/** \brief  Handler for the 'toggled' event of the 'show hidden files' checkbox
 *
 * \param[in]   widget      checkbox triggering the event
 * \param[in]   user_data   data for the event (the dialog)
 */
static void on_hidden_toggled(GtkWidget *widget, gpointer user_data)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    debug_gtk3("show hidden files: %s\n", state ? "enabled" : "disabled");

    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(user_data), state);
}



/** \brief  Handler for the 'toggled' event of the 'show preview' checkbox
 *
 * \param[in]   widget      checkbox triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void on_preview_toggled(GtkWidget *widget, gpointer user_data)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    debug_gtk3("preview %s\n", state ? "enabled" : "disabled");
}


/** \brief  Handler for 'response' event of the dialog
 *
 * This handler is called when the user clicks a button in the dialog.
 *
 * \param[in]   widget      the dialog
 * \param[in]   response_id response ID
 * \param[in]   user_data   extra data (unused)
 *
 * TODO:    proper (error) messages, which requires implementing ui_error() and
 *          ui_message() and moving them into gtk3/widgets to avoid circular
 *          references
 */
static void on_response(GtkWidget *widget, gint response_id, gpointer user_data)
{
    gchar *filename;

    debug_gtk3("got response ID %d\n", response_id);

    switch (response_id) {

        /* 'Open' button, double-click on file */
        case GTK_RESPONSE_ACCEPT:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            /* ui_message("Opening file '%s' ...", filename); */
            debug_gtk3("Attaching file '%s' to tape unit\n", filename);

            /* copied from Gtk2: I fail to see how brute-forcing your way
             * through file types is 'smart', but hell, it works */
            if (tape_image_attach(1, filename) < 0) {
                /* failed */
                debug_gtk3("tape attach failed\n");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Autostart' button clicked */
        case RESPONSE_AUTOSTART:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            debug_gtk3("Autostarting file '%s'\n", filename);
            if (autostart_tape(
                        filename,
                        NULL,   /* program name */
                        0,      /* Program number? Probably used when clicking
                                   in the preview widget to load the proper
                                   file in an image */
                        AUTOSTART_MODE_RUN) < 0) {
                /* oeps */
                debug_gtk3("autostart tape attach failed\n");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Close'/'X' button */
        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            break;
    }
}


/** \brief  Create the 'extra' widget
 *
 * \return  GtkGrid
 *
 * TODO: 'grey-out'/disable units without a proper drive attached
 */
static GtkWidget *create_extra_widget(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *hidden_check;
    GtkWidget *readonly_check;
    GtkWidget *preview_check;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    hidden_check = gtk_check_button_new_with_label("Show hidden files");
    g_signal_connect(hidden_check, "toggled", G_CALLBACK(on_hidden_toggled),
            (gpointer)(parent));
    gtk_grid_attach(GTK_GRID(grid), hidden_check, 0, 0, 1, 1);

    readonly_check = gtk_check_button_new_with_label("Attach read-only");
    gtk_grid_attach(GTK_GRID(grid), readonly_check, 1, 0, 1, 1);

    preview_check = gtk_check_button_new_with_label("Show image contents");
    g_signal_connect(preview_check, "toggled", G_CALLBACK(on_preview_toggled),
            NULL);
    gtk_grid_attach(GTK_GRID(grid), preview_check, 2, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the tape attach dialog
 *
 * \param[in]   parent  parent widget, used to get the top level window
 *
 * \return  GtkFileChooserDialog
 */
static GtkWidget *create_tape_attach_dialog(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *preview;
    size_t i;

    /* create new dialog */
    dialog = gtk_file_chooser_dialog_new(
            "Attach a tape image",
            GTK_WINDOW(gtk_widget_get_toplevel(parent)),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            /* buttons */
            "Open", GTK_RESPONSE_ACCEPT,
            "Autostart", RESPONSE_AUTOSTART,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    /* add 'extra' widget: 'readonly' and 'show preview' checkboxes */
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
                                      create_extra_widget(dialog));

    preview = content_preview_widget_create(NULL);
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);

    /* add filters */
    for (i = 0; filters[i].name != NULL; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                create_file_chooser_filter(filters[i], TRUE));
    }

    /* connect "reponse" handler: the `user_data` argument gets filled in when
     * the "response" signal is emitted: a response ID */
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    return dialog;

}


/** \brief  Callback for the "attach tape image" menu items
 *
 * Creates the dialog and runs it.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   ignored
 */
void ui_tape_attach_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    debug_gtk3("called\n");
    dialog = create_tape_attach_dialog(widget);
    gtk_widget_show(dialog);

}

/** \brief  Callback for "detach tape image" menu items
 *
 * Removes any tape from the specified drive. No additional UI is
 * presented.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   ignored
 */
void ui_tape_detach_callback(GtkWidget *widget, gpointer user_data)
{
    tape_image_detach(1);
}
