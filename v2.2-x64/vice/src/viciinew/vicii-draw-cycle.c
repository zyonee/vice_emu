/*
 * vicii-draw-cycle.c - Cycle based rendering for the VIC-II emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
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

#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "raster.h"
#include "types.h"
#include "vicii-draw-cycle.h"
#include "viciitypes.h"

/* foreground/background graphics */

/* gbuf shift register */
BYTE gbuf_reg = 0;
int gbuf_cnt = 0;
BYTE gbuf_mc_reg = 0;

/* cbuf and vbuf registers */
BYTE cbuf_reg = 0;
BYTE vbuf_reg = 0;

/* sprites */

/* sbuf shift registers */
int sbuf_cnt[8];
DWORD sbuf_reg[8]; /* maybe use those directly present in vicii.*? */
BYTE sbuf_mc_reg[8];
BYTE sbuf_exp_reg[8];


void vicii_draw_cycle(void)
{
    int cycle, offs, i;

    cycle = vicii.raster_cycle;

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
        gbuf_cnt = 0;
        gbuf_reg = 0;
        vbuf_reg = 0;
        cbuf_reg = 0;
    }
    offs = vicii.dbuf_offset;
    /* guard */
    if (offs >= VICII_DRAW_BUFFER_SIZE) 
        return;
    
    /* are we within the display area? */
    if (cycle >= 14 && cycle <= 53) {
        BYTE bg, xs;
        bg = vicii.regs[0x21];
        xs = vicii.regs[0x16] & 0x07;

        /* render pixels */
        for (i = 0; i < 8; i++) {
            int j = i + offs;
            BYTE c0, c1, c2, c3;

            if (i == xs) {
                /* latch values at time xs */
                vbuf_reg = vicii.vbuf[cycle - 14];
                cbuf_reg = vicii.cbuf[cycle - 14];
                gbuf_reg = vicii.gbuf[cycle - 14]; /* this shouldn't be a buffer */
                gbuf_cnt = 0;
            }

           
            switch (vicii.video_mode) {

            case VICII_NORMAL_TEXT_MODE:
                c1 = cbuf_reg;
                vicii.dbuf[j] = (gbuf_reg & 0x80) ? c1 : bg;
                break;
            case VICII_MULTICOLOR_TEXT_MODE:
                c1 = vicii.ext_background_color[0];
                c2 = vicii.ext_background_color[1];
                c3 = cbuf_reg & 0x07;
                if (cbuf_reg & 0x08) {
                    if ( (gbuf_cnt & 1) == 0) {
                        gbuf_mc_reg = gbuf_reg >> 6;
                    }
                    switch (gbuf_mc_reg) {
                    case 0:
                        vicii.dbuf[j] = bg;
                        break;
                    case 1:
                        vicii.dbuf[j] = c1;
                        break;
                    case 2:
                        vicii.dbuf[j] = c2;
                        break;
                    case 3:
                        vicii.dbuf[j] = c3;
                        break;
                    }
                } else {
                    vicii.dbuf[j] = (gbuf_reg & 0x80) ? c3 : bg;
                }
                break;

            case VICII_HIRES_BITMAP_MODE:
                c0 = vbuf_reg & 0x0f;
                c1 = vbuf_reg >> 4;
                vicii.dbuf[j] = (gbuf_reg & 0x80) ? c1 : c0;
                break;

            case VICII_MULTICOLOR_BITMAP_MODE:
                c1 = vbuf_reg >> 4;
                c2 = vbuf_reg & 0x0f;
                c3 = cbuf_reg;
                if ( (gbuf_cnt & 1) == 0) {
                    gbuf_mc_reg = gbuf_reg >> 6;
                }
                switch (gbuf_mc_reg) {
                case 0:
                    vicii.dbuf[j] = bg;
                    break;
                case 1:
                    vicii.dbuf[j] = c1;
                    break;
                case 2:
                    vicii.dbuf[j] = c2;
                    break;
                case 3:
                    vicii.dbuf[j] = c3;
                    break;
                }
                break;
            
            case VICII_EXTENDED_TEXT_MODE:
                c1 = cbuf_reg;

                switch (vbuf_reg & 0xc0) {
                case 0x00:
                    c0 = bg;
                    break;
                case 0x40:
                    c0 = vicii.ext_background_color[0];
                    break;
                case 0x80:
                    c0 = vicii.ext_background_color[1];
                    break;
                case 0xc0:
                    c0 = vicii.ext_background_color[2];
                    break;
                }
                vicii.dbuf[j] = (gbuf_reg & 0x80) ? c1 : c0;
                break;

            case VICII_IDLE_MODE:
                /* this currently doesn't work as expected */
                vicii.dbuf[j] = (gbuf_reg & 0x80) ? 0 : bg;
                break;

            }

            /* Brute force render sprites on top of graphics for now! */
            int s;
            for (s = 0; s < 8; s++) {
                /* fetch sprite data on position match */
                int x = cycle*8 + i - 88;
                int sprx = vicii.regs[0x00 + s*2];
                sprx |= (vicii.regs[0x10] & (1<<s)) ? 0x100 : 0;
                BYTE c1 = vicii.regs[0x27 + s];
                if (x == sprx) {
                    DWORD *tmp;
                    raster_sprite_status_t *sprite_status = vicii.raster.sprite_status;
                    DWORD *data = sprite_status->new_sprite_data;
                    sbuf_reg[s] = ((data[s] >> 16) & 0x0000ff) | (data[s] & 0x00ff00) | ((data[s] << 16) & 0xff0000);
                }
                if (sbuf_reg[s]) {

                    if (sbuf_reg[s] & 0x800000) {
                        vicii.dbuf[j] = c1;
                    }

                    /* shift the sprite buffer */
                    sbuf_reg[s] <<= 1;
                    sbuf_cnt[s]++;
                }
            }

            /* shift the graphics buffer */
            gbuf_reg <<= 1;
            gbuf_cnt++;
        }
    } else {
#if 0
        /* we are outside the display area */
        BYTE c = vicii.regs[0x20];
        /* separate function? */
        draw_border_byte(c);
#endif
    }

    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

