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

/* disable for debugging */
#define DRAW_INLINE inline

/* foreground/background graphics */

BYTE gbuf_pipe0_reg = 0;
BYTE cbuf_pipe0_reg = 0;
BYTE vbuf_pipe0_reg = 0;
BYTE gbuf_pipe1_reg = 0;
BYTE cbuf_pipe1_reg = 0;
BYTE vbuf_pipe1_reg = 0;

BYTE xscroll_pipe = 0;
BYTE vmode_pipe = 0;

/* gbuf shift register */
BYTE gbuf_reg = 0;
BYTE gbuf_mc_flop = 0;
BYTE gbuf_pixel_reg = 0;

/* cbuf and vbuf registers */
BYTE cbuf_reg = 0;
BYTE vbuf_reg = 0;

/* sprites */
int sprite_x_pipe[8];
DWORD sprite_data_pipe[8];
BYTE sprite_pri_bits = 0;
BYTE sprite_mc_bits = 0;
BYTE sprite_expx_bits = 0;

BYTE sprite_pending_bits = 0;
BYTE sprite_active_bits = 0;
BYTE sprite_halt_bits = 0;

/* sbuf shift registers */
DWORD sbuf_reg[8];
BYTE sbuf_pixel_reg[8];
BYTE sbuf_expx_flops;
BYTE sbuf_mc_flops;

/* border */
static int border_state = 0;
static int main_border_pipe = 0;

/* intermediate pixel result */
static BYTE current_pixel = 0;

static DRAW_INLINE void draw_sprites(int xpos, BYTE pixel_pri)
{
    int s;
    int active_sprite;
    BYTE collision_mask;

    /* do nothing if all sprites are disabled */
    if ( !(sprite_pending_bits || sprite_active_bits) ) {
        return;
    }

    /* check for pending */
    for (s = 0; s < 8; s++) {
        BYTE m = 1 << s;

        /* start rendering on position match */
        if ( (sprite_pending_bits & m) && !(sprite_halt_bits & m) ) {
            if ( xpos == sprite_x_pipe[s] ) {
                sbuf_reg[s] = sprite_data_pipe[s];
                sbuf_expx_flops |= m;
                sbuf_mc_flops |= m;
                sprite_active_bits |= m;
                sprite_pending_bits &= ~m;
            }
        }
    }

    /* do nothing if all sprites are inactive */
    if ( !sprite_active_bits ) {
        return;
    }

    /* check for active sprites */
    active_sprite = -1;
    collision_mask = 0;
    for (s = 7; s >= 0; --s) {
        BYTE m = 1 << s;

        if ( sprite_active_bits & m ) {
            /* render pixels if shift register or pixel reg still contains data */
            if ( sbuf_reg[s] || sbuf_pixel_reg[s] ) {
                     
                if ( !(sprite_halt_bits & m) ) {
                    if ( sbuf_expx_flops & m ) {
                        if (sprite_mc_bits & m) {
                            if ( sbuf_mc_flops & m ) {
                                /* fetch 2 bits */
                                sbuf_pixel_reg[s] = (sbuf_reg[s] >> 22) & 0x03;
                            }
                            sbuf_mc_flops ^= m;
                        } else {
                            /* fetch 1 bit and make it 0 or 2 */
                            sbuf_pixel_reg[s] = ( (sbuf_reg[s] >> 23) & 0x01 ) << 1;
                        }
                    }

                    /* shift the sprite buffer and handle expansion flags */
                    if ( sbuf_expx_flops & m ) {
                        sbuf_reg[s] <<= 1;
                    }
                    if (sprite_expx_bits & m) {
                        sbuf_expx_flops ^= m;
                    }
                }

                /*
                 * set collision mask bits and determine the highest
                 * priority sprite number that has a pixel.
                 */
                if (sbuf_pixel_reg[s]) {
                    active_sprite = s;
                    collision_mask |= m;
                }

            } else {
                sprite_active_bits &= ~m;
            }
        }
    }

    if (collision_mask) {
        int s = active_sprite;
        BYTE spri = sprite_pri_bits & (1 << s);
        if ( !(pixel_pri && spri) ) {
            switch (sbuf_pixel_reg[s]) {
            case 1:
                current_pixel = vicii.regs[0x25];
                break;
            case 2:
                current_pixel = vicii.regs[0x27 + s];
                break;
            case 3:
                current_pixel = vicii.regs[0x26];
                break;
            default:
                break;
            }
        }
    }
    
    /* if 2 or more bits are set, trigger collisions */
    if ( collision_mask & (collision_mask-1) ) {
        vicii.sprite_sprite_collisions |= collision_mask;
    }
    if (pixel_pri) {
        vicii.sprite_background_collisions |= collision_mask;
    }
}


static DRAW_INLINE void update_sprite_mc_bits(void)
{
    BYTE toggled;
    toggled = vicii.regs[0x1c] ^ sprite_mc_bits;
    sbuf_mc_flops &= ~toggled;
    sprite_mc_bits = vicii.regs[0x1c];
}

static DRAW_INLINE void update_sprite_xpos_and_data(void)
{
    int s;
    for (s=0; s<8; s++) {
        sprite_x_pipe[s] = vicii.sprite[s].x;
        sprite_data_pipe[s] = vicii.sprite[s].data;
    }
}


enum lookup_t {
    COL_BLACK    = 0,
    COL_VBUF_L   = 1,
    COL_VBUF_H   = 2,
    COL_CBUF     = 3,
    COL_CBUF_MC  = 4,
    COL_D02X_EXT = 5,
    COL_D021     = 0x21,
    COL_D022     = 0x22,
    COL_D023     = 0x23,
    COL_D024     = 0x24
};

static const BYTE colors[] = {
    COL_D021,   COL_BLACK,  COL_BLACK,  COL_CBUF,   /* ECM=0 BMM=0 MCM=0 */
    COL_D021,   COL_D022,   COL_D023,   COL_CBUF_MC,/* ECM=0 BMM=0 MCM=1 */
    COL_VBUF_L, COL_BLACK,  COL_BLACK,  COL_VBUF_H, /* ECM=0 BMM=1 MCM=0 */
    COL_D021,   COL_VBUF_H, COL_VBUF_L, COL_CBUF,   /* ECM=0 BMM=1 MCM=1 */
    COL_D02X_EXT, COL_BLACK, COL_BLACK, COL_CBUF,   /* ECM=1 BMM=0 MCM=0 */
    COL_BLACK,  COL_BLACK,  COL_BLACK,  COL_BLACK,  /* ECM=1 BMM=0 MCM=1 */
    COL_BLACK,  COL_BLACK,  COL_BLACK,  COL_BLACK,  /* ECM=1 BMM=1 MCM=0 */
    COL_BLACK,  COL_BLACK,  COL_BLACK,  COL_BLACK   /* ECM=1 BMM=1 MCM=1 */
};

void vicii_draw_cycle(void)
{
    int cycle, offs, i;
    int xpos;
    BYTE csel;
    cycle = vicii.raster_cycle;

    /* convert cycle to an x-position. */
    if (cycle < 13) {
        xpos = cycle * 8 + 0x190;
    } else if (cycle < 54) {
        xpos = (cycle - 13) * 8;
    } else {
        xpos = (cycle - 15) * 8;
    }

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
    }

    offs = vicii.dbuf_offset;
    /* guard */
    if (offs >= VICII_DRAW_BUFFER_SIZE) 
        return;

    csel = vicii.regs[0x16] & 0x8;

    /* render pixels */
    for (i = 0; i < 8; i++) {
        BYTE px = 0;
        BYTE cc;
        BYTE pixel_pri;

        /* pipe sprite related changes 2 pixels late */
        if (i == 2) {
            sprite_active_bits &= ~vicii.sprite_dma_cycle_2;
        }
        if (i == 3) {
            sprite_halt_bits |= vicii.sprite_dma_cycle_0;
        }
        if (i == 4 && cycle == VICII_PAL_CYCLE(58)) {
            sprite_pending_bits = vicii.sprite_display_bits;
        }
        if (i == 6) {
            sprite_pri_bits = vicii.regs[0x1b];
            sprite_expx_bits = vicii.regs[0x1d];
        }
        if (i == 7) {
            update_sprite_mc_bits();
            sprite_halt_bits &= ~vicii.sprite_dma_cycle_2;
        }

        /* Load new gbuf/vbuf/cbuf values at offset == xscroll */
        if (i == xscroll_pipe) {
            /* latch values at time xs */
            vbuf_reg = vbuf_pipe1_reg;
            cbuf_reg = cbuf_pipe1_reg;
            gbuf_reg = gbuf_pipe1_reg;
            gbuf_mc_flop = 1;
        }
           
        /* Load new border mask depending on csel and xscroll */
        if (csel) {
            if (i == 0) {
                border_state = main_border_pipe;
            }
        } else {
            if (i == 7) {
                border_state = vicii.main_border;
            }
        }

        /* 
         * read pixels depending on video mode
         * mc pixels if MCM=1 and BMM=1, or MCM=1 and cbuf bit 3 = 1
         */
        if ( (vmode_pipe & 0x04) &&
             ((vmode_pipe & 0x08) || (cbuf_reg & 0x08)) ) {
            /* mc pixels */
            if (gbuf_mc_flop) {
                gbuf_pixel_reg = gbuf_reg >> 6;
            }
        } else {
            /* hires pixels */
            gbuf_pixel_reg = (gbuf_reg & 0x80) ? 3 : 0;
        }
        px = gbuf_pixel_reg;

        /* Determine pixel color and priority */
        pixel_pri = (px & 0x2);
        cc = colors[vmode_pipe | px];

        /* shift the graphics buffer */
        gbuf_reg <<= 1;
        gbuf_mc_flop ^= 1;

        /* lookup colors and render pixel */
        switch (cc) {
        case COL_BLACK:
            current_pixel = 0;
            break;
        case COL_VBUF_L:
            current_pixel = vbuf_reg & 0x0f;
            break;
        case COL_VBUF_H:
            current_pixel = vbuf_reg >> 4;
            break;
        case COL_CBUF:
            current_pixel = cbuf_reg;
            break;
        case COL_CBUF_MC:
            current_pixel = cbuf_reg & 0x07;
            break;
        case COL_D02X_EXT:
            current_pixel = vicii.regs[0x21 + (vbuf_reg >> 6)];
            break;
        case COL_D021:
        case COL_D022:
        case COL_D023:
        case COL_D024:
            current_pixel = vicii.regs[cc];
            break;
        }

        /* process and render sprites */
        draw_sprites(xpos + i, pixel_pri);

        /* add border on top */
        if (border_state) {
            current_pixel = vicii.regs[0x20];
        }

        /* draw pixel to buffer */
        vicii.dbuf[offs + i] = current_pixel;

    }

    vicii.dbuf_offset += 8;


    /* shift and put the next data into the pipe. */
    vbuf_pipe1_reg = vbuf_pipe0_reg;
    cbuf_pipe1_reg = cbuf_pipe0_reg;
    gbuf_pipe1_reg = gbuf_pipe0_reg;

    /* this makes sure gbuf is 0 outside the visible area
       It should probably be done somewhere around the fetch instead */
    if ( (cycle >= VICII_PAL_CYCLE(15) && cycle <= VICII_PAL_CYCLE(54)) && vicii.vborder == 0) {
        gbuf_pipe0_reg = vicii.gbuf;
        xscroll_pipe = vicii.regs[0x16] & 0x07;
    } else {
        gbuf_pipe0_reg = 0;
    }

    /* Only update vbuf and cbuf registers in the display state. */
    if ( (cycle >= VICII_PAL_CYCLE(15) && cycle <= VICII_PAL_CYCLE(54)) && vicii.vborder == 0 ) {
        if (!vicii.idle_state) {
            vbuf_pipe0_reg = vicii.vbuf[cycle - VICII_PAL_CYCLE(15)];
            cbuf_pipe0_reg = vicii.cbuf[cycle - VICII_PAL_CYCLE(15)];
        } else {
            vbuf_pipe0_reg = 0;
            cbuf_pipe0_reg = 0;
        }
    } 

    main_border_pipe = vicii.main_border;

    vmode_pipe = vicii.video_mode << 2;

    update_sprite_xpos_and_data();
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

