
/*
 * ../../src/drive/via2drive0.c
 * This file is generated from ../../src/via-tmpl.c and ../../src/drive/via2drive0.def,
 * Do not edit!
 */
/*
 * via-tmpl.c - Template file for VIA emulation.
 *
 * Written by
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
 *
 * Patch by
 *  Andreas Boose (boose@linux.rz.fh-hannover.de)
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
 * 24jan97 a.fachat
 * new interrupt handling, hopefully according to the specs now.
 * All interrupts (note: not timer events (i.e. alarms) are put
 * into one interrupt flag, I_VIA2D0FL.
 * if an interrupt condition changes, the function (i.e. cpp macro)
 * update_via2d0irq() id called, that checks the IRQ line state.
 * This is now possible, as ettore has decoupled A_* alarm events
 * from I_* interrupts for performance reasons.
 *
 * A new function for signaling rising/falling edges on the
 * control lines is introduced:
 *      via2d0_signal(VIA_SIG_[CA1|CA2|CB1|CB2], VIA_SIG_[RISE|FALL])
 * which signals the corresponding edge to the VIA. The constants
 * are defined in via.h.
 *
 * Except for shift register and input latching everything should be ok now.
 */

#include "vice.h"

#include <stdio.h>
#include <time.h>

#include "vmachine.h"
#include "via.h"
#include "resources.h"
#include "snapshot.h"


    #include "drive.h"
    #include "viad.h"

#include "interrupt.h"

/*#define VIA2D0_TIMER_DEBUG */
				/*#define VIA2D0_NEED_PB7 *//* when PB7 is really used, set this
				   to enable pulse output from the timer.
				   Otherwise PB7 state is computed only
				   when port B is read -
				   not yet implemented */

/* global */

BYTE via2d0[16];



/*
 * Local variables
 */

static int via2d0ifr;		/* Interrupt Flag register for via2d0 */
static int via2d0ier;		/* Interrupt Enable register for via2d0 */

static unsigned int via2d0tal;	/* current timer A latch value */
static unsigned int via2d0tbl;	/* current timer B latch value */

static CLOCK via2d0tau;		/* time when via2d0 timer A is updated */
static CLOCK via2d0tbu;		/* time when via2d0 timer B is updated */
static CLOCK via2d0tai;		/* time when next timer A alarm is */
static CLOCK via2d0tbi;		/* time when next timer A alarm is */

static int via2d0pb7;		/* state of PB7 for pulse output... */
static int via2d0pb7x;		/* to be xored herewith  */
static int via2d0pb7o;		/* to be ored herewith  */
static int via2d0pb7xx;
static int via2d0pb7sx;

static BYTE oldpa;		/* the actual output on PA (input = high) */
static BYTE oldpb;		/* the actual output on PB (input = high) */

/*
 * local functions
 */

/*
 * 01apr98 a.fachat
 *
 * One-shot Timing (partly from 6522-VIA.txt):

                     +-+ +-+ +-+ +-+ +-+ +-+   +-+ +-+ +-+ +-+ +-+ +-+
                02 --+ +-+ +-+ +-+ +-+ +-+ +-#-+ +-+ +-+ +-+ +-+ +-+ +-
                       |   |                           |
                       +---+                           |
       WRITE T1C-H ----+   +-----------------#-------------------------
        ___                |                           |
        IRQ OUTPUT --------------------------#---------+
                           |                           +---------------
                           |                           |
        PB7 OUTPUT --------+                           +---------------
                           +-----------------#---------+
         T1                | N |N-1|N-2|N-3|     | 0 | -1|N  |N-1|N-2|
         T2                | N |N-1|N-2|N-3|     | 0 | -1| -2| -3| -4|
                           |                           |
                           |<---- N + 1.5 CYCLES ----->|<--- N + 2 cycles --->
                                                         +---+
 via2d0t*u* clk ------------------------------------------+   +--------
                                                     |
                                                     |
                                                  call of
			                        int_via2d0*
                                                   here

   real via2d0tau value = via2d0tau* + TAUOFFSET
   via2d0tbu = via2d0tbu* + 0

 *
 * IRQ and PB7 are set/toggled at the low-high transition of Phi2,
 * but int_* is called a half-cycle before that. Does that matter?
 *
 * PB7 output is still to be implemented
 */

/* timer values do not depend on a certain value here, but PB7 does... */
#define	TAUOFFSET	(-1)

inline static void update_via2d0irq(void)
{
    drive0_set_irq(I_VIA2D0FL, (via2d0ifr & via2d0ier & 0x7f) ? IK_IRQ : 0);
}

/* the next two are used in read_via2d0() */

inline static unsigned int via2d0ta(void)
{
    if (drive_clk[0] < via2d0tau - TAUOFFSET)
        return via2d0tau - TAUOFFSET - drive_clk[0] - 2;
    else
	return (via2d0tal - (drive_clk[0] - via2d0tau + TAUOFFSET) % (via2d0tal + 2));
}

inline static unsigned int via2d0tb(void)
{
    return via2d0tbu - drive_clk[0] - 2;
}

inline static void update_via2d0tal(CLOCK rclk)
{
    via2d0pb7x = 0;
    via2d0pb7xx = 0;

    if (rclk > via2d0tau) {
	int nuf = (via2d0tal + 1 + rclk - via2d0tau) / (via2d0tal + 2);

	if (!(via2d0[VIA_ACR] & 0x40)) {
	    if (((nuf - via2d0pb7sx) > 1) || (!via2d0pb7)) {
		via2d0pb7o = 1;
		via2d0pb7sx = 0;
	    }
	}
	via2d0pb7 ^= (nuf & 1);

	via2d0tau = TAUOFFSET + via2d0tal + 2 + (rclk - (rclk - via2d0tau + TAUOFFSET) % (via2d0tal + 2));
	if (rclk == via2d0tau - via2d0tal - 1) {
	    via2d0pb7xx = 1;
	}
    }

    if (via2d0tau == rclk)
	via2d0pb7x = 1;

    via2d0tal = via2d0[VIA_T1LL] + (via2d0[VIA_T1LH] << 8);
}

inline static void update_via2d0tbl(void)
{
    via2d0tbl = via2d0[VIA_T2CL] + (via2d0[VIA_T2CH] << 8);
}


/* ------------------------------------------------------------------------- */
/* VIA2D0 */



/*
 * according to Rockwell, all internal registers are cleared, except
 * for the Timer (1 and 2, counter and latches) and the shift register.
 */
void reset_via2d0(void)
{
    int i;
#ifdef VIA2D0_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("VIA2D0: reset\n");
#endif
    /* clear registers */
    for (i = 0; i < 4; i++)
	via2d0[i] = 0;
    for (i = 4; i < 10; i++)
        via2d0[i] = 0xff;        /* AB 98.08.23 */
    for (i = 11; i < 16; i++)
	via2d0[i] = 0;

    via2d0tal = 0;
    via2d0tbl = 0;
    via2d0tau = drive_clk[0];
    via2d0tbu = drive_clk[0];

    via2d0ier = 0;
    via2d0ifr = 0;

    /* disable vice interrupts */
    via2d0tai = 0;
    via2d0tbi = 0;
    drive0_unset_alarm(A_VIA2D0T1);
    drive0_unset_alarm(A_VIA2D0T2);
    update_via2d0irq();

    oldpa = 0xff;
    oldpb = 0xff;

    

}

void via2d0_signal(int line, int edge)
{
    switch (line) {
      case VIA_SIG_CA1:
        via2d0ifr |= ((edge ^ via2d0[VIA_PCR]) & 0x01) ?
            0 : VIA_IM_CA1;
        update_via2d0irq();
        break;
      case VIA_SIG_CA2:
        if (!(via2d0[VIA_PCR] & 0x08)) {
            via2d0ifr |= (((edge << 2) ^ via2d0[VIA_PCR]) & 0x04) ?
                0 : VIA_IM_CA2;
            update_via2d0irq();
        }
        break;
      case VIA_SIG_CB1:
        via2d0ifr |= (((edge << 4) ^ via2d0[VIA_PCR]) & 0x10) ?
            0 : VIA_IM_CB1;
        update_via2d0irq();
        break;
      case VIA_SIG_CB2:
        if (!(via2d0[VIA_PCR] & 0x80)) {
            via2d0ifr |= (((edge << 6) ^ via2d0[VIA_PCR]) & 0x40) ?
                0 : VIA_IM_CB2;
            update_via2d0irq();
        }
        break;
    }
}

void REGPARM2 store_via2d0(ADDRESS addr, BYTE byte)
{
    CLOCK rclk = drive_clk[0] - 1;	/* stores have a one-cylce offset */

    addr &= 0xf;
#ifdef VIA2D0_TIMER_DEBUG
    if ((addr < 10 && addr > 3) || (addr == VIA_ACR) || app_resources.debugFlag)
	printf("store via2d0[%x] %x, rmwf=%d, clk=%d, rclk=%d\n",
	       (int) addr, (int) byte, drive0_rmw_flag, drive_clk[0], rclk);
#endif

    switch (addr) {

      /* these are done with saving the value */
      case VIA_PRA:		/* port A */
        via2d0ifr &= ~VIA_IM_CA1;
        if ((via2d0[VIA_PCR] & 0x0a) != 0x2) {
            via2d0ifr &= ~VIA_IM_CA2;
        }
        update_via2d0irq();

      case VIA_PRA_NHS:	/* port A, no handshake */
        via2d0[VIA_PRA_NHS] = byte;
        addr = VIA_PRA;
      case VIA_DDRA:
	via2d0[addr] = byte;
	byte = via2d0[VIA_PRA] | ~via2d0[VIA_DDRA];

     drive_write_gcr(byte, 0);
	oldpa = byte;
        break;

      case VIA_PRB:		/* port B */
        via2d0ifr &= ~VIA_IM_CB1;
        if ((via2d0[VIA_PCR] & 0xa0) != 0x20) {
            via2d0ifr &= ~VIA_IM_CB2;
        }
        update_via2d0irq();

      case VIA_DDRB:
	via2d0[addr] = byte;
	byte = via2d0[VIA_PRB] | ~via2d0[VIA_DDRB];

	{
	  drive[0].led_status = byte & 8;

	  if (((oldpb ^ byte) & 0x3) && (byte & 0x4))  /* Stepper motor */
	  {
	    if ((oldpb & 0x3) == ((byte + 1) & 0x3))
              drive_move_head(-1, 0);
            else if ((oldpb & 0x3) == ((byte - 1) & 0x3))
              drive_move_head(+1, 0);
	  }
	  if ((oldpb ^ byte) & 0x60)     /* Zone bits */
	      drive_update_zone_bits((byte >> 5) & 0x3, 0);

	  if ((oldpb ^ byte) & 0x04)     /* Motor on/off */
	      drive_motor_control(byte & 0x04, 0);
	}
	oldpb = byte;
        break;

      case VIA_SR:		/* Serial Port output buffer */
        via2d0[addr] = byte;
        
            break;

        /* Timers */

      case VIA_T1CL:
      case VIA_T1LL:
        via2d0[VIA_T1LL] = byte;
        update_via2d0tal(rclk);
        break;

      case VIA_T1CH /*TIMER_AH */ :	/* Write timer A high */
#ifdef VIA2D0_TIMER_DEBUG
        if (app_resources.debugFlag)
            printf("Write timer A high: %02x\n", byte);
#endif
        via2d0[VIA_T1LH] = byte;
        update_via2d0tal(rclk);
        /* load counter with latch value */
        via2d0tau = rclk + via2d0tal + 3 + TAUOFFSET;
        via2d0tai = rclk + via2d0tal + 2;
        drive0_set_alarm_clk(A_VIA2D0T1, via2d0tai);

        /* set pb7 state */
        via2d0pb7 = 0;
        via2d0pb7o = 0;

        /* Clear T1 interrupt */
        via2d0ifr &= ~VIA_IM_T1;
        update_via2d0irq();
        break;

      case VIA_T1LH:		/* Write timer A high order latch */
        via2d0[addr] = byte;
        update_via2d0tal(rclk);

        /* Clear T1 interrupt */
        via2d0ifr &= ~VIA_IM_T1;
        update_via2d0irq();
        break;

      case VIA_T2LL:		/* Write timer 2 low latch */
        via2d0[VIA_T2LL] = byte;
        update_via2d0tbl();
        
            break;

      case VIA_T2CH:		/* Write timer 2 high */
        via2d0[VIA_T2CH] = byte;
        update_via2d0tbl();
        via2d0tbu = rclk + via2d0tbl + 3;
        via2d0tbi = rclk + via2d0tbl + 2;
        drive0_set_alarm(A_VIA2D0T2, via2d0tbi);

        /* Clear T2 interrupt */
        via2d0ifr &= ~VIA_IM_T2;
        update_via2d0irq();
        break;

        /* Interrupts */

      case VIA_IFR:		/* 6522 Interrupt Flag Register */
        via2d0ifr &= ~byte;
        update_via2d0irq();
        break;

      case VIA_IER:		/* Interrupt Enable Register */
#if defined (VIA2D0_TIMER_DEBUG)
        printf("Via#1 set VIA_IER: 0x%x\n", byte);
#endif
        if (byte & VIA_IM_IRQ) {
            /* set interrupts */
            via2d0ier |= byte & 0x7f;
        } else {
            /* clear interrupts */
            via2d0ier &= ~byte;
        }
        update_via2d0irq();
        break;

        /* Control */

      case VIA_ACR:
        /* bit 7 timer 1 output to PB7 */
        update_via2d0tal(rclk);
        if ((via2d0[VIA_ACR] ^ byte) & 0x80) {
            if (byte & 0x80) {
                via2d0pb7 = 1 ^ via2d0pb7x;
            }
        }
        if ((via2d0[VIA_ACR] ^ byte) & 0x40) {
            via2d0pb7 ^= via2d0pb7sx;
            if ((byte & 0x40)) {
                if (via2d0pb7x || via2d0pb7xx) {
                    if (via2d0tal) {
                        via2d0pb7o = 1;
                    } else {
                        via2d0pb7o = 0;
                        if ((via2d0[VIA_ACR] & 0x80) && via2d0pb7x && (!via2d0pb7xx))
                            via2d0pb7 ^= 1;
                    }
                }
            }
        }
        via2d0pb7sx = via2d0pb7x;
        via2d0[addr] = byte;

        

        /* bit 5 timer 2 count mode */
        if (byte & 32) {
            /* TODO */
            /* update_via2d0tb(0); *//* stop timer if mode == 1 */
        }

        /* bit 4, 3, 2 shift register control */
        /* bit 1, 0  latch enable port B and A */
        break;

      case VIA_PCR:

        /* if(viadebug) printf("VIA1: write %02x to PCR\n",byte); */

        /* bit 7, 6, 5  CB2 handshake/interrupt control */
        /* bit 4  CB1 interrupt control */

        /* bit 3, 2, 1  CA2 handshake/interrupt control */
        /* bit 0  CA1 interrupt control */


        if(byte != via2d0[VIA_PCR]) {
          register BYTE tmp = byte;
          /* first set bit 1 and 5 to the real output values */
          if((tmp & 0x0c) != 0x0c) tmp |= 0x02;
          if((tmp & 0xc0) != 0xc0) tmp |= 0x20;
          /* insert_your_favourite_drive_function_here(tmp);
	     bit 5 is the write output to the analog circuitry:
	     0 = writing, 0x20 = reading */
	     drive_update_viad2_pcr(tmp, 0);
          if ((byte&0x20) != (via2d0[addr]&0x20)) {
             drive_rotate_disk(0, 0);
             drive_rotate_disk(1, 0);
          }
          byte = tmp;
        }
        via2d0[addr] = byte;
        break;

      default:
        via2d0[addr] = byte;

    }				/* switch */
}


/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_via2d0(ADDRESS addr)
{
#ifdef VIA2D0_TIMER_DEBUG
    BYTE read_via2d0_(ADDRESS);
    BYTE retv = read_via2d0_(addr);
    addr &= 0x0f;
    if ((addr > 3 && addr < 10) || app_resources.debugFlag)
	printf("read_via2d0(%x) -> %02x, clk=%d\n", addr, retv, drive_clk[0]);
    return retv;
}
BYTE REGPARM1 read_via2d0_(ADDRESS addr)
{
#endif
    BYTE byte;
    CLOCK rclk = drive_clk[0];

    addr &= 0xf;

    if (via2d0tai && (via2d0tai <= drive_clk[0]))
	int_via2d0t1(drive_clk[0] - via2d0tai);
    if (via2d0tbi && (via2d0tbi <= drive_clk[0]))
	int_via2d0t2(drive_clk[0] - via2d0tbi);

    switch (addr) {

      case VIA_PRA:		/* port A */
        via2d0ifr &= ~VIA_IM_CA1;
        if ((via2d0[VIA_PCR] & 0x0a) != 0x02) {
            via2d0ifr &= ~VIA_IM_CA2;
        }
        update_via2d0irq();

      case VIA_PRA_NHS:	/* port A, no handshake */

    {
        byte = ((drive_read_disk_byte(0) /* & ~via2d0[VIA_DDRA])
            | (via2d0[VIA_PRA] & via2d0[VIA_DDRA] */ ));
        if (drive[0].type == DRIVE_TYPE_1571)
            if (drive_byte_ready(0))
                drive_set_byte_ready(0, 0);
    }
        byte = (byte & ~via2d0[VIA_DDRA]) | (via2d0[VIA_PRA] & via2d0[VIA_DDRA]);
	return byte;

      case VIA_PRB:		/* port B */
        via2d0ifr &= ~VIA_IM_CB1;
        if ((via2d0[VIA_PCR] & 0xa0) != 0x20)
            via2d0ifr &= ~VIA_IM_CB2;
        update_via2d0irq();


	byte = (drive_read_viad2_prb(0) /* & ~via2d0[VIA_DDRB])
			| (via2d0[VIA_PRB] & via2d0[VIA_DDRB] */ );
        byte = (byte & ~via2d0[VIA_DDRB]) | (via2d0[VIA_PRB] & via2d0[VIA_DDRB]);

        if (via2d0[VIA_ACR] & 0x80) {
            update_via2d0tal(rclk);
            byte = (byte & 0x7f) | (((via2d0pb7 ^ via2d0pb7x) | via2d0pb7o) ? 0x80 : 0);
        }
        return byte;

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        via2d0ifr &= ~VIA_IM_T1;
        update_via2d0irq();
        return via2d0ta() & 0xff;

      case VIA_T1CH /*TIMER_AH */ :	/* timer A high */
        return (via2d0ta() >> 8) & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        via2d0ifr &= ~VIA_IM_T2;
        update_via2d0irq();
        return via2d0tb() & 0xff;

      case VIA_T2CH /*TIMER_BH */ :	/* timer B high */
        return (via2d0tb() >> 8) & 0xff;

      case VIA_SR:		/* Serial Port Shift Register */
        return (via2d0[addr]);

        /* Interrupts */

      case VIA_IFR:		/* Interrupt Flag Register */
        {
            BYTE t = via2d0ifr;
            if (via2d0ifr & via2d0ier /*[VIA_IER] */ )
                t |= 0x80;
            return (t);
        }

      case VIA_IER:		/* 6522 Interrupt Control Register */
        return (via2d0ier /*[VIA_IER] */  | 0x80);

    }				/* switch */

    return (via2d0[addr]);
}

BYTE REGPARM1 peek_via2d0(ADDRESS addr)
{
    CLOCK rclk = drive_clk[0];

    addr &= 0xf;

    if (via2d0tai && (via2d0tai <= drive_clk[0]))
	int_via2d0t1(drive_clk[0] - via2d0tai);
    if (via2d0tbi && (via2d0tbi <= drive_clk[0]))
	int_via2d0t2(drive_clk[0] - via2d0tbi);

    switch (addr) {
      case VIA_PRA:
        return read_via2d0(VIA_PRA_NHS);

      case VIA_PRB:		/* port B */
        {
            BYTE byte;


	byte = (drive_read_viad2_prb(0) /* & ~via2d0[VIA_DDRB])
			| (via2d0[VIA_PRB] & via2d0[VIA_DDRB] */ );
            if (via2d0[VIA_ACR] & 0x80) {
                update_via2d0tal(rclk);
                byte = (byte & 0x7f) | (((via2d0pb7 ^ via2d0pb7x) | via2d0pb7o) ? 0x80 : 0);
            }
            return byte;
        }

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        return via2d0ta() & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        return via2d0tb() & 0xff;

      default:
        break;
    }				/* switch */

    return read_via2d0(addr);
}


/* ------------------------------------------------------------------------- */

int int_via2d0t1(long offset)
{
/*    CLOCK rclk = drive_clk[0] - offset; */
#ifdef VIA2D0_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("via2d0 timer A interrupt\n");
#endif

    if (!(via2d0[VIA_ACR] & 0x40)) {	/* one-shot mode */
#if 0				/* defined (VIA2D0_TIMER_DEBUG) */
	printf("VIA2D0 Timer A interrupt -- one-shot mode: next int won't happen\n");
#endif
	drive0_unset_alarm(A_VIA2D0T1);	/*int_clk[I_VIA2D0T1] = 0; */
	via2d0tai = 0;
    } else {			/* continuous mode */
	/* load counter with latch value */
	via2d0tai += via2d0tal + 2;
	drive0_set_alarm_clk(A_VIA2D0T1, via2d0tai);
    }
    via2d0ifr |= VIA_IM_T1;
    update_via2d0irq();

    /* TODO: toggle PB7? */
    return 0;			/*(viaier & VIA_IM_T1) ? 1:0; */
}

/*
 * Timer B is always in one-shot mode
 */

int int_via2d0t2(long offset)
{
#ifdef VIA2D0_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("VIA2D0 timer B interrupt\n");
#endif
    drive0_unset_alarm(A_VIA2D0T2);	/*int_clk[I_VIA2D0T2] = 0; */

    via2d0ifr |= VIA_IM_T2;
    update_via2d0irq();

    return 0;
}

void via2d0_prevent_clk_overflow(CLOCK sub)
{
    unsigned int t;
    t = (via2d0tau - (drive_clk[0] + sub)) & 0xffff;
    via2d0tau = drive_clk[0] + t;
    t = (via2d0tbu - (drive_clk[0] + sub)) & 0xffff;
    via2d0tbu = drive_clk[0] + t;
    if (via2d0tai)
	via2d0tai -= sub;
}

/*------------------------------------------------------------------------*/

/* The dump format has a module header and the data generated by the
 * chip...
 *
 * The version of this dump description is 0/0
 */

#define VIA_DUMP_VER_MAJOR      0
#define VIA_DUMP_VER_MINOR      0

/*
 * The dump data:
 *
 * UBYTE        ORA
 * UBYTE        DDRA
 * UBYTE        ORB
 * UBYTE        DDRB
 * UWORD	T1L
 * UWORD	T1C
 * UBYTE	T2L
 * UWORD	T2C
 * UBYTE	SR
 * UBYTE	ACR
 * UBYTE	PCR
 * UBYTE	IFR		 active interrupts
 * UBYTE	IER		 interrupt masks
 * UBYTE	PB7		 bit 7 = pb7 state
 * UBYTE	SRHBITS		 number of half bits to shift out on SR
 *
 */


int via2d0_write_snapshot_module(FILE * p)
{

    if (via2d0tai && (via2d0tai <= drive_clk[0]))
        int_via2d0t1(drive_clk[0] - via2d0tai);
    if (via2d0tbi && (via2d0tbi <= drive_clk[0]))
        int_via2d0t2(drive_clk[0] - via2d0tbi);

    snapshot_write_module_header(p, "VIA2D0",
                        VIA_DUMP_VER_MAJOR, VIA_DUMP_VER_MINOR);

    snapshot_write_byte(p, via2d0[VIA_PRA]);
    snapshot_write_byte(p, via2d0[VIA_DDRA]);
    snapshot_write_byte(p, via2d0[VIA_PRB]);
    snapshot_write_byte(p, via2d0[VIA_DDRB]);

    snapshot_write_word(p, via2d0tal);
    snapshot_write_word(p, via2d0ta());
    snapshot_write_byte(p, via2d0tbl);
    snapshot_write_word(p, via2d0tb());

    snapshot_write_byte(p, via2d0[VIA_SR]);
    snapshot_write_byte(p, via2d0[VIA_ACR]);
    snapshot_write_byte(p, via2d0[VIA_PCR]);

    snapshot_write_byte(p, via2d0ifr);
    snapshot_write_byte(p, via2d0ier);

						/* FIXME! */
    snapshot_write_byte(p, (((via2d0pb7 ^ via2d0pb7x) | via2d0pb7o) ? 0x80 : 0));
    snapshot_write_byte(p, 0);			/* SRHBITS */

    return 0;
}

int via2d0_read_snapshot_module(FILE * p)
{
    char name[SNAPSHOT_MODULE_NAME_LEN];
    BYTE vmajor, vminor;
    BYTE byte;
    WORD word;
    /* DWORD dword; */
    ADDRESS addr;
    CLOCK rclk = clk;

    snapshot_read_module_header(p, name, &vmajor, &vminor);

    if(strcmp(name, "VIA2D0") || vmajor != VIA_DUMP_VER_MAJOR) return -1;

    snapshot_read_byte(p, &via2d0[VIA_PRA]);
    snapshot_read_byte(p, &via2d0[VIA_DDRA]);
    snapshot_read_byte(p, &via2d0[VIA_PRB]);
    snapshot_read_byte(p, &via2d0[VIA_DDRB]);
    {
        addr = VIA_DDRA;
	byte = via2d0[VIA_PRA] | ~via2d0[VIA_DDRA];
	oldpa = byte ^ 0xff;

     drive_write_gcr(byte, 0);
	oldpa = byte;

	addr = VIA_DDRB;
	byte = via2d0[VIA_PRB] | ~via2d0[VIA_DDRB];
	oldpb = byte ^ 0xff;
	/* FIXME!!!! */
	oldpb = byte;
    }

    snapshot_read_word(p, &word);
    via2d0tal = word;
    snapshot_read_word(p, &word);
    via2d0tau = rclk + word + 1 /* 3 */ + TAUOFFSET;
    via2d0tai = rclk + word /* + 2 */;
    drive0_set_alarm_clk(A_VIA2D0T1, via2d0tai);

    snapshot_read_byte(p, &byte);
    via2d0tbl = byte;
    snapshot_read_word(p, &word);
    via2d0tbu = rclk + word + 1 /* 3 */;
    via2d0tbi = rclk + word /* + 2 */;
    drive0_set_alarm(A_VIA2D0T2, via2d0tbi);

    snapshot_read_byte(p, &via2d0[VIA_SR]);
    {
	addr = via2d0[VIA_SR];
	byte = via2d0[addr];
	
    }
    snapshot_read_byte(p, &via2d0[VIA_ACR]);
    snapshot_read_byte(p, &via2d0[VIA_PCR]);
    {
	addr = via2d0[VIA_PCR];
	byte = via2d0[addr];

        if(byte != via2d0[VIA_PCR]) {
          register BYTE tmp = byte;
          /* first set bit 1 and 5 to the real output values */
          if((tmp & 0x0c) != 0x0c) tmp |= 0x02;
          if((tmp & 0xc0) != 0xc0) tmp |= 0x20;
          /* insert_your_favourite_drive_function_here(tmp);
	     bit 5 is the write output to the analog circuitry:
	     0 = writing, 0x20 = reading */
	     drive_update_viad2_pcr(tmp, 0);
          if ((byte&0x20) != (via2d0[addr]&0x20)) {
             drive_rotate_disk(0, 0);
             drive_rotate_disk(1, 0);
          }
          byte = tmp;
        }
    }

    snapshot_read_byte(p, &byte);
    via2d0ifr = byte;
    snapshot_read_byte(p, &byte);
    via2d0ier = byte;
    update_via2d0irq();
						/* FIXME! */
    snapshot_read_byte(p, &byte);
    via2d0pb7 = byte ? 1 : 0;
    via2d0pb7x = 0;
    via2d0pb7o = 0;
    snapshot_read_byte(p, &byte);		/* SRHBITS */

    return 0;
}


