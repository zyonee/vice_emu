/** \file   src/arch/gtk3/widgets/dqbb.c
 * \brief   Widget to control Double Quick Brown Box resources
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  DQBB (x64/x64sc/xscpu64/x128)
 *  DQBBfilename (x64/x64sc/xscpu64/x128)
 *  DQBBImageWrite (x64/x64sc/xscpu64/x128)
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
#include "cartimagewidget.h"
#include "cartridge.h"
#include "carthelpers.h"

#include "dqbbwidget.h"


/* list of widgets, used to enable/disable depending on Double Quick Brown Box
 * resources */
static GtkWidget *dqbb_enable_widget = NULL;
static GtkWidget *dqbb_image = NULL;


/** \brief  Handler for the "toggled" event of the dqbb_enable widget
 *
 * \param[in]   widget      check button
 * \param[in]   user_data   unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(dqbb_image, state);
}


/** \brief  Create Double Quick Brown Box enable check button
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_dqbb_enable_widget(void)
{
    GtkWidget *check;

    check = resource_check_button_create("DQBB",
            "Enable Double Quick Brown Box");
    return check;
}


/** \brief  Create widget to load/save Double Quick Brown Box image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_dqbb_image_widget(GtkWidget *parent)
{
    return cart_image_widget_create(parent, "DQBB image",
            "DQBBfilename", "DQBBImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_DQBB, CARTRIDGE_DQBB);
}


/** \brief  Create widget to control Double Quick Brown Box resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *dqbb_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    dqbb_enable_widget = create_dqbb_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), dqbb_enable_widget, 0, 0, 1, 1);

    dqbb_image = create_dqbb_image_widget(parent);
    gtk_grid_attach(GTK_GRID(grid), dqbb_image, 0, 1, 1, 1);

    g_signal_connect(dqbb_enable_widget, "toggled", G_CALLBACK(on_enable_toggled),
            NULL);

    /* enable/disable widget based on dqbb-enable (dirty trick, I know) */
    on_enable_toggled(dqbb_enable_widget, NULL);

    gtk_widget_show_all(grid);
    return grid;
}
