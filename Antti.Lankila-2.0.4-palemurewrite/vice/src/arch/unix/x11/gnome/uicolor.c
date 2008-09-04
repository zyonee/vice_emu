/*
 * uicolor.c - X11 color routines.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdlib.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "color.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "types.h"
#include "uicolor.h"
#include "video.h"
#include "videoarch.h"


extern int screen;
extern GdkColor drive_led_on_red_pixel, drive_led_on_green_pixel,
drive_led_off_pixel, motor_running_pixel, tape_control_pixel;
extern GdkColor drive_led_on_red_pixels[16];
extern GdkColor drive_led_on_green_pixels[16];

void uicolor_init_video_colors();

/*-----------------------------------------------------------------------*/

int uicolor_alloc_colors(video_canvas_t *c)
{
    int i;

    drive_led_off_pixel.red = 0;
    drive_led_off_pixel.green = 0;
    drive_led_off_pixel.blue = 0;

    drive_led_on_red_pixel.red = 0xff00;
    drive_led_on_red_pixel.green = 0;
    drive_led_on_red_pixel.blue = 0;

    drive_led_on_green_pixel.red = 0;
    drive_led_on_green_pixel.green = 0xff00;
    drive_led_on_green_pixel.blue = 0;

    motor_running_pixel.red = 0xff00;
    motor_running_pixel.green = 0xff00;
    motor_running_pixel.blue = 0x7f00;

    tape_control_pixel.red = 0xaf00;
    tape_control_pixel.green = 0xaf00;
    tape_control_pixel.blue = 0xaf00;

    /* different colors intensities for drive leds */
    for (i = 0; i < 16; i++)
    {
	drive_led_on_red_pixels[i].red = 0x1000*i + 0xf00;
	drive_led_on_red_pixels[i].green = 0;
	drive_led_on_red_pixels[i].blue = 0;

	drive_led_on_green_pixels[i].red = 0;
	drive_led_on_green_pixels[i].green =  0x1000*i + 0xf00;
	drive_led_on_green_pixels[i].blue = 0;
    }
    
    return 0;
}

unsigned int endian_swap(unsigned int color, unsigned int bpp, unsigned int swap) {
    if (! swap)
        return color;

    if (bpp == 8)
        return color;

    if (bpp == 16)
        return ((color >> 8) & 0x00ff)
             | ((color << 8) & 0xff00);

    if (bpp == 32)
        return ((color >> 24) & 0x000000ff)
             | ((color >>  8) & 0x0000ff00)
             | ((color <<  8) & 0x00ff0000)
             | ((color << 24) & 0xff000000);
    
    /* err? */
    return color;
}

int uicolor_set_palette(struct video_canvas_s *c, const palette_t *palette)
{
    unsigned int i, rs, gs, bs, redbits, grnbits, blubits, swap = 0;

#ifdef WORDS_BIGENDIAN
    if (c->gdk_image->byte_order == GDK_LSB_FIRST)
#else
    if (c->gdk_image->byte_order == GDK_MSB_FIRST)
#endif
    {
        swap = 1;
    }

    /* I don't support indexed palettes. */
    if (c->gdk_image->depth == 16) {
        redbits = 5;
        grnbits = 6;
        blubits = 5;
    } else if (c->gdk_image->depth == 15) {
        /* When I tested this on ATI Radeon Mobility 7500 the desktop had
         * false colours, so not even GNOME really supports this. Also,
         * the color order was not rgb, but more like brg... */
        redbits = 5;
        grnbits = 5;
        blubits = 5;
    } else if (c->gdk_image->bits_per_pixel == 24
               || c->gdk_image->bits_per_pixel == 32) {
        redbits = 8;
        grnbits = 8;
        blubits = 8;
    } else {
        /* whoops. What is this mode? */
        log_error(LOG_ERR, "Sorry. I don't know how to handle your video mode colours with %d depth and %d bits per pixel. At least 8-bit pseudocolor modes are not supported by GNOMEUI. 15, 16, 24 and 32, however, should work.", c->gdk_image->depth, c->gdk_image->bits_per_pixel);
        exit(1);
    }

    rs = grnbits + blubits;
    gs = blubits;
    bs = 0;

    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        DWORD color_pixel = endian_swap(
            color.red   >> (8-redbits) << rs |
            color.green >> (8-grnbits) << gs |
            color.blue  >> (8-blubits) << bs,
            c->gdk_image->bits_per_pixel,
            swap
        );
        video_render_setphysicalcolor(c->videoconfig, i, color_pixel,
                                      c->gdk_image->bits_per_pixel);
    }
    
    for (i = 0; i < 256; i++) {
        video_render_setrawrgb(i, 
            endian_swap(i >> (8-redbits) << rs, c->gdk_image->bits_per_pixel, swap),
            endian_swap(i >> (8-grnbits) << gs, c->gdk_image->bits_per_pixel, swap),
            endian_swap(i >> (8-blubits) << bs, c->gdk_image->bits_per_pixel, swap)
        );
    }
    
    video_render_initraw();

    return 0;
}
