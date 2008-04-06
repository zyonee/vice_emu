/*
 * drivetypes.h - drive-specific types like the drive context structure.
 *
 * Written by
 *  Andreas Dehmel <dehmel@forwiss.tu-muenchen.de>
 *   Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _DRIVETYPES_H
#define _DRIVETYPES_H


#include "alarm.h"
#include "ciatimer.h"
#include "clkguard.h"
#include "drive.h"
#include "interrupt.h"
#include "log.h"
#include "mos6510.h"
#include "types.h"


/*
 *  The philosophy behind this approach is that only the drive module knows
 *  the exact layout of the drive_context_t structure. Therefore only include
 *  drivetypes.h from source files within the drive module. All other modules
 *  only need to use pointers transparently, which only requires a forward
 *  declaration of struct drive_context_s (see below).
 */

struct drive_context_s;         /* forward declaration */
struct monitor_interface_s;

/* This defines the memory access for the drive CPU.  */
typedef BYTE REGPARM2 drive_read_func_t(struct drive_context_s *, ADDRESS);
typedef void REGPARM3 drive_store_func_t(struct drive_context_s *, ADDRESS,
                                         BYTE);


/* This table is used to approximate the sync between the main and the
   drive CPU, since the two clock rates are different.  */
#define MAX_TICKS 0x1000

/*
 *  The private CPU data.
 */

typedef struct drivecpu_context_s {

  int traceflg;
  /* This is non-zero each time a Read-Modify-Write instructions that accesses
     memory is executed.  We can emulate the RMW bug of the 6502 this way.  */
  int rmw_flag; /* init to 0 */

  /* Interrupt/alarm status.  */
  struct cpu_int_status_s int_status;
  alarm_context_t alarm_context;
  /* Clk guard.  */
  clk_guard_t clk_guard;

  struct monitor_interface_s *monitor_interface;

  /* Value of clk for the last time mydrive_cpu_execute() was called.  */
  CLOCK last_clk;
  /* Number of cycles in excess we executed last time mydrive_cpu_execute()
     was called.  */
  CLOCK last_exc_cycles;

  CLOCK cycle_accum;
  BYTE *d_bank_base;
  int d_bank_limit;     /* init to -1 */

  /* Information about the last executed opcode.  */
  opcode_info_t last_opcode_info;
  /* Public copy of the registers.  */
  mos6510_regs_t cpu_regs;

  BYTE *pageone;        /* init to NULL */

  int monspace;         /* init to e_disk[89]_space */

  char snap_module_name[12];    /* init to "DRIVECPU[01]" */

  char identification_string[8];        /* init to "DRIVE#[89]" */

} drivecpu_context_t;


/*
 *  Large data used in the CPU emulation. Often more efficient to move
 *  to the end of the drive context structure to minimize the average
 *  offset of members within the context structure.
 */

typedef struct drivecpud_context_s {

  /* Drive RAM */
  BYTE drive_ram[DRIVE_RAM_SIZE];

  /* functions */
  drive_read_func_t  *read_func[0x101];
  drive_store_func_t *store_func[0x101];
  drive_read_func_t  *read_func_watch[0x101];
  drive_store_func_t *store_func_watch[0x101];
  drive_read_func_t  *read_func_nowatch[0x101];
  drive_store_func_t *store_func_nowatch[0x101];

  unsigned long clk_conv_table[MAX_TICKS + 1];
  unsigned long clk_mod_table[MAX_TICKS + 1];

} drivecpud_context_t;



/*
 *  Private VIA data shared between VIA1 and VIA2.
 */

typedef struct drivevia_context_s {

  BYTE via[16];
  int ifr;
  int ier;
  unsigned int tal;
  unsigned int tbl;
  CLOCK tau;
  CLOCK tbu;
  CLOCK tai;
  CLOCK tbi;
  int pb7;
  int pb7x;
  int pb7o;
  int pb7xx;
  int pb7sx;
  BYTE oldpa;
  BYTE oldpb;
  BYTE ila;
  BYTE ilb;
  int ca2_state;
  int cb2_state;
  alarm_t t1_alarm;
  alarm_t t2_alarm;
  log_t log;                    /* init to LOG_ERR */

  CLOCK read_clk;               /* init to 0 */
  int read_offset;              /* init to 0 */
  BYTE last_read;               /* init to 0 */

  int irq_type;                 /* I_... */
  int irq_line;                 /* IK_... */

  char myname[12];              /* init to "DriveXViaY" */
  char my_module_name[8];       /* init to "VIAXDY" */

} drivevia_context_t;


/*
 *  Some function pointers shared by several components (VIA1, CIA1581, RIOT2)
 */

typedef struct drivefunc_context_s {

  void (*iec_write)(BYTE);
  BYTE (*iec_read)(void);
  void (*parallel_set_bus)(BYTE);
  void (*parallel_set_eoi)(char);       /* we may be able to eleminate these...
*/
  void (*parallel_set_dav)(char);
  void (*parallel_set_ndac)(char);
  void (*parallel_set_nrfd)(char);
  void (*parallel_cable_write)(BYTE, int);

} drivefunc_context_t;


/*
 *  Additional data required for VIA1.
 */

typedef struct drivevia1_context_s {

  int parallel_id;

  int v_parieee_is_out;         /* init to 1 */
  struct iec_info_s *v_iec_info;

} drivevia1_context_t;


/*
 *  Private drive CIA data, shared between CIA1571 and CIA1581.
 */

typedef struct drivecia_context_s {

  BYTE c_cia[16];
  alarm_t ta_alarm;
  alarm_t tb_alarm;
  alarm_t tod_alarm;
  int irqflags;
  CLOCK rdi;
  unsigned int tat;
  unsigned int tbt;
  CLOCK todclk;
  int sr_bits;
  int sdr_valid;
  BYTE shifter;
  BYTE old_pa;
  BYTE old_pb;
  char todstopped;
  char todlatched;
  BYTE todalarm[4];
  BYTE todlatch[4];
  int todticks;                 /* init to 100000 */
  log_t log;                    /* init to LOG_ERR */

  ciat_t ta;
  ciat_t tb;
  CLOCK read_clk;               /* init to 0 */
  int read_offset;              /* init to 0 */
  BYTE last_read;               /* init to 0 */
  int debugFlag;                /* init to 0 */

  int irq_line;                 /* IK_IRQ */

  char myname[12];

} drivecia_context_t;

/*
 *  Additional data required for CIA1581: this is only one new variable,
 *  c_iec_info, so we don't use a struct.
 */




/*
 *  Private drive RIOT data, shared between RIOT1 and RIOT2
 */

typedef struct driveriot_context_s {

  BYTE riot_io[4];
  BYTE old_pa;
  BYTE old_pb;

  log_t log;            /* init to LOG_ERR */

  alarm_t alarm;

  CLOCK read_clk;       /* init to 0 */
  int read_offset;      /* init to 0 */
  BYTE last_read;       /* init to 0 */
  BYTE r_edgectrl;      /* init to 0 */
  BYTE r_irqfl;         /* init to 0 */
  BYTE r_irqline;       /* init to 0 */

  CLOCK r_write_clk;
  int r_N;
  int r_divider;
  int r_irqen;

  char myname[12];

} driveriot_context_t;


/*
 *  additional data for RIOT2.
 */

typedef struct driveriot2_context_s {

  int r_atn_active;     /* init to 0 */
  int irq_type;         /* I_RIOTD#FL */

} driveriot2_context_t;



/*
 * The context for an entire drive.
 */

typedef struct drive_context_s {

  int mynumber;         /* init to [01] */
  CLOCK *clk_ptr;       /* shortcut to drive_clk[mynumber] */
  struct drive_s *drive_ptr;    /* shortcut to drive[mynumber] */

  drivecpu_context_t cpu;
  drivefunc_context_t func;
  drivevia_context_t via1;
  drivevia1_context_t via1p;
  drivevia_context_t via2;
  drivecia_context_t cia1571;
  drivecia_context_t cia1581;
  struct iec_info_s *c_iec_info;        /* for CIA1581 */
  driveriot_context_t riot1;
  driveriot_context_t riot2;
  driveriot2_context_t riot2p;
  drivecpud_context_t cpud;

} drive_context_t;


#endif

