/*
 * filedma.h - DMA load/save support for a machine
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

#ifndef VICE_FILEDMA_H
#define VICE_FILEDMA_H

#include "types.h"
#include "log.h"

/* --- public interface --- */

/* load an external file given by file_name directly into RAM */
extern int filedma_load_via_fileio(const char *file_name,log_t log);


/* --- machine dependent implementation --- */

/* does a machine support dma loading into memory? 0=yes -1=no */
extern int filedma_can_load(void);
 
/* load directly into memory of machine. 0=ok -1=error */
extern int filedma_load(const BYTE *memory, WORD start_addr, WORD end_addr);


/* --- generic tools --- */

struct filedma_basic_s {
    WORD begin;
    WORD end;
    WORD begin_ptr;
    WORD end_ptr;
};
typedef struct filedma_basic_s filedma_basic_t;
    
/* simulate a basic load with the given basic description */
extern int filedma_load_simulate_basic(const BYTE *memory, WORD start, WORD end,
                                       const filedma_basic_t *basic);

#endif
