/*
 * ui.c - Native GTK3 UI stuff.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
/* required for g_chdir() */
#include <glib/gstdio.h>

#include "debug_gtk3.h"
#include "not_implemented.h"

#include "cmdline.h"
#include "interrupt.h"
#include "kbd.h"
#include "lib.h"
#include "machine.h"
#include "drive.h"
#include "resources.h"
#include "translate.h"
#include "util.h"
#include "videoarch.h"
#include "vsync.h"

#include "basedialogs.h"
#include "uiapi.h"
#include "uiedit.h"
#include "uimenu.h"
#include "uimonarch.h"
#include "uisettings.h"
#include "uistatusbar.h"
#include "uismartattach.h"
#include "uidiskattach.h"
#include "uiabout.h"
#include "selectdirectorydialog.h"
#include "jamdialog.h"

#include "ui.h"



/*****************************************************************************
 *                  Defines, enums, type declarations                        *
 ****************************************************************************/

/** \brief  Default HTML browser
 *
 * \todo    Needs ifdef's for different archs
 */
#define HTML_BROWSER_COMMAND_DEFAULT    "firefox %s"


/** \brief  Number of GtkWindow's in the ui_resources
 */
#define NUM_WINDOWS 3


/** \brief  Windows indici
 */
enum {
    PRIMARY_WINDOW,     /**< primary window, all emulators */
    SECONDARY_WINDOW,   /**< secondary window, C128's VDC */
    MONITOR_WINDOW,     /**< optional monitor window/terminal */
};


/** \brief  Struct holding basic UI rescources
 */
typedef struct ui_resources_s {

    char *html_browser_command; /**< HTMLBrowserCommand (str) */
    int save_resources_on_exit; /**< SaveResourcesOnExit (bool) */
    int confirm_on_exit;        /**< ConfirmOnExit (bool) */

    int depth;

    video_canvas_t *canvas[NUM_WINDOWS];
    GtkWidget *window_widget[NUM_WINDOWS]; /**< the toplevel GtkWidget (Window) */
    int window_width[NUM_WINDOWS];
    int window_height[NUM_WINDOWS];
    int window_xpos[NUM_WINDOWS];
    int window_ypos[NUM_WINDOWS];

} ui_resource_t;


/** \brief  Collection of UI resources
 *
 * This needs to stay here, to allow the command line and resources initializers
 * to references the ui resources
 */
static ui_resource_t ui_resources;



/* Forward declarations of static functions */
static void machine_reset_callback(GtkWidget *widget, gpointer user_data);
static void drive_reset_callback(GtkWidget *widget, gpointer user_data);
static void ui_close_callback(GtkWidget *widget, gpointer user_data);
static void ui_window_destroy_callback(GtkWidget *widget, gpointer user_data);
static int set_html_browser_command(const char *val, void *param);
static int set_save_resources_on_exit(int val, void *param);
static int set_confirm_on_exit(int val, void *param);
static int set_window_height(int val, void *param);
static int set_window_width(int val, void *param);
static int set_window_xpos(int val, void *param);
static int set_window_ypos(int val, void *param);



/*****************************************************************************
 *                              Static data                                  *
 ****************************************************************************/


/** \brief  String type resources list
 */
static const resource_string_t resources_string[] = {
    { "HTMLBrowserCommand", HTML_BROWSER_COMMAND_DEFAULT, RES_EVENT_NO, NULL,
        &ui_resources.html_browser_command, set_html_browser_command, NULL },
    RESOURCE_STRING_LIST_END
};


/** \brief  Integer/Boolean type resources list
 */
static const resource_int_t resources_int_primary_window[] = {
    { "SaveResourcesOnExit", 0, RES_EVENT_NO, NULL,
        &ui_resources.save_resources_on_exit, set_save_resources_on_exit, NULL },
    { "ConfirmOnExit", 1, RES_EVENT_NO, NULL,
        &ui_resources.confirm_on_exit, set_confirm_on_exit, NULL },

    /* Window size and position setters */

    /* primary window (all emulators) */
    { "Window0Height", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_height[PRIMARY_WINDOW]), set_window_height,
        (void*)PRIMARY_WINDOW },
    { "Window0Width", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_width[PRIMARY_WINDOW]), set_window_width,
        (void*)PRIMARY_WINDOW },
    { "Window0Xpos", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_xpos[PRIMARY_WINDOW]), set_window_xpos,
        (void*)PRIMARY_WINDOW },
    { "Window0Ypos", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_ypos[PRIMARY_WINDOW]), set_window_ypos,
        (void*)PRIMARY_WINDOW },

    RESOURCE_INT_LIST_END
};


static const resource_int_t resources_int_secondary_window[] = {
    /* secondary window (C128's VDC display) */
    { "Window1Height", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_height[SECONDARY_WINDOW]), set_window_height,
        (void*)SECONDARY_WINDOW },
    { "Window1Width", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_width[SECONDARY_WINDOW]), set_window_width,
        (void*)SECONDARY_WINDOW },
    { "Window1Xpos", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_xpos[SECONDARY_WINDOW]), set_window_xpos,
        (void*)SECONDARY_WINDOW },
    { "Window1Ypos", 0, RES_EVENT_NO, NULL,
        &(ui_resources.window_ypos[SECONDARY_WINDOW]), set_window_ypos,
        (void*)SECONDARY_WINDOW },

    RESOURCE_INT_LIST_END
};


/** \brief  Command line options shared between emu's, include VSID
 */
static const cmdline_option_t cmdline_options_common[] = {
    { "-htmlbrowser", SET_RESOURCE, 1,
        NULL, NULL, "HTMLBrowserCommand", NULL,
        USE_PARAM_STRING, USE_DESCRIPTION_STRING,
        IDCLS_UNUSED, IDCLS_UNUSED,
        N_("<Command>"), N_("Specify and HTML browser for the on-line help") },

    { "-confirmexit", SET_RESOURCE, 0,
        NULL, NULL, "SaveResourcesOnExit", (void*)1,
        USE_PARAM_STRING, USE_DESCRIPTION_STRING,
        IDCLS_UNUSED, IDCLS_UNUSED,
        NULL, N_("Never confirm quitting VICE") },
    { "+confirmexit", SET_RESOURCE, 0,
        NULL, NULL, "SaveResourcesOnExit", (void*)0,
        USE_PARAM_STRING, USE_DESCRIPTION_STRING,
        IDCLS_UNUSED, IDCLS_UNUSED,
        NULL, N_("Don't confirm quitting VICE") },

    { "-saveres", SET_RESOURCE, 0,
        NULL, NULL, "SaveResourcesOnExit", (void *)1,
        USE_PARAM_STRING, USE_DESCRIPTION_STRING,
        IDCLS_UNUSED, IDCLS_UNUSED,
        NULL, N_("Save settings on exit") },
    { "+saveres", SET_RESOURCE, 0,
        NULL, NULL, "SaveResourcesOnExit", (void *)0,
        USE_PARAM_STRING, USE_DESCRIPTION_STRING,
        IDCLS_UNUSED, IDCLS_UNUSED,
        NULL, N_("Never save settings on exit") },

    CMDLINE_LIST_END
};


/** \brief  File->Reset submenu
 */
static ui_menu_item_t reset_submenu[] = {
    { "Soft reset", UI_MENU_TYPE_ITEM_ACTION,
        machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_SOFT),
        GDK_KEY_F9, GDK_MOD1_MASK },
    { "Hard reset", UI_MENU_TYPE_ITEM_ACTION,
        machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_HARD),
        0,0 },

    UI_MENU_SEPARATOR,

    { "Reset drive #8", UI_MENU_TYPE_ITEM_ACTION,
        drive_reset_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Reset drive #9", UI_MENU_TYPE_ITEM_ACTION,
        drive_reset_callback, GINT_TO_POINTER(9),
        0, 0 },
    { "Reset drive #10", UI_MENU_TYPE_ITEM_ACTION,
        drive_reset_callback, GINT_TO_POINTER(10),
        0, 0 },
    { "Reset drive #11", UI_MENU_TYPE_ITEM_ACTION,
        drive_reset_callback, GINT_TO_POINTER(11),
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu
 */
static ui_menu_item_t file_menu[] = {
    { "Smart attach disk/tape ...", UI_MENU_TYPE_ITEM_ACTION,
        ui_smart_attach_callback, NULL,
        0, 0 },
    { "Autostart settings [MOVED TO SETTINGS]", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    /* disk */
    { "Attach disk image ...", UI_MENU_TYPE_ITEM_ACTION,
        ui_disk_attach_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Create and attach an empty disk ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Detach disk image", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Flip list ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    /* tape (funny how create & attach are flipped here) */
    { "Create a new tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Attach tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Detach tape image", UI_MENU_TYPE_ITEM_ACTION, NULL, NULL,
        0, 0 },
    { "Datasette controls [USE TAPEWIDGET IN STATUSBAR]",
        UI_MENU_TYPE_ITEM_ACTION, NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    /* cart */
    { "Attach cartridge image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Detach cartridge image(s)", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Cartridge freeze", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    /* monitor */
    { "Activate monitor", UI_MENU_TYPE_ITEM_ACTION,
        ui_monitor_activate_callback, NULL,
        0, 0 },
    { "Monitor settings ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Netplay ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Reset ...", UI_MENU_TYPE_SUBMENU,
        NULL, reset_submenu,
        0, 0 },
    { "Action on CPU JAM ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Exit emulator", UI_MENU_TYPE_ITEM_ACTION,
        ui_close_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Edit' menu
 */
static ui_menu_item_t edit_menu[] = {
    { "Copy", UI_MENU_TYPE_ITEM_ACTION,
        ui_copy_callback, NULL,
        0, 0 },
    { "Paste", UI_MENU_TYPE_ITEM_ACTION,
        ui_paste_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Snapshot' menu
 */
static ui_menu_item_t snapshot_menu[] = {
    { "Load snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Save snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Quickload snapshot", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Quicksave snapshot", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Select history directory ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Start recording events", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Stop recording events", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Start playing back events", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Stop playing back events", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Set recording milestone", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Return to milestone", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Recording start mode ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Save media file ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    /* XXX: this item should be removed and its functionality added to the
     *      'Save media file' item like I did in the SDL UI: Saving a media
     *      file should handle image, sound and/or video
     *      -- Compyx
     */
    { "Sound recording ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Help' menu items
 */
static ui_menu_item_t help_menu[] = {
    { "Browse manual", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Commandline options ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Compiletime features ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "_About VICE", UI_MENU_TYPE_ITEM_ACTION,
        ui_about_dialog_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Settings' menu items
 */
static ui_menu_item_t settings_menu[] = {
    { "Settings", UI_MENU_TYPE_ITEM_ACTION,
        ui_settings_dialog_create, NULL,
        0, 0 },
    UI_MENU_TERMINATOR
};


/** \brief  'Debug' menu items
 */
#ifdef DEBUG
static ui_menu_item_t debug_menu[] = {
    { "Trace mode ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Main CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Drive #8 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        NULL, NULL,
        0, 0 },
    { "Drive #9 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        NULL, NULL,
        0, 0 },
    { "Drive #10 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        NULL, NULL,
        0, 0 },
    { "Drive #11 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Autoplay playback frames ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },
    { "Save core dump", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};
#endif



/** \brief  Flag indicating pause mode
 */
static int is_paused = 0;


/** \brief  Signals the html_browser_command field of the resource got allocated
 */
static int html_browser_command_set = 0;



/******************************************************************************
 *                              Event handlers                                *
 *****************************************************************************/


/** \brief  Callback for the soft/hard reset items
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   MACHINE_RESET_MODE_SOFT/MACHINE_RESET_MODE_HARD
 */
static void machine_reset_callback(GtkWidget *widget, gpointer user_data)
{
    vsync_suspend_speed_eval();
    machine_trigger_reset(GPOINTER_TO_INT(user_data));
}


/** \brief  Callback for the drive reset items
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   drive unit number (8-11) (int)
 */
static void drive_reset_callback(GtkWidget *widget, gpointer user_data)
{
    vsync_suspend_speed_eval();
    drive_cpu_trigger_reset(GPOINTER_TO_INT(user_data) - 8);
}


/** \brief  Callback for the File->Exit menu item
 *
 * This asks the user to confirm to exit the emulator if ConfirmOnExit is set.
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   window index, optional, defaults to primary
 */
static void ui_close_callback(GtkWidget *widget, gpointer user_data)
{
    int index;
    int confirm;

    if (user_data == NULL) {
        index = PRIMARY_WINDOW;
    } else {
        index = GPOINTER_TO_INT(user_data);
    }

    resources_get_int("ConfirmOnExit", &confirm);
    if (!confirm) {
        gtk_widget_destroy(ui_resources.window_widget[index]);
        return;
    }

    if (ui_message_confirm(ui_resources.window_widget[index], "Exit VICE",
                "Do you really wish to exit VICE?")) {
        debug_gtk3("Exit confirmed\n");
        gtk_widget_destroy(ui_resources.window_widget[index]);
    }
}


/** \brief  Handler for the "delete-event" of a GtkWindow
 *
 * \param[in]   widget      window triggering the event
 * \param[in]   event       event details (unused)
 * \param[in]   user_data   extra data for the event (unused)
 *
 * \return  `FALSE` to exit the emulator, `TRUE` to continue
 */
static gboolean on_delete_event(GtkWidget *widget, GdkEvent *event,
                                gpointer user_data)
{
    int confirm;

    debug_gtk3("got 'delete-event'\n'");

    resources_get_int("ConfirmOnExit", &confirm);
    if (!confirm) {
        return FALSE;
    }

    if (ui_message_confirm(widget, "Exit VICE",
                "Do you really wish to exit VICE?")) {
        debug_gtk3("Exit confirmed\n");
        return FALSE;
    }
    return TRUE;
}


/** \brief  Callback for a windows' "destroy" event
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   extra data for the callback (unused)
 */
static void ui_window_destroy_callback(GtkWidget *widget, gpointer user_data)
{
    debug_gtk3("called\n");
    vsync_suspend_speed_eval();
    ui_exit();
}



/*****************************************************************************
 *                  Temporary windows atexit() crash workaround              *
 ****************************************************************************/
#ifdef WIN32_COMPILE
#define ATEXIT_MAX_FUNCS 64

static void *atexit_functions[ATEXIT_MAX_FUNCS + 1];

static int atexit_counter = 0;

int vice_atexit(void (*function)(void))
{
    int i;

    if (!atexit_counter) {
        for (i = 0; i <= ATEXIT_MAX_FUNCS; ++i) {
            atexit_functions[i] = NULL;
        }
    }

    if (atexit_counter == ATEXIT_MAX_FUNCS) {
        return 1;
    }

    atexit_functions[atexit_counter++] = function;

    return 0;
}

static void atexit_functions_execute(void)
{
    int i = atexit_counter -1;
    void (*f)(void) = NULL;

    while (i >= 0 && atexit_functions[i]) {
        f = atexit_functions[i--];
        f();
    }
}
#endif  /* ifdef WIN32_COMPILE */



/** \brief  Get a window-spec array index from \a param
 *
 * Also performs a bounds check and returns -1 on boundary violation.
 *
 * \param[in]   param   extra param passed to a setter
 *
 * \return  index in array or -1 on error
 */
static int window_index_from_param(void *param)
{
    int index = vice_ptr_to_int(param);
    return (index >= 0 || index < NUM_WINDOWS) ? index : -1;
}


/*
 * Resource getters/setters
 */


/** \brief  Set new HTML browser command string
 *
 * \param[in]   val     browser command string
 * \param[in]   param   extra parameter (ignored)
 *
 * \return  0
 */
static int set_html_browser_command(const char *val, void *param)
{
    util_string_set(&ui_resources.html_browser_command, val);
    html_browser_command_set = 1;
    return 0;
}


/** \brief  Set SaveResourcesOnExit resource
 *
 * \param[in]   val     new value
 * \param[in]   param   extra param (ignored)
 *
 * \return 0
 */
static int set_save_resources_on_exit(int val, void *param)
{
    ui_resources.save_resources_on_exit = val ? 1 : 0;
    return 0;
}


/** \brief  Set ConfirmOnExit resource (bool)
 *
 * \param[in]   val     new value
 * \param[in]   param   extra param (ignored)
 *
 * \return 0
 */
static int set_confirm_on_exit(int val, void *param)
{
    ui_resources.confirm_on_exit = val ? 1 : 0;
    return 0;
}


/** \brief  Set Window[X]Width resource (int)
 *
 * \param[in]   val     width in pixels
 * \param[in]   param   window index
 *
 * \return 0
 */
static int set_window_width(int val, void *param)
{
    int index = window_index_from_param(param);
    if (index < 0 || val < 0) {
        return -1;
    }
    ui_resources.window_width[index] = val;
    return 0;
}


/** \brief  Set Window[X]Height resource (int)
 *
 * \param[in]   val     height in pixels
 * \param[in]   param   window index
 *
 * \return 0
 */
static int set_window_height(int val, void *param)
{
    int index = window_index_from_param(param);
    if (index < 0 || val < 0) {
        return -1;
    }
    ui_resources.window_height[index] = val;
    return 0;
}


/** \brief  Set Window[X]Xpos resource (int)
 *
 * \param[in]   val     x-pos in pixels
 * \param[in]   param   window index
 *
 * \return 0
 */
static int set_window_xpos(int val, void *param)
{
    int index = window_index_from_param(param);
    if (index < 0 || val < 0) {
        return -1;
    }
    ui_resources.window_xpos[index] = val;
    return 0;
}


/** \brief  Set Window[X]Ypos resource (int)
 *
 * \param[in]   val     y-pos in pixels
 * \param[in]   param   window index
 *
 * \return 0
 */
static int set_window_ypos(int val, void *param)
{
    int index = window_index_from_param(param);
    if (index < 0 || val < 0) {
        return -1;
    }
    ui_resources.window_ypos[index] = val;
    return 0;
}



/* FIXME: the code that calls this apparently creates the VDC window for x128
          before the VIC window (primary) - this is probably done so the VIC
          window ends up being on top of the VDC window. however, we better call
          some "move window to front" function instead, and create the windows
          starting with the primary one. */
/* FIXME: the code below deals with the above mentioned fact and sets up the
          window_widget pointers correctly. this hackish magic can be eliminated
          when the code that creates the windows was moved over here AND the
          calling code is fixed to create the windows in different order. */
/** \brief Create a toplevel window to represent a video canvas.
 *
 * This function takes a video canvas structure and builds the widgets
 * that will represent that canvas in the UI as a whole. The
 * GtkDrawingArea that represents the actual screen backed by the
 * canvas will be entered into canvas->drawing_area.
 *
 * While it creates the widgets, it does not make them visible. The
 * video canvas routines are expected to do any last-minute processing
 * or preparation, and then call ui_display_toplevel_window() when
 * ready.
 *
 * \warning The "meaning" of the window depends on how many times the
 *          function has been called. On a C128, the first call
 *          produces the VDC window and the second produces the
 *          primary window. On all other machines, the first call
 *          produces the primary window. All subsequent calls will
 *          replace or leak the "monitor" window, but the nature of
 *          monitor windows is such that this should never happen.
 */
void ui_create_toplevel_window(struct video_canvas_s *canvas) {
    GtkWidget *new_window, *grid, *new_drawing_area, *status_bar;
    GtkWidget *menu_bar;
    int target_window;

    new_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    /* this needs to be here to make the menus with accelerators work */
    ui_menu_init_accelerators(new_window);

    grid = gtk_grid_new();
    new_drawing_area = gtk_drawing_area_new();
    status_bar = ui_statusbar_create();

    /* I'm pretty sure when running x128 we get two menu instances, so this
     * should go somewhere else: call ui_menu_bar_create() once and attach the
     * result menu to each GtkWindow instance
     */
    menu_bar = ui_menu_bar_create();

    /* generate File menu */
    ui_menu_file_add(file_menu);
    /* generate Edit menu */
    ui_menu_edit_add(edit_menu);
    /* generate Snapshot menu */
    ui_menu_snapshot_add(snapshot_menu);
    /* settings menu */
    ui_menu_settings_add(settings_menu);
    /* generate Help menu */
    ui_menu_help_add(help_menu);
#ifdef DEBUG
    ui_menu_debug_add(debug_menu);
#endif

    canvas->drawing_area = new_drawing_area;

    gtk_container_add(GTK_CONTAINER(new_window), grid);
    /* When we have a menu bar, we'll add it at the top here */
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_VERTICAL);

    gtk_container_add(GTK_CONTAINER(grid), menu_bar);
    gtk_container_add(GTK_CONTAINER(grid), new_drawing_area);
    gtk_container_add(GTK_CONTAINER(grid), status_bar);

    gtk_widget_set_hexpand(new_drawing_area, TRUE);
    gtk_widget_set_vexpand(new_drawing_area, TRUE);

    g_signal_connect(new_window, "delete-event",
            G_CALLBACK(on_delete_event), NULL);
    g_signal_connect(new_window, "destroy",
            G_CALLBACK(ui_window_destroy_callback), NULL);

    /* We've defaulted to PRIMARY_WINDOW. C128, however, gets its VDC
     * window created first, so shunt this window to secondary status
     * if that is what it is. */
    target_window = PRIMARY_WINDOW;
    if (machine_class == VICE_MACHINE_C128 && ui_resources.window_widget[SECONDARY_WINDOW] == NULL) {
        target_window = SECONDARY_WINDOW;
    }
    /* Recreated canvases go to MONITOR_WINDOW. */
    if (ui_resources.window_widget[target_window] != NULL) {
        /* TODO: This doesn't make even a little bit of sense. The monitor
         * window doesn't have a Commodore-screen canvas associated with
         * it! Monitors should be tracked completely seperately. */
        /* TODO: Ending up here should be a fatal error */
        target_window = MONITOR_WINDOW;
    }

    ui_resources.canvas[target_window] = canvas;
    ui_resources.window_widget[target_window] = new_window;

    /* gtk_window_set_title(GTK_WINDOW(new_window), canvas->viewport->title); */
    ui_display_speed(100.0f, 0.0f, 0); /* initial update of the window status bar */

    /* connect keyboard handlers */
    kbd_connect_handlers(new_window, NULL);
}

/** \brief  Finds the window associated with this canvas and makes it visible. */

void ui_display_toplevel_window(struct video_canvas_s *canvas)
{
    int i;
    for (i = 0; i < NUM_WINDOWS; ++i) {
        if (ui_resources.canvas[i] == canvas) {
            /* Normally this would show everything in the window,
             * including hidden status bar displays, but we've
             * disabled secondary displays in the status bar code with
             * gtk_widget_set_no_show_all(). */
            gtk_widget_show_all(ui_resources.window_widget[i]);
            break;
        }
    }
}



/** \brief  Initialize command line options (generic)
 *
 * \return  0 on success, -1 on failure
 */
int ui_cmdline_options_init(void)
{
    INCOMPLETE_IMPLEMENTATION();
    return cmdline_register_options(cmdline_options_common);
}


/* FIXME: what is this supposed to do? */
char *ui_get_file(const char *format, ...)
{
    NOT_IMPLEMENTED();
    return NULL;
}


/** \brief  Initialize Gtk3/GLib
 *
 * \param[in]   argc    pointer to main()'s argc
 * \param[in]   argv    main()'s argv
 *
 * \return 0;
 */
int ui_init(int *argc, char **argv)
{
    INCOMPLETE_IMPLEMENTATION();
    gtk_init(argc, &argv);
    ui_statusbar_init();
    return 0;
}


/* FIXME: do we need this? Doesn't seem like it -- compyx */
int ui_init_finalize(void)
{
    return 0;
}


/* FIXME: do we need this? Doesn't seem like it -- compyx */
int ui_init_finish(void)
{
    return 0;
}



ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    va_list args;
    char *buffer;
    int result;

    va_start(args, format);
    buffer = lib_mvsprintf(format, args);
    va_end(args);

    /* XXX: this sucks */
    result = jam_dialog(ui_resources.window_widget[PRIMARY_WINDOW], buffer);
    lib_free(buffer);

    return result;
}


/** \brief  Initialize resources related to the UI in general
 *
 * \return  0 on success, < 0 on failure
 */
int ui_resources_init(void)
{
    int i;
    /* initialize string resources */
    if (resources_register_string(resources_string) < 0) {
        return -1;
    }
    /* initialize int/bool resources */
    if (resources_register_int(resources_int_primary_window) < 0) {
        return -1;
    }

    if (machine_class == VICE_MACHINE_C128) {
        if (resources_register_int(resources_int_secondary_window) < 0) {
            return -1;
        }
    }

    for (i = 0; i < NUM_WINDOWS; ++i) {
        ui_resources.canvas[i] = NULL;
        ui_resources.window_widget[i] = NULL;
    }

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}


/** \brief  Clean up memory used by VICE resources
 */
void ui_resources_shutdown(void)
{
    if (html_browser_command_set) {
        lib_free(ui_resources.html_browser_command);
    }
    INCOMPLETE_IMPLEMENTATION();
}

void ui_shutdown(void)
{
    ui_settings_dialog_shutdown();
    ui_statusbar_shutdown();
}

void ui_update_menus(void)
{
    /* allows autostart to work */
    NOT_IMPLEMENTED_WARN_ONLY();
}


/** \brief  Dispatch next GLib main context event
 *
 * FIXME:   According to the Gtk3/GLib devs, this will at some point bite us
 *          in the arse.
 */
void ui_dispatch_next_event(void) {
    g_main_context_iteration(g_main_context_default(), FALSE);
}


/** \brief  Dispatch events pending in the GLib main context loop
 *
 * FIXME:   According to the Gtk3/GLib devs, this will at some point bite us
 *          in the arse.
 */
void ui_dispatch_events(void)
{
    while (g_main_context_pending(g_main_context_default())) {
        ui_dispatch_next_event();
    }
}

int ui_extend_image_dialog(void)
{
    NOT_IMPLEMENTED();
    return 0;
}


/** XXX: Look at src/arch/gtk3/widgets/basedialogs.c for proper implemention
 */
void ui_error(const char *format, ...)
{
    va_list ap;
    char str[1024];

    va_start(ap, format);
    vsprintf(str, format, ap);
    va_end(ap);
    fprintf(stderr, "VICE Error!:%s\n", str);
    TEMPORARY_IMPLEMENTATION();
}


/** XXX: Look at src/arch/gtk3/widgets/basedialogs.c for proper implemention
 */
void ui_message(const char *format, ...)
{
    GtkWidget *dialog;
    char buffer[1024];
    va_list ap;

    va_start(ap, format);
    g_vsnprintf(buffer, 1024, format, ap);
    va_end(ap);

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK, "%s", buffer);
    gtk_dialog_run(GTK_DIALOG(dialog));

}

/* display FPS (and some other stuff) in the title bar of the window(s) */
void ui_display_speed(float percent, float framerate, int warp_flag)
{
    int i;
    char str[128];
    int percent_int = (int)(percent + 0.5);
    int framerate_int = (int)(framerate + 0.5);
    char *warp, *mode[3] = {"", _(" (VDC)"), _(" (Monitor)")};

    for (i = 0; i < NUM_WINDOWS; i++) {
        if (ui_resources.canvas[i] && GTK_WINDOW(ui_resources.window_widget[i])) {
            warp = (warp_flag ? _("(warp)") : "");
            str[0] = 0;
            snprintf(str, 128, "%s%s - %3d%%, %2d fps %s%s",
                     ui_resources.canvas[i]->viewport->title, mode[i],
                     percent_int, framerate_int, warp,
                     is_paused ? " (Paused)" : "");
            str[127] = 0;
            gtk_window_set_title(GTK_WINDOW(ui_resources.window_widget[i]), str);
        }
    }
}


/** \brief  Keeps the ui events going while the emulation is paused
 *
 * \param[in]   addr    unused
 * \param[in]   data    unused
 */
static void pause_trap(uint16_t addr, void *data)
{
    ui_display_paused(1);
    vsync_suspend_speed_eval();
    while (is_paused) {
        ui_dispatch_next_event();
    }
}


/** \brief  This should display some 'pause' status indicator on the statusbar
 *
 * \param[in]   flag    pause state
 */
void ui_display_paused(int flag)
{
    ui_display_speed(0.0, 0.0, 0);
}


/** \brief  Pause emulation
 *
 * \param[in]   flag    pause state
 */
void ui_pause_emulation(int flag)
{
    if (flag && !is_paused) {
        is_paused = 1;
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        ui_display_paused(0);
        is_paused = 0;
    }
}


/** \brief  Check if emulation is paused
 *
 * \return  bool
 */
int ui_emulation_is_paused(void)
{
    return is_paused;
}


/** \brief  Shutdown the UI, clean up resources
 */
void ui_exit(void)
{
    int soe;    /* save on exit */

    resources_get_int("SaveResourcesOnExit", &soe);
    if (soe) {
        resources_save(NULL);
    }

    INCOMPLETE_IMPLEMENTATION();
#ifdef WIN32_COMPILE
    atexit_functions_execute();
#endif
    exit(0);
}
