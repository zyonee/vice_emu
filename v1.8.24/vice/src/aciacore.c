/*
 * aciacore.c - Template file for ACIA 6551 emulation.
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#include "acia.h"
#include "alarm.h"
#include "clkguard.h"
#include "cmdline.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "rs232.h"
#include "snapshot.h"
#include "types.h"

#undef  DEBUG

static alarm_t acia_alarm;

static int acia_ticks = 21111;  /* number of clock ticks per char */
static int fd = -1;
static int intx = 0;    /* indicates that a transmit is currently ongoing */
static int irq = 0;
static BYTE cmd;
static BYTE ctrl;
static BYTE rxdata;     /* data that has been received last */
static BYTE txdata;     /* data prepared to send */
static BYTE status;
static int alarm_active = 0;    /* if alarm is set or not */

static log_t acia_log = LOG_ERR;

static void int_acia(CLOCK offset);

static BYTE acia_last_read = 0;  /* the byte read the last time (for RMW) */

/******************************************************************/

/* rs232.h replacement functions if no rs232 device available */

#ifndef HAVE_RS232

int rs232_open(int device)
{
    return -1;
}

void rs232_close(int fd)
{
}

int rs232_putc(int fd, BYTE b)
{
    return -1;
}

int rs232_getc(int fd, BYTE *b)
{
    return -1;
}

#endif

/******************************************************************/

static CLOCK acia_alarm_clk = 0;

static int acia_device;
static int acia_irq;
static int acia_irq_res;

static int acia_set_device(resource_value_t v, void *param)
{

    if (fd >= 0) {
        log_error(acia_log,
                  "Device open, change effective only after close!");
    }
    acia_device = (int) v;
    return 0;
}

static int acia_set_irq(resource_value_t v, void *param)
{
    int new_irq_res = (int)v;
    int new_irq;
    static const int irq_tab[] = { IK_NONE, IK_IRQ, IK_NMI };

    if (new_irq_res < 0 || new_irq_res > 2) return -1;

    new_irq = irq_tab[new_irq_res];

    if (acia_irq != new_irq) {
        mycpu_set_int(I_MYACIA, IK_NONE);
        if (irq) {
            mycpu_set_int(I_MYACIA, new_irq);
        }
    }
    acia_irq = new_irq;
    acia_irq_res = new_irq_res;

    return 0;
}

static resource_t resources[] = {
    { MYACIA "Dev", RES_INTEGER, (resource_value_t) MyDevice,
      (resource_value_t *) & acia_device, acia_set_device, NULL },
    { MYACIA "Irq", RES_INTEGER, (resource_value_t) MyIrq,
      (resource_value_t *) & acia_irq_res, acia_set_irq, NULL },
    { NULL }
};

int myacia_init_resources(void) {
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-myaciadev", SET_RESOURCE, 1, NULL, NULL, MYACIA "Dev", NULL,
        "<0-3>", "Specify RS232 device this ACIA should work on" },
    { NULL }
};

int myacia_init_cmdline_options(void) {
    return cmdline_register_options(cmdline_options);
}

/******************************************************************/

/* note: the first value is bogus. It should be 16*external clock. */
static double acia_baud_table[16] = {
    10, 50, 75, 109.92, 134.58, 150, 300, 600, 1200, 1800,
    2400, 3600, 4800, 7200, 9600, 19200
};

/******************************************************************/

static void clk_overflow_callback(CLOCK sub, void *var)
{
    if (alarm_active)
        acia_alarm_clk -= sub;
}

void myacia_init(void) {
    alarm_init(&acia_alarm, mycpu_alarm_context, MYACIA, int_acia);

    clk_guard_add_callback(&mycpu_clk_guard, clk_overflow_callback, NULL);

    if (acia_log == LOG_ERR)
        acia_log = log_open(MYACIA);
}

void myacia_reset(void)
{
#ifdef DEBUG
    log_message(acia_log, "reset_myacia");
#endif
    cmd = 0;
    ctrl = 0;

    acia_ticks = (int)(machine_get_cycles_per_second()
                 / acia_baud_table[ctrl & 0xf]);

    status = 0x10;
    intx = 0;

    if (fd >= 0)
        rs232_close(fd);
    fd = -1;

    alarm_unset(&acia_alarm);
    alarm_active = 0;

    mycpu_set_int(I_MYACIA, 0);
    irq = 0;
}

/* -------------------------------------------------------------------------- */
/* The dump format has a module header and the data generated by the
 * chip...
 *
 * The version of this dump description is 0/0
 */

#define ACIA_DUMP_VER_MAJOR      1
#define ACIA_DUMP_VER_MINOR      0

/*
 * The dump data:
 *
 * UBYTE        TDR     Transmit Data Register
 * UBYTE        RDR     Receiver Data Register
 * UBYTE        SR      Status Register (includes state of IRQ line)
 * UBYTE        CMD     Command Register
 * UBYTE        CTRL    Control Register
 *
 * UBYTE        INTX    0 = no data to tx; 2 = TDR valid; 1 = in transmit
 *
 * DWORD        TICKS   ticks till the next TDR empty interrupt
 */

static const char module_name[] = MYACIA;

/* FIXME!!!  Error check.  */
/* FIXME!!!  If no connection, emulate carrier lost or so */
int myacia_snapshot_write_module(snapshot_t *p)
{
    snapshot_module_t *m;

    m = snapshot_module_create(p, module_name, (BYTE)ACIA_DUMP_VER_MAJOR,
                               (BYTE)ACIA_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    snapshot_module_write_byte(m, txdata);
    snapshot_module_write_byte(m, rxdata);
    snapshot_module_write_byte(m, (BYTE)(status | (irq?0x80:0)));
    snapshot_module_write_byte(m, cmd);
    snapshot_module_write_byte(m, ctrl);
    snapshot_module_write_byte(m, (BYTE)(intx));

    if(alarm_active) {
        snapshot_module_write_dword(m, (acia_alarm_clk - myclk));
    } else {
        snapshot_module_write_dword(m, 0);
    }

    snapshot_module_close(m);

    return 0;
}

int myacia_snapshot_read_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    BYTE byte;
    DWORD dword;
    snapshot_module_t *m;

    alarm_unset(&acia_alarm);   /* just in case we don't find module */
    alarm_active = 0;

    mycpu_set_int_noclk(I_MYACIA, 0);

    m = snapshot_module_open(p, module_name, &vmajor, &vminor);
    if (m == NULL)
        return -1;

    if (vmajor != ACIA_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_read_byte(m, &txdata);
    snapshot_module_read_byte(m, &rxdata);

    irq = 0;
    snapshot_module_read_byte(m, &status);
    if(status & 0x80) {
        status &= 0x7f;
        irq = 1;
        mycpu_set_int_noclk(I_MYACIA, acia_irq);
    } else {
        mycpu_set_int_noclk(I_MYACIA, 0);
    }

    snapshot_module_read_byte(m, &cmd);
    if((cmd & 1) && (fd < 0)) {
        fd = rs232_open(acia_device);
    } else
        if((fd >= 0) && !(cmd&1)) {
        rs232_close(fd);
        fd = -1;
    }

    snapshot_module_read_byte(m, &ctrl);
    acia_ticks = (int)(machine_get_cycles_per_second()
                                / acia_baud_table[ctrl & 0xf]);

    snapshot_module_read_byte(m, &byte);
    intx = byte;

    snapshot_module_read_dword(m, &dword);
    if (dword) {
        acia_alarm_clk = myclk + dword;
        alarm_set(&acia_alarm, acia_alarm_clk);
        alarm_active = 1;
    } else {
        alarm_unset(&acia_alarm);
        alarm_active = 0;
    }

    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}


void REGPARM2 myacia_store(ADDRESS a, BYTE b)
{
#ifdef DEBUG
    log_message(acia_log, "store_myacia(%04x,%02x)",a,b);
#endif
    if (mycpu_rmw_flag) {
        myclk --;
        mycpu_rmw_flag = 0;
        myacia_store(a, acia_last_read);
        myclk ++;
    }

    switch(a & 3) {
      case ACIA_DR:
        txdata = b;
        if (cmd & 1) {
            if (!intx) {
                acia_alarm_clk = myclk + 1;
                alarm_set(&acia_alarm, acia_alarm_clk);
                alarm_active = 1;
                intx = 2;
            } else
                if (intx == 1) {
                    intx++;
                }
                status &= 0xef;               /* clr TDRE */
        }
        break;
      case ACIA_SR:
        if (fd >= 0)
            rs232_close(fd);
        fd = -1;
        status &= ~4;
        cmd &= 0xe0;
        intx = 0;
        mycpu_set_int(I_MYACIA, 0);
        irq = 0;
        alarm_unset(&acia_alarm);
        alarm_active = 0;
        break;
      case ACIA_CTRL:
        ctrl = b;
        acia_ticks = (int)(machine_get_cycles_per_second()
                     / acia_baud_table[ctrl & 0xf]);
        break;
      case ACIA_CMD:
        cmd = b;
        if ((cmd & 1) && (fd < 0)) {
            fd = rs232_open(acia_device);
            acia_alarm_clk = myclk + acia_ticks;
            alarm_set(&acia_alarm, acia_alarm_clk);
            alarm_active = 1;
        } else
            if ((fd >= 0) && !(cmd&1)) {
                rs232_close(fd);
                alarm_unset(&acia_alarm);
                alarm_active = 0;
                fd = -1;
            }
        break;
        }
}

BYTE REGPARM1 myacia_read(ADDRESS a)
{
#if 0 /* def DEBUG */
    BYTE myacia_read_(ADDRESS);
    BYTE b = myacia_read_(a);
    static ADDRESS lasta = 0;
    static BYTE lastb = 0;

    if ((a!=lasta) || (b!=lastb)) {
        log_message(acia_log, "read_myacia(%04x) -> %02x",a,b);
    }
    lasta = a; lastb = b;
    return b;
}
BYTE myacia_read_(ADDRESS a)
{
#endif
    switch(a & 3) {
      case ACIA_DR:
        status &= ~8;
        acia_last_read = rxdata;
        return rxdata;
      case ACIA_SR:
        {
            BYTE c = status | (irq?0x80:0);
            mycpu_set_int(I_MYACIA, 0);
            irq = 0;
            acia_last_read = c;
            return c;
        }
      case ACIA_CTRL:
        acia_last_read = ctrl;
        return ctrl;
      case ACIA_CMD:
        acia_last_read = cmd;
        return cmd;
    }
    /* should never happen */
    return 0;
}

BYTE myacia_peek(ADDRESS a)
{
    switch(a & 3) {
      case ACIA_DR:
        return rxdata;
      case ACIA_SR:
        {
          BYTE c = status | (irq?0x80:0);
          return c;
        }
      case ACIA_CTRL:
        return ctrl;
      case ACIA_CMD:
        return cmd;
    }
    return 0;
}

static void int_acia(CLOCK offset)
{
    int rxirq;

#if 0 /* def DEBUG */
    log_message(acia_log, "int_acia(offset=%ld, myclk=%d", offset, myclk);
#endif
    if ((intx == 2) && (fd >= 0))
        rs232_putc(fd,txdata);
    if (intx)
        intx--;

    rxirq = 0;
    if ((fd >= 0) && (!(status&8)) && rs232_getc(fd, &rxdata)) {
        status |= 8;
        rxirq = 1;
    }

    if ((rxirq && (!(cmd & 0x02))) || ((cmd & 0x0c) == 0x04) ) {
        mycpu_set_int(I_MYACIA, acia_irq);
        irq = 1;
    }

    if(!(status&0x10)) {
        status |= 0x10;
    }

    acia_alarm_clk = myclk + acia_ticks;
    alarm_set(&acia_alarm, acia_alarm_clk);
    alarm_active = 1;
}

