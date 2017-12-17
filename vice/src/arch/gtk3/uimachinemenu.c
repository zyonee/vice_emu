/*
 * uimachinemenu.c - Native GTK3 menus for machine emulators, (not vsid.)
 *
 * Written by
 *  Marcus Sutton <loggedoubt@gmail.com>
 *
 * based on code by
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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "datasette.h"
#include "debug.h"
#include "machine.h"
#include "ui.h"
#include "uiabout.h"
#include "uicart.h"
#include "uicmdline.h"
#include "uicommands.h"
#include "uicompiletimefeatures.h"
#include "uidatasette.h"
#include "uidebug.h"
#include "uidiskattach.h"
#include "uiedit.h"
#include "uifliplist.h"
#include "uimachinemenu.h"
#include "uimedia.h"
#include "uimenu.h"
#include "uimonarch.h"
#include "uisettings.h"
#include "uismartattach.h"
#include "uisnapshot.h"
#include "uitapeattach.h"



/** \brief  File->Detach submenu
 */
static ui_menu_item_t detach_submenu[] = {
    { "Drive #8", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive8", ui_disk_detach_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Drive #9", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive9", ui_disk_detach_callback, GINT_TO_POINTER(9),
        0, 0 },
    { "Drive #10", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive10", ui_disk_detach_callback, GINT_TO_POINTER(10),
        0, 0 },
    { "Drive #11", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive11", ui_disk_detach_callback, GINT_TO_POINTER(11),
        0, 0 },

    UI_MENU_TERMINATOR
};

/** \brief  File->Flip list submenu
 */
static ui_menu_item_t fliplist_submenu[] = {
    { "Add current image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-add", ui_fliplist_add_current_cb, GINT_TO_POINTER(8),
        GDK_KEY_I, VICE_MOD_MASK },
    { "Remove current image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
       "fliplist-remove", ui_fliplist_remove_current_cb, GINT_TO_POINTER(8),
        GDK_KEY_K, VICE_MOD_MASK },
    { "Attach next image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-next", ui_fliplist_next_cb, GINT_TO_POINTER(8),
        GDK_KEY_N, VICE_MOD_MASK },
    { "Attach previous image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-prev", ui_fliplist_prev_cb, GINT_TO_POINTER(8),
        GDK_KEY_N, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Load flip list file...", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-load", ui_fliplist_load_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Save flip list file...", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-save", ui_fliplist_save_callback, GINT_TO_POINTER(8),
        0, 0 },

    UI_MENU_TERMINATOR
};

/** \brief  File->Datasette control submenu
 */

static ui_menu_item_t datasette_control_submenu[] = {
    { "Stop", UI_MENU_TYPE_ITEM_ACTION,
        "tape-stop", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_STOP),
        0, 0 },
    { "Start", UI_MENU_TYPE_ITEM_ACTION,
        "tape-start", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_START),
        0, 0 },
    { "Forward", UI_MENU_TYPE_ITEM_ACTION,
        "tape-ff", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_FORWARD),
        0, 0 },
    { "Rewind", UI_MENU_TYPE_ITEM_ACTION,
        "tape-rew", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_REWIND),
        0, 0 },
    { "Record", UI_MENU_TYPE_ITEM_ACTION,
        "tape-record", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RECORD),
        0, 0 },
    { "Reset", UI_MENU_TYPE_ITEM_ACTION,
        "tape-reset", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RESET),
        0, 0 },
    { "Reset Counter", UI_MENU_TYPE_ITEM_ACTION,
        "tape-counter", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RESET_COUNTER),
        0, 0 },
    UI_MENU_TERMINATOR
};

/** \brief  'File->Cartridge attach' submenu
 */
static ui_menu_item_t cart_attach_submenu[] = {
    { "Smart attach cart image ... ", UI_MENU_TYPE_ITEM_ACTION,
        "attach-cart", (void *)uicart_smart_attach_dialog, NULL,
        GDK_KEY_C, VICE_MOD_MASK },
    UI_MENU_TERMINATOR
};

/** \brief  File->Reset submenu
 */
static ui_menu_item_t reset_submenu[] = {
    { "Soft reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-soft", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_SOFT),
        GDK_KEY_F9, VICE_MOD_MASK },
    { "Hard reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-hard", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_HARD),
        GDK_KEY_F12, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    { "Reset drive #8", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive8", ui_drive_reset_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Reset drive #9", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive9", ui_drive_reset_callback, GINT_TO_POINTER(9),
        0, 0 },
    { "Reset drive #10", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive10", ui_drive_reset_callback, GINT_TO_POINTER(10),
        0, 0 },
    { "Reset drive #11", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive11", ui_drive_reset_callback, GINT_TO_POINTER(11),
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - head section
 */
static ui_menu_item_t file_menu_head[] = {
    { "Smart attach disk/tape ...", UI_MENU_TYPE_ITEM_ACTION,
        "smart-attach", ui_smart_attach_callback, NULL,
        GDK_KEY_A, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    /* disk */
    { "Attach disk image ...", UI_MENU_TYPE_ITEM_ACTION,
        "attach-disk", ui_disk_attach_callback, GINT_TO_POINTER(8),
        GDK_KEY_8, VICE_MOD_MASK },
    { "Create and attach an empty disk ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL, NULL,
        0, 0 },
    { "Detach disk image", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, detach_submenu,
        0, 0 },
    { "Flip list", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, fliplist_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - tape section pointer
 *
 * Set by ...
 */
static ui_menu_item_t *file_menu_tape_section = NULL;


/** \brief  'File' menu - tape section
 */
static ui_menu_item_t file_menu_tape[] = {
    /* tape (funny how create & attach are flipped here) */
    { "Create a new tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL, NULL,
        0, 0 },
    { "Attach tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        "attach-tape", ui_tape_attach_callback, NULL,
        GDK_KEY_T, VICE_MOD_MASK },
    { "Detach tape image", UI_MENU_TYPE_ITEM_ACTION,
        "detach-tape", ui_tape_detach_callback, NULL,
        0, 0 },
    { "Datasette controls", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, datasette_control_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - tail section
 */
static ui_menu_item_t file_menu_tail[] = {
    /* cart */
    { "Attach cartridge image", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, cart_attach_submenu,
        0, 0 },
    { "Detach cartridge image(s)", UI_MENU_TYPE_ITEM_ACTION,
        "detach-cart", (void *)uicart_detach, NULL,
        0, 0 },
    { "Cartridge freeze", UI_MENU_TYPE_ITEM_ACTION,
        "freeze-cart", (void *)uicart_trigger_freeze, NULL,
        GDK_KEY_Z, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    /* monitor */
    { "Activate monitor", UI_MENU_TYPE_ITEM_ACTION,
        "monitor", ui_monitor_activate_callback, NULL,
        GDK_KEY_H, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Monitor settings ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Netplay ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Reset", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, reset_submenu,
        0, 0 },
    { "Action on CPU JAM ...", UI_MENU_TYPE_ITEM_ACTION,
        NULL, NULL, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Exit emulator", UI_MENU_TYPE_ITEM_ACTION,
        "exit", ui_close_callback, NULL,
        GDK_KEY_Q, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Edit' menu
 */
static ui_menu_item_t edit_menu[] = {
    { "Copy", UI_MENU_TYPE_ITEM_ACTION,
        "copy", ui_copy_callback, NULL,
        0, 0 },
    { "Paste", UI_MENU_TYPE_ITEM_ACTION,
        "paste", ui_paste_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Snapshot' menu
 */
static ui_menu_item_t snapshot_menu[] = {
    { "Load snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-load", uisnapshot_open_file, NULL,
        GDK_KEY_L, VICE_MOD_MASK },
    { "Save snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-save", uisnapshot_save_file, NULL,
        GDK_KEY_S, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    { "Quickload snapshot", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-quickload", uisnapshot_quickload_snapshot, NULL,
        GDK_KEY_F10, VICE_MOD_MASK },   /* Shortcut doesn't work in MATE, key
                                         is mapped to Maximize Window. Using
                                         the menu to active this item does
                                         work though -- compyx */
    { "Quicksave snapshot", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-quicksave", uisnapshot_quicksave_snapshot, NULL,
        GDK_KEY_F11, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,
#if 0
    { "Select history directory ...", UI_MENU_TYPE_ITEM_ACTION,
        "history-select-dir", uisnapshot_history_select_dir, "0:3",
        0, 0 },
#endif
    { "Start recording events", UI_MENU_TYPE_ITEM_ACTION,
        "history-record-start", uisnapshot_history_record_start, NULL,
        0, 0 },
    { "Stop recording events", UI_MENU_TYPE_ITEM_ACTION,
        "history-record-stop", uisnapshot_history_record_stop, NULL,
        0, 0 },
    { "Start playing back events", UI_MENU_TYPE_ITEM_ACTION,
        "history-playback-start", uisnapshot_history_playback_start, NULL,
        0, 0 },
    { "Stop playing back events", UI_MENU_TYPE_ITEM_ACTION,
        "history-playback-stop", uisnapshot_history_playback_stop, NULL,
        0, 0 },
    { "Set recording milestone", UI_MENU_TYPE_ITEM_ACTION,
        "history-milestone-set", uisnapshot_history_milestone_set, NULL,
        GDK_KEY_E, VICE_MOD_MASK },
    { "Return to milestone", UI_MENU_TYPE_ITEM_ACTION,
        "history-milestone-reset", uisnapshot_history_milestone_reset, NULL,
        GDK_KEY_U, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,
#if 0
    { "Recording start mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "history-recording-start-mode", ui_settings_dialog_create, "20,0",
        0, 0 },

    UI_MENU_SEPARATOR,
#endif

    { "Save media file ...", UI_MENU_TYPE_ITEM_ACTION,
        "media-save", uimedia_dialog_show, NULL,
        GDK_KEY_R, VICE_MOD_MASK|GDK_SHIFT_MASK },

    { "Stop media recording", UI_MENU_TYPE_ITEM_ACTION,
        "media-stop", (void *)uimedia_stop_recording, NULL,
        GDK_KEY_S, VICE_MOD_MASK|GDK_SHIFT_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Settings' menu - head section
 */
static ui_menu_item_t settings_menu_head[] = {
    { "Toggle fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen", ui_fullscreen_callback, NULL,
        GDK_KEY_D, VICE_MOD_MASK },
#if 1
    { "Show menu/status in fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen-widgets", ui_fullscreen_decorations_callback, NULL,
        GDK_KEY_B, VICE_MOD_MASK },
#else
    /* Mac menubar version */
    { "Show statusbar in fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen-widgets", ui_fullscreen_decorations_callback, NULL,
        GDK_KEY_B, VICE_MOD_MASK },
#endif

    UI_MENU_SEPARATOR,

    { "Toggle warp mode", UI_MENU_TYPE_ITEM_CHECK,
        "warp", (void *)(ui_toggle_resource), (void *)"WarpMode",
        GDK_KEY_W, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'Settings' menu - joystick section pointer
 *
 * Set by ...
 */
static ui_menu_item_t *settings_menu_joy_section = NULL;


/** \brief  'Settings' menu - all joystick items
 *
 * Only valid for x64/x64sc/xscpu64/x128/xplus4
 */
static ui_menu_item_t settings_menu_all_joy[] = {

    { "Swap joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "joystick-swap", (void *)(ui_swap_joysticks_callback), NULL,
        GDK_KEY_J, VICE_MOD_MASK },
    { "Swap userport joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "userportjoy-swap", (void *)(ui_swap_userport_joysticks_callback), NULL,
        GDK_KEY_U, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu - control port joystick items
 *
 * Only valid for x64dtv/xcbm5x0
 */
static ui_menu_item_t settings_menu_cbm5x0_joy[] = {

    { "Swap joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "joystick-swap", (void *)(ui_swap_joysticks_callback), NULL,
        GDK_KEY_J, VICE_MOD_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu - userport joystick items
 *
 * Only valid for xvic/xpet/xcbm2
 */
static ui_menu_item_t settings_menu_userport_joy[] = {
    { "Swap userport joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "userportjoy-swap", (void *)(ui_swap_userport_joysticks_callback), NULL,
        GDK_KEY_U, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu tail section
 */
static ui_menu_item_t settings_menu_tail[] = {

    UI_MENU_SEPARATOR,

    /* the settings dialog */
    { "Settings", UI_MENU_TYPE_ITEM_ACTION,
        "settings", ui_settings_dialog_create, NULL,
        0, 0 },
    UI_MENU_TERMINATOR
};


/** \brief  'Debug' menu items
 */
#ifdef DEBUG
static ui_menu_item_t debug_menu[] = {
    { "Trace mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "tracemode", uidebug_trace_mode_callback, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Main CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-maincpu", (void *)(ui_toggle_resource), (void *)"MainCPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Drive #8 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive8", (void *)(ui_toggle_resource), (void *)"Drive0CPU_TRACE",
        0, 0 },
    { "Drive #9 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive9", (void *)(ui_toggle_resource), (void *)"Drive1CPU_TRACE",
        0, 0 },
    { "Drive #10 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive10", (void *)(ui_toggle_resource), (void *)"Drive2CPU_TRACE",
        0, 0 },
    { "Drive #11 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive11", (void *)(ui_toggle_resource), (void *)"Drive3CPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Autoplay playback frames ...", UI_MENU_TYPE_ITEM_ACTION,
        "playframes", uidebug_playback_frames_callback, NULL,
        0, 0 },
    { "Save core dump", UI_MENU_TYPE_ITEM_CHECK,
        "coredump", (void *)(ui_toggle_resource), (void *)"DoCoreDump",
        0, 0 },

    UI_MENU_TERMINATOR
};
#endif


/** \brief  'Help' menu items
 */
static ui_menu_item_t help_menu[] = {
    { "Browse manual", UI_MENU_TYPE_ITEM_ACTION,
        "manual", NULL, NULL,
        0, 0 },
    { "Command line options ...", UI_MENU_TYPE_ITEM_ACTION,
        "cmdline", uicmdline_dialog_show, NULL,
        0, 0 },
    { "Compile time features ...", UI_MENU_TYPE_ITEM_ACTION,
        "features", uicompiletimefeatures_dialog_show, NULL,
        0, 0 },
    { "About VICE", UI_MENU_TYPE_ITEM_ACTION,
        "about", ui_about_dialog_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  Create the top menu bar with standard submenus
 *
 * \return  GtkMenuBar
 */
GtkWidget *ui_machine_menu_bar_create(void)
{
    GtkWidget *menu_bar;

    /* determine which joystick swap menu items should be added */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_PLUS4:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* fall through */
        case VICE_MACHINE_SCPU64:
            /* add both swap-joy and swap-userport-joy */
            settings_menu_joy_section = settings_menu_all_joy;
            break;
        case VICE_MACHINE_CBM5x0:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* fall through */
        case VICE_MACHINE_C64DTV:
            /* only add swap-joy */
            settings_menu_joy_section = settings_menu_cbm5x0_joy;
            break;
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_CBM6x0:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* only add swap-userport-joy */
            settings_menu_joy_section = settings_menu_userport_joy;
            break;
        case VICE_MACHINE_VSID:
            exit(1);
            break;
        default:
            break;
    }

    menu_bar = ui_menu_bar_create();

    /* generate File menu */
    ui_menu_file_add(file_menu_head);
    if (file_menu_tape_section != NULL) {
        ui_menu_file_add(file_menu_tape_section);
    }
    ui_menu_file_add(file_menu_tail);

    /* generate Edit menu */
    ui_menu_edit_add(edit_menu);
    /* generate Snapshot menu */
    ui_menu_snapshot_add(snapshot_menu);

    /* generate Settings menu */
    ui_menu_settings_add(settings_menu_head);
    if (settings_menu_joy_section != NULL) {
        ui_menu_settings_add(settings_menu_joy_section);
    }
    ui_menu_settings_add(settings_menu_tail);

#ifdef DEBUG
    /* generate Debug menu */
    ui_menu_debug_add(debug_menu);
#endif

    /* generate Help menu */
    ui_menu_help_add(help_menu);

    return menu_bar;
}
