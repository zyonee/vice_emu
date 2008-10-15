/*
 * pet-cmdline-options.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  André Fachat <fachat@physik.tu-chemnitz.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include "pet-cmdline-options.h"
#include "pets.h"
#include "translate.h"

static const cmdline_option_t cmdline_options[] = {
    { "-model", CALL_FUNCTION, 1, pet_set_model, NULL, NULL, NULL,
      IDCLS_P_MODELNUMBER, IDCLS_SPECIFY_PET_MODEL },
    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_KERNAL_ROM_NAME },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_BASIC_ROM_NAME },
    { "-editor", SET_RESOURCE, 1, NULL, NULL, "EditorName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_EDITOR_ROM_NAME },
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CHARGEN_ROM_NAME },
    { "-rom9", SET_RESOURCE, 1, NULL, NULL, "RomModule9Name", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_4K_ROM_9XXX_NAME },
    { "-romA", SET_RESOURCE, 1, NULL, NULL, "RomModuleAName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_4K_ROM_AXXX_NAME },
    { "-romB", SET_RESOURCE, 1, NULL, NULL, "RomModuleBName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_4K_ROM_BXXX_NAME },
    { "-petram9", SET_RESOURCE, 0, NULL, NULL, "Ram9", (void *)1,
      0, IDCLS_PET8296_ENABLE_4K_RAM_AT_9000 },
    { "+petram9", SET_RESOURCE, 0, NULL, NULL, "Ram9", (void *)0,
      0, IDCLS_PET8296_DISABLE_4K_RAM_AT_9000 },
    { "-petramA", SET_RESOURCE, 0, NULL, NULL, "RamA", (void *)1,
      0, IDCLS_PET8296_ENABLE_4K_RAM_AT_A000 },
    { "+petramA", SET_RESOURCE, 0, NULL, NULL, "RamA", (void *)0,
      0, IDCLS_PET8296_DISABLE_4K_RAM_AT_A000 },
    { "-superpet", SET_RESOURCE, 0, NULL, NULL, "SuperPET", (void *)1,
      0, IDCLS_ENABLE_SUPERPET_IO },
    { "+superpet", SET_RESOURCE, 0, NULL, NULL, "SuperPET", (void *)0,
      0, IDCLS_DISABLE_SUPERPET_IO },
    { "-basic1", SET_RESOURCE, 0, NULL, NULL, "Basic1", (void *)1,
      0, IDCLS_ENABLE_ROM_1_KERNAL_PATCHES },
    { "+basic1", SET_RESOURCE, 0, NULL, NULL, "Basic1", (void *)0,
      0, IDCLS_DISABLE_ROM_1_KERNAL_PATCHES },
    { "-basic1char", SET_RESOURCE, 0, NULL, NULL, "Basic1Chars",
      (void *)1,
      0, IDCLS_SWITCH_UPPER_LOWER_CHARSET },
    { "+basic1char", SET_RESOURCE, 0, NULL, NULL, "Basic1Chars",
      (void *)0,
      0, IDCLS_NO_SWITCH_UPPER_LOWER_CHARSET },
    { "-eoiblank", SET_RESOURCE, 0, NULL, NULL, "EoiBlank", (void *)1,
      0, IDCLS_EOI_BLANKS_SCREEN },
    { "+eoiblank", SET_RESOURCE, 0, NULL, NULL, "EoiBlank", (void *)0,
      0, IDCLS_EOI_DOES_NOT_BLANK_SCREEN },
    {  "-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)1,
      0, IDCLS_ENABLE_EMULATOR_ID },
    {  "+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)0,
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

int pet_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}
