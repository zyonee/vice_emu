/*
 * riot3d.c - RRIOT3 emulation in the SFD1001, 8050 and 8250 disk drive.
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

#include <stdio.h>

#include "drive.h"
#include "drivetypes.h"
#include "interrupt.h"
#include "lib.h"
#include "riot.h"
#include "riotd.h"
#include "types.h"
#include "fdd.h"

typedef struct driveriot3_context_s {
    struct drive_s *drive;
    unsigned int int_num;
} driveriot3_context_t;

void riot3_store(drive_context_t *ctxptr, WORD addr, BYTE data)
{
    riotcore_store(ctxptr->riot3, addr | 0x10, data);
}

BYTE riot3_read(drive_context_t *ctxptr, WORD addr)
{
    return riotcore_read(ctxptr->riot3, addr | 0x10);
}

static void set_irq(riot_context_t *riot_context, int fl, CLOCK clk)
{
    drive_context_t *drive_context;
    driveriot3_context_t *riot3p;

    drive_context = (drive_context_t *)(riot_context->context);
    riot3p = (driveriot3_context_t *)(riot_context->prv);

    interrupt_set_irq(drive_context->fdccpu->int_status, (riot3p->int_num),
                      (fl) ? IK_IRQ : 0, clk);
}

static void restore_irq(riot_context_t *riot_context, int fl)
{
}

static void undump_pra(riot_context_t *riot_context, BYTE byte)
{
}

static void store_pra(riot_context_t *riot_context, BYTE byte)
{
    driveriot3_context_t *riot3p = (driveriot3_context_t *)(riot_context->prv);

    fdd_set_clk(riot3p->drive->fdds[0], *riot3p->drive->fdcclk);
    fdd_byte_write_fdc(riot3p->drive->fdds[0], byte);
}

static void undump_prb(riot_context_t *riot_context, BYTE byte)
{
}

static void store_prb(riot_context_t *riot_context, BYTE byte)
{
    driveriot3_context_t *riot3p = (driveriot3_context_t *)(riot_context->prv);
    drive_t *drive = riot3p->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    fdd_set_side(drive->fdds[0], (byte & 0x10) ^ 0x10);
    fdd_set_rate(drive->fdds[0], byte >> 1);
}

static void reset(riot_context_t *riot_context)
{
}

static BYTE read_pra(riot_context_t *riot_context)
{
    BYTE byte = 0xff;

    return (byte & ~(riot_context->riot_io)[1])
        | (riot_context->riot_io[0] & riot_context->riot_io[1]);
}

static BYTE read_prb(riot_context_t *riot_context)
{
    BYTE byte;
    driveriot3_context_t *riot3p = (driveriot3_context_t *)(riot_context->prv);
    drive_t *drive = riot3p->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    byte = fdd_write_protect(drive->fdds[0]) ? 0xf7 : 0xff;
    if (drive->type == DRIVE_TYPE_1001 || drive->type == DRIVE_TYPE_8250) {
        byte &= ~0x40; /* double sided jumper */
    }

    return (byte & ~(riot_context->riot_io)[3])
        | (riot_context->riot_io[2] & riot_context->riot_io[3]);
}

void riot3_init(drive_context_t *ctxptr)
{
    riotcore_init(ctxptr->riot3, ctxptr->cpu->alarm_context,
                  ctxptr->cpu->clk_guard, ctxptr->mynumber);
}

void riot3_setup_context(drive_context_t *ctxptr)
{
    riot_context_t *riot;
    driveriot3_context_t *riot3p;

    ctxptr->riot3 = lib_malloc(sizeof(riot_context_t));
    riot = ctxptr->riot3;

    riot->prv = lib_malloc(sizeof(driveriot3_context_t));
    riot3p = (driveriot3_context_t *)(riot->prv);
    riot->context = (void *)ctxptr;

    riot->rmw_flag = &(ctxptr->fdccpu->rmw_flag);
    riot->clk_ptr = ctxptr->fdcclk_ptr;

    riotcore_setup_context(riot);

    riot->myname = lib_msprintf("RIOT3D%d", ctxptr->mynumber);

    riot3p->drive = ctxptr->drive;
    riot3p->int_num = interrupt_cpu_status_int_new(ctxptr->fdccpu->int_status,
                                                   ctxptr->riot3->myname);
    riot->undump_pra = undump_pra;
    riot->undump_prb = undump_prb;
    riot->store_pra = store_pra;
    riot->store_prb = store_prb;
    riot->read_pra = read_pra;
    riot->read_prb = read_prb;
    riot->reset = reset;
    riot->set_irq = set_irq;
    riot->restore_irq = restore_irq;
}

