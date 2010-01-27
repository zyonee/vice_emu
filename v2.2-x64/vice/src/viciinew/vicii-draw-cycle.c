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

static DRAW_INLINE void trigger_sprites(int xpos)
{
    int s;

    /* do nothing if no sprites are pending */
    if ( !sprite_pending_bits ) {
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
}

static DRAW_INLINE void draw_sprites(BYTE pixel_pri)
{
    int s;
    int active_sprite;
    BYTE collision_mask;


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
    COL_NONE     = 0,
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
    COL_D021,   COL_NONE,   COL_NONE,   COL_CBUF,   /* ECM=0 BMM=0 MCM=0 */
    COL_D021,   COL_D022,   COL_D023,   COL_CBUF_MC,/* ECM=0 BMM=0 MCM=1 */
    COL_VBUF_L, COL_NONE,   COL_NONE,   COL_VBUF_H, /* ECM=0 BMM=1 MCM=0 */
    COL_D021,   COL_VBUF_H, COL_VBUF_L, COL_CBUF,   /* ECM=0 BMM=1 MCM=1 */
    COL_D02X_EXT, COL_NONE,  COL_NONE,  COL_CBUF,   /* ECM=1 BMM=0 MCM=0 */
    COL_NONE,   COL_NONE,   COL_NONE,   COL_NONE,   /* ECM=1 BMM=0 MCM=1 */
    COL_NONE,   COL_NONE,   COL_NONE,   COL_NONE,   /* ECM=1 BMM=1 MCM=0 */
    COL_NONE,   COL_NONE,   COL_NONE,   COL_NONE    /* ECM=1 BMM=1 MCM=1 */
};

#define XPOS(x)            ( (x) & 0x1f8 )
#define GET_XPOS(x)        ( (x) & 0x1f8 )

#define DMA_CYCLE_0(x)     ( ((1 << (x)) << 16))
#define DMA_CYCLE_2(x)     ( ((1 << (x)) << 24))
#define GET_DMA_CYCLE_0(x) ( ((x) >> 16) & 0xff )
#define GET_DMA_CYCLE_2(x) ( ((x) >> 24) & 0xff )

#define SPR_EN         (1)
#define IS_SPR_EN(x)   ( (x) & 1)

#define VISIBLE(x)     ( 0x8000 | ((x) << 9) )
#define IS_VISIBLE(x)  ( (x) & 0x8000 )
#define GET_VISIBLE(x) ( ((x) >> 9) & 0x3f )

static unsigned int flag_tab[] = {
    XPOS(0x190) | DMA_CYCLE_0(3), /*  1 */
    XPOS(0x198) | DMA_CYCLE_2(3), /*  2 */
    XPOS(0x1a0) | DMA_CYCLE_0(4), /*  3 */
    XPOS(0x1a8) | DMA_CYCLE_2(4), /*  4 */
    XPOS(0x1b0) | DMA_CYCLE_0(5), /*  5 */
    XPOS(0x1b8) | DMA_CYCLE_2(5), /*  6 */
    XPOS(0x1c0) | DMA_CYCLE_0(6), /*  7 */
    XPOS(0x1c8) | DMA_CYCLE_2(6), /*  8 */
    XPOS(0x1d0) | DMA_CYCLE_0(7), /*  9 */
    XPOS(0x1d8) | DMA_CYCLE_2(7), /* 10 */
    XPOS(0x1e0),  /* 11 */
    XPOS(0x1e8),  /* 12 */
    XPOS(0x1f0),  /* 13 */
    XPOS(0x000),  /* 14 */
    XPOS(0x008) | VISIBLE(0),  /* 15 */
    XPOS(0x010) | VISIBLE(1),  /* 16 */
    XPOS(0x018) | VISIBLE(2),  /* 17 */
    XPOS(0x020) | VISIBLE(3),  /* 18 */
    XPOS(0x028) | VISIBLE(4),  /* 19 */
    XPOS(0x030) | VISIBLE(5),  /* 20 */
    XPOS(0x038) | VISIBLE(6),  /* 21 */
    XPOS(0x040) | VISIBLE(7),  /* 22 */
    XPOS(0x048) | VISIBLE(8),  /* 23 */
    XPOS(0x050) | VISIBLE(9),  /* 24 */
    XPOS(0x058) | VISIBLE(10), /* 25 */
    XPOS(0x060) | VISIBLE(11), /* 26 */
    XPOS(0x068) | VISIBLE(12), /* 27 */
    XPOS(0x070) | VISIBLE(13), /* 28 */
    XPOS(0x078) | VISIBLE(14), /* 29 */
    XPOS(0x080) | VISIBLE(15), /* 30 */
    XPOS(0x088) | VISIBLE(16), /* 31 */
    XPOS(0x090) | VISIBLE(17), /* 32 */
    XPOS(0x098) | VISIBLE(18), /* 33 */
    XPOS(0x0a0) | VISIBLE(19), /* 34 */
    XPOS(0x0a8) | VISIBLE(20), /* 35 */
    XPOS(0x0b0) | VISIBLE(21), /* 36 */
    XPOS(0x0b8) | VISIBLE(22), /* 37 */
    XPOS(0x0c0) | VISIBLE(23), /* 38 */
    XPOS(0x0c8) | VISIBLE(24), /* 39 */
    XPOS(0x0d0) | VISIBLE(25), /* 40 */
    XPOS(0x0d8) | VISIBLE(26), /* 41 */
    XPOS(0x0e0) | VISIBLE(27), /* 42 */
    XPOS(0x0e8) | VISIBLE(28), /* 43 */
    XPOS(0x0f0) | VISIBLE(29), /* 44 */
    XPOS(0x0f8) | VISIBLE(30), /* 45 */
    XPOS(0x100) | VISIBLE(31), /* 46 */
    XPOS(0x108) | VISIBLE(32), /* 47 */
    XPOS(0x110) | VISIBLE(33), /* 48 */
    XPOS(0x118) | VISIBLE(34), /* 49 */
    XPOS(0x120) | VISIBLE(35), /* 50 */
    XPOS(0x128) | VISIBLE(36), /* 51 */
    XPOS(0x130) | VISIBLE(37), /* 52 */
    XPOS(0x138) | VISIBLE(38), /* 53 */
    XPOS(0x140) | VISIBLE(39), /* 54 */
    XPOS(0x000),
    XPOS(0x000),
    XPOS(0x148),  /* 55 */
    XPOS(0x150),  /* 56 */
    XPOS(0x158),  /* 57 */
    XPOS(0x160) | DMA_CYCLE_0(0) | SPR_EN, /* 58 */
    XPOS(0x168) | DMA_CYCLE_2(0), /* 59 */
    XPOS(0x170) | DMA_CYCLE_0(1), /* 60 */
    XPOS(0x178) | DMA_CYCLE_2(1), /* 61 */
    XPOS(0x180) | DMA_CYCLE_0(2), /* 62 */
    XPOS(0x188) | DMA_CYCLE_2(2)  /* 63 */
};


void vicii_draw_cycle(void)
{
    int cycle, offs, i;
    BYTE csel;
    unsigned int flags;
    int xpos;
    int spr_en;
    int vis_en;
    int li;
    BYTE dma_cycle_0;
    BYTE dma_cycle_2;

    cycle = vicii.raster_cycle;
    flags = flag_tab[cycle];

    xpos = GET_XPOS(flags);
    spr_en = IS_SPR_EN(flags);
    vis_en = IS_VISIBLE(flags);
    li = GET_VISIBLE(flags);
    dma_cycle_0 = GET_DMA_CYCLE_0(flags);
    dma_cycle_2 = GET_DMA_CYCLE_2(flags);

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

        /* pipe sprite related changes various amounts of pixels late */
        if (i == 2) {
            sprite_active_bits &= ~dma_cycle_2;
        }
        if (i == 3) {
            sprite_halt_bits |= dma_cycle_0;
        }
        if (spr_en && i == 4) {
            sprite_pending_bits = vicii.sprite_display_bits;
        }
        if (i == 6) {
            sprite_pri_bits = vicii.regs[0x1b];
            sprite_expx_bits = vicii.regs[0x1d];
        }
        if (i == 7) {
            update_sprite_mc_bits();
            sprite_halt_bits &= ~dma_cycle_2;
        }

        /* Load new gbuf/vbuf/cbuf values at offset == xscroll */
        if (i == xscroll_pipe) {
            /* latch values at time xs */
            vbuf_reg = vbuf_pipe1_reg;
            cbuf_reg = cbuf_pipe1_reg;
            gbuf_reg = gbuf_pipe1_reg;
            gbuf_mc_flop = 1;
        }
           
        /* Set new border state depending on csel and current pixel */
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
        case COL_NONE:
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
        trigger_sprites(xpos + i);
        draw_sprites(pixel_pri);

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
    if ( vis_en && vicii.vborder == 0) {
        gbuf_pipe0_reg = vicii.gbuf;
        xscroll_pipe = vicii.regs[0x16] & 0x07;
    } else {
        gbuf_pipe0_reg = 0;
    }

    /* Only update vbuf and cbuf registers in the display state. */
    if ( vis_en && vicii.vborder == 0 ) {
        if (!vicii.idle_state) {
            vbuf_pipe0_reg = vicii.vbuf[li];
            cbuf_pipe0_reg = vicii.cbuf[li];
        } else {
            vbuf_pipe0_reg = 0;
            cbuf_pipe0_reg = 0;
        }
    } 

    main_border_pipe = vicii.main_border;

    vmode_pipe = ( (vicii.regs[0x11] & 0x60) | (vicii.regs[0x16] & 0x10) ) >> 2;

    update_sprite_xpos_and_data();
}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

