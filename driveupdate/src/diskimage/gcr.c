/*
 * gcr.c - GCR handling.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Daniel Sladic <sladic@eecg.toronto.edu>
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

/* #define DEBUG_GCR */

#ifdef DEBUG_GCR
#define DBG(_x_)  log_debug _x_
#else
#define DBG(_x_)
#endif

#include "vice.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcr.h"
#include "lib.h"
#include "log.h"
#include "types.h"


static const BYTE GCR_conv_data[16] =
    { 0x0a, 0x0b, 0x12, 0x13,
      0x0e, 0x0f, 0x16, 0x17,
      0x09, 0x19, 0x1a, 0x1b,
      0x0d, 0x1d, 0x1e, 0x15 };

static const BYTE From_GCR_conv_data[32] =
    { 0,  0,  0,  0,  0,  0,  0,  0,
      0,  8,  0,  1,  0, 12,  4,  5,
      0,  0,  2,  3,  0, 15,  6,  7,
      0,  9, 10, 11,  0, 13, 14,  0 };


static void gcr_convert_4bytes_to_GCR(BYTE *source, BYTE *dest)
{
    int i;
    register unsigned int tdest = 0;    /* at least 16 bits for overflow shifting */

    for (i = 2; i < 10; i += 2, source++, dest++)
    {
        tdest <<= 5;  /* make room for the upper nybble */
        tdest  |= GCR_conv_data[(*source) >> 4];

        tdest <<= 5;  /* make room for the lower nybble */
        tdest  |= GCR_conv_data[(*source) & 0x0f];

        *dest   = (BYTE)(tdest >> i);
    }

    *dest   = (BYTE)tdest;
}

static void gcr_convert_GCR_to_4bytes(BYTE *source, BYTE *dest)
{
    int i;
        /* at least 24 bits for shifting into bits 16...20 */
    register DWORD tdest = *source;

    tdest <<= 13;

    for (i = 5; i < 13; i += 2, dest++)
    {
        source++;
        tdest  |= ((DWORD)(*source)) << i;

            /*  "tdest >> 16" could be optimized to a word
             *  aligned access, hopefully the compiler does
             *  this for us (in a portable way)
             */
        *dest   = From_GCR_conv_data[ (tdest >> 16) & 0x1f ] << 4;
        tdest <<= 5;

        *dest  |= From_GCR_conv_data[ (tdest >> 16) & 0x1f ];
        tdest <<= 5;
    }
}

void gcr_convert_sector_to_GCR(BYTE *buffer, BYTE *ptr, unsigned int track,
                               unsigned int sector, BYTE diskID1, BYTE diskID2,
                               BYTE error_code)
{
    int i;
    BYTE buf[4], chksum;

    if (error_code == 29)
        diskID1 ^= 0xff;

    memset(ptr, 0xff, 5);       /* Sync */
    ptr += 5;

    chksum = (error_code == 27) ? 0xff : 0x00;
    chksum ^= sector ^ track ^ diskID2 ^ diskID1;
    buf[0] = (error_code == 20) ? 0xff : 0x08;
    buf[1] = chksum;
    buf[2] = sector;
    buf[3] = track;
    gcr_convert_4bytes_to_GCR(buf, ptr);
    ptr += 5;

    buf[0] = diskID2;
    buf[1] = diskID1;
    buf[2] = buf[3] = 0x0f;
    gcr_convert_4bytes_to_GCR(buf, ptr);
    ptr += 5;

    ptr += 9;                   /* Gap */

    memset(ptr, 0xff, 5);       /* Sync */
    ptr += 5;

    chksum = (error_code == 23) ? 0xff : 0x00;
    buf[0] = (error_code == 22) ? 0xff : 0x07;
    memcpy(buf + 1, buffer, 3);
    chksum ^= buffer[0] ^ buffer[1] ^ buffer[2];
    gcr_convert_4bytes_to_GCR(buf, ptr);
    buffer += 3;
    ptr += 5;

    for (i = 0; i < 63; i++) {
        chksum ^= buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3];
        gcr_convert_4bytes_to_GCR(buffer, ptr);
        buffer += 4;
        ptr += 5;
    }

    buf[0] = buffer[0];
    buf[1] = chksum ^ buffer[0];
    buf[2] = buf[3] = 0;
    gcr_convert_4bytes_to_GCR(buf, ptr);
}

static void gcr_convert_GCR_to_sector(BYTE *buffer, BYTE *ptr,
                               BYTE *start, unsigned int track_size)
{
    BYTE *offset = ptr;
    BYTE *end = start + track_size;
    BYTE gcr[5], b;
    int i, j, shift;

    /* additional 1 bits are part of the previous sync and must
       be shifted out. so check/count these here */
    shift = 0;
    b = offset[0];
    while (b & 0x80) {
        b <<= 1;
        shift++;
    }

    for (i = 0; i < 65; i++) {
        /* get 5 bytes of gcr data */
        for (j = 0; j < 5; j++) {
            offset++;
            if (offset >= end) {
                offset = start;
            }
            if (shift) {
                gcr[j] = b | ((offset[0] << shift) >> 8);
                b = offset[0] << shift;
            } else {
                gcr[j] = b;
                b = offset[0];
            }
        }
        gcr_convert_GCR_to_4bytes(gcr, buffer);
        buffer += 4;
    }
}

static BYTE *gcr_find_sector_header(unsigned int track, unsigned int sector,
                             BYTE *start, unsigned int track_size)
{
    BYTE *offset = start;
    BYTE *end = start + track_size;
    BYTE gcr[5], header[4], b;
    int i, wrap_over = 0, shift;
    unsigned int sync_count;

    sync_count = 0;

    while ((offset < end) && !wrap_over) {
        /* find next sync start */
        while (*offset != 0xff) {
            offset++;
            if (offset >= end)
                return NULL;
        }
        /* skip to sync end */
        while (*offset == 0xff) {
            offset++;
            if (offset == end) {
                offset = start;
                wrap_over = 1;
            }
            /* Check for killer tracks.  */
            if ((++sync_count) >= track_size)
                return NULL;
        }
        /* shift out additional 1 bits, which are part of the sync */
        shift = 0;
        b = offset[0];
        while (b & 0x80) {
            b <<= 1;
            shift++;
        }
        for (i = 0; i < 5; i++) {
            offset++;
            if (offset >= end) {
                offset = start;
                wrap_over = 1;
            }
            if (shift) {
                gcr[i] = b | ((offset[0] << shift) >> 8);
                b = offset[0] << shift;
            } else {
                gcr[i] = b;
                b = offset[0];
            }
        }

        gcr_convert_GCR_to_4bytes(gcr, header);
        if (header[0] == 0x08) {
            /* FIXME: Add some sanity checks here.  */
            if (header[2] == sector && header[3] == track) {
                DBG(("GCR: shift: %d hdr: %02x %02x sec:%02d trk:%02d", shift, header[0], header[1], header[2], header[3]));
                return offset;
            }
        }
    }
    return NULL;
}

static BYTE *gcr_find_sector_data(BYTE *offset, BYTE *start, unsigned int track_size)
{
    BYTE *end = start + track_size;
    int header = 0;

    while (*offset != 0xff) {
        offset++;
        if (offset >= end)
            offset = start;
        header++;
        if (header >= 500)
            return NULL;
    }

    while (*offset == 0xff) {
        offset++;
        if (offset == end)
            offset = start;
    }
    return offset;
}

int gcr_read_sector(BYTE *raw, unsigned int size, BYTE *data,
                    unsigned int track, unsigned int sector)
{
    BYTE buffer[260], *offset;

    offset = gcr_find_sector_header(track, sector, raw, size);
    if (offset == NULL)
        return -1;

    offset = gcr_find_sector_data(offset, raw, size);
    if (offset == NULL)
        return -2;

    gcr_convert_GCR_to_sector(buffer, offset, raw, size);

    if (buffer[0] != 0x07)
        return -3;

    memcpy(data, buffer + 1, 256);

    return 0;
}

int gcr_write_sector(BYTE *raw, unsigned int size, BYTE *data,
                     unsigned int track, unsigned int sector)
{
    BYTE buffer[260], *offset, *buf;
    BYTE gcr[5], chksum, b;
    int i, j, shift;

    offset = gcr_find_sector_header(track, sector, raw, size);
    if (offset == NULL)
        return -1;

    offset = gcr_find_sector_data(offset, raw, size);
    if (offset == NULL)
        return -2;

    shift = 0;
    b = offset[0];
    while (b & 0x80) {
        b <<= 1;
        shift++;
    }
    b = offset[0] & (0xff00 >> shift);

    buffer[0] = 0x07;
    memcpy(buffer + 1, data, 256);
    chksum = buffer[1];
    for (i = 2; i < 257; i++)
        chksum ^= buffer[i];
    buffer[257] = chksum;
    buffer[258] = buffer[259] = 0;

    buf = buffer;

    for (i = 0; i < 65; i++) {
        gcr_convert_4bytes_to_GCR(buf, gcr);
        buf += 4;
        for (j = 0; j < 5; j++) {
            if (shift) {
                offset[0] = b | (gcr[j] >> shift);
                b = (gcr[j] << 8) >> shift;
            } else {
                offset[0] = gcr[j];
            }
            offset++;
            if (offset == raw + size)
                offset = raw;
        }
    }
    offset[0] = b | (offset[0] & (0xff >> shift));

    return 0;
}
