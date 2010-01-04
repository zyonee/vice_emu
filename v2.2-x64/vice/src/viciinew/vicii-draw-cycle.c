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

static void draw_background_byte(BYTE *p, BYTE c)
{
    memset(p, c, 8);
}

static void draw_foreground_hires_byte(BYTE *p, BYTE b, BYTE c)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (b & 0x80) {
            p[i] = c;
        }
        b <<= 1;
    }
}

static void draw_foreground_mc_byte(BYTE *p, BYTE b, BYTE c1, BYTE c2, BYTE c3)
{
    int i;
    for (i = 0; i < 8; i += 2) {
        switch (b & 0xc0) {
        case 0x40:
            p[i] = p[i+1] = c1;
            break;
        case 0x80:
            p[i] = p[i+1] = c2;
            break;
        case 0xc0:
            p[i] = p[i+1] = c3;
            break;
        }

        b <<= 2;
    }
}

void vicii_draw_cycle(void)
{
    int cycle, i;
    BYTE vbuf, cbuf, gbuf;

    cycle = vicii.raster_cycle;
    vbuf = 0;
    cbuf = 0;
    gbuf = 0;

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
    }
    i = vicii.dbuf_offset;
    /* guard */
    if (i >= VICII_DRAW_BUFFER_SIZE) 
        return;
    
    /* are we within the display area? */
    if (cycle >= 14 && cycle <= 53) {
        BYTE bg, c1, c2, c3;

        bg = vicii.regs[0x21];
        
        vbuf = vicii.vbuf[cycle - 14];
        cbuf = vicii.cbuf[cycle - 14];
        gbuf = vicii.gbuf[cycle - 14]; /* this shouldn't be a buffer */

        switch (vicii.video_mode) {

        case VICII_NORMAL_TEXT_MODE:
            draw_background_byte(&vicii.dbuf[i], bg);
            draw_foreground_hires_byte(&vicii.dbuf[i], gbuf, cbuf);
            break;

        case VICII_MULTICOLOR_TEXT_MODE:
            draw_background_byte(&vicii.dbuf[i], bg);
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = cbuf & 0x07;
            if (cbuf & 0x08) {
                draw_foreground_mc_byte(&vicii.dbuf[i], gbuf, c1, c2, c3);
            } else {
                draw_foreground_hires_byte(&vicii.dbuf[i], gbuf, c3);
            }
            break;

        case VICII_HIRES_BITMAP_MODE:
            c1 = vbuf & 0x0f;
            c2 = vbuf >> 4;
            draw_foreground_hires_byte(&vicii.dbuf[i], 0xff, c1);
            draw_foreground_hires_byte(&vicii.dbuf[i], gbuf, c2);
            break;

        case VICII_MULTICOLOR_BITMAP_MODE:
            c1 = vbuf >> 4;
            c2 = vbuf & 0x0f;
            c3 = cbuf;
            draw_background_byte(&vicii.dbuf[i], bg);
            draw_foreground_mc_byte(&vicii.dbuf[i], gbuf, c1, c2, c3);
            break;
            
        case VICII_EXTENDED_TEXT_MODE:
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = vicii.ext_background_color[2];
            switch (vbuf & 0xc0) {
            case 0x00:
                draw_background_byte(&vicii.dbuf[i], bg);
                break;
            case 0x40:
                draw_background_byte(&vicii.dbuf[i], c1);
                break;
            case 0x80:
                draw_background_byte(&vicii.dbuf[i], c2);
                break;
            case 0xc0:
                draw_background_byte(&vicii.dbuf[i], c3);
                break;
            }

            draw_foreground_hires_byte(&vicii.dbuf[i], gbuf, cbuf);
            break;

        case VICII_IDLE_MODE:
            /* this currently doesn't work as expected */
            draw_background_byte(&vicii.dbuf[i], bg);
            draw_foreground_hires_byte(&vicii.dbuf[i], gbuf, 0);         
            break;

        }
    } else {
        /* we are outside the display area */
        BYTE c = vicii.regs[0x20];
        /* separate function? */
        draw_background_byte(&vicii.dbuf[i], c);
    }
    
    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

