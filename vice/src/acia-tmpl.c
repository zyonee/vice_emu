/*
 *
 * This file is part of Commodore 64 emulator.
 * See README for copyright notice
 *
 * ACIA 6551 rs232 emulation
 *
 * Written by
 *    Andre Fachat (a.fachat@physik.tu-chemnitz.de)
 *
 */

#include <stdio.h>

#include "resources.h"
#include "cmdline.h"
#include "vice.h"
#include "types.h"
#include "vmachine.h"
#include "machine.h"
#include "interrupt.h"
#include "snapshot.h"
#include "rs232.h"
#include "acia.h"

INCLUDES

#undef	DEBUG

static int acia_ticks = 21111;	/* number of clock ticks per char */
static int fd = -1;
static int intx = 0;	/* indicates that a transmit is currently ongoing */
static int irq = 0;
static BYTE cmd;
static BYTE ctrl;
static BYTE rxdata;	/* data that has been received last */
static BYTE txdata;	/* data prepared to send */
static BYTE status;
static int alarm_active = 0;	/* if alarm is set or not */

/******************************************************************/

/* rs232.h replacement functions if no rs232 device available */

#ifndef HAVE_RS232

static int rs232_open(int device)
{
    return -1;
}

static void rs232_close(int fd)
{
}

static int rs232_putc(int fd, BYTE b)
{
    return -1;
}

static int rs232_getc(int fd, BYTE *b)
{
    return -1;
}

#endif

/******************************************************************/

static int myacia_device;
static int myacia_irq;

static int myacia_set_device(resource_value_t v) {

    if(fd>=0) {
	fprintf(stderr, "MYACIA: device open, change effective only after "
		"close!\n");
    }
    myacia_device = (int) v;
    return 0;
}

static int myacia_set_irq(resource_value_t v) {
    int new_irq = (int)v;

    if(myacia_irq != new_irq) {
	mycpu_set_int(I_MYACIA, 0);
	if(irq) {
	    mycpu_set_int(I_MYACIA, new_irq);
	}
    }
    myacia_irq = new_irq;

    return 0;
}

static resource_t resources[] = {
    { "MyAciaDev", RES_INTEGER, (resource_value_t) MyDevice,
      (resource_value_t *) & myacia_device, myacia_set_device },
    { "MyAciaIrq", RES_INTEGER, (resource_value_t) MyIrq,
      (resource_value_t *) & myacia_irq, myacia_set_irq },
    { NULL }
};

int myacia_init_resources(void) {
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-myaciadev", SET_RESOURCE, 1, NULL, NULL, "MyAciaDev", NULL,
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

void reset_myacia(void) {

#ifdef DEBUG
	printf("reset_myacia\n");
#endif

	cmd = 0;
	ctrl = 0;

        acia_ticks = machine_get_cycles_per_second() 
				/ acia_baud_table[ctrl & 0xf];

	status = 0x10;
	intx = 0;

	if(fd>=0) rs232_close(fd);
	fd = -1;

	mycpu_unset_alarm(A_MYACIA);
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

#define ACIA_DUMP_VER_MAJOR      0
#define ACIA_DUMP_VER_MINOR      0

/*
 * The dump data:
 *
 * UBYTE	TDR	Transmit Data Register
 * UBYTE	RDR	Receiver Data Register
 * UBYTE	SR	Status Register (includes state of IRQ line)
 * UBYTE	CMD	Command Register
 * UBYTE	CTRL	Control Register
 *
 * UBYTE	INTX	0 = no data to tx; 2 = TDR valid; 1 = in transmit
 *
 * DWORD	TICKS	ticks till the next TDR empty interrupt
 */

static const char module_name[] = "MYACIA";

/* FIXME!!!  Error check.  */
/* FIXME!!!  If no connection, emulate carrier lost or so */
int myacia_write_snapshot_module(snapshot_t * p)
{
    snapshot_module_t *m;

    m = snapshot_module_create(p, module_name,
                               ACIA_DUMP_VER_MAJOR, ACIA_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    snapshot_module_write_byte(m, txdata);
    snapshot_module_write_byte(m, rxdata);
    snapshot_module_write_byte(m, status | (irq?0x80:0));
    snapshot_module_write_byte(m, cmd);
    snapshot_module_write_byte(m, ctrl);
    snapshot_module_write_byte(m, intx);
    if(alarm_active) {
        snapshot_module_write_dword(m, (mycpu_int_status.alarm_clk[A_MYACIA]
                                    - myclk));
    } else {
        snapshot_module_write_dword(m, 0);
    }

    snapshot_module_close(m);

    return 0;
}

int myacia_read_snapshot_module(snapshot_t * p)
{
    BYTE vmajor, vminor;
    BYTE byte;
    DWORD dword;
    snapshot_module_t *m;

    mycpu_unset_alarm(A_MYACIA);   /* just in case we don't find module */
    alarm_active = 0;

    set_int_noclk(&mycpu_int_status, I_MYACIA, 0);

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
	set_int_noclk(&mycpu_int_status, I_MYACIA, myacia_irq);
    } else {
	set_int_noclk(&mycpu_int_status, I_MYACIA, 0);
    }

    snapshot_module_read_byte(m, &cmd);
    if((cmd & 1) && (fd<0)) {
        fd = rs232_open(myacia_device);
    } else
        if(fd>=0 && !(cmd&1)) {
        rs232_close(fd);
        fd = -1;
    }

    snapshot_module_read_byte(m, &ctrl);
    acia_ticks = machine_get_cycles_per_second() 
                                / acia_baud_table[ctrl & 0xf];

    snapshot_module_read_byte(m, &byte);
    intx = byte;

    snapshot_module_read_dword(m, &dword);
    if (dword) {
        mycpu_set_alarm(A_MYACIA, dword);
        alarm_active = 1;
    } else {
        mycpu_unset_alarm(A_MYACIA);
        alarm_active = 0;
    }

    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}


void REGPARM2 store_myacia(ADDRESS a, BYTE b) {

#ifdef DEBUG
	printf("store_myacia(%04x,%02x\n",a,b);
#endif

	switch(a & 3) {
	case ACIA_DR:
		txdata = b;
		if(cmd&1) {
		  if(!intx) {
		    mycpu_set_alarm(A_MYACIA, 1);
                    alarm_active = 1;
		    intx = 2;
		  } else
		  if(intx==1) {
		    intx++;
		  }
		  status &= 0xef;		/* clr TDRE */
		}
		break;
	case ACIA_SR:
		if(fd>=0) rs232_close(fd);
		fd = -1;
		status &= ~4;
		cmd &= 0xe0;
		intx = 0;
		mycpu_set_int(I_MYACIA, 0);
		irq = 0;
		mycpu_unset_alarm(A_MYACIA);
                alarm_active = 0;
		break;
	case ACIA_CTRL:
		ctrl = b;
                acia_ticks = machine_get_cycles_per_second() 
                                / acia_baud_table[ctrl & 0xf];
		break;
	case ACIA_CMD:
		cmd = b;
		if((cmd & 1) && (fd<0)) {
		  fd = rs232_open(myacia_device);
		  mycpu_set_alarm(A_MYACIA, acia_ticks);
                  alarm_active = 1;
		} else
		if(fd>=0 && !(cmd&1)) {
		  rs232_close(fd);
		  mycpu_unset_alarm(A_MYACIA);
                  alarm_active = 0;
		  fd = -1;
		}
		break;
	}
}

BYTE REGPARM1 read_myacia(ADDRESS a) {
#if 0 /* def DEBUG */
	BYTE read_myacia_(ADDRESS);
	BYTE b = read_myacia_(a);
	static ADDRESS lasta = 0;
	static BYTE lastb = 0;

	if((a!=lasta) || (b!=lastb)) {
	  printf("read_myacia(%04x) -> %02x\n",a,b);
	}
	lasta = a; lastb = b;
	return b;
}
BYTE read_myacia_(ADDRESS a) {
#endif

	switch(a & 3) {
	case ACIA_DR:
		status &= ~8;
		return rxdata;
	case ACIA_SR:
		{
		  BYTE c = status | (irq?0x80:0);
		  mycpu_set_int(I_MYACIA, 0);
		  irq = 0;
		  return c;
		}
	case ACIA_CTRL:
		return ctrl;
	case ACIA_CMD:
		return cmd;
	}
	return 0;
}

BYTE peek_myacia(ADDRESS a) {

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

int int_myacia(long offset) {
#if 0 /*def DEBUG*/
	printf("int_myacia(clk=%ld)\n",myclk-offset);
#endif

	if(intx==2 && fd>=0) rs232_putc(fd,txdata);
	if(intx) intx--;

	if(!(status&0x10)) {
	  status |= 0x10;
	}

        if( fd>=0 && (!(status&8)) && rs232_getc(fd, &rxdata)) {
          status |= 8;
        }

	mycpu_set_int(I_MYACIA, myacia_irq);
	irq = 1;

	mycpu_set_alarm(A_MYACIA, acia_ticks);
        alarm_active = 1;

	return 0;
}

