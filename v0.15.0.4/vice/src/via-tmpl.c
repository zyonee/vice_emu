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
 * into one interrupt flag, I_MYVIAFL.
 * if an interrupt condition changes, the function (i.e. cpp macro)
 * update_myviairq() id called, that checks the IRQ line state.
 * This is now possible, as ettore has decoupled A_* alarm events
 * from I_* interrupts for performance reasons.
 *
 * A new function for signaling rising/falling edges on the
 * control lines is introduced:
 *      myvia_signal(VIA_SIG_[CA1|CA2|CB1|CB2], VIA_SIG_[RISE|FALL])
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

INCLUDES

#include "interrupt.h"

/*#define MYVIA_TIMER_DEBUG */
				/*#define MYVIA_NEED_PB7 *//* when PB7 is really used, set this
				   to enable pulse output from the timer.
				   Otherwise PB7 state is computed only
				   when port B is read -
				   not yet implemented */

/* global */

BYTE myvia[16];

GLOBALS

/*
 * Local variables
 */

static int myviaifr;		/* Interrupt Flag register for myvia */
static int myviaier;		/* Interrupt Enable register for myvia */

static unsigned int myviatal;	/* current timer A latch value */
static unsigned int myviatbl;	/* current timer B latch value */

static CLOCK myviatau;		/* time when myvia timer A is updated */
static CLOCK myviatbu;		/* time when myvia timer B is updated */
static CLOCK myviatai;		/* time when next timer A alarm is */
static CLOCK myviatbi;		/* time when next timer A alarm is */

static int myviapb7;		/* state of PB7 for pulse output... */
static int myviapb7x;		/* to be xored herewith  */
static int myviapb7o;		/* to be ored herewith  */
static int myviapb7xx;
static int myviapb7sx;

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
 myviat*u* clk ------------------------------------------+   +--------
                                                     |
                                                     |
                                                  call of
			                        int_myvia*
                                                   here

   real myviatau value = myviatau* + TAUOFFSET
   myviatbu = myviatbu* + 0

 *
 * IRQ and PB7 are set/toggled at the low-high transition of Phi2,
 * but int_* is called a half-cycle before that. Does that matter?
 *
 * PB7 output is still to be implemented
 */

/* timer values do not depend on a certain value here, but PB7 does... */
#define	TAUOFFSET	(-1)

inline static void update_myviairq(void)
{
    via_set_int(I_MYVIAFL, (myviaifr & myviaier & 0x7f) ? MYVIA_INT : 0);
}

/* the next two are used in read_myvia() */

inline static unsigned int myviata(void)
{
    if (myclk < myviatau - TAUOFFSET)
        return myviatau - TAUOFFSET - myclk - 2;
    else
	return (myviatal - (myclk - myviatau + TAUOFFSET) % (myviatal + 2));
}

inline static unsigned int myviatb(void)
{
    return myviatbu - myclk - 2;
}

inline static void update_myviatal(CLOCK rclk)
{
    myviapb7x = 0;
    myviapb7xx = 0;

    if (rclk > myviatau) {
	int nuf = (myviatal + 1 + rclk - myviatau) / (myviatal + 2);

	if (!(myvia[VIA_ACR] & 0x40)) {
	    if (((nuf - myviapb7sx) > 1) || (!myviapb7)) {
		myviapb7o = 1;
		myviapb7sx = 0;
	    }
	}
	myviapb7 ^= (nuf & 1);

	myviatau = TAUOFFSET + myviatal + 2 + (rclk - (rclk - myviatau + TAUOFFSET) % (myviatal + 2));
	if (rclk == myviatau - myviatal - 1) {
	    myviapb7xx = 1;
	}
    }

    if (myviatau == rclk)
	myviapb7x = 1;

    myviatal = myvia[VIA_T1LL] + (myvia[VIA_T1LH] << 8);
}

inline static void update_myviatbl(void)
{
    myviatbl = myvia[VIA_T2CL] + (myvia[VIA_T2CH] << 8);
}


/* ------------------------------------------------------------------------- */
/* MYVIA */

PRE_VIA_FUNCS

/*
 * according to Rockwell, all internal registers are cleared, except
 * for the Timer (1 and 2, counter and latches) and the shift register.
 */
void reset_myvia(void)
{
    int i;
#ifdef MYVIA_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("MYVIA: reset\n");
#endif
    /* clear registers */
    for (i = 0; i < 4; i++)
	myvia[i] = 0;
    for (i = 4; i < 10; i++)
        myvia[i] = 0xff;        /* AB 98.08.23 */
    for (i = 11; i < 16; i++)
	myvia[i] = 0;

    myviatal = 0;
    myviatbl = 0;
    myviatau = myclk;
    myviatbu = myclk;

    myviaier = 0;
    myviaifr = 0;

    /* disable vice interrupts */
    myviatai = 0;
    myviatbi = 0;
    mycpu_unset_alarm(A_MYVIAT1);
    mycpu_unset_alarm(A_MYVIAT2);
    update_myviairq();

    oldpa = 0xff;
    oldpb = 0xff;

    RESET_VIA

}

void myvia_signal(int line, int edge)
{
    switch (line) {
      case VIA_SIG_CA1:
        myviaifr |= ((edge ^ myvia[VIA_PCR]) & 0x01) ?
            0 : VIA_IM_CA1;
        update_myviairq();
        break;
      case VIA_SIG_CA2:
        if (!(myvia[VIA_PCR] & 0x08)) {
            myviaifr |= (((edge << 2) ^ myvia[VIA_PCR]) & 0x04) ?
                0 : VIA_IM_CA2;
            update_myviairq();
        }
        break;
      case VIA_SIG_CB1:
        myviaifr |= (((edge << 4) ^ myvia[VIA_PCR]) & 0x10) ?
            0 : VIA_IM_CB1;
        update_myviairq();
        break;
      case VIA_SIG_CB2:
        if (!(myvia[VIA_PCR] & 0x80)) {
            myviaifr |= (((edge << 6) ^ myvia[VIA_PCR]) & 0x40) ?
                0 : VIA_IM_CB2;
            update_myviairq();
        }
        break;
    }
}

void REGPARM2 store_myvia(ADDRESS addr, BYTE byte)
{
    CLOCK rclk = myclk - 1;	/* stores have a one-cylce offset */

    addr &= 0xf;
#ifdef MYVIA_TIMER_DEBUG
    if ((addr < 10 && addr > 3) || (addr == VIA_ACR) || app_resources.debugFlag)
	printf("store myvia[%x] %x, rmwf=%d, clk=%d, rclk=%d\n",
	       (int) addr, (int) byte, myrmwf, myclk, rclk);
#endif

    switch (addr) {

      /* these are done with saving the value */
      case VIA_PRA:		/* port A */
        myviaifr &= ~VIA_IM_CA1;
        if ((myvia[VIA_PCR] & 0x0a) != 0x2) {
            myviaifr &= ~VIA_IM_CA2;
        }
        update_myviairq();

      case VIA_PRA_NHS:	/* port A, no handshake */
        myvia[VIA_PRA_NHS] = byte;
        addr = VIA_PRA;
      case VIA_DDRA:
	myvia[addr] = byte;
	byte = myvia[VIA_PRA] | ~myvia[VIA_DDRA];
        STORE_PRA
	oldpa = byte;
        break;

      case VIA_PRB:		/* port B */
        myviaifr &= ~VIA_IM_CB1;
        if ((myvia[VIA_PCR] & 0xa0) != 0x20) {
            myviaifr &= ~VIA_IM_CB2;
        }
        update_myviairq();

      case VIA_DDRB:
	myvia[addr] = byte;
	byte = myvia[VIA_PRB] | ~myvia[VIA_DDRB];
        STORE_PRB
	oldpb = byte;
        break;

      case VIA_SR:		/* Serial Port output buffer */
        myvia[addr] = byte;
        STORE_SR
            break;

        /* Timers */

      case VIA_T1CL:
      case VIA_T1LL:
        myvia[VIA_T1LL] = byte;
        update_myviatal(rclk);
        break;

      case VIA_T1CH /*TIMER_AH */ :	/* Write timer A high */
#ifdef MYVIA_TIMER_DEBUG
        if (app_resources.debugFlag)
            printf("Write timer A high: %02x\n", byte);
#endif
        myvia[VIA_T1LH] = byte;
        update_myviatal(rclk);
        /* load counter with latch value */
        myviatau = rclk + myviatal + 3 + TAUOFFSET;
        myviatai = rclk + myviatal + 2;
        mycpu_set_alarm_clk(A_MYVIAT1, myviatai);

        /* set pb7 state */
        myviapb7 = 0;
        myviapb7o = 0;

        /* Clear T1 interrupt */
        myviaifr &= ~VIA_IM_T1;
        update_myviairq();
        break;

      case VIA_T1LH:		/* Write timer A high order latch */
        myvia[addr] = byte;
        update_myviatal(rclk);

        /* Clear T1 interrupt */
        myviaifr &= ~VIA_IM_T1;
        update_myviairq();
        break;

      case VIA_T2LL:		/* Write timer 2 low latch */
        myvia[VIA_T2LL] = byte;
        update_myviatbl();
        STORE_T2L
            break;

      case VIA_T2CH:		/* Write timer 2 high */
        myvia[VIA_T2CH] = byte;
        update_myviatbl();
        myviatbu = rclk + myviatbl + 3;
        myviatbi = rclk + myviatbl + 2;
        mycpu_set_alarm(A_MYVIAT2, myviatbi);

        /* Clear T2 interrupt */
        myviaifr &= ~VIA_IM_T2;
        update_myviairq();
        break;

        /* Interrupts */

      case VIA_IFR:		/* 6522 Interrupt Flag Register */
        myviaifr &= ~byte;
        update_myviairq();
        break;

      case VIA_IER:		/* Interrupt Enable Register */
#if defined (MYVIA_TIMER_DEBUG)
        printf("Via#1 set VIA_IER: 0x%x\n", byte);
#endif
        if (byte & VIA_IM_IRQ) {
            /* set interrupts */
            myviaier |= byte & 0x7f;
        } else {
            /* clear interrupts */
            myviaier &= ~byte;
        }
        update_myviairq();
        break;

        /* Control */

      case VIA_ACR:
        /* bit 7 timer 1 output to PB7 */
        update_myviatal(rclk);
        if ((myvia[VIA_ACR] ^ byte) & 0x80) {
            if (byte & 0x80) {
                myviapb7 = 1 ^ myviapb7x;
            }
        }
        if ((myvia[VIA_ACR] ^ byte) & 0x40) {
            myviapb7 ^= myviapb7sx;
            if ((byte & 0x40)) {
                if (myviapb7x || myviapb7xx) {
                    if (myviatal) {
                        myviapb7o = 1;
                    } else {
                        myviapb7o = 0;
                        if ((myvia[VIA_ACR] & 0x80) && myviapb7x && (!myviapb7xx))
                            myviapb7 ^= 1;
                    }
                }
            }
        }
        myviapb7sx = myviapb7x;
        myvia[addr] = byte;

        STORE_ACR

        /* bit 5 timer 2 count mode */
        if (byte & 32) {
            /* TODO */
            /* update_myviatb(0); *//* stop timer if mode == 1 */
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

        STORE_PCR
        myvia[addr] = byte;
        break;

      default:
        myvia[addr] = byte;

    }				/* switch */
}


/* ------------------------------------------------------------------------- */

BYTE REGPARM1 read_myvia(ADDRESS addr)
{
#ifdef MYVIA_TIMER_DEBUG
    BYTE read_myvia_(ADDRESS);
    BYTE retv = read_myvia_(addr);
    addr &= 0x0f;
    if ((addr > 3 && addr < 10) || app_resources.debugFlag)
	printf("read_myvia(%x) -> %02x, clk=%d\n", addr, retv, myclk);
    return retv;
}
BYTE REGPARM1 read_myvia_(ADDRESS addr)
{
#endif
    BYTE byte;
    CLOCK rclk = myclk;

    addr &= 0xf;

    if (myviatai && (myviatai <= myclk))
	int_myviat1(myclk - myviatai);
    if (myviatbi && (myviatbi <= myclk))
	int_myviat2(myclk - myviatbi);

    switch (addr) {

      case VIA_PRA:		/* port A */
        myviaifr &= ~VIA_IM_CA1;
        if ((myvia[VIA_PCR] & 0x0a) != 0x02) {
            myviaifr &= ~VIA_IM_CA2;
        }
        update_myviairq();

      case VIA_PRA_NHS:	/* port A, no handshake */
        READ_PRA
        byte = (byte & ~myvia[VIA_DDRA]) | (myvia[VIA_PRA] & myvia[VIA_DDRA]);
	return byte;

      case VIA_PRB:		/* port B */
        myviaifr &= ~VIA_IM_CB1;
        if ((myvia[VIA_PCR] & 0xa0) != 0x20)
            myviaifr &= ~VIA_IM_CB2;
        update_myviairq();

        READ_PRB
        byte = (byte & ~myvia[VIA_DDRB]) | (myvia[VIA_PRB] & myvia[VIA_DDRB]);

        if (myvia[VIA_ACR] & 0x80) {
            update_myviatal(rclk);
            byte = (byte & 0x7f) | (((myviapb7 ^ myviapb7x) | myviapb7o) ? 0x80 : 0);
        }
        return byte;

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        myviaifr &= ~VIA_IM_T1;
        update_myviairq();
        return myviata() & 0xff;

      case VIA_T1CH /*TIMER_AH */ :	/* timer A high */
        return (myviata() >> 8) & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        myviaifr &= ~VIA_IM_T2;
        update_myviairq();
        return myviatb() & 0xff;

      case VIA_T2CH /*TIMER_BH */ :	/* timer B high */
        return (myviatb() >> 8) & 0xff;

      case VIA_SR:		/* Serial Port Shift Register */
        return (myvia[addr]);

        /* Interrupts */

      case VIA_IFR:		/* Interrupt Flag Register */
        {
            BYTE t = myviaifr;
            if (myviaifr & myviaier /*[VIA_IER] */ )
                t |= 0x80;
            return (t);
        }

      case VIA_IER:		/* 6522 Interrupt Control Register */
        return (myviaier /*[VIA_IER] */  | 0x80);

    }				/* switch */

    return (myvia[addr]);
}

BYTE REGPARM1 peek_myvia(ADDRESS addr)
{
    CLOCK rclk = myclk;

    addr &= 0xf;

    if (myviatai && (myviatai <= myclk))
	int_myviat1(myclk - myviatai);
    if (myviatbi && (myviatbi <= myclk))
	int_myviat2(myclk - myviatbi);

    switch (addr) {
      case VIA_PRA:
        return read_myvia(VIA_PRA_NHS);

      case VIA_PRB:		/* port B */
        {
            BYTE byte;

            READ_PRB
            if (myvia[VIA_ACR] & 0x80) {
                update_myviatal(rclk);
                byte = (byte & 0x7f) | (((myviapb7 ^ myviapb7x) | myviapb7o) ? 0x80 : 0);
            }
            return byte;
        }

        /* Timers */

      case VIA_T1CL /*TIMER_AL */ :	/* timer A low */
        return myviata() & 0xff;

      case VIA_T2CL /*TIMER_BL */ :	/* timer B low */
        return myviatb() & 0xff;

      default:
        break;
    }				/* switch */

    return read_myvia(addr);
}


/* ------------------------------------------------------------------------- */

int int_myviat1(long offset)
{
/*    CLOCK rclk = myclk - offset; */
#ifdef MYVIA_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("myvia timer A interrupt\n");
#endif

    if (!(myvia[VIA_ACR] & 0x40)) {	/* one-shot mode */
#if 0				/* defined (MYVIA_TIMER_DEBUG) */
	printf("MYVIA Timer A interrupt -- one-shot mode: next int won't happen\n");
#endif
	mycpu_unset_alarm(A_MYVIAT1);	/*int_clk[I_MYVIAT1] = 0; */
	myviatai = 0;
    } else {			/* continuous mode */
	/* load counter with latch value */
	myviatai += myviatal + 2;
	mycpu_set_alarm_clk(A_MYVIAT1, myviatai);
    }
    myviaifr |= VIA_IM_T1;
    update_myviairq();

    /* TODO: toggle PB7? */
    return 0;			/*(viaier & VIA_IM_T1) ? 1:0; */
}

/*
 * Timer B is always in one-shot mode
 */

int int_myviat2(long offset)
{
#ifdef MYVIA_TIMER_DEBUG
    if (app_resources.debugFlag)
	printf("MYVIA timer B interrupt\n");
#endif
    mycpu_unset_alarm(A_MYVIAT2);	/*int_clk[I_MYVIAT2] = 0; */

    myviaifr |= VIA_IM_T2;
    update_myviairq();

    return 0;
}

void myvia_prevent_clk_overflow(CLOCK sub)
{
    unsigned int t;
    t = (myviatau - (myclk + sub)) & 0xffff;
    myviatau = myclk + t;
    t = (myviatbu - (myclk + sub)) & 0xffff;
    myviatbu = myclk + t;
    if (myviatai)
	myviatai -= sub;
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


int myvia_write_snapshot_module(FILE * p)
{

    if (myviatai && (myviatai <= myclk))
        int_myviat1(myclk - myviatai);
    if (myviatbi && (myviatbi <= myclk))
        int_myviat2(myclk - myviatbi);

    snapshot_write_module_header(p, "MYVIA",
                        VIA_DUMP_VER_MAJOR, VIA_DUMP_VER_MINOR);

    snapshot_write_byte(p, myvia[VIA_PRA]);
    snapshot_write_byte(p, myvia[VIA_DDRA]);
    snapshot_write_byte(p, myvia[VIA_PRB]);
    snapshot_write_byte(p, myvia[VIA_DDRB]);

    snapshot_write_word(p, myviatal);
    snapshot_write_word(p, myviata());
    snapshot_write_byte(p, myviatbl);
    snapshot_write_word(p, myviatb());

    snapshot_write_byte(p, myvia[VIA_SR]);
    snapshot_write_byte(p, myvia[VIA_ACR]);
    snapshot_write_byte(p, myvia[VIA_PCR]);

    snapshot_write_byte(p, myviaifr);
    snapshot_write_byte(p, myviaier);

						/* FIXME! */
    snapshot_write_byte(p, (((myviapb7 ^ myviapb7x) | myviapb7o) ? 0x80 : 0));
    snapshot_write_byte(p, 0);			/* SRHBITS */

    return 0;
}

int myvia_read_snapshot_module(FILE * p)
{
    char name[SNAPSHOT_MODULE_NAME_LEN];
    BYTE vmajor, vminor;
    BYTE byte;
    WORD word;
    /* DWORD dword; */
    ADDRESS addr;
    CLOCK rclk = clk;

    snapshot_read_module_header(p, name, &vmajor, &vminor);

    if(strcmp(name, "MYVIA") || vmajor != VIA_DUMP_VER_MAJOR) return -1;

    snapshot_read_byte(p, &myvia[VIA_PRA]);
    snapshot_read_byte(p, &myvia[VIA_DDRA]);
    snapshot_read_byte(p, &myvia[VIA_PRB]);
    snapshot_read_byte(p, &myvia[VIA_DDRB]);
    {
        addr = VIA_DDRA;
	byte = myvia[VIA_PRA] | ~myvia[VIA_DDRA];
	oldpa = byte ^ 0xff;
	UNDUMP_PRA
	oldpa = byte;

	addr = VIA_DDRB;
	byte = myvia[VIA_PRB] | ~myvia[VIA_DDRB];
	oldpb = byte ^ 0xff;
	UNDUMP_PRB
	oldpb = byte;
    }

    snapshot_read_word(p, &word);
    myviatal = word;
    snapshot_read_word(p, &word);
    myviatau = rclk + word + 1 /* 3 */ + TAUOFFSET;
    myviatai = rclk + word /* + 2 */;
    mycpu_set_alarm_clk(A_MYVIAT1, myviatai);

    snapshot_read_byte(p, &byte);
    myviatbl = byte;
    snapshot_read_word(p, &word);
    myviatbu = rclk + word + 1 /* 3 */;
    myviatbi = rclk + word /* + 2 */;
    mycpu_set_alarm(A_MYVIAT2, myviatbi);

    snapshot_read_byte(p, &myvia[VIA_SR]);
    {
	addr = myvia[VIA_SR];
	byte = myvia[addr];
	STORE_SR
    }
    snapshot_read_byte(p, &myvia[VIA_ACR]);
    snapshot_read_byte(p, &myvia[VIA_PCR]);
    {
	addr = myvia[VIA_PCR];
	byte = myvia[addr];
	STORE_PCR
    }

    snapshot_read_byte(p, &byte);
    myviaifr = byte;
    snapshot_read_byte(p, &byte);
    myviaier = byte;
    update_myviairq();
						/* FIXME! */
    snapshot_read_byte(p, &byte);
    myviapb7 = byte ? 1 : 0;
    myviapb7x = 0;
    myviapb7o = 0;
    snapshot_read_byte(p, &byte);		/* SRHBITS */

    return 0;
}

POST_VIA_FUNCS
