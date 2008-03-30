/*
 * autostart.c - automatic image loading and starting
 *
 * Written by
 *  Teemu Rantanen      (tvr@cs.hut.fi)
 *  Ettore Perazzoli	(ettore@comm2000.it)
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

#ifndef _AUTOSTART_H
#define _AUTOSTART_H

#ifdef AUTOSTART

#define AUTOSTART_NONE			0
#define AUTOSTART_ERROR			1
#define AUTOSTART_HASTAPE		100
#define AUTOSTART_LOADINGTAPE		101
#define AUTOSTART_HASDISK		800
#define AUTOSTART_LOADINGDISK		801
#define AUTOSTART_DONE			9999

void autostart_init(void);
void autostart_advance(void);
int autostart_tape(const char *file_name, const char *program_name);
int autostart_disk(const char *file_name, const char *program_name);
int autostart_autodetect(const char *file_name, const char *program_name);
int autostart_device(int num);
void autostart_reset(void);

#endif /* AUTOSTART */

#endif /* !_AUTOSTART_H */
