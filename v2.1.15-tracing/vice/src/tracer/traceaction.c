/* traceaction.c - Trace Probe interface
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

#include <string.h>

#include "traceaction.h"
#include "traceserver.h"
#include "log.h"
#include "lib.h"
#include "montypes.h"

static trace_action_t *action_create(void)
{
    trace_action_t *action = (trace_action_t *)lib_malloc(sizeof(trace_action_t));
    if(action == NULL) {
        return NULL;
    }
    
    memset(action, 0, sizeof(trace_action_t));
    return action;
}

void trace_action_free(trace_action_t *action)
{
    if(action->buffer != NULL) {
        lib_free(action->buffer);
    }
    lib_free(action);
}

static void do_mem_dump(trace_action_t *action)
{
    DWORD offset = action->offset;
    DWORD size   = action->size;
    DWORD i;
    
    // fill buffer with dump
    for(i=0;i<size;i++) {
        action->buffer[i] = mon_get_mem_val_ex(e_comp_space, 0, offset+i);
    }
    
    trace_server_transmit(action->buffer, action->size);
}

trace_action_t *trace_action_mem_dump(DWORD offset, DWORD size)
{
    trace_action_t *action = action_create();
    if(action == NULL) {
        return NULL;
    }
    
    action->call = do_mem_dump;
    action->offset = offset;
    action->size = size;
    action->buffer = (BYTE *)lib_malloc(size);
    
    return action;
}

