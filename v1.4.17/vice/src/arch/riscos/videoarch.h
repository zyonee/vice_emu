/*
 * videoarch.h - RISC OS graphics routines.
 *
 * Written by
 *  Andreas Dehmel <dehmel@forwiss.tu-muenchen.de>
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

#ifndef _VIDEOARCH_H
#define _VIDEOARCH_H


#include "vice.h"

#include "palette.h"
#include "types.h"
#include "ui.h"


struct video_frame_buffer_s {
  int width, height;
  PIXEL *tmpframebuffer;
  unsigned int tmpframebufferlinesize;
};

typedef struct video_frame_buffer_s video_frame_buffer_t;

typedef PIXEL *frame_buffer_ptr_t;
typedef ui_exposure_handler_t canvas_redraw_t;

struct canvas_s {
  unsigned int width, height;
  unsigned int scale;
  int shiftx, shifty;
  RO_Window *window;
  video_frame_buffer_t fb;
  PIXEL *pixel_translation;
  unsigned int num_colours;
  unsigned int colour_table[256];
};

typedef struct canvas_s canvas_t;

typedef struct canvas_list_t {
  canvas_t *canvas;
  struct canvas_list_t *next;
} canvas_list_t;


#define VIDEO_FRAME_BUFFER_POINTER_FIXUP(x) ((x))
#define VIDEO_FRAME_BUFFER_START(i)         ((i)->tmpframebuffer)
#define VIDEO_FRAME_BUFFER_LINE_SIZE(i)     ((i)->tmpframebufferlinesize)
#define VIDEO_FRAME_BUFFER_LINE_START(i, n) ((i)->tmpframebuffer \
                                            + (n) * (i)->tmpframebufferlinesize)

#define CANVAS_USES_TRIPLE_BUFFERING(c) 0


extern void canvas_mode_change(void);
extern canvas_t *canvas_for_handle(int handle);
extern void canvas_next_active(int moveCaret);
extern int canvas_get_number(void);

extern int video_full_screen_on(int *sprites);
extern int video_full_screen_off(void);
extern int video_full_screen_refresh(void);
extern void video_full_screen_speed(int percent, int framerate, int warp);
extern void video_full_screen_drive_leds(unsigned int drive);
extern void video_full_screen_init_status(void);
extern void video_full_screen_plot_status(void);
extern void video_full_screen_display_image(unsigned int num, const char *img);

extern canvas_list_t *CanvasList;
extern canvas_t *ActiveCanvas;

extern int FullScreenMode;
extern int FullScreenStatLine;

#endif
