/*
 * uismartattach.c - GTK3 smart-attach dialog
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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

#include "uismartattach.h"


/** \brief  Custom response ID's for the dialog
 *
 * Negative values are reserved by Gtk to handle stock responses
 */
enum {
    RESPONSE_AUTOSTART = 1  /**< Autostart button clicked */
};


/* TODO: move these patterns into widgets/filechooserhelpers.c to be able to
 *       use them as 'stock' patterns
 */
const gchar *pattern_all[] = { "*.*", NULL };
const gchar *pattern_disk[] = {
    "*.d64", "*.d67", "*.d71", "*.d8[0-2]",
    "*.d1m", "*.d2m", "*.d4m",
    "*.g64", "*.g71", "*.g41", "*.p64",
    "*.x64",
    NULL
};
const gchar *pattern_tape[] = { "*.t64", "*.tap", NULL };   /* T64 is NOT a tape */
const gchar *pattern_prg[] = { "*.prg", "*.p[0-9][0-9]" };
const gchar *pattern_zip[] = { "*.bz2", "*.gz", ".rar", "*.[zZ]", "*.zip" };

static ui_file_filter_t filters[] = {
    { "All files", pattern_all },
    { "Disk images", pattern_disk },
    { "Tape images", pattern_tape },
    { "Program files", pattern_prg },
    { "Compressed files", pattern_zip },
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
 * \param[in]   user_data   reponse ID (`int`)
 *
 * TODO:    proper (error) messages, which requires implementing ui_error() and
 *          ui_message() and moving them into gtk3/widgets to avoid circular
 *          references
 */
static void on_response(GtkWidget *widget, gpointer user_data)
{
    int response_id = GPOINTER_TO_INT(user_data);
    gchar *filename;

    debug_gtk3("got response ID %d\n", response_id);

    switch (response_id) {

        /* 'Open' button, double-click on file */
        case GTK_RESPONSE_ACCEPT:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            /* ui_message("Opening file '%s' ...", filename); */
            debug_gtk3("Opening file '%s'\n", filename);

            /* copied from Gtk2: I fail to see how brute-forcing your way
             * through file types is 'smart', but hell, it works */
            if (file_system_attach_disk(8, filename) < 0
                    && tape_image_attach(1, filename) < 0
                    && autostart_snapshot(filename, NULL) < 0
                    && autostart_prg(filename, AUTOSTART_MODE_LOAD) < 0) {
                /* failed */
                debug_gtk3("smart attach failed\n");
            }

            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Autostart' button clicked */
        case RESPONSE_AUTOSTART:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            debug_gtk3("Autostarting file '%s'\n", filename);
            /* if this function exists, why is there no attach_autodetect()
             * or something similar? -- compyx */
            if (autostart_autodetect(
                        filename,
                        NULL,   /* program name */
                        0,      /* Program number? Probably used when clicking
                                   in the preview widget to load the proper
                                   file in an image */
                        AUTOSTART_MODE_RUN) < 0) {
                /* oeps */
                debug_gtk3("autostart-smart-attach failed\n");
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


/** \brief  Create the smart-attach dialog
 *
 * \param[in]   parent  parent widget, used to get the top level window
 *
 * \return  GtkFileChooserDialog
 */
static GtkWidget *create_smart_attach_dialog(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *preview;
    size_t i;

    /* create new dialog */
    dialog = gtk_file_chooser_dialog_new(
            "Smart-attach a file",
            GTK_WINDOW(gtk_widget_get_toplevel(parent)),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            /* buttons */
            "Open", GTK_RESPONSE_ACCEPT,
            "Autostart", RESPONSE_AUTOSTART,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    /* add 'extra' widget: 'readony' and 'show preview' checkboxes */
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
            create_extra_widget(dialog));

    preview = create_content_preview_widget(NULL);
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);

    /* add filters */
    for (i = 0; filters[i].name != NULL; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                create_file_chooser_filter(filters[i]));
    }

    /* connect "reponse" handler: the `user_data` argument gets filled in when
     * the "response" signal is emitted: a response ID */
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    return dialog;

}


/** \brief  Callback for the File menu's "smart-attach" item
 *
 * Creates the smart-dialog and runs it.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   data for the event (unused)
 */
void ui_smart_attach_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    debug_gtk3("called\n");

    dialog = create_smart_attach_dialog(widget);

    gtk_widget_show(dialog);

}





