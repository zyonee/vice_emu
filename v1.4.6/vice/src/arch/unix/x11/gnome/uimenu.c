/*
 * uimenu.c - Simple and ugly cascaded pop-up menu implementation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Oliver Schaertel GTK+ port
 *  Martin Pottendorfer Gnome port
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

/* Warning: this code sucks.  It does work, but it sucks.  */

#include "vice.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SmeBSB.h>

#include "machine.h"
#include "resources.h"
#include "utils.h"
#include "vsync.h"

#ifdef GNOME_MENUS
#include <gnome.h>
#endif
#include "uimenu.h"

/* Separator item.  */
ui_menu_entry_t ui_menu_separator[] = {
    { "--" },
    { NULL },
};

static int menu_popup = 0;

#define MAX_SUBMENUS 1024
static struct {
    GtkWidget* widget;
    GtkWidget* parent;
    int level;
} submenus[MAX_SUBMENUS];


/* This keeps a list of the menus with a checkmark on the left.  Each time
   some setting is changed, we have to update them. */
#define MAX_UPDATE_MENU_LIST_SIZE 1024
typedef struct {
    char *name;
#ifdef GNOME_MENUS
    GnomeUIInfo *uiinfo;
#else
    GtkWidget *w;
#endif
    ui_callback_t cb;
    ui_menu_cb_obj obj;
    gint handlerid;
} checkmark_t;

static GList *checkmark_list = NULL;

int num_checkmark_menu_items = 0; /* !static because vsidui needs it. ugly! */

static int num_submenus = 0;
/* ------------------------------------------------------------------------- */


static char *make_menu_label(ui_menu_entry_t *e)
{
    const char *key_string;
    char *tmp = alloca(1024);

    if (e->hotkey_keysym == (KeySym) 0)
        return stralloc(e->string);

    *tmp = '\0';
    if (e->hotkey_modifier & UI_HOTMOD_CONTROL)
        strcat(tmp, "C-");
    if (e->hotkey_modifier & UI_HOTMOD_META)
        strcat(tmp, "M-");
    if (e->hotkey_modifier & UI_HOTMOD_ALT)
        strcat(tmp, "A-");
    if (e->hotkey_modifier & UI_HOTMOD_SHIFT)
        strcat(tmp, "S-");

    key_string = strchr(XKeysymToString(e->hotkey_keysym), '_');
    if (key_string == NULL)
        key_string = XKeysymToString(e->hotkey_keysym);
    else
        key_string++;

    return concat(_(e->string), "    (", tmp, key_string, ")", NULL);
}

/* ------------------------------------------------------------------------- */

int ui_menu_init()
{
    ui_create_dynamic_menues();
    return(0);
}

#ifdef GNOME_MENUS
GnomeUIInfo* ui_menu_create(const char *menu_name, ...)
{
    static int level = 0;
    unsigned int i;
    ui_menu_entry_t *list;
    va_list ap;
    ui_menu_cb_obj *obj;
    GnomeUIInfo *uiinfo;
    int current = 0;
    int num_menu_items = 0;

    level++;
    va_start(ap, menu_name);

    /* Ugly, but I have to allocate the GnomeUIInfo array before, 
       otherwhise the pointer in checkmark_menu_items is invalid */
    while ((list = va_arg(ap, ui_menu_entry_t *)) != NULL) 
        for (i = 0; list[i].string; i++)
	    num_menu_items++;
    uiinfo = g_new(GnomeUIInfo, num_menu_items + 1);

    va_start(ap, menu_name);
    while ((list = va_arg(ap, ui_menu_entry_t *)) != NULL) 
    {
        for (i = 0; list[i].string; i++) 
	{
            switch (*list[i].string) 
	    {
	    case '-':		/* line */
		memset(&uiinfo[current], 0, sizeof(GnomeUIInfo));
		uiinfo[current].type = GNOME_APP_UI_SEPARATOR;
                break;
	    case '*':		/* toggle */
		if (list[i].callback) {
		    uiinfo[current].type = GNOME_APP_UI_TOGGLEITEM;
		    uiinfo[current].label = make_menu_label(&list[i]);
		    uiinfo[current].hint = NULL;
		    uiinfo[current].moreinfo = list[i].callback;
		    uiinfo[current].unused_data = NULL;
		    uiinfo[current].pixmap_type  = GNOME_APP_PIXMAP_NONE;
		    uiinfo[current].accelerator_key = 0;
		    uiinfo[current].widget = NULL;
		    if (num_checkmark_menu_items < MAX_UPDATE_MENU_LIST_SIZE) {
			obj = &checkmark_menu_items[num_checkmark_menu_items].obj;
			checkmark_menu_items[num_checkmark_menu_items].uiinfo =
			    &uiinfo[current];
			checkmark_menu_items[num_checkmark_menu_items].cb =
			    list[i].callback;
			checkmark_menu_items[num_checkmark_menu_items].obj.value =
			    (void*) list[i].callback_data;
			checkmark_menu_items[num_checkmark_menu_items].obj.status =
			    CB_NORMAL;
			uiinfo[current].user_data = (gpointer) obj;
			num_checkmark_menu_items++;
			
		    } else {
			fprintf(stderr,
				"Maximum number of menus reached!  "
				"Please fix the code.\n");
			exit(-1);
		    }
		} else {
			fprintf(stderr,
				"Checkbox Menu Item without callback: %s!  "
				"Please fix the code.\n", list[i].string);
			exit(-1);
		}
		break;
	    default:
	    {
		if (list[i].sub_menu) {
		    if (num_submenus > MAX_SUBMENUS) {
			fprintf(stderr,
				"Maximum number of sub menus reached! "
				"Please fix the code.\n");
			exit(-1);
			    
		    }
		    uiinfo[current].type = GNOME_APP_UI_SUBTREE;
		    uiinfo[current].moreinfo = ui_menu_create("SUB", list[i].sub_menu, NULL);
		    uiinfo[current].user_data = NULL;
		} 
		else 
		{
		    uiinfo[current].type = GNOME_APP_UI_ITEM;
		    uiinfo[current].moreinfo = list[i].callback;
			
		    if (list[i].callback) {
			obj = (ui_menu_cb_obj*) xmalloc(sizeof(ui_menu_cb_obj));
			obj->value = (void*) list[i].callback_data;
			uiinfo[current].user_data = obj;
		    }
		}
		uiinfo[current].label = make_menu_label(&list[i]);
		uiinfo[current].hint = NULL;
		uiinfo[current].unused_data = NULL;
		uiinfo[current].pixmap_type = GNOME_APP_PIXMAP_NONE;
		uiinfo[current].accelerator_key = 0;
		uiinfo[current].widget = NULL;
		
		break;
	    }
	    }
	    
	    if (list[i].hotkey_keysym != (KeySym) 0
		&& list[i].callback != NULL)
		ui_hotkey_register(list[i].hotkey_modifier,
				   list[i].hotkey_keysym,
				   list[i].callback,
				   obj);
	    
	    current++;
/* 	    uiinfo = g_renew(GnomeUIInfo, uiinfo, current + 1); */
        }
    }
    
    memset(&uiinfo[current], 0, sizeof(GnomeUIInfo));
    uiinfo[current].type = GNOME_APP_UI_ENDOFINFO;
    
    level--;
    
    va_end(ap);
    return uiinfo;
}

#else  /* !GNOME_MENUS */

static void delete_checkmark_cb(GtkWidget *w, gpointer data)
{
    checkmark_t *cm;
    
    cm = (checkmark_t *) data;
    checkmark_list = g_list_remove(checkmark_list, data);
    free(cm->name);
    free(cm);
}

GtkWidget* ui_menu_create(const char *menu_name, ...)
{
    static int level = 0;
    GtkWidget *w;
    unsigned int i, j;
    ui_menu_entry_t *list;
    va_list ap;
    ui_menu_cb_obj *obj = NULL;

    level++;
    va_start(ap, menu_name);

    w = gtk_menu_new();

    while ((list = va_arg(ap, ui_menu_entry_t *)) != NULL) {
        for (i = j = 0; list[i].string; i++) {
            GtkWidget *new_item;
            char name[256];

            sprintf(name, "MenuItem%d", j);	/* ugly... */
            switch (*list[i].string) 
	    {
	    case '-':		/* line */
		new_item  = gtk_menu_item_new();
                break;
	    case '*':		/* toggle */
	    {
		char *label = make_menu_label(&list[i]);
		/* Add this item to the list of calls to perform to update the
		   menu status. */
		if (list[i].callback) 
		{
		    checkmark_t *cmt;
		    new_item = gtk_check_menu_item_new_with_label(label+1);
		    
		    cmt = (checkmark_t *) xmalloc(sizeof(checkmark_t));
		    cmt->name = stralloc(label+1);
		    cmt->w = new_item;
		    cmt->cb = list[i].callback;
		    cmt->obj.value = (void*) list[i].callback_data;
		    cmt->obj.status = CB_NORMAL;
		    cmt->handlerid = 
			gtk_signal_connect(GTK_OBJECT(new_item),"activate",
					   GTK_SIGNAL_FUNC(list[i].callback),
					   (gpointer) &(cmt->obj)); 
		    gtk_signal_connect(GTK_OBJECT(new_item), "destroy",
				       GTK_SIGNAL_FUNC(delete_checkmark_cb),
				       (gpointer) cmt);
		    checkmark_list = g_list_prepend(checkmark_list, cmt);
		    obj = &cmt->obj;
		} 
		else 
		    new_item = gtk_menu_item_new_with_label(label+1);

		j++;
		free(label);
		break;
	    }
	    default:
	    {
		char *label = make_menu_label(&list[i]);
		new_item = gtk_menu_item_new_with_label(label);
		if (list[i].callback) {
		    obj = (ui_menu_cb_obj*) xmalloc(sizeof(ui_menu_cb_obj));
		    obj->value = (void*) list[i].callback_data;
		    
		    gtk_signal_connect(GTK_OBJECT(new_item),"activate",
				       GTK_SIGNAL_FUNC(list[i].callback),
				       (gpointer) obj); 
		}
		free(label);
		j++;
	    }
            }

	    gtk_menu_append(GTK_MENU(w),new_item);
	    gtk_widget_show(new_item);

            if (list[i].sub_menu) 
	    {
                GtkWidget *sub;
		char subname[10];

                if (num_submenus > MAX_SUBMENUS) {
                    fprintf(stderr,
                            "Maximum number of sub menus reached! "
                            "Please fix the code.\n");
                    exit(-1);
                }
		sprintf(subname, "SUB%d", num_submenus);
		sub = ui_menu_create(list[i].string, 
				     list[i].sub_menu, NULL);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_item),sub);
                submenus[num_submenus].widget = sub;
                submenus[num_submenus].parent = new_item;
                submenus[num_submenus].level = level;
                num_submenus++;
            } 
	    else 
	    {            /* no submenu */
	        if (list[i].hotkey_keysym != (KeySym) 0
		    && list[i].callback != NULL)
		    ui_hotkey_register(list[i].hotkey_modifier,
                                       list[i].hotkey_keysym,
                                       list[i].callback,
                                       obj);
            }
        }
    }
    
    level--;

    va_end(ap);
    return w;
}

#endif /* !GNOME_MENUS */

int ui_menu_any_open(void)
{
    return menu_popup;
}

static void menu_handle_block(gpointer data, gpointer user_data)
{
    checkmark_t *cm = (checkmark_t *)data;

    if (user_data)
	gtk_signal_handler_block(GTK_OBJECT(cm->w), cm->handlerid);
    else
	gtk_signal_handler_unblock(GTK_OBJECT(cm->w), cm->handlerid);
}

static void menu_update_checkmarks(gpointer data, gpointer user_data)
{
    checkmark_t *cm = (checkmark_t *) data;
    
    cm->obj.status = CB_REFRESH;
    ((void*(*)(GtkWidget*, ui_callback_data_t))
     cm->cb)(cm->w, (ui_callback_data_t) &cm->obj);
    cm->obj.status = CB_NORMAL;
}

void ui_menu_update_all_GTK(void)
{
#ifndef GNOME_MENUS		/* Well, with GNOME_MENUS this won't
				   won't work */
    g_list_foreach(checkmark_list, menu_handle_block, (gpointer) 1);
    g_list_foreach(checkmark_list, menu_update_checkmarks, NULL);
    ui_dispatch_events();
    g_list_foreach(checkmark_list, menu_handle_block, (gpointer) 0);
#else
    fprintf(stderr,
	    "Gnome menus not supported."
	    "Please fix the code.\n");
    exit(-1);
#endif
    
}

void ui_menu_update_all(void)
{
}

void ui_menu_set_tick(GtkWidget *w, int flag) {
    if(GTK_IS_CHECK_MENU_ITEM(w))
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), flag != 0);
}

void ui_menu_set_sensitive(GtkWidget *w, int flag)
{
    gtk_widget_set_sensitive(w,flag);
}

/* ------------------------------------------------------------------------- */

/* These functions are called by radio and toggle menu items if the callback
   functions are defined through `UI_MENU_DEFINE_TOGGLE()',
   `UI_MENU_DEFINE_RADIO()' or `UI_MENU_DEFINE_STRING_RADIO()'.  */

void _ui_menu_toggle_helper(GtkWidget *w,
                            ui_callback_data_t event_data,
                            const char *resource_name)
{
    int current_value;
    char buf[100];

    if (resources_get_value(resource_name,
                            (resource_value_t *) &current_value) < 0)
        return;

    if(!CHECK_MENUS) {
        resources_set_value(resource_name, (resource_value_t) !current_value);
	ui_update_menus();
	if (psid_mode) {
	    sprintf(buf, "resource %s %d\n", resource_name, !current_value);
	    ui_proc_write_msg(buf);
	}
    } else {
        ui_menu_set_tick(w, current_value);
    }
}

void _ui_menu_radio_helper(GtkWidget *w,
                           ui_callback_data_t event_data,
                           const char *resource_name)
{
    int current_value;
    char buf[100];

    resources_get_value(resource_name, (resource_value_t *) &current_value);

    if (!CHECK_MENUS) {
        if (current_value != (int) UI_MENU_CB_PARAM) {
            resources_set_value(resource_name,
				(resource_value_t) UI_MENU_CB_PARAM);
            ui_update_menus();
	    if (psid_mode) {
	        sprintf(buf, "resource %s %d\n", resource_name,
                        (int)UI_MENU_CB_PARAM);
		ui_proc_write_msg(buf);
	    }
        }
    } else {
	ui_menu_set_tick(w, current_value == (int) UI_MENU_CB_PARAM);
    }  
}

void _ui_menu_string_radio_helper(GtkWidget *w, 
                                  ui_callback_data_t event_data,
                                  const char *resource_name)
{
    resource_value_t current_value;

    resources_get_value(resource_name, &current_value);

    if( current_value == 0) return;

    if (!CHECK_MENUS) {
        if (strcmp((const char *) current_value, (const char *) UI_MENU_CB_PARAM) != 0) {
	   resources_set_value(resource_name,(resource_value_t*) UI_MENU_CB_PARAM);
	   ui_update_menus();
	}
    } else {
        ui_menu_set_tick(w, strcmp((const char *) current_value,
				   (const char *) UI_MENU_CB_PARAM) == 0);
    }
}
