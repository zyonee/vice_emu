/*
 * fdd.h - (M)FM floppy disk drive emulation
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#ifndef VICE_FDD_H
#define VICE_FDD_H

#include "types.h"

/* Number of cycles before an attached disk becomes visible to the R/W head.
   This is mostly to make routines that auto-detect disk changes happy.  */
#define DRIVE_ATTACH_DELAY           (3*600000)

/* Number of cycles the write protection is activated on detach.  */
#define DRIVE_DETACH_DELAY           (3*200000)

/* Number of cycles the after a disk can be inserted after a disk has been
   detached.  */
#define DRIVE_ATTACH_DETACH_DELAY    (3*400000)

struct disk_image_s;
struct drive_s;
typedef struct fd_drive_s fd_drive_t;

extern const int fdd_data_rates[4];
extern fd_drive_t *fdd_init(int num, struct drive_s *drive);
extern void fdd_shutdown(fd_drive_t *drv);
extern void fdd_reset(fd_drive_t *drv, CLOCK clk);

extern void fdd_image_attach(fd_drive_t *drv, struct disk_image_s *image);
extern void fdd_image_detach(fd_drive_t *drv);
extern inline void fdd_byte_write(fd_drive_t *drv, BYTE data);
extern inline BYTE fdd_byte_read(fd_drive_t *drv);
extern inline void fdd_mfm_write(fd_drive_t *drv, WORD data);
extern inline WORD fdd_mfm_read(fd_drive_t *drv);
extern inline int fdd_mfm_ready(fd_drive_t *drv);
extern void fdd_flush(fd_drive_t *drv);
extern void fdd_step_pulse(fd_drive_t *drv, int dir);
extern inline void fdd_set_side(fd_drive_t *drv, int side);
extern inline void fdd_set_motor(fd_drive_t *drv, int motor);
extern inline void fdd_set_rate(fd_drive_t *drv, int rate);
extern inline void fdd_set_write_gate(fd_drive_t *drv, int wg);
extern inline void fdd_set_soe(fd_drive_t *drv, int soe);
extern void fdd_set_clk(fd_drive_t *drv, CLOCK clk);
extern inline int fdd_index(fd_drive_t *drv);
extern inline void fdd_index_count_reset(fd_drive_t *drv);
extern inline int fdd_index_count(fd_drive_t *drv);
extern inline int fdd_track0(fd_drive_t *drv);
extern inline int fdd_write_protect(fd_drive_t *drv);
extern inline int fdd_disk_change(fd_drive_t *drv);
extern inline int fdd_byte_ready(fd_drive_t *drv);
extern inline int fdd_byte_ready_edge(fd_drive_t *drv);
extern inline int fdd_byte_ready_1551(fd_drive_t *drv);
extern inline void fdd_byte_ready_clear(fd_drive_t *drv);
extern inline int fdd_sync(fd_drive_t *drv);
extern inline WORD fdd_crc(WORD crc, BYTE b);
extern inline void fdd_overflow_callback(CLOCK sub, fd_drive_t *drv);
extern void fdd_set_frequency(fd_drive_t *drv, int mhz);
extern void fdd_set_native(fd_drive_t *drv, int mode);

#endif 

