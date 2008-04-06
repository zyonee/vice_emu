/*
 * petmem.h - PET memory handling.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef _PETMEM_H
#define _PETMEM_H

#include <stdio.h>

#include "pets.h"
#include "types.h"

#define PET_RAM_SIZE		0x8000
#define PET_ROM_SIZE		0x8000
#define PET_CHARGEN_ROM_SIZE	0x2000

#define	PET_KERNAL1_CHECKSUM	3236
#define	PET_KERNAL2_CHECKSUM	31896
#define	PET_KERNAL4_CHECKSUM	53017

#define	PET_EDIT1G_CHECKSUM	51858
#define	PET_EDIT2G_CHECKSUM	64959
#define	PET_EDIT2B_CHECKSUM	1514
#define	PET_EDIT4G40_CHECKSUM	14162
#define	PET_EDIT4B40_CHECKSUM	27250
#define	PET_EDIT4B80_CHECKSUM	21166

extern int pet_mem_init_resources(void);
extern int pet_mem_init_cmdline_options(void);

extern void mem_initialize_memory(void);
extern void petmem_patch_2001(void);
extern void petmem_unpatch_2001(void);
extern void pet_check_info(PetRes *pi);
extern void petmem_convert_chargen_2k(void);

extern void petmem_reset(void);
extern int superpet_diag(void);

extern int petmem_dump(FILE *fp);
extern int petmem_undump(FILE *fp);

extern void set_screen(void);

extern int mem_load_chargen(void);
extern int mem_load_kernal(void);
extern int mem_load_basic(void);
extern int mem_load_editor(void);
extern int mem_load_rom9(void);
extern int mem_load_romA(void);
extern int mem_load_romB(void);

#endif

