/*
 * mfm.c - MFM handling.
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

#include <string.h>

#include "vice.h"

#include "mfm.h"
#include "crc16.h"
#include "types.h"
#include "lib.h"
#include "diskimage.h"

void mfm_convert_sector_to_MFM(BYTE *buffer, BYTE *data, BYTE *sync,
                               mfm_header_t *header, int gap2)
{
    int i, size;
    WORD crc;

    memset(sync, 0x00, 12 + 3 + 1 + 4 + 2);
    memset(data, 0x00, 12);    /* Sync */
    data += 12;
    sync += 12;
    memset(data, 0xa1, 3); 
    memset(sync, 0x01, 3);
    data += 3;
    sync += 3 + 1 + 4 + 2;
    *data++ = 0xfe; /* ID mark */
    *data++ = header->track;
    crc = crc16(0xb230, header->track);
    *data++ = header->head;
    crc = crc16(crc, header->head);
    *data++ = header->sector;
    crc = crc16(crc, header->sector);
    *data++ = header->sector_size;
    crc = crc16(crc, header->sector_size);
    *data++ = crc >> 8;
    *data++ = crc & 0xff;

    data += gap2;
    sync += gap2;

    size = (128 << header->sector_size);
    memset(data, 0x00, 12);
    memset(sync, 0x00, 12 + 3 + 1 + size + 2);    /* Sync */
    data += 12;
    sync += 12;
    memset(data, 0xa1, 3); 
    memset(sync, 0x01, 3);
    data += 3;
    memset(sync, 0x00, 1 + size + 2);
    *data++ = 0xfb; /* Data mark */
    memcpy(data, buffer, size);
    data += size;
    crc = 0xe295;
    for (i = 0; i < size; i++) {
        crc = crc16(crc, buffer[i]);
    }
    *data++ = crc >> 8;
    *data++ = crc & 0xff;
}

int mfm_read_sector(disk_track_t *raw, BYTE *data, mfm_header_t header)
{
    int i;
    int step = 0;
    int d = 0;
    int p = 0;
    WORD w;

    for (i = 0; i < raw->size * 2; i++) {
        w = raw->data[p] + (raw->data[p + raw->size] << 8);
        p++;
        if (p >= raw->size) {
            p = 0;
        }
        switch (step) {
        case 0:
            if (w == 0x00) {
                step++;
            }
            continue;
        case 1:
            if (w == 0x00) {
                continue;
            }
            if (w == 0x1a1) {
                step++;
                continue;
            }
            break;
        case 2:
            if (w == 0x1a1) {
                continue;
            }
            if (w == 0xfe) {
                step++;
                continue;
            }
            break;
        case 3:
            if (w == header.track / 2) {
                step++;
                continue;
            }
            break;
        case 4:
            if (w == header.head) {
                step++;
                continue;
            }
            break;
        case 5:
            if (w == header.sector) {
                step++;
                continue;
            }
            break;
        case 6:
            if (w == header.sector_size) {
                step++;
                continue;
            }
            break;
        case 7: /* crc */
        case 8:
            step++;
            continue;
        case 9:
            if (w == 0x00) {
                step++;
            }
            continue;
        case 10:
            if (w == 0x00) {
                continue;
            }
            if (w == 0x1a1) {
                step++;
                continue;
            }
            step--;
            continue;
        case 11:
            if (w == 0x1a1) {
                continue;
            }
            if (w == 0xfb) {
                step++;
                continue;
            }
            break;
        case 12:
            data[d++] = w;
            if (d >= (128 << header.sector_size)) {
                step++;
            }
            continue;
        case 13:
            step++;
            continue;
        case 14:
            return 0;
        }
        step = 0;
    }
    return -1;
}

