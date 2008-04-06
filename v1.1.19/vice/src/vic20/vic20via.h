/*
 * via.h - VIA emulation.
 *
 * Written by
 *   Andre' Fachat <fachat@physik.tu-chemnitz.de>
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

#ifndef _VIC20_VIA_H
#define _VIC20_VIA_H

#include "snapshot.h"
#include "types.h"

extern void via1_init(void);
extern void via1_reset(void);
extern void via1_signal(int line, int edge);
extern void REGPARM2 via1_store(ADDRESS addr, BYTE byte);
extern BYTE REGPARM1 via1_read(ADDRESS addr);
extern BYTE REGPARM1 via1_peek(ADDRESS addr);
extern int via1_write_snapshot_module(snapshot_t *p);
extern int via1_read_snapshot_module(snapshot_t *p);

extern void via2_init(void);
extern void via2_reset(void);
extern void via2_signal(int line, int edge);
extern void REGPARM2 via2_store(ADDRESS addr, BYTE byte);
extern BYTE REGPARM1 via2_read(ADDRESS addr);
extern BYTE REGPARM1 via2_peek(ADDRESS addr);
extern int via2_write_snapshot_module(snapshot_t *p);
extern int via2_read_snapshot_module(snapshot_t *p);
extern void via2_set_tape_sense(int v);

#endif  /* _VIC20_VIA_H */

