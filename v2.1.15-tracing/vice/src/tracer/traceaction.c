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
    lib_free(action);
}

/* ----- Action: Mem Dump ----- 

   message (6 bytes):
       +0:      <word> offset
       +2:      <word> size
       +4:      <byte> bank
       +5:      <byte> space
       +6:      ... size bytes data ...

*/

#define MEMDUMP_HEADER_SIZE     6

/* execute mem dump aciton */
static int do_mem_dump(trace_action_t *action)
{
    WORD offset = action->args.mem_dump.offset;
    WORD size   = action->args.mem_dump.size;
    BYTE bank   = action->args.mem_dump.bank;
    BYTE space  = action->args.mem_dump.space;
    WORD i;

    /* get data buffer for action */
    BYTE *buffer = trace_server_request_buffer(ACTION_ID_MEMDUMP,size + MEMDUMP_HEADER_SIZE);
    if(buffer == NULL) {
        log_message(LOG_DEFAULT, "trace action: 'memory dump' too large!");
        return -1;
    }
    
    WORD *ptr = (WORD *)buffer;
    ptr[0] = htons(offset);
    ptr[1] = htons(size);
    buffer[4] = bank;
    buffer[5] = space;
    buffer += MEMDUMP_HEADER_SIZE;
    
    /* fill in data */
    for(i=0;i<size;i++) {
        buffer[i] = mon_get_mem_val_ex(space, bank, offset+i);
    }

    return 0;
}

/* create mem dump action */
trace_action_t *trace_action_mem_dump(WORD offset, WORD size, BYTE bank, BYTE space)
{
    trace_action_t *action = action_create();
    if(action == NULL) {
        return NULL;
    }
    
    action->call = do_mem_dump;
    action->args.mem_dump.offset = offset;
    action->args.mem_dump.size   = size;
    action->args.mem_dump.bank   = bank;
    action->args.mem_dump.space  = space;
    
    return action;
}

