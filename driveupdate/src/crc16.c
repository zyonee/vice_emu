/*
 * crc16.c - 16 bit CRC, poly: 1021
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

#include "lib.h"
#include "crc16.h"
#include "types.h"

static WORD *crc16_table = NULL;

static void crc16_init(void)
{
    int i, j;
    WORD w;

    crc16_table = lib_malloc(256 * sizeof(WORD));
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
        crc16_table[i] = w;
    }
}

inline WORD crc16(WORD crc, BYTE b)
{
    if (!crc16_table) {
        crc16_init();
    }
    return crc16_table[(crc >> 8) ^ b] ^ (crc << 8);
}
