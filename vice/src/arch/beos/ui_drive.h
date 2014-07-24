/*
 * ui_drive.h - Drive settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE__UI_DRIVE_H__
#define VICE__UI_DRIVE_H__

typedef struct drive_type_s {
    const char *name;
    int id;
} drive_type_t;

extern drive_type_t drive_type[];
extern int drive_machine_parallel_capable;

const uint32 MESSAGE_DRIVE_TYPE              = 'MD01';
const uint32 MESSAGE_DRIVE_EXTENDIMAGEPOLICY = 'MD02';
const uint32 MESSAGE_DRIVE_IDLEMETHOD        = 'MD03';
const uint32 MESSAGE_DRIVE_EXPANSION         = 'MD04';
const uint32 MESSAGE_DRIVE_PARALLELCABLE     = 'MD05';

extern void ui_drive(void);

#endif
