/*
 * maincpu.h - Emulation of the main 6510 processor.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef _MAINCPU_H
#define _MAINCPU_H

#include "6510core.h"
#include "types.h"

/* ------------------------------------------------------------------------- */

/* Information about the last opcode executed by the main CPU.  */
extern DWORD last_opcode_info;

/* The lowest 8 bits are the opcode number.  */
#define OPINFO_NUMBER_MASK	       	(last_opcode_info & 0xff)

/* If this is set to 1, the opcode has delayed interrupts by one more cycle
   (this happens with conditional jumps when jump is taken).  */
#define OPINFO_DELAYS_INTERRUPT_MASK	(last_opcode_info & 0x100)

/* The VIC-II emulation needs this ugly hack.  */
#define EXTERN_PC
extern unsigned int reg_pc;

struct mos6510_regs_s;
extern struct mos6510_regs_s maincpu_regs;

struct monitor_interface_s;
extern struct monitor_interface_s maincpu_monitor_interface;

extern int rmw_flag;
extern CLOCK clk;

/* ------------------------------------------------------------------------- */

struct alarm_context_s;
struct snapshot_s;
struct clk_guard_s;

extern CLOCK _maincpu_opcode_write_cycles[];
extern struct alarm_context_s maincpu_alarm_context;
extern struct clk_guard_s maincpu_clk_guard;

/* Return the number of write accesses in the last opcode emulated. */
inline static CLOCK maincpu_num_write_cycles(void)
{
    return _maincpu_opcode_write_cycles[OPINFO_NUMBER(last_opcode_info)];
}

extern void maincpu_init(void);
extern void maincpu_reset(void);
extern void mainloop(ADDRESS start_address);
extern int maincpu_read_snapshot_module(struct snapshot_s *s);
extern int maincpu_write_snapshot_module(struct snapshot_s *s);

#endif

