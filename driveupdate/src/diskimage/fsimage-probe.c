/*
 * fsimage-probe.c - Probe disk images.
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
#include "fsimage-probe.h"
#include "fsimage.h"
#include "lib.h"
#include "log.h"
#include "types.h"
#include "util.h"
#include "x64.h"

static log_t disk_image_probe_log = LOG_ERR;

static int disk_image_check_general(disk_image_t *image, int size1, int size2, int id)
{
    int i;
    BYTE block[256];
    fsimage_t *fsimage;
    int checkimage_errorinfo;
    size_t len;

    fsimage = image->media.fsimage;

    len = util_file_length(fsimage->fd);
    if (len == size1) {
	checkimage_errorinfo = 0;
	fsimage_error_info_destroy(fsimage);
    } else if (len == size2) {
	checkimage_errorinfo = 1;
        fsimage_error_info_create(fsimage);
    } else {
	return 0;
    }

    rewind(fsimage->fd);
    size1 /= 256;

    for (i = 0; i < size1; i++) {
	if (fread(block, 1, 256, fsimage->fd) != 256) {
	    return 0;
	}
	if (i == id) {
	    image->diskID[0] = block[0xa2];
	    image->diskID[1] = block[0xa3];
	}
    }

    if (checkimage_errorinfo
	&& fread(fsimage->error_info, 1, size1, fsimage->fd)
            < size1) {
	return 0;
    }

    if (fgetc(fsimage->fd) != EOF) {
	log_error(disk_image_probe_log, "Disk image too large.");
	return 0;
    }
    return 1;
}


static int disk_image_check_for_d64(disk_image_t *image)
{
    /*** detect 35..42 track d64 image, determine image parameters.
         Walk from 35 to 42, calculate expected image file size for each track,
         and compare this with the size of the given image. */

    int checkimage_tracks, checkimage_errorinfo, i;
    size_t checkimage_blocks, checkimage_realsize;
    fsimage_t *fsimage;
    BYTE block[256];

    fsimage = image->media.fsimage;

    checkimage_errorinfo = 0;
    checkimage_realsize = util_file_length(fsimage->fd);
    checkimage_tracks = NUM_TRACKS_1541; /* start at track 35 */
    checkimage_blocks = D64_FILE_SIZE_35 / 256;

    while (1) {
        /* check if image matches "checkimage_tracks" */
        if (checkimage_realsize == checkimage_blocks * 256) {
            /* image file matches size-with-no-error-info */
            checkimage_errorinfo = 0;
            break;

        } else if (checkimage_realsize == checkimage_blocks * 256
            + checkimage_blocks) {
            /* image file matches size-with-error-info */
            checkimage_errorinfo = 1;
            break;
        }

        /* try next track (all tracks from 35 to 42 have 17 blocks) */
        checkimage_tracks++;
        checkimage_blocks += 17;

        /* we tried them all up to 42, none worked, image must be corrupt */
        if (checkimage_tracks > MAX_TRACKS_1541)
            return 0;
    }

    rewind(fsimage->fd);

    for (i = 0; i < checkimage_blocks; i++) {
	if (fread(block, 1, 256, fsimage->fd) != 256) {
	    return 0;
	}
	if (i == 17 * 21) {
	    image->diskID[0] = block[0xa2];
	    image->diskID[1] = block[0xa3];
	}
    }

    /*** set parameters in image structure, read error info */
    image->type = DISK_IMAGE_TYPE_D64;
    image->type_name = "D64";
    image->loffset = 0;
    image->lblocks = checkimage_blocks;
    image->ltracks = checkimage_tracks;
    image->ptracks = checkimage_tracks * 2;
    image->sides = 1;
    fsimage_error_info_destroy(fsimage);

    if (checkimage_errorinfo
	&& fread(fsimage->error_info, 1, checkimage_blocks, fsimage->fd)
            < checkimage_blocks) {
	return 0;
    }
    return 1;
}


static int disk_image_check_for_d67(disk_image_t *image)
{
    if (!disk_image_check_general(image, D67_FILE_SIZE, D67_FILE_SIZE, 17 * 21)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D67;
    image->type_name = "D67";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_2040;
    image->ltracks = NUM_TRACKS_2040;
    image->ptracks = NUM_TRACKS_2040 * 2;
    image->sides = 1;
    return 1;
}

static int disk_image_check_for_d71(disk_image_t *image)
{
    if (!disk_image_check_general(image, D71_FILE_SIZE, D71_FILE_SIZE_E, 17 * 21)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D71;
    image->type_name = "D71";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1571;
    image->ltracks = NUM_TRACKS_1571;
    image->ptracks = NUM_TRACKS_1571;
    image->sides = 2;
    return 1;
}

static int disk_image_check_for_d81(disk_image_t *image)
{
    if (!disk_image_check_general(image, D81_FILE_SIZE, D81_FILE_SIZE_E, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D81;
    image->type_name = "D81";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1581;
    image->ltracks = NUM_TRACKS_1581;
    image->ptracks = NUM_TRACKS_1581;
    image->sides = 2;
    return 1;
}

static int disk_image_check_for_d80(disk_image_t *image)
{
    if (!disk_image_check_general(image, D80_FILE_SIZE, D80_FILE_SIZE, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D80;
    image->type_name = "D80";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_8050;
    image->ltracks = NUM_TRACKS_8050;
    image->ptracks = NUM_TRACKS_8050;
    image->sides = 1;
    return 1;
}

static int disk_image_check_for_d82(disk_image_t *image)
{
    if (!disk_image_check_general(image, D82_FILE_SIZE, D82_FILE_SIZE, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D82;
    image->type_name = "D82";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_8250;
    image->ltracks = NUM_TRACKS_8250;
    image->ptracks = NUM_TRACKS_8250 / 2;
    image->sides = 2;
    return 1;
}

static int disk_image_check_for_x64(disk_image_t *image)
{
    BYTE header[X64_HEADER_LENGTH];
    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    rewind(fsimage->fd);

    if (fread(header, X64_HEADER_LENGTH, 1, fsimage->fd) < 1)
        return 0;

    if (header[X64_HEADER_MAGIC_OFFSET + 0] != X64_HEADER_MAGIC_1 ||
        header[X64_HEADER_MAGIC_OFFSET + 1] != X64_HEADER_MAGIC_2 ||
        header[X64_HEADER_MAGIC_OFFSET + 2] != X64_HEADER_MAGIC_3 ||
        header[X64_HEADER_MAGIC_OFFSET + 3] != X64_HEADER_MAGIC_4)
        return 0;

    if (header[X64_HEADER_FLAGS_OFFSET + 1] > MAX_TRACKS_1541)
        return 0;

    image->type = DISK_IMAGE_TYPE_X64;
    image->type_name = "X64";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1541 + (header[X64_HEADER_FLAGS_OFFSET + 1] - 35) * 17;
    image->ltracks = header[X64_HEADER_FLAGS_OFFSET + 1];
    image->ptracks = header[X64_HEADER_FLAGS_OFFSET + 1] * 2;
    image->sides = 1;

    fsimage_error_info_destroy(fsimage);
    return 1;
}

static int disk_image_check_for_gcr(disk_image_t *image)
{
    int trackfield;
    BYTE header[32];
    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    fseek(fsimage->fd, 0, SEEK_SET);
    if (fread((BYTE *)header, sizeof (header), 1, fsimage->fd) < 1) {
        log_error(disk_image_probe_log, "Cannot read image header.");
        return 0;
    }

    if (strncmp("GCR-1541", (char*)header, 8))
        return 0;

    if (header[8] != 0) {
        log_error(disk_image_probe_log,
                  "Import GCR: Unknown GCR image version %i.",
                  (int)header[8]);
        return 0;
    }

    if (header[9] < NUM_TRACKS_1541 * 2 || header[9] > MAX_TRACKS_1541 * 2) {
        log_error(disk_image_probe_log,
                  "Import GCR: Invalid number of tracks (%i).",
                  (int)header[9]);
        return 0;
    }

    trackfield = header[10] + header[11] * 256;
    if (trackfield != 7928) {
        log_error(disk_image_probe_log,
                  "Import GCR: Invalid track field number %i.",
                  trackfield);
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_G64;
    image->type_name = "G64";
    image->loffset = 0;
    image->lblocks = MAX_BLOCKS_1541;
    image->ltracks = header[9];
    image->ptracks = header[9];
    image->sides = 1;
    fsimage_error_info_destroy(fsimage);
    return 1;
}

static int disk_image_check_for_d1m(disk_image_t *image)
{
    if (!disk_image_check_general(image, D1M_FILE_SIZE, D1M_FILE_SIZE_E, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D1M;
    image->type_name = "D1M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1000;
    image->ltracks = NUM_TRACKS_1000;
    image->ptracks = 81;
    image->sides = 2;
    return 1;
}

static int disk_image_check_for_d2m(disk_image_t *image)
{
    if (!disk_image_check_general(image, D2M_FILE_SIZE, D2M_FILE_SIZE_E, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D2M;
    image->type_name = "D2M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_2000;
    image->ltracks = NUM_TRACKS_2000;
    image->ptracks = 81;
    image->sides = 2;
    return 1;
}

static int disk_image_check_for_d4m(disk_image_t *image)
{
    if (!disk_image_check_general(image, D4M_FILE_SIZE, D4M_FILE_SIZE_E, -1)) {
	return 0;
    }

    image->type = DISK_IMAGE_TYPE_D4M;
    image->type_name = "D4M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_4000;
    image->ltracks = NUM_TRACKS_4000;
    image->ptracks = 81;
    image->sides = 2;
    return 1;
}


int fsimage_probe(disk_image_t *image)
{
    fsimage_t *fsimage = image->media.fsimage;

    if (disk_image_check_for_d64(image)
        || disk_image_check_for_d67(image)
        || disk_image_check_for_d71(image)
        || disk_image_check_for_d81(image)
        || disk_image_check_for_d80(image)
        || disk_image_check_for_d82(image)
        || disk_image_check_for_gcr(image)
        || disk_image_check_for_x64(image)
        || disk_image_check_for_d1m(image)
        || disk_image_check_for_d2m(image)
        || disk_image_check_for_d4m(image)) {

	log_verbose("%s disk image recognised: %s, %d tracks%s",
                image->type_name, fsimage->name, image->ltracks,
                image->read_only ? " (read only)." : ".");
        return 0;
    }

    return -1;
}

void fsimage_probe_init(void)
{
    disk_image_probe_log = log_open("Filesystem Image Probe");
}

