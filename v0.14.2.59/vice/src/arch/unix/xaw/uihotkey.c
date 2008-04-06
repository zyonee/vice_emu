/*
 * uihotkeys.h - Implementation of UI hotkeys.
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

#include "vice.h"

#include <stdlib.h>

#include "uihotkey.h"

#include "utils.h"

/* If this is #defined, `Alt' is handled the same as `Meta'.  On
   systems which have Meta, it's better to use Meta instead of Alt as
   a shortcut modifier (because Alt is usually used by Window
   Managers), but systems that don't have Meta (eg. GNU/Linux, HP-UX)
   would suffer then.  So it's easier to just handle Meta as Alt in
   such cases.  */
#define ALT_AS_META

typedef struct {
    ui_hotkey_modifier_t modifier;
    KeySym keysym;
    ui_callback_t callback;
    ui_callback_data_t client_data;
} registered_hotkey_t;

static registered_hotkey_t *registered_hotkeys;
static int num_registered_hotkeys;
static int num_allocated_hotkeys;

static int alt_count;
static int right_ctrl_count;
static int meta_count;

/* ------------------------------------------------------------------------- */

int ui_hotkey_init(void)
{
    if (registered_hotkeys != NULL) {
        free(registered_hotkeys);
        num_registered_hotkeys = num_allocated_hotkeys = 0;
        alt_count = right_ctrl_count = meta_count = 0;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

void ui_hotkey_register(ui_hotkey_modifier_t modifier,
                        KeySym keysym, ui_callback_t callback,
                        ui_callback_data_t client_data)
{
    registered_hotkey_t *p;

    if (registered_hotkeys == 0) {
        num_allocated_hotkeys = 32;
        registered_hotkeys = xmalloc(num_allocated_hotkeys
                                     * sizeof(registered_hotkey_t));
        num_registered_hotkeys = 0;
    } else if (num_registered_hotkeys == num_allocated_hotkeys) {
        num_allocated_hotkeys *= 2;
        registered_hotkeys = xrealloc(registered_hotkeys,
                                      (num_allocated_hotkeys
                                       * sizeof(registered_hotkey_t)));
    }

    p = registered_hotkeys + num_registered_hotkeys;

    p->modifier = modifier;
    p->keysym = keysym;
    p->callback = callback;
    p->client_data = client_data;

    num_registered_hotkeys++;
}

/* ------------------------------------------------------------------------- */

void ui_hotkey_event_handler(Widget w, XtPointer closure,
                             XEvent *xevent,
                             Boolean *continue_to_dispatch)
{
    static char buffer[20];
    KeySym keysym;
    XComposeStatus compose;
    int i;

    XLookupString(&xevent->xkey, buffer, 20, &keysym, &compose);

    /* Bad things could happen if focus goes away and then comes
       back...  */
    if (xevent->type == FocusOut) {
        alt_count = right_ctrl_count = meta_count = 0;
        return;
    }

    switch (keysym) {
#ifndef ALT_AS_META
      case XK_Alt_L:
      case XK_Alt_R:
        if (xevent->type == KeyPress)
            alt_count++;
        else if (xevent->type == KeyRelease && alt_count > 0)
            alt_count--;
        break;
#endif
#ifdef ALT_AS_META
      case XK_Alt_L:
      case XK_Alt_R:
#endif
      case XK_Meta_L:
      case XK_Meta_R:
        if (xevent->type == KeyPress)
            meta_count++;
        else if (xevent->type == KeyRelease && meta_count > 0)
            meta_count--;
        break;
      /* case XK_Control_L: */
      case XK_Control_R:
        if (xevent->type == KeyPress)
            right_ctrl_count++;
        else if (xevent->type == KeyRelease && right_ctrl_count > 0)
            right_ctrl_count--;
        break;
      default:
        if (xevent->type == KeyPress && right_ctrl_count != 0) {
            registered_hotkey_t *p = registered_hotkeys;

            for (i = 0; i < num_registered_hotkeys; i++, p++) {
                if (p->keysym == keysym) {
                    if ((p->modifier & UI_HOTMOD_ALT)) {
                        if (alt_count == 0)
                            continue;
                    } else if (alt_count > 0)
                        continue;
                    if (p->modifier & UI_HOTMOD_CTRL) {
                        if (right_ctrl_count == 0)
                            continue;
                    } else if (right_ctrl_count > 0)
                        continue;
                    if (p->modifier & UI_HOTMOD_META) {
                        if (meta_count == 0)
                            continue;
                    } else if (meta_count > 0)
                        continue;
                    p->callback(NULL, p->client_data, NULL);
                    *continue_to_dispatch = False;
                    break;
                }
            }
        }
    }
}
