/*
 * vicii-fetch.c - Phi1/Phi2 data fetch for the VIC-II emulation.
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

#include <string.h>

#include "c64cart.h"
#include "debug.h"
#include "dma.h"
#include "log.h"
#include "mainc64cpu.h"
#include "mem.h"
#include "types.h"
#include "vicii-fetch.h"
#include "vicii-irq.h"
#include "viciitypes.h"


/*-----------------------------------------------------------------------*/

inline static BYTE fetch_phi1(int addr)
{
    BYTE *p;

    addr = ((addr + vicii.vbank_phi1) & vicii.vaddr_mask_phi1) | vicii.vaddr_offset_phi1;

    if ((cart_ultimax_phi1 != 0) && ((addr & 0x3fff) >= 0x3000)) {
        p = romh_banks + (romh_bank << 13) + (addr & 0xfff) + 0x1000;
    } else if ((addr & vicii.vaddr_chargen_mask_phi1) == vicii.vaddr_chargen_value_phi1) {
        p = mem_chargen_rom_ptr + (addr & 0xc00);
    } else {
        p = vicii.ram_base_phi1 + addr;
    }

    return *p;
}

inline static BYTE fetch_phi2(int addr)
{
    BYTE *p;

    addr = ((addr + vicii.vbank_phi2) & vicii.vaddr_mask_phi2) | vicii.vaddr_offset_phi2;

    if ((cart_ultimax_phi2 != 0) && ((addr & 0x3fff) >= 0x3000)) {
        p = romh_banks + (romh_bank << 13) + (addr & 0xfff) + 0x1000;
    } else if ((addr & vicii.vaddr_chargen_mask_phi2) == vicii.vaddr_chargen_value_phi2) {
        p = mem_chargen_rom_ptr + (addr & 0xc00);
    } else {
        p = vicii.ram_base_phi2 + addr;
    }

    return *p;
}

inline static int check_sprite_dma(int i)
{
    return vicii.sprite[i].dma;
}

inline static int sprite_dma_cycle_0(int i)
{
    BYTE sprdata = vicii.last_bus_phi2;
    int ba_low = 0;

    if (check_sprite_dma(i)) {
        sprdata = fetch_phi2((vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc);

        vicii.sprite[i].mc++;
        vicii.sprite[i].mc &= 0x3f;

#ifdef DEBUG
        if (debug.maincpu_traceflg && vicii.sprite[i].dma) {
            log_debug("SDMA0 in cycle %i", vicii.raster_cycle);
        }
#endif
        ba_low = 1;
    }

    vicii.sprite[i].data &= 0x00ffff;
    vicii.sprite[i].data |= sprdata << 16;

    return ba_low;
}

inline static int sprite_dma_cycle_2(int i)
{
    BYTE sprdata = vicii.last_bus_phi2;
    int ba_low = 0;

    if (check_sprite_dma(i)) {
        sprdata = fetch_phi2((vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc);

        vicii.sprite[i].mc++;
        vicii.sprite[i].mc &= 0x3f;

#ifdef DEBUG
        if (debug.maincpu_traceflg && vicii.sprite[i].dma) {
            log_debug("SDMA2 in cycle %i", vicii.raster_cycle);
        }
#endif
        ba_low = 1;
    }

    vicii.sprite[i].data &= 0xffff00;
    vicii.sprite[i].data |= sprdata;

    return ba_low;
}

/*-----------------------------------------------------------------------*/

inline static BYTE gfx_data_illegal_bitmap(unsigned int num)
{
    unsigned int j;

    j = ((vicii.vcbase << 3) + vicii.rc + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0x9ff];
    } else {
        return vicii.bitmap_low_ptr[j & 0x9ff];
    }
}

inline static BYTE gfx_data_hires_bitmap(unsigned int num)
{
    unsigned int j;

    j = ((vicii.vcbase << 3) + vicii.rc + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0xfff];
    } else {
        return vicii.bitmap_low_ptr[j & 0xfff];
    }
}

inline static BYTE gfx_data_extended_text(unsigned int c)
{
    return vicii.chargen_ptr[(c & 0x3f) * 8 + vicii.rc];
}

inline static BYTE gfx_data_normal_text(unsigned int c)
{
    return vicii.chargen_ptr[c * 8 + vicii.rc];
}

/*-----------------------------------------------------------------------*/

void vicii_fetch_matrix(void)
{
    if (vicii.prefetch_cycles) {
        vicii.vbuf[vicii.vmli] = 0xff;
        vicii.cbuf[vicii.vmli] = vicii.ram_base_phi2[reg_pc] & 0xf;
    } else {
        vicii.vbuf[vicii.vmli] = vicii.screen_base_phi2[vicii.vc];
        vicii.cbuf[vicii.vmli] = mem_color_ram_vicii[vicii.vc];
    }
}

BYTE vicii_fetch_refresh(void)
{
    return fetch_phi1(0x3f00 + vicii.refresh_counter--);
}

BYTE vicii_fetch_idle(void)
{
    return fetch_phi1(0x3fff);
}

BYTE vicii_fetch_idle_gfx(void)
{
    BYTE data;

    if (vicii.regs[0x11] & 0x40) {
        data = fetch_phi1(0x39ff);
    } else {
        data = fetch_phi1(0x3fff);
    }
    vicii.gbuf = data;

    return data;
}

BYTE vicii_fetch_graphics(void)
{
    BYTE data;

    switch (vicii.regs[0x11] & 0x60) {
    case 0x00:                             /* ECM=0 BMM=0 MCM=x */
        data = gfx_data_normal_text(vicii.vbuf[vicii.vmli]);
        break;
    case 0x20:                             /* ECM=0 BMM=1 MCM=x */
        data = gfx_data_hires_bitmap(vicii.vmli);
        break;
    case 0x40:                             /* ECM=1 BMM=0 MCM=x */
        data = gfx_data_extended_text(vicii.vbuf[vicii.vmli]);
        break;
    case 0x60:                             /* ECM=1 BMM=1 MCM=x */
        data = gfx_data_illegal_bitmap(vicii.vmli);
        break;
    default:
        data = 0xff;
        break;
    }
    vicii.gbuf = data;
    vicii.vmli++;

    vicii.vc++;
    vicii.vc &= 0x3ff;

    return data;
}

BYTE vicii_fetch_sprite_pointer(int i)
{
    vicii.sprite[i].pointer = vicii.screen_base_phi1[0x3f8 + i];

    return vicii.sprite[i].pointer;
}

BYTE vicii_fetch_sprite_dma_1(int i)
{
    BYTE sprdata;

    if (check_sprite_dma(i)) {
        sprdata = fetch_phi1((vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc);

        vicii.sprite[i].mc++;
        vicii.sprite[i].mc &= 0x3f;
    } else {
        sprdata = vicii_fetch_idle();
    }

    vicii.sprite[i].data &= 0xff00ff;
    vicii.sprite[i].data |= sprdata << 8;

    return sprdata;
}

int vicii_fetch_sprites(int cycle)
{
    int ba_low = 0;

    switch (cycle) {
    case VICII_PAL_CYCLE(55):
        ba_low = check_sprite_dma(0);
        break;
    case VICII_PAL_CYCLE(56):
        ba_low = check_sprite_dma(0);
        break;
    case VICII_PAL_CYCLE(57):
        ba_low = check_sprite_dma(0) | check_sprite_dma(1);
        break;
    case VICII_PAL_CYCLE(58):
        ba_low = sprite_dma_cycle_0(0) | check_sprite_dma(1);
        break;
    case VICII_PAL_CYCLE(59):
        ba_low = sprite_dma_cycle_2(0) | check_sprite_dma(1) | check_sprite_dma(2);
        break;
    case VICII_PAL_CYCLE(60):
        ba_low = sprite_dma_cycle_0(1) | check_sprite_dma(2);
        break;
    case VICII_PAL_CYCLE(61):
        ba_low = sprite_dma_cycle_2(1) | check_sprite_dma(2) | check_sprite_dma(3);
        break;
    case VICII_PAL_CYCLE(62):
        ba_low = sprite_dma_cycle_0(2) | check_sprite_dma(3);
        break;
    case VICII_PAL_CYCLE(63):
        ba_low = sprite_dma_cycle_2(2) | check_sprite_dma(3) | check_sprite_dma(4);
        break;
    case VICII_PAL_CYCLE(1):
        ba_low = sprite_dma_cycle_0(3) | check_sprite_dma(4);
        break;
    case VICII_PAL_CYCLE(2):
        ba_low = sprite_dma_cycle_2(3) | check_sprite_dma(4) | check_sprite_dma(5);
        break;
    case VICII_PAL_CYCLE(3):
        ba_low = sprite_dma_cycle_0(4) | check_sprite_dma(5);
        break;
    case VICII_PAL_CYCLE(4):
        ba_low = sprite_dma_cycle_2(4) | check_sprite_dma(5) | check_sprite_dma(6);
        break;
    case VICII_PAL_CYCLE(5):
        ba_low = sprite_dma_cycle_0(5) | check_sprite_dma(6);
        break;
    case VICII_PAL_CYCLE(6):
        ba_low = sprite_dma_cycle_2(5) | check_sprite_dma(6) | check_sprite_dma(7);
        break;
    case VICII_PAL_CYCLE(7):
        ba_low = sprite_dma_cycle_0(6) | check_sprite_dma(7);
        break;
    case VICII_PAL_CYCLE(8):
        ba_low = sprite_dma_cycle_2(6) | check_sprite_dma(7);
        break;
    case VICII_PAL_CYCLE(9):
        ba_low = sprite_dma_cycle_0(7);
        break;
    case VICII_PAL_CYCLE(10):
        ba_low = sprite_dma_cycle_2(7);
        break;
    default:
        break;
    }

    return ba_low;
}
