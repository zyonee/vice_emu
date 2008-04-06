/*
 * about.c - Info about the VICE project, including the GPL.
 *
 * Written by
 *  Martin Pottendorfer
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

#include "version.h"
#include "info.h"
#include "ui.h"

#include <gnome.h>
GtkWidget *about;

static void license_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("VICE is FREE software!"), license_text, 500, 300);
}

static void warranty_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("No warranty!"), warranty_text, 500, 300);
}

static void contrib_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("Contributors to the VICE project"), contrib_text, 500, 300);
}


void ui_about(gpointer data)
{
    GtkWidget *button;
    
    const gchar *authors[] = {
#ifdef __GNUC__
	_("The VICE Team"), 
#else
	"The VICE Team",
#endif
        "Copyright @ 1997-2001 Daniel Sladic\n",
        "Copyright @ 1998-2001 Andreas Boose\n",
        "Copyright @ 1998-2001 Tibor Biczo\n",
        "Copyright @ 1999-2001 Andreas Dehmel\n",
        "Copyright @ 1999-2001 Thomas Bretz\n",
        "Copyright @ 1999-2001 Andreas Matthies\n",
        "Copyright @ 1999-2001 Martin Pottendorfer\n",
        "Copyright @ 1998-2001 Dag Lem\n",
        "Copyright @ 2000-2001 Markus Brenner\n",
        "Copyright @ 2000-2001 Spiro Trikaliotis\n",
	"",
#ifdef __GNUC__
	_("Official VICE homepage:"),
#else
	"Official VICE homepage:",
#endif
	"http://www.cs.cmu.edu/~dsladic/vice/vice.html",
	NULL};
    if (!about)
    {
	about = gnome_about_new("V I C E", VERSION, "", authors, "", NULL);
	gtk_signal_connect(GTK_OBJECT(about),
			   "destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			   &about);
	button = gnome_stock_or_ordinary_button (_("License"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG(about)->action_area), 
			    button, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(license_cb), NULL);
	gtk_widget_show(button);
	button = gnome_stock_or_ordinary_button (_("Warranty"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG(about)->action_area), 
			    button, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(warranty_cb), NULL);
	gtk_widget_show(button);
	button = gnome_stock_or_ordinary_button (_("Contributors"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG(about)->action_area), 
			    button, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(contrib_cb), NULL);
	gtk_widget_show(button);
    }
    else
    {
	gdk_window_show(about->window);
	gdk_window_raise(about->window);
    }

    ui_make_window_transient(_ui_top_level, about);
    gtk_widget_show(about);
}

