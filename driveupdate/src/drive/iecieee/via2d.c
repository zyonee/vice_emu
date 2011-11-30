/*
 * via2d.c - VIA2 emulation in the 1541, 1541II, 1571 and 2031 disk drive.
 *
 * Written by
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

/*
    FIXME: test new code for regressions, remove old code
*/

/* #define DEBUG_VIA2 */

#ifdef DEBUG_VIA2
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

#include "vice.h"

#include <stdio.h>

#include "drive.h"
#include "drivetypes.h"
#include "interrupt.h"
#include "lib.h"
#include "log.h"
#include "types.h"
#include "via.h"
#include "viad.h"
#include "fdd.h"

typedef struct drivevia2_context_s {
    unsigned int number;
    struct drive_s *drive;
} drivevia2_context_t;

static void set_ca2(via_context_t *via_context, int state)
{
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    fdd_set_clk(drive->fdds[0], *via2p->drive->clk);
    fdd_set_soe(drive->fdds[0], state);
}

static void set_cb2(via_context_t *via_context, int state)
{
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    fdd_set_clk(drive->fdds[0], *drive->clk);
    fdd_set_write_gate(drive->fdds[0], state);
}

/* see interrupt.h; ugly, but more efficient... */
static void set_int(via_context_t *via_context, unsigned int int_num,
                    int value, CLOCK rclk)
{
    drive_context_t *drive_context;

    drive_context = (drive_context_t *)(via_context->context);

    interrupt_set_irq(drive_context->cpu->int_status, int_num, value, rclk);
}

static void restore_int(via_context_t *via_context, unsigned int int_num,
                    int value)
{
    drive_context_t *drive_context;

    drive_context = (drive_context_t *)(via_context->context);

    interrupt_restore_irq(drive_context->cpu->int_status, int_num, value);
}

void via2d_store(drive_context_t *ctxptr, WORD addr, BYTE data)
{
    viacore_store(ctxptr->via2, addr, data);
}

BYTE via2d_read(drive_context_t *ctxptr, WORD addr)
{
    return viacore_read(ctxptr->via2, addr);
}

void via2d1571_store(drive_context_t *ctxptr, WORD addr, BYTE data)
{
    drive_t *drive = ctxptr->drive;

    fdd_set_clk(drive->fdds[0], *drive->clk);
    fdd_byte_ready_clear(drive->fdds[0]);
    viacore_store(ctxptr->via2, addr, data);
}

BYTE via2d1571_read(drive_context_t *ctxptr, WORD addr)
{
    drive_t *drive = ctxptr->drive;

    fdd_set_clk(drive->fdds[0], *drive->clk);
    fdd_byte_ready_clear(drive->fdds[0]);
    return viacore_read(ctxptr->via2, addr);
}

BYTE via2d_peek(drive_context_t *ctxptr, WORD addr)
{
    return viacore_peek(ctxptr->via2, addr);
}

static void store_pra(via_context_t *via_context, BYTE byte, BYTE oldpa_value,
                      WORD addr)
{
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);

    fdd_set_clk(via2p->drive->fdds[0], *via2p->drive->clk);
    fdd_byte_write(via2p->drive->fdds[0], byte);
}

static void undump_pra(via_context_t *via_context, BYTE byte)
{
}

static void store_prb(via_context_t *via_context, BYTE byte, BYTE poldpb,
                      WORD addr)
{
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    if (drive->led_status)
        drive->led_active_ticks += *(via_context->clk_ptr) - drive->led_last_change_clk;
    drive->led_last_change_clk = *(via_context->clk_ptr);
    drive->led_status = (byte & 8) ? 1 : 0;

    /* Motor on/off */
    fdd_set_clk(drive->fdds[0], *drive->clk);
    fdd_set_motor(drive->fdds[0], byte & 0x04);

    if ((poldpb ^ byte) & 3) {   /* Stepper motor */
        fdd_step_quadrature(drive->fdds[0], byte);
    }
    /* Zone bits */
    fdd_set_rate(drive->fdds[0], byte >> 5);
}

static void undump_prb(via_context_t *via_context, BYTE byte)
{
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    drive->led_status = (byte & 8) ? 1 : 0;
}

static BYTE store_pcr(via_context_t *via_context, BYTE byte, WORD addr)
{
    return byte;
}

static void undump_pcr(via_context_t *via_context, BYTE byte)
{
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
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    drive->led_status = 1;
    drive_update_ui_status();
}

static BYTE read_pra(via_context_t *via_context, WORD addr)
{
    BYTE byte;
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    fdd_set_clk(drive->fdds[0], *drive->clk);
    byte = fdd_byte_read(drive->fdds[0]);

    byte = ((byte & ~(via_context->via[VIA_DDRA]))
           | (via_context->via[VIA_PRA] & via_context->via[VIA_DDRA]));

    return byte;
}

static BYTE read_prb(via_context_t *via_context)
{
    BYTE byte;
    drivevia2_context_t *via2p = (drivevia2_context_t *)(via_context->prv);
    drive_t *drive = via2p->drive;

    fdd_set_clk(drive->fdds[0], *drive->clk);
    byte = (((fdd_sync(drive->fdds[0]) ? 0x80 : 0)
           | (fdd_write_protect(drive->fdds[0]) ? 0x10 : 0))
           & ~(via_context->via[VIA_DDRB]))
           | (via_context->via[VIA_PRB] & via_context->via[VIA_DDRB]);

    return byte;
}

void via2d_init(drive_context_t *ctxptr)
{
    viacore_init(ctxptr->via2, ctxptr->cpu->alarm_context,
                 ctxptr->cpu->int_status, ctxptr->cpu->clk_guard);
}

void via2d_setup_context(drive_context_t *ctxptr)
{
    drivevia2_context_t *via2p;
    via_context_t *via;

    /* Clear struct as snapshot code may write uninitialized values.  */
    ctxptr->via2 = lib_calloc(1, sizeof(via_context_t));
    via = ctxptr->via2;

    via->prv = lib_malloc(sizeof(drivevia2_context_t));

    via2p = (drivevia2_context_t *)(via->prv);
    via2p->number = ctxptr->mynumber;
    via2p->drive = ctxptr->drive;

    via->context = (void *)ctxptr;

    via->rmw_flag = &(ctxptr->cpu->rmw_flag);
    via->clk_ptr = ctxptr->clk_ptr;

    via->myname = lib_msprintf("Drive%dVia2", via2p->number);
    via->my_module_name = lib_msprintf("VIA2D%d", via2p->number);

    viacore_setup_context(via);

    via->irq_line = IK_IRQ;
    via->int_num
        = interrupt_cpu_status_int_new(ctxptr->cpu->int_status, via->myname);

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
