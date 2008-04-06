/*
 * uiscreenshot.c - Screenshot UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Daniel Sladic <dsladic@cs.cmu.edu>
 *  Andreas Boose <viceteam@t-online.de>
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
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Toggle.h>

#ifndef ENABLE_TEXTFIELD
#include <X11/Xaw/AsciiText.h>
#else
#include "widgets/TextField.h"
#endif

#include "gfxoutput.h"
#include "screenshot.h"
#include "uimenu.h"
#include "utils.h"

static Widget screenshot_dialog;
static Widget screenshot_dialog_pane;
static Widget file_name_form;
static Widget file_name_label;
static Widget file_name_field;
static Widget browse_button;
static Widget options_form;
static Widget options_filling_box_left;

/* static Widget options_filling_box_right; */
static Widget *driver_buttons;
static Widget driver_label;

static Widget button_box;
static Widget save_button;
static Widget cancel_button;

static char *screenshot_file_name;

#define FILL_BOX_WIDTH          10
#define OPTION_LABELS_WIDTH     50
#define OPTION_LABELS_JUSTIFY   XtJustifyLeft

static UI_CALLBACK(browse_callback)
{
    ui_button_t button;

    char *filename;

    filename = ui_select_file(_("Save screenshot file"), NULL, 0, False, NULL,
                              "*", &button, 0, NULL);

    if (button == UI_BUTTON_OK)
        XtVaSetValues(file_name_field, XtNstring, filename, NULL);

    if (filename != NULL)
        free(filename);
}

static UI_CALLBACK(cancel_callback)
{
    ui_popdown(screenshot_dialog);
}

static UI_CALLBACK(save_callback)
{
    int i, wid, num_buttons;
    String name;
    Boolean driver_flag;
    gfxoutputdrv_t *driver;
    
    ui_popdown(screenshot_dialog);
    wid = (int) UI_MENU_CB_PARAM;

    num_buttons = gfxoutput_num_drivers();
    driver = gfxoutput_drivers_iter_init();
    
    for (i = 0; i < num_buttons; i++) {
        XtVaGetValues(driver_buttons[i], XtNstate, &driver_flag, NULL);
        if (driver_flag)
           break;

        driver = gfxoutput_drivers_iter_next();
    }
    
    if (!driver)
	return;
 
    XtVaGetValues(file_name_field, XtNstring, &name, NULL);
    screenshot_save(driver->name, name, wid);
}

static void build_screenshot_dialog(int wid)
{
    int i, num_buttons;
    gfxoutputdrv_t *driver;
#ifndef ENABLE_TEXTFIELD
    static char *text_box_translations = "#override\n<Key>Return: no-op()";
#else
    static char *text_box_translations = "<Btn1Down>: select-start() focus-in()";
#endif

    if (screenshot_dialog != NULL)
        return;

    screenshot_dialog = ui_create_transient_shell(_ui_top_level,
                                                "screenshotDialog");

    screenshot_dialog_pane = XtVaCreateManagedWidget
        ("screenshotDialogPane",
         panedWidgetClass, screenshot_dialog,
         NULL);
    
    file_name_form = XtVaCreateManagedWidget
        ("fileNameForm",
         formWidgetClass, screenshot_dialog_pane,
         XtNshowGrip, False, NULL);
    
    file_name_label = XtVaCreateManagedWidget
        ("fileNameLabel",
         labelWidgetClass, file_name_form,
         XtNjustify, XtJustifyLeft,
         XtNlabel, _("File name:"),
         XtNborderWidth, 0,
         NULL);

#ifndef ENABLE_TEXTFIELD
    file_name_field = XtVaCreateManagedWidget
        ("fileNameField",
         asciiTextWidgetClass, file_name_form,
         XtNfromHoriz, file_name_label,
         XtNwidth, 240,
         XtNtype, XawAsciiString,
         XtNeditType, XawtextEdit,
         NULL);
#else
    file_name_field = XtVaCreateManagedWidget
        ("fileNameField",
         textfieldWidgetClass, file_name_form,
         XtNfromHoriz, file_name_label,
         XtNwidth, 240,
         XtNstring, "",         /* Otherwise, it does not work correctly.  */
         NULL);
#endif
    XtOverrideTranslations(file_name_field,
                               XtParseTranslationTable(text_box_translations));

    browse_button = XtVaCreateManagedWidget
        ("browseButton",
         commandWidgetClass, file_name_form,
         XtNfromHoriz, file_name_field,
         XtNlabel, _("Browse..."),
         NULL);
    XtAddCallback(browse_button, XtNcallback, browse_callback, NULL);

    options_form = XtVaCreateManagedWidget
        ("optionsForm",
         formWidgetClass, screenshot_dialog_pane,
         XtNskipAdjust, True,
         NULL);

    driver_label = XtVaCreateManagedWidget
        ("ImageTypeLabel",
         labelWidgetClass, options_form,
         XtNborderWidth, 0,
         XtNfromHoriz, options_filling_box_left,
         XtNjustify, OPTION_LABELS_JUSTIFY,
         XtNwidth, OPTION_LABELS_WIDTH,
         XtNleft, XawChainLeft,
         XtNright, XawChainRight,
         XtNheight, 20,
         XtNlabel, _("Image format:"),
         NULL);

    num_buttons = gfxoutput_num_drivers();
    driver_buttons = (Widget *) xmalloc(sizeof(Widget) * num_buttons);
    driver = gfxoutput_drivers_iter_init();

    driver_buttons[0] = XtVaCreateManagedWidget
        (driver->name,
         toggleWidgetClass, options_form,
         XtNfromHoriz, driver_label,
         XtNfromVert, browse_button,
         XtNwidth, 40,
         XtNheight, 20,
         XtNright, XtChainRight,
         XtNleft, XtChainRight,
         XtNlabel, driver->name,
         NULL);
    driver = gfxoutput_drivers_iter_next();
    for (i = 1; i < num_buttons; i++) {
        driver_buttons[i] = XtVaCreateManagedWidget
            (driver->name,
             toggleWidgetClass, options_form,
             XtNfromHoriz, driver_buttons[i-1],
             XtNfromVert, browse_button,
             XtNwidth, 40,
             XtNheight, 20,
             XtNright, XtChainRight,
             XtNleft, XtChainRight,
             XtNlabel, driver->name,
             XtNradioGroup, driver_buttons[i-1],
             NULL);
        driver = gfxoutput_drivers_iter_next();
    }

    button_box = XtVaCreateManagedWidget
        ("buttonBox",
         boxWidgetClass, screenshot_dialog_pane,
         XtNshowGrip, False, NULL);

    save_button = XtVaCreateManagedWidget
        ("saveButton",
         commandWidgetClass, button_box,
         XtNlabel, _("Save"),
         NULL);
    XtAddCallback(save_button, XtNcallback, save_callback, (XtPointer) wid);
    
    cancel_button = XtVaCreateManagedWidget
        ("cancelButton",
         commandWidgetClass, button_box,
         XtNlabel, _("Cancel"),
         NULL);
    XtAddCallback(cancel_button, XtNcallback, cancel_callback, NULL);

    XtVaSetValues(driver_buttons[0], XtNstate, True, NULL);
/*
    XtVaSetValues(save_disk_off_button, XtNstate, True, NULL);
*/
    XtSetKeyboardFocus(screenshot_dialog_pane, file_name_field);
}

int ui_screenshot_dialog(char *name, int wid)
{
    screenshot_file_name = name;
    *screenshot_file_name= 0;
    
    build_screenshot_dialog(wid);
    ui_popup(screenshot_dialog, _("Screen Snapshot"), True);
    return *name ? 0 : -1;
}

