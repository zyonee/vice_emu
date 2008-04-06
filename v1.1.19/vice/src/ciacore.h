/*
 * ciacore.h - Template file for MOS6526 (CIA) emulation.
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@rzgw.rz.fh-hannover.de>
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
#include <string.h>

#include "alarm.h"
#include "cia.h"
#include "interrupt.h"
#include "log.h"
#include "resources.h"
#include "snapshot.h"
#include "types.h"

/*
 * Prototypes for the used inline functions 
 */

static inline void do_reset_cia(void);
static inline void store_sdr(BYTE byte);
static inline void read_ciaicr(void);
static inline void store_ciapa(CLOCK rclk, BYTE byte);
static inline void store_ciapb(CLOCK rclk, BYTE byte);
static inline void pulse_ciapc(CLOCK rclk);
static inline void undump_ciapa(CLOCK rclk, BYTE byte);
static inline void undump_ciapb(CLOCK rclk, BYTE byte);
static inline BYTE read_ciapa(void);
static inline BYTE read_ciapb(void);


#define	STORE_OFFSET 1
#define	READ_OFFSET 0

#define	CIAT_STOPPED	0
#define	CIAT_RUNNING	1
#define	CIAT_COUNTTA	2

/*
 * Local variable and prototypes - moved here because they're used by
 * the inline functions 
 */

static int int_ciata(long offset);
static int int_ciatb(long offset);
static int int_ciatod(long offset);

#define	ciaier	cia[CIA_ICR]

static alarm_t cia_ta_alarm;
static alarm_t cia_tb_alarm;
static alarm_t cia_tod_alarm;

static int ciaint;		/* Interrupt Flag register for cia 1 */
static CLOCK ciardi;		/* real clock = clk-offset */

static unsigned int cia_tat;	/* timer A toggle bit */
static unsigned int cia_tbt;	/* timer B toggle bit */

static CLOCK cia_todclk;	/* when is the next TOD alarm scheduled? */

static int ciasr_bits;		/* number of bits still to send */
static BYTE cia_shifter;	/* actual shifter */
static int cia_sdr_valid;	/* when set, SDR contains new data to send */

static BYTE oldpa;              /* the actual output on PA (input = high) */
static BYTE oldpb;              /* the actual output on PB (input = high) */

static BYTE ciatodalarm[4];
static BYTE ciatodlatch[4];
static char ciatodstopped;
static char ciatodlatched;
static int ciatodticks = 100000;	/* approx. a 1/10 sec. */

static log_t cia_log = LOG_ERR;

/* global */

static BYTE cia[16];

/* local functions */

static void check_ciatodalarm(CLOCK rclk);


