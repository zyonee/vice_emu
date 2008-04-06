/*
 * plus4-cmdline-options.c
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "cmdline.h"
#include "resources.h"


static cmdline_option_t cmdline_options[] =
{
    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      "<name>", "Specify name of Kernal ROM image" },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      "<name>", "Specify name of BASIC ROM image" },
    { "-3plus1lo", SET_RESOURCE, 1, NULL, NULL, "3plus1loName", NULL,
      "<name>", "Specify name of 3plus1 low ROM image" },
    { "-3plus1hi", SET_RESOURCE, 1, NULL, NULL, "3plus1hiName", NULL,
      "<name>", "Specify name of 3plus1 high ROM image" },
    { "-ramsize", SET_RESOURCE, 1, NULL, NULL, "RamSize", NULL,
      "<ramsize>", "Specify size of RAM installed in kb (16/32/64)" },
    { NULL }
};

int plus4_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

