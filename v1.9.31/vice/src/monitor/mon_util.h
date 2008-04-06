/*
 * mon_util.h - Utilities for the VICE built-in monitor.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
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

#ifndef _MON_UTIL_H
#define _MON_UTIL_H

#include "mon.h"
#include "types.h"

struct console_s;

extern char *mon_disassemble_with_label(MEMSPACE memspace, ADDRESS loc,
                                        int hex, unsigned *opc_size_p,
                                        unsigned *label_p);
extern void mon_set_command(struct console_s *console_log, char *command,
                            void (*)(void));

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)

extern int mon_out(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

#else

extern int mon_out(const char *format, ...);

#endif

#endif /* #ifndef _MON_UTIL_H */

