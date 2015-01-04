/*
 * via2d1990.c - VIA emulation in the 1990 disk drive.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
 *
 * Based on old code by
 *  Andreas Boose <viceteam@t-online.de>
 *  Andre' Fachat <fachat@physik.tu-chemnitz.de>
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "debug.h"
#include "drive.h"
#include "drivecpu.h"
#include "drivesync.h"
#include "drivetypes.h"
#include "iecbus.h"
#include "iecdrive.h"
#include "interrupt.h"
#include "lib.h"
#include "types.h"
#include "via.h"
#include "via2d1990.h"
#include "viad.h"
#include "scsi.h"
#include "machine.h"


typedef struct drivevia_context_s {
    unsigned int number;
    struct drive_s *drive;
    WORD scsi_bus;
} drivevia_context_t;


void via2d1990_store(drive_context_t *ctxptr, WORD addr, BYTE data)
{
    viacore_store(ctxptr->via2d1990, addr, data);
}

BYTE via2d1990_read(drive_context_t *ctxptr, WORD addr)
{
    return viacore_read(ctxptr->via2d1990, addr);
}

BYTE via2d1990_peek(drive_context_t *ctxptr, WORD addr)
{
    return viacore_peek(ctxptr->via2d1990, addr);
}

static void set_ca2(via_context_t *via_context, int state)
{
}

static void set_cb2(via_context_t *via_context, int state)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);

    if (state) {
        viap->scsi_bus |= SCSI_ATN;
    } else {
        viap->scsi_bus &= ~SCSI_ATN;
    }
    scsi_store(drive_context->scsi_drive, viap->scsi_bus);
}

static void set_int(via_context_t *via_context, unsigned int int_num,
                    int value, CLOCK rclk)
{
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);

    interrupt_set_irq(drive_context->cpu->int_status, int_num, value, rclk);
}

static void restore_int(via_context_t *via_context, unsigned int int_num,
                        int value)
{
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);

    interrupt_restore_irq(drive_context->cpu->int_status, int_num, value);
}

static void undump_pra(via_context_t *via_context, BYTE byte)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);

    viap->scsi_bus = (viap->scsi_bus | 0xff) ^ byte;
}

static void store_pra(via_context_t *via_context, BYTE byte, BYTE oldpa,
                      WORD addr)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);

    viap->scsi_bus = (viap->scsi_bus | 0xff) ^ byte;

    if (addr == 1) {
        scsi_store(drive_context->scsi_drive, viap->scsi_bus | SCSI_ACK);
    } else {
        scsi_store(drive_context->scsi_drive, viap->scsi_bus);
    }
}

static void undump_prb(via_context_t *via_context, BYTE byte)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);

    if (byte & 0x10) {
        viap->scsi_bus |= SCSI_SEL;
    } else {
        viap->scsi_bus &= ~SCSI_SEL;
    }
}

static void store_prb(via_context_t *via_context, BYTE byte, BYTE p_oldpb,
                      WORD addr)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);

    if (byte & 0x10) {
        viap->scsi_bus |= SCSI_SEL;
    } else {
        viap->scsi_bus &= ~SCSI_SEL;
    }
    scsi_store(drive_context->scsi_drive, viap->scsi_bus);
}

static void undump_pcr(via_context_t *via_context, BYTE byte)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);

    if (via_context->cb2_state) {
        viap->scsi_bus |= SCSI_ATN;
    } else {
        viap->scsi_bus &= ~SCSI_ATN;
    }
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
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);
    BYTE byte = ~scsi_read(drive_context->scsi_drive, viap->scsi_bus | SCSI_ACK);

    byte = ((via_context->via[VIA_PRA] & via_context->via[VIA_DDRA])
               | (byte & ~(via_context->via[VIA_DDRA])));

    return byte;
}

static BYTE read_prb(via_context_t *via_context)
{
    drivevia_context_t *viap = (drivevia_context_t *)(via_context->prv);
    drive_context_t *drive_context = (drive_context_t *)(via_context->context);
    WORD bus = scsi_read(drive_context->scsi_drive, viap->scsi_bus);
    BYTE byte;

    if (bus & SCSI_C_D) {
        byte = 0x01;
    } else {
        byte = 0x00;
    }
    if (bus & SCSI_MSG) {
        byte |= 0x02;
    }
    if (bus & SCSI_I_O) {
        byte |= 0x04;
    }
    if (bus & SCSI_SEL) {
        byte |= 0x08;
    }
    if (bus & SCSI_BSY) {
        byte |= 0x20;
    }
    if (bus & SCSI_REQ) {
        byte |= 0x80;
    }

    byte = ((via_context->via[VIA_PRB] & via_context->via[VIA_DDRB])
               | (byte & ~(via_context->via[VIA_DDRB])));

    return byte;
}

void via2d1990_init(drive_context_t *ctxptr)
{
    viacore_init(ctxptr->via2d1990, ctxptr->cpu->alarm_context,
                 ctxptr->cpu->int_status, ctxptr->cpu->clk_guard);

}

void via2d1990_setup_context(drive_context_t *ctxptr)
{
    drivevia_context_t *viap;
    via_context_t *via;

    /* Clear struct as snapshot code may write uninitialized values.  */
    ctxptr->via2d1990 = lib_calloc(1, sizeof(via_context_t));
    via = ctxptr->via2d1990;

    via->prv = lib_malloc(sizeof(drivevia_context_t));
    viap = (drivevia_context_t *)(via->prv);
    viap->number = ctxptr->mynumber;

    via->context = (void *)ctxptr;

    via->rmw_flag = &(ctxptr->cpu->rmw_flag);
    via->clk_ptr = ctxptr->clk_ptr;

    via->myname = lib_msprintf("1990Drive%dVia2", ctxptr->mynumber);
    via->my_module_name = lib_msprintf("1990VIA2D%d", ctxptr->mynumber);

    viacore_setup_context(via);

    via->my_module_name_alt1 = lib_msprintf("VIA2D%d", ctxptr->mynumber);
    via->my_module_name_alt2 = lib_msprintf("VIA2D1990");

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
