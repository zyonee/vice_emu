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

#include "vice.h"

#include "mfm.h"
#include "types.h"
#include "lib.h"

static WORD *crc1021 = NULL;

static void mfm_init_crc1021(void)
{
    int i, j;
    WORD w;

    crc1021 = lib_malloc(256 * sizeof(WORD));
    for (i = 0; i < 256; i++) {
        w = i << 8;
        for (j = 0; j < 8; j++) {
            if (w & 0x8000) {
                w <<= 1;
                w ^= 0x1021;
            } else {
                w <<= 1;
            }
        }
        crc1021[i] = w;
    }
}

inline WORD mfm_crc(WORD crc, BYTE b)
{
    if (!crc1021) {
        mfm_init_crc1021();
    }
    return crc1021[(crc >> 8) ^ b] ^ (crc << 8);
}

void mfm_convert_sector_to_MFM(BYTE *buffer, WORD *raw, mfm_header_t header,
                               unsigned int gap2)
{
    int i;
    WORD crc;

    for (i = 0; i < 12; i++) { /* Sync */
        *raw++ = 0x00;
    }
    for (i = 0; i < 3; i++) {
        *raw++ = 0x1a1;
    }
    *raw++ = 0xfe; /* ID mark */
    *raw++ = header.track;
    crc = mfm_crc(0xb230, header.track);
    *raw++ = header.head;
    crc = mfm_crc(crc, header.head);
    *raw++ = header.sector;
    crc = mfm_crc(crc, header.sector);
    *raw++ = header.sector_size;
    crc = mfm_crc(crc, header.sector_size);
    *raw++ = crc >> 8;
    *raw++ = crc & 0xff;

    raw += gap2;

    for (i = 0; i < 12; i++) { /* Sync */
        *raw++ = 0x00;
    }
    for (i = 0; i < 3; i++) {
        *raw++ = 0x1a1;
    }
    *raw++ = 0xfb; /* Data mark */
    crc = 0xe295;
    for (i = 0; i < (1 << header.sector_size); i++) {
        *raw++ = buffer[i];
        crc = mfm_crc(crc, buffer[i]);
    }
    *raw++ = crc >> 8;
    *raw++ = crc & 0xff;
}

int mfm_read_sector(WORD *raw, unsigned int size, BYTE *data,
                    mfm_header_t header)
{
    int i;
    int step = 0;
    int d = 0;
    int p = 0;
    WORD w;

    for (i = 0; i < size * 2; i++) {
        w = raw[p];
        p++;
        if (p >= size) {
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
            step = 9;
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

