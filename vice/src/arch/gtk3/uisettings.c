/*
 * uisettings.c - GTK3 settings dialog
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


/* The settings_grid is supposed to become this:
 *
 * +------------+-------------------------+
 * | treeview   |                         |
 * |  with      |                         |
 * |   settings |   central widget,       |
 * |  more      |   depending on which    |
 * |   foo      |   item is selected in   |
 * |   bar      |   the treeview          |
 * |    whatever|                         |
 * | burp       |                         |
 * +------------+-------------------------+
 * | load  save  load...  save...  close  |
 * +--------------------------------------+
 */


#include "vice.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib.h"
#include "resources.h"
#include "vsync.h"

#include "widgethelpers.h"
#include "uispeed.h"


#include "uisettings.h"


#define NUM_COLUMNS 1



static void on_load_clicked(GtkWidget *widget, gpointer data);
static void on_save_clicked(GtkWidget *widget, gpointer data);
static void on_load_file_clicked(GtkWidget *widget, gpointer data);
static void on_save_file_clicked(GtkWidget *widget, gpointer data);
static void on_close_clicked(GtkWidget *widget, gpointer data);




#if 0
typedef struct button_info_s {
    const gchar *text;
    void (*callback)(GtkWidget *widget, gpointer user_data);
} button_info_t;
#endif

static ui_button_t buttons[] = {
    { "Load", on_load_clicked },
    { "Save", on_load_file_clicked },
    { "Load from ...", on_save_clicked },
    { "Save as ..", on_save_file_clicked },
    { "Close", on_close_clicked },
    { NULL, NULL }
};


static GtkWidget *settings_window = NULL;
static GtkWidget *settings_grid = NULL;


static void on_load_clicked(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif
}


static void on_save_clicked(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif
}


static void on_load_file_clicked(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif
}


static void on_save_file_clicked(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif
}


static void on_close_clicked(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(settings_window);
}



/* TODO:    find a non-convoluted way to add items with callbacks setting the
 *          proper 'central widget'
 */
static GtkWidget *create_treeview(void)
{
    GtkWidget *tree;
    GtkTreeStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeIter iter;   /* parent iter */
#if 0
    GtkTreeIter child;  /* child iter */
#endif

    /* create the model */
    store = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING);

    /* add root node */
    gtk_tree_store_append(store, &iter, NULL);

    gtk_tree_store_set(
            store, &iter,
            0, "Speed",
            -1);

    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
        NULL,
        renderer,
        "text", 0,
        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    return tree;
}


/** \brief  Set the 'central'/action widget for the settings dialog
 *
 * Destroys the old 'central' widget and sets the new one.
 *
 *  \param[in,out]  widget  widget to use as the new 'central' widget
 */
static void ui_settings_set_central_widget(GtkWidget *widget)
{
    GtkWidget *child;

    child = gtk_grid_get_child_at(GTK_GRID(settings_grid), 1, 0);
    if (child != NULL) {
        gtk_widget_destroy(child);
    }

    gtk_grid_attach(GTK_GRID(settings_grid), widget, 1, 0, 1, 1);
}



/** \brief  Setup the settings dialog, called from a menu item
 */
void ui_settings_dialog_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *tree;
    GtkWidget *parent;

#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif


    settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(settings_window), TRUE);
    gtk_window_set_title(GTK_WINDOW(settings_window), "VICE settings");


    /* make the settings window a child of the toplevel window
     *
     * FIXME:   Does block the parent's input, but doesn't keep the window on
     *          top of its parent, so perhaps use a GtkDialog, which is very
     *          limited in use?
     */
    parent = gtk_widget_get_toplevel(widget);
    if (gtk_widget_is_toplevel(parent)) {
        gtk_window_set_transient_for(
                GTK_WINDOW(settings_window),
                GTK_WINDOW(parent));
    }

    settings_grid = gtk_grid_new();
    tree = create_treeview();
    gtk_grid_attach(GTK_GRID(settings_grid), tree, 0, 0, 1, 1);

    ui_settings_set_central_widget(uispeed_create_central_widget());

    gtk_grid_attach(GTK_GRID(settings_grid),
            uihelpers_create_button_box(buttons, GTK_ORIENTATION_HORIZONTAL),
            0, 1, 2, 1);



    gtk_widget_show(settings_grid);
    gtk_widget_show(tree);


    gtk_container_add(GTK_CONTAINER(settings_window), settings_grid);
    gtk_widget_show(settings_window);
}
