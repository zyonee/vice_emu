
/*
 * ../../src/drive/cia1581drive0.c
 * This file is generated from ../../src/cia-tmpl.c and ../../src/drive/cia1581drive0.def,
 * Do not edit!
 */
/*
 * cia-tmpl.c - Template file for MOS6526 (CIA) emulation.
 *
 * Written by
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
 *
 * Patches and improvements by
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Andreas Boose (boose@rzgw.rz.fh-hannover.de)
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
 * 03jun1997 a.fachat
 * complete timer rewrite
 *
 * There now is a new function, update_cia1581d0(). It computes all differences
 * from one given state to a new state at rclk.  Well, not everything. For
 * the cascade mode (timer B counting timer A) as well as the shift register
 * the interrupt function is being used.
 *
 * The update routine computes, completely independent from the interrupt
 * stuff, the timer register values. The update routines are only called when
 * a timer register is touched.  int_*() is now called only _once_ per
 * interrupt, and it is much smaller than the last version. It is disabled
 * when the interrupt flag is not set (at least timer B. For timer A there
 * must not be a shift register operation and not timer B counting timer A)
 *
 * The timer B when counting timer A and the Shift register interrupt flags
 * may be set a few cycles too late due to late call of int_cia1581d0t*() due to
 * opcode execution time. This can be fixed by checking in the beginning of
 * read_* and write_* if an int_* is scheduled and executing it before.  Then
 * the setting of the ICR could also be moved from update to int_*().  But
 * the bug only affects the contents of the ICR. The interrupt is generated
 * at the right time (hopefully).
 *
 * There is one HACK to make a game work: in update_cia1581d0() a fix is done for
 * Arkanoid. This game counts shift register bits (i.e. TA underflows) by
 * setting TA to one-shot.  The ICR is read just before the int_cia1581d0ta()
 * function is called, and the int bit is missed, so there is a check in
 * update_cia1581d0() (this is probably a fix and not a hack... :-)
 *
 * Still some work has to be put into the dump functions, to really show the
 * state of the chip and not the state of the emulation. I.e. we need to
 * compute the bits described in the emulator test suite.
 *
 */

/*
 * 29jun1998 a.fachat
 *
 * Implementing the peek function assumes that the READ_PA etc macros
 * do not have side-effects, i.e. they can be called more than once
 * at one clock cycle.
 *
 */

#include "vice.h"

#ifdef STDC_HEADERS
#include <stdio.h>
#include <time.h>
#include <string.h>
#endif

#include "cia.h"
#include "log.h"
#include "resources.h"
#include "snapshot.h"

#include "interrupt.h"


#include "drive.h"
#include "iecdrive.h"
#include "ciad.h"

#undef CIA1581D0_TIMER_DEBUG
#undef CIA1581D0_IO_DEBUG
#undef CIA1581D0_DUMP_DEBUG

#define	STORE_OFFSET 0
#define	READ_OFFSET 0

#define	CIAT_STOPPED	0
#define	CIAT_RUNNING	1
#define	CIAT_COUNTTA	2

/*
 * Local variable and prototypes - moved here because they're used by
 * the inline functions 
 */

static void my_set_tbi_clk(CLOCK tbi_clk);
static void my_unset_tbi(void);
static void my_set_tai_clk(CLOCK tai_clk);
static void my_unset_tai(void);

#define	cia1581d0ier	cia1581d0[CIA_ICR]

static alarm_t cia1581d0_ta_alarm;
static alarm_t cia1581d0_tb_alarm;
static alarm_t cia1581d0_tod_alarm;

static int cia1581d0int;		/* Interrupt Flag register for cia 1 */
static CLOCK cia1581d0rdi;		/* real clock = clk-offset */

static CLOCK cia1581d0_tau;		/* when is the next underflow? */
static CLOCK cia1581d0_tai;		/* when is the next int_* scheduled? */
static unsigned int cia1581d0_tal;	/* latch value */
static unsigned int cia1581d0_tac;	/* counter value */
static unsigned int cia1581d0_tat;	/* timer A toggle bit */
static unsigned int cia1581d0_tap;	/* timer A port bit */
static int cia1581d0_tas;		/* timer state (CIAT_*) */

static CLOCK cia1581d0_tbu;		/* when is the next underflow? */
static CLOCK cia1581d0_tbi;		/* when is the next int_* scheduled? */
static unsigned int cia1581d0_tbl;	/* latch value */
static unsigned int cia1581d0_tbc;	/* counter value */
static unsigned int cia1581d0_tbt;	/* timer B toggle bit */
static unsigned int cia1581d0_tbp;	/* timer B port bit */
static int cia1581d0_tbs;		/* timer state (CIAT_*) */

static int cia1581d0sr_bits;	/* number of bits still to send * 2 */

static BYTE oldpa;              /* the actual output on PA (input = high) */
static BYTE oldpb;              /* the actual output on PB (input = high) */

static BYTE cia1581d0todalarm[4];
static BYTE cia1581d0todlatch[4];
static char cia1581d0todstopped;
static char cia1581d0todlatched;
static int cia1581d0todticks = 100000;	/* approx. a 1/10 sec. */

static BYTE cia1581d0flag = 0;

static log_t cia1581d0_log = LOG_ERR;

/* Make the TOD count 50/60Hz even if we do not run at 1MHz ... */
#ifndef CYCLES_PER_SEC
#define	CYCLES_PER_SEC 	1000000
#endif

/* The next two defines are the standard use of the chips. However,
   they can be overriden by a define from the .def file. That's where
   these defs should actually go in the first place. This here is not
   the best solution, but at the moment it's better than editing 9+
   .def files only to find it's slower... However, putting them in the
   .def files would allow making those functions static inline, which
   should be better. But is it as fast? */

/* Fallback for "normal" use of the chip. In real operation the interrupt
   number can be replaced with the known constant I_CIA1581D0FL (see 
   cia1581d0_restore_int() below. */
#ifndef drive0_set_irq_clk
#define	drive0_set_irq_clk(value,num,clk)					\
		set_int(&drive0_int_status,(num),(value),(clk))
#endif

/* Fallback for "normal" use of the chip. */
#ifndef cia1581d0_restore_int
#define	cia1581d0_restore_int(value)					\
		set_int_noclk(&drive0_int_status,(I_CIA1581D0FL),(value))
#endif

/* The following is an attempt in rewriting the interrupt defines into 
   static inline functions. This should not hurt, but I still kept the
   define below, to be able to compare speeds. 
   The semantics of the call has changed, the interrupt number is
   not needed anymore (because it's known to my_set_int(). Actually
   one could also remove IK_IRQ as it is also know... */

/* new semantics and as inline function, value can be replaced by 0/1 */
static inline void my_set_int(int value, CLOCK rclk)
{
#ifdef CIA1581D0_TIMER_DEBUG
    if(cia1581d0_debugFlag) {
        log_message(cia1581d0_log, "set_int(rclk=%d, int=%d, d=%d pc=).",
           rclk,(int_num),(value));
    }
#endif
    if ((value)) {
        cia1581d0int |= 0x80;
        drive0_set_irq_clk((I_CIA1581D0FL), (IK_IRQ), (rclk));
    } else {
        drive0_set_irq_clk((I_CIA1581D0FL), 0, (rclk));
    }
}

/*
 * scheduling int_cia1581d0t[ab] calls -
 * warning: int_cia1581d0ta uses drive0_* stuff!
 */

static inline void my_set_tai_clk(CLOCK tai_clk) 
{
    cia1581d0_tai = tai_clk;
    alarm_set(&cia1581d0_ta_alarm, tai_clk);
}

static inline void my_unset_tai(void) 
{
    cia1581d0_tai = -1;							\
    alarm_unset(&cia1581d0_ta_alarm);
}

static inline void my_set_tbi_clk(CLOCK tbi_clk) 
{
    cia1581d0_tbi = tbi_clk;
    alarm_set(&cia1581d0_tb_alarm, tbi_clk);
}

static inline void my_unset_tbi(void)
{
    cia1581d0_tbi = -1;
    alarm_unset(&cia1581d0_tb_alarm);
}

/*
 * Those routines setup the cia1581d0t[ab]i clocks to a value above
 * rclk and schedule the next int_cia1581d0t[ab] alarm
 */
static inline void update_tai(CLOCK rclk)
{
    if(cia1581d0_tai < rclk) {
        int t = cia1581d0int;
        cia1581d0int = 0;
        int_cia1581d0ta(rclk - cia1581d0_tai);
        cia1581d0int |= t;
    }
}

static inline void update_tbi(CLOCK rclk)
{
    if(cia1581d0_tbi < rclk) {
        int t = cia1581d0int;
        cia1581d0int = 0;
        int_cia1581d0tb(rclk - cia1581d0_tbi);
        cia1581d0int |= t;
    }
}

/* global */

static BYTE cia1581d0[16];

#if defined(CIA1581D0_TIMER_DEBUG) || defined(CIA1581D0_IO_DEBUG)
int cia1581d0_debugFlag = 0;

#endif

/* local functions */

static int update_cia1581d0(CLOCK rclk);
static void check_cia1581d0todalarm(CLOCK rclk);


/* ------------------------------------------------------------------------- */
/* CIA1581D0 */


    static iec_info_t *iec_info;

inline static void check_cia1581d0todalarm(CLOCK rclk)
{
    if (!memcmp(cia1581d0todalarm, cia1581d0 + CIA_TOD_TEN, sizeof(cia1581d0todalarm))) {
	cia1581d0int |= CIA_IM_TOD;
	if (cia1581d0[CIA_ICR] & CIA_IM_TOD) {
            my_set_int(IK_IRQ, drive_clk[0]);
	}
    }
}

static int update_cia1581d0(CLOCK rclk)
{
    int tmp = 0;
    unsigned int ista = 0;
    BYTE sif = (cia1581d0int & cia1581d0ier & 0x7f);
    /* Tick when we virtually added an interrupt flag first. */
    CLOCK added_int_clk = (cia1581d0int & 0x80) ? rclk - 3 : CLOCK_MAX;

#ifdef CIA1581D0_TIMER_DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "update: rclk=%d, tas=%d, tau=%d, tal=%u, ",
	       rclk, cia1581d0_tas, cia1581d0_tau, cia1581d0_tal);
#endif

    if (cia1581d0_tas == CIAT_RUNNING) {
	if (rclk < cia1581d0_tau + 1) {
	    cia1581d0_tac = cia1581d0_tau - rclk;
	    tmp = 0;
	} else {
	    if (cia1581d0[CIA_CRA] & 0x08) {
		tmp = 1;
		if ((cia1581d0ier & CIA_IM_TA)
		    && (cia1581d0_tau < added_int_clk))
		    added_int_clk = cia1581d0_tau;
		cia1581d0_tau = 0;
		my_unset_tai();
		cia1581d0_tac = cia1581d0_tal;
		cia1581d0_tas = CIAT_STOPPED;
		cia1581d0[CIA_CRA] &= 0xfe;

		/* this is a HACK for arkanoid... */
		if (cia1581d0sr_bits) {
		    cia1581d0sr_bits--;
		    if(cia1581d0sr_bits==16) {

    iec_fast_drive_write(cia1581d0[CIA_SDR]);
		    }
		    if (!cia1581d0sr_bits) {
			cia1581d0int |= CIA_IM_SDR;
			if ((cia1581d0ier & CIA_IM_SDR)
			    && (cia1581d0_tau < added_int_clk))
			    added_int_clk = cia1581d0_tau;
		    }
		}
	    } else {
		tmp = (rclk - cia1581d0_tau - 1) / (cia1581d0_tal + 1);
		cia1581d0_tau += tmp * (cia1581d0_tal + 1);
		if ((cia1581d0ier & CIA_IM_TA)
		    && (cia1581d0_tau < added_int_clk))
		    added_int_clk = cia1581d0_tau;
		cia1581d0_tau += 1 * (cia1581d0_tal + 1);
		cia1581d0_tac = cia1581d0_tau - rclk;
	    }

	    if (cia1581d0_tac == cia1581d0_tal)
		ista = 1;

            cia1581d0int |= CIA_IM_TA;
	}
    }
#ifdef CIA1581D0_TIMER_DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "aic=%d, tac-> %u, tau-> %d tmp=%u",
                    added_int_clk, cia1581d0_tac, cia1581d0_tau, tmp);
#endif

    if (cia1581d0[CIA_CRA] & 0x04) {
	cia1581d0_tap = cia1581d0_tat;
    } else {
	cia1581d0_tap = cia1581d0_tac ? 0 : 1;
    }

    cia1581d0_tbp = 0;
    if (cia1581d0_tbs == CIAT_RUNNING) {
	if (rclk < cia1581d0_tbu + 1) {
	    cia1581d0_tbc = cia1581d0_tbu - rclk;
	} else {
	    if (cia1581d0[CIA_CRB] & 0x08) {
		tmp = 1;
		if ((cia1581d0ier & CIA_IM_TB) && (cia1581d0_tbu < added_int_clk))
		    added_int_clk = cia1581d0_tbu;
		cia1581d0_tbu = 0;
		my_unset_tbi();
		cia1581d0_tbc = cia1581d0_tbl;
		cia1581d0_tbs = CIAT_STOPPED;
		cia1581d0[CIA_CRB] &= 0xfe;
	    } else {
		tmp = (rclk - cia1581d0_tbu - 1) / (cia1581d0_tbl + 1);
		cia1581d0_tbu += tmp * (cia1581d0_tbl + 1);
		if ((cia1581d0ier & CIA_IM_TB) && (cia1581d0_tbu < added_int_clk))
		    added_int_clk = cia1581d0_tbu;
		cia1581d0_tbu += 1 * (cia1581d0_tbl + 1);
		cia1581d0_tbc = cia1581d0_tbu - rclk;
	    }
	    if (!cia1581d0_tbc)
		cia1581d0_tbc = cia1581d0_tbl;
            cia1581d0int |= CIA_IM_TB;
	}
    } else if (cia1581d0_tbs == CIAT_COUNTTA) {
	/* missing: set added_int */
	if ((!cia1581d0_tbc) && ista) {
	    cia1581d0_tbp = 1;
	    cia1581d0_tbc = cia1581d0_tbl;
	    cia1581d0int |= CIA_IM_TB;
	}
    }
    if (cia1581d0[CIA_CRB] & 0x04) {
	cia1581d0_tbp ^= cia1581d0_tbt;
    } else {
	cia1581d0_tbp = cia1581d0_tbc ? 0 : 1;
    }

#ifdef CIA1581D0_TIMER_DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "tbc-> %u, tbu-> %d, int %02x ->",
	       cia1581d0_tbc, cia1581d0_tbu, cia1581d0int);
#endif

    /* have we changed the interrupt flags? */
    if (sif != (cia1581d0ier & cia1581d0int & 0x7f)) {
	/* if we do not read ICR, do standard operation */
	if (rclk != cia1581d0rdi) {
	    if (cia1581d0ier & cia1581d0int & 0x7f) {
		/* sets bit 7 */
		my_set_int(IK_IRQ, rclk);
	    }
	} else {
	    if (added_int_clk == cia1581d0rdi) {
		cia1581d0int &= 0x7f;
#ifdef CIA1581D0_TIMER_DEBUG
		if (cia1581d0_debugFlag)
		    log_message(cia1581d0_log, "TA Reading ICR at rclk=%d prevented IRQ.",
			   rclk);
#endif
	    } else {
		if (cia1581d0ier & cia1581d0int & 0x7f) {
		    /* sets bit 7 */
		    my_set_int(IK_IRQ, rclk);
		}
	    }
	}
    }
#ifdef CIA1581D0_TIMER_DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "%02x.", cia1581d0int);
#endif

    /* return true sif interrupt line is set at this clock time */
    return (!sif) && (cia1581d0int & cia1581d0ier & 0x7f);
}

/* ------------------------------------------------------------------------- */

void cia1581d0_init(void)
{
    alarm_init(&cia1581d0_ta_alarm, &drive0_alarm_context, "CIA1581D0TimerA",
               int_cia1581d0ta);
    alarm_init(&cia1581d0_tb_alarm, &drive0_alarm_context, "CIA1581D0TimerB",
               int_cia1581d0tb);
    alarm_init(&cia1581d0_tod_alarm, &drive0_alarm_context, "CIA1581D0TimeOfDay",
               int_cia1581d0tod);
}

void reset_cia1581d0(void)
{
    int i;

    if (cia1581d0_log == LOG_ERR)
        cia1581d0_log = log_open("CIA1581D0");

    cia1581d0todticks = CYCLES_PER_SEC / 10;  /* cycles per tenth of a second */

    for (i = 0; i < 16; i++)
	cia1581d0[i] = 0;

    cia1581d0rdi = 0;
    cia1581d0sr_bits = 0;

    cia1581d0_tac = cia1581d0_tbc = 0xffff;
    cia1581d0_tal = cia1581d0_tbl = 0xffff;

    cia1581d0_tas = CIAT_STOPPED;
    cia1581d0_tbs = CIAT_STOPPED;
    cia1581d0_tat = 0;
    cia1581d0_tbt = 0;

    my_unset_tbi();
    my_unset_tai();

    memset(cia1581d0todalarm, 0, sizeof(cia1581d0todalarm));
    cia1581d0todlatched = 0;
    cia1581d0todstopped = 0;
    alarm_set(&cia1581d0_tod_alarm, drive_clk[0] + cia1581d0todticks);

    cia1581d0int = 0;
    my_set_int(0, drive_clk[0]);

    oldpa = 0xff;
    oldpb = 0xff;


    iec_info = iec_get_drive_port();
}


void REGPARM2 store_cia1581d0(ADDRESS addr, BYTE byte)
{
    CLOCK rclk;

    addr &= 0xf;

    

    rclk = drive_clk[0] - STORE_OFFSET;

#ifdef CIA1581D0_TIMER_DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "store cia1581d0[%02x] %02x @ clk=%d",
                    (int) addr, (int) byte, rclk);
#endif

    switch (addr) {

      case CIA_PRA:		/* port A */
      case CIA_DDRA:
	cia1581d0[addr] = byte;
	byte = cia1581d0[CIA_PRA] | ~cia1581d0[CIA_DDRA];

    drive[0].led_status = byte & 0x40;
	oldpa = byte;
	break;

      case CIA_PRB:		/* port B */
      case CIA_DDRB:
	cia1581d0[addr] = byte;
	byte = cia1581d0[CIA_PRB] | ~cia1581d0[CIA_DDRB];
	if ((cia1581d0[CIA_CRA] | cia1581d0[CIA_CRB]) & 0x02) {
	    update_cia1581d0(rclk);
	    if (cia1581d0[CIA_CRA] & 0x02) {
		byte &= 0xbf;
		if (cia1581d0_tap)
		    byte |= 0x40;
	    }
	    if (cia1581d0[CIA_CRB] & 0x02) {
		byte &= 0x7f;
		if (cia1581d0_tbp)
		    byte |= 0x80;
	    }
	}

    if (byte != oldpb) {
        if (iec_info != NULL) {
            iec_info->drive_data = ~byte;
            iec_info->drive_bus = (((iec_info->drive_data << 3) & 0x40)
                | ((iec_info->drive_data << 6)
                & ((iec_info->drive_data | iec_info->cpu_bus) << 3) & 0x80));
            iec_info->cpu_port = iec_info->cpu_bus & iec_info->drive_bus
                & iec_info->drive2_bus;
            iec_info->drive_port = iec_info->drive2_port = (((iec_info->cpu_port >> 4) & 0x4)
                | (iec_info->cpu_port >> 7)
                | ((iec_info->cpu_bus << 3) & 0x80));
        } else {
            iec_drive0_write(~byte);
        }
    }
	oldpb = byte;
	break;

	/* This handles the timer latches.  The kludgy stuff is an attempt
           emulate the correct behavior when the latch is written to during
           an underflow.  */
      case CIA_TAL:
	update_tai(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk - 1);
	if (cia1581d0_tac == cia1581d0_tal && cia1581d0_tas == CIAT_RUNNING) {
	    cia1581d0_tac = cia1581d0_tal = (cia1581d0_tal & 0xff00) | byte;
	    cia1581d0_tau = rclk + cia1581d0_tac;
	    update_cia1581d0(rclk);
	} else {
	    cia1581d0_tal = (cia1581d0_tal & 0xff00) | byte;
	}
	break;
      case CIA_TBL:
	update_tbi(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk - 1);
	if (cia1581d0_tbc == cia1581d0_tbl && cia1581d0_tbs == CIAT_RUNNING) {
	    cia1581d0_tbc = cia1581d0_tbl = (cia1581d0_tbl & 0xff00) | byte;
	    cia1581d0_tbu = rclk + cia1581d0_tbc + 1;
	    update_cia1581d0(rclk);
	} else {
	    cia1581d0_tbl = (cia1581d0_tbl & 0xff00) | byte;
	}
	break;
      case CIA_TAH:
	update_tai(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk - 1);
	if (cia1581d0_tac == cia1581d0_tal && cia1581d0_tas == CIAT_RUNNING) {
	    cia1581d0_tac = cia1581d0_tal = (cia1581d0_tal & 0x00ff) | (byte << 8);
	    cia1581d0_tau = rclk + cia1581d0_tac;
	    update_cia1581d0(rclk);
	} else {
	    cia1581d0_tal = (cia1581d0_tal & 0x00ff) | (byte << 8);
	}
	if (cia1581d0_tas == CIAT_STOPPED)
	    cia1581d0_tac = cia1581d0_tal;
	break;
      case CIA_TBH:
	update_tbi(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk - 1);
	if (cia1581d0_tbc == cia1581d0_tbl && cia1581d0_tbs == CIAT_RUNNING) {
	    cia1581d0_tbc = cia1581d0_tbl = (cia1581d0_tbl & 0x00ff) | (byte << 8);
	    cia1581d0_tbu = rclk + cia1581d0_tbc + 1;
	    update_cia1581d0(rclk);
	} else {
	    cia1581d0_tbl = (cia1581d0_tbl & 0x00ff) | (byte << 8);
	}
	if (cia1581d0_tbs == CIAT_STOPPED)
	    cia1581d0_tbc = cia1581d0_tbl;
	break;

	/*
	 * TOD clock is stopped by writing Hours, and restarted
	 * upon writing Tenths of Seconds.
	 *
	 * REAL:  TOD register + (wallclock - cia1581d0todrel)
	 * VIRT:  TOD register + (cycles - begin)/cycles_per_sec
	 */
      case CIA_TOD_TEN:	/* Time Of Day clock 1/10 s */
      case CIA_TOD_HR:		/* Time Of Day clock hour */
      case CIA_TOD_SEC:	/* Time Of Day clock sec */
      case CIA_TOD_MIN:	/* Time Of Day clock min */
	/* Mask out undefined bits and flip AM/PM on hour 12
	   (Andreas Boose <boose@rzgw.rz.fh-hannover.de> 1997/10/11). */
	if (addr == CIA_TOD_HR)
	    byte = ((byte & 0x1f) == 18) ? (byte & 0x9f) ^ 0x80 : byte & 0x9f;
	if (cia1581d0[CIA_CRB] & 0x80)
	    cia1581d0todalarm[addr - CIA_TOD_TEN] = byte;
	else {
	    if (addr == CIA_TOD_TEN)
		cia1581d0todstopped = 0;
	    if (addr == CIA_TOD_HR)
		cia1581d0todstopped = 1;
	    cia1581d0[addr] = byte;
	}
	check_cia1581d0todalarm(rclk);
	break;

      case CIA_SDR:		/* Serial Port output buffer */
	cia1581d0[addr] = byte;
	if ((cia1581d0[CIA_CRA] & 0x40) == 0x40) {
	    if (cia1581d0sr_bits <= 16) {
		if(!cia1581d0sr_bits) {

    iec_fast_drive_write(cia1581d0[CIA_SDR]);
		}
		if(cia1581d0sr_bits < 16) {
	            /* switch timer A alarm on again, if necessary */
	            update_cia1581d0(rclk);
	            if (cia1581d0_tau) {
		        my_set_tai_clk(cia1581d0_tau + 1);
	            }
		}

	        cia1581d0sr_bits += 16;

#if defined (CIA1581D0_TIMER_DEBUG)
	        if (cia1581d0_debugFlag)
	    	    log_message(cia1581d0_log, "start SDR rclk=%d.", rclk);
#endif
  	    }
	}
	break;

	/* Interrupts */

      case CIA_ICR:		/* Interrupt Control Register */
	update_cia1581d0(rclk);

#if defined (CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log, "CIA1581D0 set CIA_ICR: 0x%x.", byte);
#endif

	if (byte & CIA_IM_SET) {
	    cia1581d0ier |= (byte & 0x7f);
	} else {
	    cia1581d0ier &= ~(byte & 0x7f);
	}

	/* This must actually be delayed one cycle! */
#if defined(CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log, "    set icr: ifr & ier & 0x7f -> %02x, int=%02x.",
                        cia1581d0ier & cia1581d0int & 0x7f, cia1581d0int);
#endif
	if (cia1581d0ier & cia1581d0int & 0x7f) {
	    my_set_int(IK_IRQ, rclk);
	}
	if (cia1581d0ier & (CIA_IM_TA + CIA_IM_TB)) {
	    if ((cia1581d0ier & CIA_IM_TA) && cia1581d0_tau) {
		my_set_tai_clk(cia1581d0_tau + 1);
	    }
	    if ((cia1581d0ier & CIA_IM_TB) && cia1581d0_tbu) {
		my_set_tbi_clk(cia1581d0_tbu + 1);
	    }
	}
	/* Control */
	break;

      case CIA_CRA:		/* control register A */
	update_tai(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk);
#if defined (CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log, "CIA1581D0 set CIA_CRA: 0x%x (clk=%d, pc=, tal=%u, tac=%u).",
		   byte, rclk, /*program_counter,*/ cia1581d0_tal, cia1581d0_tac);
#endif

	/* bit 7 tod frequency */
	/* bit 6 serial port mode */

	/* bit 4 force load */
	if (byte & 0x10) {
	    cia1581d0_tac = cia1581d0_tal;
	    if (cia1581d0_tas == CIAT_RUNNING) {
		cia1581d0_tau = rclk + cia1581d0_tac + 2;
		my_set_tai_clk(cia1581d0_tau + 1);
	    }
	}
	/* bit 3 timer run mode */
	/* bit 2 & 1 timer output to PB6 */

	/* bit 0 start/stop timer */
	/* bit 5 timer count mode */
	if ((byte & 1) && !(cia1581d0[CIA_CRA] & 1))
	    cia1581d0_tat = 1;
	if ((byte ^ cia1581d0[addr]) & 0x21) {
	    if ((byte & 0x21) == 0x01) {	/* timer just started */
		cia1581d0_tas = CIAT_RUNNING;
		cia1581d0_tau = rclk + (cia1581d0_tac + 1) + ((byte & 0x10) >> 4);
		my_set_tai_clk(cia1581d0_tau + 1);
	    } else {		/* timer just stopped */
		cia1581d0_tas = CIAT_STOPPED;
		cia1581d0_tau = 0;
		/* 1 cycle delay for counter stop. */
		if (!(byte & 0x10)) {
		    /* 1 cycle delay for counter stop.  This must only happen
                       if we are not forcing load at the same time (i.e. bit
                       4 in `byte' is zero). */
		    if (cia1581d0_tac > 0)
			cia1581d0_tac--;
		}
		my_unset_tai();
	    }
	}
#if defined (CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log, "    -> tas=%d, tau=%d.", cia1581d0_tas, cia1581d0_tau);
#endif
	cia1581d0[addr] = byte & 0xef;	/* remove strobe */

	break;

      case CIA_CRB:		/* control register B */
	update_tbi(rclk); /* schedule alarm in case latch value is changed */
	update_cia1581d0(rclk);

#if defined (CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log, "CIA1581D0 set CIA_CRB: 0x%x (clk=%d, pc=, tbl=%u, tbc=%u).",
		   byte, rclk, cia1581d0_tbl, cia1581d0_tbc);
#endif


	/* bit 7 set alarm/tod clock */
	/* bit 4 force load */
	if (byte & 0x10) {
	    cia1581d0_tbc = cia1581d0_tbl;
	    if (cia1581d0_tbs == CIAT_RUNNING) {
		cia1581d0_tbu = rclk + cia1581d0_tbc + 2;
#if defined(CIA1581D0_TIMER_DEBUG)
		if (cia1581d0_debugFlag)
		    log_message(cia1581d0_log, "rclk=%d force load: set tbu alarm to %d.", rclk, cia1581d0_tbu);
#endif
		my_set_tbi_clk(cia1581d0_tbu + 1);
	    }
	}
	/* bit 3 timer run mode */
	/* bit 2 & 1 timer output to PB6 */

	/* bit 0 stbrt/stop timer */
	/* bit 5 & 6 timer count mode */
	if ((byte & 1) && !(cia1581d0[CIA_CRB] & 1))
	    cia1581d0_tbt = 1;
	if ((byte ^ cia1581d0[addr]) & 0x61) {
	    if ((byte & 0x61) == 0x01) {	/* timer just started */
		cia1581d0_tbu = rclk + (cia1581d0_tbc + 1) + ((byte & 0x10) >> 4);
#if defined(CIA1581D0_TIMER_DEBUG)
		if (cia1581d0_debugFlag)
		    log_message(cia1581d0_log, "rclk=%d start timer: set tbu alarm to %d.", rclk, cia1581d0_tbu);
#endif
		my_set_tbi_clk(cia1581d0_tbu + 1);
		cia1581d0_tbs = CIAT_RUNNING;
	    } else {		/* timer just stopped */
#if defined(CIA1581D0_TIMER_DEBUG)
		if (cia1581d0_debugFlag)
		    log_message(cia1581d0_log, "rclk=%d stop timer: set tbu alarm.", rclk);
#endif
		my_unset_tbi();
		cia1581d0_tbu = 0;
		if (!(byte & 0x10)) {
		    /* 1 cycle delay for counter stop.  This must only happen
                       if we are not forcing load at the same time (i.e. bit
                       4 in `byte' is zero). */
		    if (cia1581d0_tbc > 0)
			cia1581d0_tbc--;
		}
		/* this should actually read (byte & 0x61), but as CNT is high
		   by default, bit 0x20 is a `don't care' bit */
		if ((byte & 0x41) == 0x41) {
		    cia1581d0_tbs = CIAT_COUNTTA;
		    update_cia1581d0(rclk);
		    /* switch timer A alarm on if necessary */
		    if (cia1581d0_tau) {
			my_set_tai_clk(cia1581d0_tau + 1);
		    }
		} else {
		    cia1581d0_tbs = CIAT_STOPPED;
		}
	    }
	}
	cia1581d0[addr] = byte & 0xef;	/* remove strobe */
	break;

      default:
	cia1581d0[addr] = byte;
    }				/* switch */
}


/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_cia1581d0(ADDRESS addr)
{

#if defined( CIA1581D0_TIMER_DEBUG )

    BYTE read_cia1581d0_(ADDRESS addr);
    BYTE tmp = read_cia1581d0_(addr);

    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log, "read cia1581d0[%x] returns %02x @ clk=%d.",
                    addr, tmp, drive_clk[0] - READ_OFFSET);
    return tmp;
}

BYTE read_cia1581d0_(ADDRESS addr)
{

#endif

    BYTE byte = 0xff;
    CLOCK rclk;

    addr &= 0xf;

    

    rclk = drive_clk[0] - READ_OFFSET;


    switch (addr) {

      case CIA_PRA:		/* port A */
        /* WARNING: this pin reads the voltage of the output pins, not
           the ORA value. Value read might be different from what is 
	   expected due to excessive load. */

    byte = (0 & ~cia1581d0[CIA_DDRA]) | (cia1581d0[CIA_PRA] & cia1581d0[CIA_DDRA]);
	return byte;
	break;

      case CIA_PRB:		/* port B */
        /* WARNING: this pin reads the voltage of the output pins, not
           the ORA value. Value read might be different from what is 
	   expected due to excessive load. */

    if (iec_info != NULL)
        byte = ((cia1581d0[CIA_PRB] & 0x1a) | iec_info->drive_port) ^ 0x85;
    else
        byte = ((cia1581d0[CIA_PRB] & 0x1a) | iec_drive0_read()) ^ 0x85;
        if ((cia1581d0[CIA_CRA] | cia1581d0[CIA_CRB]) & 0x02) {
	    update_cia1581d0(rclk);
	    if (cia1581d0[CIA_CRA] & 0x02) {
		byte &= 0xbf;
		if (cia1581d0_tap)
		    byte |= 0x40;
	    }
	    if (cia1581d0[CIA_CRB] & 0x02) {
		byte &= 0x7f;
		if (cia1581d0_tbp)
		    byte |= 0x80;
	    }
	}
	return byte;
	break;

	/* Timers */
      case CIA_TAL:		/* timer A low */
	update_cia1581d0(rclk);
	return ((cia1581d0_tac ? cia1581d0_tac : cia1581d0_tal) & 0xff);

      case CIA_TAH:		/* timer A high */
	update_cia1581d0(rclk);
	return ((cia1581d0_tac ? cia1581d0_tac : cia1581d0_tal) >> 8) & 0xff;

      case CIA_TBL:		/* timer B low */
	update_cia1581d0(rclk);
	return cia1581d0_tbc & 0xff;

      case CIA_TBH:		/* timer B high */
	update_cia1581d0(rclk);
	return (cia1581d0_tbc >> 8) & 0xff;

	/*
	 * TOD clock is latched by reading Hours, and released
	 * upon reading Tenths of Seconds. The counter itself
	 * keeps ticking all the time.
	 * Also note that this latching is different from the input one.
	 */
      case CIA_TOD_TEN:	/* Time Of Day clock 1/10 s */
      case CIA_TOD_SEC:	/* Time Of Day clock sec */
      case CIA_TOD_MIN:	/* Time Of Day clock min */
      case CIA_TOD_HR:		/* Time Of Day clock hour */
	if (!cia1581d0todlatched)
	    memcpy(cia1581d0todlatch, cia1581d0 + CIA_TOD_TEN, sizeof(cia1581d0todlatch));
	if (addr == CIA_TOD_TEN)
	    cia1581d0todlatched = 0;
	if (addr == CIA_TOD_HR)
	    cia1581d0todlatched = 1;
	return cia1581d0todlatch[addr - CIA_TOD_TEN];

      case CIA_SDR:		/* Serial Port Shift Register */
	return (cia1581d0[addr]);

	/* Interrupts */

      case CIA_ICR:		/* Interrupt Flag Register */
	{
	    BYTE t = 0;




#ifdef CIA1581D0_TIMER_DEBUG
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log, "CIA1581D0 read intfl: rclk=%d, alarm_ta=%d, alarm_tb=%d",
			rclk, drive0_int_status.alarm_clk[A_CIA1581D0TA],
			drive0_int_status.alarm_clk[A_CIA1581D0TB]);
#endif

	    cia1581d0rdi = rclk;
            t = cia1581d0int;	/* we clean cia1581d0int anyway, so make int_* */
	    cia1581d0int = 0;	/* believe it is already */

            if (rclk >= cia1581d0_tai)
                int_cia1581d0ta(rclk - cia1581d0_tai);
            if (rclk >= cia1581d0_tbi)
                int_cia1581d0tb(rclk - cia1581d0_tbi);

	    cia1581d0int |= t;	/* some bits can be set -> or with old value */

	    update_cia1581d0(rclk);
	    t = cia1581d0int | cia1581d0flag;

#ifdef CIA1581D0_TIMER_DEBUG
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "read intfl gives cia1581d0int=%02x -> %02x "
                            "sr_bits=%d, clk=%d, ta=%d, tb=%d.",
                            cia1581d0int, t, cia1581d0sr_bits, clk,
                            (cia1581d0_tac ? cia1581d0_tac : cia1581d0_tal),
                            cia1581d0_tbc);
#endif

	    cia1581d0flag = 0;
	    cia1581d0int = 0;
	    my_set_int(0, rclk);

	    return (t);
	}
      case CIA_CRA:		/* Control Register A */
      case CIA_CRB:		/* Control Register B */
	update_cia1581d0(rclk);
	return cia1581d0[addr];
    }				/* switch */

    return (cia1581d0[addr]);
}

BYTE REGPARM1 peek_cia1581d0(ADDRESS addr)
{
    /* This code assumes that update_cia1581d0 is a projector - called at
     * the same cycle again it doesn't change anything. This way
     * it does not matter if we call it from peek first in the monitor
     * and probably the same cycle again when the CPU runs on...
     */
    CLOCK rclk;

    addr &= 0xf;

    

    rclk = drive_clk[0] - READ_OFFSET;

    switch (addr) {

	/*
	 * TOD clock is latched by reading Hours, and released
	 * upon reading Tenths of Seconds. The counter itself
	 * keeps ticking all the time.
	 * Also note that this latching is different from the input one.
	 */
      case CIA_TOD_TEN:	/* Time Of Day clock 1/10 s */
      case CIA_TOD_SEC:	/* Time Of Day clock sec */
      case CIA_TOD_MIN:	/* Time Of Day clock min */
      case CIA_TOD_HR:		/* Time Of Day clock hour */
	if (!cia1581d0todlatched)
	    memcpy(cia1581d0todlatch, cia1581d0 + CIA_TOD_TEN, sizeof(cia1581d0todlatch));
	return cia1581d0[addr];

	/* Interrupts */

      case CIA_ICR:		/* Interrupt Flag Register */
	{
	    BYTE t = 0;



#ifdef CIA1581D0_TIMER_DEBUG
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "CIA1581D0 read intfl: rclk=%d, alarm_ta=%d, alarm_tb=%d.",
                            rclk, drive0_int_status.alarm_clk[A_CIA1581D0TA],
                            drive0_int_status.alarm_clk[A_CIA1581D0TB]);
#endif

	    /* cia1581d0rdi = rclk; makes int_* and update_cia1581d0 fiddle with IRQ */
            t = cia1581d0int;	/* we clean cia1581d0int anyway, so make int_* */
	    cia1581d0int = 0;	/* believe it is already */

            if (rclk >= cia1581d0_tai)
                int_cia1581d0ta(rclk - cia1581d0_tai);
            if (rclk >= cia1581d0_tbi)
                int_cia1581d0tb(rclk - cia1581d0_tbi);

	    cia1581d0int |= t;	/* some bits can be set -> or with old value */

	    update_cia1581d0(rclk);
	    t = cia1581d0int | cia1581d0flag;

#ifdef CIA1581D0_TIMER_DEBUG
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "read intfl gives cia1581d0int=%02x -> %02x "
                            "sr_bits=%d, clk=%d, ta=%d, tb=%d.",
                            cia1581d0int, t, cia1581d0sr_bits, clk,
                            (cia1581d0_tac ? cia1581d0_tac : cia1581d0_tal),
                            cia1581d0_tbc);
#endif

/*
	    cia1581d0flag = 0;
	    cia1581d0int = 0;
	    my_set_int(0, rclk);
*/
	    return (t);
	}
      default:
	break;
    }				/* switch */

    return read_cia1581d0(addr);
}

/* ------------------------------------------------------------------------- */

int int_cia1581d0ta(long offset)
{
    CLOCK rclk = drive_clk[0] - offset;

#if defined(CIA1581D0_TIMER_DEBUG)
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log,
                    "int_cia1581d0ta(rclk = %u, tal = %u, cra=%02x.",
                    rclk, cia1581d0_tal, cia1581d0[CIA_CRA]);
#endif

    cia1581d0_tat = (cia1581d0_tat + 1) & 1;

    if ((cia1581d0_tas == CIAT_RUNNING) && !(cia1581d0[CIA_CRA] & 8)) {
	/* if we do not need alarm, no PB6, no shift register, and not timer B
	   counting timer A, then we can savely skip alarms... */
	if ( ( (cia1581d0ier & CIA_IM_TA) &&
		(!(cia1581d0int & 0x80)) )
	    || (cia1581d0[CIA_CRA] & 0x42)
	    || (cia1581d0_tbs == CIAT_COUNTTA)) {
	    if(offset > cia1581d0_tal+1) {
	        my_set_tai_clk(
			drive_clk[0] - (offset % (cia1581d0_tal+1)) + cia1581d0_tal + 1 );
	    } else {
	        my_set_tai_clk(rclk + cia1581d0_tal + 1 );
	    }
	} else {
            alarm_unset(&cia1581d0_ta_alarm); /* do _not_ clear cia1581d0_tai */
	}
    } else {
	my_unset_tai();
    }

    if (cia1581d0[CIA_CRA] & 0x40) {
	if (cia1581d0sr_bits) {
#if defined(CIA1581D0_TIMER_DEBUG)
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log, "rclk=%d SDR: timer A underflow, bits=%d",
		       rclk, cia1581d0sr_bits);
#endif
	    if (!(--cia1581d0sr_bits)) {
		cia1581d0int |= CIA_IM_SDR;
	    }
	    if(cia1581d0sr_bits == 16) {

    iec_fast_drive_write(cia1581d0[CIA_SDR]);
	    }
	}
    }
    if (cia1581d0_tbs == CIAT_COUNTTA) {
	if (!cia1581d0_tbc) {
	    cia1581d0_tbc = cia1581d0_tbl;
	    cia1581d0_tbu = rclk;
#if defined(CIA1581D0_TIMER_DEBUG)
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "timer B underflow when counting timer A occured, rclk=%d!", rclk);
#endif
	    cia1581d0int |= CIA_IM_TB;
	    my_set_tbi_clk(rclk);
	} else {
	    cia1581d0_tbc--;
	}
    }

    /* CIA_IM_TA is not set here, as it can be set in update(), reset
       by reading the ICR and then set again here because of delayed
       calling of int() */
    if ((IK_IRQ == IK_NMI && cia1581d0rdi != rclk - 1)
        || (IK_IRQ == IK_IRQ && cia1581d0rdi < rclk - 1)) {
        if ((cia1581d0int | CIA_IM_TA) & cia1581d0ier & 0x7f) {
            my_set_int(IK_IRQ, rclk);
        }
    }

    return 0;
}


/*
 * Timer B can run in 2 (4) modes
 * cia1581d0[f] & 0x60 == 0x00   count system 02 pulses
 * cia1581d0[f] & 0x60 == 0x40   count timer A underflows
 * cia1581d0[f] & 0x60 == 0x20 | 0x60 count CNT pulses => counter stops
 */


int int_cia1581d0tb(long offset)
{
    CLOCK rclk = drive_clk[0] - offset;

#if defined(CIA1581D0_TIMER_DEBUG)
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log,
                    "timer B int_cia1581d0tb(rclk=%d, tbs=%d).", rclk, cia1581d0_tbs);
#endif

    cia1581d0_tbt = (cia1581d0_tbt + 1) & 1;

    /* running and continous, then next alarm */
    if (cia1581d0_tbs == CIAT_RUNNING) {
	if (!(cia1581d0[CIA_CRB] & 8)) {
#if defined(CIA1581D0_TIMER_DEBUG)
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "rclk=%d cia1581d0tb: set tbu alarm to %d.",
                            rclk, rclk + cia1581d0_tbl + 1);
#endif
	    /* if no interrupt flag we can safely skip alarms */
	    if (cia1581d0ier & CIA_IM_TB) {
		if(offset > cia1581d0_tbl+1) {
		    my_set_tbi_clk(
			drive_clk[0] - (offset % (cia1581d0_tbl+1)) + cia1581d0_tbl + 1);
		} else {
		    my_set_tbi_clk(rclk + cia1581d0_tbl + 1);
		}
	    } else {
		/* cia1581d0_tbi = rclk + cia1581d0_tbl + 1; */
		alarm_unset(&cia1581d0_tb_alarm);
	    }
	} else {
#if 0
	    cia1581d0_tbs = CIAT_STOPPED;
	    cia1581d0[CIA_CRB] &= 0xfe; /* clear start bit */
	    cia1581d0_tbu = 0;
#endif /* 0 */
#if defined(CIA1581D0_TIMER_DEBUG)
	    if (cia1581d0_debugFlag)
		log_message(cia1581d0_log,
                            "rclk=%d cia1581d0tb: unset tbu alarm.", rclk);
#endif
	    my_unset_tbi();
	}
    } else {
	if (cia1581d0_tbs == CIAT_COUNTTA) {
	    if ((cia1581d0[CIA_CRB] & 8)) {
		cia1581d0_tbs = CIAT_STOPPED;
		cia1581d0[CIA_CRB] &= 0xfe;		/* clear start bit */
		cia1581d0_tbu = 0;
	    }
	}
	cia1581d0_tbu = 0;
	my_unset_tbi();
#if defined(CIA1581D0_TIMER_DEBUG)
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log,
                        "rclk=%d cia1581d0tb: unset tbu alarm.", rclk);
#endif
    }

    if ((IK_IRQ == IK_NMI && cia1581d0rdi != rclk - 1)
        || (IK_IRQ == IK_IRQ && cia1581d0rdi < rclk - 1)) {
        if ((cia1581d0int | CIA_IM_TB) & cia1581d0ier & 0x7f) {
            my_set_int(IK_IRQ, rclk);
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

void cia1581d0_set_flag(void)
{
    cia1581d0int |= CIA_IM_FLG;
    if (cia1581d0[CIA_ICR] & CIA_IM_FLG) {
        my_set_int(IK_IRQ, drive_clk[0]);
    }
}

void cia1581d0_set_sdr(BYTE data)
{
    cia1581d0[CIA_SDR] = data;
    cia1581d0int |= CIA_IM_SDR;
    if (cia1581d0[CIA_ICR] & CIA_IM_SDR) {
        my_set_int(IK_IRQ, drive_clk[0]);
    }
}

/* ------------------------------------------------------------------------- */

int int_cia1581d0tod(long offset)
{
    int t, pm;
    CLOCK rclk = drive_clk[0] - offset;

#ifdef DEBUG
    if (cia1581d0_debugFlag)
	log_message(cia1581d0_log,
                    "TOD timer event (1/10 sec tick), tod=%02x:%02x,%02x.%x.",
                    cia1581d0[CIA_TOD_HR], cia1581d0[CIA_TOD_MIN], cia1581d0[CIA_TOD_SEC],
                    cia1581d0[CIA_TOD_TEN]);
#endif

    /* set up new int */
    alarm_set(&cia1581d0_tod_alarm, drive_clk[0] + cia1581d0todticks);

    if (!cia1581d0todstopped) {
	/* inc timer */
	t = bcd2byte(cia1581d0[CIA_TOD_TEN]);
	t++;
	cia1581d0[CIA_TOD_TEN] = byte2bcd(t % 10);
	if (t >= 10) {
	    t = bcd2byte(cia1581d0[CIA_TOD_SEC]);
	    t++;
	    cia1581d0[CIA_TOD_SEC] = byte2bcd(t % 60);
	    if (t >= 60) {
		t = bcd2byte(cia1581d0[CIA_TOD_MIN]);
		t++;
		cia1581d0[CIA_TOD_MIN] = byte2bcd(t % 60);
		if (t >= 60) {
		    pm = cia1581d0[CIA_TOD_HR] & 0x80;
		    t = bcd2byte(cia1581d0[CIA_TOD_HR] & 0x1f);
		    if (!t)
			pm ^= 0x80;	/* toggle am/pm on 0:59->1:00 hr */
		    t++;
		    t = t % 12 | pm;
		    cia1581d0[CIA_TOD_HR] = byte2bcd(t);
		}
	    }
	}
#ifdef DEBUG
	if (cia1581d0_debugFlag)
	    log_message(cia1581d0_log,
                        "TOD after event :tod=%02x:%02x,%02x.%x.",
                        cia1581d0[CIA_TOD_HR], cia1581d0[CIA_TOD_MIN], cia1581d0[CIA_TOD_SEC],
                        cia1581d0[CIA_TOD_TEN]);
#endif
	/* check alarm */
	check_cia1581d0todalarm(rclk);
    }
    return 0;
}

/* -------------------------------------------------------------------------- */


void cia1581d0_prevent_clk_overflow(CLOCK sub)
{

    update_tai(drive_clk[0]);
    update_tbi(drive_clk[0]);

    update_cia1581d0(drive_clk[0]);

    if(cia1581d0_tai && (cia1581d0_tai != -1))
        cia1581d0_tai -= sub;
    if(cia1581d0_tbi && (cia1581d0_tbi != -1))
        cia1581d0_tbi -= sub;

    if (cia1581d0_tau)
	cia1581d0_tau -= sub;
    if (cia1581d0_tbu)
	cia1581d0_tbu -= sub;
    if (cia1581d0rdi > sub)
	cia1581d0rdi -= sub;
    else
	cia1581d0rdi = 0;
}

/* -------------------------------------------------------------------------- */

/* The dump format has a module header and the data generated by the
 * chip...
 *
 * The version of this dump description is 0/0
 */

#define	CIA_DUMP_VER_MAJOR	1
#define	CIA_DUMP_VER_MINOR	0

/*
 * The dump data:
 *
 * UBYTE	ORA
 * UBYTE	ORB
 * UBYTE	DDRA
 * UBYTE	DDRB
 * UWORD	TA
 * UWORD 	TB
 * UBYTE	TOD_TEN		current value
 * UBYTE	TOD_SEC
 * UBYTE	TOD_MIN
 * UBYTE	TOD_HR
 * UBYTE	SDR
 * UBYTE	IER		enabled interrupts mask
 * UBYTE	CRA
 * UBYTE	CRB
 *
 * UWORD	TAL		latch value
 * UWORD	TBL		latch value
 * UBYTE	IFR		interrupts currently active
 * UBYTE	PBSTATE		bit6/7 reflect PB6/7 toggle state
 *				bit 2/3 reflect port bit state
 * UBYTE	SRHBITS		number of half-bits to still shift in/out SDR
 * UBYTE	ALARM_TEN
 * UBYTE	ALARM_SEC
 * UBYTE	ALARM_MIN
 * UBYTE	ALARM_HR
 *
 * UBYTE	READICR		clk - when ICR was read last + 128
 * UBYTE	TODLATCHED	0=running, 1=latched, 2=stopped (writing)
 * UBYTE	TODL_TEN		latch value
 * UBYTE	TODL_SEC
 * UBYTE	TODL_MIN
 * UBYTE	TODL_HR
 * DWORD	TOD_TICKS	clk ticks till next tenth of second
 *
 * UBYTE	IRQ		0=IRQ line inactive, 1=IRQ line active
 */

/* FIXME!!!  Error check.  */
int cia1581d0_write_snapshot_module(snapshot_t *p)
{
    snapshot_module_t *m;
    int byte;

    m = snapshot_module_create(p, "CIA1581D0",
                               CIA_DUMP_VER_MAJOR, CIA_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    update_tai(drive_clk[0]); /* schedule alarm in case latch value is changed */
    update_tbi(drive_clk[0]); /* schedule alarm in case latch value is changed */
    update_cia1581d0(drive_clk[0]);

#ifdef CIA1581D0_DUMP_DEBUG
    log_message(cia1581d0_log, "clk=%d, cra=%02x, crb=%02x, tas=%d, tbs=%d",drive_clk[0], cia1581d0[CIA_CRA], cia1581d0[CIA_CRB],cia1581d0_tas, cia1581d0_tbs);
    log_message(cia1581d0_log, "tai=%d, tau=%d, tac=%04x, tal=%04x",cia1581d0_tai, cia1581d0_tau, cia1581d0_tac, cia1581d0_tal);
    log_message(cia1581d0_log, "tbi=%d, tbu=%d, tbc=%04x, tbl=%04x",cia1581d0_tbi, cia1581d0_tbu, cia1581d0_tbc, cia1581d0_tbl);
    log_message(cia1581d0_log, "write cia1581d0int=%02x, cia1581d0ier=%02x", cia1581d0int, cia1581d0ier);
#endif

    snapshot_module_write_byte(m, cia1581d0[CIA_PRA]);
    snapshot_module_write_byte(m, cia1581d0[CIA_PRB]);
    snapshot_module_write_byte(m, cia1581d0[CIA_DDRA]);
    snapshot_module_write_byte(m, cia1581d0[CIA_DDRB]);
    snapshot_module_write_word(m, cia1581d0_tac);
    snapshot_module_write_word(m, cia1581d0_tbc);
    snapshot_module_write_byte(m, cia1581d0[CIA_TOD_TEN]);
    snapshot_module_write_byte(m, cia1581d0[CIA_TOD_SEC]);
    snapshot_module_write_byte(m, cia1581d0[CIA_TOD_MIN]);
    snapshot_module_write_byte(m, cia1581d0[CIA_TOD_HR]);
    snapshot_module_write_byte(m, cia1581d0[CIA_SDR]);
    snapshot_module_write_byte(m, cia1581d0[CIA_ICR]);
    snapshot_module_write_byte(m, cia1581d0[CIA_CRA]);
    snapshot_module_write_byte(m, cia1581d0[CIA_CRB]);

    snapshot_module_write_word(m, cia1581d0_tal);
    snapshot_module_write_word(m, cia1581d0_tbl);
    snapshot_module_write_byte(m, peek_cia1581d0(CIA_ICR));
    snapshot_module_write_byte(m, ((cia1581d0_tat ? 0x40 : 0)
                                   | (cia1581d0_tbt ? 0x80 : 0)));
    snapshot_module_write_byte(m, cia1581d0sr_bits);
    snapshot_module_write_byte(m, cia1581d0todalarm[0]);
    snapshot_module_write_byte(m, cia1581d0todalarm[1]);
    snapshot_module_write_byte(m, cia1581d0todalarm[2]);
    snapshot_module_write_byte(m, cia1581d0todalarm[3]);

    if(cia1581d0rdi) {
	if((drive_clk[0] - cia1581d0rdi) > 120) {
	    byte = 0;
	} else {
	    byte = drive_clk[0] + 128 - cia1581d0rdi;
	}
    } else {
	byte = 0;
    }
    snapshot_module_write_byte(m, byte);

    snapshot_module_write_byte(m, ((cia1581d0todlatched ? 1 : 0)
                                   | (cia1581d0todstopped ? 2 : 0)));
    snapshot_module_write_byte(m, cia1581d0todlatch[0]);
    snapshot_module_write_byte(m, cia1581d0todlatch[1]);
    snapshot_module_write_byte(m, cia1581d0todlatch[2]);
    snapshot_module_write_byte(m, cia1581d0todlatch[3]);

    /* FIXME */
#if 0
    snapshot_module_write_dword(m, (drive0_int_status.alarm_clk[A_CIA1581D0TOD]
                                    - drive_clk[0]));
#endif

    snapshot_module_close(m);

    return 0;
}

int cia1581d0_read_snapshot_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    BYTE byte;
    WORD word;
    DWORD dword;
    ADDRESS addr;
    CLOCK rclk = drive_clk[0];
    snapshot_module_t *m;

    m = snapshot_module_open(p, "CIA1581D0", &vmajor, &vminor);
    if (m == NULL)
        return -1;

    if (vmajor != CIA_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    /* stop timers, just in case */
    cia1581d0_tas = CIAT_STOPPED;
    cia1581d0_tau = 0;
    my_unset_tai();
    cia1581d0_tbs = CIAT_STOPPED;
    cia1581d0_tbu = 0;
    my_unset_tbi();
    alarm_unset(&cia1581d0_tod_alarm);

    {
        snapshot_module_read_byte(m, &cia1581d0[CIA_PRA]);
        snapshot_module_read_byte(m, &cia1581d0[CIA_PRB]);
        snapshot_module_read_byte(m, &cia1581d0[CIA_DDRA]);
        snapshot_module_read_byte(m, &cia1581d0[CIA_DDRB]);

        addr = CIA_DDRA;
	byte = cia1581d0[CIA_PRA] | ~cia1581d0[CIA_DDRA];
        oldpa = byte ^ 0xff;	/* all bits change? */

    drive[0].led_status = byte & 0x40;
        oldpa = byte;

        addr = CIA_DDRB;
	byte = cia1581d0[CIA_PRB] | ~cia1581d0[CIA_DDRB];
        oldpb = byte ^ 0xff;	/* all bits change? */
        /* FIXME!!!! */
        oldpb = byte;
    }

    snapshot_module_read_word(m, &word);
    cia1581d0_tac = word;
    snapshot_module_read_word(m, &word);
    cia1581d0_tbc = word;
    snapshot_module_read_byte(m, &cia1581d0[CIA_TOD_TEN]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_TOD_SEC]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_TOD_MIN]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_TOD_HR]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_SDR]);
    {

    iec_fast_drive_write(cia1581d0[CIA_SDR]);
    }
    snapshot_module_read_byte(m, &cia1581d0[CIA_ICR]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_CRA]);
    snapshot_module_read_byte(m, &cia1581d0[CIA_CRB]);

    snapshot_module_read_word(m, &word);
    cia1581d0_tal = word;
    snapshot_module_read_word(m, &word);
    cia1581d0_tbl = word;

    snapshot_module_read_byte(m, &byte);
    cia1581d0int = byte;

#ifdef CIA1581D0_DUMP_DEBUG
log_message(cia1581d0_log, "read cia1581d0int=%02x, cia1581d0ier=%02x.", cia1581d0int, cia1581d0ier);
#endif

    snapshot_module_read_byte(m, &byte);
    cia1581d0_tat = (byte & 0x40) ? 1 : 0;
    cia1581d0_tbt = (byte & 0x80) ? 1 : 0;
    cia1581d0_tap = (byte & 0x04) ? 1 : 0;
    cia1581d0_tbp = (byte & 0x08) ? 1 : 0;

    snapshot_module_read_byte(m, &byte);
    cia1581d0sr_bits = byte;

    snapshot_module_read_byte(m, &cia1581d0todalarm[0]);
    snapshot_module_read_byte(m, &cia1581d0todalarm[1]);
    snapshot_module_read_byte(m, &cia1581d0todalarm[2]);
    snapshot_module_read_byte(m, &cia1581d0todalarm[3]);

    snapshot_module_read_byte(m, &byte);
    if(byte) {
	cia1581d0rdi = drive_clk[0] + 128 - byte;
    } else {
	cia1581d0rdi = 0;
    }
#ifdef CIA1581D0_DUMP_DEBUG
log_message(cia1581d0_log, "snap read rdi=%02x", byte);
log_message(cia1581d0_log, "snap setting rdi to %d (rclk=%d)", cia1581d0rdi, drive_clk[0]);
#endif

    snapshot_module_read_byte(m, &byte);
    cia1581d0todlatched = byte & 1;
    cia1581d0todstopped = byte & 2;
    snapshot_module_read_byte(m, &cia1581d0todlatch[0]);
    snapshot_module_read_byte(m, &cia1581d0todlatch[1]);
    snapshot_module_read_byte(m, &cia1581d0todlatch[2]);
    snapshot_module_read_byte(m, &cia1581d0todlatch[3]);

    snapshot_module_read_dword(m, &dword);
    alarm_set(&cia1581d0_tod_alarm, drive_clk[0] + dword);

    /* timer switch-on code from store_cia1581d0[CIA_CRA/CRB] */

#ifdef CIA1581D0_DUMP_DEBUG
log_message(cia1581d0_log, "clk=%d, cra=%02x, crb=%02x, tas=%d, tbs=%d",drive_clk[0], cia1581d0[CIA_CRA], cia1581d0[CIA_CRB],cia1581d0_tas, cia1581d0_tbs);
log_message(cia1581d0_log, "tai=%d, tau=%d, tac=%04x, tal=%04x",cia1581d0_tai, cia1581d0_tau, cia1581d0_tac, cia1581d0_tal);
log_message(cia1581d0_log, "tbi=%d, tbu=%d, tbc=%04x, tbl=%04x",cia1581d0_tbi, cia1581d0_tbu, cia1581d0_tbc, cia1581d0_tbl);
#endif

    if ((cia1581d0[CIA_CRA] & 0x21) == 0x01) {        /* timer just started */
        cia1581d0_tas = CIAT_RUNNING;
        cia1581d0_tau = rclk + (cia1581d0_tac /*+ 1) + ((byte & 0x10) >> 4*/ );
        my_set_tai_clk(cia1581d0_tau + 1);
    }

    if ((cia1581d0[CIA_CRB] & 0x61) == 0x01) {        /* timer just started */
        cia1581d0_tbu = rclk + (cia1581d0_tbc /*+ 1) + ((byte & 0x10) >> 4*/ );
        my_set_tbi_clk(cia1581d0_tbu + 1);
        cia1581d0_tbs = CIAT_RUNNING;
    } else
    if ((cia1581d0[CIA_CRB] & 0x41) == 0x41) {
        cia1581d0_tbs = CIAT_COUNTTA;
        update_cia1581d0(rclk);
        /* switch timer A alarm on if necessary */
        if (cia1581d0_tau) {
            my_set_tai_clk(cia1581d0_tau + 1);
        }
    }

#ifdef CIA1581D0_DUMP_DEBUG
log_message(cia1581d0_log, "clk=%d, cra=%02x, crb=%02x, tas=%d, tbs=%d",drive_clk[0], cia1581d0[CIA_CRA], cia1581d0[CIA_CRB],cia1581d0_tas, cia1581d0_tbs);
log_message(cia1581d0_log, "tai=%d, tau=%d, tac=%04x, tal=%04x",cia1581d0_tai, cia1581d0_tau, cia1581d0_tac, cia1581d0_tal);
log_message(cia1581d0_log, "tbi=%d, tbu=%d, tbc=%04x, tbl=%04x",cia1581d0_tbi, cia1581d0_tbu, cia1581d0_tbc, cia1581d0_tbl);
#endif

    if (cia1581d0[CIA_ICR] & 0x80) {
        cia1581d0_restore_int(IK_IRQ);
    } else {
        cia1581d0_restore_int(0);
    }

    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}





