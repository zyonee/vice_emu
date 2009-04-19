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

#include "filedma.h"
#include "mem.h"
#include "fileio.h"
#include "lib.h"

int filedma_load_via_fileio(const char *file_name,log_t log)
{
    fileio_info_t *finfo;
    WORD addr,size,ptr;
    int  end;
    BYTE lo,hi;
    BYTE *memory;
    int  result,i;

    /* does the current machine support DMA loads? */
    if(filedma_can_load()!=0) {
        log_error(log, "Machine does not support DMA load");
        return -1;
    }

    /* try to open file */
    finfo = fileio_open(file_name, NULL, FILEIO_FORMAT_RAW | FILEIO_FORMAT_P00,
                        FILEIO_COMMAND_READ | FILEIO_COMMAND_FSNAME,
                        FILEIO_TYPE_PRG);

    if (finfo == NULL) {
        log_error(log, "Cannot open `%s'.", file_name);
        return -1;
    }

    /* get data size of file */
    size = fileio_get_bytes_left(finfo);

    /* read start address */
    if((fileio_read(finfo, &lo, 1) != 1) ||
       (fileio_read(finfo, &hi, 1) != 1)) {
        log_error(log, "Cannot read start address from '%s'", file_name);
        fileio_close(finfo);
        return -1;   
    }
    addr = (WORD)hi << 8 | (WORD)lo;
    size -= 2; /* skip load addr */

    /* check range */
    end = addr + size - 1;
    if(end > 0xffff) {
        log_error(log, "Invalid size of '%s': %d", file_name, size);
        fileio_close(finfo);
        return -1;
    }

    /* load to memory */
    memory = (BYTE *)lib_malloc(size);
    if(memory == NULL) {
        log_error(log, "No memory for '%s'", file_name);
        fileio_close(finfo);
        return -1;        
    }
    
    ptr = addr;
    i = 0;
    while(ptr <= end) {
        if(fileio_read(finfo, &memory[i], 1) != 1) {
            log_error(log, "Load error '%s'", file_name);
            fileio_close(finfo);
            lib_free(memory);
            return -1;
        }
        ptr++;
        i++;
    }

    /* close file again */
    fileio_close(finfo);
    
    /* now perform dma load on machine */
    log_message(log, "DMA Loading '%s' to $%04x-$%04x", file_name, addr, end);
    if(filedma_load(memory, addr, end)==0) {
        result = 0;
    } else {
        log_message(log, "DMA load failed '%s'", file_name);
        result = -1;
    }
    
    lib_free(memory);    
    return result;
}
    
int filedma_load_simulate_basic(const BYTE *memory, WORD start, WORD end,
                                const filedma_basic_t *basic)
{
    int ptr,i;
    
    /* invalid range */
    if(end > basic->end) {
        return -1;
    }
    
    /* store in memory */
    i = 0;
    for(ptr = start; ptr <= end; ptr++) {
        mem_store(ptr, memory[i++]);
    }
    
    /* set load end */
    if(start <= basic->begin) {
        BYTE lo,hi;
        lo = (BYTE)(end & 0xff);
        hi = (BYTE)((end >> 8) & 0xff);
        mem_store(basic->end_ptr, lo);
        mem_store(basic->end_ptr+1, hi);
    }

    return 0;
}
