/*
 * scsi.h - SCSI device emulation
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

#ifndef VICE_SCSI
#define VICE_SCSI

#include "types.h"

typedef enum scsi_drive_type_e {
    SCSI_DRIVE_NONE, SCSI_DRIVE_HDD, SCSI_DRIVE_FDD, SCSI_DRIVE_CD
} scsi_drive_type_t;

typedef struct scsi_drive_s scsi_drive_t;

#define SCSI_C_D 0x0100
#define SCSI_MSG 0x0200
#define SCSI_I_O 0x0400
#define SCSI_SEL 0x0800
#define SCSI_BSY 0x1000
#define SCSI_REQ 0x2000
#define SCSI_ATN 0x4000
#define SCSI_ACK 0x8000

extern scsi_drive_t *scsi_init(int drive);
extern void scsi_shutdown(scsi_drive_t *drv);
extern void scsi_store(scsi_drive_t *cdrive, WORD value);
extern WORD scsi_read(scsi_drive_t *cdrive, WORD bus);
extern void scsi_image_attach(scsi_drive_t *cdrive, char *filename, scsi_drive_type_t type);
extern void scsi_image_detach(scsi_drive_t *cdrive);
extern int scsi_image_change(scsi_drive_t *cdrive, char *filename, scsi_drive_type_t type);
extern void scsi_reset(scsi_drive_t *cdrive);

struct snapshot_s;
extern int scsi_snapshot_read_module(scsi_drive_t *drv, struct snapshot_s *s);
extern int scsi_snapshot_write_module(scsi_drive_t *drv, struct snapshot_s *s);

#endif
