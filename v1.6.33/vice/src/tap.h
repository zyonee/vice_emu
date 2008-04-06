/*
 * tap.h - TAP file support.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _TAP_H
#define _TAP_H

#include <stdio.h>

#include "types.h"

#define TAP_HDR_SIZE         20
#define TAP_HDR_MAGIC_OFFSET 0
#define TAP_HDR_VERSION      12
#define TAP_HDR_LEN          16

typedef struct tap_s {
    /* File name.  */
    char *file_name;

    /* File descriptor.  */
    FILE *fd;

    /* Size of the image.  */
    int size;

    /* The TAP version byte.  */
    BYTE version;

    /* Position in the current file.  */
    int current_file_seek_position;

    /* Header offset.  */
    int offset;

    /* Tape counter in machine-cycles/8 for even looong tapes */
    long cycle_counter;

    /* Tape length in machine-cycles/8 */
    long cycle_counter_total;

    /* Tape counter.  */
    int counter;

    /* Which mode is activated?  */
    int mode;

    /* Is the file opened read only?  */
    int read_only;

    /* Has the tap changed? We correct the size then.  */
    int has_changed;

} tap_t;

extern tap_t *tap_open(const char *name);
extern int tap_close(tap_t *tap);

#endif

