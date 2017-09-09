/*
 * filechooserhelpers.c - GtkFileChooser helper functions
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

#include "vice.h"

#include <gtk/gtk.h>

#include "lib.h"
#include "util.h"

#include "filechooserhelpers.h"


/*
 * 'Stock' file patterns
 */

/** \brief  Patterns for all files
 */
const gchar *file_chooser_pattern_all[] = {
    "*.*", NULL
};


/** \brief  Patterns for disk images
 */
const gchar *file_chooser_pattern_disk[] = {
    "*.d64", "*.d67", "*.d71", "*.d8[0-2]",
    "*.d1m", "*.d2m", "*.d4m",
    "*.g64", "*.g71", "*.g41", "*.p64",
    "*.x64",
    NULL
};


/** \brief  Patterns for tapes
 *
 * T64 is NOT a tape, so probably should be moved to a 'archive' pattern group,
 * together with ZipCode, Lynx, Ark, etc.
 */
const gchar *file_chooser_pattern_tape[] = {
    "*.t64", "*.tap", NULL
};


/** \brief  Patterns for program files
 */
const gchar *file_chooser_pattern_prg[] = {
    "*.prg", "*.p[0-9][0-9]"
};


/** \brief  C64 native archives
 *
 * Not all of these are supported, Lynx and ZipCoded disks are supported through
 * calling c1541.
 */
const char *file_chooser_pattern_archive[] = {
    "*.ark",    /* ARK archive */
    "*.lnx",    /* Lynx archive */
    "[1-4]1*",  /* ZipCode disk */
    "[1-6]!!*", /* ZipSix */
    "[a-z]!*",  /* ZipFile*/
    NULL
};


/** \brief  Patterns for host-compressed files
 *
 * XXX: Once we have libarchive implemented, we could probably query libarchive
 *      for the extensions supported
 */
const gchar *file_chooser_pattern_zip[] = {
    "*.bz2", "*.gz", ".rar", "*.[zZ]", "*.zip"
};



/** \brief  Create a GtkFileFilter instance from \a filter
 *
 * \param[in]   filter  name and patterns for the filter
 *
 * Example:
 * \code{.c}
 *  const ui_file_filter_t data = {
 *      "disk image",
 *      { "*.d64", "*.d71", "*.d8[0-2]", NULL }
 *  };
 *  GtkFileFilter *filter = create_file_chooser_filter(data);
 * \endcode
 *
 * \return  a new GtkFileFilter instance
 */
GtkFileFilter *create_file_chooser_filter(const ui_file_filter_t filter)
{
    GtkFileFilter *ff;
    size_t i;
    char *globs;
    char *name;

    globs = util_strjoin(filter.patterns, ";");
    name = util_concat(filter.name, " (", globs, ")", NULL);
    lib_free(globs);

    ff = gtk_file_filter_new();
    gtk_file_filter_set_name(ff, name);
    for (i = 0; filter.patterns[i] != NULL; i++) {
        gtk_file_filter_add_pattern(ff, filter.patterns[i]);
    }

    /* gtk_file_filter_set_name() makes a copy, so we can free name here
     * (according to the Git repo on 2017-09-09)
     */
    lib_free(name);

    return ff;
}

