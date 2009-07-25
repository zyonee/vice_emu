/* tracer.c - Tracer interface
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

#include "vice.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "monitor.h"
#include "resources.h"
#include "socket.h"
#include "tracer.h"
#include "traceprobe.h"
#include "traceaction.h"
#include "traceserver.h"
#include "translate.h"
#include "uiapi.h"
#include "util.h"

#ifdef USE_TRACER

int tracer_enabled = 0;

static int tracer_init(void)
{
    log_message(LOG_DEFAULT, "tracer: init");
    
    // TEST: setup probes
    trace_probe_table[TRACE_PROBE_VSYNC].action = trace_action_mem_dump(0,0xffff);
    trace_probe_table[TRACE_PROBE_VSYNC].enabled= 1;
    
    return 0;
}

static void tracer_quit(void)
{
    log_message(LOG_DEFAULT, "tracer: quit");
}

void tracer_worker(void)
{
#ifdef HAVE_NETWORK
    if(trace_server_data_available()>0) {
        
    }
#endif
}

/* ----- Resources ------------------------------------------------------- */

static int set_tracer_enabled(int val, void *param)
{
    if (!val) {
        if (tracer_enabled) {
            tracer_quit();
        }
        tracer_enabled = 0;
        return 0;
    } else { 
        if (!tracer_enabled) {
            if (tracer_init() < 0) {
                return -1;
            }
        }
        tracer_enabled = 1;
        return 0;
    }
}

static const resource_int_t resources_int[] = {
    { "Tracer", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &tracer_enabled, set_tracer_enabled, NULL },
    { NULL }
};

int tracer_resources_init(void)
{
#ifdef HAVE_NETWORK
    if(trace_server_resources_init()<0) {
        return -1;
    }
#endif
    
    return resources_register_int(resources_int);
}

void tracer_resources_shutdown(void)
{
#ifdef HAVE_NETWORK
    trace_server_resources_shutdown();
#endif
    
    if(tracer_enabled) {
        tracer_quit();
    }
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
    { "-tracer", SET_RESOURCE, 0,
      NULL, NULL, "Tracer", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable Tracer") },
    { "+tracer", SET_RESOURCE, 0,
      NULL, NULL, "Tracer", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable Tracer") },
    { NULL }
};

int tracer_cmdline_options_init(void)
{
#ifdef HAVE_NETWORK
    if(trace_server_cmdline_options_init()<0) {
        return -1;
    }
#endif
    
    return cmdline_register_options(cmdline_options);
}

#endif
