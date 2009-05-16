/*
 * viciidtv-cycle.c - VIC-II DTV emulation subcycle handling.
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

#include "vice.h"

#include "maindtvcpu.h"
#include "types.h"
#include "viciidtv.h"
#include "viciidtvtypes.h"

/* Memory access cycle 1:
    Character fetch/Counter A */
void viciidtv_cycle_1(void)
{
}

/* Memory access cycle 2:
    Color/sprite fetch/Blitter/DMA
  Returns 1 if access was done (BA low) */
int viciidtv_cycle_2(void)
{
    return 0;
}

/* Memory access cycle 3:
    Graphic fetch/Counter B */
void viciidtv_cycle_3(void)
{
}
