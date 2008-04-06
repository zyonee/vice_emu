/*
 * fullscreen.h
 *
 * Written by
 *  Oliver Schaertel <orschaer@forwiss.uni-erlangen.de>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _FULLSCREEN_H
#define _FULLSCREEN_H

#include "resources.h"

struct video_frame_buffer_s;

extern int fullscreen_is_enabled;
extern char *fullscreen_selected_videomode;
extern char *fullscreen_selected_videomode_at_start;
extern int fullscreen_width, fullscreen_height;
extern void fullscreen_mode_init(void);
extern void fullscreen_update(void);

extern int fullscreen_mode_on(void);
extern int fullscreen_mode_off(void);
extern void fullscreen_mode_on_restore(void);
extern void fullscreen_mode_off_restore(void);

extern int fullscreen_available(void);
extern int fullscreen_vidmode_available(void);
extern int fullscreen_available_modes(void);
extern char *fullscreen_mode_name(int);
extern int fullscreen_set_bestmode(resource_value_t, void*);
extern int fullscreen_set_mode(resource_value_t, void*);
extern void fullscreen_refresh_func(struct video_frame_buffer_s *f,
                                    int src_x, int src_y,
                                    int dest_x, int dest_y,
                                    unsigned int width, unsigned int height);

extern void fullscreen_mode_exit(void);

#endif

