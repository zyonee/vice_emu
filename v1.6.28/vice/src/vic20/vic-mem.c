/*
 * vic-mem.c - Memory interface for the VIC-I emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "maincpu.h"
#include "types.h"
#include "vic.h"
#include "vic-mem.h"
#include "vic20mem.h"
#include "vic20sound.h"



/* VIC access functions. */

void REGPARM2 
vic_store(ADDRESS addr, BYTE value)
{
    addr &= 0xf;
    vic.regs[addr] = value;
    VIC_DEBUG_REGISTER (("VIC: write $90%02X, value = $%02X.", addr, value));

    if (clk >= vic.draw_clk)
        vic_raster_draw_alarm_handler (clk - vic.draw_clk);


    switch (addr)
    {
    case 0:                     /* $9000  Screen X Location. */
        /* 
            VIC checks in cycle n for peek($9000)=n 
            and in this case opens the horizontal flipflop
        */
        {
            int xstart, xstop;
            
            value &= 0x7f;

            xstart = value * 4;
            
            xstop = xstart + vic.text_cols * 8;
            if (xstop >= vic.screen_width)
                xstop = vic.screen_width - 1; /* FIXME: SCREEN-MIXUP not handled */

            if (vic.raster.display_xstart < (signed int)VIC_RASTER_CYCLE(clk) * 4
                && !vic.raster.blank_this_line)
            {
                /* the line has already started */
                raster_add_int_change_next_line (&vic.raster,
                    &vic.raster.display_xstart, xstart);
            
                raster_add_int_change_next_line (&vic.raster,
                    (int *)(&vic.raster.geometry.gfx_position.x), xstart);
            } else {
                vic.raster.display_xstart = xstart;
                
                vic.raster.display_xstop = xstop;
                
                vic.raster.geometry.gfx_position.x = xstart;
                
                /* the line may not start, if new xstart is already passed */
                vic.raster.blank_this_line = (value < VIC_RASTER_CYCLE(clk));
            }
            return;
        }

    case 1:                     /* $9001  Screen Y Location. */
        /*
            VIC checks from cycle 1 of line r*2 to cycle 0 of line r*2+2
            if peek($9001)=r is true and in this case it opens the vertical flipflop
        */
        {
            unsigned int ystart;
            ystart = value * 2;

            /* at least the next frame will start at the new ystart */
            vic.pending_ystart = ystart;

            if ((vic.raster.display_ystart > VIC_RASTER_Y(clk - 2))
                || vic.raster.display_ystart < 0)
            {
                /* the vertical flipflop isn't open yet */
                if (ystart / 2 < VIC_RASTER_Y(clk - 1) / 2)
                {
                    /* new ystart is already passed, vertical flipflop may not open this frame */
                    vic.raster.display_ystart = vic.raster.display_ystop = -1;
                } 
                else if (ystart / 2 > VIC_RASTER_Y(clk - 1) / 2)
                {
                    /* the frame starts later */
                    vic.raster.display_ystart = ystart;

                    vic.raster.display_ystop = ystart
                        + vic.text_lines * vic.char_height;
                    
                    vic.raster.geometry.gfx_position.y = ystart - vic.first_displayed_line;
                }
                else
                {
                    /* the frame starts somewhere here */
                    vic.raster.display_ystart = VIC_RASTER_Y(clk);
                    
                    vic.raster.display_ystop = vic.raster.display_ystart
                        + vic.text_lines * vic.char_height;
                    
                    vic.raster.geometry.gfx_position.y = VIC_RASTER_Y(clk)
                        - vic.first_displayed_line;

                    if (vic.raster.display_xstart < (signed int)VIC_RASTER_CYCLE(clk))
                    {
                        /* too late to start in current line */
                        vic.raster.blank_this_line = 1;
                    }
                }
            }
            return;
        }

    case 2:                     /* $9002  Columns Displayed. */
        {
            int new_text_cols = MIN(value & 0x7f, VIC_SCREEN_MAX_TEXT_COLS);
            
            int new_xstop = MIN(vic.raster.display_xstart + new_text_cols * 8, 
                vic.screen_width - 1);
            
            if (VIC_RASTER_CYCLE(clk) <= 1)
            {
                /* changes up to cycle 1 are visible in the current line */
                vic.text_cols = new_text_cols;
                
                vic.raster.display_xstop = new_xstop;
                
                vic.raster.geometry.gfx_size.width = new_text_cols * 8;
                
                vic.raster.geometry.text_size.width = new_text_cols;
            } else {
                /* later changes are visible in the next line */
                raster_add_int_change_next_line (&vic.raster,
                    (int *)(&vic.text_cols), new_text_cols);
                
                raster_add_int_change_next_line (&vic.raster,
                    &vic.raster.display_xstop, new_xstop);
                
                raster_add_int_change_next_line (&vic.raster,
                    (int *)(&vic.raster.geometry.gfx_size.width),
                    new_text_cols * 8);
                
                raster_add_int_change_next_line (&vic.raster,
                    (int *)(&vic.raster.geometry.text_size.width),
                    new_text_cols);
            }
        }

        vic_update_memory_ptrs ();

        return;

    case 3:                     /* $9003  Rows Displayed, Character size . */
        {
            static int old_char_height = -1;

            int new_text_lines = MIN((value & 0x7e) >> 1, VIC_SCREEN_MAX_TEXT_LINES);
            
            int new_char_height = (value & 0x1) ? 16 : 8;

            if (VIC_RASTER_Y(clk) == 0 && VIC_RASTER_CYCLE(clk) <= 2)
            {
                /* changes up to cycle 2 of rasterline 0 are visible in current frame */
                vic.text_lines = new_text_lines;
            
                vic.raster.geometry.gfx_size.height = new_text_lines * 8;
                
                vic.raster.geometry.text_size.height = new_text_lines;

            } else {
                /* later changes are visible in the next frame */
                vic.pending_text_lines = new_text_lines;
            }


            if (old_char_height != new_char_height)
            {
                if (VIC_RASTER_CYCLE(clk) >= 1)
                {
                    raster_add_int_change_next_line (&vic.raster,
                        (int *)(&vic.row_increase_line), new_char_height);
                } else {
                    vic.row_increase_line = new_char_height;
                }

                /* this is quite strange */
                if (vic.raster.ycounter == 7 && !vic.raster.blank_this_line)
                {
                    if (old_char_height == 8 && new_char_height == 16)
                    {
                        vic.row_offset =
                            (VIC_RASTER_CYCLE(clk) - vic.raster.display_xstart / 4 - 3) / 2;
                    } else {
                        vic.row_offset = -1; /* use vic.text_cols for offset */
                    }
                }
                raster_add_int_change_foreground (&vic.raster,
                    VIC_RASTER_CHAR(VIC_RASTER_CYCLE(clk) + 2),
                    &vic.char_height,
                    new_char_height);

                old_char_height = new_char_height;
            }
        }

      return;

    case 4:                     /* $9004  Raster line count -- read only. */
      return;

    case 5:                     /* $9005  Video and char matrix base
                                   address. */
      vic_update_memory_ptrs ();
      return;

    case 6:                     /* $9006. */
    case 7:                     /* $9007  Light Pen X,Y. */
      VIC_DEBUG_REGISTER (("(light pen register, read-only)."));
      return;

    case 8:                     /* $9008. */
    case 9:                     /* $9009  Paddle X,Y. */
      return;

    case 10:                    /* $900A  Bass Enable and Frequency. */
    case 11:                    /* $900B  Alto Enable and Frequency. */
    case 12:                    /* $900C  Soprano Enable and Frequency. */
    case 13:                    /* $900D  Noise Enable and Frequency. */
        store_vic_sound (addr, value);
        return;

    case 14:                    /* $900E  Auxiliary Colour, Master Volume. */
        /*
            changes of auxiliary color in cycle n is visible at pixel 4*(n-7)
        */
        {
            static int old_aux_color = -1;
            int new_aux_color = value>>4;

            if (new_aux_color != old_aux_color)
            {
                raster_add_int_change_foreground (&vic.raster,
                    VIC_RASTER_CHAR(VIC_RASTER_CYCLE(clk)),
                    &vic.auxiliary_color,
                    new_aux_color);


                old_aux_color = new_aux_color;
            }
        }

        store_vic_sound (addr, value);
        return;

    case 15:                    /* $900F  Screen and Border Colors,
                                   Reverse Video. */
        /*
            changes of border/background in cycle n are visible at pixel 4*(n-7),
            changes of reverse mode at pixel 4*(n-7)+3, which is quite ugly 'cause
            we're using a character-based emulation :(FIXME)
        */
        {
            static int old_video_mode = -1;
            static int old_background_color = -1;
            static int old_border_color = -1;
            int new_video_mode, new_background_color, new_border_color;
        
            new_background_color = value>>4;
            new_border_color = value & 0x7;
            new_video_mode = ((value & 0x8) ? VIC_STANDARD_MODE : VIC_REVERSE_MODE);

            if (new_background_color != old_background_color)
            {
                raster_add_int_change_background (&vic.raster,
                    VIC_RASTER_X(VIC_RASTER_CYCLE(clk)),
                    &vic.raster.background_color,
                    new_background_color);

                old_background_color = new_background_color;
            }

            if (new_border_color != old_border_color)
            {
                raster_add_int_change_border (&vic.raster,
                    VIC_RASTER_X(VIC_RASTER_CYCLE(clk)),
                    &vic.raster.border_color,
                    new_border_color);

                /* we also need the border color in multicolor mode, so we duplicate it */
                raster_add_int_change_foreground (&vic.raster,
                    VIC_RASTER_CHAR(VIC_RASTER_CYCLE(clk)),
                    &vic.mc_border_color,
                    new_border_color);


                old_border_color = new_border_color;
            }

            if (new_video_mode != old_video_mode)
            {
                raster_add_int_change_foreground (&vic.raster,
                    VIC_RASTER_CHAR(VIC_RASTER_CYCLE(clk)),
                    &vic.raster.video_mode,
                    new_video_mode);

                old_video_mode = new_video_mode;
            }
            
            return;
        }
    }
}



BYTE REGPARM1 
vic_read(ADDRESS addr)
{
  addr &= 0xf;

  switch (addr)
    {
    case 3:
      return ((VIC_RASTER_Y (clk + vic.cycle_offset) & 1) << 7) | (vic.regs[3] & ~0x80);
    case 4:
      return VIC_RASTER_Y (clk + vic.cycle_offset) >> 1;
    default:
      return vic.regs[addr];
    }
}

