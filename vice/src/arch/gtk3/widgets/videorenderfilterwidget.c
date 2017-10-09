/** \file   src/arch/gtk3/videorenderfilterwidget.c
 * \brief   Widget to select renderer filter
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  CrtcFilter
 *  TEDFilter
 *  VDCFilter
 *  VICFilter
 *  VICIIFilter
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

#include "debug_gtk3.h"
#include "widgethelpers.h"
#include "resources.h"
#include "video.h"

#include "videorenderfilterwidget.h"


/** \brief  Video chip prefix, used in the resource getting/setting
 */
static const char *chip_prefix;


/** \brief  List of radio buttons
 */
static ui_radiogroup_entry_t filters[] = {
    { "Unfiltered",     VIDEO_FILTER_NONE },
    { "CRT emulation",  VIDEO_FILTER_CRT },
    { "Scale2x",        VIDEO_FILTER_SCALE2X },
    { NULL, -1 }
};


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   new value for render filter (`int1)
 */
static void on_render_filter_toggled(GtkWidget *widget, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        int value = GPOINTER_TO_INT(user_data);

        debug_gtk3("setting %sFilter to %d\n", chip_prefix, value);
        resources_set_int_sprintf("%sFilter", value, chip_prefix);
    }
}


/** \brief  Create widget to control render filter resources
 *
 * \param[in]   chip    video chip prefix
 *
 * \return  GtkGrid
 */
GtkWidget *video_render_filter_widget_create(const char *chip)
{
    GtkWidget *grid;
    int current;

    chip_prefix = chip;
    resources_get_int_sprintf("%Filter", &current, chip);

    grid = uihelpers_radiogroup_create("Render filter", filters,
            on_render_filter_toggled, current);
    gtk_widget_show_all(grid);
    return grid;
}
