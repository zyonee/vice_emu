/* traceprobe.h - Trace Probe interface
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


#ifndef VICE_TRACEPROBE_H
#define VICE_TRACEPROBE_H

#include "types.h"

#ifdef USE_TRACER

struct trace_action_s;
struct trace_probe_s;

/* a pointer to a predicate function for a trace probe */
typedef int (*trace_predicate_f)(struct trace_probe_s *);

/* define the memory layout of a trace probe */
struct trace_probe_s {
    const char *name;                           /*< name of probe */
    int enabled;                                /*< is probe currently enabled? */
    int enabled_before_suspend;                 /*< store enabled flag for suspend */
    struct trace_action_s *action;              /*< first action at probe */
    trace_predicate_f *predicate;               /*< optional predicate */ 
};
typedef struct trace_probe_s trace_probe_t;

/* helper to declare global trace probe table in traceprobe.c */
#define TRACE_PROBE_DECLARE(x) \
{ #x, 0, NULL, NULL }

/* the global table of all trace probes */
extern trace_probe_t trace_probe_table[];

/* if the probe is enabled and the predicate matches then "fire" probe */
extern void trace_probe_fire(struct trace_probe_s *tp);

/* trigger a probe in the code */
#define TRACE_PROBE_TRIGGER(index, extra)                       \
do {                                                            \
    trace_probe_t *probe = &trace_probe_table[index];           \
    if(probe->enabled) {                                        \
        if( (probe->predicate == NULL) ||                       \
            ( (*probe->predicate)(probe) ) ) {                  \
               extra;                                           \
               trace_probe_fire(probe);                         \
        }                                                       \
    }                                                           \
} while(0)

#else /* USE_TRACER */

/* declare trace trigger empty if tracing is disabled */
#define TRACE_PROBE_TRIGGER(index, extra)          do {} while(0)

#endif /* USE_TRACER */

extern void trace_probe_add_action(int index, struct trace_action_s *action);
extern void trace_probe_remove_all_actions(int index);
extern void trace_probe_enable(int index);
extern void trace_probe_disable(int index);
extern void trace_probe_enable_all(void);
extern void trace_probe_disable_all(void);
extern void trace_probe_suspend_all(void);
extern void trace_probe_resume_all(void);

/* ----- Trace Probe Indexes ----- */
#define TRACE_PROBE_VSYNC       0
#define TRACE_PROBE_MAIN_CPU    1
#define TRACE_PROBE_NUM         2

#endif
