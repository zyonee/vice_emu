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
#include "mfm.h"
#include "log.h"
#include "types.h"
#include "util.h"
#include "x64.h"
#include "cbmdos.h"
#include "lib.h"


static log_t fsimage_flat_log = LOG_ERR;

const int mfm_data_rates[4] = {500, 300, 250, 1000}; /* kbit/s */

/*-----------------------------------------------------------------------*/
/* Calculates sector offset in image file from logical address */
static int fsimage_calc_logical_offset(disk_image_t *image, unsigned int track,
                            unsigned int sector)
{
    int sectors, i;

    if (track < 1 || track > image->ltracks
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
static int fsimage_flat_seek_track(disk_image_t *image, unsigned int track, unsigned head)
{
    fsimage_t *fsimage = image->media.fsimage;
    int i, offset, errorinfo;

    switch (image->type) {
    case DISK_IMAGE_TYPE_D81:
    case DISK_IMAGE_TYPE_D1M:
    case DISK_IMAGE_TYPE_D2M:
    case DISK_IMAGE_TYPE_D4M:
        offset = (128 << image->mfm.sector_size) * image->mfm.sectors * (track * 2 + head);
        errorinfo = offset / 256;
        break;
    default:
        offset = 0;

        track += head ? image->gcr.doublesided : 0;
        for (i = 1; i < track; i++) {
            offset += disk_image_sector_per_track(image, i);
        }

        errorinfo = offset;
        offset *= 256;

        if (image->type == DISK_IMAGE_TYPE_X64) {
            offset += X64_HEADER_LENGTH;
        }
    }

    fseek(fsimage->fd, offset, SEEK_SET);
    if (errorinfo < image->lblocks && fsimage->errors) {
        return errorinfo;
    }
    return -1;
}


/*-----------------------------------------------------------------------*/
/* Read an entire track from the disk image. */

static int fsimage_flat_read_track_gcr(disk_image_t *image, unsigned int track,
                                       unsigned int head, disk_track_t *raw)
{
    BYTE buffer[256], *data;
    unsigned int sector;
    int max_sector, gap;
    int rc, errorinfo;
    gcr_header_t header;
    fsimage_t *fsimage = image->media.fsimage;

    switch (image->type) {
    case DISK_IMAGE_TYPE_D80:
    case DISK_IMAGE_TYPE_D82:
        header.track = track + 1;
        raw->size = disk_image_raw_track_size_8050(header.track);
        gap = disk_image_gap_size_8050(header.track);
        break;
    default:
        header.track = track / 2 + 1;
        raw->size = disk_image_raw_track_size_1541(header.track);
        gap = disk_image_gap_size_1541(header.track);
        break;
    }
    raw->data = lib_realloc(raw->data, raw->size);

    if (track >= image->ptracks || head >= image->sides) {
        memset(raw->data, 0x00, raw->size);
        return -1;
    }

    header.id1 = image->gcr.diskid[0];
    header.id2 = image->gcr.diskid[1];

    max_sector = disk_image_sector_per_track(image, header.track);

    memset(raw->data, 0x55, raw->size);

    errorinfo = fsimage_flat_seek_track(image, header.track, head);

    header.track += head ? image->gcr.doublesided : 0;

    data = raw->data;
    for (sector = 0; sector < max_sector; sector++) {
        if (fread((char *)buffer, 256, 1, fsimage->fd) < 1) {
            log_error(fsimage_flat_log,
                    "Error reading T:%d H:%d S:%d from disk image.",
                    track / 2 + 1, head, sector);
        } else {
            rc = (errorinfo >= 0) ? fsimage->errors[errorinfo++] : 0;

            header.sector = sector;
            gcr_convert_sector_to_GCR(buffer, data, &header, image->gcr.gap2, image->gcr.sync, (BYTE)(rc));
        }

        data += SECTOR_GCR_SIZE_WITH_HEADER + image->gcr.gap2 + gap + image->gcr.sync;
    }
    return 0;
}

static int fsimage_flat_read_track_mfm(disk_image_t *image, unsigned int track,
                                       unsigned int head, disk_track_t *raw)
{
    BYTE *data, *sync, *buffer;
    int sector;
    mfm_header_t header;
    fsimage_t *fsimage = image->media.fsimage;

    raw->size = 25 * mfm_data_rates[image->mfm.rate];
    raw->data = lib_realloc(raw->data, raw->size * 2);
    data = raw->data;
    sync = raw->data + raw->size;

    if (track >= image->ptracks || head >= image->sides) {
        memset(raw->data, 0x00, raw->size * 2);
        return -1;
    }

    memset(data, 0x4e, raw->size);
    memset(sync, 0x00, raw->size);

    if (image->mfm.iso) {
        data += 32; /* GAP 4a */
        sync += 32;
    } else {
        data += 80;
        sync += 80;
        memset(sync, 0x00, 12);
        memset(data, 0x00, 12);    /* Sync */
        data += 12;
        sync += 12;
        memset(data, 0xa1, 3); 
        memset(sync, 0x01, 3);
        data += 3;
        sync += 3;
        *data++ = 0xfc; /* Index mark */
        *sync++ = 0x00;
        data += 50;
        sync += 50;
    }

    fsimage_flat_seek_track(image, track, head ^ image->mfm.head_swap);
    buffer = lib_malloc(128 << image->mfm.sector_size);

    header.track = track;
    header.head = head ^ image->mfm.head_swap;
    header.sector_size = image->mfm.sector_size;
    for (sector = 1; sector <= image->mfm.sectors; sector++) {
        if (fread((char *)buffer, (128 << image->mfm.sector_size), 1, fsimage->fd) < 1) {
            log_error(fsimage_flat_log,
                    "Error reading T:%d H:%d S:%d from disk image.",
                    track, head, sector);
        } else {
            header.sector = sector;
            mfm_convert_sector_to_MFM(buffer, data, sync, &header, image->mfm.gap2);
        }
        data += 12 + 3 + 1 + 4 + 2 + image->mfm.gap2 + 12 + 3 + 1 + (128 << header.sector_size) + 2;
        sync += 12 + 3 + 1 + 4 + 2 + image->mfm.gap2 + 12 + 3 + 1 + (128 << header.sector_size) + 2;
        data += image->mfm.gap3;
        sync += image->mfm.gap3;
    }
    lib_free(buffer);
    return 0;
}

int fsimage_flat_read_track(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    switch (image->type) {
    case DISK_IMAGE_TYPE_D64:
    case DISK_IMAGE_TYPE_X64:
    case DISK_IMAGE_TYPE_D67:
    case DISK_IMAGE_TYPE_D71:
    case DISK_IMAGE_TYPE_D80:
    case DISK_IMAGE_TYPE_D82:
        return fsimage_flat_read_track_gcr(image, track, head, raw);
    case DISK_IMAGE_TYPE_D81:
    case DISK_IMAGE_TYPE_D1M:
    case DISK_IMAGE_TYPE_D2M:
    case DISK_IMAGE_TYPE_D4M:
        return fsimage_flat_read_track_mfm(image, track, head, raw);
    }
    return -1;
}

/*-----------------------------------------------------------------------*/
/* Write an entire track to the disk image. */
int fsimage_flat_write_track_gcr(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    unsigned int sector, max_sector;
    BYTE buffer[256];
    fsimage_t *fsimage = image->media.fsimage;
    unsigned int ltrack;
    int rc, errorinfo;

    switch (image->type) {
    case DISK_IMAGE_TYPE_D80:
    case DISK_IMAGE_TYPE_D82:
        track = track + 1;
        break;
    default:
        track = track / 2 + 1;
        break;
    }
    ltrack = track;
    if (image->type == DISK_IMAGE_TYPE_D71 && head !=0) {
        ltrack += 35;
    }
    max_sector = disk_image_sector_per_track(image, track);

    errorinfo = fsimage_flat_seek_track(image, track, head);

    for (sector = 0; sector < max_sector; sector++) {
        rc = gcr_read_sector(raw, buffer, sector);
        if (errorinfo >= 0) {
            fsimage->errors[errorinfo + sector] = rc;
        }
        switch (rc) {
        case CBMDOS_FDC_ERR_DCHECK:
        case CBMDOS_FDC_ERR_OK:
            if (fwrite((char *)buffer, 256, 1, fsimage->fd) < 1) {
                log_error(fsimage_flat_log, "Error writing T:%d H:%d S:%d to disk image.",
                        track, head, sector);
            }
            continue;
        case CBMDOS_FDC_ERR_HEADER:
            log_error(fsimage_flat_log, "Could not find header of T:%d H:%d S:%d.",
                    track, head, sector);
            break;
        case CBMDOS_FDC_ERR_SYNC:
            log_error(fsimage_flat_log, "Could not find data sync of T:%d H:%d S:%d.",
                    track, head, sector);
            break;
        case CBMDOS_FDC_ERR_NOBLOCK:
            log_error(fsimage_flat_log, "Could not find data block id of T:%d H:%d S:%d.",
                    track, head, sector);
            break;
        default:
            log_error(fsimage_flat_log, "Unknown error at T:%d H:%d S:%d.", track, head, sector);
        } 
        fread((char *)buffer, 256, 1, fsimage->fd); /* skip */
    }

    if (fsimage->errors) {
        fseek(fsimage->fd, image->lblocks * 256 + errorinfo, SEEK_SET);
        fwrite(fsimage->errors + errorinfo, 1, max_sector, fsimage->fd);
    }

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);

    return 1; /* always different due to timing of read/write switch */
}

int fsimage_flat_write_track_mfm(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    unsigned int sector, res;
    BYTE *buffer;
    disk_track_t old_track;
    fsimage_t *fsimage = image->media.fsimage;
    mfm_header_t header;

    fsimage_flat_seek_track(image, track, head ^ image->mfm.head_swap);

    buffer = lib_malloc(128 << image->mfm.sector_size);

    header.track = track;
    header.head = head ^ image->mfm.head_swap;
    header.sector_size = image->mfm.sector_size;
    for (sector = 0; sector < image->mfm.sectors; sector++) {
        header.sector = sector + 1;
        switch (mfm_read_sector(raw, buffer, &header)) {
        case -1:
            log_error(fsimage_flat_log, "Could not find header of T:%d H:%d S:%d.",
                      track, head, sector);
            continue;
        } 
        if (fwrite((char *)buffer, 128 << image->mfm.sector_size, 1, fsimage->fd) < 1) {
            log_error(fsimage_flat_log, "Error writing T:%d H:%d S:%d to disk image.",
                    track, head, sector);
        }
    }
    lib_free(buffer);

    /* Make sure the stream is visible to other readers.  */
    fflush(fsimage->fd);

    memset(&old_track, 0, sizeof(old_track));
    res = fsimage_flat_read_track_mfm(image, track, head, &old_track);
    res |= raw->size != old_track.size || memcmp(raw->data, old_track.data, 2 * old_track.size); 
    lib_free(old_track.data);

    return res; /* return if there was any change */
}

int fsimage_flat_write_track(disk_image_t *image, unsigned int track,
                            unsigned int head, disk_track_t *raw)
{
    if (track >= image->ptracks || head >= image->sides) {
        return -1;
    }

    switch (image->type) {
    case DISK_IMAGE_TYPE_D64:
    case DISK_IMAGE_TYPE_X64:
    case DISK_IMAGE_TYPE_D67:
    case DISK_IMAGE_TYPE_D71:
    case DISK_IMAGE_TYPE_D80:
    case DISK_IMAGE_TYPE_D82:
        return fsimage_flat_write_track_gcr(image, track, head, raw);
    case DISK_IMAGE_TYPE_D81:
    case DISK_IMAGE_TYPE_D1M:
    case DISK_IMAGE_TYPE_D2M:
    case DISK_IMAGE_TYPE_D4M:
        return fsimage_flat_write_track_mfm(image, track, head, raw);
    }
    return -1;
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

    if (fsimage->errors != NULL) {
        return cbmdos_fdc_to_ipe(fsimage->errors[sectors]);
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

static int fsimage_flat_probe_general(disk_image_t *image, int size, int id)
{
    int i;
    BYTE block[256];
    fsimage_t *fsimage;
    size_t len;

    fsimage = image->media.fsimage;

    len = util_file_length(fsimage->fd);
    if (len != size * 256 && len != size * 257) {
        return 0;
    }

    rewind(fsimage->fd);

    for (i = 0; i < size; i++) {
        if (fread(block, 1, 256, fsimage->fd) != 256) {
            lib_free(fsimage->errors);
            return 0;
        }
        if (i == (id >> 8)) {
            image->gcr.doublesided = block[0x03] & 0x80;
            image->gcr.diskid[0] = block[id & 0xff];
            image->gcr.diskid[1] = block[(id + 1) & 0xff];
        }
    }

    if (len == size * 257) {
        fsimage->errors = lib_calloc(1, size);
        if (fread(fsimage->errors, 1, size, fsimage->fd)
            < size) {
            lib_free(fsimage->errors);
            fsimage->errors = NULL;
            return 0;
        }
    }
    return 1;
}

static int fsimage_flat_probe_d64(disk_image_t *image)
{
    /*** detect 35..42 track d64 image, determine image parameters.
         Walk from 35 to 42, calculate expected image file size for each track,
         and compare this with the size of the given image. */
    int i;

    for (i = NUM_BLOCKS_1541; i <= MAX_BLOCKS_1541; i += 17) {
        if (fsimage_flat_probe_general(image, i, 17 * 21 * 256 + BAM_ID_1541)) {

            image->type = DISK_IMAGE_TYPE_D64;
            image->type_name = "D64";
            image->loffset = 0;
            image->lblocks = i;
            image->ltracks = (i - (NUM_BLOCKS_1541 - 35 * 17)) / 17;
            image->ptracks = image->ltracks * 2;
            image->sides = 1;
            image->gcr.doublesided = 0;
            image->gcr.gap2 = 9;
            image->gcr.sync = 5;
            return 1;
        }
    }
    return 0;
}


static int fsimage_flat_probe_d67(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_2040, 17 * 21 * 256 + BAM_ID_2040)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D67;
    image->type_name = "D67";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_2040;
    image->ltracks = NUM_TRACKS_2040;
    image->ptracks = NUM_TRACKS_2040 * 2;
    image->sides = 1;
    image->gcr.doublesided = 0;
    image->gcr.gap2 = 10;
    image->gcr.sync = 5;
    return 1;
}

static int fsimage_flat_probe_d71(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_1571, 17 * 21 * 256 + BAM_ID_1571)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D71;
    image->type_name = "D71";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1571;
    image->ltracks = NUM_TRACKS_1571;
    image->ptracks = NUM_TRACKS_1571;
    image->sides = 2;
    image->gcr.doublesided = image->gcr.doublesided ? 35 : 0;
    image->gcr.gap2 = 9;
    image->gcr.sync = 5;
    return 1;
}

static int fsimage_flat_probe_d81(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_1581, -1)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D81;
    image->type_name = "D81";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1581;
    image->ltracks = NUM_TRACKS_1581;
    image->ptracks = NUM_TRACKS_1581;
    image->sides = 2;
    image->mfm.sectors = 10;
    image->mfm.sector_size = 2;
    image->mfm.head_swap = 1;
    image->mfm.rate = 2;
    image->mfm.iso = 1;
    image->mfm.gap2 = 22;
    image->mfm.gap3 = 35;
    return 1;
}

static int fsimage_flat_probe_d80(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_8050, 38 * 29 * 256 + BAM_ID_8050)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D80;
    image->type_name = "D80";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_8050;
    image->ltracks = NUM_TRACKS_8050;
    image->ptracks = NUM_TRACKS_8050;
    image->sides = 1;
    image->gcr.doublesided = 0;
    image->gcr.gap2 = 24;
    image->gcr.sync = 6;
    return 1;
}

static int fsimage_flat_probe_d82(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_8250, 38 * 29 * 256 + BAM_ID_8250)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D82;
    image->type_name = "D82";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_8250;
    image->ltracks = NUM_TRACKS_8250;
    image->ptracks = NUM_TRACKS_8250 / 2;
    image->sides = 2;
    image->gcr.doublesided = 77;
    image->gcr.gap2 = 24;
    image->gcr.sync = 6;
    return 1;
}

static int fsimage_flat_probe_x64(disk_image_t *image)
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
    image->ptracks = image->ltracks * 2;
    image->sides = 1;

    return 1;
}
static int fsimage_flat_probe_d1m(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_1000, -1)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D1M;
    image->type_name = "D1M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_1000;
    image->ltracks = NUM_TRACKS_1000;
    image->ptracks = 81;
    image->sides = 2;
    image->mfm.sectors = 10;
    image->mfm.sector_size = 2;
    image->mfm.head_swap = 1;
    image->mfm.rate = 2;
    image->mfm.iso = 0;
    image->mfm.gap2 = 22;
    image->mfm.gap3 = 35;
    return 1;
}

static int fsimage_flat_probe_d2m(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_2000, -1)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D2M;
    image->type_name = "D2M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_2000;
    image->ltracks = NUM_TRACKS_2000;
    image->ptracks = 81;
    image->sides = 2;
    image->mfm.sectors = 10;
    image->mfm.sector_size = 3;
    image->mfm.head_swap = 1;
    image->mfm.rate = 0;
    image->mfm.iso = 0;
    image->mfm.gap2 = 22;
    image->mfm.gap3 = 100;
    return 1;
}

static int fsimage_flat_probe_d4m(disk_image_t *image)
{
    if (!fsimage_flat_probe_general(image, NUM_BLOCKS_4000, -1)) {
        return 0;
    }

    image->type = DISK_IMAGE_TYPE_D4M;
    image->type_name = "D4M";
    image->loffset = 0;
    image->lblocks = NUM_BLOCKS_4000;
    image->ltracks = NUM_TRACKS_4000;
    image->ptracks = 81;
    image->sides = 2;
    image->mfm.sectors = 20;
    image->mfm.sector_size = 3;
    image->mfm.head_swap = 1;
    image->mfm.rate = 3;
    image->mfm.iso = 0;
    image->mfm.gap2 = 41;
    image->mfm.gap3 = 100;
    return 1;
}

int fsimage_flat_open(disk_image_t *image)
{
    return fsimage_flat_probe_d64(image)
        || fsimage_flat_probe_d67(image)
        || fsimage_flat_probe_d71(image)
        || fsimage_flat_probe_d81(image)
        || fsimage_flat_probe_d80(image)
        || fsimage_flat_probe_d82(image)
        || fsimage_flat_probe_x64(image)
        || fsimage_flat_probe_d1m(image)
        || fsimage_flat_probe_d2m(image)
        || fsimage_flat_probe_d4m(image);
}

void fsimage_flat_close(disk_image_t *image)
{
    fsimage_t *fsimage = image->media.fsimage;

    if (fsimage->errors) {
        lib_free(fsimage->errors);
        fsimage->errors = NULL;
    }
}
/*-----------------------------------------------------------------------*/

void fsimage_flat_init(void)
{
    fsimage_flat_log = log_open("Filesystem Image Flat");
}

