/*
 * fliplist.h
 *
 * Written by
 *  Martin Pottendorfer <Martin.Pottendorfer@aut.alcatel.at>
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

#ifndef _FLIPLIST_H
#define _FLIPLIST_H

extern void flip_set_current(int unit, const char *image);
extern void flip_add_image(int unit);
extern void flip_remove(int unit, char *image);
extern void flip_attach_head(int unit, int direction);
extern void *flip_init_iterate(int unit);
extern void *flip_next_iterate(int unit);
extern char *flip_get_head(int unit);
extern char *flip_get_next(int unit);
extern char *flip_get_prev(int unit);
extern char *flip_get_image(void *fl);
extern int flip_get_unit(void *fl);

#endif /* _FLIPLIST_H */

