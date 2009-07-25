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
    TRACE_PROBE_DECLARE(VSync)
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
