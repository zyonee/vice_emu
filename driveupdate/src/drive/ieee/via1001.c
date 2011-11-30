/*
 * via1001.c - VIA1 emulation in the 1001 disk drive.
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
#include "fdccpu.h"
#include "drivesync.h"
#include "drivetypes.h"
#include "interrupt.h"
#include "lib.h"
#include "parallel.h"
#include "types.h"
#include "via.h"
#include "via1001.h"
#include "viad.h"
#include "fdd.h"

typedef struct drivevia1_context_s {
    struct drive_s *drive;
} drivevia1_context_t;


void via1001_store(drive_context_t *ctxptr, WORD addr, BYTE data)
{
    viacore_store(ctxptr->via1001, addr, data);
}

BYTE via1001_read(drive_context_t *ctxptr, WORD addr)
{
    return viacore_read(ctxptr->via1001, addr);
}

BYTE via1001_peek(drive_context_t *ctxptr, WORD addr)
{
    return viacore_peek(ctxptr->via1001, addr);
}

static void set_ca2(via_context_t *via_context, int state)
{
    drivevia1_context_t *viap = (drivevia1_context_t *)(via_context->prv);
    drive_t *drive = viap->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    fdd_set_mode(drive->fdds[0], state);
}

static void set_cb2(via_context_t *via_context, int state)
{
    drivevia1_context_t *viap = (drivevia1_context_t *)(via_context->prv);
    drive_t *drive = viap->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    fdd_set_write_gate(drive->fdds[0], state);
}

static void set_int(via_context_t *via_context, unsigned int int_num,
                    int value, CLOCK rclk)
{
    drive_context_t *drive_context;

    drive_context = (drive_context_t *)(via_context->context);

    interrupt_set_irq(drive_context->fdccpu->int_status, int_num, value, rclk);
}

static void restore_int(via_context_t *via_context, unsigned int int_num,
                    int value)
{
    drive_context_t *drive_context;

    drive_context = (drive_context_t *)(via_context->context);

    interrupt_restore_irq(drive_context->fdccpu->int_status, int_num, value);
}

static void undump_pra(via_context_t *via_context, BYTE byte)
{
}

static void store_pra(via_context_t *via_context, BYTE byte, BYTE oldpa_value,
                      WORD addr)
{
}

static void undump_prb(via_context_t *via_context, BYTE byte)
{
}

static void store_prb(via_context_t *via_context, BYTE byte, BYTE p_oldpb,
                      WORD addr)
{
    drivevia1_context_t *viap = (drivevia1_context_t *)(via_context->prv);
    drive_t *drive = viap->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    fdd_set_motor(drive->fdds[0], (byte & 0x20) ^ 0x20);

    if ((p_oldpb ^ byte) & 12) {   /* Stepper motor */
        fdd_step_quadrature(drive->fdds[0], (byte >> 2) ^ ((byte >> 3) & 1) ^ 2);
    }
}

static void undump_pcr(via_context_t *via_context, BYTE byte)
{
}

static BYTE store_pcr(via_context_t *via_context, BYTE byte, WORD addr)
{
    return byte;
}

static void undump_acr(via_context_t *via_context, BYTE byte)
{
}

static void store_acr(via_context_t *via_context, BYTE byte)
{
}

static void store_sr(via_context_t *via_context, BYTE byte)
{
}

static void store_t2l(via_context_t *via_context, BYTE byte)
{
}

static void reset(via_context_t *via_context)
{
}

static BYTE read_pra(via_context_t *via_context, WORD addr)
{
    BYTE byte;
    drivevia1_context_t *via1p = (drivevia1_context_t *)(via_context->prv);
    drive_t *drive = via1p->drive;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    byte = fdd_byte_read_fdc(drive->fdds[0]);

    return (byte & ~(via_context->via[VIA_DDRA]))
           | (via_context->via[VIA_PRA] & via_context->via[VIA_DDRA]);
}

static BYTE read_prb(via_context_t *via_context)
{
    drivevia1_context_t *viap = (drivevia1_context_t *)(via_context->prv);
    drive_t *drive = viap->drive;
    BYTE byte;

    fdd_set_clk(drive->fdds[0], *drive->fdcclk);
    byte = ((fdd_sync(drive->fdds[0]) ? 0x80 : 0)
           & ~(via_context->via[VIA_DDRB]))
           | (via_context->via[VIA_PRB] & via_context->via[VIA_DDRB]);

    return byte;
}

void via1001_init(drive_context_t *ctxptr)
{
    viacore_init(ctxptr->via1001, ctxptr->fdccpu->alarm_context,
                 ctxptr->fdccpu->int_status, ctxptr->fdccpu->clk_guard);
}

void via1001_setup_context(drive_context_t *ctxptr)
{
    drivevia1_context_t *viap;
    via_context_t *via;

    /* Clear struct as snapshot code may write uninitialized values.  */
    ctxptr->via1001 = lib_calloc(1, sizeof(via_context_t));
    via = ctxptr->via1001;

    via->prv = lib_malloc(sizeof(drivevia1_context_t));
    viap = (drivevia1_context_t *)(via->prv);

    via->context = (void *)ctxptr;

    via->rmw_flag = &(ctxptr->fdccpu->rmw_flag);
    via->clk_ptr = ctxptr->fdcclk_ptr;

    via->myname = lib_msprintf("1001Drive%dVia", ctxptr->mynumber);
    via->my_module_name = lib_msprintf("1001VIA%d", ctxptr->mynumber);

    viacore_setup_context(via);

    via->my_module_name_alt1 = lib_msprintf("VIA%d", ctxptr->mynumber);

    via->irq_line = IK_IRQ;

    viap->drive = ctxptr->drive;

    via->undump_pra = undump_pra;
    via->undump_prb = undump_prb;
    via->undump_pcr = undump_pcr;
    via->undump_acr = undump_acr;
    via->store_pra = store_pra;
    via->store_prb = store_prb;
    via->store_pcr = store_pcr;
    via->store_acr = store_acr;
    via->store_sr = store_sr;
    via->store_t2l = store_t2l;
    via->read_pra = read_pra;
    via->read_prb = read_prb;
    via->set_int = set_int;
    via->restore_int = restore_int;
    via->set_ca2 = set_ca2;
    via->set_cb2 = set_cb2;
    via->reset = reset;
}

