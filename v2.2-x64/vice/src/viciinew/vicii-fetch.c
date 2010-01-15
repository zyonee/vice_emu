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

inline static int check_sprite_dma(int i)
{
    return vicii.sprite[i].dma;
}

inline static int sprite_dma_cycle_0(int i)
{
    BYTE sprdata = 0xff;
    int ba_low = 0;

    if (check_sprite_dma(i)) {
        sprdata = vicii.ram_base_phi2[vicii.vbank_phi2 + (vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc];

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
    BYTE sprdata = 0xff;
    int ba_low = 0;

    if (check_sprite_dma(i)) {
        sprdata = vicii.ram_base_phi2[vicii.vbank_phi2 + (vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc];

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

    j = ((vicii.memptr << 3) + vicii.ycounter + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0x9ff];
    } else {
        return vicii.bitmap_low_ptr[j & 0x9ff];
    }
}

inline static BYTE gfx_data_hires_bitmap(unsigned int num)
{
    unsigned int j;

    j = ((vicii.memptr << 3) + vicii.ycounter + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0xfff];
    } else {
        return vicii.bitmap_low_ptr[j & 0xfff];
    }
}

inline static BYTE gfx_data_extended_text(unsigned int c)
{
    return vicii.chargen_ptr[(c & 0x3f) * 8 + vicii.ycounter];
}

inline static BYTE gfx_data_normal_text(unsigned int c)
{
    return vicii.chargen_ptr[c * 8 + vicii.ycounter];
}

/*-----------------------------------------------------------------------*/

void vicii_fetch_matrix(void)
{
    if (vicii.prefetch_cycles) {
        vicii.vbuf[vicii.buf_offset] = 0xff;
        vicii.cbuf[vicii.buf_offset] = vicii.ram_base_phi2[reg_pc] & 0xf;
    } else {
        vicii.vbuf[vicii.buf_offset] = vicii.screen_base_phi2[vicii.mem_counter];
        vicii.cbuf[vicii.buf_offset] = mem_color_ram_vicii[vicii.mem_counter];
    }
}

BYTE vicii_fetch_refresh(void)
{
    return vicii.ram_base_phi1[vicii.vbank_phi1 + 0x3f00 + vicii.refresh_counter--];
}

BYTE vicii_fetch_idle(void)
{
    return vicii.ram_base_phi1[vicii.vbank_phi1 + 0x3fff];
}

BYTE vicii_fetch_idle_gfx(void)
{
    BYTE data;

    if (vicii.regs[0x11] & 0x40) {
        data = vicii.ram_base_phi1[vicii.vbank_phi1 + 0x39ff];
    } else {
        data = vicii.ram_base_phi1[vicii.vbank_phi1 + 0x3fff];
    }
    vicii.gbuf = data;
    vicii.buf_offset++;

    return data;
}

BYTE vicii_fetch_graphics(void)
{
    BYTE data;

    switch (vicii.video_mode) {
        case VICII_NORMAL_TEXT_MODE:
        case VICII_MULTICOLOR_TEXT_MODE:
            data = gfx_data_normal_text(vicii.vbuf[vicii.buf_offset]);
            break;
        case VICII_HIRES_BITMAP_MODE:
        case VICII_MULTICOLOR_BITMAP_MODE:
            data = gfx_data_hires_bitmap(vicii.buf_offset);
            break;
        case VICII_EXTENDED_TEXT_MODE:
        case VICII_ILLEGAL_TEXT_MODE:
            data = gfx_data_extended_text(vicii.vbuf[vicii.buf_offset]);
            break;
        case VICII_ILLEGAL_BITMAP_MODE_1:
        case VICII_ILLEGAL_BITMAP_MODE_2:
            data = gfx_data_illegal_bitmap(vicii.buf_offset);
            break;
        default:
            data = 0xff;
            break;
    }
    vicii.gbuf = data;
    vicii.buf_offset++;

    vicii.mem_counter++;
    vicii.mem_counter &= 0x3ff;

    return data;
}

BYTE vicii_fetch_sprite_pointer(int i)
{
    BYTE *spr_base = vicii.screen_base_phi1 + 0x3f8 + i;
    vicii.sprite[i].pointer = *spr_base;
    return *spr_base;
}

BYTE vicii_fetch_sprite_dma_1(int i)
{
    BYTE sprdata;

    if (check_sprite_dma(i)) {
        sprdata = vicii.ram_base_phi1[vicii.vbank_phi1 + (vicii.sprite[i].pointer << 6) + vicii.sprite[i].mc];

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
        case 56:
            ba_low = check_sprite_dma(0);
            break;
        case 57:
            ba_low = check_sprite_dma(0);
            break;
        case 58:
            ba_low = check_sprite_dma(0) | check_sprite_dma(1);
            break;
        case 59:
            ba_low = sprite_dma_cycle_0(0) | check_sprite_dma(1);
            break;
        case 60:
            ba_low = sprite_dma_cycle_2(0) | check_sprite_dma(1) | check_sprite_dma(2);
            break;
        case 61:
            ba_low = sprite_dma_cycle_0(1) | check_sprite_dma(2);
            break;
        case 62:
            ba_low = sprite_dma_cycle_2(1) | check_sprite_dma(2) | check_sprite_dma(3);
            break;
        case 63:
            ba_low = sprite_dma_cycle_0(2) | check_sprite_dma(3);
            break;
        case 64:
            ba_low = sprite_dma_cycle_2(2) | check_sprite_dma(3) | check_sprite_dma(4);
            break;
        case 0:
            ba_low = sprite_dma_cycle_0(3) | check_sprite_dma(4);
            break;
        case 1:
            ba_low = sprite_dma_cycle_2(3) | check_sprite_dma(4) | check_sprite_dma(5);
            break;
        case 2:
            ba_low = sprite_dma_cycle_0(4) | check_sprite_dma(5);
            break;
        case 3:
            ba_low = sprite_dma_cycle_2(4) | check_sprite_dma(5) | check_sprite_dma(6);
            break;
        case 4:
            ba_low = sprite_dma_cycle_0(5) | check_sprite_dma(6);
            break;
        case 5:
            ba_low = sprite_dma_cycle_2(5) | check_sprite_dma(6) | check_sprite_dma(7);
            break;
        case 6:
            ba_low = sprite_dma_cycle_0(6) | check_sprite_dma(7);
            break;
        case 7:
            ba_low = sprite_dma_cycle_2(6) | check_sprite_dma(7);
            break;
        case 8:
            ba_low = sprite_dma_cycle_0(7);
            break;
        case 9:
            ba_low = sprite_dma_cycle_2(7);
            break;
        default:
            break;
    }

    return ba_low;
}
