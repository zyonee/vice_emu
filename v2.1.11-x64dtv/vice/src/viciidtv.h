/*
 * viciidtv.h - A cycle-exact event-driven VIC-II DTV emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#ifndef VICE_VICIIDTV_H
#define VICE_VICIIDTV_H

/* Memory access cycle 1:
    Character fetch/Counter A */
extern void viciidtv_cycle_1(void);

/* Memory access cycle 2:
    Color/sprite fetch/Blitter/DMA
  Returns 1 if access was done (BA low) */
extern int viciidtv_cycle_2(void);

/* Memory access cycle 3:
    Graphic fetch/Counter B */
extern void viciidtv_cycle_3(void);

#endif
