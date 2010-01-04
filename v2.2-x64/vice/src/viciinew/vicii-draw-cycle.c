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

#include "types.h"
#include "vicii-draw-cycle.h"
#include "viciitypes.h"

/* graphics shift register */
int gfx_cnt = 0;
BYTE gfx_shiftreg = 0;

/* cbuf and vbuf registers */
BYTE cbuf_reg = 0;
BYTE vbuf_reg = 0;



#if 0
static void draw_foreground_mc_byte(BYTE b, BYTE c0, BYTE c1, BYTE c2, BYTE c3)
{
    int i;
    int xs = vicii.regs[0x16] & 0x07;
    for (i = 0; i < 8; i += 2) {
        switch (b & 0xc0) {
        case 0x00:
            push_gfx_buffer(c0, i+0+xs);
            push_gfx_buffer(c0, i+1+xs);
            break;
        case 0x40:
            push_gfx_buffer(c1, i+0+xs);
            push_gfx_buffer(c1, i+1+xs);
            break;
        case 0x80:
            push_gfx_buffer(c2, i+0+xs);
            push_gfx_buffer(c2, i+1+xs);
            break;
        case 0xc0:
            push_gfx_buffer(c3, i+0+xs);
            push_gfx_buffer(c3, i+1+xs);
            break;
        }

        b <<= 2;
    }
}
#endif

void vicii_draw_cycle(void)
{
    int cycle, offs, i;

    cycle = vicii.raster_cycle;

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
        gfx_cnt = 0;
        gfx_shiftreg = 0;
        vbuf_reg = 0;
        cbuf_reg = 0;
    }
    offs = vicii.dbuf_offset;
    /* guard */
    if (offs >= VICII_DRAW_BUFFER_SIZE) 
        return;
    
    /* are we within the display area? */
    if (cycle >= 14 && cycle <= 53) {
        BYTE bg, c1, c2, c3;
        BYTE xs;
        bg = vicii.regs[0x21];
        xs = vicii.regs[0x16] & 0x07;

        /* render pixels */
        for (i = 0; i < 8; i++) {
            int j = i + offs;

            if (i == xs) {
                /* latch values at time xs */
                vbuf_reg = vicii.vbuf[cycle - 14];
                cbuf_reg = vicii.cbuf[cycle - 14];
                gfx_shiftreg = vicii.gbuf[cycle - 14]; /* this shouldn't be a buffer */
                gfx_cnt = 0;
            }

           
            switch (vicii.video_mode) {

            case VICII_NORMAL_TEXT_MODE:
                c1 = cbuf_reg;
                vicii.dbuf[j] = (gfx_shiftreg & 0x80) ? c1 : bg;
                break;
#if 0
            case VICII_MULTICOLOR_TEXT_MODE:
                c1 = vicii.ext_background_color[0];
                c2 = vicii.ext_background_color[1];
                c3 = cbuf & 0x07;
                if (cbuf & 0x08) {
                    draw_foreground_mc_byte(gbuf, bg, c1, c2, c3);
                } else {
                    draw_foreground_hires_byte(gbuf, bg, c3);
                }
                break;

            case VICII_HIRES_BITMAP_MODE:
                c1 = vbuf & 0x0f;
                c2 = vbuf >> 4;
                draw_foreground_hires_byte(gbuf, c1, c2);
                break;

            case VICII_MULTICOLOR_BITMAP_MODE:
                c1 = vbuf >> 4;
                c2 = vbuf & 0x0f;
                c3 = cbuf;
                draw_foreground_mc_byte(gbuf, bg, c1, c2, c3);
                break;
            
            case VICII_EXTENDED_TEXT_MODE:
                c1 = vicii.ext_background_color[0];
                c2 = vicii.ext_background_color[1];
                c3 = vicii.ext_background_color[2];
                switch (vbuf & 0xc0) {
                case 0x00:
                    draw_foreground_hires_byte(gbuf, bg, cbuf);
                    break;
                case 0x40:
                    draw_foreground_hires_byte(gbuf, c1, cbuf);
                    break;
                case 0x80:
                    draw_foreground_hires_byte(gbuf, c2, cbuf);
                    break;
                case 0xc0:
                    draw_foreground_hires_byte(gbuf, c3, cbuf);
                    break;
                }

                break;

            case VICII_IDLE_MODE:
                /* this currently doesn't work as expected */
                draw_foreground_hires_byte(gbuf, bg, 0);         
                break;
#endif

            }
            /* shift */
            gfx_shiftreg <<= 1;
            gfx_cnt++;
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

