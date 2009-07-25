/* tracer.h - Tracer interface
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


#ifndef VICE_TRACER_H
#define VICE_TRACER_H

#include "types.h"

extern int tracer_enabled;

extern int tracer_transmit(const char * buffer, size_t buffer_length);
extern int tracer_receive(char * buffer, size_t buffer_length);
extern int tracer_data_available(void);
extern int tracer_is_connected(void);

extern void tracer_worker(void);

#define TRACER_WORKER               \
if(tracer_enabled) {                \
    tracer_worker();                \
}

extern int tracer_resources_init(void);
extern void tracer_resources_shutdown(void);
extern int tracer_cmdline_options_init(void);

#endif
