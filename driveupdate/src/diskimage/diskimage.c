/*
 * diskimage.c - Common low-level disk image access.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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
#include <stdlib.h>
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "fsimage.h"
#include "lib.h"
#include "log.h"
#include "rawimage.h"
#include "realimage.h"
#include "types.h"
#include "cbmdos.h"

static log_t disk_image_log = LOG_DEFAULT;

/*-----------------------------------------------------------------------*/
/* Disk constants.  */

static const unsigned int raw_track_size_1541[4] = { 6250, 6666, 7142, 7692 };
static const unsigned int gaps_between_sectors_1541[4] = { 9, 12, 17, 8 };
static const int sectors_1541[4] = { 17, 18, 19, 21 };
static const char sectors_2040[4] = { 17, 18, 20, 21 };
static const char sectors_8050[4] = { 23, 25, 27, 29 };

unsigned int disk_image_speed_map_1541(unsigned int track)
{
    return (track < 25) ? ((track < 18) ? 3 : 2) : ((track < 31) ? 1: 0);
}

static unsigned int disk_image_speed_map_8050(unsigned int track)
{
    return (track < 54) ? ((track < 40) ? 3 : 2) : ((track < 65) ? 1: 0);
}

unsigned int disk_image_raw_track_size_1541(unsigned int track)
{
    return raw_track_size_1541[disk_image_speed_map_1541(track)];
}

unsigned int disk_image_gap_size_1541(unsigned int track)
{
    return gaps_between_sectors_1541[disk_image_speed_map_1541(track)];
}

/*-----------------------------------------------------------------------*/
/* Check for track out of bounds.  */

unsigned int disk_image_sector_per_track(disk_image_t *image,
                                         unsigned int track)
{
    if (track > 0) {
        switch (image->type) {
        case DISK_IMAGE_TYPE_D64:
        case DISK_IMAGE_TYPE_X64:
        case DISK_IMAGE_TYPE_G64:
            if (track > MAX_TRACKS_1541) {
                break;
            }
            return sectors_1541[disk_image_speed_map_1541(track)];
        case DISK_IMAGE_TYPE_D67:
            if (track > MAX_TRACKS_2040) {
                break;
            }
            return sectors_2040[disk_image_speed_map_1541(track)];
        case DISK_IMAGE_TYPE_D71:
            if (track > MAX_TRACKS_1571) {
                break;
            }
            return sectors_1541[disk_image_speed_map_1541((track > 35) ? (track - 35) : track)];
        case DISK_IMAGE_TYPE_D80:
            if (track > MAX_TRACKS_8050) {
                break;
            }
            return sectors_8050[disk_image_speed_map_8050(track)];
        case DISK_IMAGE_TYPE_D82:
            if (track > MAX_TRACKS_8250) {
                break;
            }
            return sectors_8050[disk_image_speed_map_8050((track > 77) ? (track - 77) : track)];
        case DISK_IMAGE_TYPE_D81:
            if (track > NUM_TRACKS_1581) {
                break;
            }
            return 40;
        case DISK_IMAGE_TYPE_D1M:
            if (track > NUM_TRACKS_1000) {
                break;
            }
            return (track == NUM_TRACKS_1000) ? 128 : 256;
        case DISK_IMAGE_TYPE_D2M:
            if (track > NUM_TRACKS_2000) {
                break;
            }
            return 256;
        case DISK_IMAGE_TYPE_D4M:
            if (track > NUM_TRACKS_4000) {
                break;
            }
            return 256;
        default:
            log_message(disk_image_log,
                    "Unknown disk type %i.  Cannot calculate sectors per track",
                    image->type);
            return 0;
        }
    }
    log_message(disk_image_log, "Track %i exceeds sector map.", track);
    return 0;
}

/*-----------------------------------------------------------------------*/

int disk_image_check_sector(disk_image_t *image, unsigned int track,
                            unsigned int sector)
{
    int sectors, i;

    if (track < 1)
        return -1;

    if (sector >= disk_image_sector_per_track(image, track))
        return -1;

    sectors = sector;
    for (i = 1; i < track; i++) {
        sectors += disk_image_sector_per_track(image, i);
    }
    return sectors;
}

/*-----------------------------------------------------------------------*/

void disk_image_attach_log(disk_image_t *image, signed int lognum,
                           unsigned int unit)
{
    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        log_verbose("Unit %d: %s disk image attached: %s.",
                    unit, image->type_name, fsimage_name_get(image));
        break;
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        log_verbose("Unit %d: %s disk attached (drive: %s).",
                    unit, image->type_name, rawimage_name_get(image));
        break;
#endif
    }
}

void disk_image_detach_log(disk_image_t *image, signed int lognum,
                           unsigned int unit)
{
    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        log_verbose("Unit %d: %s disk image detached: %s.",
                    unit, image->type_name, fsimage_name_get(image));
        break;
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        log_verbose("Unit %d: %s disk detached (drive: %s).",
                    unit, image->type_name, rawimage_name_get(image));
        break;
#endif
    }
}
/*-----------------------------------------------------------------------*/

void disk_image_fsimage_name_set(disk_image_t *image, char *name)
{
    fsimage_name_set(image, name);
}

char *disk_image_fsimage_name_get(disk_image_t *image)
{
    return fsimage_name_get(image);
}

int disk_image_fsimage_create(const char *name, unsigned int type)
{
    return fsimage_create(name, type);
}

/*-----------------------------------------------------------------------*/

void disk_image_rawimage_name_set(disk_image_t *image, char *name)
{
#ifdef HAVE_RAWDRIVE
    rawimage_name_set(image, name);
#endif
}

void disk_image_rawimage_driver_name_set(disk_image_t *image)
{
#ifdef HAVE_RAWDRIVE
    rawimage_driver_name_set(image);
#endif
}

/*-----------------------------------------------------------------------*/

void disk_image_name_set(disk_image_t *image, char *name)
{
    if (image == NULL) {
        return;
    }
    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        fsimage_name_set(image, name);
        break;
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rawimage_name_set(image, name);
        break;
#endif
    }
}

char *disk_image_name_get(disk_image_t *image)
{
    if (image == NULL) {
        return NULL;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        return fsimage_name_get(image);
        break;
    }

    return NULL;
}

/*-----------------------------------------------------------------------*/

disk_image_t *disk_image_create(void)
{
    return (disk_image_t *)lib_malloc(sizeof(disk_image_t));
}

void disk_image_destroy(disk_image_t *image)
{
    lib_free(image);
}

/*-----------------------------------------------------------------------*/

void disk_image_media_create(disk_image_t *image)
{
    if (image == NULL) {
        return;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        fsimage_media_create(image);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        realimage_media_create(image);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rawimage_media_create(image);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device);
    }
}

void disk_image_media_destroy(disk_image_t *image)
{
    if (image == NULL)
        return;

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        fsimage_media_destroy(image);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        realimage_media_destroy(image);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rawimage_media_destroy(image);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device);
    }
}

/*-----------------------------------------------------------------------*/

int disk_image_open(disk_image_t *image)
{
    int rc = 0;

    if (image == NULL) {
        return -1;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_open(image);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        rc = realimage_open(image);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rc = rawimage_open(image);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device);
        rc = -1;
    }

    return rc;
}


int disk_image_close(disk_image_t *image)
{
    int rc = -1;

    if (image == NULL)
        return 0;

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_close(image);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        rc = realimage_close(image);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rc = rawimage_close(image);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device);
    }

    return rc;
}

/*-----------------------------------------------------------------------*/

int disk_image_read_sector(disk_image_t *image, BYTE *buf, unsigned int track,
                           unsigned int sector)
{
    int rc = -1;

    if (image == NULL) {
        return CBMDOS_IPE_NOT_READY;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_read_sector(image, buf, track, sector);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        rc = realimage_read_sector(image, buf, track, sector);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rc = rawimage_read_sector(image, buf, track, sector);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device); 
    }

    return rc;
}

int disk_image_write_sector(disk_image_t *image, BYTE *buf, unsigned int track,
                            unsigned int sector)
{
    int rc = -1;

    if (image == NULL) {
        return CBMDOS_IPE_NOT_READY;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_write_sector(image, buf, track, sector);
        break;
#ifdef HAVE_OPENCBM
    case DISK_IMAGE_DEVICE_REAL:
        rc = realimage_write_sector(image, buf, track, sector);
        break;
#endif
#ifdef HAVE_RAWDRIVE
    case DISK_IMAGE_DEVICE_RAW:
        rc = rawimage_write_sector(image, buf, track, sector);
        break;
#endif
    default:
        log_error(disk_image_log, "Unknow image device %i.", image->device);
    }

    return rc;
}

/*-----------------------------------------------------------------------*/

int disk_image_read_track(disk_image_t *image, int track, int head,
                          disk_track_t *raw)
{
    int rc = -1;

    if (image == NULL) {
        return -1;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_read_track(image, track, head, raw);
        break;
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device); 
    }
    return rc;
}

int disk_image_write_track(disk_image_t *image, int track, int head,
                           disk_track_t *raw)
{
    int rc = -1;

    if (image == NULL) {
        return -1;
    }

    switch (image->device) {
    case DISK_IMAGE_DEVICE_FS:
        rc = fsimage_write_track(image, track, head, raw);
        break;
    default:
        log_error(disk_image_log, "Unknown image device %i.", image->device); 
    }
    return rc;
}

/*-----------------------------------------------------------------------*/
/* Initialization.  */

void disk_image_init(void)
{
    disk_image_log = log_open("Disk Access");
    fsimage_init();
#ifdef HAVE_OPENCBM
    realimage_init();
#endif
#ifdef HAVE_RAWDRIVE
    rawimage_init();
#endif
}

int disk_image_resources_init(void)
{
#ifdef HAVE_RAWDRIVE
    if (rawimage_resources_init() < 0)
        return -1;
#endif
    return 0;
}

void disk_image_resources_shutdown(void)
{
#ifdef HAVE_RAWDRIVE
    rawimage_resources_shutdown();
#endif
}

int disk_image_cmdline_options_init(void)
{
#ifdef HAVE_RAWDRIVE
    if (rawimage_cmdline_options_init() < 0)
        return -1;
#endif
    return 0;
}

