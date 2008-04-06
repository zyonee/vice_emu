/*
 * ui.h - Simple Xaw-based graphical user interface.  It uses widgets
 * from the Free Widget Foundation and Robert W. McMullen.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#ifndef _UI_XAW_H
#define _UI_XAW_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "types.h"
#include "palette.h"

typedef Widget ui_window_t;

typedef void (*ui_exposure_handler_t) (unsigned int width,
                                       unsigned int height);

typedef enum {
    UI_BUTTON_NONE, UI_BUTTON_CLOSE, UI_BUTTON_OK, UI_BUTTON_CANCEL,
    UI_BUTTON_YES, UI_BUTTON_NO, UI_BUTTON_RESET, UI_BUTTON_HARDRESET,
    UI_BUTTON_MON, UI_BUTTON_DEBUG, UI_BUTTON_CONTENTS, UI_BUTTON_AUTOSTART
} ui_button_t;

typedef enum {
    UI_JAM_RESET, UI_JAM_HARD_RESET, UI_JAM_MONITOR, UI_JAM_DEBUG
} ui_jam_action_t;

typedef XtCallbackProc ui_callback_t;
typedef XtPointer ui_callback_data_t;

#define UI_CALLBACK(name)                               \
    void name(Widget w, ui_callback_data_t client_data, \
              ui_callback_data_t call_data)

/* These resources are used only by the UI modules.  */
typedef struct {
    char *html_browser_command;
    int use_private_colormap;
    int save_resources_on_exit;
    int depth;
} _ui_resources_t;
extern _ui_resources_t _ui_resources;

extern Widget _ui_top_level;

extern Display *display;
extern int screen;
extern Visual *visual;
extern int depth;

/* ------------------------------------------------------------------------- */

extern int ui_init_resources(void);
extern int ui_init_cmdline_options(void);

extern int ui_init(int *argc, char **argv);
extern int ui_init_finish(void);
extern void ui_set_left_menu(Widget w);
extern void ui_set_right_menu(Widget w);
extern void ui_set_application_icon(Pixmap icon_pixmap);
extern ui_window_t ui_open_canvas_window(const char *title, int width, int height, int no_autorepeat, ui_exposure_handler_t exposure_proc, const palette_t *p, PIXEL pixel_return[]);
extern void ui_resize_canvas_window(ui_window_t w, int height, int width);
extern void ui_map_canvas_window(ui_window_t w);
extern void ui_unmap_canvas_window(ui_window_t w);
extern Window ui_canvas_get_drawable(ui_window_t w);
extern void ui_display_speed(float percent, float framerate, int warp_flag);
extern void ui_toggle_drive_status(int state);
extern void ui_display_drive_track(double track_number);
extern void ui_display_drive_led(int status);
extern void ui_display_paused(int flag);
extern void ui_dispatch_next_event(void);
extern void ui_dispatch_events(void);
extern void ui_error(const char *format,...);
extern ui_jam_action_t ui_jam_dialog(const char *format,...);
extern void ui_message(const char *format,...);
extern void ui_show_text(const char *title, const char *text, int width, int height);
extern char *ui_select_file(const char *title, char *(*read_contents_func)(const char *), int allow_autostart, ui_button_t *button_return);
extern ui_button_t ui_input_string(const char *title, const char *prompt, char *buf, unsigned int buflen);
extern ui_button_t ui_ask_confirmation(const char *title, const char *text);
extern void ui_autorepeat_on(void);
extern void ui_autorepeat_off(void);
extern void ui_update_menus(void);
extern int ui_extend_image_dialog(void);
extern Widget ui_create_transient_shell(Widget parent, const char *name);
extern void ui_popdown(Widget w);
extern void ui_popup(Widget w, const char *title, Boolean wait_popdown);

#endif /* !defined (_UI_XAW_H) */
