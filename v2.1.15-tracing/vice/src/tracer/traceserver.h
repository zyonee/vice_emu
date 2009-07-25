/* traceserver.h - Tracer interface
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


#ifndef VICE_TRACESERVER_H
#define VICE_TRACESERVER_H

#include "types.h"

extern int trace_server_transmit(const char * buffer, size_t buffer_length);
extern int trace_server_receive(char * buffer, size_t buffer_length);
extern int trace_server_data_available(void);
extern int trace_server_is_connected(void);

extern int trace_server_resources_init(void);
extern void trace_server_resources_shutdown(void);
extern int trace_server_cmdline_options_init(void);

#endif
