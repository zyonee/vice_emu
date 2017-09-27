/*
 * uimenu.h - Native GTK3 menu handling - header
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
 *
 *  TODO:   add structs to specify menu items and submenus, like the shared UI
 *          code of Gtk2/Xaw, but less heavy on the preprocessor use, please
 */

#ifndef VICE_UIMENU_H
#define VICE_UIMENU_H

#include "vice.h"

#include <gtk/gtk.h>


/** \brief  Menu item types
 *
 * The submenu types needs special handling, no more callbacks to create the
 * menu so we won't have to deal with the `ui_update_checmarks` crap of Gtk2.
 *
 * The 'layer' between VICE and Gtk3 should be as thin as possible, so no
 * UI_CREATE_TOGGLE_BUTTON() stuff.
 */
typedef enum ui_menu_item_type_e {
    UI_MENU_TYPE_GUARD = -1,    /**< list terminator */
    UI_MENU_TYPE_ITEM_ACTION,   /**< standard list item: activate dialog */
    UI_MENU_TYPE_ITEM_CHECK,    /**< menu item with checkmark */
    UI_MENU_TYPE_SUBMENU,       /**< submenu */
    UI_MENU_TYPE_SEPARATOR      /**< items separator */
} ui_menu_item_type_t;



/** \brief  Menu item object
 *
 * Contains information on a menu item
 */
typedef struct ui_menu_item_s {
    char *              label;  /**< menu item label */
    ui_menu_item_type_t type;   /**< menu item type, \see ui_menu_item_type_t */

    /* callbacks, accelerators and other things, again light on the CPP/layer
     * stuff to keep things clean and maintainable.
     */

    /** menu item callback function (NULL == no callback) */
    void (*callback)(GtkWidget *widget, gpointer user_data);

    /** callback data, or a pointer to an array of submenu items if the menu
     *  type is UI_MENU_TYPE_SUBMENU */
    void *data;

    /** accelerator key, without modifier
     * (see /usr/include/gtk-3.0/gdk/gdkkeysyms.h)
     */
    guint keysym;

    /** modifier (ie Alt) */
    GdkModifierType modifier;

} ui_menu_item_t;


/** \brief  Terminator of a menu items list
 */
#define UI_MENU_TERMINATOR { NULL, UI_MENU_TYPE_GUARD, NULL, NULL, 0, 0 }


/** \brief  Menu items separator
 */
#define UI_MENU_SEPARATOR { "---", UI_MENU_TYPE_SEPARATOR, NULL, NULL, 0, 0 }


/*
 * Public functions
 */

GtkWidget *ui_menu_bar_create(void);

GtkWidget *ui_menu_add(GtkWidget *menu, ui_menu_item_t *items);
GtkWidget *ui_menu_file_add(ui_menu_item_t *items);
GtkWidget *ui_menu_edit_add(ui_menu_item_t *items);
GtkWidget *ui_menu_snapshot_add(ui_menu_item_t *items);
GtkWidget *ui_menu_settings_add(ui_menu_item_t *items);
GtkWidget *ui_menu_help_add(ui_menu_item_t *items);
#ifdef DEBUG
GtkWidget *ui_menu_debug_add(ui_menu_item_t *items);
#endif


void ui_menu_init_accelerators(GtkWidget *window);


#endif
