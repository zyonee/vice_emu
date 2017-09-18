/** \file   src/arch/gtk3/widgets/driveparallelcablewidget.c
 * \brief   Drive parallel cable widget
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  Drive[8-11]ParallelCable
 *
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
#include "drive.h"
#include "drive-check.h"
#include "machine.h"

#include "driveparallelcablewidget.h"


/** \brief  The current unit number
 */
static int unit_number = 8;


/** \brief  List of possible parallel cables
 */
static ui_text_int_pair_t parallel_cables[] = {
    { "None", 0 },
    { "Standard", 1 },
    { "Professional DOS", 2 },
    { "Formel 64", 3 },
    { NULL, -1 }
};


/** \brief  Handler for "toggled" event of the radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   parallel cable ID (0-3) (int)
 */
static void on_parallel_cable_changed(GtkWidget *widget, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        char res_name[256];
        int value = GPOINTER_TO_INT(user_data);

        g_snprintf(res_name, 256, "Drive%dParallelCable", unit_number);
        debug_gtk3("setting %s to %d\n", res_name, value);
        resources_set_int(res_name, value);
    }
}


/** \brief  Get drive type for \a unit
 *
 * \param[in]   unit
 *
 * \return  drive type
 */
static int get_drive_type(int unit)
{
    char buffer[256];
    int type;

    g_snprintf(buffer, 256, "Drive%dType", unit);
    resources_get_int(buffer, &type);

    return type;
}


/** \brief  Create drive parallel cable widget
 *
 * \param[in]   unit    drive unit
 *
 * \return  GtkGrid
 */
GtkWidget *create_drive_parallel_cable_widget(int unit)
{
    GtkWidget *widget;
    int i;

    unit_number = unit;

    widget = uihelpers_create_int_radiogroup_with_label(
            "Parallel cable",
            parallel_cables,
            NULL,   /* NULL: connect event handlers after setting the value */
            0);

    /* first update the widget */
    update_drive_parallel_cable_widget(widget, unit);

    /* now connect the signal handlers */
    for (i = 0; parallel_cables[i].text != NULL; i++) {
        GtkWidget *radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, i + 1);
        if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)) {
            g_signal_connect(radio, "toggled",
                    G_CALLBACK(on_parallel_cable_changed),
                    GINT_TO_POINTER(parallel_cables[i].value));
        }
    }

    return widget;
}


/** \brief  Update the widget
 *
 * Enables/disable both the widget and its children depending on the drive type
 * and the machine class.
 *
 * \param[in,out]   widget  drive parallel cable widget
 * \param[in]       unit    drive unit number
 */
void update_drive_parallel_cable_widget(GtkWidget *widget, int unit)
{
    char res_name[256];
    int cable_type = 0; /* cable type, if set to < 0 causes the cable type
                           to revert to None (used for machines/drives that
                           don't support drive parallel cables */
    int drive_type;
    GtkWidget *radio;
    int enabled;
    int count;
    int i;

    debug_gtk3("called with unit #%d\n", unit);

    unit_number = unit;

    /* determine if parallel cables are supported by the currently selected
     * drive type */
    drive_type = get_drive_type(unit);
    gtk_widget_set_sensitive(widget, drive_check_parallel_cable(drive_type));

    /* determine if the parallel cable is supported by the machine */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:
            /* all four types supported */
            enabled = TRUE;
            count = 4;
            break;
        case VICE_MACHINE_PLUS4:
            /* only the first two supported */
            enabled = TRUE;
            count = 2;
            break;
        default:
            /* none supported */
            enabled = FALSE;
            count = 4;
            cable_type = -1;
            break;
    }

    for (i = 0; i < 4; i++) {
        radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, i + 1);
        if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)) {
            if (i == count) {
                enabled = !enabled;
            }
            gtk_widget_set_sensitive(radio, enabled);
        }
    }

    if (cable_type >= 0) {
        snprintf(res_name, 256, "Drive%dParallelCable", unit);
        resources_get_int(res_name, &cable_type);
    } else {
        cable_type = 0; /* unsupported, set to 0 */
    }
    uihelpers_set_radio_button_grid_by_index(widget, cable_type);
}

