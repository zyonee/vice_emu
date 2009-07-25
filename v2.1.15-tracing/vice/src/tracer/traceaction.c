/* traceaction.c - Trace Action interface
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
#include "maincpu.h"
#include "montypes.h"
#include "mos6510.h"

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
       +0:      <word> start addr
       +2:      <word> end addr
       +4:      <byte> bank
       +5:      <byte> space
       +6:      ... size bytes data ...

*/

#define MEMDUMP_HEADER_SIZE     6

/* execute mem dump aciton */
static int do_mem_dump(trace_action_t *action)
{
    WORD start  = action->args.mem_dump.start;
    WORD end    = action->args.mem_dump.end;
    BYTE bank   = action->args.mem_dump.bank;
    BYTE space  = action->args.mem_dump.space;
    DWORD i;
    WORD *ptr;
    BYTE *buffer;
    DWORD size;

    /* get data buffer for action */
    size = end - start + 1;
    buffer = trace_server_request_buffer(ACTION_ID_MEM_DUMP,size + MEMDUMP_HEADER_SIZE);
    if(buffer == NULL) {
        log_message(LOG_DEFAULT, "trace action: 'memory dump' too large!");
        return -1;
    }
    
    ptr = (WORD *)buffer;
    ptr[0] = htons(start);
    ptr[1] = htons(end);
    buffer[4] = bank;
    buffer[5] = space;
    buffer += MEMDUMP_HEADER_SIZE;
    
    /* fill in data */
    for(i=start;i<=end;i++) {
        buffer[i] = mon_get_mem_val_ex(space, bank, i);
    }

    return 0;
}

/* create mem dump action */
trace_action_t *trace_action_mem_dump(WORD start, WORD end, BYTE bank, BYTE space)
{
    trace_action_t *action = action_create();
    if(action == NULL) {
        return NULL;
    }
    
    action->call = do_mem_dump;
    action->args.mem_dump.start = start;
    action->args.mem_dump.end   = end;
    action->args.mem_dump.bank  = bank;
    action->args.mem_dump.space = space;
    
    return action;
}

/* ----- Action: Mem Map ----- */

extern BYTE *mon_memmap;

static int do_mem_map(trace_action_t *action)
{
#ifdef FEATURE_CPUMEMHISTORY
    WORD start = action->args.mem_dump.start;
    WORD end   = action->args.mem_dump.end;
    BYTE bank  = action->args.mem_dump.bank;
    WORD *ptr;
    BYTE *buffer;
    BYTE *mem_base;
    DWORD size;

    /* get data buffer for action */
    size = end - start + 1;
    buffer = trace_server_request_buffer(ACTION_ID_MEM_MAP,size + MEMDUMP_HEADER_SIZE);
    if(buffer == NULL) {
        log_message(LOG_DEFAULT, "trace action: 'memory map' too large!");
        return -1;
    }
    
    /* write mem map header (same as mem dump) */
    ptr = (WORD *)buffer;
    ptr[0] = htons(start);
    ptr[1] = htons(end);
    buffer[4] = bank;
    buffer[5] = 0;
    buffer += MEMDUMP_HEADER_SIZE;
    
    /* calc base address in memmap */
    mem_base = mon_memmap + start;
    if(bank > 0) {
        mem_base += bank << 16;
    }

    /* fill in data */
    memcpy(buffer+6, mem_base, size);
    
    /* reset memmap */
    memset(mem_base, 0, size);

    return 0;
#else
    log_message(LOG_DEFAULT, "trace action: 'memory map' not compiled in!");
    return -1;
#endif
}

/* create mem dump action */
trace_action_t *trace_action_mem_map(WORD start, WORD end, BYTE bank)
{
    trace_action_t *action = action_create();
    if(action == NULL) {
        return NULL;
    }
    
    action->call = do_mem_map;
    action->args.mem_dump.start = start;
    action->args.mem_dump.end   = end;
    action->args.mem_dump.bank  = bank;
    
    return action;
}

/* ----- Action: CPU State ----- 

    +0: <byte> pc hi/lo
    +2: <byte> stack pointer offset
    +3: <byte> status flags
    +4: <byte> A
    +5: <byte> X
    +6: <word> Y

*/

#ifndef C64DTV
#define CPUSTATE_HEADER_SIZE     8
#else
#define CPUSTATE_HEADER_SIZE     22
#endif

static int do_main_cpu_state(trace_action_t *action)
{
    BYTE *buffer;
    WORD  pc;

    buffer = trace_server_request_buffer(ACTION_ID_CPU_STATE,CPUSTATE_HEADER_SIZE);
    if(buffer == NULL) {
        log_message(LOG_DEFAULT, "trace action: 'cpu state' no memory?");
        return -1;
    }
    
    /* recreate status register: see mos6510.h */
#ifndef C64DTV
    pc = MOS6510_REGS_GET_PC(&maincpu_regs);
    buffer[0]  = (BYTE)((pc >> 8) & 0xff);
    buffer[1]  = (BYTE)(pc & 0xff);
    buffer[2]  = MOS6510_REGS_GET_SP(&maincpu_regs);
    buffer[3]  = MOS6510_REGS_GET_STATUS(&maincpu_regs);
    buffer[4]  = MOS6510_REGS_GET_A(&maincpu_regs);
    buffer[5]  = MOS6510_REGS_GET_X(&maincpu_regs);
    buffer[6]  = MOS6510_REGS_GET_Y(&maincpu_regs);
#else
    pc = MOS6510DTV_REGS_GET_PC(&maincpu_regs);
    buffer[0]  = (BYTE)((pc >> 8) & 0xff);
    buffer[1]  = (BYTE)(pc & 0xff);
    buffer[2]  = MOS6510DTV_REGS_GET_SP(&maincpu_regs);
    buffer[3]  = MOS6510DTV_REGS_GET_STATUS(&maincpu_regs);
    buffer[4]  = MOS6510DTV_REGS_GET_A(&maincpu_regs);
    buffer[5]  = MOS6510DTV_REGS_GET_X(&maincpu_regs);
    buffer[6]  = MOS6510DTV_REGS_GET_Y(&maincpu_regs);
    buffer[7]  = MOS6510DTV_REGS_GET_R3(&maincpu_regs);
    buffer[8]  = MOS6510DTV_REGS_GET_R4()&maincpu_regs);
    buffer[9]  = MOS6510DTV_REGS_GET_R5(&maincpu_regs);
    buffer[10] = MOS6510DTV_REGS_GET_R6(&maincpu_regs);
    buffer[11] = MOS6510DTV_REGS_GET_R7()&maincpu_regs);
    buffer[12] = MOS6510DTV_REGS_GET_R8(&maincpu_regs);
    buffer[13] = MOS6510DTV_REGS_GET_R9(&maincpu_regs);
    buffer[14] = MOS6510DTV_REGS_GET_R10()&maincpu_regs);
    buffer[15] = MOS6510DTV_REGS_GET_R11(&maincpu_regs);
    buffer[16] = MOS6510DTV_REGS_GET_R12(&maincpu_regs);
    buffer[17] = MOS6510DTV_REGS_GET_R13()&maincpu_regs);
    buffer[18] = MOS6510DTV_REGS_GET_R14(&maincpu_regs);
    buffer[19] = MOS6510DTV_REGS_GET_R15(&maincpu_regs);
    buffer[20] = MOS6510DTV_REGS_GET_ACM(&maincpu_regs);
    buffer[21] = MOS6510DTV_REGS_GET_YXM(&maincpu_regs);
#endif
    
    return 0;
}

trace_action_t *trace_action_main_cpu_state(void)
{
    trace_action_t *action = action_create();
    if(action == NULL) {
        return NULL;
    }
    
    action->call = do_main_cpu_state;
    return action;
}







