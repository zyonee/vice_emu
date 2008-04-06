
/*
 * ciad.h - Drive CIA definitions.
 *
 * Written by
 *  Andreas Boose (boose@linux.rz.fh-hannover.de)
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

#ifndef _CIAD_H
#define _CIAD_H

#include "types.h"
#include "cia.h"

extern void reset_cia1571d0(void);
extern void REGPARM2 store_cia1571d0(ADDRESS addr, BYTE value);
extern BYTE REGPARM1 read_cia1571d0(ADDRESS addr);
extern BYTE REGPARM1 peek_cia1571d0(ADDRESS addr);
extern int int_cia1571d0ta(long offset);
extern int int_cia1571d0tb(long offset);
extern int int_cia1571d0tod(long offset);
extern void cia1571d0_prevent_clk_overflow(CLOCK sub);
extern void cia1571d0_set_flag(void);
extern void cia1571d0_set_sdr(BYTE received_byte);

extern void reset_cia1571d1(void);
extern void REGPARM2 store_cia1571d1(ADDRESS addr, BYTE value);
extern BYTE REGPARM1 read_cia1571d1(ADDRESS addr);
extern BYTE REGPARM1 peek_cia1571d1(ADDRESS addr);
extern int int_cia1571d1ta(long offset);
extern int int_cia1571d1tb(long offset);
extern int int_cia1571d1tod(long offset);
extern void cia1571d1_prevent_clk_overflow(CLOCK sub);
extern void cia1571d1_set_flag(void);
extern void cia1571d1_set_sdr(BYTE received_byte);
extern void reset_cia1571d0(void);

extern void REGPARM2 store_cia1581d0(ADDRESS addr, BYTE value);
extern BYTE REGPARM1 read_cia1581d0(ADDRESS addr);
extern BYTE REGPARM1 peek_cia1581d0(ADDRESS addr);
extern int int_cia1581d0ta(long offset);
extern int int_cia1581d0tb(long offset);
extern int int_cia1581d0tod(long offset);
extern void cia1581d0_prevent_clk_overflow(CLOCK sub);
extern void cia1581d0_set_flag(void);
extern void cia1581d0_set_sdr(BYTE received_byte);
extern void reset_cia1581d0(void);

extern void REGPARM2 store_cia1581d1(ADDRESS addr, BYTE value);
extern BYTE REGPARM1 read_cia1581d1(ADDRESS addr);
extern BYTE REGPARM1 peek_cia1581d1(ADDRESS addr);
extern int int_cia1581d1ta(long offset);
extern int int_cia1581d1tb(long offset);
extern int int_cia1581d1tod(long offset);
extern void cia1581d1_prevent_clk_overflow(CLOCK sub);
extern void cia1581d1_set_flag(void);
extern void cia1581d1_set_sdr(BYTE received_byte);
extern void reset_cia1581d1(void);

#endif				/* _CIAD_H */
