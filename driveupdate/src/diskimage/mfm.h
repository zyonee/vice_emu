/*
 * mfm.h - MFM handling.
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

#ifndef VICE_MFM_H
#define VICE_MFM_H

#include "types.h"

struct disk_track_s;
typedef struct mfm_header_s {
    BYTE track, head, sector, sector_size;
} mfm_header_t;

extern void mfm_convert_sector_to_MFM(BYTE *buffer, BYTE *data, BYTE *sync,
                                      mfm_header_t *header, int gap2);
extern int mfm_read_sector(struct disk_track_s *raw, BYTE *data,
                           mfm_header_t header);
#endif

