/*
 * uihelpers.h - GTK3 helper function for create widgets - header
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

#ifndef HAVE_UIHELPERS_H
#define HAVE_UIHELPERS_H

#include "vice.h"

#include <gtk/gtk.h>


typedef struct ui_text_int_pair_s {
    char *text;
    int value;
} ui_text_int_pair_t;




GtkWidget *uihelpers_create_grid_with_label(const gchar *text, gint columns);

GtkWidget *uihelpers_create_int_radiogroup_with_label(
        const gchar *label,
        ui_text_int_pair_t *data,
        void (*callback)(GtkWidget *, gpointer));


#endif


