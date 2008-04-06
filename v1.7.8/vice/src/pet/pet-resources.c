/*
 * pet-resources.c
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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
#include <string.h>

#include "crtc.h"
#include "pet.h"
#include "petmem.h"
#include "pets.h"
#include "resources.h"
#include "utils.h"

/* Flag: Do we enable the Emulator ID?  */
int emu_id_enabled;

static int set_iosize(resource_value_t v, void *param)
{
    petres.IOSize = (int)v;

    mem_initialize_memory();
    return 0;
}

static int set_crtc_enabled(resource_value_t v, void *param)
{
    petres.crtc = (int)v;
    return 0;
}

static int set_superpet_enabled(resource_value_t v, void *param)
{
    if ((unsigned int) v < 2)
        petres.superpet = (unsigned int)v;
    mem_initialize_memory();
    return 0;
}

static int set_ram_9_enabled(resource_value_t v, void *param)
{
    if ((unsigned int) v < 2)
        petres.mem9 = (unsigned int)v;
    mem_initialize_memory();
    return 0;
}

static int set_ram_a_enabled(resource_value_t v, void *param)
{
    if ((unsigned int) v < 2)
        petres.memA = (unsigned int)v;
    mem_initialize_memory();
    return 0;
}

static int set_ramsize(resource_value_t v, void *param)
{
    int size = (int) v;
    int i, sizes[] = { 4, 8, 16, 32, 96, 128 };

    for (i = 0; i < 6; i++) {
        if (size <= sizes[i])
            break;
    }
    if (i > 5)
        i = 5;
    size = sizes[i];

    petres.ramSize = size;
    petres.map = 0;
    if (size == 96) {
        petres.map = 1;         /* 8096 mapping */
    } else if (size == 128) {
        petres.map = 2;         /* 8296 mapping */
    }
    pet_check_info(&petres);
    mem_initialize_memory();

    return 0;
}

static int set_video(resource_value_t v, void *param)
{
    int col = (int)v;

    if (col != petres.video) {
        if (col == 0 || col == 40 || col == 80) {

            petres.video = col;

            pet_check_info(&petres);

            pet_crtc_set_screen();
        }
    }
    return 0;
}

/* ROM filenames */

static int set_chargen_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.chargenName, (const char *)v))
        return 0;

    return mem_load_chargen();
}

static int set_kernal_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.kernalName, (const char *)v))
        return 0;

    return mem_load_kernal();
}

static int set_basic_rom_name(resource_value_t v, void *param)
{
/*  do we want to reload the basic even with the same name - romB can
    overload the basic ROM image and we can restore it only here ?
*/
    if (util_string_set(&petres.basicName, (const char *)v))
        return 0;

    return mem_load_basic();
}

static int set_editor_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.editorName, (const char *)v))
        return 0;

    return mem_load_editor();
}

static int set_rom_module_9_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.mem9name, (const char *)v))
        return 0;

    return mem_load_rom9();
}

static int set_rom_module_a_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.memAname, (const char *)v))
        return 0;

    return mem_load_romA();
}

static int set_rom_module_b_name(resource_value_t v, void *param)
{
    if (util_string_set(&petres.memBname, (const char *)v))
        return 0;

    return mem_load_romB();
}

/* Enable/disable patching the PET 2001 chargen ROM/kernal ROM */

static int set_pet2k_enabled(resource_value_t v, void *param)
{
    int i = (((int)v) ? 1 : 0);

    if (i != petres.pet2k) {
        if (petres.pet2k)
            petmem_unpatch_2001();

        petres.pet2k = i;

        if (petres.pet2k)
            petmem_patch_2001();
    }
    return 0;
}

static int set_pet2kchar_enabled(resource_value_t v, void *param)
{
    int i = (((int)v) ? 1 : 0);

    if (i != petres.pet2kchar) {
        petres.pet2kchar = i;

        /* function reverses itself -> no reload necessary */
        petmem_convert_chargen_2k();
    }
    return 0;
}

static int set_eoiblank_enabled(resource_value_t v, void *param)
{
    int i = (((int)v) ? 1 : 0);

    petres.eoiblank = i;

    crtc_enable_hw_screen_blank(petres.eoiblank);

    return 0;
}

/* Enable/disable the Emulator ID.  */

static int set_emu_id_enabled(resource_value_t v, void *param)
{
    emu_id_enabled = (int)v;
    return 0;
}


static resource_t resources[] = {
    {"RamSize", RES_INTEGER, (resource_value_t)32,
     (resource_value_t *)&petres.ramSize,
     set_ramsize, NULL },
    {"IOSize", RES_INTEGER, (resource_value_t)0x800,
     (resource_value_t *)&petres.IOSize,
     set_iosize, NULL },
    {"Crtc", RES_INTEGER, (resource_value_t)1,
     (resource_value_t *)&petres.crtc,
     set_crtc_enabled, NULL },
    {"VideoSize", RES_INTEGER, (resource_value_t)1,
     (resource_value_t *)&petres.video,
     set_video, NULL },
    {"Ram9", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&petres.mem9,
     set_ram_9_enabled, NULL },
    {"RamA", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&petres.memA,
     set_ram_a_enabled, NULL },
    {"SuperPET", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&petres.superpet,
     set_superpet_enabled, NULL },
    {"Basic1", RES_INTEGER, (resource_value_t)1,
     (resource_value_t *)&petres.pet2k,
     set_pet2k_enabled, NULL },
    {"Basic1Chars", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&petres.pet2kchar,
     set_pet2kchar_enabled, NULL },
    {"EoiBlank", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&petres.eoiblank,
     set_eoiblank_enabled, NULL },
    {"ChargenName", RES_STRING, (resource_value_t)"chargen",
     (resource_value_t *)&petres.chargenName,
     set_chargen_rom_name, NULL },
    {"KernalName", RES_STRING, (resource_value_t)PET_KERNAL4NAME,
     (resource_value_t *)&petres.kernalName,
     set_kernal_rom_name, NULL },
    {"EditorName", RES_STRING, (resource_value_t)PET_EDITOR4B80NAME,
     (resource_value_t *)&petres.editorName,
     set_editor_rom_name, NULL },
    {"BasicName", RES_STRING, (resource_value_t)PET_BASIC4NAME,
     (resource_value_t *)&petres.basicName,
     set_basic_rom_name, NULL },
    {"RomModule9Name", RES_STRING, (resource_value_t)"",
     (resource_value_t *)&petres.mem9name,
     set_rom_module_9_name, NULL },
    {"RomModuleAName", RES_STRING, (resource_value_t)"",
     (resource_value_t *)&petres.memAname,
     set_rom_module_a_name, NULL },
    {"RomModuleBName", RES_STRING, (resource_value_t)"",
     (resource_value_t *)&petres.memBname,
     set_rom_module_b_name, NULL },
    { "EmuID", RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&emu_id_enabled,
     set_emu_id_enabled, NULL  },
    { NULL }
};

int pet_init_resources(void)
{
    return resources_register(resources);
}

