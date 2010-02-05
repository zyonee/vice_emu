/*
 * vicii-chip-model.h - Chip model definitions for the VIC-II emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
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

#ifndef VICE_VICII_CHIP_MODEL_H
#define VICE_VICII_CHIP_MODEL_H

extern void vicii_chip_model_init(void);

static inline BYTE get_sprite_ba_mask(unsigned int flags)
{
    return flags & 0xff;
}

static inline int is_fetch_ba(unsigned int flags)
{
    return flags & 0x100;
}

static inline int is_sprite_ptr_dma0(unsigned int flags)
{
    return (flags & 0x3000) == 0x1000;
}

static inline int is_sprite_dma1_dma2(unsigned int flags)
{
    return (flags & 0x3000) == 0x2000;
}

static inline int get_sprite_num(unsigned int flags)
{
    return (flags & 0x0e00) >> 9;
}

static inline int is_refresh(unsigned int flags)
{
    return (flags & 0x3800) == 0x3000;
}

static inline int is_fetch_g(unsigned int flags)
{
    return (flags & 0x3800) == 0x3800;
}

static inline int may_fetch_c(unsigned int flags)
{
    return (flags & 0x4000);
}

#endif

