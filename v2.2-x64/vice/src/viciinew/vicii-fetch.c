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
#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "raster.h"
#include "types.h"
#include "vicii-fetch.h"
#include "vicii-irq.h"
#include "vicii-sprites.h"
#include "viciitypes.h"


/*-----------------------------------------------------------------------*/

inline static void swap_sprite_data_buffers(void)
{
    DWORD *tmp;
    raster_sprite_status_t *sprite_status;

    /* Swap sprite data buffers.  */
    sprite_status = vicii.raster.sprite_status;
    tmp = sprite_status->sprite_data;
    sprite_status->sprite_data = sprite_status->new_sprite_data;
    sprite_status->new_sprite_data = tmp;
    sprite_status->new_dma_msk = sprite_status->dma_msk;
}

/* Enable DMA for sprite `num'.  */
inline static void turn_sprite_dma_on(unsigned int num)
{
    raster_sprite_status_t *sprite_status;
    raster_sprite_t *sprite;

    sprite_status = vicii.raster.sprite_status;
    sprite = sprite_status->sprites + num;

    sprite_status->new_dma_msk |= 1 << num;
    sprite->dma_flag = 1;
    sprite->memptr = 0;
    sprite->exp_flag = sprite->y_expanded ? 0 : 1;
    sprite->memptr_inc = sprite->exp_flag ? 3 : 0;
}

inline static int check_sprite_dma(int i)
{
    return vicii.raster.sprite_status->sprites[i].dma_flag;
}

static BYTE *cached_sprite_dest = NULL;
static BYTE *cached_src_phi1 = NULL;
static BYTE *cached_src_phi2 = NULL;
static int cached_sprite_memptr = 0;

inline static int sprite_dma_cycle_0(void)
{
    if (check_sprite_dma(vicii.sprite_fetch_idx)) {
        cached_sprite_dest[0] = cached_src_phi1[cached_sprite_memptr];
        return 1;
    }
    return 0;
}

inline static int sprite_dma_cycle_2(void)
{
    int fetched = 0;

    if (check_sprite_dma(vicii.sprite_fetch_idx)) {
        cached_sprite_dest[2] = cached_src_phi1[++cached_sprite_memptr & 0x3f];
        fetched = 1;
    }
    vicii.sprite_fetch_idx++;

    return fetched;
}

inline static int trigger_sprite_dma(int i)
{
    raster_sprite_status_t *sprite_status;
    raster_sprite_t *sprite;
    const int b = (1 << i);

    sprite_status = vicii.raster.sprite_status;
    sprite = sprite_status->sprites + i;

    if (!sprite_status->visible_msk && !sprite_status->dma_msk) {
        return 0;
    }

    if ((sprite_status->visible_msk & b)
        && sprite->y == ((int)((vicii.raster_line - ((i<4)?0:1)) & 0xff))
        && !sprite->dma_flag) {
        turn_sprite_dma_on(i);
    } else if (sprite->dma_flag) {
        sprite->memptr = (sprite->memptr + sprite->memptr_inc) & 0x3f;

        if (sprite->y_expanded) {
            sprite->exp_flag = !sprite->exp_flag;
        }

        sprite->memptr_inc = sprite->exp_flag ? 3 : 0;

        if (sprite->memptr == 63) {
            sprite->dma_flag = 0;
            sprite_status->new_dma_msk &= ~b;

            if ((sprite_status->visible_msk & b)
                && sprite->y == ((int)vicii.raster.current_line & 0xff)) {
                turn_sprite_dma_on(i);
            }
        }
    }

    return sprite->dma_flag;
}

/*-----------------------------------------------------------------------*/

inline static BYTE gfx_data_illegal_bitmap(unsigned int num)
{
    unsigned int j;

    j = ((vicii.memptr << 3) + vicii.raster.ycounter + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0x9ff];
    } else {
        return vicii.bitmap_low_ptr[j & 0x9ff];
    }
}

inline static BYTE gfx_data_hires_bitmap(unsigned int num)
{
    unsigned int j;

    j = ((vicii.memptr << 3) + vicii.raster.ycounter + num * 8);

    if (j & 0x1000) {
        return vicii.bitmap_high_ptr[j & 0xfff];
    } else {
        return vicii.bitmap_low_ptr[j & 0xfff];
    }
}

inline static BYTE gfx_data_extended_text(unsigned int c)
{
    return vicii.chargen_ptr[(c & 0x3f) * 8 + vicii.raster.ycounter];
}

inline static BYTE gfx_data_normal_text(unsigned int c)
{
    return vicii.chargen_ptr[c * 8 + vicii.raster.ycounter];
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
    vicii.gbuf[vicii.gbuf_offset++] = data;
    vicii.buf_offset++;

    return data;
}

BYTE vicii_fetch_graphics(void)
{
    BYTE data;

    switch (vicii.raster.video_mode) {
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
    vicii.gbuf[vicii.gbuf_offset++] = data;
    vicii.buf_offset++;

    vicii.mem_counter++;
    vicii.mem_counter &= 0x3ff;

    return data;
}

BYTE vicii_fetch_sprite_pointer(void)
{
    raster_sprite_status_t *sprite_status;
    BYTE *bank_phi1, *bank_phi2, *spr_base;
    int i;

    i = vicii.sprite_fetch_idx;
    sprite_status = vicii.raster.sprite_status;
    bank_phi1 = vicii.ram_base_phi1 + vicii.vbank_phi1;
    bank_phi2 = vicii.ram_base_phi2 + vicii.vbank_phi2;
    spr_base = vicii.screen_base_phi1 + 0x3f8 + i;

    /* Set up pointers for actual fetch.  */
    if (check_sprite_dma(i)) {
        cached_src_phi1 = bank_phi1 + (*spr_base << 6);
        cached_src_phi2 = bank_phi2 + (*spr_base << 6);
        cached_sprite_memptr = sprite_status->sprites[i].memptr;
        cached_sprite_dest = (BYTE *)(sprite_status->new_sprite_data + i);

        if (((vicii.vbank_phi1 + (*spr_base << 6))
            & vicii.vaddr_chargen_mask_phi1)
            == vicii.vaddr_chargen_value_phi1) {
            cached_src_phi1 = mem_chargen_rom_ptr + ((*spr_base & 0x3f) << 6);
        }

        if (((vicii.vbank_phi2 + (*spr_base << 6))
            & vicii.vaddr_chargen_mask_phi2)
            == vicii.vaddr_chargen_value_phi2) {
            cached_src_phi2 = mem_chargen_rom_ptr + ((*spr_base & 0x3f) << 6);
        }
    }

    return *spr_base;
}

BYTE vicii_fetch_sprite_dma_1(void)
{
    if (check_sprite_dma(vicii.sprite_fetch_idx)) {
        return (cached_sprite_dest[1] = cached_src_phi1[++cached_sprite_memptr & 0x3f]);
    } else {
        return vicii_fetch_idle();
    }
}

int vicii_fetch_sprites(int cycle)
{
    int ba_low = 0;

    switch (cycle) {
        case 56: /* sprite 0 trigger */
            swap_sprite_data_buffers();
            vicii_sprites_reset_sprline();
            ba_low = trigger_sprite_dma(0);
            vicii.sprite_fetch_idx = 0;
            break;
        case 57:
            ba_low = check_sprite_dma(0);
            break;
        case 58: /* sprite 1 trigger */
            ba_low = trigger_sprite_dma(1) | check_sprite_dma(0);
            break;
        case 59: /* sprite 0 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(1);
            break;
        case 60: /* sprite 0 data / 2 trigger */
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(2) | check_sprite_dma(1);
            break;
        case 61: /* sprite 1 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(2);
            break;
        case 62: /* sprite 1 data / 3 trigger */
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(3) | check_sprite_dma(2);
            break;
        case 63: /* sprite 2 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(3);
            break;
        case 64: /* sprite 2 data / 4 trigger */
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(4) | check_sprite_dma(3);
            break;
        case 0: /* sprite 3 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(4);
            break;
        case 1: /* sprite 3 data / 5 trigger*/
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(5) | check_sprite_dma(4);
            break;
        case 2: /* sprite 4 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(5);
            break;
        case 3: /* sprite 4 data / 6 trigger */
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(6) | check_sprite_dma(5);
            break;
        case 4: /* sprite 5 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(6);
            break;
        case 5: /* sprite 5 data / 7 trigger */
            ba_low = sprite_dma_cycle_2() | trigger_sprite_dma(7) | check_sprite_dma(6);
            break;
        case 6: /* sprite 6 pointer */
            ba_low = sprite_dma_cycle_0() | check_sprite_dma(7);
            break;
        case 7: /* sprite 6 data */
            ba_low = sprite_dma_cycle_2() | check_sprite_dma(7);
            break;
        case 8: /* sprite 7 pointer */
            ba_low = sprite_dma_cycle_0();
            break;
        case 9: /* sprite 7 data */
            ba_low = sprite_dma_cycle_2();
            break;
        default:
            break;
    }

    return ba_low;
}
