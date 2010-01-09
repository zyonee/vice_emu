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
BYTE idle_state_pipe = 0;

/* gbuf shift register */
BYTE gbuf_reg = 0;
BYTE gbuf_mc_flop = 0;
BYTE gbuf_pixel_reg = 0;

/* cbuf and vbuf registers */
BYTE cbuf_reg = 0;
BYTE vbuf_reg = 0;

/* sprites */
int sprite_x_pipe[8];
int sprite_x_pipe0[8];
BYTE sprite_mc_bits = 0;
BYTE sprite_expx_bits = 0;

BYTE sprite_pending_bits = 0;
BYTE sprite_active_bits = 0;
BYTE sprite_halt_bits = 0;

/* sbuf shift registers */
DWORD sbuf_reg[8];
BYTE sbuf_pixel_reg[8];
BYTE sbuf_expx_flop[8];
BYTE sbuf_mc_flop[8];

/* border */
int border_state = 0;
int main_border_pipe = 0;


/* intermediate pixel result */
static BYTE current_pixel = 0;

static DRAW_INLINE void draw_sprites(int xpos, int pixel_pri)
{
    int s;
    BYTE c[4];
    int rendered;
    BYTE collision_mask;
    int collision_count;
    int x;

    /* do nothing if all sprites are disabled */
    if (!vicii.sprite_display_bits) {
        return;
    }

    c[1] = vicii.regs[0x25];
    c[3] = vicii.regs[0x26];


    rendered = 0;
    collision_mask = 0;
    collision_count = 0;
    for (s = 0; s < 8; s++) {

       if ( vicii.sprite_display_bits & (1<<s) ) {

            /* start rendering on position match */
            if ( sprite_pending_bits & (1 << s) ) {
                if ( xpos == sprite_x_pipe[s] ) {
                    sbuf_reg[s] = vicii.sprite[s].data;
                    sbuf_expx_flop[s] = 0;
                    sbuf_mc_flop[s] = 0;
                    sprite_active_bits |= (1 << s);
                    sprite_pending_bits &= ~(1 << s);
                }
            }

            if ( sprite_active_bits & (1 << s) ) {
                int spri = vicii.regs[0x1b] & (1 << s);
                c[2] = vicii.regs[0x27 + s];

                /* render pixels if shift register or pixel reg still contains data */
                if ( sbuf_reg[s] || sbuf_pixel_reg[s] ) {
                    

                    
                    if ( sbuf_expx_flop[s] == 0 ) {
                        if (sprite_mc_bits & (1 << s)) {
                            if (sbuf_mc_flop[s] == 0) {
                            } else {
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
                    if (sbuf_pixel_reg[s]) {
                        if (!rendered) {
                            if ( !(pixel_pri && spri) ) {
                                current_pixel = c[ sbuf_pixel_reg[s] ];
                            }
                            rendered = 1;
                        }
                        collision_mask |= (1 << s);
                        collision_count++;
                    }

                    if ( !(sprite_halt_bits & (1 << s)) ) {
                        /* shift the sprite buffer and handle expansion flags */
                        if (sbuf_expx_flop[s] == 0) {
                            sbuf_reg[s] <<= 1;
                        }
                        if (sprite_expx_bits & (1<<s)) {
                            sbuf_expx_flop[s] = ~sbuf_expx_flop[s];
                        }
                    }
                } else {
                    sprite_active_bits &= ~(1 << s);
                }
            }
        }
    }
    
    if (collision_count > 1) {
        vicii.sprite_sprite_collisions |= collision_mask;
    }

    if (!vicii.vborder && pixel_pri) {
        /* only flag collisions when not in the vborder area */
            vicii.sprite_background_collisions |= collision_mask;
    }
}


static DRAW_INLINE void update_sprite_flags4(int cycle)
{
    sprite_halt_bits = 0;
    /* this is replicated a bit from vicii-cycle.c */
    switch (cycle) {
    case 59:
        sprite_halt_bits = (1<<0);
        sprite_pending_bits |= (1<<0);
        break;
    case 61:
        sprite_halt_bits = (1<<1);
        sprite_pending_bits |= (1<<1);
        break;
    case 63:
        sprite_halt_bits = (1<<2);
        sprite_pending_bits |= (1<<2);
        break;
    case 0:
        sprite_halt_bits = (1<<3);
        sprite_pending_bits |= (1<<3);
        break;
    case 2:
        sprite_halt_bits = (1<<4);
        sprite_pending_bits |= (1<<4);
        break;
    case 4:
        sprite_halt_bits = (1<<5);
        sprite_pending_bits |= (1<<5);
        break;
    case 6:
        sprite_halt_bits = (1<<6);
        sprite_pending_bits |= (1<<6);
        break;
    case 8:
        sprite_halt_bits = (1<<7);
        sprite_pending_bits |= (1<<7);
        break;
    }
}

static DRAW_INLINE void update_sprite_flags7(int cycle)
{
    sprite_mc_bits = vicii.regs[0x1c];
    sprite_expx_bits = vicii.regs[0x1d];
}

static DRAW_INLINE void update_sprite_flags0(int cycle)
{
    int s;
    for (s=0; s<8; s++) {
        sprite_x_pipe[s] = sprite_x_pipe0[s];
        sprite_x_pipe0[s] = vicii.sprite[s].x;
    }
}


void vicii_draw_cycle(void)
{
    int cycle, offs, i, s;
    int xpos;
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

    {
        BYTE bg, csel;
        bg = vicii.regs[0x21];
        csel = vicii.regs[0x16] & 0x8;

        /* render pixels */
        for (i = 0; i < 8; i++) {
            BYTE px;
            BYTE c[4];
            int pixel_pri;
           
            /* pipe sprite related changes 2 pixels late */
            if (i == 4) {
                update_sprite_flags4(cycle);
            }
            if (i == 7) {
                update_sprite_flags7(cycle);
            }

            if (i == 0) {
                update_sprite_flags0(cycle);
            }

            /* Load new gbuf/vbuf/cbuf values at offset == xscroll */
            if (i == xscroll_pipe) {
                /* latch values at time xs */
                vbuf_reg = vbuf_pipe1_reg;
                cbuf_reg = cbuf_pipe1_reg;
                gbuf_reg = gbuf_pipe1_reg;
                gbuf_mc_flop = 0;
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


            c[0] = bg;

            /* setup colors and read pixels depending on video mode */
            switch (vmode_pipe) {

            case VICII_NORMAL_TEXT_MODE:
                c[3] = cbuf_reg;

                px = (gbuf_reg & 0x80) ? 3 : 0;
                break;

            case VICII_MULTICOLOR_TEXT_MODE:
                c[1] = vicii.regs[0x22];
                c[2] = vicii.regs[0x23];
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
                c[0] = vicii.regs[0x21 + (vbuf_reg >> 6)];
                c[3] = cbuf_reg;

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

            /* Determine pixel color and priority */
            current_pixel = c[px];
            pixel_pri = (px & 0x2);

            /* shift the graphics buffer */
            gbuf_reg <<= 1;
            gbuf_mc_flop = ~gbuf_mc_flop;
            
            /* process sprites */
            draw_sprites(xpos + i, pixel_pri);

            /* add border on top */
            if (border_state) {
                current_pixel = vicii.regs[0x20];
            }

            /* draw pixel to buffer */
            vicii.dbuf[offs + i] = current_pixel;

        }

    }

    vicii.dbuf_offset += 8;


    /* shift and put the next data into the pipe. */
    vbuf_pipe1_reg = vbuf_pipe0_reg;
    cbuf_pipe1_reg = cbuf_pipe0_reg;
    gbuf_pipe1_reg = gbuf_pipe0_reg;

    /* this makes sure gbuf is 0 outside the visible area
       I should probably be done somewhere around the fetch instead */
    if ( (cycle >= 14 && cycle <= 53) ) {
        gbuf_pipe0_reg = vicii.gbuf;
    } else {
        gbuf_pipe0_reg = 0;
    }

    /* Only update vbuf and cbuf registers in the display state. */
    if ( cycle >= 14 && cycle <= 53 ) {
        if (!vicii.idle_state) {
            vbuf_pipe0_reg = vicii.vbuf[cycle - 14];
            cbuf_pipe0_reg = vicii.cbuf[cycle - 14];
        } else {
            vbuf_pipe0_reg = 0;
            cbuf_pipe0_reg = 0;
        }
    } 

    main_border_pipe = vicii.main_border;

    idle_state_pipe = vicii.idle_state;
    vmode_pipe = vicii.video_mode;
    xscroll_pipe = vicii.regs[0x16] & 0x07;

}


void vicii_draw_cycle_init(void)
{
    memset(vicii.dbuf, 0, VICII_DRAW_BUFFER_SIZE);
    vicii.dbuf_offset = 0;
}

