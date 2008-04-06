/*
 * tcbm.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "mem1551.h"
#include "glue1551.h"
#include "tcbm.h"
#include "tia1551.h"


void tcbm_drive_init(struct drive_context_s *drv)
{
    tia1551_init(drv);
    glue1551_init(drv);
}

void tcbm_drive_reset(struct drive_context_s *drv)
{
    tia1551_reset(drv);
    glue1551_reset(drv);
}

void tcbm_drive_mem_init(struct drive_context_s *drv, unsigned int type)
{
    mem1551_init(drv, type);
}

void tcbm_drive_setup_context(struct drive_context_s *drv)
{
}

int tcbm_drive_snapshot_read(struct drive_context_s *ctxptr,
                             struct snapshot_s *s)
{
    return 0;
}

int tcbm_drive_snapshot_write(struct drive_context_s *ctxptr,
                              struct snapshot_s *s)
{
    return 0;
}

/* FIXME */
void ieee_drive0_parallel_set_atn(int state)
{
}

void ieee_drive1_parallel_set_atn(int state)
{
}

