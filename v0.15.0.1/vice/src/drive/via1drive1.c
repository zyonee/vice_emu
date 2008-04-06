
/*
 * ../../src/drive/via1drive1.c
 * This file is generated from ../../src/via-tmpl.c and ../../src/drive/via1drive1.def,
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
 * into one interrupt flag, I_VIA1D1FL.
 * if an interrupt condition changes, the function (i.e. cpp macro)
 * update_via1d1irq() id called, that checks the IRQ line state.
 * This is now possible, as ettore has decoupled A_* alarm events
 * from I_* interrupts for performance reasons.
 *
 * A new function for signaling rising/falling edges on the
 * control lines is introduced:
 *      via1d1_signal(VIA_SIG_[CA1|CA2|CB1|CB2], VIA_SIG_[RISE|FALL])
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


    #include "drive.h"
    #include "drivecpu.h"
    #include "iecdrive.h"
    #include "viad.h"

#include "interrupt.h"

/*#define VIA1D1_TIMER_DEBUG */
				/*#define VIA1D1_NEED_PB7 *//* when PB7 is really used, set this
				   to enable pulse output from the timer.
				   Otherwise PB7 state is computed only
				   when port B is read -
				   not yet implemented */

/* global */

BYTE via1d1[16];



/*
 * Local variables
 */

static int via1d1ifr;		/* Interrupt Flag register for via1d1 */
static int via1d1ier;		/* Interrupt Enable register for via1d1 */

static unsigned int via1d1tal;	/* current timer A latch value */
static unsigned int via1d1tbl;	/* current timer B latch value */

static CLOCK via1d1tau;		/* time when via1d1 timer A is updated */
static CLOCK via1d1tbu;		/* time when via1d1 timer B is updated */
static CLOCK via1d1tai;		/* time when next timer A alarm is */
static CLOCK via1d1tbi;		/* time when next timer A alarm is */

static int via1d1pb7;		/* state of PB7 for pulse output... */
static int via1d1pb7x;		/* to be xored herewith  */
static int via1d1pb7o;		/* to be ored herewith  */
static int via1d1pb7xx;
static int via1d1pb7sx;

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
 via1d1t*u* clk ------------------------------------------+   +--------
                                                     |
                                                     |
                                                  call of
			                        int_via1d1*
                                                   here

   real via1d1tau value = via1d1tau* + TAUOFFSET
   via1d1tbu = via1d1tbu* + 0

 *
 * IRQ and PB7 are set/toggled at the low-high transition of Phi2,
 * but int_* is called a half-cycle before that. Does that matter?
 *
 * PB7 output is still to be implemented
 */

/* timer values do not depend on a certain value here, but PB7 does... */
#define	TAUOFFSET	(-1)

inline static void update_via1d1irq(void)
{
    drive1_set_irq(I_VIA1D1FL, (via1d1ifr & via1d1ier & 0x7f) ? IK_IRQ : 0);
}

/* the next two are used in read_via1d1() */

inline static unsigned int via1d1ta(void)
{
    if (drive_clk[1] < via1d1tau - TAUOFFSET)
        return via1d1tau - TAUOFFSET - drive_clk[1] - 2;
    else
	return (via1d1tal - (drive_clk[1] - via1d1tau + TAUOFFSET) % (via1d1tal + 2));
}

inline static unsigned int via1d1tb(void)
{
    return via1d1tbu - drive_clk[1] - 2;
}

inline static void update_via1d1tal(CLOCK rclk)
{
    via1d1pb7x = 0;
    via1d1pb7xx = 0;

    if (rclk > via1d1tau) {
	int nuf = (via1d1tal + 1 + rclk - via1d1tau) / (via1d1tal + 2);

	if (!(via1d1[VIA_ACR] & 0x40)) {
	    if (((nuf - via1d1pb7sx) > 1) || (!via1d1pb7)) {
		via1d1pb7o = 1;
		via1d1pb7sx = 0;
	    }
	}
	via1d1pb7 ^= (nuf & 1);

	via1d1tau = TAUOFFSET + via1d1tal + 2 + (rclk - (rclk - via1d1tau + TAUOFFSET) % (via1d1tal + 2));
	if (rclk == via1d1tau - via1d1tal - 1) {
	    via1d1pb7xx = 1;
	}
    }

    if (via1d1tau == rclk)
	via1d1pb7x = 1;

    via1d1tal = via1d1[VIA_T1LL] + (via1d1[VIA_T1LH] << 8);
}

inline static void update_via1d1tbl(void)
{
    via1d1tbl = via1d1[VIA_T2CL] + (via1d1[VIA_T2CH] << 8);
}


/* ------------------------------------------------------------------------- */
/* VIA1D1 */



void via1d1_set_atn(BYTE state)
{
#ifdef OLDIRQ
   if (state) {
      via1d1ifr |= 2;
   } else {
      via1d1ifr &= 0xfe;
   }
#else
   via1d1_signal(VIA_SIG_CA1, state ? VIA_SIG_RISE : 0);
#endif
}

static iec_info_t *iec_info;


/*
 * according to Rockwell, all internal registers are cleared, except
 * for the Timer (1 and 2, counter and latches) and the shift register.
 */
void reset_via1d1(void)
{
    int i;
#ifdef VIA1D1_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("VIA1D1: reset\n");
#endif
    /* clear registers */
    for (i = 0; i < 4; i++)
	via1d1[i] = 0;
    for (i = 4; i < 10; i++)
        via1d1[i] = 0xff;        /* AB 98.08.23 */
    for (i = 11; i < 16; i++)
	via1d1[i] = 0;

    via1d1tal = 0;
    via1d1tbl = 0;
    via1d1tau = drive_clk[1];
    via1d1tbu = drive_clk[1];

    via1d1ier = 0;
    via1d1ifr = 0;

    /* disable vice interrupts */
    via1d1tai = 0;
    via1d1tbi = 0;
    drive1_unset_alarm(A_VIA1D1T1);
    drive1_unset_alarm(A_VIA1D1T2);
    update_via1d1irq();


    iec_info = iec_get_drive_port();

}

void via1d1_signal(int line, int edge)
{
    switch (line) {
      case VIA_SIG_CA1:
        via1d1ifr |= ((edge ^ via1d1[VIA_PCR]) & 0x01) ?
            0 : VIA_IM_CA1;
        update_via1d1irq();
        break;
      case VIA_SIG_CA2:
        if (!(via1d1[VIA_PCR] & 0x08)) {
            via1d1ifr |= (((edge << 2) ^ via1d1[VIA_PCR]) & 0x04) ?
                0 : VIA_IM_CA2;
            update_via1d1irq();
        }
        break;
      case VIA_SIG_CB1:
        via1d1ifr |= (((edge << 4) ^ via1d1[VIA_PCR]) & 0x10) ?
            0 : VIA_IM_CB1;
        update_via1d1irq();
        break;
      case VIA_SIG_CB2:
        if (!(via1d1[VIA_PCR] & 0x80)) {
            via1d1ifr |= (((edge << 6) ^ via1d1[VIA_PCR]) & 0x40) ?
                0 : VIA_IM_CB2;
            update_via1d1irq();
        }
        break;
    }
}

void REGPARM2 store_via1d1(ADDRESS addr, BYTE byte)
{
    CLOCK rclk = drive_clk[1] - 1;	/* stores have a one-cylce offset */

    addr &= 0xf;
#ifdef VIA1D1_TIMER_DEBUG
    if ((addr < 10 && addr > 3) || (addr == VIA_ACR) || app_resources.debugFlag)
	printf("store via1d1[%x] %x, rmwf=%d, clk=%d, rclk=%d\n",
	       (int) addr, (int) byte, drive1_rmw_flag, drive_clk[1], rclk);
#endif

    switch (addr) {

      /* these are done with saving the value */
      case VIA_PRA:		/* port A */
        via1d1ifr &= ~VIA_IM_CA1;
        if ((via1d1[VIA_PCR] & 0x0a) != 0x2) {
            via1d1ifr &= ~VIA_IM_CA2;
        }
        update_via1d1irq();

      case VIA_PRA_NHS:	/* port A, no handshake */
        via1d1[VIA_PRA_NHS] = byte;
        addr = VIA_PRA;
      case VIA_DDRA:

    {
        BYTE oldval = via1d1[VIA_PRA] | ~via1d1[VIA_DDRA];

        via1d1[addr] = byte;
        byte = via1d1[VIA_PRA] | ~via1d1[VIA_DDRA];

        printf("PREWERT adr: %x oldval: %x byte %x\n",addr,oldval,byte);
        if (oldval != byte) printf("Wert: %x\n",byte&0x99);

        if (drive[1].type == DRIVE_TYPE_1571) {
            if ((oldval ^ byte) & 0x20)
                drive_set_1571_sync_factor(byte & 0x20, 1);
        }
        if (drive_parallel_cable_enabled && drive[1].type == DRIVE_TYPE_1541)
            parallel_cable_drive_write(via1d1[VIA_PRA] | ~via1d1[VIA_DDRA],
                                        (((addr == VIA_PRA)
                                        && ((via1d1[VIA_PCR] & 0xe) == 0xa))
                                        ? 1 : 0));
    }
            break;

      case VIA_PRB:		/* port B */
        via1d1ifr &= ~VIA_IM_CB1;
        if ((via1d1[VIA_PCR] & 0xa0) != 0x20) {
            via1d1ifr &= ~VIA_IM_CB2;
        }
        update_via1d1irq();

      case VIA_DDRB:

    if (byte != via1d1[addr]) {
	via1d1[addr] = byte;
	if (iec_info != NULL) {
	    iec_info->drive2_data = via1d1[VIA_DDRB] & ~via1d1[VIA_PRB];
	    iec_info->drive2_bus = (((iec_info->drive2_data << 3) & 0x40)
	        | ((iec_info->drive2_data << 6)
	        & ((~iec_info->drive2_data ^ iec_info->cpu_bus) << 3) & 0x80));
	    iec_info->cpu_port = iec_info->cpu_bus & iec_info->drive2_bus
	        & iec_info->drive_bus;
	    iec_info->drive2_port = iec_info->drive_port = (((iec_info->cpu_port >> 4) & 0x4)
	        | (iec_info->cpu_port >> 7)
	        | ((iec_info->cpu_bus << 3) & 0x80));
	} else {
	    iec_drive_write(via1d1[VIA_DDRB] & ~via1d1[VIA_PRB]);
	}
    }
            break;

      case VIA_SR:		/* Serial Port output buffer */
        via1d1[addr] = byte;
        
            break;

        /* Timers */

      case VIA_T1CL:
      case VIA_T1LL:
        via1d1[VIA_T1LL] = byte;
        update_via1d1tal(rclk);
        break;

      case VIA_T1CH /*TIMER_AH */ :	/* Write timer A high */
#ifdef VIA1D1_TIMER_DEBUG
        if (app_resources.debugFlag)
            printf("Write timer A high: %02x\n", byte);
#endif
        via1d1[VIA_T1LH] = byte;
        update_via1d1tal(rclk);
        /* load counter with latch value */
        via1d1tau = rclk + via1d1tal + 3 + TAUOFFSET;
        via1d1tai = rclk + via1d1tal + 2;
        drive1_set_alarm_clk(A_VIA1D1T1, via1d1tai);

        /* set pb7 state */
        via1d1pb7 = 0;
        via1d1pb7o = 0;

        /* Clear T1 interrupt */
        via1d1ifr &= ~VIA_IM_T1;
        update_via1d1irq();
        break;

      case VIA_T1LH:		/* Write timer A high order latch */
        via1d1[addr] = byte;
        update_via1d1tal(rclk);

        /* Clear T1 interrupt */
        via1d1ifr &= ~VIA_IM_T1;
        update_via1d1irq();
        break;

      case VIA_T2LL:		/* Write timer 2 low latch */
        via1d1[VIA_T2LL] = byte;
        update_via1d1tbl();
        
            break;

      case VIA_T2CH:		/* Write timer 2 high */
        via1d1[VIA_T2CH] = byte;
        update_via1d1tbl();
        via1d1tbu = rclk + via1d1tbl + 3;
        via1d1tbi = rclk + via1d1tbl + 2;
        drive1_set_alarm(A_VIA1D1T2, via1d1tbi);

        /* Clear T2 interrupt */
        via1d1ifr &= ~VIA_IM_T2;
        update_via1d1irq();
        break;

        /* Interrupts */

      case VIA_IFR:		/* 6522 Interrupt Flag Register */
        via1d1ifr &= ~byte;
        update_via1d1irq();
        break;

      case VIA_IER:		/* Interrupt Enable Register */
#if defined (VIA1D1_TIMER_DEBUG)
        printf("Via#1 set VIA_IER: 0x%x\n", byte);
#endif
        if (byte & VIA_IM_IRQ) {
            /* set interrupts */
            via1d1ier |= byte & 0x7f;
        } else {
            /* clear interrupts */
            via1d1ier &= ~byte;
        }
        update_via1d1irq();
        break;

        /* Control */

      case VIA_ACR:
        /* bit 7 timer 1 output to PB7 */
        update_via1d1tal(rclk);
        if ((via1d1[VIA_ACR] ^ byte) & 0x80) {
            if (byte & 0x80) {
                via1d1pb7 = 1 ^ via1d1pb7x;
            }
        }
        if ((via1d1[VIA_ACR] ^ byte) & 0x40) {
            via1d1pb7 ^= via1d1pb7sx;
            if ((byte & 0x40)) {
                if (via1d1pb7x || via1d1pb7xx) {
                    if (via1d1tal) {
                        via1d1pb7o = 1;
                    } else {
                        via1d1pb7o = 0;
                        if ((via1d1[VIA_ACR] & 0x80) && via1d1pb7x && (!via1d1pb7xx))
                            via1d1pb7 ^= 1;
                    }
                }
            }
        }
        via1d1pb7sx = via1d1pb7x;
        via1d1[addr] = byte;

        

        /* bit 5 timer 2 count mode */
        if (byte & 32) {
            /* TODO */
            /* update_via1d1tb(0); *//* stop timer if mode == 1 */
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

        
        via1d1[addr] = byte;
        break;

      default:
        via1d1[addr] = byte;

    }				/* switch */
}


/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_via1d1(ADDRESS addr)
{
#ifdef VIA1D1_TIMER_DEBUG
    BYTE read_via1d1_(ADDRESS);
    BYTE retv = read_via1d1_(addr);
    addr &= 0x0f;
    if ((addr > 3 && addr < 10) || app_resources.debugFlag)
	printf("read_via1d1(%x) -> %02x, clk=%d\n", addr, retv, drive_clk[1]);
    return retv;
}
BYTE REGPARM1 read_via1d1_(ADDRESS addr)
{
#endif
    CLOCK rclk = drive_clk[1];

    addr &= 0xf;

    if (via1d1tai && (via1d1tai <= drive_clk[1]))
	int_via1d1t1(drive_clk[1] - via1d1tai);
    if (via1d1tbi && (via1d1tbi <= drive_clk[1]))
	int_via1d1t2(drive_clk[1] - via1d1tbi);

    switch (addr) {

      case VIA_PRA:		/* port A */
        via1d1ifr &= ~VIA_IM_CA1;
        if ((via1d1[VIA_PCR] & 0x0a) != 0x02) {
            via1d1ifr &= ~VIA_IM_CA2;
        }
        update_via1d1irq();

      case VIA_PRA_NHS:	/* port A, no handshake */

    if (drive[1].type == DRIVE_TYPE_1571)
        return (((drive_byte_ready(1) ? 0 : 0x80) & ~via1d1[VIA_DDRA])
          | (via1d1[VIA_PRA] & via1d1[VIA_DDRA]));

    return (drive_parallel_cable_enabled
            ? parallel_cable_drive_read((((addr == VIA_PRA) &&
                                          (via1d1[VIA_PCR] & 0xe) == 0xa))
                                        ? 1 : 0)
            : ((via1d1[VIA_PRA] & via1d1[VIA_DDRA])
               | (0xff & ~via1d1[VIA_DDRA])));


      case VIA_PRB:		/* port B */
        via1d1ifr &= ~VIA_IM_CB1;

        if ((via1d1[VIA_PCR] & 0xa0) != 0x20)
            via1d1ifr &= ~VIA_IM_CB2;

        update_via1d1irq();
        {
            BYTE byte;


    if (iec_info != NULL)
	byte = (((via1d1[VIA_PRB] & 0x1a) | iec_info->drive2_port) ^ 0x85) | 0x20;
    else
	byte = (((via1d1[VIA_PRB] & 0x1a) | iec_drive_read()) ^ 0x85) | 0x20;

            if (via1d1[VIA_ACR] & 0x80) {
                update_via1d1tal(rclk);
                byte = (byte & 0x7f) | (((via1d1pb7 ^ via1d1pb7x) | via1d1pb7o) ? 0x80 : 0);
            }
            return byte;
        }

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        via1d1ifr &= ~VIA_IM_T1;
        update_via1d1irq();
        return via1d1ta() & 0xff;

      case VIA_T1CH /*TIMER_AH */ :	/* timer A high */
        return (via1d1ta() >> 8) & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        via1d1ifr &= ~VIA_IM_T2;
        update_via1d1irq();
        return via1d1tb() & 0xff;

      case VIA_T2CH /*TIMER_BH */ :	/* timer B high */
        return (via1d1tb() >> 8) & 0xff;

      case VIA_SR:		/* Serial Port Shift Register */
        return (via1d1[addr]);

        /* Interrupts */

      case VIA_IFR:		/* Interrupt Flag Register */
        {
            BYTE t = via1d1ifr;
            if (via1d1ifr & via1d1ier /*[VIA_IER] */ )
                t |= 0x80;
            return (t);
        }

      case VIA_IER:		/* 6522 Interrupt Control Register */
        return (via1d1ier /*[VIA_IER] */  | 0x80);

    }				/* switch */

    return (via1d1[addr]);
}

BYTE REGPARM1 peek_via1d1(ADDRESS addr)
{
    CLOCK rclk = drive_clk[1];

    addr &= 0xf;

    if (via1d1tai && (via1d1tai <= drive_clk[1]))
	int_via1d1t1(drive_clk[1] - via1d1tai);
    if (via1d1tbi && (via1d1tbi <= drive_clk[1]))
	int_via1d1t2(drive_clk[1] - via1d1tbi);

    switch (addr) {
      case VIA_PRA:
        return read_via1d1(VIA_PRA_NHS);

      case VIA_PRB:		/* port B */
        {
            BYTE byte;


    if (iec_info != NULL)
	byte = (((via1d1[VIA_PRB] & 0x1a) | iec_info->drive2_port) ^ 0x85) | 0x20;
    else
	byte = (((via1d1[VIA_PRB] & 0x1a) | iec_drive_read()) ^ 0x85) | 0x20;
            if (via1d1[VIA_ACR] & 0x80) {
                update_via1d1tal(rclk);
                byte = (byte & 0x7f) | (((via1d1pb7 ^ via1d1pb7x) | via1d1pb7o) ? 0x80 : 0);
            }
            return byte;
        }

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        return via1d1ta() & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        return via1d1tb() & 0xff;

      default:
        break;
    }				/* switch */

    return read_via1d1(addr);
}


/* ------------------------------------------------------------------------- */

int int_via1d1t1(long offset)
{
/*    CLOCK rclk = drive_clk[1] - offset; */
#ifdef VIA1D1_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("via1d1 timer A interrupt\n");
#endif

    if (!(via1d1[VIA_ACR] & 0x40)) {	/* one-shot mode */
#if 0				/* defined (VIA1D1_TIMER_DEBUG) */
	printf("VIA1D1 Timer A interrupt -- one-shot mode: next int won't happen\n");
#endif
	drive1_unset_alarm(A_VIA1D1T1);	/*int_clk[I_VIA1D1T1] = 0; */
	via1d1tai = 0;
    } else {			/* continuous mode */
	/* load counter with latch value */
	via1d1tai += via1d1tal + 2;
	drive1_set_alarm_clk(A_VIA1D1T1, via1d1tai);
    }
    via1d1ifr |= VIA_IM_T1;
    update_via1d1irq();

    /* TODO: toggle PB7? */
    return 0;			/*(viaier & VIA_IM_T1) ? 1:0; */
}

/*
 * Timer B is always in one-shot mode
 */

int int_via1d1t2(long offset)
{
#ifdef VIA1D1_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("VIA1D1 timer B interrupt\n");
#endif
    drive1_unset_alarm(A_VIA1D1T2);	/*int_clk[I_VIA1D1T2] = 0; */

    via1d1ifr |= VIA_IM_T2;
    update_via1d1irq();

    return 0;
}

void via1d1_prevent_clk_overflow(CLOCK sub)
{
    unsigned int t;
    t = (via1d1tau - (drive_clk[1] + sub)) & 0xffff;
    via1d1tau = drive_clk[1] + t;
    t = (via1d1tbu - (drive_clk[1] + sub)) & 0xffff;
    via1d1tbu = drive_clk[1] + t;
    if (via1d1tai)
	via1d1tai -= sub;
}


