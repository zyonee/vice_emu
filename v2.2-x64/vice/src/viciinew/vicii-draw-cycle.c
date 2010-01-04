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

struct RingEntry {
    BYTE color;
    enum { BG_COLOR } type;
};

/* graphics shift register */
#define GFX_SHIFTREG_MASK 15
static int gfx_bp = 0;
static struct RingEntry gfx_buffer[16];


static void push_gfx_buffer(BYTE c, int offs)
{
    gfx_buffer[(gfx_bp + offs) & GFX_SHIFTREG_MASK].color = c;
}


static void draw_border_byte(BYTE c)
{
    int i;
    int xs = vicii.regs[0x16] & 0x07;
    for (i = 0; i < 8; i++) {
        push_gfx_buffer(c, i + xs);
    }
}

static void draw_foreground_hires_byte(BYTE b, BYTE c0, BYTE c1)
{
    int i;
    int xs = vicii.regs[0x16] & 0x07;
    for (i = 0; i < 8; i++) {
        push_gfx_buffer( (b & 0x80) ? c1 : c0, i+xs );
        b <<= 1;
    }
}

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


static void render_8pixels(BYTE *p)
{
    int i;
    int bg = vicii.regs[0x21];

    for (i = 0; i < 8; i++) {
        p[i] = gfx_buffer[gfx_bp].color;
        gfx_buffer[gfx_bp].color = bg;
        gfx_bp = (gfx_bp + 1) & GFX_SHIFTREG_MASK;
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
        gfx_bp = 0;
        for (i=0; i < 16; i++) {
            gfx_buffer[i].color = 0;
        }
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
            draw_foreground_hires_byte(gbuf, bg, cbuf);
            break;

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

        }
    } else {
#if 0
        /* we are outside the display area */
        BYTE c = vicii.regs[0x20];
        /* separate function? */
        draw_border_byte(c);
#endif
    }

    render_8pixels(&vicii.dbuf[i]);
    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

