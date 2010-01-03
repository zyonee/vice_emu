/*
 * vicii-draw-cycle.c - Cycle based rendering for the VIC-II emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
 *
 * Based on code written by
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

#include "types.h"
#include "vicii-draw-cycle.h"
#include "viciitypes.h"
#include "viewport.h"


#define DRAW_STD_TEXT_BYTE(p, b, f)       \
    do {                                  \
        if ((b) & 0x80) *(p) = (f);       \
        if ((b) & 0x40) *((p) + 1) = (f); \
        if ((b) & 0x20) *((p) + 2) = (f); \
        if ((b) & 0x10) *((p) + 3) = (f); \
        if ((b) & 0x08) *((p) + 4) = (f); \
        if ((b) & 0x04) *((p) + 5) = (f); \
        if ((b) & 0x02) *((p) + 6) = (f); \
        if ((b) & 0x01) *((p) + 7) = (f); \
    } while (0)                           \


#define DRAW_MC_BYTE(p, b, f1, f2, f3)          \
    do {                                        \
        if ((b) & 0x80) {                       \
            if ((b) & 0x40)                     \
                *(p) = *((p) + 1) = (f3);       \
            else                                \
                *(p) = *((p) + 1) = (f2);       \
        } else if ((b) & 0x40)                  \
            *(p) = *((p) + 1) = (f1);           \
                                                \
        if ((b) & 0x20) {                       \
            if ((b) & 0x10)                     \
                *((p) + 2) = *((p) + 3) = (f3); \
            else                                \
                *((p) + 2) = *((p) + 3) = (f2); \
        } else if ((b) & 0x10)                  \
            *((p) + 2) = *((p) + 3) = (f1);     \
                                                \
        if ((b) & 0x08) {                       \
            if ((b) & 0x04)                     \
                *((p) + 4) = *((p) + 5) = (f3); \
            else                                \
                *((p) + 4) = *((p) + 5) = (f2); \
        } else if ((b) & 0x04)                  \
            *((p) + 4) = *((p) + 5) = (f1);     \
                                                \
        if ((b) & 0x02) {                       \
            if ((b) & 0x01)                     \
                *((p) + 6) = *((p) + 7) = (f3); \
            else                                \
                *((p) + 6) = *((p) + 7) = (f2); \
        } else if ((b) & 0x01)                  \
            *((p) + 6) = *((p) + 7) = (f1);     \
    } while (0)



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
    if (i >= 65*8) 
        return;
    

    if (cycle >= 14 && cycle <= 53) {
        BYTE bg, c1, c2, c3;

        bg = vicii.regs[0x21];
        
        vbuf = vicii.vbuf[cycle - 14];
        cbuf = vicii.cbuf[cycle - 14];
        gbuf = vicii.gbuf[cycle - 14];

        switch (vicii.video_mode) {

        case VICII_NORMAL_TEXT_MODE:
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, cbuf);
            break;

        case VICII_MULTICOLOR_TEXT_MODE:
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = cbuf & 0x07;
            if (cbuf & 0x08) {
                DRAW_MC_BYTE(&vicii.dbuf[i], gbuf, c1, c2, c3);
            } else {
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, c3);
            }
            break;

        case VICII_HIRES_BITMAP_MODE:
            c1 = vbuf & 0x0f;
            c2 = vbuf >> 4;
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c1);
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, c2);
            break;

        case VICII_MULTICOLOR_BITMAP_MODE:
            c1 = vbuf >> 4;
            c2 = vbuf & 0x0f;
            c3 = cbuf;
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            DRAW_MC_BYTE(&vicii.dbuf[i], gbuf, c1, c2, c3);
            break;
            
        case VICII_EXTENDED_TEXT_MODE:
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = vicii.ext_background_color[2];
            switch (vbuf & 0xc0) {
            case 0x00:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
                break;
            case 0x40:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c1);
                break;
            case 0x80:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c2);
                break;
            case 0xc0:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c3);
                break;
            }

            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, cbuf);
            break;

        case VICII_IDLE_MODE:
            /* this currently doesn't work as expected */
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, 0);         
            break;

        }
    } else {
        /* border */
        BYTE c = vicii.regs[0x20];
        DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c+1);
    }
    
    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, 65*8); /* this should really use a define */
    vicii.dbuf_offset = 0;
}

