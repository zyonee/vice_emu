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
#include "cbmdos.h"
#include "diskimage.h"

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

void gcr_convert_sector_to_GCR(BYTE *buffer, BYTE *data, gcr_header_t *header,
                               BYTE error_code)
{
    int i;
    BYTE buf[4], chksum, idm;

    idm = (error_code == CBMDOS_IPE_DISK_ID_MISMATCH) ? 0xff : 0x00;

    memset(data, 0xff, 5);       /* Sync */
    data += 5;

    chksum = (error_code == CBMDOS_IPE_READ_ERROR_BCHK) ? 0xff : 0x00;
    chksum ^= header->sector ^ header->track ^ header->id2 ^ header->id1 ^ idm;
    buf[0] = (error_code == CBMDOS_IPE_READ_ERROR_BNF) ? 0xff : 0x08;
    buf[1] = chksum;
    buf[2] = header->sector;
    buf[3] = header->track;
    gcr_convert_4bytes_to_GCR(buf, data);
    data += 5;

    buf[0] = header->id2;
    buf[1] = header->id1 ^ idm;
    buf[2] = buf[3] = 0x0f;
    gcr_convert_4bytes_to_GCR(buf, data);
    data += 5;

    data += 9;                   /* Gap */

    memset(data, 0xff, 5);       /* Sync */
    data += 5;

    chksum = (error_code == CBMDOS_IPE_READ_ERROR_CHK) ? 0xff : 0x00;
    buf[0] = (error_code == CBMDOS_IPE_READ_ERROR_DATA) ? 0xff : 0x07;
    memcpy(buf + 1, buffer, 3);
    chksum ^= buffer[0] ^ buffer[1] ^ buffer[2];
    gcr_convert_4bytes_to_GCR(buf, data);
    buffer += 3;
    data += 5;

    for (i = 0; i < 63; i++) {
        chksum ^= buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3];
        gcr_convert_4bytes_to_GCR(buffer, data);
        buffer += 4;
        data += 5;
    }

    buf[0] = buffer[0];
    buf[1] = chksum ^ buffer[0];
    buf[2] = buf[3] = 0;
    gcr_convert_4bytes_to_GCR(buf, data);
}

static int gcr_find_sync(disk_track_t *raw, int p, int s) {
    int w, b;

    w = 0;
    b = raw->data[p >> 3] << (p & 7);
    while (s--) {
        if (b & 0x80) {
            w = (w << 1) | 1;
        } else {
            if (~w & 0x3ff) {
                w <<= 1;
            } else {
                return p;
            }
        }
        if (~p & 7) {
            p++;
            b <<= 1;
        } else {
            p++;
            if (p >= raw->size * 8) {
                p = 0;
            }
            b = raw->data[p >> 3];
        }
    }
    return -1;
}

static int gcr_find_sector_header(disk_track_t *raw, BYTE sector)
{
    BYTE gcr[5], header[4], b;
    BYTE *offset, *end = raw->data + raw->size;
    int i, p, p2, shift;

    p = 0;
    for (;;) {
        p2 = p;
        p = gcr_find_sync(raw, p, raw->size * 8);
        if (p <= p2) {
            break;
        }
        shift = p & 7;
        offset = raw->data + (p >> 3);

        b = offset[0] << shift;
        for (i = 0; i < 5; i++) {
            offset++;
            if (offset >= end) {
                offset = raw->data;
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
        if (header[0] == 0x08 && header[2] == sector) {
            /* Track, checksum or ID's are not checked here */
            DBG(("GCR: shift: %d hdr: %02x %02x sec:%02d trk:%02d", shift, header[0], header[1], header[2], header[3]));
            return p;
        }
    }
    return -1;
}

int gcr_read_sector(disk_track_t *raw, BYTE *data, BYTE sector)
{
    BYTE buffer[260], *buf, *offset;
    BYTE *end = raw->data + raw->size;
    BYTE gcr[5], b;
    int i, j, shift, p;

    p = gcr_find_sector_header(raw, sector);
    if (p < 0)
        return -1;

    p = gcr_find_sync(raw, p, 500 * 8);
    if (p < 0)
        return -2;

    shift = p & 7;
    offset = raw->data + (p >> 3);

    b = offset[0] << shift;
    buf = buffer;
    for (i = 0; i < 65; i++) {
        /* get 5 bytes of gcr data */
        for (j = 0; j < 5; j++) {
            offset++;
            if (offset >= end) {
                offset = raw->data;
            }
            if (shift) {
                gcr[j] = b | ((offset[0] << shift) >> 8);
                b = offset[0] << shift;
            } else {
                gcr[j] = b;
                b = offset[0];
            }
        }
        gcr_convert_GCR_to_4bytes(gcr, buf);
        buf += 4;
    }

    memcpy(data, buffer + 1, 256);

    return (buffer[0] == 0x07) ? 0 : -3;
}

int gcr_write_sector(disk_track_t  *raw, BYTE *data, BYTE sector)
{
    BYTE buffer[260], *offset, *buf;
    BYTE *end = raw->data + raw->size;
    BYTE gcr[5], chksum, b;
    int i, j, shift, p;

    p = gcr_find_sector_header(raw, sector);
    if (p < 0)
        return -1;

    p = gcr_find_sync(raw, p, 500 * 8);
    if (p < 0)
        return -2;

    shift = p & 7;
    offset = raw->data + (p >> 3);

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
            if (offset >= end)
                offset = raw->data;
        }
    }
    offset[0] = b | (offset[0] & (0xff >> shift));

    return 0;
}
