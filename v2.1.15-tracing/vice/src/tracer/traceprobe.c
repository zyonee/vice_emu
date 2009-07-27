/* traceprobe.c - Trace Probe interface
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

#include "traceprobe.h"
#include "traceaction.h"
#include "log.h"

/* static definition of probes */
trace_probe_t trace_probe_table[] = {
    TRACE_PROBE_DECLARE(VSync),
    TRACE_PROBE_DECLARE(MainCPU)
};

void trace_probe_fire(struct trace_probe_s *tp)
{
    trace_action_t *action = tp->action;
    while(action != NULL) {
        /* call action. if the action failed then disable probe */
        if(action->call(action) < 0) {
            log_message(LOG_DEFAULT, "trace action in probe '%s' failed. disabling probe!", tp->name);
            tp->enabled = 0;
            break;
        }
        action = action->next;
    }
}

void trace_probe_add_action(int index, struct trace_action_s *action)
{
    if(action == NULL) {
        return;
    }
    
    trace_probe_t *probe = &trace_probe_table[index];
    action->next = probe->action;
    probe->action = action;
}

void trace_probe_remove_all_actions(int index)
{
    trace_probe_t *probe = &trace_probe_table[index];
    trace_action_t *action = probe->action;
    while(action != NULL) {
        trace_action_t *next_action = action->next;
        trace_action_free(action);
        action = next_action;
    }
    probe->action = NULL;
}

void trace_probe_enable(int index)
{
    trace_probe_t *probe = &trace_probe_table[index];
    if(probe->action != NULL) {
        probe->enabled = 1;
    }
}

void trace_probe_disable(int index)
{
    trace_probe_t *probe = &trace_probe_table[index];
    probe->enabled = 0;
}

void trace_probe_enable_all(void)
{
    int i;
    
    for(i=0;i<TRACE_PROBE_NUM;i++) {
        trace_probe_t *probe = &trace_probe_table[i];
        if(probe->action != NULL) {
            probe->enabled = 1;
        }
    }
}

void trace_probe_disable_all(void)
{
    int i;
    
    for(i=0;i<TRACE_PROBE_NUM;i++) {
        trace_probe_t *probe = &trace_probe_table[i];
        probe->enabled = 0;
    }
}

void trace_probe_suspend_all(void)
{
    int i;
    
    for(i=0;i<TRACE_PROBE_NUM;i++) {
        trace_probe_t *probe = &trace_probe_table[i];
        probe->enabled_before_suspend = probe->enabled;
        probe->enabled = 0;
    }
    
}

void trace_probe_resume_all(void)
{
    int i;
    
    for(i=0;i<TRACE_PROBE_NUM;i++) {
        trace_probe_t *probe = &trace_probe_table[i];
        probe->enabled = probe->enabled_before_suspend;
    }    
}


