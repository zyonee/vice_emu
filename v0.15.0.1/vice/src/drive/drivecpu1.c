
/*
 * ../../src/drive/drivecpu1.c
 * This file is generated from drivecpu-tmpl.c and ../../src/drive/drivecpu1.def,
 * Do not edit!
 */
/*
 * drivecpu.c - Template file of the 6502 processor in the Commodore 1541
 * floppy disk drive.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Andreas Boose (boose@unixserv.rz.fh-hannover.de)
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

#define __1541__

#include "vice.h"

#include <stdio.h>

#include "types.h"
#include "drive.h"
#include "via.h"
#include "interrupt.h"
#include "ui.h"
#include "resources.h"
#include "viad.h"
#include "ciad.h"
#include "6510core.h"
#include "misc.h"
#include "mon.h"
#include "drivecpu.h"

/* -------------------------------------------------------------------------- */

/* Define this to enable tracing of 1541 instructions.  Warning: this slows
   it down!  */
#define TRACE

/* Force `TRACE' in unstable versions.  */
#if 0 && defined UNSTABLE && !defined TRACE
#define TRACE
#endif

#ifdef TRACE
/* Flag: do we trace instructions while they are executed?  */
int drive1_traceflg;
#endif

/* Interrupt/alarm status.  */
struct cpu_int_status drive1_int_status;

/* Value of clk for the last time drive1_cpu_execute() was called.  */
static CLOCK last_clk;

/* Number of cycles in excess we executed last time drive1_cpu_execute()
   was called.  */
static CLOCK last_exc_cycles;

/* This is non-zero each time a Read-Modify-Write instructions that accesses
   memory is executed.  We can emulate the RMW bug of the 6502 this way.  */
int drive1_rmw_flag = 0;

/* Information about the last executed opcode.  */
opcode_info_t drive1_last_opcode_info;

/* Public copy of the registers.  */
mos6510_regs_t drive1_cpu_regs;

/* Monitor interface.  */
monitor_interface_t drive1_monitor_interface = {

    /* Pointer to the registers of the CPU.  */
    &drive1_cpu_regs,

    /* Pointer to the alarm/interrupt status.  */
    &drive1_int_status,

    /* Pointer to the machine's clock counter.  */
    &drive_clk[1],

    /* Pointer to a function that writes to memory.  */
    drive1_read,

    /* Pointer to a function that reads from memory.  */
    drive1_store,

    /* Pointer to a function to disable/enable watchpoint checking.  */
    drive1_toggle_watchpoints

};

/* ------------------------------------------------------------------------- */

/* This defines the memory access for the 1541 CPU.  */

typedef BYTE REGPARM1 drive1_read_func_t(ADDRESS);
typedef void REGPARM2 drive1_store_func_t(ADDRESS, BYTE);

static drive1_read_func_t *read_func[0x41];
static drive1_store_func_t *store_func[0x41];
static drive1_read_func_t *read_func_watch[0x41];
static drive1_store_func_t *store_func_watch[0x41];
static drive1_read_func_t *read_func_nowatch[0x41];
static drive1_store_func_t *store_func_nowatch[0x41];

#define LOAD(a)		  (read_func[(a) >> 10]((ADDRESS)(a)))
#define LOAD_ZERO(a)	  (drive1_ram[(a) & 0xff])
#define LOAD_ADDR(a)      (LOAD(a) | (LOAD((a) + 1) << 8))
#define LOAD_ZERO_ADDR(a) (LOAD_ZERO(a) | (LOAD_ZERO((a) + 1) << 8))
#define STORE(a, b)	  (store_func[(a) >> 10]((ADDRESS)(a), (BYTE)(b)))
#define STORE_ZERO(a, b)  (drive1_ram[(a) & 0xff] = (b))


static BYTE REGPARM1 drive1_read_ram(ADDRESS address)
{
    return drive1_ram[address & 0x1fff];
}

static void REGPARM2 drive1_store_ram(ADDRESS address, BYTE value)
{
    /* FIXME: Needs to be `0x1fff' for 1581.  */
    drive1_ram[address & 0x7ff] = value;
}

static BYTE REGPARM1 drive1_read_rom(ADDRESS address)
{
    return drive[1].rom[address & 0x7fff];
}

static BYTE REGPARM1 drive1_read_free(ADDRESS address)
{
    return address >> 8;
}

static void REGPARM2 drive1_store_free(ADDRESS address, BYTE value)
{
    return;
}

/* This defines the watchpoint memory access for the 1541 CPU.  */

static BYTE REGPARM1 drive1_read_watch(ADDRESS address)
{
    mon_watch_push_load_addr(address, e_disk_space);
    return read_func_nowatch[address>>10](address);
}

static void REGPARM2 drive1_store_watch(ADDRESS address, BYTE value)
{
    mon_watch_push_store_addr(address, e_disk_space);
    store_func_nowatch[address>>10](address, value);
}
/* FIXME: pc can not jump to VIA adress space in 1541 and 1571 emulation.  */
#define JUMP(addr)				\
  do {						\
      reg_pc = (addr);				\
      if (reg_pc < 0x2000) {			\
	  bank_base = drive1_ram;		\
      } else if (reg_pc >= 0x8000)		\
	  bank_base = drive[1].rom - 0x8000;	\
      else					\
	  bank_base = NULL;			\
  } while (0)

#define pagezero	(drive1_ram)
#define pageone		(drive1_ram + 0x100)

/* ------------------------------------------------------------------------- */

/* This is the external interface for memory access.  */

BYTE REGPARM1 drive1_read(ADDRESS address)
{
    return read_func[address >> 10](address);
}

void REGPARM2 drive1_store(ADDRESS address, BYTE value)
{
    store_func[address >> 10](address, value);
}

/* ------------------------------------------------------------------------- */

/* This table is used to approximate the sync between the main and the 1541
   CPUs, since the two clock rates are different.  */
#define MAX_TICKS 0x1000
static unsigned long clk_conv_table[MAX_TICKS + 1];
static unsigned long clk_mod_table[MAX_TICKS + 1];

void drive1_cpu_set_sync_factor(unsigned int sync_factor)
{
    unsigned long i;

    for (i = 0; i <= MAX_TICKS; i++) {
	unsigned long tmp = i * (unsigned long)sync_factor;

	clk_conv_table[i] = tmp / 0x10000;
	clk_mod_table[i] = tmp % 0x10000;
    }
}

/* ------------------------------------------------------------------------- */

static void reset(void)
{
    int preserve_monitor;

    preserve_monitor = drive1_int_status.global_pending_int & IK_MONITOR;

    printf("DRIVE#9: RESET\n");
    cpu_int_status_init(&drive1_int_status, DRIVE_NUMOFALRM,
			DRIVE_NUMOFINT, &drive1_last_opcode_info);
    drive1_int_status.alarm_handler[A_VIA1D1T1] = int_via1d1t1;
    drive1_int_status.alarm_handler[A_VIA1D1T2] = int_via1d1t2;
    drive1_int_status.alarm_handler[A_VIA2D1T1] = int_via2d1t1;
    drive1_int_status.alarm_handler[A_VIA2D1T2] = int_via2d1t2;
    drive1_int_status.alarm_handler[A_CIA1571D1TOD] = int_cia1571d1tod;
    drive1_int_status.alarm_handler[A_CIA1571D1TA] = int_cia1571d1ta;
    drive1_int_status.alarm_handler[A_CIA1571D1TB] = int_cia1571d1tb;
    drive1_int_status.alarm_handler[A_CIA1581D1TOD] = int_cia1581d1tod;
    drive1_int_status.alarm_handler[A_CIA1581D1TA] = int_cia1581d1ta;
    drive1_int_status.alarm_handler[A_CIA1581D1TB] = int_cia1581d1tb;

    drive_clk[1] = 6;
    reset_via1d1();
    reset_via2d1();
    reset_cia1571d1();
    reset_cia1581d1();
    if (preserve_monitor)
	monitor_trap_on(&drive1_int_status);
}

static void drive1_mem_init(int type)
{
    int i;

    for (i = 0; i < 0x41; i++) {
	read_func_watch[i] = drive1_read_watch;
	store_func_watch[i] = drive1_store_watch;
	read_func_nowatch[i] = drive1_read_free;
	store_func_nowatch[i] = drive1_store_free;
    }

    /* Setup firmware ROM.  */
    if (type == DRIVE_TYPE_1541)
        for (i = 0x30; i < 0x40; i++)
            read_func_nowatch[i] = drive1_read_rom;

    if (type == DRIVE_TYPE_1571 || type == DRIVE_TYPE_1581)
        for (i = 0x20; i < 0x40; i++)
            read_func_nowatch[i] = drive1_read_rom;

    /* Setup drive RAM.  */
    read_func_nowatch[0x0] = read_func_nowatch[0x1] = drive1_read_ram;
    store_func_nowatch[0x0] = store_func_nowatch[0x1] = drive1_store_ram;
    read_func_nowatch[0x40] = drive1_read_ram;
    store_func_nowatch[0x40] = drive1_store_ram;

    if (type == DRIVE_TYPE_1581)
        for (i = 0x0; i < 0x8; i++) {
            read_func_nowatch[i] = drive1_read_ram;
            store_func_nowatch[i] = drive1_store_ram;
        }

    /* Setup 1541 and 1571 VIAs.  */
    if (type == DRIVE_TYPE_1541 || type == DRIVE_TYPE_1571) {
        read_func_nowatch[0x6] = read_via1d1;
        store_func_nowatch[0x6] = store_via1d1;
        read_func_nowatch[0x7] = read_via2d1;
        store_func_nowatch[0x7] = store_via2d1;
    }

    /* Setup 1571 CIA.  */
    if (type == DRIVE_TYPE_1571) {
        read_func_nowatch[0x10] = read_cia1571d1;
        store_func_nowatch[0x10] = store_cia1571d1;
    }

    /* Setup 1581 CIA.  */
    if (type == DRIVE_TYPE_1581) {
        read_func_nowatch[0x10] = read_cia1581d1;
        store_func_nowatch[0x10] = store_cia1581d1;
    }

    memcpy(read_func, read_func_nowatch, sizeof(drive1_read_func_t *) * 0x41);
    memcpy(store_func, store_func_nowatch, sizeof(drive1_store_func_t *) * 0x41);
}

void drive1_toggle_watchpoints(int flag)
{
    if (flag) {
        memcpy(read_func, read_func_watch,
               sizeof(drive1_read_func_t *) * 0x41);
        memcpy(store_func, store_func_watch,
               sizeof(drive1_store_func_t *) * 0x41);
    } else {
        memcpy(read_func, read_func_nowatch,
               sizeof(drive1_read_func_t *) * 0x41);
        memcpy(store_func, store_func_nowatch,
               sizeof(drive1_store_func_t *) * 0x41);
    }
}

void drive1_cpu_reset(void)
{
    int preserve_monitor;

    drive_clk[1] = 0;
    last_clk = 0;
    last_exc_cycles = 0;

    preserve_monitor = drive1_int_status.global_pending_int & IK_MONITOR;

    cpu_int_status_init(&drive1_int_status,
			DRIVE_NUMOFALRM, DRIVE_NUMOFINT,
			&drive1_last_opcode_info);

    if (preserve_monitor)
	monitor_trap_on(&drive1_int_status);

    drive1_trigger_reset();
}

void drive1_cpu_init(int type)
{
    drive1_mem_init(type);
    drive1_cpu_reset();
}

inline void drive1_cpu_wake_up(void)
{
    /* FIXME: this value could break some programs, or be way too high for
       others.  Maybe we should put it into a user-definable resource.  */
    if (clk - last_clk > 0xffffff && drive_clk[1] > 934639) {
	printf("1541: skipping cycles.\n");
	last_clk = clk;
    }
}

inline void drive1_cpu_sleep(void)
{
    /* Currently does nothing.  But we might need this hook some day.  */
}

/* Make sure the 1541 clock counters never overflow; return nonzero if they
   have been decremented to prevent overflow.  */
CLOCK drive1_cpu_prevent_clk_overflow(CLOCK sub)
{
    CLOCK our_sub;

    /* First, get in sync with what the main CPU has done.  */
    last_clk -= sub;

    /* Then, check our own clock counters, and subtract from them if they are
       going to overflow.  The `baseval' is 1 because we don't need the
       number of cycles subtracted to be multiple of a particular value
       (unlike the main CPU).  */
    our_sub = prevent_clk_overflow(&drive1_int_status, &drive_clk[1], 1);
    if (our_sub > 0) {
	via1d1_prevent_clk_overflow(sub);
	via2d1_prevent_clk_overflow(sub);
	cia1571d1_prevent_clk_overflow(sub);
	cia1581d1_prevent_clk_overflow(sub);
    }

    /* Let the caller know what we have done.  */
    return our_sub;
}

/* -------------------------------------------------------------------------- */

/* Execute up to the current main CPU clock value.  This automatically
   calculates the corresponding number of clock ticks in the drive.  */
void drive1_cpu_execute(void)
{
    static BYTE *bank_base;
    static CLOCK cycle_accum;
    static int old_reg_pc;
    CLOCK cycles;

/* #Define the variables for the CPU registers.  In the drive, there is no
   exporting/importing and we just use global variables.  This also makes it
   possible to let the monitor access the CPU status without too much
   headache.   */
#define reg_a   drive1_cpu_regs.reg_a
#define reg_x   drive1_cpu_regs.reg_x
#define reg_y   drive1_cpu_regs.reg_y
#define reg_pc  drive1_cpu_regs.reg_pc
#define reg_sp  drive1_cpu_regs.reg_sp
#define reg_p   drive1_cpu_regs.reg_p
#define flag_z  drive1_cpu_regs.flag_z
#define flag_n  drive1_cpu_regs.flag_n

    drive1_cpu_wake_up();

    if (old_reg_pc != reg_pc) {
	/* Update `bank_base'.  */
	JUMP(reg_pc);
	old_reg_pc = reg_pc;
    }

    cycles = clk - last_clk;

    while (cycles > 0) {
	CLOCK stop_clk;

	if (cycles > MAX_TICKS) {
	    stop_clk = (drive_clk[1] + clk_conv_table[MAX_TICKS]
			- last_exc_cycles);
	    cycle_accum += clk_mod_table[MAX_TICKS];
	    cycles -= MAX_TICKS;
	} else {
	    stop_clk = (drive_clk[1] + clk_conv_table[cycles]
			- last_exc_cycles);
	    cycle_accum += clk_mod_table[cycles];
	    cycles = 0;
	}

	if (cycle_accum >= 0x10000) {
	    cycle_accum -= 0x10000;
	    stop_clk++;
	}

	while (drive_clk[1] < stop_clk) {

#ifdef IO_AREA_WARNING
#warning IO_AREA_WARNING
	    if (!bank_base)
		printf ("Executing from I/O area at $%04X: "
			"$%02X $%02X $%04X at clk %ld\n",
			reg_pc, p0, p1, p2, clk);
#endif

/* Include the 6502/6510 CPU emulation core.  */

#define CLK drive_clk[1]
#define RMW_FLAG drive1_rmw_flag
#define PAGE_ONE (drive1_ram + 0x100)
#define LAST_OPCODE_INFO (drive1_last_opcode_info)
#define TRACEFLG drive1_traceflg

#define CPU_INT_STATUS drive1_int_status

/* FIXME:  We should activate the monitor here.  */
#define JAM()                                                           \
    do {                                                                \
        ui_jam_dialog("   " CPU_STR ": JAM at $%04X   ", reg_pc);       \
        DO_INTERRUPT(IK_RESET);                                         \
    } while (0)

#define ROM_TRAP_ALLOWED() 1

#define ROM_TRAP_HANDLER() \
    drive1_trap_handler()

#define CALLER e_disk_space

#define _drive_set_byte_ready(value) drive_set_byte_ready(value, 1)

#define _drive_byte_ready() drive_byte_ready(1)

#include "6510core.c"

	}

        last_exc_cycles = drive_clk[1] - stop_clk;
    }

    last_clk = clk;
    old_reg_pc = reg_pc;
    drive1_cpu_sleep();
}
