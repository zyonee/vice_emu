/*
 * plus4-cmdline-options.c
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

#include "cmdline.h"
#include "machine.h"
#include "plus4-cmdline-options.h"
#include "plus4memcsory256k.h"
#include "plus4memhannes256k.h"
#include "translate.h"

static const cmdline_option_t cmdline_options[] =
{
    { "-pal", SET_RESOURCE, 0, NULL, NULL, "MachineVideoStandard",
      (void *)MACHINE_SYNC_PAL, 0, IDCLS_USE_PAL_SYNC_FACTOR },
    { "-ntsc", SET_RESOURCE, 0, NULL, NULL, "MachineVideoStandard",
      (void *)MACHINE_SYNC_NTSC, 0, IDCLS_USE_NTSC_SYNC_FACTOR },
    { "-kernal", SET_RESOURCE, 1, NULL, NULL, "KernalName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_KERNAL_ROM_NAME },
    { "-basic", SET_RESOURCE, 1, NULL, NULL, "BasicName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_BASIC_ROM_NAME },
    { "-functionlo", SET_RESOURCE, 1, NULL, NULL, "FunctionLowName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_FUNCTION_LOW_ROM_NAME },
    { "-functionhi", SET_RESOURCE, 1, NULL, NULL, "FunctionHighName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_FUNCTION_HIGH_ROM_NAME },
    { "-c1lo", SET_RESOURCE, 1, NULL, NULL, "c1loName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_1_LOW_ROM_NAME },
    { "-c1hi", SET_RESOURCE, 1, NULL, NULL, "c1hiName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_1_HIGH_ROM_NAME },
    { "-c2lo", SET_RESOURCE, 1, NULL, NULL, "c2loName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_2_LOW_ROM_NAME },
    { "-c2hi", SET_RESOURCE, 1, NULL, NULL, "c2hiName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CART_2_HIGH_ROM_NAME },
    { "-ramsize", SET_RESOURCE, 1, NULL, NULL, "RamSize", NULL,
      IDCLS_P_RAMSIZE, IDCLS_SPECIFY_RAM_INSTALLED },
#ifdef COMMON_KBD
    { "-keymap", SET_RESOURCE, 1, NULL, NULL, "KeymapIndex", NULL,
      IDCLS_P_NUMBER, IDCLS_SPECIFY_KEYMAP_FILE_INDEX },
    { "-symkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapSymFile", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_SYM_KEYMAP_FILE_NAME },
    { "-poskeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapPosFile", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_POS_KEYMAP_FILE_NAME },
#endif
    { NULL }
};

int plus4_cmdline_options_init(void)
{
    if (h256k_cmdline_options_init()<0)
      return -1;
    if (cs256k_cmdline_options_init()<0)
      return -1;

    return cmdline_register_options(cmdline_options);
}
