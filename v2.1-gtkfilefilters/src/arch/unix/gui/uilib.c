/*
 * uilib.c
 *
 * Written by
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
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "uilib.h"
#include "uimenu.h"
#include "vsync.h"


void uilib_select_string(const char *resname, const char *title,
                         const char *name)
{
    ui_button_t button;
    const char *value;
    char *new_value;
    int len;

    vsync_suspend_speed_eval();

    resources_get_string(resname, &value);

    if (value == NULL)
        value = "";

    len = strlen(value) * 2;
    if (len < 255)
        len = 255;

    new_value = lib_calloc(1, len + 1);
    strncpy(new_value, value, len);

    button = ui_input_string(title, name, new_value, len);

    if (button == UI_BUTTON_OK)
        resources_set_string(resname, new_value);

    lib_free(new_value);
}

static const char* all_files_filter[]       = { "*", NULL };
static const char* palette_filter[]         = { "*.[vV][pP][lL]", NULL };
static const char* snapshot_filter[]        = { "*.vsf", NULL };
static const char* disk_image_filter[]      = { "*.[dD]64", "*.[dD]71", "*.[dD]80", "*.[dD]81", "*.[dD]82", "*.[gG]64", "*.[gG]41", "*.[xX]64", NULL };
static const char* tape_image_filter[]      = { "*.[tT]64", "*.[tT][aA][pP]", NULL };
static const char* cartridge_image_filter[] = { "*.[cC][rR][tT]", NULL };
static const char* raw_cart_filter[]        = { "*.[bB][iI][nN]", NULL };
static const char* flip_list_filter[]       = { "*.[vV][fF][lL]", NULL };
static const char* romset_filter[]          = { "*.[vV][rR][sS]", NULL };
static const char* romset_archives_filter[] = { "*.[vV][rR][aA]", NULL };
static const char* keymap_filter[]          = { "*.[vV][kK][mM]", NULL };
static const char* emulator_filter[]        = { "*.[pP][rR][gG]", "*.[pP]00", NULL };
static const char* wav_filter[]             = { "*.[wW][aA][vV]", NULL };
static const char* voc_filter[]             = { "*.[vV][oO][cC]", NULL };
static const char* iff_filter[]             = { "*.[iI][fF][fF]", NULL };
static const char* aiff_filter[]            = { "*.[aA][iI][fF][fF]", NULL };
static const char* mp3_filter[]             = { "*.[mM][pP]3", NULL };
static const char* serial_filter[]          = { "ttyS*", NULL };
static const char* vic20cart_filter[]       = { "*.[pP][rR][gG]", NULL };
static const char* sid_filter[]             = { "*.[pP][sS][iI][dD]", "*.sid", NULL };
static const char* dtvrom_filter[]          = { "*.[bB][iI][nN]", NULL };

/* this must be in sync with uilib_file_filter_enum_t */
uilib_file_filter_t filters[] = {
  { "All files"            , all_files_filter },
  { "Palette files"        , palette_filter },
  { "Snapshot files"       , snapshot_filter },
  { "Disk image files"     , disk_image_filter },
  { "Tape image files"     , tape_image_filter },
  { "Cartridge image files", cartridge_image_filter },
  { "Raw cart files"       , raw_cart_filter },
  { "Flip list files"      , flip_list_filter },
  { "Romset files"         , romset_filter },
  { "Romset archives"      , romset_archives_filter },
  { "Keymap files"         , keymap_filter },
  { "Emulator files"       , emulator_filter },
  { "WAV files"            , wav_filter },
  { "VOC files"            , voc_filter },
  { "IFF files"            , iff_filter },
  { "AIFF files"           , aiff_filter },
  { "MP3 files"            , mp3_filter },
  { "Serial ports"         , serial_filter },
  { "VIC20 cartridges"     , vic20cart_filter },
  { "SID files"            , sid_filter },
  { "C64DTV ROM images"    , dtvrom_filter }
};

