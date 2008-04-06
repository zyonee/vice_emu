/*
 * patchrom.c - C64 ROM patcher.
 *
 * Written by
 *  Peter Weighill <stuce@csv.warwick.ac.uk>
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *
 * Patches by
 *  Marko M�kel� <msmakela@nic.funet.fi>
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "c64memrom.h"
#include "patchrom.h"
#include "types.h"


/*
 * By an option on the x64 command line you can patch between the
 * different roms.
 *
 *   x64 -kernalrev 0           KERNAL ROM R0
 *   x64 -kernalrev 3           KERNAL ROM R3
 *   x64 -kernalrev 67          SX-64 or DX-64
 *   x64 -kernalrev 100         PET 64 aka 4064 aka Educator 64
 */

/*
 * Table of differences between the two versions of the C64 Kernel ROM.
 * To use your national patches, just add the data following
 * the format used below. (Note: Each block must contain data for
 * each patch version.)
 */


#define PATCH_VERSIONS 3   /* counting from 0 */

static const unsigned short patch_bytes[] = {

  3, 0xE42D,
        0x20, 0x1E, 0xAB,
        0x20, 0x1E, 0xAB,
        0x20, 0x1E, 0xAB,
        0x4C, 0x41, 0xE4,

 54, 0xE477,
        0x20, 0x20, 0x2A, 0x2A, 0x2A, 0x2A, 0x20, 0x43, 0x4F, 0x4D,
        0x4D, 0x4F, 0x44, 0x4F, 0x52, 0x45, 0x20, 0x36, 0x34, 0x20,
        0x42, 0x41, 0x53, 0x49, 0x43, 0x20, 0x56, 0x32, 0x20, 0x2A,
        0x2A, 0x2A, 0x2A, 0x0D, 0x0D, 0x20, 0x36, 0x34, 0x4B, 0x20,
        0x52, 0x41, 0x4D, 0x20, 0x53, 0x59, 0x53, 0x54, 0x45, 0x4D,
        0x20, 0x20, 0x00, 0x5C,

        0x20, 0x20, 0x2A, 0x2A, 0x2A, 0x2A, 0x20, 0x43, 0x4F, 0x4D,
        0x4D, 0x4F, 0x44, 0x4F, 0x52, 0x45, 0x20, 0x36, 0x34, 0x20,
        0x42, 0x41, 0x53, 0x49, 0x43, 0x20, 0x56, 0x32, 0x20, 0x2A,
        0x2A, 0x2A, 0x2A, 0x0D, 0x0D, 0x20, 0x36, 0x34, 0x4B, 0x20,
        0x52, 0x41, 0x4D, 0x20, 0x53, 0x59, 0x53, 0x54, 0x45, 0x4D,
        0x20, 0x20, 0x00, 0x81,

        0x20, 0x20, 0x20, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x20, 0x20,
        0x53, 0x58, 0x2D, 0x36, 0x34, 0x20, 0x42, 0x41, 0x53, 0x49,
        0x43, 0x20, 0x56, 0x32, 0x2E, 0x30, 0x20, 0x20, 0x2A, 0x2A,
        0x2A, 0x2A, 0x2A, 0x0D, 0x0D, 0x20, 0x36, 0x34, 0x4B, 0x20,
        0x52, 0x41, 0x4D, 0x20, 0x53, 0x59, 0x53, 0x54, 0x45, 0x4D,
        0x20, 0x20, 0x00, 0xB3,

        0x2A, 0x2A, 0x2A, 0x2A, 0x20, 0x43, 0x4F, 0x4D, 0x4D, 0x4F,
        0x44, 0x4F, 0x52, 0x45, 0x20, 0x34, 0x30, 0x36, 0x34, 0x20,
        0x20, 0x42, 0x41, 0x53, 0x49, 0x43, 0x20, 0x56, 0x32, 0x2E,
        0x30, 0x20, 0x2A, 0x2A, 0x2A, 0x2A, 0x0D, 0x0D, 0x00, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x63,

 21, 0xE4C8,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAD, 0x21, 0xD0,

        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0x85, 0xA9, 0xA9, 0x01, 0x85, 0xAB, 0x60, 0xAD, 0x86, 0x02,

        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0x85, 0xA9, 0xA9, 0x01, 0x85, 0xAB, 0x60, 0xAD, 0x86, 0x02,

        0x2C, 0x86, 0x02, 0x30, 0x0A, 0xA9, 0x00, 0xA2, 0x0E, 0x9D,
        0x20, 0xD0, 0xCA, 0x10, 0xFA, 0x4C, 0x87, 0xEA, 0xAD, 0x21, 0xD0,

  1, 0xE535,
        0x0E,
        0x0E,
        0x06,
        0x01,

 30, 0xE57C,
        0xB5, 0xD9, 0x29, 0x03, 0x0D, 0x88, 0x02, 0x85, 0xD2, 0xBD,
        0xF0, 0xEC, 0x85, 0xD1, 0xA9, 0x27, 0xE8, 0xB4, 0xD9, 0x30,
        0x06, 0x18, 0x69, 0x28, 0xE8, 0x10, 0xF6, 0x85, 0xD5, 0x60,

        0x20, 0xF0, 0xE9, 0xA9, 0x27, 0xE8, 0xB4, 0xD9, 0x30, 0x06,
        0x18, 0x69, 0x28, 0xE8, 0x10, 0xF6, 0x85, 0xD5, 0x4C, 0x24,
        0xEA, 0xE4, 0xC9, 0xF0, 0x03, 0x4C, 0xED, 0xE6, 0x60, 0xEA,

        0x20, 0xF0, 0xE9, 0xA9, 0x27, 0xE8, 0xB4, 0xD9, 0x30, 0x06,
        0x18, 0x69, 0x28, 0xE8, 0x10, 0xF6, 0x85, 0xD5, 0x4C, 0x24,
        0xEA, 0xE4, 0xC9, 0xF0, 0x03, 0x4C, 0xED, 0xE6, 0x60, 0xEA,

        0x20, 0xF0, 0xE9, 0xA9, 0x27, 0xE8, 0xB4, 0xD9, 0x30, 0x06,
        0x18, 0x69, 0x28, 0xE8, 0x10, 0xF6, 0x85, 0xD5, 0x4C, 0x24,
        0xEA, 0xE4, 0xC9, 0xF0, 0x03, 0x4C, 0xED, 0xE6, 0x60, 0xEA,

  1, 0xE5EF,
        0x09,
        0x09,
        0x0F,
        0x09,

  2, 0xE5F4,
        0xE6, 0xEC,
        0xE6, 0xEC,
        0xD7, 0xF0,
        0xE6, 0xEC,

  2, 0xE622,
        0xED, 0xE6,
        0x91, 0xE5,
        0x91, 0xE5,
        0x91, 0xE5,

 12, 0xEA07,
        0xA9, 0x20, 0x91, 0xD1, 0x20, 0xDA, 0xE4, 0xEA, 0x88, 0x10,
        0xF5, 0x60,
        0x20, 0xDA, 0xE4, 0xA9, 0x20, 0x91, 0xD1, 0x88, 0x10, 0xF6,
        0x60, 0xEA,
        0x20, 0xDA, 0xE4, 0xA9, 0x20, 0x91, 0xD1, 0x88, 0x10, 0xF6,
        0x60, 0xEA,
        0xA9, 0x20, 0x91, 0xD1, 0x20, 0xDA, 0xE4, 0xEA, 0x88, 0x10,
        0xF5, 0x60,

 14, 0xECD9,
        0x0E, 0x06, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x0E, 0x06, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x03, 0x01, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

  3, 0xEF94,
        0x85, 0xA9, 0x60,
        0x4C, 0xD3, 0xE4,
        0x4C, 0xD4, 0xE4,
        0x85, 0xA9, 0x60,

 15, 0xF0D8,
        0x0D, 0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0x50, 0x4C, 0x41,
        0x59, 0x20, 0x4F, 0x4E, 0x20,
        0x0D, 0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0x50, 0x4C, 0x41,
        0x59, 0x20, 0x4F, 0x4E, 0x20,
        0x4C, 0x4F, 0x41, 0x44, 0x22, 0x3A, 0x2A, 0x22, 0x2C, 0x38,
        0x0D, 0x52, 0x55, 0x4E, 0x0D,
        0x0D, 0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0x50, 0x4C, 0x41,
        0x59, 0x20, 0x4F, 0x4E, 0x20,

  1, 0xF387,
        0x03,
        0x03,
        0x08,
        0x03,

  1, 0xF4B7,
        0x7B,
        0x7B,
        0xF7,
        0x7B,

  1, 0xF5F9,
        0x5F,
        0x5F,
        0xF7,
        0x5F,

  1, 0xF81F,
        0x2F,
        0x2F,
        0x2F,
        0x2B,

  1, 0xF82C,
        0x2F,
        0x2F,
        0x2F,
        0x2B,

  1, 0xFF80,    /* The version ID byte */
        0x00,
        0x03,
        0x43,
        0x64,

  2, 0xFFF8,
        0x42, 0x59,
        0x42, 0x59,
        0x42, 0x59,
        0x00, 0x00,

  0, 00
};


int patch_rom(const char *str)
{
    int rev, curr, num, lcount, isnum;
    short bytes, n, i = 0;
    WORD a;

    if (str == NULL || *str == '\0')
        return 0;

    for (isnum = 1, i = 0; str[i] != '\0'; i++) {
        if (!isdigit((int) str[i]))
            isnum = 0;
    }

    if (!isnum) {
        if (strcasecmp(str, "sx") == 0) {
            rev = 67;
        } else {
            log_error(LOG_DEFAULT, "Invalid ROM revision `%s'.", str);
            return -1;
        }
    } else {
        rev = atoi (str);
    }

    curr = rom64_read(0xff80);

    if (rev == curr) {
        log_warning(LOG_DEFAULT, "ROM not patched: Already revision #%d.",
                    curr);
        return (0);
    }

    if (rev < 0) {
          rev = 0;
    }

    /* create index */

    num = rev;

    switch (rev) {
      case 4064:
      case 100:
        rev = 3; /* index for rev100 data */
        break;
      case 67:
        rev = 2; /* index for rev67 data */
        break;
      case 3:
        rev = 1; /* index for rev03 data */
        break;
      case 0:
        break;
      default:
        log_error(LOG_DEFAULT, "Cannot patch ROM to revision #%d.", rev);
        return (-1);
    }

    log_message(LOG_DEFAULT, "Installing ROM patch for revision #%d:", num);

    lcount = 0;
    i = 0;
    while ((bytes = patch_bytes[i++]) > 0) {
        a = (WORD)patch_bytes[i++];

        log_message(LOG_DEFAULT, "%.4X (%d byte%s)",
                    a & 0xFFFF, bytes, ((bytes > 1) ? "s":""));

        i += (bytes * rev);     /* select patch */
        for(n = bytes; n--;)
            rom64_store(a++, (BYTE)patch_bytes[i++]);

        i += (bytes * (PATCH_VERSIONS - rev));  /* skip patch */
    }

    log_message(LOG_DEFAULT, "Patch installed.");

    return (0);
}

