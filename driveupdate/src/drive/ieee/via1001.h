/*
 * via1001.h - Drive 1001 VIA definitions.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#ifndef VICE_VIA1001_H
#define VICE_VIA1001_H

#include "types.h"

struct drive_context_s;
struct via_context_s;

extern void via1001_setup_context(struct drive_context_s *ctxptr);

extern void via1001_init(struct drive_context_s *ctxptr);
extern void via1001_store(struct drive_context_s *ctxptr, WORD addr,
                                     BYTE byte);
extern BYTE via1001_read(struct drive_context_s *ctxptr, WORD addr);
extern BYTE via1001_peek(struct drive_context_s *ctxptr, WORD addr);

#endif

