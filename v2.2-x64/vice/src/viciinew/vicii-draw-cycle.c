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
BYTE gbuf_mc_flop = 0;
BYTE gbuf_pixel_reg = 0;

/* cbuf and vbuf registers */
BYTE cbuf_reg = 0;
BYTE vbuf_reg = 0;

/* sprites */

/* sbuf shift registers */
DWORD sbuf_reg[8]; /* maybe use those directly present in vicii.*? */
BYTE sbuf_pixel_reg[8];
BYTE sbuf_expx_flop[8];
BYTE sbuf_mc_flop[8];


static void draw_sprites(int cycle, int i, int j, int pri)
{
    int s;
    BYTE c[4];
    int rendered;
    int x;

    /* convert cycle to an x-position. */
    if (cycle < 11) {
        x = cycle * 8 + 0x1a0 + i;
    } else {
        x = (cycle - 11) * 8 + i;
    }

    c[1] = vicii.regs[0x25];
    c[3] = vicii.regs[0x26];

    rendered = 0;
    /* Brute force render sprites on top of graphics for now! */
    for (s = 0; s < 8; s++) {
        int spri = vicii.regs[0x1b] & (1<<s);
        int mc = vicii.regs[0x1c] & (1<<s);
        int expx = vicii.regs[0x1d] & (1<<s);
        int sprx = vicii.regs[0x00 + s*2];
        sprx |= (vicii.regs[0x10] & (1<<s)) ? 0x100 : 0;
        c[2] = vicii.regs[0x27 + s];

        /* fetch sprite data on position match */
        if (x == sprx) {
            DWORD *tmp;
            raster_sprite_status_t *sprite_status = vicii.raster.sprite_status;
            DWORD *data = sprite_status->new_sprite_data;
            sbuf_reg[s] = ((data[s] >> 16) & 0x0000ff) | (data[s] & 0x00ff00) | ((data[s] << 16) & 0xff0000);

            sbuf_expx_flop[s] = 0;
            sbuf_mc_flop[s] = 0;
        }

        /* render pixels if shift register or pixel reg still contains data */
        if ( sbuf_reg[s] || sbuf_pixel_reg[s] ) {

            if ( sbuf_expx_flop[s] == 0 ) {
                if (mc) {
                    if (sbuf_mc_flop[s] == 0) {
                        /* fetch 2 bits */
                        sbuf_pixel_reg[s] = (sbuf_reg[s] >> 22) & 0x03;
                    }
                    sbuf_mc_flop[s] = ~sbuf_mc_flop[s];
                } else {
                    /* fetch 1 bit and make it 0 or 2 */
                    sbuf_pixel_reg[s] = ( (sbuf_reg[s] >> 23) & 0x01 ) << 1;
                }
            }

            /*
             * render pixels unless a higher priority sprite has already
             * rendered pixels.
             */
            if (!rendered && sbuf_pixel_reg[s]) {
                if ( !(pri && spri) ) {
                    vicii.dbuf[j] = c[ sbuf_pixel_reg[s] ];
                }
                rendered = 1;
            }

            /* shift the sprite buffer and handle expansion flags */
            if (sbuf_expx_flop[s] == 0) {
                sbuf_reg[s] <<= 1;
            }
            if (expx) {
                sbuf_expx_flop[s] = ~sbuf_expx_flop[s];
            }
        }
    }
}

void vicii_draw_cycle(void)
{
    int cycle, offs, i;

    cycle = vicii.raster_cycle;

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
        gbuf_mc_flop = 0;
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
        xs = vicii.regs[0x16];

        /* render pixels */
        for (i = 0; i < 8; i++) {
            int j = i + offs;
            BYTE px;
            BYTE c[4];
            int pri;
            int vmode;

            if (i == xs) {
                /* latch values at time xs */
                vbuf_reg = vicii.vbuf[cycle - 14];
                cbuf_reg = vicii.cbuf[cycle - 14];
                gbuf_reg = vicii.gbuf[cycle - 14]; /* this shouldn't be a buffer */
                gbuf_mc_flop = 0;
            }

           
            c[0] = bg;

            /*
             * force VICII_IDLE_MODE if we are in idle state
             * Why is VICII_IDLE_MODE a real mode? Should be checked!
             */
            if (vicii.idle_state) {
                vmode = VICII_IDLE_MODE;
            } else {
                vmode = vicii.video_mode;
            }

            /* setup colors and read pixels depending on video mode */
            switch (vmode) {

            case VICII_NORMAL_TEXT_MODE:
                c[3] = cbuf_reg;

                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_MULTICOLOR_TEXT_MODE:
                c[1] = vicii.ext_background_color[0];
                c[2] = vicii.ext_background_color[1];
                c[3] = cbuf_reg & 0x07;
                if (cbuf_reg & 0x08) {
                    /* mc pixels */
                    if (gbuf_mc_flop == 0) {
                        gbuf_pixel_reg = gbuf_reg >> 6;
                    }
                    px = gbuf_pixel_reg;
                } else {
                    /* hires pixels */
                    px = (gbuf_reg & 0x80) ? 3 : 0;
                }
                break;

            case VICII_HIRES_BITMAP_MODE:
                c[0] = vbuf_reg & 0x0f;
                c[3] = vbuf_reg >> 4;
                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_MULTICOLOR_BITMAP_MODE:
                c[1] = vbuf_reg >> 4;
                c[2] = vbuf_reg & 0x0f;
                c[3] = cbuf_reg;
                if (gbuf_mc_flop == 0) {
                    gbuf_pixel_reg = gbuf_reg >> 6;
                }
                px = gbuf_pixel_reg;
                break;
            
            case VICII_EXTENDED_TEXT_MODE:
                c[3] = cbuf_reg;

                switch (vbuf_reg & 0xc0) {
                case 0x00:
                    c[0] = bg;
                    break;
                case 0x40:
                    c[0] = vicii.ext_background_color[0];
                    break;
                case 0x80:
                    c[0] = vicii.ext_background_color[1];
                    break;
                case 0xc0:
                    c[0] = vicii.ext_background_color[2];
                    break;
                }

                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_IDLE_MODE:
                /* this currently doesn't work as expected */
                c[3] = 0;

                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_ILLEGAL_TEXT_MODE:
                c[0] = 0;
                c[1] = 0;
                c[2] = 0;
                c[3] = 0;
                if (cbuf_reg & 0x08) {
                    /* mc pixels */
                    if (gbuf_mc_flop == 0) {
                        gbuf_pixel_reg = gbuf_reg >> 6;
                    }
                    px = gbuf_pixel_reg;
                } else {
                    /* hires pixels */
                    px = (gbuf_reg & 0x80) ? 3 : 0;
                }
                break;

            case VICII_ILLEGAL_BITMAP_MODE_1:
                c[0] = 0;
                c[3] = 0;
                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_ILLEGAL_BITMAP_MODE_2:
                c[0] = 0;
                c[1] = 0;
                c[2] = 0;
                c[3] = 0;

                /* mc pixels */
                if (gbuf_mc_flop == 0) {
                    gbuf_pixel_reg = gbuf_reg >> 6;
                }
                px = gbuf_pixel_reg;
                break;

            }

            /* plot color from prepared array */
            vicii.dbuf[j] = c[px];
            pri = (px & 0x2);

            /* shift the graphics buffer */
            gbuf_reg <<= 1;
            gbuf_mc_flop = ~gbuf_mc_flop;
            
            draw_sprites(cycle, i, j, pri);

        }
    } else {

        /* we are outside the display area */

        /* render pixels */
        for (i = 0; i < 8; i++) {
            int j = i + offs;
            /* plot bg for now */
            vicii.dbuf[j] = vicii.regs[0x21];
            draw_sprites(cycle, i, j, 0);
        }

    }

    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

