
/*
 * ciad.h - Drive CIA definitions.
 *
 * Written by
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

#ifndef _CIAD_H
#define _CIAD_H

#include "types.h"

struct drive_context_s;
struct drivecia_context_s;
struct snapshot_s;

extern void cia1571_setup_context(struct drive_context_s *ctxptr);
extern void cia1581_setup_context(struct drive_context_s *ctxptr);

extern void cia1571_init(struct drive_context_s *ctxptr);
extern void REGPARM3 cia1571_store(struct drive_context_s *ctxptr, ADDRESS addr, BYTE value);
extern BYTE REGPARM2 cia1571_read(struct drive_context_s *ctxptr, ADDRESS addr);
extern BYTE REGPARM2 cia1571_peek(struct drive_context_s *ctxptr, ADDRESS addr);
extern void cia1571_prevent_clk_overflow(struct drive_context_s *ctxptr, CLOCK sub);
extern void cia1571_set_flag(struct drive_context_s *ctxptr);
extern void cia1571_set_sdr(struct drive_context_s *ctxptr, BYTE received_byte);
extern void cia1571_reset(struct drive_context_s *ctxptr);
extern int cia1571_write_snapshot_module(struct drive_context_s *ctxptr,
                                         struct snapshot_s *p);
extern int cia1571_read_snapshot_module(struct drive_context_s *ctxptr,
                                        struct snapshot_s *p);

extern void cia1581_init(struct drive_context_s *ctxptr);
extern void REGPARM3 cia1581_store(struct drive_context_s *ctxptr, ADDRESS addr, BYTE value);
extern BYTE REGPARM2 cia1581_read(struct drive_context_s *ctxptr, ADDRESS addr);
extern BYTE REGPARM2 cia1581_peek(struct drive_context_s *ctxptr, ADDRESS addr);
extern void cia1581_prevent_clk_overflow(struct drive_context_s *ctxptr,
                                         CLOCK sub);
extern void cia1581_set_flag(struct drive_context_s *ctxptr);
extern void cia1581_set_sdr(struct drive_context_s *ctxptr, BYTE received_byte);
extern void cia1581_reset(struct drive_context_s *ctxptr);
extern int cia1581_write_snapshot_module(struct drive_context_s *ctxptr,
                                         struct snapshot_s *p);
extern int cia1581_read_snapshot_module(struct drive_context_s *ctxptr,
                                        struct snapshot_s *p);

typedef struct cia_initdesc_s {
    struct drivecia_context_s *cia_ptr;
    void (*clk)(CLOCK, void*);
    void (*int_ta)(CLOCK);
    void (*int_tb)(CLOCK);
    void (*int_tod)(CLOCK);
} cia_initdesc_t;

/* init callbacks, shared by both cias; defined in cia1571d. */
extern void cia_drive_init(struct drive_context_s *ctxptr, const cia_initdesc_t *cia_desc);

#define cia1581d0_set_flag()	cia1581_set_flag(&drive0_context)
#define cia1581d1_set_flag()	cia1581_set_flag(&drive1_context)

#endif

