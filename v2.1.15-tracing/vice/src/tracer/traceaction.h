/* traceaction.h - Trace Probe interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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


#ifndef VICE_TRACEACTION_H
#define VICE_TRACEACTION_H

#include "types.h"

struct trace_action_s;

typedef int (*trace_action_call_f)(struct trace_action_s *);

struct trace_action_args_mem_dump_s
{
    WORD                   offset;
    WORD                   size;
    BYTE                   bank;
    BYTE                   space;
};

struct trace_action_s 
{
    struct trace_action_s *next;
    trace_action_call_f    call;
    union {
        struct trace_action_args_mem_dump_s     mem_dump;
    } args;
};

typedef struct trace_action_s trace_action_t;

extern trace_action_t *trace_action_mem_dump(WORD offset, WORD size, BYTE bank, BYTE space);
extern void trace_action_free(trace_action_t *action);

/* ----- Action IDs ----- */
#define ACTION_ID_MEMDUMP       1

#endif
