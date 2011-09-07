/*
 * 65816core.c - 65816/65802 emulation core.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

/* This file is currently not included by any CPU definition files */

/* any CPU definition file that includes this file needs to do the following:
 *
 * - define reg_a as 16bit (8bit on 6502/65C02).
 * - define reg_x as 16bit (8bit on 6502/65C02).
 * - define reg_y as 16bit (8bit on 6502/65C02).
 * - define reg_pbr (Program Bank Register) as 8bit.
 * - define reg_dbr (Data Bank Register) as 8bit.
 * - define reg_dpr (Direct Page Register) as 16bit.
 * - define reg_emul (65C02 Emulation) as int.
 * - define reg_sp as 16bit (8bit on 6502/65C02).
 * - define a function to handle the STP opcode (STP_65816(void)).
 * - define a function to handle the WAI opcode (WAI_65816(void)).
 * - define a function to handle the COP opcode (COP_65816(BYTE value)).
 */

#define CPU_STR "65816/65802 CPU"

#include "traps.h"

#define CLK_RTS 3
#define CLK_RTI 4
#define CLK_BRK 5
#define CLK_ABS_I_STORE2 2
#define CLK_STACK_PUSH 1
#define CLK_STACK_PULL 2
#define CLK_ABS_RMW2 3
#define CLK_ABS_I_RMW2 3
#define CLK_ZERO_I_STORE 2
#define CLK_ZERO_I2 2
#define CLK_ZERO_RMW 3
#define CLK_ZERO_I_RMW 4
#define CLK_IND_X_RMW 3
#define CLK_IND_Y_RMW1 1
#define CLK_IND_Y_RMW2 3
#define CLK_BRANCH2 1
#define CLK_INT_CYCLE 1
#define CLK_JSR_INT_CYCLE 1
#define CLK_IND_Y_W 2
#define CLK_NOOP_ZERO_X 2

#define IRQ_CYCLES      7
#define NMI_CYCLES      7
#define RESET_CYCLES    6

/* ------------------------------------------------------------------------- */
/* Backup for non-6509 CPUs.  */

#ifndef LOAD_IND
#define LOAD_IND(a)     LOAD(a)
#endif
#ifndef STORE_IND
#define STORE_IND(a,b)  STORE(a,b)
#endif

/* ------------------------------------------------------------------------- */
/* Backup for non-variable cycle CPUs.  */

#ifndef CLK_ADD
#define CLK_ADD(clock, amount) clock += amount
#endif

#ifndef REWIND_FETCH_OPCODE
#define REWIND_FETCH_OPCODE(clock, amount) clock -= amount
#endif

/* ------------------------------------------------------------------------- */
/* Hook for additional delay.  */

#ifndef CPU_DELAY_CLK
#define CPU_DELAY_CLK
#endif

#ifndef CPU_REFRESH_CLK
#define CPU_REFRESH_CLK
#endif

/* ------------------------------------------------------------------------- */

#define LOCAL_SET_NZ(val, bits8)         \
    do {                                 \
        if (reg_emul) {                  \
            (flag_z = flag_n = (val));   \
        } else {                         \
            if (!bits8) {                \
                flag_z = (val) ? 1 : 0;  \
                flag_n = (val >> 8);     \
            } else {                     \
                flag_z = flag_n = (val); \
            }                            \
        }                                \
    }

#define LOCAL_SET_OVERFLOW(val)   \
    do {                          \
        if (val)                  \
            reg_p |= P_OVERFLOW;  \
        else                      \
            reg_p &= ~P_OVERFLOW; \
    } while (0)

#define LOCAL_SET_BREAK(val)   \
    do {                       \
        if (val)               \
            reg_p |= P_BREAK;  \
        else                   \
            reg_p &= ~P_BREAK; \
    } while (0)

#define LOCAL_SET_DECIMAL(val)   \
    do {                         \
        if (val)                 \
            reg_p |= P_DECIMAL;  \
        else                     \
            reg_p &= ~P_DECIMAL; \
    } while (0)

#define LOCAL_SET_INTERRUPT(val)   \
    do {                           \
        if (val)                   \
            reg_p |= P_INTERRUPT;  \
        else                       \
            reg_p &= ~P_INTERRUPT; \
    } while (0)

#define LOCAL_SET_CARRY(val)   \
    do {                       \
        if (val)               \
            reg_p |= P_CARRY;  \
        else                   \
            reg_p &= ~P_CARRY; \
    } while (0)

#define LOCAL_SET_65816_M(val)   \
    do {                         \
        if (val)                 \
            reg_p |= P_65816_M;  \
        else                     \
            reg_p &= ~P_65816_M; \
    } while (0)

#define LOCAL_SET_65816_X(val)   \
    do {                         \
        if (val)                 \
            reg_p |= P_65816_X;  \
        else                     \
            reg_p &= ~P_65816_X; \
    } while (0)

#define LOCAL_SET_SIGN(val)      (flag_n = (val) ? 0x80 : 0)
#define LOCAL_SET_ZERO(val)      (flag_z = !(val))
#define LOCAL_SET_STATUS(val)    (reg_p = ((val) & ~(P_ZERO | P_SIGN)), \
                                  LOCAL_SET_ZERO((val) & P_ZERO),       \
                                  flag_n = (val))

#define LOCAL_OVERFLOW()         (reg_p & P_OVERFLOW)
#define LOCAL_BREAK()            (reg_p & P_BREAK)
#define LOCAL_DECIMAL()          (reg_p & P_DECIMAL)
#define LOCAL_INTERRUPT()        (reg_p & P_INTERRUPT)
#define LOCAL_CARRY()            (reg_p & P_CARRY)
#define LOCAL_SIGN()             (flag_n & 0x80)
#define LOCAL_ZERO()             (!flag_z)
#define LOCAL_STATUS()           (reg_p | (flag_n & 0x80) | P_UNUSED    \
                                  | (LOCAL_ZERO() ? P_ZERO : 0))

#define LOCAL_65816_M()          ((reg_emul) ? (reg_p & P_65816_M))
#define LOCAL_65816_X()          ((reg_emul) ? (reg_p & P_65816_X))

#define LOCAL_65816_STATUS()     (reg_p | (flag_n & 0x80) | (LOCAL_ZERO() ? P_ZERO : 0)

/* determine the bitsizes,
   0 = 8bit A and 8bit X/Y
   1 = 8bit A and 16bit X/Y
   2 = 16bit A and 8bit X/Y
   3 = 16bit A and 16bit X/Y
 */
#define LOCAL_REGISTER_SIZES() \
    (LOCAL_65816_M() ? (LOCAL_65816_X() ? 0 : 1) : (LOCAL_65816_X() ? 2 : 3))

#ifdef LAST_OPCODE_INFO

/* If requested, gather some info about the last executed opcode for timing
   purposes.  */

/* Remember the number of the last opcode.  By default, the opcode does not
   delay interrupt and does not change the I flag.  */
#define SET_LAST_OPCODE(x) \
    OPINFO_SET(LAST_OPCODE_INFO, (x), 0, 0, 0)

/* Remember that the last opcode delayed a pending IRQ or NMI by one cycle.  */
#define OPCODE_DELAYS_INTERRUPT() \
    OPINFO_SET_DELAYS_INTERRUPT(LAST_OPCODE_INFO, 1)

/* Remember that the last opcode changed the I flag from 0 to 1, so we have
   to dispatch an IRQ even if the I flag is 0 when we check it.  */
#define OPCODE_DISABLES_IRQ() \
    OPINFO_SET_DISABLES_IRQ(LAST_OPCODE_INFO, 1)

/* Remember that the last opcode changed the I flag from 1 to 0, so we must
   not dispatch an IRQ even if the I flag is 1 when we check it.  */
#define OPCODE_ENABLES_IRQ() \
    OPINFO_SET_ENABLES_IRQ(LAST_OPCODE_INFO, 1)

#else

/* Info about the last opcode is not needed.  */
#define SET_LAST_OPCODE(x)
#define OPCODE_DELAYS_INTERRUPT()
#define OPCODE_DISABLES_IRQ()
#define OPCODE_ENABLES_IRQ()

#endif

#ifdef LAST_OPCODE_ADDR
#define SET_LAST_ADDR(x) LAST_OPCODE_ADDR = (x)
#else
#error "please define LAST_OPCODE_ADDR"
#endif

/* Export the local version of the registers.  */
#define EXPORT_REGISTERS()     \
  do {                         \
      GLOBAL_REGS.pc = reg_pc; \
      GLOBAL_REGS.a = reg_a;   \
      GLOBAL_REGS.x = reg_x;   \
      GLOBAL_REGS.y = reg_y;   \
      GLOBAL_REGS.sp = reg_sp; \
      GLOBAL_REGS.p = reg_p;   \
      GLOBAL_REGS.n = flag_n;  \
      GLOBAL_REGS.z = flag_z;  \
  } while (0)

/* Import the public version of the registers.  */
#define IMPORT_REGISTERS()     \
  do {                         \
      reg_a = GLOBAL_REGS.a;   \
      reg_x = GLOBAL_REGS.x;   \
      reg_y = GLOBAL_REGS.y;   \
      reg_sp = GLOBAL_REGS.sp; \
      reg_p = GLOBAL_REGS.p;   \
      flag_n = GLOBAL_REGS.n;  \
      flag_z = GLOBAL_REGS.z;  \
      JUMP(GLOBAL_REGS.pc);    \
  } while (0)

/* Stack operations. */

#ifndef PUSH
#define PUSH(val) STORE(reg_sp--, (BYTE)(val))
#endif
#ifndef PULL
#define PULL()    LOAD(++reg_sp)
#endif

#ifdef DEBUG
#define TRACE_NMI() \
    do { if (TRACEFLG) debug_nmi(CPU_INT_STATUS, CLK); } while (0)
#define TRACE_IRQ() \
    do { if (TRACEFLG) debug_irq(CPU_INT_STATUS, CLK); } while (0)
#define TRACE_BRK() do { if (TRACEFLG) debug_text("*** BRK"); } while (0)
#else
#define TRACE_NMI()
#define TRACE_IRQ()
#define TRACE_BRK()
#endif

/* Perform the interrupts in `int_kind'.  If we have both NMI and IRQ,
   execute NMI.  */
/* FIXME: Dummy LOAD() cycles are missing!  */
/* FIXME: Improper BRK handling!  */
/* FIXME: LOCAL_STATUS() should check byte ready first.  */
#define DO_INTERRUPT(int_kind)                                                 \
    do {                                                                       \
        BYTE ik = (int_kind);                                                  \
                                                                               \
        if (ik & (IK_IRQ | IK_IRQPEND | IK_NMI)) {                             \
            if ((ik & IK_NMI)                                                  \
                && interrupt_check_nmi_delay(CPU_INT_STATUS, CLK)) {           \
                TRACE_NMI();                                                   \
                if (monitor_mask[CALLER] & (MI_STEP)) {                        \
                    monitor_check_icount_interrupt();                          \
                }                                                              \
                interrupt_ack_nmi(CPU_INT_STATUS);                             \
                LOCAL_SET_BREAK(0);                                            \
                PUSH(reg_pc >> 8);                                             \
                PUSH(reg_pc & 0xff);                                           \
                PUSH(LOCAL_STATUS());                                          \
                LOCAL_SET_DECIMAL(0);                                          \
                LOCAL_SET_INTERRUPT(1);                                        \
                JUMP(LOAD_ADDR(0xfffa));                                       \
                SET_LAST_OPCODE(0);                                            \
                CLK_ADD(CLK, NMI_CYCLES);                                      \
            } else if ((ik & (IK_IRQ | IK_IRQPEND))                            \
                       && (!LOCAL_INTERRUPT()                                  \
                       || OPINFO_DISABLES_IRQ(LAST_OPCODE_INFO))               \
                       && interrupt_check_irq_delay(CPU_INT_STATUS,            \
                                                    CLK)) {                    \
                TRACE_IRQ();                                                   \
                if (monitor_mask[CALLER] & (MI_STEP)) {                        \
                    monitor_check_icount_interrupt();                          \
                }                                                              \
                interrupt_ack_irq(CPU_INT_STATUS);                             \
                LOCAL_SET_BREAK(0);                                            \
                PUSH(reg_pc >> 8);                                             \
                PUSH(reg_pc & 0xff);                                           \
                PUSH(LOCAL_STATUS());                                          \
                LOCAL_SET_INTERRUPT(1);                                        \
                LOCAL_SET_DECIMAL(0);                                          \
                JUMP(LOAD_ADDR(0xfffe));                                       \
                SET_LAST_OPCODE(0);                                            \
                CLK_ADD(CLK, IRQ_CYCLES);                                      \
            }                                                                  \
        }                                                                      \
        if (ik & (IK_TRAP | IK_RESET)) {                                       \
            if (ik & IK_TRAP) {                                                \
                EXPORT_REGISTERS();                                            \
                interrupt_do_trap(CPU_INT_STATUS, (WORD)reg_pc);               \
                IMPORT_REGISTERS();                                            \
                if (CPU_INT_STATUS->global_pending_int & IK_RESET)             \
                    ik |= IK_RESET;                                            \
            }                                                                  \
            if (ik & IK_RESET) {                                               \
                interrupt_ack_reset(CPU_INT_STATUS);                           \
                cpu_reset();                                                   \
                JUMP(LOAD_ADDR(0xfffc));                                       \
                DMA_ON_RESET;                                                  \
            }                                                                  \
        }                                                                      \
        if (ik & (IK_MONITOR | IK_DMA)) {                                      \
            if (ik & IK_MONITOR) {                                             \
                caller_space = CALLER;                                         \
                if (monitor_force_import(CALLER))                              \
                    IMPORT_REGISTERS();                                        \
                if (monitor_mask[CALLER])                                      \
                    EXPORT_REGISTERS();                                        \
                if (monitor_mask[CALLER] & (MI_BREAK)) {                       \
                    if (monitor_check_breakpoints(CALLER,                      \
                        (WORD)reg_pc)) {                                       \
                        monitor_startup();                                     \
                        IMPORT_REGISTERS();                                    \
                    }                                                          \
                }                                                              \
                if (monitor_mask[CALLER] & (MI_STEP)) {                        \
                    monitor_check_icount((WORD)reg_pc);                        \
                    IMPORT_REGISTERS();                                        \
                }                                                              \
                if (monitor_mask[CALLER] & (MI_WATCH)) {                       \
                    monitor_check_watchpoints(LAST_OPCODE_ADDR, (WORD)reg_pc); \
                    IMPORT_REGISTERS();                                        \
                }                                                              \
            }                                                                  \
            if (ik & IK_DMA) {                                                 \
                EXPORT_REGISTERS();                                            \
                DMA_FUNC;                                                      \
                interrupt_ack_dma(CPU_INT_STATUS);                             \
                IMPORT_REGISTERS();                                            \
                JUMP(reg_pc);                                                  \
            }                                                                  \
        }                                                                      \
    } while (0)

/* ------------------------------------------------------------------------- */

/* Addressing modes.  For convenience, page boundary crossing cycles and
   ``idle'' memory reads are handled here as well. */

#define LOAD_DBR(addr) \
    LOAD_LONG(addr + (reg_dbr * 0x10000))

#define LOAD_BANK0(addr) \
    LOAD_LONG(addr & 0xffff)

#define LOAD_DIRECT_PAGE8(addr) \
    LOAD_BANK0(addr + reg_dpr)

#define LOAD_DIRECT_PAGE16(addr) \
    (LOAD_DIRECT_PAGE8(addr) | (LOAD_DIRECT_PAGE8(addr + 1) << 8))

#define LOAD_DIRECT_PAGE24(addr) \
    (LOAD_DIRECT_PAGE8(addr) | (LOAD_DIRECT_PAGE16(addr + 1) << 8))

#define LOAD_DIRECT_PAGE(addr, bits8)                          \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), ((bits8) \
    ? LOAD_DIRECT_PAGE8(addr)                                  \
    : (CLK_ADD(CLK, 1), LOAD_DIRECT_PAGE16(addr))))

#define LOAD_DIRECT_PAGE_X(addr, bits8)                        \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), ((bits8) \
    ? LOAD_BANK0(addr + reg_dpr + reg_x)                       \
    : (CLK_ADD(CLK, 1), (LOAD_BANK0(addr + reg_dpr + reg_x) | (LOAD_BANK0(addr + reg_dpr + reg_x + 1) << 8)))))

#define LOAD_DIRECT_PAGE_Y(addr, bits8)                        \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), ((bits8) \
    ? LOAD_BANK0(addr + reg_dpr + reg_y)                       \
    : (CLK_ADD(CLK, 1), (LOAD_BANK0(addr + reg_dpr + reg_y) | (LOAD_BANK0(addr + reg_dpr + reg_y + 1) << 8 )))))

#define LOAD_INDIRECT(addr, bits8)                                              \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), CLK_ADD(CLK, 3), ((bits8) \
    ? LOAD_DBR(LOAD_DIRECT_PAGE16(addr))                                        \
    : (CLK_ADD(CLK, 1), (LOAD_DBR(LOAD_DIRECT_PAGE16(addr)) | (LOAD_DBR(LOAD_DIRECT_PAGE16(addr) + 1) << 8)))))

#define LOAD_INDIRECT_X(addr, bits8)                                            \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), CLK_ADD(CLK, 3), ((bits8) \
    ? LOAD_DBR(LOAD_DIRECT_PAGE16(addr + reg_x))                                \
    : (CLK_ADD(CLK, 1), (LOAD_DBR(LOAD_DIRECT_PAGE16(addr + reg_x)) | (LOAD_DBR(LOAD_DIRECT_PAGE16(addr + reg_x) + 1) << 8)))))

#define LOAD_INDIRECT_Y(addr, bits8)                                                            \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)),                                           \
    ((((LOAD_DIRECT_PAGE16(addr) & 0xff) + reg_y) > 0xff) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), \
    CLK_ADD(CLK, 3), ((bits8)                                                                   \
    ? LOAD_LONG((reg_dbr * 0x10000) + LOAD_DIRECT_PAGE16(addr) + reg_y)                         \
    : (CLK_ADD(CLK, 1), (LOAD_LONG((reg_dbr * 0x10000) + LOAD_DIRECT_PAGE16(addr) + reg_y) | (LOAD_LONG((reg_dbr * 0x10000) + LOAD_DIRECT_PAGE16(addr) + reg_y + 1) << 8)))))

#define LOAD_INDIRECT_LONG(addr, bits8)                                         \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), CLK_ADD(CLK, 4), ((bits8) \
    ? LOAD_LONG(LOAD_DIRECT_PAGE24(addr))                                       \
    : (CLK_ADD(CLK, 1), (LOAD_LONG(LOAD_DIRECT_PAGE24(addr)) | (LOAD_LONG(LOAD_DIRECT_PAGE24(addr) + 1) << 8)))))

#define LOAD_INDIRECT_LONG_Y(addr, bits8)                                       \
    (((reg_dpr) ? CLK_ADD(CLK, 1) : CLK_ADD(CLK, 0)), CLK_ADD(CLK, 4), ((bits8) \
    ? LOAD_LONG(LOAD_DIRECT_PAGE24(addr) + reg_y)                               \
    : (CLK_ADD(CLK, 1), (LOAD_LONG(LOAD_DIRECT_PAGE24(addr) + reg_y) | (LOAD_LONG(LOAD_DIRECT_PAGE24(addr) + reg_y + 1) << 8)))))

#define LOAD_ABS(addr, bits8) \
    ((bits8)                  \
    ? LOAD_DBR(addr)          \
    : (CLK_ADD(CLK, 1), (LOAD_DBR(addr) | (LOAD_DBR((addr + 1) & 0xffff) << 8))))

#define LOAD_ABS_X(addr, bits8)                                           \
    ((bits8)                                                              \
    ? (((((addr) & 0xff) + reg_x) > 0xff)                                 \
        ? (LOAD((reg_pbr * 0x10000) + reg_pc + 2),                        \
            CLK_ADD(CLK, CLK_INT_CYCLE),                                  \
            LOAD_DBR(addr + reg_x))                                       \
        : LOAD_DBR(addr + reg_x))                                         \
    : (((((addr) & 0xff) + reg_x) > 0xff)                                 \
        ? (LOAD((reg_pbr * 0x10000) + reg_pc + 2),                        \
            CLK_ADD(CLK, CLK_INT_CYCLE),                                  \
            (LOAD_DBR(addr + reg_x) | (LOAD_DBR(addr + reg_x + 1) << 8))) \
        : (CLK_ADD(CLK, 1), (LOAD_DBR(addr + reg_x) | (LOAD_DBR(addr + reg_x + 1) << 8)))))

#define LOAD_ABS_Y(addr, bits8)                                           \
    ((bits8)                                                              \
    ? (((((addr) & 0xff) + reg_y) > 0xff)                                 \
        ? (LOAD((reg_pbr * 0x10000) + reg_pc + 2),                        \
            CLK_ADD(CLK, CLK_INT_CYCLE),                                  \
            LOAD_DBR(addr + reg_y))                                       \
        : LOAD_DBR(addr + reg_y))                                         \
    : (((((addr) & 0xff) + reg_y) > 0xff)                                 \
        ? (LOAD((reg_pbr * 0x10000) + reg_pc + 2),                        \
            CLK_ADD(CLK, CLK_INT_CYCLE),                                  \
            (LOAD_DBR(addr + reg_y) | (LOAD_DBR(addr + reg_y + 1) << 8))) \
        : (CLK_ADD(CLK, 1), (LOAD_DBR(addr + reg_y) | (LOAD_DBR(addr + reg_y + 1) << 8)))))

#define LOAD_ABS_LONG(addr, bits8) \
    ((bits8)                       \
    ? LOAD_LONG(addr)              \
    : (CLK_ADD(CLK, 1), (LOAD_LONG(addr) | (LOAD_LONG(addr + 1) << 8)))))

#define LOAD_ABS_LONG_X(addr, bits8) \
    ((bits8)                         \
    ? LOAD_LONG(addr + reg_x)        \
    : (CLK_ADD(CLK, 1), (LOAD_LONG(addr + reg_x) | (LOAD_LONG(addr + reg_x + 1) << 8))))

#define LOAD_STACK_REL8(addr) \
    LOAD_BANK0(addr + reg_sp)

#define LOAD_STACK_REL16(addr) \
    LOAD_BANK0(addr + reg_sp) | (LOAD_BANK0(addr + reg_sp + 1) << 8)

#define LOAD_STACK_REL(addr, bits8) \
    ((bits8)                        \
    ? LOAD_STACK_REL8(addr)         \
    : (CLK_ADD(CLK, 1), LOAD_STACK_REL16(addr)))

#define LOAD_STACK_REL_Y(addr, bits8)          \
    ((bits8)                                   \
    ? LOAD_LONG((reg_dbr * 0x10000) + LOAD_STACK_REL16(addr) + reg_y) \
    : (CLK_ADD(CLK, 1), (LOAD_LONG((reg_dbr * 0x10000) + LOAD_STACK_REL16(addr) + reg_y) | (LOAD_LONG((reg_dbr * 0x10000) + LOAD_STACK_REL16(addr) + reg_y + 1) << 8))))

#define STORE_DBR(addr, value)                  \
  do {                                          \
      CLK_ADD(CLK, 1);                          \
      STORE((reg_dbr * 0x10000) + addr, value); \
  } while (0)

#define STORE_LONG(addr, value) \
  do {                          \
      CLK_ADD(CLK, 1);          \
      STORE(addr, value);       \
  } while (0)

#define STORE_BANK0(addr, value)   \
  do {                             \
      CLK_ADD(CLK, 1);             \
      STORE(addr & 0xffff, value); \
  } while (0)

#define STORE_DIRECT_PAGE(addr, value, bits8) \
  do {                                        \
      unsigned int dst = (addr);              \
                                              \
      if (reg_dpr) {                          \
          CLK_ADD(CLK, 1);                    \
          LOAD_BANK0(dst);                    \
          dst = (dst + reg_dpr) & 0xffff;     \
      }                                       \
      STORE_BANK0(dst, value & 0xff);         \
      if (!bits8) {                           \
          STORE_BANK0(dst + 1, value >> 8);   \
      }                                       \
  } while (0)

#define STORE_DIRECT_PAGE_X(addr, value, bits8) \
  do {                                          \
      unsigned int dst = (addr);                \
                                                \
      if (reg_dpr) {                            \
          CLK_ADD(CLK, 1);                      \
          LOAD_BANK0(dst);                      \
          dst = (dst + reg_dpr) & 0xffff;       \
      }                                         \
      CLK_ADD(CLK, 1);                          \
      LOAD_BANK0(dst);                          \
      dst = (dst + reg_x) & 0xffff;             \
      STORE_BANK0(dst, value & 0xff);           \
      if (!bits8) {                             \
          STORE_BANK0(dst + 1, value >> 8);     \
      }                                         \
  } while (0)

#define STORE_DIRECT_PAGE_Y(addr, value, bits8) \
  do {                                          \
      unsigned int dst = (addr);                \
                                                \
      if (reg_dpr) {                            \
          CLK_ADD(CLK, 1);                      \
          LOAD_BANK0(dst);                      \
          dst = (dst + reg_dpr) & 0xffff;       \
      }                                         \
      CLK_ADD(CLK, 1);                          \
      LOAD_BANK0(dst);                          \
      dst = (dst + reg_y) & 0xffff;             \
      STORE_BANK0(dst, value & 0xff);           \
      if (!bits8) {                             \
          STORE_BANK0(dst + 1, value >> 8);     \
      }                                         \
  } while (0)

#define STORE_INDIRECT(addr, value, bits8) \
  do {                                     \
      unsigned int dst = (addr);           \
      unsigned int ea;                     \
                                           \
      if (reg_dpr) {                       \
          CLK_ADD(CLK, 1);                 \
          LOAD_BANK0(dst);                 \
          dst = (dst + reg_dpr) & 0xffff;  \
      }                                    \
      CLK_ADD(CLK, 1);                     \
      ea = LOAD_BANK0(dst);                \
      CLK_ADD(CLK, 1);                     \
      ea |= (LOAD_BANK0(dst + 1) << 8);    \
      STORE_DBR(ea, value & 0xff);         \
      if (!bits8) {                        \
          STORE_DBR(ea + 1, value >> 8);   \
      }                                    \
  } while (0)

#define STORE_INDIRECT_X(addr, value, bits8) \
  do {                                       \
      unsigned int dst = (addr);             \
      unsigned int ea;                       \
                                             \
      if (reg_dpr) {                         \
          CLK_ADD(CLK, 1);                   \
          LOAD_BANK0(dst);                   \
          dst = (dst + reg_dpr) & 0xffff;    \
      }                                      \
      CLK_ADD(CLK, 1);                       \
      LOAD_BANK0(dst);                       \
      dst = (dst + x) & 0xffff;              \
      CLK_ADD(CLK, 1);                       \
      ea = LOAD_BANK0(dst);                  \
      CLK_ADD(CLK, 1);                       \
      ea |= (LOAD_BANK0(dst + 1) << 8);      \
      STORE_DBR(ea, value & 0xff);           \
      if (!bits8) {                          \
          STORE_DBR(ea + 1, value >> 8);     \
      }                                      \
  } while (0)

#define STORE_INDIRECT_Y(addr, value, bits8)                    \
  do {                                                          \
      unsigned int dst = (addr);                                \
      unsigned int ea;                                          \
                                                                \
      if (reg_dpr) {                                            \
          CLK_ADD(CLK, 1);                                      \
          LOAD_BANK0(dst, 1);                                   \
          dst = (dst + reg_dpr) & 0xffff;                       \
      }                                                         \
      CLK_ADD(CLK, 1);                                          \
      ea = LOAD_BANK0(dst);                                     \
      CLK_ADD(CLK, 1);                                          \
      ea |= (LOAD_BANK0(dst + 1) << 8);                         \
      CLK_ADD(CLK, 1);                                          \
      LOAD_LONG((reg_pbr * 0x10000) + ea, 1);                   \
      ea += reg_y;                                              \
      STORE_LONG((reg_pbr * 0x10000) + ea, value & 0xff);       \
      if (!bits8) {                                             \
          STORE_LONG((reg_pbr * 0x10000) + ea + 1, value >> 8); \
      }                                                         \
  } while (0);

#define STORE_INDIRECT_LONG(addr, value, bits8) \
  do {                                          \
      unsigned int dst = (addr);                \
      unsigned int ea;                          \
                                                \
      if (reg_dpr) {                            \
          CLK_ADD(CLK, 1);                      \
          LOAD_BANK0(dst);                      \
          dst = (dst + reg_dpr) & 0xffff;       \
      }                                         \
      CLK_ADD(CLK, 1);                          \
      ea = LOAD_BANK0(dst);                     \
      CLK_ADD(CLK, 1);                          \
      ea |= (LOAD_BANK0(dst + 1) << 8);         \
      CLK_ADD(CLK, 1);                          \
      ea |= (LOAD_BANK0(dst + 2) << 16);        \
      STORE_LONG(ea, value & 0xff);             \
      if (!bits8) {                             \
          STORE_LONG(ea + 1, value >> 8);       \
      }                                         \
  } while (0)

#define STORE_INDIRECT_LONG_Y(addr, value, bits8) \
  do {                                            \
      unsigned int dst = (addr);                  \
      unsigned int ea;                            \
                                                  \
      if (reg_dpr) {                              \
          CLK_ADD(CLK, 1);                        \
          LOAD_BANK0(dst, 1);                     \
          dst = (dst + reg_dpr) & 0xffff;         \
      }                                           \
      CLK_ADD(CLK, 1);                            \
      ea = LOAD_BANK0(dst);                       \
      CLK_ADD(CLK, 1);                            \
      ea |= (LOAD_BANK0(dst + 1) << 8);           \
      CLK_ADD(CLK, 1);                            \
      ea |= (LOAD_BANK0(dst + 2) << 16);          \
      ea += reg_y;                                \
      STORE_LONG(ea, value & 0xff);               \
      if (!bits8) {                               \
          STORE_LONG(ea + 1, value >> 8);         \
      }                                           \
  } while (0)

#define STORE_ABS(addr, value, bits8)                 \
  do {                                                \
      STORE_DBR(addr, value & 0xff);                  \
      if (!bits8) {                                   \
          STORE_DBR((addr + 1) & 0xffff, value >> 8); \
      }                                               \
  } while (0)

#define STORE_ABS_DPR(addr, value, bits8) \
  STORE_ABS((addr + reg_dpr) & 0xffff, value, bits8)

#define STORE_ABS_X_RRW(addr, value, bits8) \
  STORE_ABS((addr + reg_x) & 0xffff, value, bits8)

#define STORE_ABS_X(addr, value, bits8)   \
  do {                                    \
      unsigned int dst = (addr);          \
                                          \
      CLK_ADD(CLK, 1);                    \
      LOAD_DBR(dst);                      \
      dst += reg_x;                       \
      STORE_DBR(dst, value & 0xff);       \
      if (!bits8) {                       \
          STORE_DBR(dst + 1, value >> 8); \
      }                                   \
  } while (0)

#define STORE_ABS_Y(addr, value, bits8)   \
  do {                                    \
      unsigned int dst = (addr);          \
                                          \
      CLK_ADD(CLK, 1);                    \
      LOAD_DBR(dst);                      \
      dst += reg_y;                       \
      STORE_DBR(dst, value & 0xff);       \
      if (!bits8) {                       \
          STORE_DBR(dst + 1, value >> 8); \
      }                                   \
  } while (0)

#define STORE_ABS_LONG(addr, value, bits8) \
  do {                                     \
      STORE_LONG(dst, value & 0xff);       \
      if (!bits8) {                        \
          STORE_LONG(dst + 1, value >> 8); \
      }                                    \
  } while (0)

#define STORE_ABS_LONG_X(addr, value, bits8)        \
  do {                                              \
      dst += reg_x;                                 \
      STORE_LONG(addr + reg_x, value & 0xff);       \
      if (!bits8) {                                 \
          STORE_LONG(addr + reg_x + 1, value >> 8); \
      }                                             \
  } while (0)

#define STORE_STACK_REL(addr, value, bits8) \
  do {                                      \
      unsigned int dst = (addr);            \
      unsigned int ea;                      \
                                            \
      CLK_ADD(CLK, 1);                      \
      LOAD_BANK0(dst);                      \
      dst = (dst + reg_sp) & 0xffff;        \
      STORE_BANK0(dst, value & 0xff);       \
      if (!bits8) {                         \
          STORE_BANK0(dst + 1, value >> 8); \
      }                                     \
  } while (0)

#define STORE_STACK_REL_Y(addr, value, bits8)                   \
  do {                                                          \
      unsigned int dst = (addr);                                \
      unsigned int ea;                                          \
                                                                \
      CLK_ADD(CLK, 1);                                          \
      LOAD_BANK0(dst);                                          \
      dst = (dst + reg_sp) & 0xffff;                            \
      CLK_ADD(CLK, 1);                                          \
      ea = LOAD_BANK0(dst);                                     \
      CLK_ADD(CLK, 1);                                          \
      ea |= (LOAD_BANK0(dst + 1) << 8);                         \
      CLK_ADD(CLK, 1);                                          \
      LOAD_LONG((reg_dbr * 0x10000) + ea);                      \
      ea += reg_y;                                              \
      STORE_LONG((reg_dbr * 0x10000) + ea, value & 0xff);       \
      if (!bits8) {                                             \
          STORE_LONG((reg_dbr * 0x10000) + ea + 1, value >> 8); \
      }                                                         \
  } while (0)

#define INC_PC(value)   (reg_pc += (value))

/* ------------------------------------------------------------------------- */

/* Opcodes.  */

/*
   A couple of caveats about PC:

   - the VIC-II emulation requires PC to be incremented before the first
     write access (this is not (very) important when writing to the zero
     page);

   - `p0', `p1' and `p2' can only be used *before* incrementing PC: some
     machines (eg. the C128) might depend on this.
*/

#define ADC(value, clk_inc, pc_inc)                                                                  \
  do {                                                                                               \
      unsigned int tmp_value;                                                                        \
      unsigned int tmp;                                                                              \
                                                                                                     \
      tmp_value = (value);                                                                           \
      CLK_ADD(CLK, (clk_inc));                                                                       \
                                                                                                     \
      if (LOCAL_65816_M()) {                                                                         \
          if (LOCAL_DECIMAL()) {                                                                     \
              tmp = (reg_a & 0xff) - (tmp_value & 0xf) + LOCAL_CARRY() - 1;                          \
              if ((tmp & 0xf) > (reg_a & 0xf)) {                                                     \
                  tmp -= 6;                                                                          \
              }                                                                                      \
              tmp -= (tmp_value & 0xf0);                                                             \
              if ((tmp & 0xf0) > (reg_a & 0xf0)) {                                                   \
                  tmp -= 60;                                                                         \
              }                                                                                      \
              LOCAL_SET_OVERFLOW(!(tmp > (reg_a & 0xff)));                                           \
              LOCAL_SET_CARRY(!(tmp > (reg_a & 0xff));                                               \
              LOCAL_SET_NZ(tmp & 0xff);                                                              \
          } else {                                                                                   \
              tmp = tmp_value + (reg_a & 0xff) + LOCAL_CARRY();                                      \
              LOCAL_SET_NZ(tmp & 0xff);                                                              \
              LOCAL_SET_OVERFLOW(!(((reg_a & 0xff) ^ tmp_value) & 0x80)                              \
                                  && (((reg_a & 0xff) ^ tmp) & 0x80));                               \
              LOCAL_SET_CARRY(tmp > 0xff);                                                           \
          }                                                                                          \
          reg_a = (reg_a & 0xff00) | (tmp & 0xff);                                                   \
      } else {                                                                                       \
          if (!LOCAL_DECIMAL()) {                                                                    \
              tmp = reg_a + tmp_value + LOCAL_CARRY();                                               \
          } else {                                                                                   \
              tmp = (reg_a & 0xf) + (tmp_value & 0xf) + LOCAL_CARRY;                                 \
              if (tmp > 9) {                                                                         \
                  tmp += 6;                                                                          \
              }                                                                                      \
              LOCAL_SET_CARRY(tmp > 0xf);                                                            \
              tmp = (reg_a & 0xf0) + (tmp_value & 0xf0) + (LOCAL_CARRY() << 5) + (tmp & 0xf);        \
              if (tmp > 0x9f) {                                                                      \
                  tmp += 0x60;                                                                       \
              }                                                                                      \
              LOCAL_SET_CARRY(tmp > 0xff);                                                           \
              tmp = (reg_a & 0xf00) + (tmp_value & 0xf00) + (LOCAL_CARRY() << 8) + (tmp & 0xff);     \
              if (tmp > 0x9ff) {                                                                     \
                  tmp += 0x600;                                                                      \
              }                                                                                      \
              LOCAL_SET_CARRY(tmp > 0xfff);                                                          \
              tmp = (reg_a & 0xf000) + (tmp_value & 0xf000) + (LOCAL_CARRY() << 12) + (tmp & 0xfff); \
          }                                                                                          \
          LOCAL_SET_OVERFLOW((~(reg_a ^ tmp_value) & (reg_a ^ tmp) & 0x8000) >> 8);                  \
          if (LOCAL_DECIMAL() && (tmp > 0x9fff)) {                                                   \
              tmp += 0x6000;                                                                         \
          }                                                                                          \
          LOCAL_SET_CARRY(result > 0xffff);                                                          \
          LOCAL_SET_NZ(tmp & 0xffff, 0);                                                             \
          reg_a = tmp & 0xffff;                                                                      \
      }                                                                                              \
      INC_PC(pc_inc);                                                                                \
  } while (0)

#define AND(value, clk_inc, pc_inc)                            \
  do {                                                         \
      if (LOCAL_65816_M()) {                                   \
          reg_a = (reg_a & 0xff00) | ((reg_a & value) & 0xff); \
      } else {                                                 \
          reg_a &= value;                                      \
      }                                                        \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                    \
      CLK_ADD(CLK, (clk_inc));                                 \
      INC_PC(pc_inc);                                          \
  } while (0)

#define ASL(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp_value, tmp_addr;                   \
                                                          \
      tmp_addr = (addr);                                  \
      tmp_value = load_func(tmp_addr);                    \
      LOCAL_SET_CARRY(tmp_value & 0x80);                  \
      tmp_value = (tmp_value << 1) & 0xff;                \
      LOCAL_SET_NZ(tmp_value);                            \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp_value, LOCAL_65816_M());   \
  } while (0)

#define ASL_A()                                             \
  do {                                                      \
      if (LOCAL_65816_M()) {                                \
          LOCAL_SET_CARRY(reg_a & 0x80);                    \
          reg_a = (reg_a & 0xff00) | ((reg_a << 1) & 0xff); \
      } else {                                              \
          LOCAL_SET_CARRY(reg_a & 0x8000);                  \
          reg_a <<= reg_a;                                  \
      }                                                     \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                 \
      INC_PC(1);                                            \
  } while (0)

#define BIT_IMM(value, clk_inc, pc_inc)            \
  do {                                             \
      unsigned int tmp;                            \
                                                   \
      tmp = (value);                               \
      CLK_ADD(CLK, clk_inc);                       \
      if (LOCAL_65816_M()) {                       \
          LOCAL_SET_ZERO(!(tmp & (reg_a & 0xff))); \
      } else {                                     \
          LOCAL_SET_ZERO(!(tmp & reg_a));          \
      }                                            \
      INC_PC(pc_inc);                              \
  } while (0)

#define BIT(value, clk_inc, pc_inc)                \
  do {                                             \
      unsigned int tmp;                            \
                                                   \
      tmp = (value);                               \
      CLK_ADD(CLK, clk_inc);                       \
      if (LOCAL_65816_M()) {                       \
          LOCAL_SET_SIGN(tmp & 0x80);              \
          LOCAL_SET_OVERFLOW(tmp & 0x40);          \
          LOCAL_SET_ZERO(!(tmp & (reg_a & 0xff))); \
      } else {                                     \
          LOCAL_SET_SIGN(tmp & 0x8000);            \
          LOCAL_SET_OVERFLOW(tmp & 0x4000);        \
          LOCAL_SET_ZERO(!(tmp & reg_a));          \
      }                                            \
      INC_PC(pc_inc);                              \
  } while (0)

#define BRANCH(cond, value)                                  \
  do {                                                       \
      unsigned int dest_addr = 0;                            \
      INC_PC(2);                                             \
                                                             \
      if (cond) {                                            \
          dest_addr = reg_pc + (signed char)(value);         \
                                                             \
          LOAD(reg_pc);                                      \
          CLK_ADD(CLK, CLK_BRANCH2);                         \
          if (((reg_pc ^ dest_addr) & 0xff00) && reg_emul) { \
              LOAD((reg_pc & 0xff00) | (dest_addr & 0xff));  \
              CLK_ADD(CLK, CLK_BRANCH2);                     \
          } else {                                           \
              OPCODE_DELAYS_INTERRUPT();                     \
          }                                                  \
          JUMP(dest_addr & 0xffff);                          \
      }                                                      \
  } while (0)

#define BRANCH_LONG(value)                        \                                                                      
  do {                                            \                                                                      
      unsigned int dest_addr = 0;                 \                                                                      
      INC_PC(3);                                  \
                                                  \
      dest_addr = reg_pc + (signed short)(value); \
                                                  \
      LOAD(reg_pc);                               \
      CLK_ADD(CLK, CLK_BRANCH2);                  \
      OPCODE_DELAYS_INTERRUPT();                  \
      JUMP(dest_addr & 0xffff);                   \
  } while (0)

#define BRK()                         \
  do {                                \
      EXPORT_REGISTERS();             \
      CLK_ADD(CLK, CLK_BRK);          \
      TRACE_BRK();                    \
      INC_PC(2);                      \
      if (reg_emul) {                 \
          LOCAL_SET_BREAK(1);         \
      } else {                        \
          CLK_ADD(CLK, 1);            \
          PUSH(reg_pbr);              \
      }                               \
      PUSH(reg_pc >> 8);              \
      PUSH(reg_pc & 0xff);            \
      if (reg_emul) {                 \
          PUSH(LOCAL_STATUS());       \
      } else {                        \
          PUSH(LOCAL_65816_STATUS()); \
      }                               \
      LOCAL_SET_DECIMAL(0);           \
      LOCAL_SET_INTERRUPT(1);         \
      if (reg_emul) {                 \
          JUMP(LOAD_BANK0(0xfffe));   \
      } else {                        \
          JUMP(LOAD_BANK0(0xffe6));   \
      }                               \
  } while (0)

#define CLC()             \
  do {                    \
      INC_PC(1);          \
      LOCAL_SET_CARRY(0); \
  } while (0)

#define CLD()               \
  do {                      \
      INC_PC(1);            \
      LOCAL_SET_DECIMAL(0); \
  } while (0)

#define CLI()                   \
  do {                          \
      INC_PC(1);                \
      if (LOCAL_INTERRUPT()) {  \
          OPCODE_ENABLES_IRQ(); \
      }                         \
      LOCAL_SET_INTERRUPT(0);   \
  } while (0)

#define CLV()                \
  do {                       \
      INC_PC(1);             \
      LOCAL_SET_OVERFLOW(0); \
  } while (0)

#define CMP(value, clk_inc, pc_inc)       \
  do {                                    \
      unsigned int tmp;                   \
                                          \
      if (LOCAL_65816_M()) {              \
          tmp = (reg_a & 0xff) - value;   \
          LOCAL_SET_CARRY(tmp < 0x100);   \
          LOCAL_SET_NZ(tmp & 0xff, 1);    \
      } else {                            \
          tmp = reg_a - value;            \
          LOCAL_SET_CARRY(tmp < 0x10000); \
          LOCAL_SET_NZ(tmp & 0xffff, 0);  \
      }                                   \
      CLK_ADD(CLK, (clk_inc));            \
      INC_PC(pc_inc);                     \
  } while (0)

#define CPX(value, clk_inc, pc_inc)       \
  do {                                    \
      unsigned int tmp;                   \
                                          \
      tmp = reg_x - value;                \
      if (LOCAL_65816_X()) {              \
          LOCAL_SET_CARRY(tmp < 0x100);   \
          LOCAL_SET_NZ(tmp & 0xff, 1);    \
      } else {                            \
          LOCAL_SET_CARRY(tmp < 0x10000); \
          LOCAL_SET_NZ(tmp & 0xffff, 0);  \
      }                                   \
      CLK_ADD(CLK, (clk_inc));            \
      INC_PC(pc_inc);                     \
  } while (0)

#define CPY(value, clk_inc, pc_inc)       \
  do {                                    \
      unsigned int tmp;                   \
                                          \
      tmp = reg_y - value;                \
      if (LOCAL_65816_X()) {              \
          LOCAL_SET_CARRY(tmp < 0x100);   \
          LOCAL_SET_NZ(tmp & 0xff, 1);    \
      } else {                            \
          LOCAL_SET_CARRY(tmp < 0x10000); \
          LOCAL_SET_NZ(tmp & 0xffff, 0);  \
      }                                   \
      CLK_ADD(CLK, (clk_inc));            \
      INC_PC(pc_inc);                     \
  } while (0)

#define DEA()                                              \
  do {                                                     \
      if (LOCAL_65816_M()) {                               \
          reg_a = (reg_a & 0xff00) | ((reg_a - 1) & 0xff); \
      } else {                                             \
          reg_a--;                                         \
      }                                                    \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                \
      INC_PC(1);                                           \
  } while (0)

#define DEC(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp, tmp_addr;                         \
                                                          \
      tmp_addr = (addr);                                  \
      tmp = load_func(tmp_addr);                          \
      tmp = (tmp - 1) & 0xff;                             \
      LOCAL_SET_NZ(tmp);                                  \
      INC_PC(pc_inc);                                     \
      store_func(tmp_addr, tmp, (clk_inc));               \
  } while (0)

#define DEX()                               \
  do {                                      \
      reg_x--;                              \
      if (LOCAL_65816_X()) {                \
          reg_x &= 0xff;                    \
      }                                     \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define DEY()                               \
  do {                                      \
      reg_y--;                              \
      if (LOCAL_65816_X()) {                \
          reg_y &= 0xff;                    \
      }                                     \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define EOR(value, clk_inc, pc_inc)                            \
  do {                                                         \
      if (LOCAL_65816_M()) {                                   \
          reg_a = (reg_a & 0xff00) | ((reg_a ^ value) & 0xff); \
      } else {                                                 \
          reg_a = reg_a ^ value;                               \
      }                                                        \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                    \
      CLK_ADD(CLK, (clk_inc));                                 \
      INC_PC(pc_inc);                                          \
  } while (0)

#define INA()                                              \
  do {                                                     \
      if (LOCAL_65816_M()) {                               \
          reg_a = (reg_a & 0xff00) | ((reg_a + 1) & 0xff); \
      } else {                                             \
          reg_a++;                                         \
      }                                                    \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                \
      INC_PC(1);                                           \
  } while (0)

#define INC(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp, tmp_addr;                         \
                                                          \
      tmp_addr = (addr);                                  \
      tmp = (load_func(tmp_addr) + 1) & 0xff;             \
      LOCAL_SET_NZ(tmp);                                  \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp, LOCAL_65816_M());         \
  } while (0)

#define INX()                               \
  do {                                      \
      reg_x++;                              \
      if (LOCAL_65816_X()) {                \
          reg_x &= 0xff;                    \
      }                                     \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define INY()                               \
  do {                                      \
      reg_y++;                              \
      if (LOCAL_65816_X()) {                \
          reg_y &= 0xff;                    \
      }                                     \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

/* The 0x02 NOP opcode is also used to patch the ROM.  The function trap_handler()
   returns nonzero if this is not a patch, but a `real' NOP instruction. */

#define NOP_02()                                                \
  do {                                                          \
      DWORD trap_result;                                        \
      EXPORT_REGISTERS();                                       \
      if (!ROM_TRAP_ALLOWED()                                   \
          || (trap_result = ROM_TRAP_HANDLER()) == (DWORD)-1) { \
          NOOP_IMM(2);                                          \
      } else {                                                  \
          if (trap_result) {                                    \
             REWIND_FETCH_OPCODE(CLK, 2);                       \
             SET_OPCODE(trap_result);                           \
             IMPORT_REGISTERS();                                \
             goto trap_skipped;                                 \
          } else {                                              \
             IMPORT_REGISTERS();                                \
          }                                                     \
      }                                                         \
  } while (0)
  
#define JMP(addr) \
  do {            \
      JUMP(addr); \
  } while (0)

#define JMP_IND()                                      \
  do {                                                 \
      WORD dest_addr;                                  \
      dest_addr = LOAD_PBR(p2);                        \
      CLK_ADD(CLK, 1);                                 \
      dest_addr |= (LOAD_PBR((p2 + 1) & 0xffff) << 8); \
      CLK_ADD(CLK, 1);                                 \
      JUMP(dest_addr);                                 \
  } while (0)

#define JMP_IND_LONG()                                   \
  do {                                                   \
      WORD dest_addr;                                    \
      dest_addr = LOAD_BANK0(p2);                        \
      CLK_ADD(CLK, 1);                                   \
      dest_addr |= (LOAD_BANK0((p2 + 1) & 0xffff) << 8); \
      CLK_ADD(CLK, 1);                                   \
      reg_pbr = LOAD_BANK0((p2 + 2) & 0xffff);           \
      CLK_ADD(CLK, 1);                                   \
      JUMP(dest_addr);                                   \
  } while (0)

#define JMP_IND_X()                                            \
  do {                                                         \
      WORD dest_addr;                                          \
      CLK_ADD(CLK, 1);                                         \
      dest_addr = LOAD_PBR((p2 + reg_x) & 0xffff);             \
      CLK_ADD(CLK, 1);                                         \
      dest_addr |= (LOAD_PBR((p2 + reg_x + 1) & 0xffff) << 8); \
      CLK_ADD(CLK, 1);                                         \
      JUMP(dest_addr);                                         \
  } while (0)

#define JMP_LONG(addr)      \
  do {                      \
      reg_pbr = addr >> 16; \
      JUMP(addr & 0xffff);  \
  } while (0)

#define JSR()                                \
  do {                                       \
      unsigned int tmp_addr;                 \
                                             \
      INC_PC(2);                             \
      CLK_ADD(CLK, 2);                       \
      PUSH(((reg_pc) >> 8) & 0xff);          \
      PUSH((reg_pc) & 0xff);                 \
      tmp_addr = (p1 | (LOAD(reg_pc) << 8)); \
      CLK_ADD(CLK, CLK_JSR_INT_CYCLE);       \
      JUMP(tmp_addr);                        \
  } while (0)

#define JSR_IND_X()                               \
  do {                                            \
      unsigned int tmp_addr;                      \
                                                  \
      INC_PC(2)                                   \
      CLK_ADD(CLK, 2);                            \
      PUSH(((reg_pc) >> 8) & 0xff);               \
      PUSH((reg_pc) & 0xff);                      \
      CLK_ADD(CLK, 1);                            \
      tmp_addr = LOAD(p2);                        \
      CLK_ADD(CLK, 1);                            \
      tmp_addr |= (LOAD((p2 + 1) & 0xffff) << 8); \
      CLK_ADD(CLK, 1);                            \
      JUMP(tmp_addr);                             \
  } while (0)

#define JSR_LONG()                     \
  do {                                 \
      unsigned int tmp_addr;           \
                                       \
      INC_PC(3);                       \
      CLK_ADD(CLK, 3);                 \
      PUSH(reg_pbr);                   \
      PUSH(((reg_pc) >> 8) & 0xff);    \
      PUSH((reg_pc) & 0xff);           \
      tmp_addr = p2;                   \
      CLK_ADD(CLK, CLK_JSR_INT_CYCLE); \
      reg_pbr = (p3 >> 16);            \
      JUMP(tmp_addr);                  \
  } while (0)

#define LDA(value, clk_inc, pc_inc)                  \
  do {                                               \
      if (LOCAL_65816_M()) {                         \
          reg_a = (reg_a & 0xff00) | (value & 0xff); \
      } else {                                       \
          reg_a = value;                             \
      }                                              \
      CLK_ADD(CLK, (clk_inc));                       \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());          \
      INC_PC(pc_inc);                                \
  } while (0)

#define LDX(value, clk_inc, pc_inc)         \
  do {                                      \
      reg_x = value;                        \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      CLK_ADD(CLK, (clk_inc));              \
      INC_PC(pc_inc);                       \
  } while (0)

#define LDY(value, clk_inc, pc_inc)         \
  do {                                      \
      reg_y = value;                        \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      CLK_ADD(CLK, (clk_inc));              \
      INC_PC(pc_inc);                       \
  } while (0)

#define LSR(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp, tmp_addr;                         \
                                                          \
      tmp_addr = (addr);                                  \
      tmp = load_func(tmp_addr);                          \
      LOCAL_SET_CARRY(tmp & 1);                           \
      tmp >>= 1;                                          \
      LOCAL_SET_NZ(tmp, LOCAL_65816_M());                 \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp, LOCAL_65816_M());         \
  } while (0)

#define LSR_A()                                    \
  do {                                             \
      LOCAL_SET_CARRY(reg_a & 1);                  \
      if (LOCAL_65816_M()) {                       \
          reg_a = (reg_a & 0xff00) | (reg_a >> 1); \
      } else {                                     \
          reg_a = reg_a >> 1;                      \
      }                                            \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());        \
      INC_PC(1);                                   \
  } while (0)

#define ORA(value, clk_inc, pc_inc)                            \
  do {                                                         \
      if (LOCAL_65816_M()) {                                   \
          reg_a = (reg_a & 0xff00) | ((reg_a | value) & 0xff); \
      } else {                                                 \
          reg_a |= value;                                      \
      }                                                        \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                    \
      CLK_ADD(CLK, (clk_inc));                                 \
      INC_PC(pc_inc);                                          \
  } while (0)

#define NOOP(clk_inc, pc_inc) \
    (CLK_ADD(CLK, (clk_inc)), INC_PC(pc_inc))

#define NOOP_IMM(pc_inc) INC_PC(pc_inc)

#define NOP()  NOOP_IMM(1)

#define PEA(value)        \
  do {                    \
      CLK_ADD(CLK, 1);    \
      PUSH(value >> 8);   \
      CLK_ADD(CLK, 1);    \
      PUSH(value & 0xff); \
      INC_PC(3);          \
  } while (0)

#define PEI(value)         \
  do {                     \
      if (reg_dpr) {       \
          CLK_ADD(CLK, 1); \
      }                    \
      CLK_ADD(CLK, 1);     \
      PUSH(value >> 8);    \
      CLK_ADD(CLK, 1);     \
      PUSH(value & 0xff);  \
      INC_PC(2);           \
  } while (0)

#define PER(value)                                \
  do {                                            \
      WORD dest_addr = 0;                         \
                                                  \
      dest_addr = reg_pc + (signed short)(value); \
      CLK_ADD(CLK, 2);                            \
      PUSH(dest_addr >> 8);                       \
      CLK_ADD(CLK, 1);                            \
      PUSH(dest_addr & 0xff);                     \
      INC_PC(3);                                  \
  } while (0)

#define PHA()                           \
  do {                                  \
      if (!LOCAL_65816_M()) {           \
          CLK_ADD(CLK, CLK_STACK_PUSH); \
          PUSH(reg_a >> 0xff);          \
      }                                 \
      CLK_ADD(CLK, CLK_STACK_PUSH);     \
      PUSH(reg_a & 0xff);               \
      INC_PC(1);                        \
  } while (0)

#define PHB()                       \
  do {                              \
      CLK_ADD(CLK, CLK_STACK_PUSH); \
      PUSH(reg_dbr);                \
      INC_PC(1);                    \
  } while (0)

#define PHD()                       \
  do {                              \
      CLK_ADD(CLK, CLK_STACK_PUSH); \
      PUSH(reg_dpr >> 8);           \
      CLK_ADD(CLK, CLK_STACK_PUSH); \
      PUSH(reg_dpr & 0xff);         \
      INC_PC(1);                    \
  } while (0)

#define PHK()                       \
  do {                              \
      CLK_ADD(CLK, CLK_STACK_PUSH); \
      PUSH(reg_pbr);                \
      INC_PC(1);                    \
  } while (0)

#define PHP()                             \
  do {                                    \
      CLK_ADD(CLK, CLK_STACK_PUSH);       \
      if (reg_emul) {                     \
          PUSH(LOCAL_STATUS() | P_BREAK); \
      } else {                            \
          PUSH(LOCAL_65816_STATUS());     \
      }                                   \
      INC_PC(1);                          \
  } while (0)

#define PHX()                           \
  do {                                  \
      if (!LOCAL_65816_X()) {           \
          CLK_ADD(CLK, CLK_STACK_PUSH); \
          PUSH(reg_x >> 8);             \
      }                                 \
      CLK_ADD(CLK, CLK_STACK_PUSH);     \
      PUSH(reg_x & 0xff);               \
      INC_PC(1);                        \
  } while (0)

#define PHY()                           \
  do {                                  \
      if (!LOCAL_65816_X()) {           \
          CLK_ADD(CLK, CLK_STACK_PUSH); \
          PUSH(reg_y >> 8);             \
      }                                 \
      CLK_ADD(CLK, CLK_STACK_PUSH);     \
      PUSH(reg_y & 0xff);               \
      INC_PC(1);                        \
  } while (0)

#define PLA()                                     \
  do {                                            \
      CLK_ADD(CLK, CLK_STACK_PULL);               \
      reg_a = (reg_a & 0xff00) | PULL();          \
      if (!LOCAL_65816_M()) {                     \
          CLK_ADD(CLK, 1);                        \
          reg_a = (reg_a & 0xff) | (PULL() << 8); \
      }                                           \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());       \
      INC_PC(1);                                  \
  } while (0)

#define PLB()                       \
  do {                              \
      CLK_ADD(CLK, CLK_STACK_PULL); \
      reg_dbr = PULL();             \
      LOCAL_SET_NZ(reg_dbr, 1);     \
      INC_PC(1);                    \
  } while (0)

#define PLD()                       \
  do {                              \
      CLK_ADD(CLK, CLK_STACK_PULL); \
      reg_dpr = PULL();             \
      CLK_ADD(CLK, 1);              \
      reg_dpr |= (PULL() << 8);     \
      LOCAL_SET_NZ(reg_dpr, 0);     \
      INC_PC(1);                    \
  } while (0)

#define PLP()                                               \
  do {                                                      \
      BYTE s = PULL();                                      \
                                                            \
      if (!(s & P_INTERRUPT) && LOCAL_INTERRUPT()) {        \
          OPCODE_ENABLES_IRQ();                             \
      } else if ((s & P_INTERRUPT) && !LOCAL_INTERRUPT()) { \
          OPCODE_DISABLES_IRQ();                            \
      }                                                     \
      CLK_ADD(CLK, CLK_STACK_PULL);                         \
      LOCAL_SET_STATUS(s);                                  \
      INC_PC(1);                                            \
  } while (0)

#define PLX()                               \
  do {                                      \
      CLK_ADD(CLK, CLK_STACK_PULL);         \
      reg_x = PULL();                       \
      if (!LOCAL_65816_X()) {               \
          CLK_ADD(CLK, 1);                  \
          reg_x |= (PULL() << 8);           \
      }                                     \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define PLY()                               \
  do {                                      \
      CLK_ADD(CLK, CLK_STACK_PULL);         \
      reg_y = PULL();                       \
      if (!LOCAL_65816_X()) {               \
          CLK_ADD(CLK, 1);                  \
          reg_Y |= (PULL() << 8);           \
      }                                     \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define REP(mask, v)                    \
  do {                                  \
      if (mask & 0x80) {                \
          LOCAL_SET_SIGN(v);            \
      }                                 \
      if (mask & 0x40) {                \
          LOCAL_SET_OVERFLOW(v);        \
      }                                 \
      if ((mask & 0x20) && !reg_emul) { \
          LOCAL_SET_65816_M(v);         \
      }                                 \
      if ((mask & 0x10) && !reg_emul) { \
          LOCAL_SET_65816_X(v);         \
      }                                 \
      if (mask & 0x08) {                \
          LOCAL_SET_DECIMAL(v);         \
      }                                 \
      if (mask & 0x04) {                \
          LOCAL_SET_INTERRUPT(v);       \
      }                                 \
      if (mask & 0x02) {                \
          LOCAL_SET_ZERO(v);            \
      }                                 \
      if (mask & 0x01) {                \
          LOCAL_SET_CARRY(v);           \
      }                                 \
      CLK_ADD(CLK, 1);                  \
      INC_PC(1);                        \
  } while (0)

#define REP(mask) REPSEP(mask, 0)

#define ROL(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp, tmp_addr;                         \
                                                          \
      tmp_addr = (addr);                                  \
      tmp = load_func(tmp_addr);                          \
      tmp = (tmp << 1) | LOCAL_CARRY();                   \
      if (LOCAL_65816_M()) {                              \
          LOCAL_SET_CARRY(tmp & 0x100);                   \
          tmp &= 0xff;                                    \
      } else {                                            \
          LOCAL_SET_CARRY(tmp & 0x10000);                 \
          tmp &= 0xffff;                                  \
      }                                                   \
      LOCAL_SET_NZ(tmp, LOCAL_65816_M());                 \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp, LOCAL_65816_M());         \
  } while (0)

#define ROL_A()                                                      \
  do {                                                               \
      unsigned int tmp = reg_a << 1;                                 \
                                                                     \
      reg_a = tmp | LOCAL_CARRY();                                   \
      if (LOCAL_65816_M()) {                                         \
          reg_a = (reg_a & 0xff00) | ((tmp | LOCAL_CARRY()) & 0xff); \
          LOCAL_SET_CARRY(tmp & 0x100);                              \
      } else {                                                       \
          reg_a = tmp | LOCAL_CARRY();                               \
          LOCAL_SET_CARRY(tmp & 0x10000);                            \
      }                                                              \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                          \
      INC_PC(1);                                                     \
  } while (0)

#define ROR(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int src, tmp_addr;                         \
                                                          \
      tmp_addr = (addr);                                  \
      src = load_func(tmp_addr);                          \
      if (LOCAL_CARRY()) {                                \
          src |= 0x100;                                   \
      }                                                   \
      LOCAL_SET_CARRY(src & 0x01);                        \
      src >>= 1;                                          \
      LOCAL_SET_NZ(src);                                  \
      INC_PC(pc_inc);                                     \
      store_func(tmp_addr, src, (clk_inc));               \
  } while (0)

#define ROR_A()                                                                      \
  do {                                                                               \
      BYTE tmp = reg_a;                                                              \
                                                                                     \
      if (LOCAL_65816_M()) {                                                         \
          reg_a = (reg_a & 0xff00) | (((reg_a >> 1) | (LOCAL_CARRY() << 7)) & 0xff); \
      } else {                                                                       \
          reg_a = (reg_a >> 1) | (LOCAL_CARRY() << 15);                              \
      }                                                                              \
      LOCAL_SET_CARRY(tmp & 1);                                                      \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());                                          \
      INC_PC(1);                                                                     \
  } while (0)

/* RTI does must not use `OPCODE_ENABLES_IRQ()' even if the I flag changes
   from 1 to 0 because the value of I is set 3 cycles before the end of the
   opcode, and thus the 6510 has enough time to call the interrupt routine as
   soon as the opcode ends, if necessary.  */
#define RTI()                      \
  do {                             \
      WORD tmp;                    \
                                   \
      CLK_ADD(CLK, CLK_RTI);       \
      tmp = (WORD)PULL();          \
      LOCAL_SET_STATUS((BYTE)tmp); \
      tmp = (WORD)PULL();          \
      tmp |= (WORD)PULL() << 8;    \
      if (!reg_emul) {             \
          CLK_ADD(CLK, 1);         \
          reg_pbr = PULL();        \
      }                            \
      JUMP(tmp);                   \
  } while (0)

#define RTL()                      \
  do {                             \
      WORD tmp;                    \
                                   \
      CLK_ADD(CLK, CLK_RTS);       \
      tmp = PULL();                \
      tmp = tmp | (PULL() << 8);   \
      LOAD(tmp);                   \
      CLK_ADD(CLK, CLK_INT_CYCLE); \
      tmp++;                       \
      reg_pbr = PULL();            \
      JUMP(tmp);                   \
  } while (0)

#define RTS()                      \
  do {                             \
      WORD tmp;                    \
                                   \
      CLK_ADD(CLK, CLK_RTS);       \
      tmp = PULL();                \
      tmp = tmp | (PULL() << 8);   \
      LOAD(tmp);                   \
      CLK_ADD(CLK, CLK_INT_CYCLE); \
      tmp++;                       \
      JUMP(tmp);                   \
  } while (0)

#define SBC(value, clk_inc, pc_inc)                                                            \
  do {                                                                                         \
      WORD src, tmp;                                                                           \
                                                                                               \
      src = (WORD)(value);                                                                     \
      CLK_ADD(CLK, (clk_inc));                                                                 \
      if (LOCAL_65816_M()) {                                                                   \
          if (LOCAL_DECIMAL()) {                                                               \
              tmp = (reg_a & 0xff) - (src & 0xf) + LOCAL_CARRY() - 1;                          \
              if ((tmp & 0xf) > (reg_a & 0xf)) {                                               \
                  tmp -= 6;                                                                    \
              }                                                                                \
              tmp -= (src & 0xf0);                                                             \
              if ((tmp & 0xf0) > (reg_a & 0xf0)) {                                             \
                  tmp -= 0x60;                                                                 \
              }                                                                                \
              LOCAL_SET_OVERFLOW(!(tmp > (reg_a & 0xff)));                                     \
              LOCAL_SET_CARRY(!(tmp > (reg_a & 0xff)));                                        \
              LOCAL_SET_NZ(tmp & 0xff);                                                        \
          } else {                                                                             \
              tmp = (reg_a & 0xff) - src - ((LOCAL_CARRY()) ? 0 : 1);                          \
              LOCAL_SET_NZ(tmp & 0xff);                                                        \
              LOCAL_SET_CARRY(tmp < 0x100);                                                    \
              LOCAL_SET_OVERFLOW((((reg_a & 0xff) ^ tmp) & 0x80)                               \
                                    && (((reg_a & 0xff) ^ src) & 0x80));                       \
          }                                                                                    \
          reg_a = tmp & 0xff;                                                                  \
      } else {                                                                                 \
          src ^= 0xffff;                                                                       \
          if (!LOCAL_DECIMAL()) {                                                              \
              tmp = reg_a + src + LOCAL_CARRY();                                               \
          } else {                                                                             \
              tmp = (reg_a & 0xf) + (src & 0xf) + LOCAL_CARRY();                               \
              if (tmp <= 0xf) {                                                                \
                  tmp -= 6;                                                                    \
              }                                                                                \
              LOCAL_SET_CARRY(tmp > 0xf);                                                      \
              tmp = (reg_a & 0xf0) + (src ^ 0xf0) + (LOCAL_CARRY() << 4) + (tmp & 0xf);        \
              if (tmp <= 0xff) {                                                               \
                  tmp -= 0x60;                                                                 \
              }                                                                                \
              LOCAL_SET_CARRY(tmp > 0xff);                                                     \
              tmp = (reg_a & 0xf00) + (src & 0xf00) + (LOCAL_CARRY() << 8) + (tmp & 0xff);     \
              if (tmp <= 0xfff) {                                                              \
                  tmp -= 0x600;                                                                \
              }                                                                                \
              LOCAL_SET_CARRY(tmp > 0xfff);                                                    \
              tmp = (reg_a & 0xf000) + (src & 0xf000) + (LOCAL_CARRY() << 12) + (tmp & 0xfff); \
          }                                                                                    \
          LOCAL_SET_OVERFLOW((~(reg_a ^ src) & (reg_a ^ tmp) & 0x8000) >> 8);                  \
          if (LOCAL_DECIMAL() && tmp <= 0xffff) {                                              \
              tmp -= 0x6000;                                                                   \
          }                                                                                    \
          LOCAL_SET_CARRY((tmp > 0xffff) ? 1 : 0);                                             \
          LOCAL_SET_NZ(tmp, LOCAL_65816_M());                                                  \
          reg_a = tmp & 0xffff;                                                                \
      }                                                                                        \
      INC_PC(pc_inc);                                                                          \
  } while (0)

#undef SEC    /* defined in time.h on SunOS. */
#define SEC()             \
  do {                    \
      LOCAL_SET_CARRY(1); \
      INC_PC(1);          \
  } while (0)

#define SED()               \
  do {                      \
      LOCAL_SET_DECIMAL(1); \
      INC_PC(1);            \
  } while (0)

#define SEI()                    \
  do {                           \
      if (!LOCAL_INTERRUPT()) {  \
          OPCODE_DISABLES_IRQ(); \
      }                          \
      LOCAL_SET_INTERRUPT(1);    \
      INC_PC(1);                 \
  } while (0)

#define SEP(mask) REPSEP(mask, 1)

#define STA(addr, pc_inc, store_func)          \
  do {                                         \
      unsigned int tmp;                        \
                                               \
      tmp = (addr);                            \
      INC_PC(pc_inc);                          \
      store_func(tmp, reg_a, LOCAL_65816_M()); \
  } while (0)

#define STP()          \
  do {                 \
      CLK_ADD(CLK, 1); \
      STP_65816();     \
  } while (0)

#define STX(addr, pc_inc, store_func)          \
  do {                                         \
      unsigned int tmp;                        \
                                               \
      tmp = (addr);                            \
      INC_PC(pc_inc);                          \
      store_func(tmp, reg_x, LOCAL_65816_X()); \
  } while (0)

#define STY(addr, pc_inc, store_func)          \
  do {                                         \
      unsigned int tmp;                        \
                                               \
      tmp = (addr);                            \
      INC_PC(pc_inc);                          \
      store_func(tmp, reg_y, LOCAL_65816_X()); \
  } while (0)

#define STZ(addr, pc_inc, store_func)      \
  do {                                     \
      unsigned int tmp;                    \
                                           \
      tmp = (addr);                        \
      INC_PC(pc_inc);                      \
      store_func(tmp, 0, LOCAL_65816_M()); \
  } while (0)

#define TAX()                               \
  do {                                      \
      if (LOCAL_REGISTER_SIZES() & 1) {     \
          reg_x = reg_a;                    \
      } else {                              \
          reg_x = reg_a & 0xff;             \
      }                                     \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define TAY()                               \
  do {                                      \
      if (LOCAL_REGISTER_SIZES() & 1) {     \
          reg_y = reg_a;                    \
      } else {                              \
          reg_y = reg_a & 0xff;             \
      }                                     \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define TCD()                   \
  do {                          \
      reg_dpr = reg_a;          \
      LOCAL_SET_NZ(reg_dpr, 1); \
      INC_PC(1);                \
  } while (0)

#define TCS()                                          \
  do {                                                 \
      if (reg_emul) {                                  \
          reg_sp = (reg_sp & 0xff00) | (reg_a & 0xff); \
      } else {                                         \
          reg_sp = reg_a;                              \
      }                                                \
      LOCAL_SET_NZ(reg_sp, 0);                         \
      INC_PC(1);                                       \
  } while (0)

#define TDC()                 \
  do {                        \
      reg_a = reg_dpr;        \
      LOCAL_SET_NZ(reg_a, 1); \
      INC_PC(1);              \
  } while (0)

#define TRB(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp_value, tmp_addr;                   \
                                                          \
      tmp_addr = (addr);                                  \
      tmp_value = load_func(tmp_addr);                    \
      if (LOCAL_65816_M()) {                              \
          LOCAL_SET_ZERO(!(tmp_value & (reg_a & 0xff));   \
          tmp_value &= (~(reg_a & 0xff));                 \
      } else {                                            \
          LOCAL_SET_ZERO(!(tmp_value & reg_a));           \
          tmp_value &= (~reg_a);                          \
      }                                                   \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp_value, LOCAL_65816_M());   \
  } while (0)

#define TSB(addr, clk_inc, pc_inc, load_func, store_func) \
  do {                                                    \
      unsigned int tmp_value, tmp_addr;                   \
                                                          \
      tmp_addr = (addr);                                  \
      tmp_value = load_func(tmp_addr);                    \
      if (LOCAL_65816_M()) {                              \
          LOCAL_SET_ZERO(!(tmp_value & (reg_a & 0xff));   \
          tmp_value |= (reg_a & 0xff);                    \
      } else {                                            \
          LOCAL_SET_ZERO(!(tmp_value & reg_a));           \
          tmp_value |= reg_a;                             \
      }                                                   \
      INC_PC(pc_inc);                                     \
      CLK_ADD(CLK, clk_inc);                              \
      store_func(tmp_addr, tmp_value, LOCAL_65816_M());   \
  } while (0)

#define TSC()                 \
  do {                        \
      reg_a = reg_sp;         \
      LOCAL_SET_NZ(reg_a, 0); \
      INC_PC(1);              \
  } while (0)


#define TSX()                               \
  do {                                      \
      if (LOCAL_65816_X()) {                \
          reg_x = reg_sp & 0xff;            \
      } else {                              \
          reg_x = reg_sp;                   \
      }                                     \
      LOCAL_SET_NZ(reg_x, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define TXA()                                        \
  do {                                               \
      if (LOCAL_REGISTER_SIZES() >= 2) {             \
          reg_a = reg_x;                             \
      } else {                                       \
          reg_a = (reg_a & 0xff00) | (reg_x & 0xff); \
      }                                              \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());          \
      INC_PC(1);                                     \
  } while (0)

#define TXS()                                 \
  do {                                        \
      if (LOCAL_65816_X()) {                  \
          reg_sp = (reg_sp & 0xff00) | reg_x; \
      } else {                                \
          reg_sp = reg_x;                     \
      }                                       \
      INC_PC(1);                              \
  } while (0)

#define TXY()                               \
  do {                                      \
      reg_y = reg_x;                        \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define TYA()                                        \
  do {                                               \
      if (LOCAL_REGISTER_SIZES() >= 2) {             \
          reg_a = reg_y;                             \
      } else {                                       \
          reg_a = (reg_a & 0xff00) | (reg_y & 0xff); \
      }                                              \
      LOCAL_SET_NZ(reg_a, LOCAL_65816_M());          \
      INC_PC(1);                                     \
  } while (0)

#define TYX()                               \
  do {                                      \
      reg_x = reg_y;                        \
      LOCAL_SET_NZ(reg_y, LOCAL_65816_X()); \
      INC_PC(1);                            \
  } while (0)

#define WAI()          \
  do {                 \
      CLK_ADD(CLK, 1); \
      WAI_65816();     \
  } while (0)

#define XBA()                                                  \
  do {                                                         \
      CLK_ADD(CLK, 1);                                         \
      reg_a = ((reg_a & 0xff00) >> 8) | ((reg_a & 0xff) << 8); \
      INC_PC(1);                                               \
  } while (0)

#define XCE()                                   \
  do {                                          \
      if (LOCAL_CARRY() != reg_emul) {          \
          if (LOCAL_CARRY()) {                  \
              reg_emul = 1;                     \
              LOCAL_SET_CARRY(0);               \
              LOCAL_SET_BREAK(0);               \
              reg_x &= 0xff;                    \
              reg_y &= 0xff;                    \
              reg_sp = 0x100 | (reg_sp & 0xff); \
          } else {                              \
              reg_emul = 0;                     \
              LOCAL_SET_CARRY(1);               \
              LOCAL_SET_65816_M(1);             \
              LOCAL_SET_65816_X(1);             \
          }                                     \
      }                                         \
      INC_PC(1);                                \
  } while (0)

 static const BYTE fetch_tab[] = {
            /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
    /* $00 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $00 */
    /* $10 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $10 */
    /* $20 */  1, 0, 2, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $20 */
    /* $30 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $30 */
    /* $40 */  0, 0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $40 */
    /* $50 */  0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, 1, 1, 2, /* $50 */
    /* $60 */  0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $60 */
    /* $70 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $70 */
    /* $80 */  0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $80 */
    /* $90 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $90 */
    /* $A0 */  8, 0, 8, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $A0 */
    /* $B0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $B0 */
    /* $C0 */  8, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $C0 */
    /* $D0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2, /* $D0 */
    /* $E0 */  8, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 2, /* $E0 */
    /* $F0 */  0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 2  /* $F0 */
};

#define FETCH_M16(opcode) (!LOCAL_65816_M() && (fetch_tab[opcode & 0xff] & 4))
#define FETCH_X16(opcode) (!LOCAL_65816_X() && (fetch_tab[opcode & 0xff] & 8))
#define FETCH_16(opcode) (FETCH_M16(opcode) || FETCH_X16(opcode))

#if !defined WORDS_BIGENDIAN && defined ALLOW_UNALIGNED_ACCESS

#define opcode_t DWORD

#define FETCH_OPCODE(o)                                                     \
    do {                                                                    \
        if (((int)reg_pc) < bank_limit) {                                   \
            o = (*((DWORD *)(bank_base + (reg_pbr * 0x10000) + reg_pc)));   \
            CLK_ADD(CLK, 2);                                                \
            if (fetch_tab[o & 0xff]) {                                      \
                CLK_ADD(CLK, fetch_tab[o & 0xff] & 3);                      \
            }                                                               \
        } else {                                                            \
            o = LOAD((reg_pbr * 0x10000) + reg_pc);                         \
            CLK_ADD(CLK, 1);                                                \
            o |= (LOAD((reg_pbr * 0x10000) + reg_pc + 1) << 8);             \
            CLK_ADD(CLK, 1);                                                \
            if ((fetch_tab[o & 0xff] & 3) || FETCH16(opcode)) {             \
                o |= (LOAD((reg_pbr * 0x10000) + reg_pc + 2) << 16);        \
                CLK_ADD(CLK, 1);                                            \
            }                                                               \
            if (fetch_tab[o & 0xff] & 2) {                                  \
                o |= LOAD((reg_pbr * 0x10000) + reg_pc + 3) << 24);         \
                CLK_ADD(CLK, 1);                                            \
            }                                                               \
        }                                                                   \
    } while (0)

#define p0 (opcode & 0xff)
#define p1 ((opcode >> 8) & 0xff)
#define p2 ((opcode >> 8) & 0xffff)
#define p3 (opcode >> 8)

#else /* WORDS_BIGENDIAN || !ALLOW_UNALIGNED_ACCESS */

#define opcode_t         \
    struct {             \
        BYTE ins;        \
        union {          \
            BYTE op8[2]; \
            WORD op16;   \
        } op;            \
        BYTE extra_op;   \
    }

#define FETCH_OPCODE(o)                                                              \
    do {                                                                             \
        if (((int)reg_pc) < bank_limit) {                                            \
            (o).ins = *(bank_base + (reg_pbr * 0x10000) + reg_pc);                   \
            (o).op.op16 = (*(bank_base + (reg_pbr * 0x10000) + reg_pc + 1)           \
                          | (*(bank_base + (reg_pbr * 0x10000) + reg_pc + 2) << 8)); \
            (o).extra_op = *(bank_base + (reg_pbr * 0x10000) + reg_pc + 3);          \
            CLK_ADD(CLK, 2);                                                         \
            if (fetch_tab[(o).ins]) {                                                \
                CLK_ADD(CLK, fetch_tab[(o).ins] & 3);                                \
            }                                                                        \
        } else {                                                                     \
            (o).ins = LOAD((reg_pbr * 0x10000) + reg_pc);                            \
            CLK_ADD(CLK, 1);                                                         \
            (o).op.op16 = LOAD((reg_pbr * 0x10000) + reg_pc + 1);                    \
            CLK_ADD(CLK, 1);                                                         \
            if ((fetch_tab[(o).ins] & 3) || (FETCH16((o).ins)) {                     \
                (o).op.op16 |= (LOAD((reg_pbr * 0x10000) + reg_pc + 2) << 8);        \
                CLK_ADD(CLK, 1);                                                     \
            }                                                                        \
            if (fetch_tab[(o).ins] & 2) {                                            \
                (o).extra_op = LOAD((reg_pbr * 0x10000) + reg_pc + 3);               \
                CLK_ADD(CLK, 1);                                                     \
            }
        }                                                      \
    } while (0)

#define p0 (opcode.ins)
#define p2 (opcode.op.op16)

#ifdef WORDS_BIGENDIAN
#  define p1 (opcode.op.op8[1])
#else
#  define p1 (opcode.op.op8[0])
#endif

#define p3 (opcode.op.op16) | (opcode.extra_op << 24)

#endif /* !WORDS_BIGENDIAN */

/*  SET_OPCODE for traps */
#if !defined WORDS_BIGENDIAN && defined ALLOW_UNALIGNED_ACCESS
#define SET_OPCODE(o) (opcode) = o;
#else
#if !defined WORDS_BIGENDIAN
#define SET_OPCODE(o)                          \
    do {                                       \
        opcode.ins = (o) & 0xff;               \
        opcode.op.op8[0] = ((o) >> 8) & 0xff;  \
        opcode.op.op8[1] = ((o) >> 16) & 0xff; \
    } while (0)
#else
#define SET_OPCODE(o)                          \
    do {                                       \
        opcode.ins = (o) & 0xff;               \
        opcode.op.op8[1] = ((o) >> 8) & 0xff;  \
        opcode.op.op8[0] = ((o) >> 16) & 0xff; \
    } while (0)
#endif
#endif

/* ------------------------------------------------------------------------ */

/* Here, the CPU is emulated. */

{
    CPU_DELAY_CLK

#ifndef CYCLE_EXACT_ALARM
    while (CLK >= alarm_context_next_pending_clk(ALARM_CONTEXT)) {
        alarm_context_dispatch(ALARM_CONTEXT, CLK);
        CPU_DELAY_CLK
    }
#endif

    {
        enum cpu_int pending_interrupt;

        if (!(CPU_INT_STATUS->global_pending_int & IK_IRQ)
            && (CPU_INT_STATUS->global_pending_int & IK_IRQPEND)
            && CPU_INT_STATUS->irq_pending_clk <= CLK) {
            interrupt_ack_irq(CPU_INT_STATUS);
        }

        pending_interrupt = CPU_INT_STATUS->global_pending_int;
        if (pending_interrupt != IK_NONE) {
            DO_INTERRUPT(pending_interrupt);
            if (!(CPU_INT_STATUS->global_pending_int & IK_IRQ)
                && CPU_INT_STATUS->global_pending_int & IK_IRQPEND)
                    CPU_INT_STATUS->global_pending_int &= ~IK_IRQPEND;
            CPU_DELAY_CLK
#ifndef CYCLE_EXACT_ALARM
            while (CLK >= alarm_context_next_pending_clk(ALARM_CONTEXT)) {
                alarm_context_dispatch(ALARM_CONTEXT, CLK);
                CPU_DELAY_CLK
            }
#endif
        }
    }

    {
        opcode_t opcode;
#ifdef DEBUG
        CLOCK debug_clk;
        debug_clk = maincpu_clk;
#endif

#ifdef FEATURE_CPUMEMHISTORY
        memmap_state |= (MEMMAP_STATE_INSTR | MEMMAP_STATE_OPCODE);
#endif
        SET_LAST_ADDR(reg_pc);
        FETCH_OPCODE(opcode);

#ifdef FEATURE_CPUMEMHISTORY
        /* HACK to cope with FETCH_OPCODE optimization in x64 */
        if (((int)reg_pc) < bank_limit) {
            memmap_mem_read(reg_pc);
        }
        if (p0 == 0x20) {
            monitor_cpuhistory_store(reg_pc, p0, p1, LOAD(reg_pc+2), reg_a, reg_x, reg_y, reg_sp, LOCAL_STATUS());
        } else {
            monitor_cpuhistory_store(reg_pc, p0, p1, p2 >> 8, reg_a, reg_x, reg_y, reg_sp, LOCAL_STATUS());
        }
        memmap_state &= ~(MEMMAP_STATE_INSTR | MEMMAP_STATE_OPCODE);
#endif

#ifdef DEBUG
        if (TRACEFLG) {
            BYTE op = (BYTE)(p0);
            BYTE lo = (BYTE)(p1);
            BYTE hi = (BYTE)(p2 >> 8);

            if (op == 0x20) {
               hi = LOAD(reg_pc + 2);
            }

            debug_maincpu((DWORD)(reg_pc), debug_clk,
                          mon_disassemble_to_string(e_comp_space,
                                                    reg_pc, op,
                                                    lo, hi, 0, 1, "6502"),
                          reg_a, reg_x, reg_y, reg_sp);
        }
        if (debug.perform_break_into_monitor)
        {
            monitor_startup_trap();
            debug.perform_break_into_monitor = 0;
        }
#endif

trap_skipped:
        SET_LAST_OPCODE(p0);

        switch (p0) {

          case 0x42:            /* NOP #$nn */
          case 0x82:            /* NOP #$nn */
            NOOP_IMM(2);
            break;

          case 0x44:            /* NOP $nn */
            NOOP(1, 2);
            break;

          case 0x54:            /* NOP $nn,X */
            NOOP(2, 2);
            break;

          case 0x00:            /* BRK */
            BRK();
            break;

          case 0x01:            /* ORA ($nn,X) */
            ORA(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x02:            /* NOP #$nn - also used for traps */
            STATIC_ASSERT(TRAP_OPCODE == 0x02);
            NOP_02();
            break;

          case 0x03:            /* ORA $nn,S */
            ORA(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0x04:            /* TSB $nn */
            TSB(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x05:            /* ORA $nn */
            ORA(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x06:            /* ASL $nn */
            ASL(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x07:            /* ORA [$nn] */
            ORA(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M(), 0, 2);
            break;

          case 0x08:            /* PHP */
            PHP();
            break;

          case 0x09:            /* ORA #$nn */
            ORA((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0x0a:            /* ASL A */
            ASL_A();
            break;

          case 0x0b:            /* PHD */
            PHD();
            break;

          case 0x0c:            /* TSB $nnnn */
            TSB(p2, 2, 3, p2, LOAD_ABS, STORE_ABS);
            break;

          case 0x0d:            /* ORA $nnnn */
            ORA(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x0e:            /* ASL $nnnn */
            ASL(p2, 2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x0f:            /* ORA $nnnnnn */
            ORA(LOAD_ABS_LONG(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x10:            /* BPL $nnnn */
            BRANCH(!LOCAL_SIGN(), p1);
            break;

          case 0x11:            /* ORA ($nn),Y */
            ORA(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x12:            /* ORA ($nn) */
            ORA(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x13:            /* ORA ($nn,S),Y */
            ORA(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0x14:            /* TRB $nn */
            TRB(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS);
            break;

          case 0x15:            /* ORA $nn,X */
            ORA(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0x16:            /* ASL $nn,X */
            ASL(p1 + reg_x, 3, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x17:            /* ORA [$nn],Y */
            ORA(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x18:            /* CLC */
            CLC();
            break;

          case 0x19:            /* ORA $nnnn,Y */
            ORA(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x1a:            /* INA */
            INA();
            break;

          case 0x1b:            /* TCS */
            TCS();
            break;

          case 0x1c:            /* TRB $nnnn */
            TRB(p2, 2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x1d:            /* ORA $nnnn,X */
            ORA(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x1e:            /* ASL $nnnn,X */
            ASL(p2, 2, 3, LOAD_ABS_X, STORE_ABS_X_RRW);
            break;

          case 0x1f:            /* ORA $nnnnnn,X */
            ORA(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x20:            /* JSR $nnnn */
            JSR();
            break;

          case 0x21:            /* AND ($nn,X) */
            AND(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x22:            /* JSR $nnnnnn */
            JSR_LONG();
            break;

          case 0x23:            /* AND $nn,S */
            AND(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0x24:            /* BIT $nn */
            BIT(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x25:            /* AND $nn */
            AND(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x26:            /* ROL $nn */
            ROL(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x27:            /* AND [$nn] */
            AND(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x28:            /* PLP */
            PLP();
            break;

          case 0x29:            /* AND #$nn */
            AND((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0x2a:            /* ROL A */
            ROL_A();
            break;

          case 0x2b:            /* PLD */
            PLD();
            break;

          case 0x2c:            /* BIT $nnnn */
            BIT(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x2d:            /* AND $nnnn */
            AND(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x2e:            /* ROL $nnnn */
            ROL(p2, 2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x2f:            /* AND $nnnnnn */
            AND(LOAD_ABS_LONG(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x30:            /* BMI $nnnn */
            BRANCH(LOCAL_SIGN(), p1);
            break;

          case 0x31:            /* AND ($nn),Y */
            AND(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x32:            /* AND ($nn) */
            AND(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x33:            /* AND ($nn,S),Y */
            AND(LOAD_STACK_REL_Y(p1, LOCAL_65816_M(), 5, 2);
            break;

          case 0x34:            /* BIT $nn,X */
            BIT(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0x35:            /* AND $nn,X */
            AND(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0x36:            /* ROL $nn,X */
            ROL(p1 + reg_x, 3, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x37:            /* AND [$nn],Y */
            AND(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x38:            /* SEC */
            SEC();
            break;

          case 0x39:            /* AND $nnnn,Y */
            AND(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x3a:            /* DEA */
            DEA();
            break;

          case 0x3b:            /* TSC */
            TSC();
            break;

          case 0x3c:            /* BIT $nnnn,X */
            BIT(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x3d:            /* AND $nnnn,X */
            AND(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x3e:            /* ROL $nnnn,X */
            ROL(p2, 2, 3, LOAD_ABS_X, STORE_ABS_X_RRW);
            break;

          case 0x3f:            /* AND $nnnnnn,X */
            AND(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x40:            /* RTI */
            RTI();
            break;

          case 0x41:            /* EOR ($nn,X) */
            EOR(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x43:            /* EOR $nn,S */
            EOR(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0x45:            /* EOR $nn */
            EOR(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x46:            /* LSR $nn */
            LSR(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x47:            /* EOR [$nn] */
            EOR(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x48:            /* PHA */
            PHA();
            break;

          case 0x49:            /* EOR #$nn */
            EOR((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0x4a:            /* LSR A */
            LSR_A();
            break;

          case 0x4b:            /* PHK */
            PHK();
            break;

          case 0x4c:            /* JMP $nnnn */
            JMP(p2);
            break;

          case 0x4d:            /* EOR $nnnn */
            EOR(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x4e:            /* LSR $nnnn */
            LSR(p2, 2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x4f:            /* EOR $nnnnnn */
            EOR(LOAD_ABS_LONG(p3), 1, 4);
            break;

          case 0x50:            /* BVC $nnnn */
            BRANCH(!LOCAL_OVERFLOW(), p1);
            break;

          case 0x51:            /* EOR ($nn),Y */
            EOR(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x52:            /* EOR ($nn) */                                                                                   
            EOR(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x53:            /* EOR ($nn,S),Y */
            EOR(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0x55:            /* EOR $nn,X */
            EOR(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0x56:            /* LSR $nn,X */
            LSR(p1 + reg_x, 3, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0x57:            /* EOR [$nn],Y */
            EOR(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x58:            /* CLI */
            CLI();
            break;

          case 0x59:            /* EOR $nnnn,Y */
            EOR(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x5a:            /* PHY */
            PHY();
            break;

          case 0x5b:            /* TCD */
            TCD();
            break;

          case 0x5c:            /* JMP $nnnnnn */
            JMP_LONG(p3);
            break;

          case 0x5d:            /* EOR $nnnn,X */
            EOR(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x5e:            /* LSR $nnnn,X */
            LSR(p2, 2, 3, LOAD_ABS_X, STORE_ABS_X_RRW);
            break;

          case 0x5f:            /* EOR $nnnnnn,X */
            EOR(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x60:            /* RTS */
            RTS();
            break;

          case 0x61:            /* ADC ($nn,X) */
            ADC(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x62:            /* PER $nnnn */
            PER(p2);
            break;

          case 0x63:            /* ADC $nn,S */
            ADC(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0x64:            /* STZ $nn */
            STZ(p1, 2, STORE_DIRECT_PAGE);
            break;

          case 0x65:            /* ADC $nn */
            ADC(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0x66:            /* ROR $nn */
            ROR(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x67:            /* ADC [$nn] */
            ADC(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x68:            /* PLA */
            PLA();
            break;

          case 0x69:            /* ADC #$nn */
            ADC((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0x6a:            /* ROR A */
            ROR_A();
            break;

          case 0x6b:            /* RTL */
            RTL();
            break;

          case 0x6c:            /* JMP ($nnnn) */
            JMP_IND();
            break;

          case 0x6d:            /* ADC $nnnn */
            ADC(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x6e:            /* ROR $nnnn */
            ROR(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x6f:            /* ADC $nnnnnn */
            ADC(LOAD_ABS_LONG(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x70:            /* BVS $nnnn */
            BRANCH(LOCAL_OVERFLOW(), p1);
            break;

          case 0x71:            /* ADC ($nn),Y */
            ADC(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x72:            /* ADC ($nn) */
            ADC(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x73:            /* ADC ($nn,S),Y */
            ADC(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0x74:            /* STZ $nn,X */
            STZ(p1, 2, STORE_DIRECT_PAGE_X);
            break;

          case 0x75:            /* ADC $nn,X */
            ADC(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0x76:            /* ROR $nn,X */
            ROR((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x77:            /* ADC [$nn],Y */
            ADC(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0x78:            /* SEI */
            SEI();
            break;

          case 0x79:            /* ADC $nnnn,Y */
            ADC(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x7a:            /* PLY */
            PLY();
            break;

          case 0x7b:            /* TDC */
            TDC();
            break;

          case 0x7c:            /* JMP ($nnnn,X) */
            JMP_IND_X();
            break;

          case 0x7d:            /* ADC $nnnn,X */
            ADC(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0x7e:            /* ROR $nnnn,X */
            ROR(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X, STORE_ABS_X_RMW);
            break;

          case 0x7f:            /* ADC $nnnnnn,X */
            ADC(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0x80:            /* BRA $nnnn */
            BRANCH(1, p1);
            break;

          case 0x81:            /* STA ($nn,X) */
            STA(p1, 2, STORE_INDIRECT_X);
            break;

          case 0x82:            /* BRL $nnnn */
            BRANCH_LONG(p2);
            break;

          case 0x83:            /* STA $nn,S */
            STA(p1, 2, STORE_STACK_REL);
            break;

          case 0x84:            /* STY $nn */
            STY(p1, 2, STORE_DIRECT_PAGE);
            break;

          case 0x85:            /* STA $nn */
            STA(p1, 2, STORE_DIRECT_PAGE);
            break;

          case 0x86:            /* STX $nn */
            STX(p1, 2, STORE_DIRECT_PAGE);
            break;

          case 0x87:            /* STA [$nn] */
            STA(p1, 2, STORE_INDIRECT_LONG);
            break;

          case 0x88:            /* DEY */
            DEY();
            break;

          case 0x89:            /* BIT #$nn */
            BIT_IMM((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0x8a:            /* TXA */
            TXA();
            break;

          case 0x8b:            /* PHB */
            PHB();
            break;

          case 0x8c:            /* STY $nnnn */
            STY(p2, 3, STORE_ABS);
            break;

          case 0x8d:            /* STA $nnnn */
            STA(p2, 3, STORE_ABS);
            break;

          case 0x8e:            /* STX $nnnn */
            STX(p2, 3, STORE_ABS);
            break;

          case 0x8f:            /* STA $nnnnnn */
            STA(p3, 4, STORE_ABS_LONG);
            break;

          case 0x90:            /* BCC $nnnn */
            BRANCH(!LOCAL_CARRY(), p1);
            break;

          case 0x91:            /* STA ($nn),Y */
            STA(p1, 2, STORE_INDIRECT_Y);
            break;

          case 0x92:            /* STA ($nn) */
            STA(p1, 2, STORE_INDIRECT);
            break;

          case 0x93:            /* STA ($nn,S),Y */
            STA(p1, 2, STORE_STACK_REL_Y);
            break;

          case 0x94:            /* STY $nn,X */
            STY(p1, 2, STORE_DIRECT_PAGE_X);
            break;

          case 0x95:            /* STA $nn,X */
            STA(p1, 2, STORE_DIRECT_PAGE_X);
            break;

          case 0x96:            /* STX $nn,Y */
            STX(p1, 2, STORE_DIRECT_PAGE_Y);
            break;

          case 0x97:            /* STA [$nn],Y */
            STA(p1, 2, STORE_INDIRECT_LONG_Y);
            break;

          case 0x98:            /* TYA */
            TYA();
            break;

          case 0x99:            /* STA $nnnn,Y */
            STA(p2, 3, STORE_ABS_Y);
            break;

          case 0x9a:            /* TXS */
            TXS();
            break;

          case 0x9b:            /* TXY */
            TXY();
            break;

          case 0x9c:            /* STZ $nnnn */
            STZ(p2, 3, STORE_ABS);
            break;                         

          case 0x9d:            /* STA $nnnn,X */
            STA(p2, 3, STORE_ABS_X);
            break;

          case 0x9e:            /* STZ $nnnn,X */
            STZ(p2, 3, STORE_ABS_X);
            break;

          case 0x9f:            /* STA $nnnnnn,X */
            STA(p3, 4, STORE_ABS_LONG_X);
            break;

          case 0xa0:            /* LDY #$nn */
            LDY((LOCAL_65816_X()) ? p1 : p2, 0, 2);
            break;

          case 0xa1:            /* LDA ($nn,X) */
            LDA(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xa2:            /* LDX #$nn */
            LDX((LOCAL_65816_X()) ? p1 : p2, 0, 2);
            break;

          case 0xa3:            /* LDA $nn,S */
            LDA(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0xa4:            /* LDY $nn */
            LDY(LOAD_DIRECT_PAGE(p1, LOCAL_65816_X()), 1, 2);
            break;

          case 0xa5:            /* LDA $nn */
            LDA(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xa6:            /* LDX $nn */
            LDX(LOAD_DIRECT_PAGE(p1, LOCAL_65816_X()), 1, 2);
            break;

          case 0xa7:            /* LDA [$nn] */
            LDA(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xa8:            /* TAY */
            TAY();
            break;

          case 0xa9:            /* LDA #$nn */
            LDA((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0xaa:            /* TAX */
            TAX();
            break;

          case 0xab:            /* PLB */
            PLB();
            break;

          case 0xac:            /* LDY $nnnn */
            LDY(LOAD_ABS(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xad:            /* LDA $nnnn */
            LDA(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xae:            /* LDX $nnnn */
            LDX(LOAD_ABS(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xaf:            /* LDA $nnnnnn */
            LDA(LOAD_ABS_LONG(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0xb0:            /* BCS $nnnn */
            BRANCH(LOCAL_CARRY(), p1);
            break;

          case 0xb1:            /* LDA ($nn),Y */
            LDA(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xb2:            /* LDA ($nn) */
            LDA(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xb3:            /* LDA ($nn,S),Y */
            LDA(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0xb4:            /* LDY $nn,X */
            LDY(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_X()), CLK_ZERO_I2, 2);
            break;

          case 0xb5:            /* LDA $nn,X */
            LDA(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0xb6:            /* LDX $nn,Y */
            LDX(LOAD_DIRECT_PAGE_Y(p1, LOCAL_65816_X()), CLK_ZERO_I2, 2);
            break;

          case 0xb7:            /* LDA [$nn],Y */
            LDA(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xb8:            /* CLV */
            CLV();
            break;

          case 0xb9:            /* LDA $nnnn,Y */
            LDA(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xba:            /* TSX */
            TSX();
            break;

          case 0xbb:            /* TYX */
            TYX();
            break;

          case 0xbc:            /* LDY $nnnn,X */
            LDY(LOAD_ABS_X(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xbd:            /* LDA $nnnn,X */
            LDA(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xbe:            /* LDX $nnnn,Y */
            LDX(LOAD_ABS_Y(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xbf:            /* LDA $nnnnnn,X */
            LDA(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0xc0:            /* CPY #$nn */
            CPY((LOCAL_65816_X()) ? p1 : p2, 0, 2);
            break;

          case 0xc1:            /* CMP ($nn,X) */
            CMP(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xc2:            /* REP #$nn */
            REP(p1);
            break;

          case 0xc3:            /* CMP $nn,S */
            CMP(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0xc4:            /* CPY $nn */
            CPY(LOAD_DIRECT_PAGE(p1, LOCAL_65816_X()), 1, 2);
            break;

          case 0xc5:            /* CMP $nn */
            CMP(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xc6:            /* DEC $nn */
            DEC(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0xc7:            /* CMP [$nn] */
            CMP(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xc8:            /* INY */
            INY();
            break;

          case 0xc9:            /* CMP #$nn */
            CMP((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0xca:            /* DEX */
            DEX();
            break;

          case 0xcb:            /* WAI */
            WAI();
            break;

          case 0xcc:            /* CPY $nnnn */
            CPY(LOAD_ABS(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xcd:            /* CMP $nnnn */
            CMP(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xce:            /* DEC $nnnn */
            DEC(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0xcf:            /* CMP $nnnnnn */
            CMP(LOAD_ABS_LONG(p3), 1, 4);
            break;

          case 0xd0:            /* BNE $nnnn */
            BRANCH(!LOCAL_ZERO(), p1);
            break;

          case 0xd1:            /* CMP ($nn),Y */
            CMP(LOAD_IND_IRECTY(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xd2:            /* CMP ($nn) */
            CMP(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xd3:            /* CMP ($nn,S),Y */
            CMP(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0xd4:            /* PEI ($nn) */
            PEI(LOAD_DIRECT_PAGE(p1, 0));
            break;

          case 0xd5:            /* CMP $nn,X */
            CMP(LOAD_DIRECT_PAGE_X(p1, LOCAL_65816_M()), CLK_ZERO_I2, 2);
            break;

          case 0xd6:            /* DEC $nn,X */
            DEC((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0xd7:            /* CMP [$nn],Y */
            CMP(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xd8:            /* CLD */
            CLD();
            break;

          case 0xd9:            /* CMP $nnnn,Y */
            CMP(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xda:            /* PHX */
            PHX();
            break;

          case 0xdb:            /* STP (WDC65C02) */
            STP();
            break;

          case 0xdc:            /* JMP [$nnnn] */
            JMP_IND_LONG();
            break;

          case 0xdd:            /* CMP $nnnn,X */
            CMP(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xde:            /* DEC $nnnn,X */
            DEC(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0xdf:            /* CMP $nnnnnn,X */
            CMP(LOAD_ABS_LONG_X(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0xe0:            /* CPX #$nn */
            CPX((LOCAL_65816_X()) ? p1 : p2, 0, 2);
            break;

          case 0xe1:            /* SBC ($nn,X) */
            SBC(LOAD_INDIRECT_X(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xe2:            /* SEP #$nn */
            SEP(p1);
            break;

          case 0xe3:            /* SBC $nn,S */
            SBC(LOAD_STACK_REL(p1, LOCAL_65816_M()), 2, 2);
            break;

          case 0xe4:            /* CPX $nn */
            CPX(LOAD_DIRECT_PAGE(p1, LOCAL_65816_X()), 1, 2);
            break;

          case 0xe5:            /* SBC $nn */
            SBC(LOAD_DIRECT_PAGE(p1, LOCAL_65816_M()), 1, 2);
            break;

          case 0xe6:            /* INC $nn */
            INC(p1, 2, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0xe7:            /* SBC [$nn] */
            SBC(LOAD_INDIRECT_LONG(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xe8:            /* INX */
            INX();
            break;

          case 0xe9:            /* SBC #$nn */
            SBC((LOCAL_65816_M()) ? p1 : p2, 0, 2);
            break;

          case 0xea:            /* NOP */
            NOP();
            break;

          case 0xeb:            /* XBA */
            XBA();
            break;

          case 0xec:            /* CPX $nnnn */
            CPX(LOAD_ABS(p2, LOCAL_65816_X()), 1, 3);
            break;

          case 0xed:            /* SBC $nnnn */
            SBC(LOAD_ABS(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xee:            /* INC $nnnn */
            INC(p2, 2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0xef:            /* SBC $nnnnnn */
            SBC(LOAD_ABS_LONG(p3, LOCAL_65816_M()), 1, 4);
            break;

          case 0xf0:            /* BEQ $nnnn */
            BRANCH(LOCAL_ZERO(), p1);
            break;

          case 0xf1:            /* SBC ($nn),Y */
            SBC(LOAD_INDIRECT_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xf2:            /* SBC ($nn) */
            SBC(LOAD_INDIRECT(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xf3:            /* SBC ($nn,S),Y */
            SBC(LOAD_STACK_REL_Y(p1, LOCAL_65816_M()), 5, 2);
            break;

          case 0xf4:            /* PEA $nnnn */
            PEA(p2);
            break;

          case 0xf5:            /* SBC $nn,X */
            SBC(LOAD_DIRECT_PAGE_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0xf6:            /* INC $nn,X */
            INC(p1 + reg_x, 3, 2, LOAD_DIRECT_PAGE, STORE_ABS_DPR);
            break;

          case 0xf7:            /* SBC [$nn],Y */
            SBC(LOAD_INDIRECT_LONG_Y(p1, LOCAL_65816_M()), 0, 2);
            break;

          case 0xf8:            /* SED */
            SED();
            break;

          case 0xf9:            /* SBC $nnnn,Y */
            SBC(LOAD_ABS_Y(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xfa:            /* PLX */
            PLX();
            break;

          case 0xfb:            /* XCE */
            XCE();
            break;

          case 0xfc:            /* JSR ($nnnn,X) */
            JSR_IND_X();
            break;

          case 0xfd:            /* SBC $nnnn,X */
            SBC(LOAD_ABS_X(p2, LOCAL_65816_M()), 1, 3);
            break;

          case 0xfe:            /* INC $nnnn,X */
            INC(p2, 2, 3, LOAD_ABS_X, STORE_ABS_X_RRW);
            break;

          case 0xff:            /* SBC $nnnnnn,X */
            SBC(LOAD_ABS_LONG_X(p1, LOCAL_65816_M()), 1, 4);
            break;
        }
    }
}
