/*
 * interrupt.h - Implementation of 6510 interrupts and alarms.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
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

#include "types.h"
#include "vmachine.h"

#include <stdio.h>

#include "6510core.h"

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

/* Define the number of cycles needed by the CPU to detect the NMI or IRQ.  */
#ifdef HANDLE_INTERRUPT_DELAY
#  define INTERRUPT_DELAY 2
#else
#  define INTERRUPT_DELAY 0
#endif

/* Maximum value of a CLOCK.  */
#define CLOCK_MAX (~((CLOCK)0))

/* Define the tick when the clock counter overflow must be prevented and what
   value should be subtracted from the counter when the tick is reached.  */
#define PREVENT_CLK_OVERFLOW_TICK (CLOCK_MAX - 0x100000)

/* This defines the value to subtract from the CLOCK counters.  The formula
   is to make sure that it is a multiple of `CYCLES_PER_RFSH' and that it
   allows 16 bit counters to work.  */
#define PREVENT_CLK_OVERFLOW_SUB \
    (((PREVENT_CLK_OVERFLOW_TICK & ~((CLOCK)0xffff)) / CYCLES_PER_RFSH - 1) \
     * CYCLES_PER_RFSH)

/* These are the available types of interrupt lines.  */
enum cpu_int {
    IK_NONE = 0,
    IK_NMI = 1 << 0,
    IK_IRQ = 1 << 1,
    IK_RESET = 1 << 2,
    IK_TRAP = 1 << 3,
    IK_BREAKPT = 1 << 4
};

/* This defines the type for a CPU alarm handler.  The passed argument is set
   to the number of ticks between the time of call and the time of
   scheduling.  */
typedef int cpu_alarm_handler_t(long delay);

/* We not care about wasted space here, and make fixed-length large enough
   arrays since static allocation can be handled more easily...  */
struct cpu_int_status {
    /* Number of possible alarms.  */
    int num_alarms;

    /* Number of interrupt lines.  */
    int num_ints;

    /* Next CPU alarm to be served.  */
    int next_alarm;

    /* Clock tick when the next pending CPU alarm has to be served.  */
    CLOCK next_alarm_clk;

    /* This is nonzero if we might have a pending interrupt.  */
    int should_check_pending_interrupt;

    /* Value of clk when each CPU alarm has to be served. ~0 means `not
       active'.  */
    CLOCK alarm_clk[0x100];

    /* Define, for each interrupt source, whether it has a pending interrupt
       (IK_IRQ, IK_NMI, IK_RESET and IK_TRAP) or not (IK_NONE).  */
    enum cpu_int pending_int[0x100];

    /* CPU alarm handlers.  */
    cpu_alarm_handler_t *alarm_handler[0x100];

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
    void (*trap_func)(ADDRESS);

    /* Pointer to the last executed opcode information.  */
    opcode_info_t *last_opcode_info_ptr;

    /* Number of cycles we have stolen to the processor last time.  */
    int num_last_stolen_cycles;

    /* Clock tick at which these cycles have been stolen.  */
    CLOCK last_stolen_cycles_clk;

    enum cpu_int global_pending_int;
};

/* ------------------------------------------------------------------------- */

/* If we do not want the interrupt functions to be inlined, they are only
   compiled once when included in `maincpu.c'.  */

#ifdef INLINE_INTERRUPT_FUNCS
#  define _INT_FUNC inline static
#else
#  define _INT_FUNC
#endif

#if defined INLINE_INTERRUPT_FUNCS || defined _MAINCPU_C

#include <string.h>		/* memset() */

/* Initialization.  */
_INT_FUNC void cpu_int_status_init(struct cpu_int_status *cs, int num_ints,
 				   int num_alarms,
				   opcode_info_t *last_opcode_info_ptr)
{
    int i;

    memset(cs, 0, sizeof(struct cpu_int_status));
    cs->num_ints = num_ints;
    cs->num_alarms = num_alarms;
    for (i = 0; i < 0x100; i++)
	cs->alarm_clk[i] = CLOCK_MAX;
    cs->next_alarm_clk = CLOCK_MAX;
    cs->last_opcode_info_ptr = last_opcode_info_ptr;
    cs->num_last_stolen_cycles = 0;
    cs->last_stolen_cycles_clk = (CLOCK)0;
    cs->global_pending_int = IK_NONE;
}

/* Set the IRQ line state.  */
_INT_FUNC void set_irq(struct cpu_int_status *cs, int int_num, int value,
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
		printf("set_irq(): wrong nirq!\n");
	}
    }
}

/* Set the NMI line state.  */
_INT_FUNC void set_nmi(struct cpu_int_status *cs, int int_num, int value,
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
		printf("set_nmi(): wrong nnmi!\n");
	}
    }
}

/* Change the interrupt line state: this can be used to change both NMI and IRQ
   lines.  It is slower than `set_nmi()' and `set_irq()', but is left for
   backward compatibility (it works like the old `setirq()').  */
_INT_FUNC void set_int(struct cpu_int_status *cs, int int_num,
		       enum cpu_int value, CLOCK clk)
{
    set_nmi(cs, int_num, value & IK_NMI, clk);
    set_irq(cs, int_num, value & IK_IRQ, clk);
}

/* Trigger a RESET.  This resets the machine.  */
_INT_FUNC void trigger_reset(struct cpu_int_status *cs, CLOCK clk)
{
    cs->global_pending_int |= IK_RESET;
}

/* Acknowledge a RESET condition, by removing it.  */
_INT_FUNC void ack_reset(struct cpu_int_status *cs)
{
    cs->global_pending_int &= ~IK_RESET;
}

/* Trigger a TRAP.  This is a special condition that can be used for
   debugging.  `trap_func' will be called with PC as the argument when this
   condition is detected.  */
_INT_FUNC void trigger_trap(struct cpu_int_status *cs,
			    void (*trap_func)(ADDRESS), CLOCK clk)
{
    cs->global_pending_int |= IK_TRAP;
    cs->trap_func = trap_func;
}

/* Acknowledge a TRAP condition, by resetting the TRAP line.  */
_INT_FUNC void ack_trap(struct cpu_int_status *cs)
{
    cs->global_pending_int &= ~IK_TRAP;
}

_INT_FUNC void breakpoints_on(struct cpu_int_status *cs)
{
    cs->global_pending_int |= IK_BREAKPT;
}

_INT_FUNC void breakpoints_off(struct cpu_int_status *cs)
{
    cs->global_pending_int &= ~IK_BREAKPT;
}

/* ------------------------------------------------------------------------- */

/* Find the next pending alarm.  */
_INT_FUNC void find_next_alarm(struct cpu_int_status *cs)
{
    CLOCK next_alarm_clk = CLOCK_MAX;
    int next_alarm = 0;
    int i;

    for (i = 0; i < cs->num_ints; i++)
	if (cs->alarm_clk[i] < next_alarm_clk) {
	    next_alarm = i;
	    next_alarm_clk = cs->alarm_clk[i];
	}

    cs->next_alarm = next_alarm;
    cs->next_alarm_clk = next_alarm_clk;
}

/* Return the clock tick for the next alarm.  */
_INT_FUNC CLOCK next_alarm_clk(struct cpu_int_status *cs)
{
    return cs->next_alarm_clk;
}

/* Schedule one cpu alarm.  */
_INT_FUNC void set_alarm_clk(struct cpu_int_status *cs, int alarm,
			     CLOCK tick)
{
    if (tick != 0 && tick < cs->next_alarm_clk) {
        cs->next_alarm_clk = cs->alarm_clk[alarm] = tick;
	cs->next_alarm = alarm;
    } else {
	if (tick == 0)
	    tick = CLOCK_MAX;
	cs->alarm_clk[alarm] = tick;
	if (alarm == cs->next_alarm)
	    find_next_alarm(cs);
    }
}

/* Unschedule one cpu alarm.  */
_INT_FUNC void unset_alarm(struct cpu_int_status *cs, int alarm)
{
    cs->alarm_clk[alarm] = CLOCK_MAX;
    if (alarm == cs->next_alarm)
	find_next_alarm(cs);
}

/* ------------------------------------------------------------------------- */

/* Return the current status of the IRQ, NMI, RESET and TRAP lines.  */
_INT_FUNC enum cpu_int check_pending_interrupt(struct cpu_int_status *cs)
{
    return cs->global_pending_int;
}

/* Return nonzero if a pending NMI should be dispatched now.  This takes
   account for the internal delays of the 6510, but does not actually check
   the status of the NMI line.  */
_INT_FUNC int check_nmi_delay(struct cpu_int_status *cs, CLOCK clk)
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
_INT_FUNC int check_irq_delay(struct cpu_int_status *cs, CLOCK clk)
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
_INT_FUNC void ack_nmi(struct cpu_int_status *cs)
{
    cs->global_pending_int &= ~IK_NMI;
}


/* Serve the next pending alarm and update the struct so that we know what
   comes next.  If there is a pending interrupt, return its kind.  Otherwise,
   return IK_NONE.  */
_INT_FUNC void serve_next_alarm(struct cpu_int_status *cs, CLOCK clk)
{
    long offset = clk - cs->next_alarm_clk;

    (cs->alarm_handler[cs->next_alarm])(offset);
}

/* This is used to avoid clock counter overflows.  Return 1 if the clock
   counters have been changed (this happens if the clock counter `clk' has
   reached `PREVENT_CLK_OVERFLOW_TICK').  */
_INT_FUNC int prevent_clk_overflow(struct cpu_int_status *cs, CLOCK *clk)
{
    if (*clk > PREVENT_CLK_OVERFLOW_TICK) {
	int i;

	*clk -= PREVENT_CLK_OVERFLOW_SUB;
	cs->next_alarm_clk -= PREVENT_CLK_OVERFLOW_SUB;
	cs->irq_clk -= PREVENT_CLK_OVERFLOW_SUB;
	cs->nmi_clk -= PREVENT_CLK_OVERFLOW_SUB;
	if (cs->last_stolen_cycles_clk > PREVENT_CLK_OVERFLOW_SUB)
	    cs->last_stolen_cycles_clk -= PREVENT_CLK_OVERFLOW_SUB;
	else
	    cs->last_stolen_cycles_clk = (CLOCK) 0;
	for (i = 0; i < cs->num_alarms; i++) {
	    if (cs->alarm_clk[i] != CLOCK_MAX)
		cs->alarm_clk[i] -= PREVENT_CLK_OVERFLOW_SUB;
	}
	return 1;
    } else
	return 0;
}

/* Asynchronously steal `num' cycles from the CPU, starting from cycle
   `start_clk'.  */
_INT_FUNC void steal_cycles(struct cpu_int_status *cs, CLOCK start_clk,
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

#else  /* defined INLINE_INTERRUPT_FUNCS || defined _MAINCPU_C */

/* We don't want inline definitions: just provide the prototypes.  */

extern void cpu_int_status_init(struct cpu_int_status *cs, int num_ints,
				int num_alarms,
				opcode_info_t *last_opcode_info_ptr);
extern void set_irq(struct cpu_int_status *cs, int int_num, int value,
		    CLOCK clk);
extern void set_nmi(struct cpu_int_status *cs, int int_num, int value,
		    CLOCK clk);
extern void set_int(struct cpu_int_status *cs, int int_num,
		    enum cpu_int value, CLOCK clk);
extern void trigger_reset(struct cpu_int_status *cs, CLOCK clk);
extern void trigger_trap(struct cpu_int_status *cs,
			 void (*trap_func)(ADDRESS), CLOCK clk);
extern void find_next_alarm(struct cpu_int_status *cs);
extern CLOCK next_alarm_clk(struct cpu_int_status *cs);
extern void set_alarm_clk(struct cpu_int_status *cs, int alarm, CLOCK tick);
extern void unset_alarm(struct cpu_int_status *cs, int alarm);
extern int check_pending_interrupt(struct cpu_int_status *cs);
extern int serve_next_alarm(struct cpu_int_status *cs, CLOCK clk);
extern int prevent_clk_overflow(struct cpu_int_status *cs, CLOCK *clk);
extern void steal_cycles(struct cpu_int_status *cs, CLOCK start_clk,
			 CLOCK *clk_ptr, int num);
extern int check_irq_delay(struct cpu_int_status *cs, CLOCK clk);
extern int check_nmi_delay(struct cpu_int_status *cs, CLOCK clk);
extern void ack_nmi(struct cpu_int_status *cs);
extern void ack_reset(struct cpu_int_status *cs);
extern void ack_trap(struct cpu_int_status *cs);

#endif /* defined INLINE_INTERRUPT_FUNCS || defined _MAINCPU_C */

/* ------------------------------------------------------------------------- */

extern struct cpu_int_status maincpu_int_status;
extern CLOCK clk;

extern struct cpu_int_status true1541_int_status;
extern CLOCK true1541_clk;

/* For convenience...  */

#define maincpu_set_alarm_clk(alarm, tick) \
    set_alarm_clk(&maincpu_int_status, (alarm), (tick))
#define maincpu_set_alarm(alarm, ticks_from_now) \
    maincpu_set_alarm_clk((alarm), clk + (ticks_from_now))
#define maincpu_unset_alarm(alarm) \
    unset_alarm(&maincpu_int_status, (alarm))
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
#define maincpu_trigger_reset() \
    trigger_reset(&maincpu_int_status, clk)
#define maincpu_trigger_trap(trap_func) \
    trigger_trap(&maincpu_int_status, (trap_func), clk)
#define maincpu_serve_next_alarm() \
    serve_next_alarm(&maincpu_int_status, clk)
#define maincpu_prevent_clk_overflow() \
    prevent_clk_overflow(&maincpu_int_status, &clk)
#define maincpu_steal_cycles(start_clk, num) \
    steal_cycles(&maincpu_int_status, (start_clk), &clk, (num))
#define maincpu_breakpoints_on() \
    breakpoints_on(&maincpu_int_status)
#define maincpu_breakpoints_off() \
    breakpoints_off(&maincpu_int_status)

#define true1541_set_alarm_clk(alarm, tick) \
    set_alarm_clk(&true1541_int_status, (alarm), (tick))
#define true1541_set_alarm(alarm, ticks_from_now) \
    true1541_set_alarm_clk((alarm), true1541_clk + (ticks_from_now))
#define true1541_unset_alarm(alarm) \
    unset_alarm(&true1541_int_status, (alarm))
#define true1541_set_irq(int_num, value)	\
    set_irq(&true1541_int_status, (int_num), (value), true1541_clk)
#define true1541_set_irq_clk(int_num, value, clk) \
    set_irq(&true1541_int_status, (int_num), (value), (clk))
#define true1541_set_nmi(int_num, value) \
    set_nmi(&true1541_int_status, (int_num), (value), true1541_clk)
#define true1541_set_nmi_clk(int_num, value, clk) \
    set_nmi(&true1541_int_status, (int_num), (value), (clk))
#define true1541_set_int(int_num, value) \
    set_int(&true1541_int_status, (int_num), (value), true1541_clk)
#define true1541_set_int_clk(int_num, value, clk) \
    set_int_clk(&true1541_int_status, (int_num), (value), (clk))
#define true1541_trigger_reset() \
    trigger_reset(&true1541_int_status, true1541_clk + 1)
#define true1541_trigger_trap(trap_func) \
    trigger_trap(&true1541_int_status, (trap_func), true1541_clk + 1)
#define true1541_serve_next_alarm() \
    serve_next_alarm(&true1541_int_status, true1541_clk)
#define true1541_breakpoints_on() \
    breakpoints_on(&true1541_int_status)
#define true1541_breakpoints_off() \
    breakpoints_off(&true1541_int_status)

#endif /* !_INTERRUPT_H */
