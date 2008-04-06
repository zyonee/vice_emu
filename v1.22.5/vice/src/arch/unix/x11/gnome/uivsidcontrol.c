/*
 * uivsidcontrol.c - UI controls for VSID
 *
 * Written by
 *  Martin Pottendorfer (Martin.Pottendorfer@alcatel.at)
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

#include "lib.h"
#include "ui.h"
#include "uiarch.h"
#include "vsiduiunix.h"


static GtkWidget *current_line;
static char *author, *copyright, *name, *vsidsync, *model, *irq;
static int tune;
static char *line;

static void update_line(void)
{
    if (line)
        lib_free(line);
    line = lib_msprintf(_("Name: %s\nTune: %d\nAuthor: %s\nCopyright: %s\n"
                       "%s\nModel: %s\nIRQ: %s") ,
                     name, tune,
                     author, copyright, vsidsync, model, irq);
    gtk_label_set_text(GTK_LABEL(current_line), line);
}


ui_window_t build_vsid_ctrl_widget(void)
{
    GtkWidget *event_box, *f;

    event_box = gtk_event_box_new();
    f = gtk_frame_new("VSID");

    current_line = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(f), current_line);
    gtk_widget_show(current_line);

    gtk_container_add(GTK_CONTAINER(event_box), f);
    gtk_widget_show(f);

    return event_box;
}

void ui_vsid_setpsid(const char *psid)
{
    if (name)
        lib_free(name);
    name = lib_stralloc(psid);
    update_line();
}

void ui_vsid_settune(const int t)
{
    tune = t;
    update_line();
}

void ui_vsid_setauthor(const char *a)
{
    if (author)
        lib_free(author);
    author = lib_stralloc(a);
    update_line();
}
void ui_vsid_setcopyright(const char *c)
{
    if (copyright)
        lib_free(copyright);
    copyright = lib_stralloc(c);
    update_line();
}

void ui_vsid_setmodel(const char *c)
{
    if (model)
        lib_free(model);
    model = lib_stralloc(c);
    update_line();
}

void ui_vsid_setsync(const char *c)
{
    if (vsidsync)
        lib_free(vsidsync);
    vsidsync = lib_stralloc(c);
    update_line();
}

void ui_vsid_setirq(const char *c)
{
    if (irq)
        lib_free(irq);
    irq = lib_stralloc(c);
    update_line();
}

