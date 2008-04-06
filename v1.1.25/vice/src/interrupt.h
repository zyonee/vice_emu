/*
 * interrupt.h - Implementation of 6510 interrupts and alarms.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "6510core.h"
#include "snapshot.h"
#include "types.h"

/* This handles the interrupt lines and the CPU alarms (i.e. events that happen
   at specified clock ticks during emulation).  */

/* For maximum performance (these routines are small but used very often), we
   allow use of inlined functions.  This can be overridden by defining
   NO_INLINE (useful for debugging and profiling).  */
#if !defined NO_INLINE
#  ifndef INLINE_INTERRUPT_FUNCS
#    define INLINE_INTERRUPT_FUNCS
#  endif
#else
#  undef INLINE_INTERRUPT_FUNCS
#endif

/* If this is defined, the interrupt routines take account of the 2-cycle delay
   required by the CPU to detect the NMI/IRQ line transition (otherwise, it
   must be handled somewhere else).  */
#define HANDLE_INTERRUPT_DELAY

#ifdef DEBUG
extern int debugflg;
#endif

/* ------------------------------------------------------------------------- */

/* Interrupts.  This is a bit of a mess...  */
enum {
    I_ACIA1,                    /* ACIA 1 */
    I_ACIA2,                    /* ACIA 2 (unused) */
    I_CIA1FL,                   /* CIA 1 */
    I_CIA2FL,                   /* CIA 2 */
    I_FREEZE,                   /* Cartridge freeze */
    I_PIA1,                     /* PIA1 */
    I_PIA2,                     /* PIA2 */
    I_RASTER,                   /* Raster compare */
    I_RESTORE,                  /* RESTORE key */
    I_REU,                      /* REU */
    I_TPI1,                     /* TPI 1 (CBM-II) */
    I_VIA1FL,                   /* VIA 1 */
    I_VIA2FL,                   /* VIA 2 */
    I_VIAFL,                    /* VIA (PET) */
    I_IEEEVIA1FL,               /* IEEE488 VIC20 VIA1 */
    I_IEEEVIA2FL,               /* IEEE488 VIC20 VIA2 */
    NUMOFINT
};

/* ------------------------------------------------------------------------- */

/* Define the number of cycles needed by the CPU to detect the NMI or IRQ.  */
#ifdef HANDLE_INTERRUPT_DELAY
#  define INTERRUPT_DELAY 2
#else
#  define INTERRUPT_DELAY 0
#endif

/* These are the available types of interrupt lines.  */
enum cpu_int {
    IK_NONE    = 0,
    IK_NMI     = 1 << 0,
    IK_IRQ     = 1 << 1,
    IK_RESET   = 1 << 2,
    IK_TRAP    = 1 << 3,
    IK_MONITOR = 1 << 4,
    IK_DMA     = 1 << 5
};

/* We not care about wasted space here, and make fixed-length large enough
   arrays since static allocation can be handled more easily...  */
struct cpu_int_status {
    /* Number of interrupt lines.  */
    int num_ints;

    /* Define, for each interrupt source, whether it has a pending interrupt
       (IK_IRQ, IK_NMI, IK_RESET and IK_TRAP) or not (IK_NONE).  */
    enum cpu_int pending_int[0x100];

    /* Number of active IRQ lines.  */
    int nirq;

    /* Tick when the IRQ was triggered.  */
    CLOCK irq_clk;

    /* Number of active NMI lines.  */
    int nnmi;

    /* Tick when the NMI was triggered.  */
    CLOCK nmi_clk;

    /* If 1, do a RESET.  */
    int reset;

    /* If 1, call the trapping function.  */
    int trap;

    /* Debugging function.  */
    void (*trap_func)(ADDRESS, void *data);

    /* Data to pass to the debugging function when called.  */
    void *trap_data;

    /* Pointer to the last executed opcode information.  */
    opcode_info_t *last_opcode_info_ptr;

    /* Number of cycles we have stolen to the processor last time.  */
    int num_last_stolen_cycles;

    /* Clock tick at which these cycles have been stolen.  */
    CLOCK last_stolen_cycles_clk;

    enum cpu_int global_pending_int;
};
typedef struct cpu_int_status cpu_int_status_t;

/* ------------------------------------------------------------------------- */

extern void interrupt_log_wrong_nirq(void);
extern void interrupt_log_wrong_nnmi(void);

/* If we do not want the interrupt functions to be inlined, they are only
   compiled once when included in `maincpu.c'.  */

#ifdef INLINE_INTERRUPT_FUNCS
#  define _INT_FUNC inline static
#else
#  define _INT_FUNC
#endif

#if defined INLINE_INTERRUPT_FUNCS || defined _INTERRUPT_C

/* Set the IRQ line state.  */
_INT_FUNC void set_irq(cpu_int_status_t *cs, int int_num, int value,
		       CLOCK clk)
{
    if (value) {		/* Trigger the IRQ.  */
	if (!(cs->pending_int[int_num] & IK_IRQ)) {
	    cs->nirq++;
	    cs->global_pending_int |= IK_IRQ;
	    cs->pending_int[int_num] |= IK_IRQ;

#ifdef HANDLE_INTERRUPT_DELAY
	    /* This makes sure that IRQ delay is correctly emulated when
               cycles are stolen from the CPU.  */
	    if (cs->last_stolen_cycles_clk <= clk) {
  	        cs->irq_clk = clk;
	    } else {
		cs->irq_clk = cs->last_stolen_cycles_clk - 1;
	    }
#endif
	}
    } else {			/* Remove the IRQ condition.  */
	if (cs->pending_int[int_num] & IK_IRQ) {
	    if (cs->nirq > 0) {
		cs->pending_int[int_num] &= ~IK_IRQ;
 		if (--cs->nirq == 0)
		    cs->global_pending_int &= ~IK_IRQ;
	    } else
		interrupt_log_wrong_nirq();
	}
    }
}

/* Set the NMI line state.  */
_INT_FUNC void set_nmi(cpu_int_status_t *cs, int int_num, int value,
		       CLOCK clk)
{
    if (value) {		/* Trigger the NMI.  */
	if (!(cs->pending_int[int_num] & IK_NMI)) {
	    if (cs->nnmi == 0 && !(cs->global_pending_int & IK_NMI)) {
		cs->global_pending_int |= IK_NMI;
#ifdef HANDLE_INTERRUPT_DELAY
		/* This makes sure that NMI delay is correctly emulated when
		   cycles are stolen from the CPU.  */
		if (cs->last_stolen_cycles_clk <= clk) {
		    cs->nmi_clk = clk;
		} else {
		    cs->nmi_clk = cs->last_stolen_cycles_clk - 1;
		}
#endif
	    }
	    cs->nnmi++;
	    cs->pending_int[int_num] |= IK_NMI;
	}
    } else {			/* Remove the NMI condition.  */
	if (cs->pending_int[int_num] & IK_NMI) {
	    if (cs->nnmi > 0) {
		cs->nnmi--;
		cs->pending_int[int_num] &= ~IK_NMI;
		if (clk == cs->nmi_clk)
		    cs->global_pending_int &= ~IK_NMI;
	    } else
		interrupt_log_wrong_nnmi();
	}
    }
}

_INT_FUNC void trigger_dma(cpu_int_status_t *cs, CLOCK clk)
{
    cs->global_pending_int |= IK_DMA;
}

_INT_FUNC void ack_dma(cpu_int_status_t *cs)
{
    cs->global_pending_int &= ~IK_DMA;
}

/* Change the interrupt line state: this can be used to change both NMI and IRQ
   lines.  It is slower than `set_nmi()' and `set_irq()', but is left for
   backward compatibility (it works like the old `setirq()').  */
_INT_FUNC void set_int(cpu_int_status_t *cs, int int_num,
		       enum cpu_int value, CLOCK clk)
{
    set_nmi(cs, int_num, (int)(value & IK_NMI), clk);
    set_irq(cs, int_num, (int)(value & IK_IRQ), clk);
}

/* ------------------------------------------------------------------------- */

/* Return the current status of the IRQ, NMI, RESET and TRAP lines.  */
_INT_FUNC enum cpu_int check_pending_interrupt(cpu_int_status_t *cs)
{
    return cs->global_pending_int;
}

/* Return nonzero if a pending NMI should be dispatched now.  This takes
   account for the internal delays of the 6510, but does not actually check
   the status of the NMI line.  */
_INT_FUNC int check_nmi_delay(cpu_int_status_t *cs, CLOCK clk)
{
    CLOCK nmi_clk = cs->nmi_clk + INTERRUPT_DELAY;

    /* Branch instructions delay IRQs and NMI by one cycle if branch
       is taken with no page boundary crossing.  */
    if (OPINFO_DELAYS_INTERRUPT(*cs->last_opcode_info_ptr))
	nmi_clk++;

    if (clk >= nmi_clk)
	return 1;

    return 0;
}

/* Return nonzero if a pending IRQ should be dispatched now.  This takes
   account for the internal delays of the 6510, but does not actually check
   the status of the IRQ line.  */
_INT_FUNC int check_irq_delay(cpu_int_status_t *cs, CLOCK clk)
{
    CLOCK irq_clk = cs->irq_clk + INTERRUPT_DELAY;

    /* Branch instructions delay IRQs and NMI by one cycle if branch
       is taken with no page boundary crossing.  */
    if (OPINFO_DELAYS_INTERRUPT(*cs->last_opcode_info_ptr))
	irq_clk++;

    /* If an opcode changes the I flag from 1 to 0, the 6510 needs
       one more opcode before it triggers the IRQ routine.  */
    if (clk >= irq_clk && !OPINFO_ENABLES_IRQ(*cs->last_opcode_info_ptr))
	return 1;

    return 0;
}

/* This function must be called by the CPU emulator when a pending NMI
   request is served.  */
_INT_FUNC void ack_nmi(cpu_int_status_t *cs)
{
    cs->global_pending_int &= ~IK_NMI;
}

/* Asynchronously steal `num' cycles from the CPU, starting from cycle
   `start_clk'.  */
_INT_FUNC void steal_cycles(cpu_int_status_t *cs, CLOCK start_clk,
			    CLOCK *clk_ptr, int num)
{
    if (num == 0)
	return;

    if (start_clk == cs->last_stolen_cycles_clk)
	cs->num_last_stolen_cycles += num;
    else
	cs->num_last_stolen_cycles = num;

    *clk_ptr += num;
    cs->last_stolen_cycles_clk = start_clk + num;

    cs->irq_clk += num - 1;
    cs->nmi_clk += num - 1;
}

#else  /* defined INLINE_INTERRUPT_FUNCS || defined _INTERRUPT_C */

/* We don't want inline definitions: just provide the prototypes.  */

extern void set_irq(cpu_int_status_t *cs, int int_num, int value,
		    CLOCK clk);
extern void set_nmi(cpu_int_status_t *cs, int int_num, int value,
		    CLOCK clk);
extern void set_dma(cpu_int_status_t *cs, int int_num, int value,
                    CLOCK clk);
extern void set_int(cpu_int_status_t *cs, int int_num,
		    enum cpu_int value, CLOCK clk);
extern int check_pending_interrupt(cpu_int_status_t *cs);
extern void steal_cycles(cpu_int_status_t *cs, CLOCK start_clk,
			 CLOCK *clk_ptr, int num);
extern int check_irq_delay(cpu_int_status_t *cs, CLOCK clk);
extern int check_nmi_delay(cpu_int_status_t *cs, CLOCK clk);
extern void ack_nmi(cpu_int_status_t *cs);
extern void ack_dma(cpu_int_status_t *cs);

#endif /* defined INLINE_INTERRUPT_FUNCS || defined _INTERRUPT_C */

/* ------------------------------------------------------------------------- */

/* Extern functions.  These are defined in `interrupt.c'.  */

extern void trigger_reset(cpu_int_status_t *cs, CLOCK clk);
extern void ack_reset(cpu_int_status_t *cs);
extern void trigger_trap(cpu_int_status_t *cs,
                         void (*trap_func)(ADDRESS, void *data),
                         void *data, CLOCK clk);
extern void do_trap(cpu_int_status_t *cs, ADDRESS reg_pc);

extern void monitor_trap_on(cpu_int_status_t *cs);
extern void monitor_trap_off(cpu_int_status_t *cs);

extern void cpu_int_status_init(cpu_int_status_t *cs, int num_ints,
				opcode_info_t *last_opcode_info_ptr);
extern void cpu_int_status_time_warp(cpu_int_status_t *cs, CLOCK warp_amount,
                                     int warp_direction);

extern int interrupt_read_snapshot(cpu_int_status_t *cs, snapshot_module_t *m);
extern int interrupt_write_snapshot(cpu_int_status_t *cs,
                                    snapshot_module_t *m);

extern void set_irq_noclk(cpu_int_status_t *cs, int int_num, int value);
extern void set_nmi_noclk(cpu_int_status_t *cs, int int_num, int value);
extern void set_int_noclk(cpu_int_status_t *cs, int int_num,
                          enum cpu_int value);
extern int get_irq(cpu_int_status_t *cs, int int_num);
extern int get_nmi(cpu_int_status_t *cs, int int_num);
extern enum cpu_int get_int(cpu_int_status_t *cs, int int_num);

/* ------------------------------------------------------------------------- */

extern cpu_int_status_t maincpu_int_status;
extern CLOCK clk;

extern cpu_int_status_t drive0_int_status;
extern cpu_int_status_t drive1_int_status;
extern CLOCK drive_clk[2];

/* For convenience...  */

#define maincpu_set_irq(int_num, value)	\
    set_irq(&maincpu_int_status, (int_num), (value), clk)
#define maincpu_set_irq_clk(int_num, value, clk) \
    set_irq(&maincpu_int_status, (int_num), (value), (clk))
#define maincpu_set_nmi(int_num, value) \
    set_nmi(&maincpu_int_status, (int_num), (value), clk)
#define maincpu_set_nmi_clk(int_num, value, clk) \
    set_nmi(&maincpu_int_status, (int_num), (value), (clk))
#define maincpu_set_int(int_num, value) \
    set_int(&maincpu_int_status, (int_num), (value), clk)
#define maincpu_set_int_clk(int_num, value, clk) \
    set_int(&maincpu_int_status, (int_num), (value), (clk))
#define maincpu_set_int_noclk(int_num, value) \
    set_int_noclk(&maincpu_int_status, (int_num), (value))
#define maincpu_trigger_reset() \
    trigger_reset(&maincpu_int_status, clk)
#define maincpu_trigger_dma() \
    trigger_dma(&maincpu_int_status, clk)
#define maincpu_trigger_trap(trap_func, data) \
    trigger_trap(&maincpu_int_status, (trap_func), (data), clk)
#define maincpu_steal_cycles(start_clk, num) \
    steal_cycles(&maincpu_int_status, (start_clk), &clk, (num))

#define drive0_set_irq(int_num, value) \
    set_irq(&drive0_int_status, (int_num), (value), drive_clk[0])
#define drive1_set_irq(int_num, value)    \
    set_irq(&drive1_int_status, (int_num), (value), drive_clk[1])
#define drive0_set_irq_clk(int_num, value, clk) \
    set_irq(&drive0_int_status, (int_num), (value), (clk))
#define drive1_set_irq_clk(int_num, value, clk) \
    set_irq(&drive1_int_status, (int_num), (value), (clk))
#define drive0_set_nmi(int_num, value) \
    set_nmi(&drive0_int_status, (int_num), (value), drive_clk[0])
#define drive1_set_nmi(int_num, value) \
    set_nmi(&drive1_int_status, (int_num), (value), drive_clk[1])
#define drive0_set_nmi_clk(int_num, value, clk) \
    set_nmi(&drive0_int_status, (int_num), (value), (clk))
#define drive1_set_nmi_clk(int_num, value, clk) \
    set_nmi(&drive1_int_status, (int_num), (value), (clk))
#define drive0_set_int(int_num, value) \
    set_int(&drive0_int_status, (int_num), (value), drive_clk[0])
#define drive1_set_int(int_num, value) \
    set_int(&drive1_int_status, (int_num), (value), drive_clk[1])
#define drive0_set_int_clk(int_num, value, clk) \
    set_int_clk(&drive0_int_status, (int_num), (value), (clk))
#define drive1_set_int_clk(int_num, value, clk) \
    set_int_clk(&drive1_int_status, (int_num), (value), (clk))
#define drive0_trigger_reset() \
    trigger_reset(&drive0_int_status, drive_clk[0] + 1)
#define drive1_trigger_reset() \
    trigger_reset(&drive1_int_status, drive_clk[1] + 1)
#define drive0_trigger_trap(trap_func, data) \
    trigger_trap(&drive0_int_status, (trap_func), (data), drive_clk[0] + 1)
#define drive1_trigger_trap(trap_func, data) \
    trigger_trap(&drive1_int_status, (trap_func), (data), drive_clk[1] + 1)

#endif /* !_INTERRUPT_H */
