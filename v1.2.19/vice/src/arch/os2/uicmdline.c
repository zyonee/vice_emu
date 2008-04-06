/*
 * ui_cmdline.c - Commandline output for Vice/2
 *
 * Written by
 *  Thomas Bretz <tbretz@gsi.de>
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

#include <string.h>

#include "cmdline.h"

#include "pm_cmdline.h"

void ui_cmdline_show_help(int num_options, cmdline_option_t *options)
{
    int i, j;
    int jmax=0;

    ui_cmdline_options=options;
    ui_cmdline_lines=num_options;
    for (i=0; i<ui_cmdline_lines; i++) {
        j    =strlen(ui_cmdline_options[i].name)+1;
        j   +=strlen((ui_cmdline_options[i].need_arg && ui_cmdline_options[i].param_name)?
                     ui_cmdline_options[i].param_name:"")+1;
        jmax = j>jmax ? j : jmax;
        j   += strlen(ui_cmdline_options[i].description)+1;
        ui_cmdline_chars = j>ui_cmdline_chars ? j : ui_cmdline_chars;
    }
    sprintf(optFormat,"%%-%ds%%s",jmax);

    ui_cmdline_window(jmax, ui_cmdline_chars);
}
