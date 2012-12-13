/*
 * scpu64meminit.c -- Initialize C64 memory.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64cia.h"
#include "scpu64mem.h"
#include "scpu64meminit.h"
#include "cartio.h"
#include "sid.h"
#include "vicii-mem.h"

/*

 missing: BA,CAS(not needed),RW(through tables),AEC

 bit 5 - boot
 bit 4 - !game
 bit 3 - !exrom
 bit 2 - loram
 bit 1 - hiram
 bit 0 - charen

         8000      a000      d000      e000

 0 0x00
 1 0x01                      chr
 2 0x02                      chr       ker
 3 0x03            bas       chr       ker
 4 0x04
 5 0x05                      io
 6 0x06                      io        ker
 7 0x07            bas       io        ker

 8 0x08
 9 0x09                      chr
10 0x0a            romh      chr       ker
11 0x0b  roml      bas/romh  chr       ker      8k game
12 0x0c
13 0x0d                      io
14 0x0e            romh      io        ker
15 0x0f  roml      bas/romh  io        ker      8k game

16 0x00
17 0x01                      chr
18 0x02                      chr       ker
19 0x03            bas       chr       ker
20 0x04
21 0x05                      io
22 0x06                      io        ker
23 0x07            bas       io        ker

24 0x18
25 0x19                      chr
26 0x1a            romh      chr       ker
27 0x1b  roml      romh      chr       ker      16k game
28 0x1c
29 0x1d                      io
30 0x1e            romh      io        ker
31 0x1f  roml      romh      io        ker      16k game

*/

/* IO is enabled at memory configs 5, 6, 7 and Ultimax.  */
const unsigned int scpu64meminit_io_config[64] = {
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1
};

/* ROML is enabled at memory configs 11, 15, 27, 31  */
static const unsigned int scpu64meminit_roml_config[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* ROMH is enabled at memory configs 10, 11, 14, 15, 26, 27, 30, 31
   and Ultimax.  */
static const unsigned int scpu64meminit_romh_config[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void scpu64meminit(unsigned int base)
{
    unsigned int i, j;

    /* Setup BASIC ROM at $A000-$BFFF (memory configs 3, 7, 11, 15, 19, 23).  */
    for (i = 0xa0; i <= 0xbf; i++) {
        mem_read_tab_set(base + 3, i, scpu64_basic_read);
        mem_read_tab_set(base + 7, i, scpu64_basic_read);
        mem_read_tab_set(base + 11, i, scpu64_basic_read);
        mem_read_tab_set(base + 15, i, scpu64_basic_read);
        mem_read_tab_set(base + 19, i, scpu64_basic_read);
        mem_read_tab_set(base + 23, i, scpu64_basic_read);
        mem_read_base_set(base + 3, i, mem_sram + 0x10000);
        mem_read_base_set(base + 7, i, mem_sram + 0x10000);
        mem_read_base_set(base + 11, i, mem_sram + 0x10000);
        mem_read_base_set(base + 15, i, mem_sram + 0x10000);
        mem_read_base_set(base + 19, i, mem_sram + 0x10000);
        mem_read_base_set(base + 23, i, mem_sram + 0x10000);
    }

    /* Setup I/O at $D000-$DFFF (memory configs 5, 6, 7).  */
    for (j = 0; j < 64; j++) {
        if (scpu64meminit_io_config[j] == 1) {
            mem_read_tab_set(base + j, 0xd0, scpu64io_d000_read);
            mem_set_write_hook(base + j, 0xd0, scpu64io_d000_store);
            mem_read_tab_set(base + j, 0xd1, scpu64io_d100_read);
            mem_set_write_hook(base + j, 0xd1, scpu64io_d100_store);
            mem_read_tab_set(base + j, 0xd2, scpu64io_d200_read);
            mem_set_write_hook(base + j, 0xd2, scpu64io_d200_store);
            mem_read_tab_set(base + j, 0xd3, scpu64io_d300_read);
            mem_set_write_hook(base + j, 0xd3, scpu64io_d300_store);
            mem_read_tab_set(base + j, 0xd4, scpu64io_d400_read);
            mem_set_write_hook(base + j, 0xd4, scpu64io_d400_store);
            mem_read_tab_set(base + j, 0xd5, scpu64io_d500_read);
            mem_set_write_hook(base + j, 0xd5, scpu64io_d500_store);
            mem_read_tab_set(base + j, 0xd6, scpu64io_d600_read);
            mem_set_write_hook(base + j, 0xd6, scpu64io_d600_store);
            mem_read_tab_set(base + j, 0xd7, scpu64io_d700_read);
            mem_set_write_hook(base + j, 0xd7, scpu64io_d700_store);
            for (i = 0xd8; i <= 0xdb; i++) {
                mem_read_tab_set(base + j, i, scpu64io_colorram_read);
                mem_set_write_hook(base + j, i, scpu64io_colorram_store);
            }

            mem_read_tab_set(base + j, 0xdc, scpu64_cia1_read);
            mem_set_write_hook(base + j, 0xdc, scpu64_cia1_store);
            mem_read_tab_set(base + j, 0xdd, scpu64_cia2_read);
            mem_set_write_hook(base + j, 0xdd, scpu64_cia2_store);

            mem_read_tab_set(base + j, 0xde, scpu64io_de00_read);
            mem_set_write_hook(base + j, 0xde, scpu64io_de00_store);
            mem_read_tab_set(base + j, 0xdf, scpu64io_df00_read);
            mem_set_write_hook(base + j, 0xdf, scpu64io_df00_store);

            for (i = 0xd0; i <= 0xdf; i++) {
                mem_read_base_set(base + j, i, NULL);
            }
        }
    }

    /* Setup Kernal ROM at $E000-$FFFF (memory configs 2, 3, 6, 7, 10,
       11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31).  */
    for (i = 0xe0; i <= 0xff; i++) {
        mem_read_tab_set(base + 2, i, scpu64_kernal_read);
        mem_read_tab_set(base + 3, i, scpu64_kernal_read);
        mem_read_tab_set(base + 6, i, scpu64_kernal_read);
        mem_read_tab_set(base + 7, i, scpu64_kernal_read);
        mem_read_tab_set(base + 10, i, scpu64_kernal_read);
        mem_read_tab_set(base + 11, i, scpu64_kernal_read);
        mem_read_tab_set(base + 14, i, scpu64_kernal_read);
        mem_read_tab_set(base + 15, i, scpu64_kernal_read);
        mem_read_tab_set(base + 18, i, scpu64_kernal_read);
        mem_read_tab_set(base + 19, i, scpu64_kernal_read);
        mem_read_tab_set(base + 22, i, scpu64_kernal_read);
        mem_read_tab_set(base + 23, i, scpu64_kernal_read);
        mem_read_tab_set(base + 26, i, scpu64_kernal_read);
        mem_read_tab_set(base + 27, i, scpu64_kernal_read);
        mem_read_tab_set(base + 30, i, scpu64_kernal_read);
        mem_read_tab_set(base + 31, i, scpu64_kernal_read);
        mem_read_base_set(base + 2, i, mem_sram + 0x10000); /* to prevent cartridges only */
        mem_read_base_set(base + 3, i, mem_sram + 0x10000);
        mem_read_base_set(base + 6, i, mem_sram + 0x10000);
        mem_read_base_set(base + 7, i, mem_sram + 0x10000);
        mem_read_base_set(base + 10, i, mem_sram + 0x10000);
        mem_read_base_set(base + 11, i, mem_sram + 0x10000);
        mem_read_base_set(base + 14, i, mem_sram + 0x10000);
        mem_read_base_set(base + 15, i, mem_sram + 0x10000);
        mem_read_base_set(base + 18, i, mem_sram + 0x10000);
        mem_read_base_set(base + 19, i, mem_sram + 0x10000);
        mem_read_base_set(base + 22, i, mem_sram + 0x10000);
        mem_read_base_set(base + 23, i, mem_sram + 0x10000);
        mem_read_base_set(base + 26, i, mem_sram + 0x10000);
        mem_read_base_set(base + 27, i, mem_sram + 0x10000);
        mem_read_base_set(base + 30, i, mem_sram + 0x10000);
        mem_read_base_set(base + 31, i, mem_sram + 0x10000);
    }

    /* Setup ROML at $8000-$9FFF.  */
    for (j = 0; j < 32; j++) {
        if (scpu64meminit_roml_config[j]) {
            for (i = 0x80; i <= 0x9f; i++) {
                mem_read_tab_set(base + j, i, scpu64_roml_read);
                mem_read_base_set(base + j, i, NULL);
                mem_set_write_hook(base + j, i, roml_no_ultimax_store);
            }
        }
    }

    /* Setup write Hook for when ROML is NOT selected at $8000-$9FFF */
    for (j = 0; j < 32; j++) {
        if (!scpu64meminit_roml_config[j]) {
            for (i = 0x80; i <= 0x9f; i++) {
                mem_set_write_hook(base + j, i, raml_no_ultimax_store);
            }
        }
    }
    /* Setup ROMH at $A000-$BFFF */
    for (j = 24; j < 32; j++) {
        if (scpu64meminit_romh_config[j]) {
            for (i = 0xa0; i <= 0xbf; i++) {
                mem_read_tab_set(base + j, i, scpu64_romh_read);
                mem_read_base_set(base + j, i, NULL);
                mem_set_write_hook(base + j, i, romh_no_ultimax_store);
            }
        }
    }

    /* Setup bootmap */
    for (j = 32; j < 64; j++) {
        for (i = 0x80; i <= 0xcf; i++) {
            mem_read_tab_set(base + j, i, scpu64memrom_scpu64_read);
            mem_read_base_set(base + j, i, scpu64memrom_scpu64_rom);
        }
        for (i = 0xe0; i <= 0xff; i++) {
            mem_read_tab_set(base + j, i, scpu64memrom_scpu64_read);
            mem_read_base_set(base + j, i, scpu64memrom_scpu64_rom);
        }
    }
    /* Setup bootmap ROM at $D000-$DFFF */
    for (i = 0xd0; i <= 0xdf; i++) {
        mem_read_tab_set(base + 32, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 32, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 36, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 36, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 40, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 40, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 44, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 44, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 48, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 48, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 52, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 52, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 56, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 56, i, scpu64memrom_scpu64_rom);
        mem_read_tab_set(base + 60, i, scpu64memrom_scpu64_read);
        mem_read_base_set(base + 60, i, scpu64memrom_scpu64_rom);
    }
}
