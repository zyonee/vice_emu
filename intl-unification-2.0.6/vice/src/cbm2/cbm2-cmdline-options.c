/*
 * cbm2-cmdline-options.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  André Fachat <fachat@physik.tu-chemnitz.de>
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

#include "cbm2-cmdline-options.h"
#include "cbm2mem.h"
#include "cmdline.h"
#include "translate.h"

static const cmdline_option_t cmdline_options[] = {
    { "-model", CALL_FUNCTION, 1, cbm2_set_model, NULL, NULL, NULL,
     IDCLS_P_MODELNUMBER, IDCLS_SPECIFY_CBM2_MODEL },
    { "-usevicii", SET_RESOURCE, 0, NULL, NULL, "UseVicII", (void *)1,
     0, IDCLS_SPECIFY_TO_USE_VIC_II },
    { "+usevicii", SET_RESOURCE, 0, NULL, NULL, "UseVicII", (void *)0,
     0, IDCLS_SPECIFY_TO_USE_CRTC },
    { "-modelline", SET_RESOURCE, 1, NULL, NULL, "ModelLine", NULL,
     IDCLS_P_LINENUMBER, IDCLS_SPECIFY_CBM2_MODEL_HARDWARE },
    { "-ramsize", SET_RESOURCE, 1, NULL, NULL, "RamSize", NULL,
     IDCLS_P_RAMSIZE, IDCLS_SPECIFY_SIZE_OF_RAM },

    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_KERNAL_ROM_NAME },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_BASIC_ROM_NAME },
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CHARGEN_ROM_NAME },

    { "-cart1", SET_RESOURCE, 1, NULL, NULL, "Cart1Name", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_ROM_1000_NAME },
    { "-cart2", SET_RESOURCE, 1, NULL, NULL, "Cart2Name", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_ROM_2000_NAME },
    { "-cart4", SET_RESOURCE, 1, NULL, NULL, "Cart4Name", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_ROM_4000_NAME },
    { "-cart6", SET_RESOURCE, 1, NULL, NULL, "Cart6Name", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_ROM_6000_NAME },
    { "-ram08", SET_RESOURCE, 0, NULL, NULL, "Ram08", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_0800 },
    { "+ram08", SET_RESOURCE, 0, NULL, NULL, "Ram08", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_0800 },
    { "-ram1", SET_RESOURCE, 0, NULL, NULL, "Ram1", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_1000 },
    { "+ram1", SET_RESOURCE, 0, NULL, NULL, "Ram1", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_1000 },
    { "-ram2", SET_RESOURCE, 0, NULL, NULL, "Ram2", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_2000 },
    { "+ram2", SET_RESOURCE, 0, NULL, NULL, "Ram2", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_2000 },
    { "-ram4", SET_RESOURCE, 0, NULL, NULL, "Ram4", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_4000 },
    { "+ram4", SET_RESOURCE, 0, NULL, NULL, "Ram4", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_4000 },
    { "-ram6", SET_RESOURCE, 0, NULL, NULL, "Ram6", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_6000 },
    { "+ram6", SET_RESOURCE, 0, NULL, NULL, "Ram6", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_6000 },
    { "-ramC", SET_RESOURCE, 0, NULL, NULL, "RamC", (void *)1,
      0, IDCLS_ENABLE_RAM_MAPPING_IN_C000 },
    { "+ramC", SET_RESOURCE, 0, NULL, NULL, "RamC", (void *)0,
      0, IDCLS_DISABLE_RAM_MAPPING_IN_C000 },
    { "-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)1,
      0, IDCLS_ENABLE_EMULATOR_ID },
    { "+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)0,
      0, IDCLS_DISABLE_EMULATOR_ID },
#ifdef COMMON_KBD
    { "-keymap", SET_RESOURCE, 1, NULL, NULL, "KeymapIndex", NULL,
      IDCLS_P_NUMBER, IDCLS_SPECIFY_KEYMAP_INDEX },
    { "-grsymkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapGraphicsSymFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME },
    { "-grposkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapGraphicsPosFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME },
    { "-buksymkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapBusinessUKSymFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME },
    { "-bukposkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapBusinessUKPosFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME },
    { "-bdesymkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapBusinessDESymFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME },
    { "-bdeposkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapBusinessDEPosFile",
      NULL, IDCLS_P_NAME,
      IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME },
#endif
    { NULL }
};

int cbm2_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}
