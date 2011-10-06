/*
 * fsimage-flat.c - flat image file handling (e.g. D64)
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

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "fsimage-flat.h"
#include "fsimage.h"
#include "gcr.h"
#include "log.h"
#include "types.h"
#include "util.h"
#include "x64.h"
#include "cbmdos.h"


static log_t fsimage_flat_log = LOG_ERR;

/*-----------------------------------------------------------------------*/
/* Calculates sector offset in image file from logical address */
static int fsimage_calc_logical_offset(disk_image_t *image, unsigned int track,
                            unsigned int sector)
{
    int sectors, i;

    if (track < 1 || track >= image->ltracks
        || sector >= disk_image_sector_per_track(image, track)) {
        log_error(fsimage_flat_log, "Track: %i, Sector: %i out of bounds.",
                track, sector);
        return -1;
    }

    sectors = sector + image->loffset;
    for (i = 1; i < track; i++) {
        sectors += disk_image_sector_per_track(image, i);
    }
    return sectors;
}

/* Calculates physical track offset in image file */
static int fsimage_calc_physical_offset(disk_image_t *image, unsigned int track, unsigned head)
{
    int sectors, i;

    if (image->type == DISK_IMAGE_TYPE_D71 && head == 1) {
        track += 35;
    }

    sectors = 0;
    for (i = 1; i < track; i++) {
        sectors += disk_image_sector_per_track(image, i);
    }
    return sectors;
}


/*-----------------------------------------------------------------------*/
/* Read an entire track from the disk image. */

int fsimage_flat_read_track(disk_image_t *image, unsigned int track,
                           unsigned int head, BYTE *gcr_speed_zone,
                           BYTE *gcr_data, int *track_size)
{
    BYTE buffer[256];
    unsigned int sector;
    BYTE *ptr;
    unsigned int max_sector;
    int speed_zone = 0;
    int rc, offset, sectors;
    fsimage_t *fsimage = image->media.fsimage;
    unsigned int ltrack;

    track /= 2;
    ltrack = track;
    if (image->type == DISK_IMAGE_TYPE_D71) {
        ltrack += head * 35;
    }

    /* Since the D64/D71 format does not provide the actual track sizes or
       speed zones, we set them to standard values.  */

    speed_zone = disk_image_speed_map_1541(track);
    max_sector = disk_image_sector_per_track(image, track);

    /* Clear track to avoid read errors.  */
    *track_size = disk_image_raw_track_size_1541(track);
    memset(gcr_data, 0x55, *track_size);
    memset(gcr_speed_zone, speed_zone, *track_size);

    sectors = fsimage_calc_physical_offset(image, track, head);
    offset = sectors * 256;

    if (image->type == DISK_IMAGE_TYPE_X64)
        offset += X64_HEADER_LENGTH;

    fseek(fsimage->fd, offset, SEEK_SET);

    ptr = gcr_data;
    for (sector = 0; sector < max_sector; sector++) {
        if (fread((char *)buffer, 256, 1, fsimage->fd) < 1) {
            log_error(fsimage_flat_log,
                    "Error reading T:%d H:%d S:%d from disk image.",
                    track, head, sector);
        } else {
            rc = (fsimage->error_info != NULL) ? fsimage->error_info[sectors++] : 0;
            if (rc == 21) {
                memset(gcr_data, 0x00, *track_size);
                break;
            }

            gcr_convert_sector_to_GCR(buffer, ptr, ltrack, sector,
                                      image->diskID[0], image->diskID[1],
                                      (BYTE)(rc));
        }

        ptr += SECTOR_GCR_SIZE_WITH_HEADER + disk_image_gap_size_1541(track);
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Write an entire track to the disk image. */

int fsimage_flat_write_track(disk_image_t *image, unsigned int track,
                            unsigned int head, int track_size,
                            BYTE *gcr_speed_zone, BYTE *gcr_track_start_ptr)
{
    unsigned int sector, max_sector;
    BYTE buffer[256];
    fsimage_t *fsimage = image->media.fsimage;
    int offset;
    unsigned int ltrack;

    if (track > image->ptracks || track < 2 || head >= image->sides) {
        return -1;
    }

    track /= 2;
    ltrack = track;
    if (image->type == DISK_IMAGE_TYPE_D71) {
        ltrack += head * 35;
    }
    max_sector = disk_image_sector_per_track(image, track);

    offset = fsimage_calc_physical_offset(image, track, head) * 256;

    if (image->type == DISK_IMAGE_TYPE_X64) {
        offset += X64_HEADER_LENGTH;
    }

    fseek(fsimage->fd, offset, SEEK_SET);

    for (sector = 0; sector < max_sector; sector++) {

        switch (gcr_read_sector(gcr_track_start_ptr, track_size, buffer, ltrack, sector)) {
        case -1:
            log_error(fsimage_flat_log, "Could not find header of T:%d H:%d S:%d.",
                      track, head, sector);
            continue;
        case -2:
            log_error(fsimage_flat_log, "Could not find data sync of T:%d H:%d S:%d.",
                    track, head, sector);
            continue;
        case -3:
            log_error(fsimage_flat_log, "Could not find data block id of T:%d H:%d S:%d.",
                      track, head, sector);
            continue;
        } 
        if (fwrite((char *)buffer, 256, 1, fsimage->fd) < 1) {
            log_error(fsimage_flat_log, "Error writing T:%d H:%d S:%d to disk image.",
                    track, head, sector);
        }
    }

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Read a 256 byte logical sector from the flat disk image.  */

int fsimage_flat_read_sector(disk_image_t *image, BYTE *buf,
                               unsigned int track, unsigned int sector)
{
    int sectors;
    long offset;
    fsimage_t *fsimage = image->media.fsimage;

    sectors = fsimage_calc_logical_offset(image, track, sector);

    if (sectors < 0) {
        return CBMDOS_IPE_ILLEGAL_TRACK_OR_SECTOR;
    }
    offset = sectors * 256;

    if (image->type == DISK_IMAGE_TYPE_X64)
        offset += X64_HEADER_LENGTH;

    fseek(fsimage->fd, offset, SEEK_SET);

    if (fread((char *)buf, 256, 1, fsimage->fd) < 1) {
        log_error(fsimage_flat_log,
                "Error reading T:%i S:%i from disk image.",
                track, sector);
        return -1;
    }

    if (fsimage->error_info != NULL) {
        switch (fsimage->error_info[sectors]) {
        case 0x0:
        case 0x1:
            return CBMDOS_IPE_OK;               /* 0 */
        case 0x2:
            return CBMDOS_IPE_READ_ERROR_BNF;   /* 20 */
        case 0x3:
            return CBMDOS_IPE_READ_ERROR_SYNC;  /* 21 */
        case 0x4:
            return CBMDOS_IPE_READ_ERROR_DATA;  /* 22 */
        case 0x5:
            return CBMDOS_IPE_READ_ERROR_CHK;   /* 23 */ 
        case 0x7:
            return CBMDOS_IPE_WRITE_ERROR_VER;  /* 25 */
        case 0x8:
            return CBMDOS_IPE_WRITE_PROTECT_ON; /* 26 */
        case 0x9:
            return CBMDOS_IPE_READ_ERROR_BCHK;  /* 27 */
        case 0xA:
            return CBMDOS_IPE_WRITE_ERROR_BIG;  /* 28 */
        case 0xB:
            return CBMDOS_IPE_DISK_ID_MISMATCH; /* 29 */
        case 0xF:
            return CBMDOS_IPE_NOT_READY;        /* 74 */
        case 0x10:
            return CBMDOS_IPE_READ_ERROR_GCR;   /* 24 */
        default:
            return 0;
        }
    }
    return 0;
}


/*-----------------------------------------------------------------------*/
/* Write a 256 byte logical sector to a flat disk image.  */

int fsimage_flat_write_sector(disk_image_t *image, BYTE *buf,
                                unsigned int track, unsigned int sector)
{
    int sectors;
    long offset;
    fsimage_t *fsimage = image->media.fsimage;

    sectors = fsimage_calc_logical_offset(image, track, sector);

    if (sectors < 0) {
        return CBMDOS_IPE_ILLEGAL_TRACK_OR_SECTOR;
    }
    offset = sectors * 256;

    if (image->type == DISK_IMAGE_TYPE_X64)
        offset += X64_HEADER_LENGTH;

    fseek(fsimage->fd, offset, SEEK_SET);

    if (fwrite((char *)buf, 256, 1, fsimage->fd) < 1) {
        log_error(fsimage_flat_log, "Error writing T:%i S:%i to disk image.",
                track, sector);
        return -1;
    }

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);
    return 0;
}

int fsimage_flat_create(disk_image_t *image, unsigned int type)
{
    fsimage_t *fsimage = image->media.fsimage;
    unsigned int size, i, size2;
    BYTE block[256];
    int rc = 0;

    size = 0; size2 = 0;

    switch(type) {
      case DISK_IMAGE_TYPE_D64:
      case DISK_IMAGE_TYPE_X64:
        size = D64_FILE_SIZE_35;
        break;
      case DISK_IMAGE_TYPE_D67:
        size = D67_FILE_SIZE;
        break;
      case DISK_IMAGE_TYPE_D71:
        size = D71_FILE_SIZE;
        break;
      case DISK_IMAGE_TYPE_D81:
        size = D81_FILE_SIZE;
        break;
      case DISK_IMAGE_TYPE_D80:
        size = D80_FILE_SIZE;
        break;
      case DISK_IMAGE_TYPE_D82:
        size = D82_FILE_SIZE;
        break;
      case DISK_IMAGE_TYPE_D1M:
        size = D1M_FILE_SIZE;
        size2 = 40 * 256;
        break;
      case DISK_IMAGE_TYPE_D2M:
        size = D2M_FILE_SIZE;
        size2 = 80 * 256;
        break;
      case DISK_IMAGE_TYPE_D4M:
        size = D4M_FILE_SIZE;
        size2 = 160 * 256;
        break;
    }

    memset(block, 0, sizeof(block));

    switch (type) {
      case DISK_IMAGE_TYPE_X64:
        {
            BYTE header[X64_HEADER_LENGTH];

            memset(header, 0, X64_HEADER_LENGTH);

            header[X64_HEADER_MAGIC_OFFSET + 0] = X64_HEADER_MAGIC_1;
            header[X64_HEADER_MAGIC_OFFSET + 1] = X64_HEADER_MAGIC_2;
            header[X64_HEADER_MAGIC_OFFSET + 2] = X64_HEADER_MAGIC_3;
            header[X64_HEADER_MAGIC_OFFSET + 3] = X64_HEADER_MAGIC_4;
            header[X64_HEADER_VERSION_OFFSET + 0] = X64_HEADER_VERSION_MAJOR;
            header[X64_HEADER_VERSION_OFFSET + 1] = X64_HEADER_VERSION_MINOR;
            header[X64_HEADER_FLAGS_OFFSET + 0] = 1;
            header[X64_HEADER_FLAGS_OFFSET + 1] = NUM_TRACKS_1541;
            header[X64_HEADER_FLAGS_OFFSET + 2] = 1;
            header[X64_HEADER_FLAGS_OFFSET + 3] = 0;
            if (fwrite(header, X64_HEADER_LENGTH, 1, fsimage->fd) < 1) {
                log_error(fsimage_flat_log,
                          "Cannot write X64 header to disk image `%s'.",
                          fsimage->name);
            }

        }
        /* Fall through.  */
      case DISK_IMAGE_TYPE_D64:
      case DISK_IMAGE_TYPE_D67:
      case DISK_IMAGE_TYPE_D71:
      case DISK_IMAGE_TYPE_D81:
      case DISK_IMAGE_TYPE_D80:
      case DISK_IMAGE_TYPE_D82:
      case DISK_IMAGE_TYPE_D1M:
      case DISK_IMAGE_TYPE_D2M:
      case DISK_IMAGE_TYPE_D4M:
        for (i = 0; i < ((size - size2) / 256); i++) {
            if (fwrite(block, 256, 1, fsimage->fd) < 1) {
                log_error(fsimage_flat_log,
                          "Cannot seek to end of disk image `%s'.",
                          fsimage->name);
                rc = -1;
                break;
            }
        }
        if (!rc && size2) {
            for (i = 0; i < size2 / 256; i++) {
                memset(block, 0, 256);
                if (i == 5) {
                    memset(block, 255, 224);
                    block[0] = 0x00;
                    block[0x38] = 0x00;
                    block[0x70] = 0x00;
                    block[0xa8] = 0x00;
                    block[0x39] = 0x00;
                    block[0x71] = (size - size2) >> 17;
                    block[0xa9] = (size - size2) >> 9;
                    block[0xe2] = 1;
                    block[0xe3] = 1;
                    memcpy(block + 0xf0, "\x43\x4d\x44\x20\x46\x44\x20\x53\x45\x52\x49\x45\x53\x20\x20\x20",16);
                } else if (i == 8) {
                    block[0x00] = 0x01;
                    block[0x01] = 0x01;
                    block[0x02] = 0xff;
                    memcpy(block + 5, "\x53\x59\x53\x54\x45\x4d\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0", 16);
                    block[0x22] = 0x01;
                    memcpy(block + 0x25, "\x50\x41\x52\x54\x49\x54\x49\x4f\x4e\x20\x31\xa0\xa0\xa0\xa0\xa0", 16);
                    block[0x3e] = (size - size2) >> 17;
                    block[0x3f] = (size - size2) >> 9;
                } else if (i > 8 && i < 11) {
                    block[0x00] = 0x01;
                    block[0x01] = i - 7;
                } else if (i == 11) {
                    block[0x01] = 0xff;
                }
                if (fwrite(block, 256, 1, fsimage->fd) < 1) {
                    log_error(fsimage_flat_log,
                            "Cannot seek to end of disk image `%s'.",
                            fsimage->name);
                    rc = -1;
                    break;
                }
            }
            break;
        }
        break;
    }
    return rc;
}

/*-----------------------------------------------------------------------*/

void fsimage_flat_init(void)
{
    fsimage_flat_log = log_open("Filesystem Image Flat");
}

