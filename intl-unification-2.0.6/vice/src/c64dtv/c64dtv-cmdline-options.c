/*
 * c64dtv-cmdline-options.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "c64dtv-cmdline-options.h"
#include "cmdline.h"
#include "machine.h"
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
    { "-chargen", SET_RESOURCE, 1, NULL, NULL, "ChargenName", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_CHARGEN_ROM_NAME },
    { "-emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)1,
      0, IDCLS_ENABLE_EMULATOR_ID },
    { "+emuid", SET_RESOURCE, 0, NULL, NULL, "EmuID", (void *)0,
      0, IDCLS_DISABLE_EMULATOR_ID },
    { "-kernalrev", SET_RESOURCE, 1, NULL, NULL, "KernalRev", NULL,
      IDCLS_P_REVISION, IDCLS_PATCH_KERNAL_TO_REVISION },
#ifdef COMMON_KBD
    { "-keymap", SET_RESOURCE, 1, NULL, NULL, "KeymapIndex", NULL,
      IDCLS_P_NUMBER, IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2 },
    { "-symkeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapSymFile", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_SYM_KEYMAP_FILE_NAME },
    { "-symdekeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapSymDeFile", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP },
    { "-poskeymap", SET_RESOURCE, 1, NULL, NULL, "KeymapPosFile", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_POS_KEYMAP_FILE_NAME },
#endif
    { "-hummerusrjoy", SET_RESOURCE, 0, NULL, NULL, "HummerUserportJoy", (void *)1,
      0, IDCLS_ENABLE_HUMMER_USERPORT_JOY },
    { "+hummerusrjoy", SET_RESOURCE, 0, NULL, NULL, "HummerUserportJoy", (void *)0,
      0, IDCLS_DISABLE_HUMMER_USERPORT_JOY },
    { "-hummerusrjoyport", SET_RESOURCE, 1, NULL, NULL, "HummerUserportJoyPort", NULL,
      IDCLS_P_PORT, IDCLS_HUMMER_JOY_PORT },
    { NULL }
};

int c64dtv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}
