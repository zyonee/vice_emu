/*
 * memutils.c -- Miscellaneous memory-related functions.
 *
 * Written by
 *  Jarkko Sonninen (sonninen@lut.fi)
 *  Teemu Rantanen (tvr@cs.hut.fi)
 *  Jouko Valta (jopi@stekt.oulu.fi)
 *  Andre' Fachat (fachat@physik.tu-chemnitz.de)
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#ifndef _MEMUTILS_H
#define _MEMUTILS_H

#include "types.h"

extern int mem_load_sys_file(const char *name, BYTE *dest,
                             int minsize, int maxsize);

#endif
