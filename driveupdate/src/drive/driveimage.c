/*
 * driveimage.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "drive.h"
#include "driveimage.h"
#include "drivetypes.h"
#include "log.h"
#include "types.h"
#include "uiapi.h"
#include "lib.h"
#include "fdd.h"


/* Logging goes here.  */
static log_t driveimage_log = LOG_DEFAULT;

static int drive_check_image_format(unsigned int format, unsigned int dnr)
{
    drive_t *drive;

    drive = drive_context[dnr]->drive;

    switch (format) {
      case DISK_IMAGE_TYPE_D64:
      case DISK_IMAGE_TYPE_D71:
      case DISK_IMAGE_TYPE_G64:
      case DISK_IMAGE_TYPE_X64:
        if (drive->type != DRIVE_TYPE_1541
            && drive->type != DRIVE_TYPE_1541II
            && drive->type != DRIVE_TYPE_1551
            && drive->type != DRIVE_TYPE_1570
            && drive->type != DRIVE_TYPE_1571
            && drive->type != DRIVE_TYPE_1571CR
            && drive->type != DRIVE_TYPE_2031
            && drive->type != DRIVE_TYPE_2040 /* FIXME: only read compat */
            && drive->type != DRIVE_TYPE_3040
            && drive->type != DRIVE_TYPE_4040)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D67:
        /* New drives and 2031, 3040 and 4040 are only read compatible.  */
        if (drive->type != DRIVE_TYPE_1541
            && drive->type != DRIVE_TYPE_1541II
            && drive->type != DRIVE_TYPE_1551
            && drive->type != DRIVE_TYPE_1570
            && drive->type != DRIVE_TYPE_1571
            && drive->type != DRIVE_TYPE_1571CR
            && drive->type != DRIVE_TYPE_2031
            && drive->type != DRIVE_TYPE_2040
            && drive->type != DRIVE_TYPE_3040
            && drive->type != DRIVE_TYPE_4040)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D81:
        if (drive->type != DRIVE_TYPE_1581
            && drive->type != DRIVE_TYPE_2000
            && drive->type != DRIVE_TYPE_4000)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D80:
      case DISK_IMAGE_TYPE_D82:
        if ((drive->type != DRIVE_TYPE_1001)
            && (drive->type != DRIVE_TYPE_8050)
            && (drive->type != DRIVE_TYPE_8250))
            return -1;
        break;
      case DISK_IMAGE_TYPE_D1M:
      case DISK_IMAGE_TYPE_D2M:
      case DISK_IMAGE_TYPE_D4M:
        if (drive->type != DRIVE_TYPE_2000
            && drive->type != DRIVE_TYPE_4000)
            return -1;
        break;
      default:
        return -1;
    }
    return 0;
}

#if 0
static void drive_extend_disk_image(drive_t *drive)
{
    int rc;
    unsigned int track, sector;
    BYTE buffer[256];

    drive->image->ltracks = EXT_TRACKS_1541;
    drive->image->ptracks = EXT_TRACKS_1541 * 2;
    memset(buffer, 0, 256);
    for (track = NUM_TRACKS_1541 + 1; track <= EXT_TRACKS_1541; track++) {
        for (sector = 0;
             sector < disk_image_sector_per_track(drive->image, track);
             sector++) {
             rc = disk_image_write_sector(drive->image, buffer, track,
                                          sector);
             if (rc < 0)
                 log_error(drive->log,
                           "Could not update T:%d S:%d.", track, sector);
        }
    }
}

void drive_image_writeback(drive_t *drive, int free)
{
    int extend;
    unsigned int track;

    if (drive->image == NULL || drive->raw == NULL)
        return;

    if (!(drive->raw->dirty)) {
        if (free && !drive->raw->pinned) {
            lib_free(drive->raw->data);
            lib_free(drive->raw);
            drive->raw_cache[drive->side][drive->current_half_track - 2] = NULL;
            drive->raw = NULL;
        }
        return;
    }

    track = drive->current_half_track / 2;

    if (drive->image->type == DISK_IMAGE_TYPE_D64
        || drive->image->type == DISK_IMAGE_TYPE_X64) {
        if (track > EXT_TRACKS_1541)
            return;

        if (drive->current_half_track > drive->image->ptracks) {
            switch (drive->extend_image_policy) {
              case DRIVE_EXTEND_NEVER:
                drive->ask_extend_disk_image = 1;
                return;
              case DRIVE_EXTEND_ASK:
                if (drive->ask_extend_disk_image == 1) {
                    extend = ui_extend_image_dialog();
                    if (extend == 0) {
                        drive->ask_extend_disk_image = 0;
                        return;
                    } else {
                        drive_extend_disk_image(drive);
                    }
                } else {
                    return;
                }
                break;
              case DRIVE_EXTEND_ACCESS:
                drive->ask_extend_disk_image = 1;
                drive_extend_disk_image(drive);
                break;
            }
        }
    }

    disk_image_write_track(drive->image, drive->current_half_track,
                           drive->side, drive->raw);
    drive->raw->dirty = 0;
    drive->raw->pinned = (drive->image->type != DISK_IMAGE_TYPE_G64);
}
#endif

/* Attach a disk image to the true drive emulation. */
int drive_image_attach(disk_image_t *image, unsigned int unit)
{
    unsigned int dnr;
    drive_t *drive;

    if (unit < 8 || unit >= 8 + DRIVE_NUM)
        return -1;

    dnr = unit - 8;
    drive = drive_context[dnr]->drive;

    if (drive_check_image_format(image->type, dnr) < 0)
        return -1;

    drive->ask_extend_disk_image = 1;

    switch (image->type) {
    case DISK_IMAGE_TYPE_D64:
    case DISK_IMAGE_TYPE_D67:
    case DISK_IMAGE_TYPE_D71:
    case DISK_IMAGE_TYPE_G64:
    case DISK_IMAGE_TYPE_X64:
    case DISK_IMAGE_TYPE_D81:
    case DISK_IMAGE_TYPE_D1M:
    case DISK_IMAGE_TYPE_D2M:
    case DISK_IMAGE_TYPE_D4M:
        disk_image_attach_log(image, driveimage_log, unit);
        break;
    default:
        return -1;
    }

    fdd_image_attach(drive->fdds[0], image);
    return 0;
}

/* Detach a disk image from the true drive emulation. */
int drive_image_detach(disk_image_t *image, unsigned int unit)
{
    unsigned int dnr;
    drive_t *drive;

    if (unit < 8 || unit >= 8 + DRIVE_NUM)
        return -1;

    dnr = unit - 8;
    drive = drive_context[dnr]->drive;

    switch(image->type) {
    case DISK_IMAGE_TYPE_D64:
    case DISK_IMAGE_TYPE_D67:
    case DISK_IMAGE_TYPE_D71:
    case DISK_IMAGE_TYPE_G64:
    case DISK_IMAGE_TYPE_X64:
    case DISK_IMAGE_TYPE_D81:
    case DISK_IMAGE_TYPE_D1M:
    case DISK_IMAGE_TYPE_D2M:
    case DISK_IMAGE_TYPE_D4M:
        disk_image_detach_log(image, driveimage_log, unit);
        break;
    default:
        return -1;
    }

    fdd_image_detach(drive->fdds[0]);
    return 0;
}

void drive_image_init(void)
{
    driveimage_log = log_open("DriveImage");
}

