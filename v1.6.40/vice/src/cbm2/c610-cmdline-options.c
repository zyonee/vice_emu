/*
 * c610-cmdline-options.c
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#include "c610mem.h"
#include "cmdline.h"
#include "resources.h"


static cmdline_option_t cmdline_options[] = {
    { "-model", CALL_FUNCTION, 1, cbm2_set_model, NULL, NULL, NULL,
     "<modelnumber>", "Specify CBM-II model to emulate" },
    { "-usevicii", SET_RESOURCE, 0, NULL, NULL, "UseVicII", (resource_value_t)1,
     NULL, "Specify to use VIC-II" },
    { "+usevicii", SET_RESOURCE, 0, NULL, NULL, "UseVicII", (resource_value_t)0,
     NULL, "Specify use CRTC" },
    { "-modelline", SET_RESOURCE, 1, NULL, NULL, "ModelLine", NULL,
     "<linenumber>", "Specify CBM-II model hardware (0=6x0, 1=7x0)" },
    { "-ramsize", SET_RESOURCE, 1, NULL, NULL, "RamSize", NULL,
     "<ramsize>", "Specify size of RAM (64/128/256/512/1024 kByte)" },

    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      "<name>", "Specify name of Kernal ROM image" },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      "<name>", "Specify name of BASIC ROM image" },
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      "<name>", "Specify name of character generator ROM image" },

    { "-cart1", SET_RESOURCE, 1, NULL, NULL, "Cart1Name", NULL,
      "<name>", "Specify name of cartridge ROM image for $1000" },
    { "-cart2", SET_RESOURCE, 1, NULL, NULL, "Cart2Name", NULL,
      "<name>", "Specify name of cartridge ROM image for $2000-$3fff" },
    { "-cart4", SET_RESOURCE, 1, NULL, NULL, "Cart4Name", NULL,
      "<name>", "Specify name of cartridge ROM image for $4000-$5fff" },
    { "-cart6", SET_RESOURCE, 1, NULL, NULL, "Cart6Name", NULL,
      "<name>", "Specify name of cartridge ROM image for $6000-$7fff" },
    { "-ram08", SET_RESOURCE, 0, NULL, NULL, "Ram08", (resource_value_t)1,
      NULL, "Enable RAM mapping in $0800-$0FFF" },
    { "+ram08", SET_RESOURCE, 0, NULL, NULL, "Ram08", (resource_value_t)0,
      NULL, "Disable RAM mapping in $0800-$0FFF" },
    { "-ram1", SET_RESOURCE, 0, NULL, NULL, "Ram1", (resource_value_t)1,
      NULL, "Enable RAM mapping in $1000-$1FFF" },
    { "+ram1", SET_RESOURCE, 0, NULL, NULL, "Ram1", (resource_value_t)0,
      NULL, "Disable RAM mapping in $1000-$1FFF" },
    { "-ram2", SET_RESOURCE, 0, NULL, NULL, "Ram2", (resource_value_t)1,
      NULL, "Enable RAM mapping in $2000-$3FFF" },
    { "+ram2", SET_RESOURCE, 0, NULL, NULL, "Ram2", (resource_value_t)0,
      NULL, "Disable RAM mapping in $2000-$3FFF" },
    { "-ram4", SET_RESOURCE, 0, NULL, NULL, "Ram4", (resource_value_t)1,
      NULL, "Enable RAM mapping in $4000-$5FFF" },
    { "+ram4", SET_RESOURCE, 0, NULL, NULL, "Ram4", (resource_value_t)0,
      NULL, "Disable RAM mapping in $4000-$5FFF" },
    { "-ram6", SET_RESOURCE, 0, NULL, NULL, "Ram6", (resource_value_t)1,
      NULL, "Enable RAM mapping in $6000-$7FFF" },
    { "+ram6", SET_RESOURCE, 0, NULL, NULL, "Ram6", (resource_value_t)0,
      NULL, "Disable RAM mapping in $6000-$7FFF" },
    { "-ramC", SET_RESOURCE, 0, NULL, NULL, "RamC", (resource_value_t)1,
      NULL, "Enable RAM mapping in $C000-$CFFF" },
    { "+ramC", SET_RESOURCE, 0, NULL, NULL, "RamC", (resource_value_t)0,
      NULL, "Disable RAM mapping in $C000-$CFFF" },

    { "-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t)1,
      NULL, "Enable emulator identification" },
    { "+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (resource_value_t)0,
      NULL, "Disable emulator identification" },
    { NULL }
};

int c610_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

