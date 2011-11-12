/*
 * fsimage-gcr.c
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
#include "fsimage-gcr.h"
#include "fsimage.h"
#include "gcr.h"
#include "log.h"
#include "types.h"
#include "util.h"
#include "lib.h"

static log_t fsimage_gcr_log = LOG_ERR;

/* Hardcoded/expected values VICE works with:
 * 0x1EF8 - 7928: Maximum container size for a raw GCR track
 * 0x54   -   84: Maximum container size for (half) track pointers
 * 0x00   -    0: GCR image file version number
 */

/*-----------------------------------------------------------------------*/
/* Read an entire GCR track from the disk image.  */

int fsimage_gcr_read_track(disk_image_t *image, unsigned int track,
                           unsigned int head, disk_track_t *raw)
{
    int track_len;
    int num_tracks;
    BYTE len[2];
    DWORD gcr_track_p = 0;
    DWORD gcr_speed_p = 0;
    int read = 0;
    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    raw->size = 6250;

    do {
        if (fsimage->fd == NULL) {
            log_error(fsimage_gcr_log, "Attempt to read without disk image.");
            break;
        }
        num_tracks = image->ptracks;

        if (track >= num_tracks || head != 0) {
            break;
        }
        if (fseek(fsimage->fd, 12 + track * 4, SEEK_SET) < 0
                || util_dword_read(fsimage->fd, &gcr_track_p, 1) < 0) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            break;
        }
        if (fseek(fsimage->fd, 12 + (num_tracks + track) * 4, SEEK_SET) < 0
                || util_dword_read(fsimage->fd, &gcr_speed_p, 1) < 0) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            break;
        }

        if (gcr_track_p == 0) {
            break;
        }
        if (fseek(fsimage->fd, gcr_track_p, SEEK_SET) < 0
            || fread(len, 2, 1, fsimage->fd) < 1) {
            log_error(fsimage_gcr_log, "Could not read GCR disk image.");
            break;
        }

        track_len = len[0] + len[1] * 256;
        raw->size = track_len;

        if (track_len < 5000 || track_len > NUM_MAX_BYTES_TRACK) {
            log_error(fsimage_gcr_log,
                      "Track field length %i is not supported.",
                      track_len);
            break;
        }
        if (gcr_speed_p >= 4) {
            log_error(fsimage_gcr_log, "Variable rate not supported yet.");
            break;
        }
        if (gcr_speed_p != 16 - (200000 / track_len + 1) / 2) {
            log_error(fsimage_gcr_log, "Rate has no correlation to track size, unsupported.");
            break;
        }
        read = 1;
    } while (0);

    raw->data = lib_realloc(raw->data, raw->size);
    if (!read || fread(raw->data, raw->size, 1, fsimage->fd) < 1) {
        memset(raw->data, 0, raw->size);
        return -1;
    }

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Write an entire GCR track to the disk image.  */

int fsimage_gcr_write_track(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    unsigned int num_tracks;
    int gap;
    BYTE len[2], padding[NUM_MAX_BYTES_TRACK];
    DWORD gcr_track_p = 0;
    DWORD gcr_speed_p;
    fsimage_t *fsimage = image->media.fsimage;

    if (head != 0) {
        log_error(fsimage_gcr_log,
                  "Attempt to write to non-existing side.");
        return -1;
    }

    num_tracks = image->ptracks;

    if (track >= num_tracks) {
        log_error(fsimage_gcr_log, "Could not extend GCR disk image.");
        return -1;
    }

    if (fseek(fsimage->fd, 12 + track * 4, SEEK_SET) < 0
        || util_dword_read(fsimage->fd, &gcr_track_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not read GCR disk image header.");
        return -1;
    }

    if (gcr_track_p == 0) {
        if (fseek(fsimage->fd, 0, SEEK_END) < 0) {
            log_error(fsimage_gcr_log, "Could not extend GCR disk image.");
            return -1;
        }
        gcr_track_p = ftell(fsimage->fd);
    } else if (fseek(fsimage->fd, gcr_track_p, SEEK_SET) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }

    len[0] = raw->size % 256;
    len[1] = raw->size / 256;
    if (fwrite(len, 2, 1, fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }

    if (fwrite(raw->data, raw->size, 1, fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image.");
        return -1;
    }
    gap = NUM_MAX_BYTES_TRACK - raw->size;
    if (gap > 0) {
        memset(padding, 0, gap);
        if (fwrite(padding, gap, 1, fsimage->fd) < 1) {
            log_error(fsimage_gcr_log, "Could not write GCR disk image.");
            return -1;
        }
    }

    fseek(fsimage->fd, 12 + track * 4, SEEK_SET);
    if (util_dword_write(fsimage->fd, &gcr_track_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image header.");
        return -1;
    }

    gcr_speed_p = 16 - (200000 / raw->size + 1) / 2;
    fseek(fsimage->fd, 12 + (num_tracks + track) * 4, SEEK_SET);
    if (util_dword_write(fsimage->fd, &gcr_speed_p, 1) < 0) {
        log_error(fsimage_gcr_log, "Could not write GCR disk image header.");
        return -1;
    }

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);

    return 0; /* always the same */
}

/*-----------------------------------------------------------------------*/
/* Read a sector from the GCR disk image.  */

int fsimage_gcr_read_sector(disk_image_t *image, BYTE *buf,
                               unsigned int track, unsigned int sector)
{
    disk_track_t raw;

    if (track > image->ltracks || track < 1) {
        log_error(fsimage_gcr_log,
                  "Track %i out of bounds.  Cannot read GCR track.",
                  track);
        return -1;
    }

    raw.data = NULL;

    if (fsimage_gcr_read_track(image, (track - 1) * 2, 0,
        &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot read track %i from GCR image.", track);
        lib_free(raw.data);
        return -1;
    }

    if (gcr_read_sector(&raw, buf, sector) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot find track: %i sector: %i within GCR image.",
                  track, sector);
        lib_free(raw.data);
        return -1;
    }
    lib_free(raw.data);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* Write a sector to the GCR disk image.  */

int fsimage_gcr_write_sector(disk_image_t *image, BYTE *buf,
                                unsigned int track, unsigned int sector)
{
    disk_track_t raw;

    if (track > image->ltracks || track < 1) {
        log_error(fsimage_gcr_log,
                  "Track %i out of bounds.  Cannot write GCR sector",
                  track);
        return -1;
    }

    raw.data = NULL;

    if (fsimage_gcr_read_track(image, (track - 1) * 2, 0, &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Cannot read track %i from GCR image.", track);
        lib_free(raw.data);
        return -1;
    }

    if (gcr_write_sector(&raw, buf, sector) < 0) {
        log_error(fsimage_gcr_log,
                  "Could not find track %i sector %i in disk image",
                  track, sector);
        lib_free(raw.data);
        return -1;
    }

    if (fsimage_gcr_write_track(image, (track - 1) * 2, 0, &raw) < 0) {
        log_error(fsimage_gcr_log,
                  "Failed writing track %i to disk image.", track);
        lib_free(raw.data);
        return -1;
    }

    lib_free(raw.data);
    return 0;
}

int fsimage_gcr_create(disk_image_t *image, unsigned int type)
{
    fsimage_t *fsimage = image->media.fsimage;
    BYTE gcr_header[12], gcr_track[NUM_MAX_BYTES_TRACK + 2], *gcrptr;
    DWORD gcr_track_p[MAX_TRACKS_1541 * 2];
    DWORD gcr_speed_p[MAX_TRACKS_1541 * 2];
    unsigned int track, sector;
    gcr_header_t header;
    BYTE rawdata[256];
    int i;

    strcpy((char *)gcr_header, "GCR-1541");

    gcr_header[8] = 0;
    gcr_header[9] = MAX_TRACKS_1541 * 2;
    gcr_header[10] = NUM_MAX_BYTES_TRACK % 256;
    gcr_header[11] = NUM_MAX_BYTES_TRACK / 256;

    if (fwrite((char *)gcr_header, sizeof(gcr_header), 1, fsimage->fd) < 1) {
        log_error(fsimage_gcr_log, "Cannot write GCR header.");
        return -1;
    }

    memset(gcr_track_p, 0, sizeof(gcr_track_p));
    memset(gcr_speed_p, 0, sizeof(gcr_speed_p));

    for (track = 0; track < NUM_TRACKS_1541; track++) {
        gcr_track_p[track * 2] = 12 + MAX_TRACKS_1541 * 16 + track * (NUM_MAX_BYTES_TRACK + 2);
        gcr_speed_p[track * 2] = disk_image_speed_map_1541(track + 1);
    }

    if (util_dword_write(fsimage->fd, gcr_track_p, util_arraysize(gcr_track_p)) < 0) {
        log_error(fsimage_gcr_log, "Cannot write track header.");
        return -1;
    }
    if (util_dword_write(fsimage->fd, gcr_speed_p, util_arraysize(gcr_speed_p)) < 0) {
        log_error(fsimage_gcr_log, "Cannot write speed header.");
        return -1;
    }
    memset(rawdata, 0, sizeof(rawdata));

    header.id1 = image->diskid[0];
    header.id2 = image->diskid[1];
    for (track = 1; track <= NUM_TRACKS_1541; track++) {
        gcrptr = gcr_track;
        i = disk_image_raw_track_size_1541(track);
        *gcrptr++ = i % 256;
        *gcrptr++ = i / 256;
        memset(gcrptr, 0x55, NUM_MAX_BYTES_TRACK);

        header.track = track;
        for (sector = 0;
        sector < disk_image_sector_per_track(image, track);
        sector++) {

            header.sector = sector;
            gcr_convert_sector_to_GCR(rawdata, gcrptr, &header, 9, 5, 0);

            gcrptr += SECTOR_GCR_SIZE_WITH_HEADER + disk_image_gap_size_1541(track);
        }
        if (fwrite((char *)gcr_track, sizeof(gcr_track), 1, fsimage->fd) < 1 ) {
            log_error(fsimage_gcr_log, "Cannot write track data.");
            return -1;
        }
    }

   return 0;
}

int fsimage_gcr_probe(disk_image_t *image)
{
    int trackfield;
    BYTE header[32];
    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    fseek(fsimage->fd, 0, SEEK_SET);
    if (fread((BYTE *)header, sizeof (header), 1, fsimage->fd) < 1) {
        return 0;
    }

    if (strncmp("GCR-1541", (char*)header, 8))
        return 0;

    if (header[8] != 0) {
        log_error(fsimage_gcr_log,
                  "Import GCR: Unknown GCR image version %i.",
                  (int)header[8]);
        return 0;
    }

    if (header[9] < NUM_TRACKS_1541 * 2 || header[9] > MAX_TRACKS_1541 * 2) {
        log_error(fsimage_gcr_log,
                  "Import GCR: Invalid number of tracks (%i).",
                  (int)header[9]);
        return 0;
    }

    trackfield = header[10] + header[11] * 256;
    if (trackfield != NUM_MAX_BYTES_TRACK) {
        log_error(fsimage_gcr_log,
                  "Import GCR: Invalid track field number %i.",
                  trackfield);
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_G64;
    image->type_name = "G64";
    image->loffset = 0;
    image->lblocks = MAX_BLOCKS_1541;
    image->ltracks = NUM_TRACKS_1541;
    image->ptracks = header[9];
    image->sides = 1;
    fsimage_error_info_destroy(fsimage);
    return 1;
}
/*-----------------------------------------------------------------------*/

void fsimage_gcr_init(void)
{
    fsimage_gcr_log = log_open("Filesystem Image GCR");
}

