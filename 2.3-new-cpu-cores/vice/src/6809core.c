/*
 * 6809core.c - M6809/HD6309 emulation core.
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

/* any CPU definition file that includes this file needs to do the following:
 *
 * - define reg_a as 8bit (6809/6309).
 * - define reg_b as 8bit (6809/6309).
 * - define reg_e as 8bit (6309).
 * - define reg_f as 8bit (6309).
 * - define reg_x as 16bit (6809/6309).
 * - define reg_y as 16bit (6809/6309).
 * - define reg_v as 16bit (6309).
 * - define reg_usp (User Stack Pointer) as 16bit (6809/6309).
 * - define reg_ssp (System Stack Pointer) as 16bit (6809/6309).
 * - define reg_dpr (Direct Page Register) as 8bit (6809/6309).
 * - define a function to handle the SYNC6809() call.
 * - define a function to handle the CWAI6809() call.
 * - define cpu_type as either 6809 or 6309.
 *
 * Notes:
 * reg_d is reg_a and reg_b combined (6809/6309).
 * reg_w is reg_e and reg_f combined (6309).
 * reg_q is reg_d and reg_w combined (6309).
 * reg_z is always 0 (6309).
 *
 * the way to define the a, b, d, e, f, w and q regs is:
 * union regs {
 *     DWORD reg_l;
 *     WORD reg_s[2];
 *     BYTE reg_c[4];
 * } regs6309;
 *
 * #define reg_q regs6309_reg_l
 * #ifndef WORDS_BIGENDIAN
 * #define reg_w regs6309.reg_s[0]
 * #define reg_d regs6309.reg_s[1]
 * #define reg_f regs6309.reg_c[0]
 * #define reg_e regs6309.reg_c[1]
 * #define reg_b regs6309.reg_c[2]
 * #define reg_a regs6309.reg_c[3]
 * #else
 * #define reg_w regs6309.reg_s[1]
 * #define reg_d regs6309.reg_s[0]
 * #define reg_f regs6309.reg_c[3]
 * #define reg_e regs6309.reg_c[2]
 * #define reg_b regs6309.reg_c[1]
 * #define reg_a regs6309.reg_c[0]
 * #endif
 */

/* This file is currently no included by any CPU definition files */

#define CPU_STR "6809/6309 CPU"

#include "traps.h"

#define CLK_ADD(clock, nr6809, nr6309)        \
  do {                                        \
      if (cpu_type == 6309 && LOCAL_MD_E()) { \
          clock += nr6309;                    \
      } else {                                \
          clock += nr6809;                    \
      }                                       \
  } while (0)

#ifndef REWIND_FETCH_OPCODE
#define REWIND_FETCH_OPCODE(clock) clock -= 2
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

/* 6309 reg_md -> DI----FE
 *
 * D = Division by zero trap
 * I = Illigal instruction trap
 * F = Fast IRQ behaves like normal IRQ
 * E = Execution mode (0 = 6809 emulation mode, 1 = 6309 native mode)
 */

#define LOCAL_SET_MD_D(val) \
  do {                      \
      if (val) {            \
          reg_md |= MD_D;   \
      } else {              \
          reg_md &= ~MD_D;  \
      }                     \
  } while (0)

#define LOCAL_SET_MD_I(val) \
  do {                      \
      if (val) {            \
          reg_md |= MD_I;   \
      } else {              \
          reg_md &= ~MD_I;  \
      }                     \
  } while (0)

#define LOCAL_SET_MD_F(val) \
  do {                      \
      if (val) {            \
          reg_md |= MD_F;   \
      } else {              \
          reg_md &= ~MD_F;  \
      }                     \
  } while (0)

#define LOCAL_SET_MD_E(val) \
  do {                      \
      if (val) {            \
          reg_md |= MD_E;   \
      } else {              \
          reg_md &= ~MD_E;  \
      }                     \
  } while (0)

#define LOCAL_MD_D()       (reg_md & MD_D)
#define LOCAL_MD_I()       (reg_md & MD_I)
#define LOCAL_MD_F()       (reg_md & MD_F)
#define LOCAL_MD_E()       (reg_md & MD_E)


/* 6809/6309 reg_p -> EFHINZVC
 *
 * E = Entire flag, all registers have been saved to the stack
 * F = Fast IRQ flag
 * H = Half carry flag
 * I = IRQ flag
 * N = Negative flag
 * Z = Zero flag
 * V = oVerflow flag
 * C = Carry flag
 */

#define LOCAL_SET_ENTIRE(val) \
  do {                        \
      if (val) {              \
          reg_p |= P_ENTIRE;  \
      } else {                \
          reg_p &= ~P_ENTIRE; \
      }                       \
  } while (0)

#define LOCAL_SET_FIRQ(val) \
  do {                      \
      if (val) {            \
          reg_p |= P_FIRQ;  \
      } else {              \
          reg_p &= ~P_FIRQ; \
      }                     \
  } while (0)

#define LOCAL_SET_HALF_CARRY(val) \
  do {                            \
      if (val) {                  \
          reg_p |= P_HALF_CARRY;  \
      } else {                    \
          reg_p &= ~P_HALF_CARRY; \
      }                           \
  } while (0)

#define LOCAL_SET_IRQ(val) \
  do {                     \
      if (val) {           \
          reg_p |= P_IRQ;  \
      } else {             \
          reg_p &= ~P_IRQ; \
      }                    \
  } while (0)

#define LOCAL_SET_NEGATIVE(val) \
  do {                          \
      if (val) {                \
          reg_p |= P_NEGATIVE;  \
      } else {                  \
          reg_p &= ~P_NEGATIVE; \
      }                         \
  } while (0)

#define LOCAL_SET_ZERO(val) \
  do {                      \
      if (val) {            \
          reg_p |= P_ZERO;  \
      } else {              \
          reg_p &= ~P_ZERO; \
      }                     \
  } while (0)

#define LOCAL_SET_OVERFLOW(val) \
  do {                          \
      if (val) {                \
          reg_p |= P_OVERFLOW;  \
      } else {                  \
          reg_p &= ~P_OVERFLOW; \
      }                         \
  } while (0)

#define LOCAL_SET_CARRY(val) \
  do {                       \
      if (val) {             \
          reg_p |= P_CARRY;  \
      } else {               \
          reg_p &= ~P_CARRY; \
      }                      \
  } while (0)

#define LOCAL_ENTIRE()       (reg_p & P_ENTIRE)
#define LOCAL_FIRQ()         (reg_p & P_FIRQ)
#define LOCAL_HALF_CARRY()   (reg_p & P_HALF_CARRY)
#define LOCAL_IRQ()          (reg_p & P_IRQ)
#define LOCAL_NEGATIVE()     (reg_p & P_NEGATIVE)
#define LOCAL_ZERO()         (reg_p & P_ZERO)
#define LOCAL_OVERFLOW()     (reg_p & P_OVERFLOW)
#define LOCAL_CARRY()        (reg_p & P_CARRY)


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
#define EXPORT_REGISTERS()       \
  do {                           \
      GLOBAL_REGS.pc = reg_pc;   \
      GLOBAL_REGS.a = reg_a;     \
      GLOBAL_REGS.b = reg_b;     \
      GLOBAL_REGS.e = reg_e;     \
      GLOBAL_REGS.f = reg_f;     \
      GLOBAL_REGS.x = reg_x;     \
      GLOVAL_REGS.y = reg_y;     \
      GLOBAL_REGS.p = reg_p;     \
      GLOBAL_REGS.md = reg_md;   \
      GLOBAL_REGS.v = reg_v;     \
      GLOBAL_REGS.ssp = reg_ssp; \
      GLOBAL_REGS.usp = reg_usp; \
      GLOBAL_REGS.dpr = reg_dpr; \
  } while (0)

/* Import the public version of the registers.  */
#define IMPORT_REGISTERS()       \
  do {                           \
      reg_a = GLOBAL_REGS.a;     \
      reg_b = GLOBAL_REGS.b;     \
      reg_e = GLOBAL_REGS.e;     \
      reg_f = GLOBAL_REGS.f;     \
      reg_x = GLOBAL_REGS.x;     \
      reg_y = GLOBAL_REGS.y;     \
      reg_p = GLOBAL_REGS.p;     \
      reg_md = GLOBAL_REGS.md;   \
      reg_v = GLOBAL_REGS.v;     \
      reg_ssp = GLOBAL_REGS.ssp; \
      reg_usp = GLOBAL_REGS.usp; \
      reg_dpr = GLOBAL_REGS.dpr; \
      JUMP(GLOBAL_REGS.pc);      \
  } while (0)

/* Stack operations. */

#ifndef PUSHS
#define PUSHS(val) STORE(reg_ssp--, (BYTE)(val))
#endif
#ifndef PUSHU
#define PUSHU(val) STORE(reg_usp--, (BYTE)(val))
#endif
#ifndef PULLS
#define PULLS()    LOAD(++reg_ssp)
#endif
#ifndef PULLU
#define PULLU()    LOAD(++reg_usp)
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

#define opcode_t  \
    struct {      \
        BYTE ins; \
        BYTE p1;  \
        BYTE p2;  \
        BYTE p3;  \
        BYTE p4;  \
    }

#define FETCH_OPCODE(o, page)                 \
    do {                                      \
        o.ins = LOAD(reg_pc);                 \
        page = 0;                             \
        if (o.ins == 0x10 || o.ins == 0x11) { \
            page = o.ins;                     \
        }                                     \
        if (page) {                           \
            o.ins = LOAD(reg_pc + 1);         \
            o.p1 = LOAD(reg_pc + 2);          \
            o.p2 = LOAD(reg_pc + 3);          \
            o.p3 = LOAD(reg_pc + 4);          \
            o.p4 = LOAD(reg_pc + 5);          \
        } else {                              \
            o.p1 = LOAD(reg_pc + 1);          \
            o.p2 = LOAD(reg_pc + 2);          \
            o.p3 = LOAD(reg_pc + 3);          \
            o.p4 = LOAD(reg_pc + 4);          \
        }                                     \
  } while (0)

#define p0 opcode.ins
#define p1 opcode.p1
#define p2 opcode.p2
#define p3 opcode.p3
#define p4 opcode.p4

/* ------------------------------------------------------------------------- */

/* Addressing modes.  Memory reads are handled here as well. */

#define INC_PC(value)   (reg_pc += (value))

#define PB_REG2VAR(pb, var)       \
  do {                            \
      switch ((pb & 0x60) >> 5) { \
          case 0:                 \
          default:                \
              var = reg_x;        \
              break;              \
          case 1:                 \
              var = reg_y;        \
              break;              \
          case 2:                 \
              var = reg_usp;      \
              break;              \
          case 3:                 \
              var = reg_ssp;      \
              break;              \
      }                           \
  } while (0)

#define PB_REGADD(pb, amount)     \
  do {                            \
      switch ((pb & 0x60) >> 5) { \
          case 0:                 \
          default:                \
              reg_x += amount;    \
              break;              \
          case 1:                 \
              reg_y += amount;    \
              break;              \
          case 2:                 \
              reg_usp += amount;  \
              break;              \
          case 3:                 \
              reg_ssp += amount;  \
              break;              \
      }                           \
  } while (0)

/* ------------------------------------------------------------------------- */

#define LOAD_ABS(addr) \
   (CLK_ADD(CLK, 1), LOAD(addr))

/* instead of doing this one as a macro I'm doing this one as a function, less bloating */
static WORD GET_IND_MA(BYTE b1, BYTE b2, BYTE b3)
{
    WORD ma = 0;

    if (!(b1 & 0x80)) {
        /* n,R (5bit offset) */
        PB_REG2VAR(b1, ma);
        ma += (b1 & 0x1f);
        CLK_ADD(CLK, 1, 1);
    } else {
        PC_INC(1);       /* postbyte fetch */
        PB_REG2VAR(b1, ma);
        switch (b1 & 0x1f) {
            case 0:      /* ,R+ */
                PB_REGADD(b1, 1);
                CLK_ADD(CLK, 2, 1);
                break;
            case 1:      /* ,R++ */
                PB_REGADD(b1, 2);
                CLK_ADD(CLK, 3, 2);
                break;
            case 2:      /* ,R- */
                PB_REGADD(b1, -1);
                CLK_ADD(CLK, 2, 1);
                break;
            case 3:      /* ,R-- */
                PB_REGADD(b1, -2);
                CLK_ADD(CLK, 3, 2);
                break;
            case 4:      /* ,R */
                break;
            case 5:      /* B,R */
                ma += (signed char)reg_b;
                CLK_ADD(CLK, 1, 1);
                break;
            case 6:      /* A,R */
                ma += (signed char)reg_a;
                CLK_ADD(CLK, 1, 1);
                break;
            case 7:      /* E,R */
                if (cpu_type == 6309) {
                    ma += (signed char)reg_e;
                    CLK_ADD(CLK, 1, 1);
                }
                break;
            case 8:      /* n,R (8bit offset) */
                ma += b2;
                PC_INC(1);
                CLK_ADD(CLK, 1, 1);
                break;
            case 9:      /* n,R (16bit offset) */
                ma += ((b2 << 8) | b3);
                PC_INC(2);
                CLK_ADD(CLK, 4, 3);
                break;
            case 10:     /* F,R */
                if (cpu_type == 6309) {
                    ma += (signed char)reg_f;
                    CLK_ADD(CLK, 1, 1);
                }
                break;
            case 11:     /* D,R */
                ma += (signed short)reg_d;
                CLK_ADD(CLK, 4, 2);
                break;
            case 12:     /* n,PC (8bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(1);
                ma = reg_pc + (signed char)b2;
                CLK_ADD(CLK, 1, 1);
                break;
            case 13:     /* n,PC (16bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((b2 << 8) | b3);
                CLK_ADD(CLK, 5, 3);
                break;
            case 14:     /* W,R */
                if (cpu_type == 6309) {
                    ma += (signed short)((reg_e << 8) | reg_f);
                    CLK_ADD(CLK, 4, 1);
                }
                break;
            case 15:
                if (cpu_type == 6309) {
                    switch ((b1 & 0x60) >> 5) {
                        case 0:     /* ,W */
                            ma = reg_w;
                            break;
                        case 1:     /* n,W (16bit offset) */
                            ma = reg_w + (signed short)((b2 << 8) | b3);
                            PC_INC(2);
                            CLK_ADD(CLK, 5, 2);
                            break;
                        case 2:     /* ,W++ */
                            ma = reg_w;
                            reg_w += 2;
                            CLK_ADD(CLK, 3, 1);
                            break;
                        case 3:     /* ,W-- */
                            ma = reg_w;
                            reg_w -= 2;
                            CLK_ADD(CLK, 3, 1);
                            break;
                    }
                }
                break;
            case 16:
                if (cpu_type == 6309) {
                    switch ((p1 & 0x60) >> 5) {
                        case 0:     /* [,W] */
                            ma = LOAD(reg_w) << 8;
                            ma |= LOAD(reg_w + 1);
                            break;
                        case 1:     /* [n,W] (16bit offset) */
                            ma = LOAD(reg_w + (signed short)((b2 << 8) | b3)) << 8;
                            ma |= LOAD(reg_w + 1 + (signed short)((b2 << 8) | b3));
                            PC_INC(2);
                            CLK_ADD(CLK, 5, 5);
                            break;
                        case 2:     /* [,W++] */
                            ma = LOAD(reg_w) << 8;
                            ma |= LOAD(reg_w + 1);
                            reg_w += 2;
                            CLK_ADD(CLK, 3, 3);
                            break;
                        case 3:     /* [,W--] */
                            ma = LOAD(reg_w) << 8;
                            ma |= LOAD(reg_w + 1);
                            reg_w -= 2;
                            CLK_ADD(CLK, 3, 3);
                            break;
                    }
                }
                break;
            case 17:     /* [,R++] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PB_REGADD(b1, 2);
                CLK_ADD(CLK, 6, 6);
                break;
            case 18:
                break;
            case 19:     /* [,R--] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PB_REGADD(b1, -2);
                CLK_ADD(CLK, 6, 6);
                break;
            case 20:     /* [,R] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 3, 3);
                break;
            case 21:     /* [B,R] */
                ma += (signed char)reg_b;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 4, 4);
                break;
            case 22:     /* [A,R] */
                ma += (signed char)reg_a;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 4, 4);
                break;
            case 23:     /* [E,R] */
                if (cpu_type == 6309) {
                    ma += (signed char)reg_e;
                    ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                    CLK_ADD(CLK, 1, 1);
                }
                break;
            case 24:     /* [n,R] (8bit offset) */
                ma += b2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PC_INC(1);
                CLK_ADD(CLK, 4, 4);
                break;
            case 25:     /* [n,R] (16bit offset) */
                ma += ((b2 << 8) | b3);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PC_INC(2);
                CLK_ADD(CLK, 7, 7);
                break;
            case 26:     /* [F,R] */
                if (cpu_type == 6309) {
                    ma += (signed char)reg_f;
                    ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                    CLK_ADD(CLK, 1, 1);
                }
                break;
            case 27:     /* [D,R] */
                ma += (signed short)reg_d;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 4, 4);
                break;
            case 28:     /* [n,PC] (8bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(1);
                ma = reg_pc + (signed char)b2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 4, 4);
                break;
            case 29:     /* [n,PC] (16bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((b2 << 8) | b3);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                CLK_ADD(CLK, 8, 8);
                break;
            case 30:     /* [W,R] */
                if (cpu_type == 6309) {
                    ma += (signed short)((reg_e << 8) | reg_f);
                    ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                    CLK_ADD(CLK, 4, 4);
                }
                break;
            case 31:     /* [n] (16bit extended indirect) */
                ma = LOAD((b2 << 8) | b3) << 8;
                ma |= LOAD(((b2 << 8) | b3) + 1);
                PC_INC(2);
                CLK_INC(CLK, 5, 5);
                break;
        }
    }
    return ma;
}

#define LOAD_DIRECT8(addr) \
   (LOAD((reg_dpr << 8) | addr))

#define LOAD_DIRECT16(addr) \
   ((LOAD_DIRECT8(addr) << 8) | (LOAD_DIRECT8((addr + 1) & 0xff)))

#define LOAD_DIRECT32(addr) \
   ((LOAD_DIRECT16(addr) << 16) | (LOAD_DIRECT16((addr + 2) & 0xff)))

#define LOAD_EXT8() \
   (LOAD((p1 << 8) | p2))

#define LOAD_EXT16() \
   ((LOAD_EXT8((p1 << 8) | p2) << 8) | (LOAD_EXT8((p1 << 8) + p2 + 1)))

#define LOAD_EXT32() \
   ((LOAD_EXT16((p1 << 8) | p2) << 16) | (LOAD_EXT16((p1 << 8) + p2 + 2)))

static BYTE LOAD_IND8(void)
{
    return LOAD(GET_IND_MA(p1, p2, p3));
}

static WORD LOAD_IND16(void)
{
    WORD ma = GET_IND_MA(p1, p2, p3);
    WORD retval;

    retval = LOAD(ma) << 8;
    retval |= LOAD(ma + 1);
    return retval;
}

static DWORD LOAD_IND32(void)
{
    WORD ma = GET_IND_MA(p1, p2, p3);
    WORD retval;

    retval = LOAD(ma) << 24;
    retval |= (LOAD(ma + 1) << 16);
    retval |= (LOAD(ma + 2) << 8);
    retval |= LOAD(ma + 3);
    return retval;
}

/* ------------------------------------------------------------------------- */

#define BT(val, bit) (val & (1 < bit)) ? 1 : 0

#define MAKE16(r0, r1, mixed)  \
  do {                         \
      if (mixed) {             \
          r0 = r1 | (r1 << 8); \
      } else {                 \
          r0 = r1;             \
      }                        \
  } while (0)

#define MAKE8(r0, r1, mixed, pr) \
  do {                           \
      if (mixed) {               \
          if (pr) {              \
              r0 = r1 >> 8;      \
          } else {               \
              r0 = r1 & 0xff;    \
          }                      \
      } else {                   \
          r0 = r1;               \
      }                          \
  } while (0)


/* FIXME: the following 2 macros need to be fixed for 6809 */

#define REG2VAR(rnr, var, mixed)           \
  do {                                     \
      switch (rnr) {                       \
          case 0:                          \
              var = reg_d;                 \
              break;                       \
          case 1:                          \
              var = reg_x;                 \
              break;                       \
          case 2:                          \
              var = reg_y;                 \
              break;                       \
          case 3:                          \
              var = reg_usp;               \
              break;                       \
          case 4:                          \
              var = reg_ssp;               \
              break;                       \
          case 5:                          \
              var = reg_pc;                \
              break;                       \
          case 6:                          \
              var = reg_w;                 \
              break;                       \
          case 7:                          \
              var = reg_v;                 \
              break;                       \
          case 8:                          \
              MAKE16(var, reg_a, mixed);   \
              break;                       \
          case 9:                          \
              MAKE16(var, reg_b, mixed);   \
              break;                       \
          case 10:                         \
              MAKE16(var, reg_p, mixed);   \
              break;                       \
          case 11:                         \
              MAKE16(var, reg_dpr, mixed); \
              break;                       \
          case 12:                         \
          case 13:                         \
              var = 0;                     \
              break;                       \
          case 14:                         \
              MAKE16(var, reg_e, mixed);   \
              break;                       \
          case 15:                         \
              MAKE16(var, reg_f, mixed);   \
              break;                       \
      }                                    \
  } while (0)

#define VAR2REG(rnr, var, mixed)             \
  do {                                       \
      switch (rnr) {                         \
          case 0:                            \
              reg_d = var;                   \
              break;                         \
          case 1:                            \
              reg_x = var;                   \
              break;                         \
          case 2:                            \
              reg_y = var;                   \
              break;                         \
          case 3:                            \
              reg_usp = var;                 \
              break;                         \
          case 4:                            \
              reg_ssp = var;                 \
              break;                         \
          case 5:                            \
              reg_pc = var;                  \
              break;                         \
          case 6:                            \
              reg_w = var;                   \
              break;                         \
          case 7:                            \
              reg_v = var;                   \
              break;                         \
          case 8:                            \
              MAKE8(reg_a, var, mixed, 1);   \
              break;                         \
          case 9:                            \
              MAKE8(reg_b, var, mixed, 0);   \
              break;                         \
          case 10:                           \
              MAKE8(reg_p, var, mixed, 0);   \
              break;                         \
          case 11:                           \
              MAKE8(reg_dpr, var, mixed, 1); \
              break;                         \
          case 12:                           \
          case 13:                           \
              break;                         \
          case 14:                           \
              MAKE8(reg_e, var, mixed, 1);   \
              break;                         \
          case 15:                           \
              MAKE8(reg_f, var, mixed, 0);   \
              break;                         \
      }                                      \
  } while (0)

#define R_REG2VAR(rnr, var, bits)                \
  do {                                           \
      switch (rnr) {                             \
          case 0:                                \
              var = reg_d;                       \
              break;                             \
          case 1:                                \
              var = reg_x;                       \
              break;                             \
          case 2:                                \
              var = reg_y;                       \
              break;                             \
          case 3:                                \
              var = reg_usp;                     \
              break;                             \
          case 4:                                \
              var = reg_ssp;                     \
              break;                             \
          case 5:                                \
              var = reg_pc;                      \
              break;                             \
          case 6:                                \
              var = reg_w;                       \
              break;                             \
          case 7:                                \
              var = reg_v;                       \
              break;                             \
          case 8:                                \
              var = (bits == 8) ? reg_a : reg_d; \
              break;                             \
          case 9:                                \
              var = (bits == 8) ? reg_b : reg_d; \
              break;                             \
          case 10:                               \
              var = (bits == 8) ? reg_p : 0;     \
              break;                             \
          case 11:                               \
              var = (bits == 8) ? reg_dpr : 0;   \
              break;                             \
          case 12:                               \
          case 13:                               \
              var = 0;                           \
              break;                             \
          case 14:                               \
              var = (bits == 8) ? reg_e : reg_w; \
              break;                             \
          case 15:                               \
              var = (bits == 8) ? reg_f : reg_w; \
              break;                             \
      }                                          \
  } while (0)

#define R_VAR2REG(rnr, var, bits)      \
  do {                                 \
      switch (rnr) {                   \
          case 0:                      \
              reg_d = var;             \
              break;                   \
          case 1:                      \
              reg_x = var;             \
              break;                   \
          case 2:                      \
              reg_y = var;             \
              break;                   \
          case 3:                      \
              reg_usp = var;           \
              break;                   \
          case 4:                      \
              reg_ssp = var;           \
              break;                   \
          case 5:                      \
              reg_pc = var;            \
              break;                   \
          case 6:                      \
              reg_w = var;             \
              break;                   \
          case 7:                      \
              reg_v = var;             \
              break;                   \
          case 8:                      \
              if (bits == 8) {         \
                  reg_a = var;         \
              } else {                 \
                  reg_d = var;         \
              }                        \
              break;                   \
          case 9:                      \
              if (bits == 8) {         \
                  reg_b = var;         \
              } else {                 \
                  reg_d = var;         \
              }                        \
              break;                   \
          case 10:                     \
              if (bits == 8) {         \
                  reg_p = var;         \
              }                        \
              break;                   \
          case 11:                     \
              if (bits == 8) {         \
                  reg_dpr = var;       \
              }                        \
              break;                   \
          case 12:                     \
          case 13:                     \
              break;                   \
          case 14:                     \
              if (bits == 8) {         \
                  reg_e = var;         \
              } else {                 \
                  reg_w = var;         \
              }                        \
              break;                   \
          case 15:                     \
              if (bits == 8) {         \
                  reg_f = var;         \
              } else {                 \
                  reg_w = var;         \
              }                        \
              break;                   \
      }                                \
  } while (0)

#define MIXEDBITS(rnr)                     \
  do {                                     \
      if (((rnr >> 4) & 8) != (rnr & 8)) { \
          mixed = 1;                       \
      } else {                             \
          mixed = 0;                       \
      }                                    \
      if (mixed) {                         \
          bits = 16;                       \
      } else {                             \
          if (rnr & 8) {                   \
              bits = 8;                    \
          } else {                         \
              bits = 16;                   \
          }                                \
      }                                    \
  } while (0)

#define R_WRAP(rnr, func, pc_inc)         \
  do {                                    \
      int mixed, bits;                    \
      WORD tmp0, tmp1;                    \
                                          \
      MIXEDBITS(rnr);                     \
      R_REG2VAR((rnr >> 4), tmp0, bits);  \
      R_REG2VAR((rnr & 0xf), tmp1, bits); \
      func(tmp0, tmp1, bits8, pc_inc);    \
      R_VAR2REG((rnr & 0xf), tmp1, bits); \
  } while (0)

#define BTM_REG2VAR(rnr, var) \
  do {                        \
      switch (rnr >> 6) {     \
          case 0:             \
              var = reg_p;    \
              break;          \
          case 1:             \
              var = reg_a;    \
              break;          \
          case 2:             \
              var = reg_b;    \
              break;          \
          case 3:             \
              var = 0;        \
              break;          \
      }                       \
  } while (0)

define BTM_VAR2REG(rnr, var) \
  do {                        \
      switch (rnr >> 6) {     \
          case 0:             \
              reg_p = var;    \
              break;          \
          case 1:             \
              reg_a = var;    \
              break;          \
          case 2:             \
              reg_b = var;    \
              break;          \
          case 3:             \
              break;          \
      }                       \
  } while (0)

#define TFM_REG2VAR(rnr, var, ill) \
  do {                             \
      ill = 0;                     \
      switch (rnr) {               \
          case 0:                  \
              var = reg_d;         \
              break;               \
          case 1:                  \
              var = reg_x;         \
              break;               \
          case 2:                  \
              var = reg_y;         \
              break;               \
          case 3:                  \
              var = reg_usp;       \
              break;               \
          case 4:                  \
              var = reg_ssp;       \
              break;               \
          default:                 \
              ill = 1;             \
              break;               \
      }                            \
  } while (0)

#define TFM_VAR2REG(rnr, var, ill) \
  do {                             \
      ill = 0;                     \
      switch (rnr) {               \
          case 0:                  \
              reg_d = var;         \
              break;               \
          case 1:                  \
              reg_x = var;         \
              break;               \
          case 2:                  \
              reg_y = var;         \
              break;               \
          case 3:                  \
              reg_usp = var;       \
              break;               \
          case 4:                  \
              reg_ssp = var;       \
              break;               \
          default:                 \
              ill = 1;             \
              break;               \
      }                            \
  } while (0)

#define TFMPP_REG(r0, r1)     \
  do {                        \
      BYTE val;               \
                              \
      while (reg_w) {         \
          val = LOAD(r0++);   \
          STORE(r1++, val);   \
          reg_w--;            \
          CLK_ADD(CLK, 3, 3); \
      }                       \
      PC_INC(3);              \
      CLK_ADD(CLK, 6, 6);     \
  } while (0)

#define TFMMM_REG(r0, r1)     \
  do {                        \
      BYTE val;               \
                              \
      while (reg_w) {         \
          val = LOAD(r0--);   \
          STORE(r1--, val);   \
          reg_w--;            \
          CLK_ADD(CLK, 3, 3); \
      }                       \
      PC_INC(3);              \
      CLK_ADD(CLK, 6, 6);     \
  } while (0)

#define TFMPC_REG(r0, r1)     \
  do {                        \
      BYTE val;               \
                              \
      while (reg_w) {         \
          val = LOAD(r0++);   \
          STORE(r1, val);     \
          reg_w--;            \
          CLK_ADD(CLK, 3, 3); \
      }                       \
      PC_INC(3);              \
      CLK_ADD(CLK, 6, 6);     \
  } while (0)

#define TFMCP_REG(r0, r1)     \
  do {                        \
      BYTE val = LOAD(r0);    \
                              \
      while (reg_w) {         \
          STORE(r1++, val);   \
          CLK_ADD(CLK, 3, 3); \
      }                       \
      PC_INC(3);              \
      CLK_ADD(CLK, 6, 6);     \
  } while (0)

#define TFM_WRAP(rnr, func)                    \
  do {                                         \
      unsigned int ill;                        \
      WORD tmp0, tmp1;                         \
                                               \
      TFM_REG2VAR((rnr >> 4), tmp0, ill);      \
      if (!ill) {                              \
          TFM_REG2VAR((rnr & 0xf), tmp1, ill); \
      }                                        \
      if (!ill) {                              \
          func(tmp0, tmp1);                    \
          TFM_VAR2REG((rnr >> 4), tmp0, ill);  \
          TFM_VAR2REG((rnr & 0xf), tmp1, ill); \
      } else {                                 \
          ILLEGAL_OPCODE_TRAP();               \
      }                                        \
  } while (0)

/* ------------------------------------------------------------------------- */

/* Opcodes.  */

#defime ABX()                       \
  do {                              \
      reg_a += reg_b;               \
      PC_INC(1 + ec);               \
      CLK_ADD(CLK, 3 + ec, 1 + ec); \
  } while (0)

#define ADD(RR, m, CC, bits, pc_inc, clk6809, clk6309) \
  do {                                                 \
      unsigned int tmp;                                \
                                                       \
      if (bits == 8) {                                 \
          tmp = (RR & 0xf) + (m & 0xf) + CC;           \
          LOCAL_SET_HALF_CARRY(BT(tmp, 4));            \
          tmp = (RR & 0x7f) + (m & 0x7f) + CC;         \
      } else {                                         \
          tmp = (RR & 0x7fff) + (m & 0x7fff) + CC;     \
      }                                                \
      LOCAL_SET_OVERFLOW(BT(tmp, bits - 1));           \
      tmp = RR + m + CC;                               \
      LOCAL_SET_CARRY(BT(tmp, bits));                  \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1));           \
      LOCAL_SET_ZERO(!tmp);                            \
      RR = tmp & (bits == 8) ? 0xff : 0xffff;          \
      PC_INC(pc_inc);                                  \
      CLK_ADD(CLK, clk6809, clk6309);                  \
  } while (0)

#define AIM(m, ma, pc_inc, clk6809, clk6309) \
  do {                                       \
      BYTE tmp;                              \
                                             \
      tmp = LOAD(ma);                        \
      tmp &= m;                              \
      LOCAL_SET_OVERFLOW(0);                 \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));        \
      LOCAL_SET_ZERO(!tmp);                  \
      STORE(ma, tmp);                        \
      PC_INC(pc_inc);                        \
      CLK_ADD(CLK, clk6809, clk6309);        \
  } while (0)

#define AND(RR, m, bits, pc_inc, clk6809, clk6309) \
  do {                                             \
      RR &= m;                                     \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));        \
      LOCAL_SET_ZERO(!RR);                         \
      LOCAL_SET_OVERFLOW(0);                       \
      PC_INC(pc_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);              \
  } while (0)

#define ASL_REG(RR, bits, pc_inc, clk6809, clk6309)            \
  do {                                                         \
      LOCAL_SET_CARRY(BT(RR, bits - 1));                       \
      LOCAL_SET_OVERFLOW(BT(RR, bits - 1) ^ BT(RR, bits - 2)); \
      RR <<= 1;                                                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));                    \
      LOCAL_SET_ZERO(!RR);                                     \
      PC_INC(pc_inc);                                          \
      CLK_ADD(CLK, clk6809, clk6309);                          \
  } while (0)

#define ASL(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      tmp = LOAD(ma);                            \
      ASL_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define ASR_REG(RR, bits, pc_inc, clk6809, clk6309)          \
  do {                                                       \
      LOCAL_SET_CARRY(BT(RR, 0));                            \
      RR = (RR & ((bits == 8) ? 0x80 : 0x8000)) | (RR >> 1); \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));                  \
      LOCAL_SET_ZERO(!RR);                                   \
      PC_INC(pc_inc);                                        \
      CLK_ADD(CLK, clk6809, clk6309);                        \
  } while (0)

#define ASR(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      ASR_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define BAND(rnr, ma)                              \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = stmp & dtmp;                          \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BEOR(rnr, ma)                              \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = stmp ^ dtmp;                          \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BIAND(rnr, ma)                             \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = !(stmp & dtmp);                       \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BIEOR(rnr, ma)                             \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = !(stmp ^ dtmp);                       \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BIOR(rnr, ma)                              \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = !(stmp | dtmp);                       \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BIT(RR, bits, m, pc_inc, clk6809, clk6309) \
  do {                                             \
      unsigned int tmp;                            \
                                                   \
      tmp = RR & m;                                \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1));       \
      LOCAL_SET_ZERO(!tmp);                        \
      LOCAL_SET_OVERFLOW(0);                       \
      PC_INC(pc_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);              \
  } while (0)

#define BOR(rnr, ma)                               \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
      unsigned int dtmp;                           \
      unsigned int atmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      dtmp = (tmp & (1 << dbit)) ? 1 : 0;          \
      atmp = stmp | dtmp;                          \
      tmp = (tmp & ~(1 << dbit)) | (atmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      PC_INC(4);                                   \
      CLK_ADD(CLK, 7, 6);                          \
  } while (0)

#define BRANCH(cond, value, pc_inc, clk6809, clk6309) \
  do {                                                \
      unsigned int dest_addr = 0;                     \
                                                      \
      PC_INC(pc_inc);                                 \
      if (cond) {                                     \
          dest_addr = reg_pc + (signed char)(value);  \
          reg_pc = dest_addr & 0xffff;                \
      }                                               \
      CLK_ADD(CLK, clk6809, clk6309);                 \
  } while (0)

#define BRANCH_LONG(cond, value, pc_inc, clk6809, clk6309) \
  do {                                                     \
      unsigned int dest_addr = 0;                          \
                                                           \
      PC_INC(pc_inc);                                      \
      if (cond) {                                          \
          dest_addr = reg_pc + (signed short)(value);      \
          reg_pc = dest_addr & 0xffff;                     \
          CLK_ADD(CLK, 1, 1);                              \
      }                                                    \
      CLK_ADD(CLK, clk6809, clk6309);                      \
  } while (0)

#define BSR()                           \
  do {                                  \
      PC_INC(ec);                       \
      PUSHS((reg_pc + 2) & 0xff);       \
      PUSHS((reg_pc + 2) >> 8);         \
      BRANCH(1, p1, 2, 7 + ec, 6 + ec); \
  } while (0)

#define LBSR()                                           \
  do {                                                   \
      PC_INC(ec);                                        \
      PUSHS((reg_pc + 3) & 0xff);                        \
      PUSHS((reg_pc + 3) >> 8);                          \
      BRANCH_LONG(1, (p1 << 8) | p2, 3, 8 + ec, 6 + ec); \
  } while (0)

#define CCRS()                      \
  do {                              \
      int tmp_c, tmp_h;             \
                                    \
      tmp_c = LOCAL_OVERFLOW();     \
      tmp_h = LOCAL_IRQ();          \
      reg_p = 0;                    \
      LOCAL_SET_CARRY(tmp_c);       \
      LOCAL_SET_HALF_CARRY(tmp_h);  \
      PC_INC(1 + ec);               \
      CLK_ADD(CLK, 3 + ec, 3 + ec); \
  } while (0)

#define CLR_REG(RR, pc_inc, clk6809, clk6309) \
  do {                                        \
      RR = 0;                                 \
      PC_INC(pc_inc);                         \
      CLK_ADD(CLK, clk6809, clk6309);         \
  } while (0)

#define CLR(ma, pc_inc, clk6809, clk6309) \
  do {                                    \
      STORE(ma, 0);                       \
      PC_INC(pc_inc);                     \
      CLK_ADD(CLK, clk6809, clk6309);     \
  } while (0)

#define COM_REG(RR, bits, pc_inc, clk6809, clk6309) \
  do {                                              \
      RR = ~RR;                                     \
      LOCAL_SET_CARRY(1);                           \
      LOCAL_SET_OVERFLOW(0);                        \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(pc_inc);                               \
      CLK_ADD(CLK, clk6809, clk6309);               \
  } while (0)

#define COM(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      COM_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define CMP(RR, bits, m, pc_inc, clk6809, clk6309)                       \
  do {                                                                   \
      unsigned int tmp;                                                  \
                                                                         \
      tmp = RR - m;                                                      \
      if (bits == 8) {                                                   \
          LOCAL_SET_HALF_CARRY((tmp & 0xf) < (m & 0xf));                 \
          LOCAL_SET_OVERFLOW(BT((BYTE)(RR ^ m ^ tmp ^ (tmp >> 1)), 7));  \
      } else {                                                           \
          LOCAL_SET_OVERFLOW(BT((WORD)(RR ^ m ^ tmp ^ (tmp >> 1)), 15)); \
      }                                                                  \
      LOCAL_SET_CARRY(BT(tmp, bits));                                    \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1));                             \
      LOCAL_SET_ZERO(!tmp);                                              \
      PC_INC(pc_inc);                                                    \
      CLK_ADD(CLK, clk6809, clk6309);                                    \
  } while (0)

#define CWAI(m)                       \
  do {                                \
      PC_INC(2 + ec);                 \
      reg_p &= m;                     \
      LOCAL_SET_ENTIRE(1);            \
      PUSHS(reg_pc & 0xff);           \
      PUSHS(reg_pc >> 8);             \
      PUSHS(reg_usp & 0xff);          \
      PUSHS(reg_usp >> 8);            \
      PUSHS(reg_y & 0xff);            \
      PUSHS(reg_y >> 8);              \
      PUSHS(reg_X & 0xff);            \
      PUSHS(reg_x >> 8);              \
      PUSHS(reg_dpr);                 \
      if (LOCAL_MD_E()) {             \
          PUSHS(reg_f);               \
          PUSHS(reg_e);               \
      }                               \
      PUSHS(reg_b);                   \
      PUSHS(reg_a);                   \
      PUSHS(reg_p);                   \
      CLK_ADD(CLK, 22 + ec, 20 + ec); \
      CWAI6809();                     \
  } while (0)

#define DAA()                                                       \
  do {                                                              \
      unsigned int c, lsn, msn, tmp;                                \
                                                                    \
      c = 0;                                                        \
      lsn = reg_a & 0xf;                                            \
      msn = (reg_a & 0xf0) >> 4;                                    \
      if (LOCAL_HALF_CARRY() || (lsn > 9)) {                        \
          c |= 6;                                                   \
      }                                                             \
      if (LOCAL_CARRY() || (msn > 9) || ((msn > 8) && (lsn > 9))) { \
          c |= 0x60;                                                \
      }                                                             \
      tmp = reg_a + c;                                              \
      LOCAL_SET_CARRY(BT(tmp, 8));                                  \
      reg_a = tmp & 0xff;                                           \
      LOCAL_SET_NEGATIVE(BT(reg_a, 7));                             \
      LOCAL_SET_ZERO(!reg_a);                                       \
      PC_INC(1 + ec);                                               \
      CLK_ADD(CLK, 2 + ec, 1 + ec);                                 \
  } while (0)

#define DEC_REG(RR, bits, pc_inc, clk6809, clk6309) \
  do {                                              \
      if (bits == 8) {                              \
          LOCAL_SET_OVERFLOW(RR == 0x80);           \
      } else {                                      \
          LOCAL_SET_OVERFLOW(RR == 0x8000);         \
      }                                             \
      RR--;                                         \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(pc_inc);                               \
      CLK_ADD(CLK, clk6809, clk6309);               \
  } while (0)

#define DEC(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      DEC_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define DIVD(m, pc_inc, clk6809, clk6309)          \
  do {                                             \
      SWORD val, bak;                              \
                                                   \
      if (m) {                                     \
          bak = reg_d;                             \
          val = (SWORD)reg_d / (signed char)m;     \
          reg_a = (SWORD)reg_d % (signed char)m;   \
          reg_b = val & 0xff;                      \
          LOCAL_SET_NEGATIVE(BT(reg_b, 7));        \
          LOCAL_SET_ZERO(!reg_b);                  \
          LOCAL_SET_CARRY(BT(reg_b, 0));           \
          if ((SWORD)reg_d < 0) {                  \
              LOCAL_SET_NEGATIVE(1);               \
          }                                        \
          LOCAL_SET_OVERFLOW(0);                   \
          if ((val > 127) || (v < -128)) {         \
              LOCAL_SET_OVERFLOW(1);               \
              if ((v > 255) || (v < -256)) {       \
                  LOCAL_SET_NEGATIVE(BT(bak, 15)); \
                  LOCAL_SET_ZERO(!bak);            \
                  reg_d = abs(bak);                \
              }                                    \
          }                                        \
      } else {                                     \
          DIV_0_TRAP();                            \
      }                                            \
      PC_INC(pc_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);              \
  } while (0)

#define DIVQ(m, pc_inc, clk6809, clk6309)          \
  do {                                             \
      SDWORD val, bak;                             \
                                                   \
      if (m) {                                     \
          bak = reg_q;                             \
          val = (SDWORD)reg_q / (SWORD)m;          \
          reg_d = (SDWORD)reg_d % (SWORD)m;        \
          reg_w = val & 0xffff;                    \
          LOCAL_SET_NEGATIVE(BT(reg_w, 15));       \
          LOCAL_SET_ZERO(!reg_w);                  \
          LOCAL_SET_CARRY(BT(reg_w, 0));           \
          if ((SDWORD)reg_q < 0) {                 \
              LOCAL_SET_NEGATIVE(1);               \
          }                                        \
          LOCAL_SET_OVERFLOW(0);                   \
          if ((val > 32768) || (v < -32767)) {     \
              LOCAL_SET_OVERFLOW(1);               \
              if ((v > 65536) || (v < -65535)) {   \
                  LOCAL_SET_NEGATIVE(BT(bak, 31)); \
                  LOCAL_SET_ZERO(!bak);            \
                  reg_q = abs(bak);                \
              }                                    \
          }                                        \
      } else {                                     \
          DIV_0_TRAP();                            \
      }                                            \
      PC_INC(pc_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);              \
  } while (0)

#define EIM(m, ma, pc_inc, clk6809, clk6309) \
  do {                                       \
      BYTE tmp;                              \
                                             \
      tmp = LOAD(ma);                        \
      tmp ^= m;                              \
      LOCAL_SET_OVERFLOW(0);                 \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));        \
      LOCAL_SET_ZERO(!tmp);                  \
      STORE(ma, tmp);                        \
      PC_INC(pc_inc);                        \
      CLK_ADD(CLK, clk6809, clk6309);        \
  } while (0)

#define EXG(rnr)                           \
  do {                                     \
      int mixed;                           \
      WORD tmp1, tmp2;                     \
                                           \
      if (((rnr >> 4) & 8) != (rnr & 8)) { \
          mixed = 1;                       \
      } else {                             \
          mixed = 0;                       \
      }                                    \
      REG2VAR((rnr >> 4), tmp1, mixed);    \
      REG2VAR((rnr & 0xf), tmp2, mixed);   \
      VAR2REG((rnr >> 4), tmp2, mixed);    \
      VAR2REG((rnr & 0xf), tmp1, mixed);   \
      PC_INC(2 + ec);                      \
      CLK_ADD(CLK, 8 + ec, 5 + ec);        \
  } while (0)

#define EOR(RR, bits, m, clk_inc, clk6809, clk6309) \
  do {                                              \
      RR ^= m;                                      \
      LOCAL_SET_OVERFLOW(0);                        \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(clk_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);               \
  } while (0)

#define INC_REG(RR, bits, pc_inc, clk6809, clk6309) \
  do {                                              \
      if (bits == 8) {                              \
          LOCAL_SET_OVERFLOW(RR == 0x7f);           \
      } else {                                      \
          LOCAL_SET_OVERFLOW(RR == 0x7fff);         \
      }                                             \
      RR += 1;                                      \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(pc_inc);                               \
      CLK_ADD(CLK, clk6809, clk6309);               \
  } while (0)

#define INC(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      INC_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)


/* The 0x02 AIM opcode is also used to patch the ROM.  The function trap_handler()
   returns nonzero if this is not a patch, but a `real' AIM instruction. */
#define AIM_02()                                                    \
  do {                                                              \
      DWORD trap_result;                                            \
      EXPORT_REGISTERS();                                           \
      if (!ROM_TRAP_ALLOWED()                                       \
          || (trap_result = ROM_TRAP_HANDLER()) == (DWORD)-1) {     \
          if (cpu_type == 6309) {                                   \
              AIM(p1, (reg_dpr << 8) | p2, 3, 6, 5);                \
          } else {                                                  \
              if (LOCAL_CARRY()) {                                  \
                  COM((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec); \
              } else {                                              \
                  NEG(p1 | (reg_dpr << 8), 2 + ec, 6 + ec, 5 + ec); \
              }                                                     \
          }                                                         \
      } else {                                                      \
          if (trap_result) {                                        \
             REWIND_FETCH_OPCODE(CLK);                              \
             SET_OPCODE(trap_result);                               \
             IMPORT_REGISTERS();                                    \
             goto trap_skipped;                                     \
          } else {                                                  \
             IMPORT_REGISTERS();                                    \
          }                                                         \
      }                                                             \
  } while (0)
  
#define JMP(ma, clk6809, clk6309)     \
  do {                                \
      reg_pc = ma;                    \
      CLK_ADD(CLK, clk6809, clk6309); \
  } while (0)

#define JSR(ma, pc_inc, clk6809, clk6309) \
  do {                                    \
      PC_INC(pc_inc);                     \
      PUSHS(reg_pc & 0xff);               \
      PUSHS(reg_pc >> 8);                 \
      reg_pc = ma;                        \
      CLK_ADD(CLK, clk6809, clk6309);     \
  } while (0)

#define LD(RR, bits, m, pc_inc, clk6809, clk6309) \
  do {                                            \
      RR = m;                                     \
      LOCAL_SET_OVERFLOW(0);                      \
      LOCAL_SET_ZERO(!RR);                        \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));       \
      PC_INC(pc_inc);                             \
      CLK_ADD(CLK, clk6809, clk6309);             \
  } while (0)

#define LDBT(rnr, ma)                            \
  do {                                           \
      BYTE rr;                                   \
      BYTE tmp;                                  \
      unsigned int sbit = (rnr >> 3) & 7;        \
      unsigned int dbit = rnr & 7;               \
      unsigned int stmp;                         \
                                                 \
      BTM_REG2VAR(rnr, rr);                      \
      tmp = LOAD(ma);                            \
      stmp = (tmp & (1 << sbit)) ? 1 : 0;        \
      rr = (rr & ~(1 << dbit)) | (stmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                     \
      LOCAL_SET_NEGATIVE(BT(rr, 7));             \
      LOCAL_SET_ZERO(!rr);                       \
      BTM_VAR2REG(rnr, rr);                      \
      PC_INC(4);                                 \
      CLK_ADD(CLK, 7, 6);                        \
  } while (0)

#define LEA(RR, ma)                 \
  do {                              \
      RR = ma;                      \
      LOCAL_SET_ZERO(!RR);          \
      PC_INC(2 + ec);               \
      CLK_ADD(CLK, 4 + ec, 4 + ec); \
  } while (0)

#define LSR_REG(RR, pc_inc, clk6809, clk6309) \
  do {                                        \
      LOCAL_SET_CARRY(BT(RR, 0));             \
      RR >>= 1;                               \
      LOCAL_SET_NEGATIVE(0);                  \
      LOCAL_SET_ZERO(!RR);                    \
      PC_INC(pc_inc);                         \
      CLK_ADD(CLK, clk6809, clk6309);         \
  } while (0)

#define LSR(ma, pc_inc, clk6809, clk6309)     \
  do {                                        \
      BYTE val;                               \
                                              \
      val = LOAD(ma);                         \
      LSR_REG(val, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                         \
  } while (0)

#define MUL()                         \
  do {                                \
      reg_d = reg_a * reg_b;          \
      LOCAL_SET_CARRY(BT(reg_b, 7));  \
      LOCAL_SET_ZERO(!reg_d);         \
      PC_INC(1 + ec);                 \
      CLK_ADD(CLK, 11 + ec, 10 + ec); \
  } while (0)

#define MULD(m, pc_inc, clk6809, clk6309) \
  do {                                    \
      reg_q = reg_d * m;                  \
      LOCAL_SET_OVERFLOW(0);              \
      LOCAL_SET_CARRY(0);                 \
      LOCAL_SET_NEGATIVE(BT(reg_d, 15));  \
      LOCAL_SET_ZERO(!reg_d);             \
      PC_INC(pc_inc);                     \
      CLK_ADD(CLK, clk6809, clk6309);     \
  } while (0)

#define NEG_REG(RR, bits, pc_inc, clk6809, clk6309)     \
  do {                                                  \
      unsigned int tmp;                                 \
                                                        \
      LOCAL_SET_OVERFLOW(RR == (1 << (bits -1)));       \
      tmp = ((~RR) & (bits == 8) ? 0xff : 0xffff) + 1;  \        
      RR = tmp & (bits == 8) ? 0xff " 0xffff;           \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));             \
      LOCAL_SET_ZERO(!RR);                              \
      LOCAL_SET_CARRY(RR);                              \
      PC_INC(pc_inc);                                   \
      CLK_INC(CLK, clk6809, clk6309);                   \
  } while (0)

#define NEG(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      NEG_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define NOP()                       \
  do {                              \
      PC_INC(1 + ec);               \
      CLK_ADD(CLK, 2 + ec, 1 + ec); \
  } while (0)

#define OIM(m, ma, pc_inc, clk6809, clk6309) \
  do {                                       \
      BYTE tmp;                              \
                                             \
      tmp = LOAD(ma);                        \
      tmp |= m;                              \
      LOCAL_SET_OVERFLOW(0);                 \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));        \
      LOCAL_SET_ZERO(!tmp);                  \
      STORE(ma, tmp);                        \
      PC_INC(pc_inc);                        \
      CLK_ADD(CLK, clk6809, clk6309);        \
  } while (0)

#define OR(RR, bits, m, pc_inc, clk6809, clk6309) \
  do {                                            \
      RR |= m;                                    \
      LOCAL_SET_OVERFLOW(0);                      \
      LOCAL_SET_ZERO(!RR);                        \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));       \
      PC_INC(pc_inc);                             \
      CLK_ADD(CLK, clk6809, clk6309);             \
  } while (0)

#define PSHS(m)                     \
  do {                              \
      PC_INC(2 + ec);               \
      if (m & 0x80) {               \
          PUSHS(reg_pc & 0xff);     \
          PUSHS(reg_pc >> 8);       \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x40) {               \
          PUSHS(reg_usp & 0xff);    \
          PUSHS(reg_usp >> 8);      \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x20) {               \
          PUSHS(reg_y & 0xff);      \
          PUSHS(reg_y >> 8);        \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x10) {               \
          PUSHS(reg_x & 0xff);      \
          PUSHS(reg_x >> 8);        \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 8) {                  \
          PUSHS(reg_dpr);           \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 4) {                  \
          PUSHS(reg_b);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 2) {                  \
          PUSHS(reg_a);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 1) {                  \
          PUSHS(reg_p);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      CLK_ADD(CLK, 5 + ec, 4 + ec); \
  } while (0)

#define PSHSW()           \
  do {                    \
      PUSHS(reg_f);       \
      PUSHS(reg_e);       \
      PC_INC(2);          \
      CLK_ADD(CLK, 6, 6); \
  } while (0)

#define PSHU(m)                     \
  do {                              \
      PC_INC(2 + ec);               \
      if (m & 0x80) {               \
          PUSHU(reg_pc & 0xff);     \
          PUSHU(reg_pc >> 8);       \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x40) {               \
          PUSHU(reg_ssp & 0xff);    \
          PUSHU(reg_ssp >> 8);      \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x20) {               \
          PUSHU(reg_y & 0xff);      \
          PUSHU(reg_y >> 8);        \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x10) {               \
          PUSHU(reg_x & 0xff);      \
          PUSHU(reg_x >> 8);        \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 8) {                  \
          PUSHU(reg_dpr);           \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 4) {                  \
          PUSHU(reg_b);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 2) {                  \
          PUSHU(reg_a);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 1) {                  \
          PUSHU(reg_p);             \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      CLK_ADD(CLK, 5 + ec, 4 + ec); \
  } while (0)

#define PSHUW()           \
  do {                    \
      PUSHU(reg_f);       \
      PUSHU(reg_e);       \
      PC_INC(2);          \
      CLK_ADD(CLK, 6, 6); \
  } while (0)

#define PULS(m)                     \
  do {                              \
      if (m & 1) {                  \
          reg_p = PULLS();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 2) {                  \
          reg_a = PULLS();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 4) {                  \
          reg_b = PULLS();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 8) {                  \
          reg_dpr = PULLS();        \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 0x10) {               \
          reg_x = PULLS() << 8;     \
          reg_x |= PULLS();         \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x20) {               \
          reg_y = PULLS() << 8;     \
          reg_y |= PULLS();         \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x40) {               \
          reg_usp = PULLS() << 8;   \
          reg_usp |= PULLS();       \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x80) {               \
          reg_pc = PULLS() << 8;    \
          reg_pc |= PULLS();        \
          CLK_ADD(CLK, 2, 2);       \
      } else {                      \
          PC_INC(2 + ec);           \
      }                             \
      CLK_ADD(CLK, 5 + ce, 4 + ce); \
  } while (0)

#define PULSW()           \
  do {                    \
      reg_e = PULLS();    \
      reg_f = PULLS();    \
      PC_INC(2);          \
      CLK_ADD(CLK, 6, 6); \
  } while (0)

#define PULU(m)                     \
  do {                              \
      if (m & 1) {                  \
          reg_p = PULLU();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 2) {                  \
          reg_a = PULLU();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 4) {                  \
          reg_b = PULLU();          \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 8) {                  \
          reg_dpr = PULLU();        \
          CLK_ADD(CLK, 1, 1);       \
      }                             \
      if (m & 0x10) {               \
          reg_x = PULLU() << 8;     \
          reg_x |= PULLU();         \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x20) {               \
          reg_y = PULLU() << 8;     \
          reg_y |= PULLU();         \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x40) {               \
          reg_ssp = PULLU() << 8;   \
          reg_ssp |= PULLU();       \
          CLK_ADD(CLK, 2, 2);       \
      }                             \
      if (m & 0x80) {               \
          reg_pc = PULLU() << 8;    \
          reg_pc |= PULLU();        \
          CLK_ADD(CLK, 2, 2);       \
      } else {                      \
          PC_INC(2 + ec);           \
      }                             \
      CLK_ADD(CLK, 5 + ec, 4 + ec); \
  } while (0)

#define PULUW()           \
  do {                    \
      reg_e = PULLU();    \
      reg_f = PULLU();    \
      PC_INC(2);          \
      CLK_ADD(CLK, 6, 6); \
  } while (0)

#define ROL_REG(RR, bits, pc_inc, clk6809, clk6309)            \
  do {                                                         \
      unsigned int tmp;                                        \
                                                               \
      tmp = LOCAL_CARRY();                                     \
      LOCAL_SET_OVERFLOW(BT(RR, bits - 1) ^ BT(RR, bits - 2)); \
      LOCAL_SET_CARRY(BT(RR, bits - 1));                       \
      RR <<= 1;                                                \
      RR |= tmp;                                               \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));                    \
      LOCAL_SET_ZERO(!RR);                                     \
      PC_INC(pc_inc);                                          \
      CLK_ADD(CLK, clk6809, clk6309);                          \
  } while (0)

#define ROL(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      ROL_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define ROR_REG(RR, bits, pc_inc, clk6809, clk6309) \
  do {                                              \
      unsigned int tmp;                             \
                                                    \
      tmp = LOCAL_CARRY();                          \
      RR >>= 1;                                     \
      RR |= (tmp << (bits - 1));                    \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(pc_inc);                               \
      CLK_ADD(CLK, clk6809, clk6309);               \
  } while (0)

#define ROR(ma, pc_inc, clk6809, clk6309)        \
  do {                                           \
      BYTE val;                                  \
                                                 \
      val = LOAD(ma);                            \
      ROR_REG(val, 8, pc_inc, clk6809, clk6309); \
      STORE(ma, val);                            \
  } while (0)

#define RTI()                       \
  do {                              \
      reg_p = PULLS();              \
      if (LOCAL_ENTIRE()) {         \
          reg_a = PULLS();          \
          reg_b = PULLS();          \
          if (LOCAL_MD_E()) {       \
              reg_e = PULLS();      \
              reg_f = PULLS();      \
              CLK_ADD(CLK, 2, 2);   \
          }                         \
          reg_dpr = PULLS();        \
          reg_x = PULLS() << 8;     \
          reg_x |= PULLS();         \
          reg_y = PULLS() << 8;     \
          reg_y |= PULLS();         \
          reg_usp = PULLS() << 8;   \
          reg_usp |= PULLS();       \
          CLK_ADD(CLK, 9, 9);       \
      }                             \
      reg_pc = PULLS() << 8;        \
      reg_pc |= PULLS();            \
      CLK_ADD(CLK, 6 + ec, 6 + ec); \
  } while (0)

#define RTS()                       \
  do {                              \
      reg_pc = PULLS() << 8;        \
      reg_pc |= PULLS();            \
      CLK_ADD(CLK, 5 + ec, 1 + ec); \
  } while (0)

#define SCC()                       \
  do {                              \
      LOCAL_SET_NEGATIVE(1);        \
      LOCAL_SET_ZERO(0);            \
      LOCAL_SET_OVERFLOW(0);        \
      PC_INC(2 + ec);               \
      CLK_ADD(CLK, 2 + ec, 2 + ec); \
  } while (0)

#define SEX()                              \
  do {                                     \
      LOCAL_SET_NEGATIVE(BT(reg_b, 7));    \
      reg_a = LOCAL_NEGATIVE() ? 0xff : 0; \
      LOCAL_SET_ZERO(!reg_a);              \
      PC_INC(1 + ec);                      \
      CLK_ADD(CLK, 2 + ec, 1 + ec);        \
  } while (0)

#define SEXW()                               \
  do {                                       \
      LOCAL_SET_NEGATIVE(BT(reg_w, 15));     \
      reg_d = LOCAL_NEGATIVE() ? 0xffff : 0; \
      LOCAL_SET_ZERO(!reg_d);                \
      PC_INC(1);                             \
      CLK_ADD(CLK, 4, 4);                    \
  } while (0)

#define ST(RR, bits, ma, pc_inc, clk6809, clk6309) \
  do {                                             \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));        \
      LOCAL_SET_ZERO(!RR);                         \
      if (bits == 8) {                             \
          STORE(ma, RR);                           \
      }                                            \
      if (bits == 16) {                            \
          STORE(ma, RR >> 8);                      \
          STORE(ma + 1, RR & 0xff);                \
      }                                            \
      if (bits == 32) {                            \
          STORE(ma, RR >> 24);                     \
          STORE(ma + 1, (RR >> 16) & 0xff);        \
          STORE(ma + 2, (RR >> 8) & 0xff);         \
          STORE(ma + 3, RR & 0xff);                \
      }                                            \
      PC_INC(pc_inc);                              \
      CLK_ADD(CLK, clk6809, clk6309);              \
  } while (0)

#define STBT(rnr, ma)                              \
  do {                                             \
      BYTE rr;                                     \
      BYTE tmp;                                    \
      unsigned int sbit = (rnr >> 3) & 7;          \
      unsigned int dbit = rnr & 7;                 \
      unsigned int stmp;                           \
                                                   \
      BTM_REG2VAR(rnr, rr);                        \
      tmp = LOAD(ma);                              \
      stmp = (rr & (1 << sbit)) ? 1 : 0;           \
      tmp = (tmp & ~(1 << dbit)) | (stmp << dbit); \
      LOCAL_SET_OVERFLOW(0);                       \
      LOCAL_SET_NEGATIVE(BT(tmp, 7));              \
      LOCAL_SET_ZERO(!tmp);                        \
      STORE(ma, tmp);                              \
      CLK_INC(4);                                  \
      CLK_ADD(CLK, 8, 7);                          \
  } while (0)

#define STI(RR)                     \
  do {                              \
      INC_PC(2 + ec);               \
      STORE(reg_pc, RR & 0xff);     \
      LOCAL_SET_NEGATIVE(1);        \
      LOCAL_SET_ZERO(0);            \
      LOCAL_SET_OVERFLOW(0);        \
      INC_PC(1);                    \
      CLK_ADD(CLK, 3 + ec, 3 + ec); \
  } while (0)

#define SUB(RR, CC, m, bits, pc_inc, clk6809, clk6309)                   \
  do {                                                                   \
      unsigned int tmp;                                                  \
                                                                         \
      tmp = RR - m - CC;                                                 \
      if (bits == 8) {                                                   \
          LOCAL_SET_HALF_CARRY((tmp & 0xf) < (m & 0xf));                 \
          LOCAL_SET_OVERFLOW(BT((BYTE)(RR ^ m ^ tmp ^ (tmp >> 1)), 7));  \
      } else {                                                           \
          LOCAL_SET_OVERFLOW(BT((WORD)(RR ^ m ^ tmp ^ (tmp >> 1)), 15)); \
      }                                                                  \
      LOCAL_SET_CARRY(BT(tmp, bits));                                    \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1));                             \
      LOCAL_SET_ZERO(!tmp);                                              \
      RR = tmp & (bits == 8) ? 0xff : 0xffff;                            \
      PC_INC(pc_inc);                                                    \
      CLK_ADD(CLK, clk6809, clk6309);                                    \
  } while (0)

#define SWI_REG(nr, pc_inc)               \
  do {                                    \
      PC_INC(pc_inc);                     \
      if (nr) {                           \
          LOCAL_SET_ENTIRE(1);            \
      }                                   \
      PUSHS(reg_pc & 0xff);               \
      PUSHS(reg_pc >> 8);                 \
      PUSHS(reg_usp & 0xff);              \
      PUSHS(reg_usp >> 8);                \
      PUSHS(reg_y & 0xff);                \
      PUSHS(reg_y >> 8);                  \
      PUSHS(reg_x & 0xff);                \
      PUSHS(reg_x >> 8);                  \
      PUSHS(reg_dpr);                     \
      if (LOCAL_MD_E()) {                 \
          PUSHS(reg_f);                   \
          PUSHS(reg_e);                   \
      }                                   \
      PUSHS(reg_b);                       \
      PUSHS(reg_a);                       \
      PUSHS(reg_p);                       \
      if (nr == 0) {                      \
          LOCAL_SET_FIRQ(1);              \
          LOCAL_SET_IRQ(1);               \
          reg_pc = LOAD(0xfffe) << 8;     \
          reg_pc |= LOAD(0xffff);         \
          CLK_ADD(CLK, 19 + ec, 21 + ec); \
      }                                   \
      if (nr == 1) {                      \
          LOCAL_SET_FIRQ(1);              \
          LOCAL_SET_IRQ(1);               \
          reg_pc = LOAD(0xfffa) << 8;     \
          reg_pc |= LOAD(0xfffb);         \
          CLK_ADD(CLK, 19, 21);           \
      }                                   \
      if (nr == 2) {                      \
          reg_pc = LOAD(0xfff4) << 8;     \
          reg_pc |= LOAD(0xfff5);         \
          CLK_ADD(CLK, 20, 22);           \
      }                                   \
      if (nr == 3) {                      \
          reg_pc = LOAD(0xfff2) << 8;     \
          reg_pc |= LOAD(0xfff3);         \
          CLK_ADD(CLK, 20, 22);           \
      }                                   \
  } while (0)

#define SYNC()                      \
  do {                              \
      PC_INC(1 + ec);               \
      CLK_ADD(CLK, 2 + ec, 1 + ec); \
      SYNC6809();                   \
  while (0)

#define TFR(rnr)                           \
  do {                                     \
      int mixed;                           \
      WORD tmp;                            \
                                           \
      if (((rnr >> 4) & 8) != (rnr & 8)) { \
          mixed = 1;                       \
      } else {                             \
          mixed = 0;                       \
      }                                    \
      REG2VAR((rnr >> 4), tmp, mixed);     \
      VAR2REG((rnr & 0xf), tmp, mixed);    \
      PC_INC(2 + ec);                      \
      CLK_ADD(CLK, 6 + ec, 4 + ec);        \
  } while (0)

#define TIM(m, ma, pc_inc, clk6809, clk6309) \
  do {                                       \
      LOCAL_SET_OVERFLOW(0);                 \
      LOCAL_SET_NEGATIVE(BT(m, 7));          \
      LOCAL_SET_ZERO(!m);                    \
      STORE(ma, m);                          \
      PC_INC(pc_inc);                        \
      CLK_ADD(CLK, clk6809, clk6309);        \
  } while (0)

#define TST_REG(RR, bits, pc_inc, clk6809, clk6309) \
  do {                                              \
      LOCAL_SET_OVERFLOW(0);                        \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));         \
      LOCAL_SET_ZERO(!RR);                          \
      PC_INC(pc_inc);                               \
      CLK_ADD(CLK, clk6809, clk6309);               \
  while (0)

#define TST(ma, pc_inc, clk6809, clk6309) \
  (TST_REG(LOAD(ma), 8), pc_inc, clk6809, clk6309)

/* ------------------------------------------------------------------------- */

#define ADCR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (ADD(r1, r0, LOCAL_CARRY(), bits, pc_inc, clk6809, clk6309))

#define ADDR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (ADD(r1, r0, 0, bits, pc_inc, clk6809, clk6309))

#define ANDR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (AND(r1, r0, bits, pc_inc, clk6809, clk6309))

#define CMPR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (CMP(r1, bits, r0, pc_inc, clk6809, clk6309))

#define EORR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (EOR(r1, bits, r0, pc_inc, clk6809, clk6309))

#define ORR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (OR(r1, bits, r0, pc_inc, clk6809, clk6309))

#define SBCR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (SUB(r1, LOCAL_CARRY(), r0, bits, pc_inc, clk6809, clk6309))

#define SUBR_WRAP(r0, r1, bits, pc_inc, clk6809, clk6309) \
  (SUB(r1, 0, r0, bits, pc_inc, clk6809, clk6309))

#define TFMPP(rnr) \
   (TFM_WRAP(rnr, TFMPP_REG))

#define TFMMM(rnr) \
   (TFM_WRAP(rnr, TFMMM_REG))

#define TFMPC(rnr) \
   (TFM_WRAP(rnr, TFMPC_REG))

#define TFMCP(rnr) \
   (TFM_WRAP(rnr, TFMCP_REG))

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
        int ec;

        SET_LAST_ADDR(reg_pc);
        FETCH_OPCODE(opcode, page);

        ec = (page) ? 1 : 0;
trap_skipped:
        switch ((page << 8) | p0 | (cpu_type == 6809) ? 0x80000 : 0x30000) {

          default:
            ILLEGAL_OPCODE_TRAP();
            break

          case 0x30000:         /* NEG direct */
          case 0x80000:         /* NEG direct */
          case 0x80001:         /* NEG direct (illegal 6809) */
          case 0x81000:         /* NEG direct (illegal 6809) */
          case 0x81001:         /* NEG direct (illegal 6809) */
            NEG(p1 | (reg_dpr << 8), 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30001:         /* OIM IM-direct */
            OIM(p1, (reg_dpr << 8) | p2, 3, 6, 5);
            break;

          case 0x30002:         /* AIM IM-direct, also used for traps */
          case 0x80002:         /* NEG/COM direct (6809 illegal) */
          case 0x81002:         /* NEG/COM direct (6809 illegal) */
            STATIC_ASSERT(TRAP_OPCODE == 0x02);
            AIM_02();
            break;

          case 0x30003:         /* COM direct */
          case 0x80003:         /* COM direct */
          case 0x81003:         /* COM direct (6809 illegal) */
            COM((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30004:         /* LSR direct */
          case 0x80004:         /* LSR direct */
          case 0x80005:         /* LSR direct (6809 illegal) */
          case 0x81004:         /* LSR direct (6809 illegal) */
          case 0x81005:         /* LSR direct (6809 illegal) */
            LSR((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30005:         /* EIM IM-direct */
            EIM(p1, (reg_dpr << 8) | p2, 3, 6, 5);
            break;

          case 0x30006:         /* ROR direct */
          case 0x80006:         /* ROR direct */
          case 0x81006:         /* ROR direct (6809 illegal) */
            ROR((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30007:         /* ASR direct */
          case 0x80007:         /* ASR direct */
          case 0x81007:         /* ASR direct (6809 illegal) */
            ASR((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30008:         /* ASL/LSL direct */
          case 0x80008:         /* ASL/LSL direct */
          case 0x81008:         /* ASL/LSL direct (6809 illegal) */
            ASL((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30009:         /* ROL direct */
          case 0x80009:         /* ROL direct */
          case 0x81009:         /* ROL direct (6809 illegal) */
            ROL((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x3000a:         /* DEC direct */
          case 0x8000a:         /* DEC direct */
          case 0x8000b:         /* DEC direct (6809 illegal) */
          case 0x8100a:         /* DEC direct (6809 illegal) */
          case 0x8100b:         /* DEC direct (6809 illegal) */
            DEC((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x3000b:         /* TIM IM-direct */
            TIM(p1, (reg_dpr << 8) | p2, 3, 6, 6);
            break;

          case 0x3000c:         /* INC direct */
          case 0x8000c:         /* INC direct */
          case 0x8100c:         /* INC direct (6809 illegal) */
            INC((reg_dpr << 8) | p1, 2 + ce, 6 + ce, 5 + ce);
            break;

          case 0x3000d:         /* TST direct */
          case 0x8000d:         /* TST direct */
          case 0x8100d:         /* TST direct */
            TST((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 4 + ec);
            break;

          case 0x3000e:         /* JMP direct */
          case 0x8000e:         /* JMP direct */
          case 0x8100e:         /* JMP direct (6809 illegal) */
            JMP((reg_dpr << 8) | p1, 3 + ec, 2 + ec);
            break;

          case 0x3000f:         /* CLR direct */
          case 0x8000f:         /* CLR direct */
          case 0x8100f:         /* CLR direct (6809 illegal) */
            CLR((reg_dpr << 8) | p1, 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x30012:         /* NOP */
          case 0x80012:         /* NOP */
          case 0x8001b:         /* NOP (6809 illegal) */
          case 0x81012:         /* NOP (6809 illegal) */
          case 0x8101b:         /* NOP (6809 illegal) */
            NOP();
            break;

          case 0x30013:         /* SYNC */
          case 0x80013:         /* SYNC */
          case 0x81013:         /* SYNC (6809 illegal) */
            SYNC();
            break;

          case 0x30014:         /* SEXW */
            SEXW();
            break

          case 0x80014:         /* HCF (6809 illegal) */
          case 0x80015:         /* HCF (6809 illegal) */
          case 0x800cd:         /* HCF (6809 illegal) */
          case 0x81014:         /* HCF (6809 illegal) */
          case 0x81015:         /* HCF (6809 illegal) */
            HCF();
            break;

          case 0x30016:         /* LBRA offset */
          case 0x80016:         /* LBRA offset */
          case 0x81016:         /* LBRA offset (6809 illegal) */
          case 0x81020:         /* LBRA offset (6809 illegal) */
            BRANCH_LONG(1, (p1 << 8) | p2, 3 + ec, 5 + ec, 4 + ec);
            break;

          case 0x30017:         /* LBSR offset */
          case 0x80017:         /* LBSR offset */
          case 0x81017:         /* LBSR offset (6809 illegal) */
            LBSR();
            break;

          case 0x80018:         /* CCRS (6809 illegal) */
          case 0x81018:         /* CCRS (6809 illegal) */
            CCRS();
            break;

          case 0x30019:         /* DAA */
          case 0x80019:         /* DAA */
          case 0x81019:         /* DAA (6809 illegal) */
            DAA();
            break;

          case 0x3001a:         /* ORCC immediate */
          case 0x8001a:         /* ORCC immediate */
          case 0x8101a:         /* ORCC immediate (6809 illegal) */
            OR(reg_p, 8, p1, 2 + ec, 3 + ec, 2 + ec);
            break;

          case 0x3001c:         /* ANDCC immediate */
          case 0x8001c:         /* ANDCC immediate */
          case 0x8101c:         /* ANDCC immediate (6809 illegal) */
            AND(reg_p, p1, 8, 2 + ec, 3 + ec, 3 + ec);
            break;

          case 0x3001d:         /* SEX (Sign EXtend, not the other thing ;) */
          case 0x8001d:         /* SEX */
          case 0x8101d:         /* SEX (6809 illegal) */
            SEX();
            break;

          case 0x3001e:         /* EXG registers */
          case 0x8001e:         /* EXG registers */
          case 0x8101e:         /* EXG registers */
            EXG(p1);
            break;

          case 0x3001f:         /* TFR registers */
          case 0x8001f:         /* TFR registers */
          case 0x8101f:         /* TFR registers (6809 illegal) */
            TFR(p1);
            break;

          case 0x30020:         /* BRA offset */
          case 0x80020:         /* BRA offset */
            BRANCH(1, p1, 2, 3, 2);
            break;

          case 0x30021:         /* BRN offset */
          case 0x80021:         /* BRN offset */
            BRANCH(0, p1, 2, 3, 2);
            break;

          case 0x30022:         /* BHI offset */
          case 0x80022:         /* BHI offset */
            BRANCH(!(LOCAL_CARRY() | LOCAL_ZERO()), p1, 2, 3, 2);
            break;

          case 0x30023:         /* BLS offset */
          case 0x80023:         /* BLS offset */
            BRANCH(LOCAL_CARRY() | LOCAL_ZERO(), p1, 2, 3, 2);
            break;

          case 0x30024:         /* BHS/BCC offset */
          case 0x80024:         /* BHS/BCC offset */
            BRANCH(!LOCAL_CARRY(), p1, 2, 3, 2);
            break;

          case 0x30025:         /* BLO/BCS offset */
          case 0x80025:         /* BLO/BCS offset */
            BRANCH(LOCAL_CARRY(), p1, 2, 3, 2);
            break;

          case 0x30026:         /* BNE offset */
          case 0x80026:         /* BNE offset */
            BRANCH(!LOCAL_ZERO(), p1, 2, 3, 2);
            break;

          case 0x30027:         /* BEQ offset */
          case 0x80027:         /* BEQ offset */
            BRANCH(LOCAL_ZERO(), p1, 2, 3, 2);
            break;

          case 0x30028:         /* BVC offset */
          case 0x80028:         /* BVC offset */
            BRANCH(!LOCAL_OVERFLOW(), p1, 2, 3, 2);
            break;

          case 0x30029:         /* BVS offset */
          case 0x80029:         /* BVS offset */
            BRANCH(LOCAL_OVERFLOW(), p1, 2, 3, 2);
            break;

          case 0x3002a:         /* BPL offset */
          case 0x8002a:         /* BPL offset */
            BRANCH(!LOCAL_NEGATIVE(), p1, 2, 3, 2);
            break;

          case 0x3002b:         /* BMI offset */
          case 0x8002b:         /* BMI offset */
            BRANCH(LOCAL_NEGATIVE(), p1, 2, 3, 2);
            break;

          case 0x3002c:         /* BGE offset */
          case 0x8002c:         /* BGE offset */
            BRANCH(!(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), p1, 2, 3, 2);
            break;

          case 0x3002d:         /* BLT offset */
          case 0x8002d:         /* BLT offset */
            BRANCH(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW(), p1, 2, 3, 2);
            break;

          case 0x3002e:         /* BGT offset */
          case 0x8002e:         /* BGT offset */
            BRANCH((!LOCAL_ZERO() & !(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW())), p1, 2, 3, 2);
            break;

          case 0x3002f:         /* BLE offset */
          case 0x8002f:         /* BLE offset */
            BRANCH(LOCAL_ZERO() | (LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), p1, 2, 3, 2);
            break;

          case 0x30030:         /* LEAX indexed */
          case 0x80030:         /* LEAX indexed */
          case 0x81030:         /* LEAX indexed (6809 illegal) */
            LEA(reg_x, GET_IND_MA(p1, p2, p3));
            break;

          case 0x30031:         /* LEAY indexed */
          case 0x80031:         /* LEAY indexed */
          case 0x81031:         /* LEAY indexed (6809 illegal) */
            LEA(reg_y, GET_IND_MA(p1, p2, p3));
            break;

          case 0x30032:         /* LEAS indexed */
          case 0x80032:         /* LEAS indexed */
          case 0x81032:         /* LEAS indexed (6809 illegal) */
            LEA(reg_ssp, GET_IND_MA(p1, p2, p3));
            break;

          case 0x30033:         /* LEAU indexed */
          case 0x80033:         /* LEAU indexed */
          case 0x81033:         /* LEAU indexed (6809 illegal) */
            LEA(reg_usp, GET_IND_MA(p1, p2, p3));
            break;

          case 0x30034:         /* PSHS immediate */
          case 0x80034:         /* PSHS immediate */
          case 0x81034:         /* PSHS immediate (6809 illegal) */
            PSHS(p1);
            break;

          case 0x30035:         /* PULS immediate */
          case 0x80035:         /* PULS immediate */
          case 0x81035:         /* PULS immediate (6809 illegal) */
            PULS(p1);
            break;

          case 0x30036:         /* PSHU immediate */
          case 0x80036:         /* PSHU immediate */
          case 0x81036:         /* PSHU immediate (6809 illegal) */
            PSHU(p1);
            break;

          case 0x30037:         /* PULU immediate */
          case 0x80037:         /* PULU immediate */
          case 0x81037:         /* PULU immediate (6809 illegal) */
            PULU(p1);
            break;

          case 0x80038:         /* ANDCC immediate (+1 cycle) (6809 illegal) */
          case 0x81038:         /* ANDCC immediate (+1 cycle) (6809 illegal) */
            AND(reg_p, p1, 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x30039:         /* RTS */
          case 0x80039:         /* RTS */
          case 0x81039:         /* RTS (6809 illegal) */
            RTS();
            break;

          case 0x3003a:         /* ABX */
          case 0x8003a:         /* ABX */
          case 0x8103a:         /* ABX (6809 illegal) */
            ABX();
            break;

          case 0x3003b:         /* RTI */
          case 0x8003b:         /* RTI */
          case 0x8103b:         /* RTI (6809 illegal) */
            RTI();
            break;

          case 0x3003c:         /* CWAI immediate */
          case 0x8003c:         /* CWAI immediate */
          case 0x8103c:         /* CWAI immediate */
            CWAI(p1);
            break;

          case 0x3003d:         /* MUL */
          case 0x8003d:         /* MUL */
          case 0x8103d:         /* MUL (6809 illegal) */
            MUL();
            break;

          case 0x8003e:         /* like SWI but using reset vector (6809 illegal) */
          case 0x8103e:         /* like SWI but using reset vector (6809 illegal) */
            SWI_REG(0, 2);
            break;

          case 0x3003f:         /* SWI */
          case 0x8003f:         /* SWI */
            SWI_REG(1, 1);
            break;

          case 0x30040:         /* NEGA */
          case 0x80040:         /* NEGA */
          case 0x80041:         /* NEGA (6809 illegal) */
          case 0x81040:         /* NEGA (6809 illegal) */
          case 0x81041:         /* NEGA (6809 illegal) */
            NEG_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x80042:          /* NEGA / COMA (6809 illegal)
          case 0x81042:          /* NEGA / COMA (6809 illegal)
            if (!LOCAL_CARRY()) {
                NEG_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            } else {
                COM_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            }
            break;

          case 0x30043:         /* COMA */
          case 0x80043:         /* COMA */
          case 0x81043:         /* COMA (6809 illegal) */
            COM_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30044:         /* LSRA */
          case 0x80044:         /* LSRA */
          case 0x80045:         /* LSRA (6809 illegal) */
          case 0x81044:         /* LSRA (6809 illegal) */
          case 0x81045:         /* LSRA (6809 illegal) */
            LSR_REG(reg_a, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30046:         /* RORA */
          case 0x80046:         /* RORA */
          case 0x81046:         /* RORA (6809 illegal) */
            ROR_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30047:         /* ASRA */
          case 0x80047:         /* ASRA */
          case 0x81047:         /* ASRA (6809 illegal) */
            ASR_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30048:         /* ASLA/LSLA */
          case 0x80048:         /* ASLA/LSLA */
          case 0x81048:         /* ASLA/LSLA (6809 illegal) */
            ASL_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30049:         /* ROLA */
          case 0x80049:         /* ROLA */
          case 0x81049:         /* ROLA (6809 illegal) */
            ROL_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3004a:         /* DECA */
          case 0x8004a:         /* DECA */
          case 0x8004b:         /* DECA (6809 illegal) */
          case 0x8104a:         /* DECA (6809 illegal) */
          case 0x8104b:         /* DECA (6809 illegal) */
            DEC_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3004c:         /* INCA */
          case 0x8004c:         /* INCA */
          case 0x8104c:         /* INCA (6809 illegal) */
            INC_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3004d:         /* TSTA */
          case 0x8004d:         /* TSTA */
          case 0x8104d:         /* TSTA (6809 illegal) */
            TST_REG(reg_a, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3004f:         /* CLRA */
          case 0x8004f:         /* CLRA */
          case 0x8004e:         /* CLRA (6809 illegal) */
          case 0x8104e:         /* CLRA (6809 illegal) */
          case 0x8104f:         /* CLRA (6809 illegal) */
            CLR_REG(reg_a, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30050:         /* NEGB */
          case 0x80050:         /* NEGB */
          case 0x80051:         /* NEGB (6809 illegal) */
          case 0x81050:         /* NEGB (6809 illegal) */
          case 0x81051:         /* NEGB (6809 illegal) */
            NEG_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

        case 0x80052:          /* NEGB / COMB (6809 illegal) */
        case 0x81052:          /* NEGB / COMB (6809 illegal) */
            if (!LOCAL_CARRY()) {
                NEG_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            } else {
                COM_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            }
            break;

          case 0x30053:         /* COMB */
          case 0x80053:         /* COMB */
          case 0x81053:         /* COMB (6809 illegal) */
            COM_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30054:         /* LSRB */
          case 0x80054:         /* LSRB */
          case 0x80055:         /* LSRB (6809 illegal) */
          case 0x81054:         /* LSRB (6809 illegal) */
          case 0x81055:         /* LSRB (6809 illegal) */
            LSR_REG(reg_b, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30056:         /* RORB */
          case 0x80056:         /* RORB */
          case 0x81056:         /* RORB (6809 illegal) */
            ROR_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30057:         /* ASRB */
          case 0x80057:         /* ASRB */
          case 0x81057:         /* ASRB (6809 illegal) */
            ASR_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30058:         /* ASLB/LSLB */
          case 0x80058:         /* ASLB/LSLB */
          case 0x81058:         /* ASLB/LSLB (6809 illegal) */
            ASL_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30059:         /* ROLB */
          case 0x80059:         /* ROLB */
          case 0x81059:         /* ROLB (6809 illegal) */
            ROL_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3005a:         /* DECB */
          case 0x8005a:         /* DECB */
          case 0x8005b:         /* DECB (6809 illegal) */
          case 0x8105a:         /* DECB (6809 illegal) */
          case 0x8105b:         /* DECB (6809 illegal) */
            DEC_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3005c:         /* INCB */
          case 0x8005c:         /* INCB */
          case 0x8105c:         /* INCB (6809 illegal) */
            INC_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3005d:         /* TSTB */
          case 0x8005d:         /* TSTB */
          case 0x8105d:         /* TSTB (6809 illegal) */
            TST_REG(reg_b, 8, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x3005f:         /* CLRB */
          case 0x8005f:         /* CLRB */
          case 0x8005e:         /* CLRB (6809 illegal) */
          case 0x8105e:         /* CLRB (6809 illegal) */
          case 0x8105f:         /* CLRB (6809 illegal) */
            CLR_REG(reg_b, 1 + ec, 2 + ec, 1 + ec);
            break;

          case 0x30060:         /* NEG indexed */
          case 0x80060:         /* NEG indexed */
          case 0x80061:         /* NEG indexed (6809 illegal) */
          case 0x81060:         /* NEG indexed (6809 illegal) */
          case 0x81061:         /* NEG indexed (6809 illegal) */
            NEG(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30061:         /* OIM IM-indexed */
            OIM(p1, GET_IND_MA(p2, p3, p4), 3, 6, 6);
            break;

          case 0x30062:         /* AIM IM-indexed */
            AIM(p1, GET_IND_MA(p2, p3, p4), 3);
            break;

          case 0x80062:         /* NEG indexed / COM indexed (6809 illegal) */
          case 0x81062:         /* NEG indexed / COM indexed (6809 illegal) */
            if (!LOCAL_CARRY()) {
                NEG(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            } else {
                COM(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            }
            break;                

          case 0x30063:         /* COM indexed */
          case 0x80063:         /* COM indexed */
          case 0x81063:         /* COM indexed */
            COM(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30064:         /* LSR indexed */
          case 0x80064:         /* LSR indexed */
          case 0x80065:         /* LSR indexed (6809 illegal) */
          case 0x81064:         /* LSR indexed (6809 illegal) */
          case 0x81065:         /* LSR indexed (6809 illegal) */
            LSR(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30065:         /* EIM IM-indexed */
            EIM(p1, GET_IND_MA(p2, p3, p4), 3, 7, 7);
            break;

          case 0x30066:         /* ROR indexed */
          case 0x80066:         /* ROR indexed */
          case 0x81066:         /* ROR indexed (6809 illegal) */
            ROR(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30067:         /* ASR indexed */
          case 0x80067:         /* ASR indexed */
          case 0x81067:         /* ASR indexed (6809 illegal) */
            ASR(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30068:         /* ASL/LSL indexed */
          case 0x80068:         /* ASL/LSL indexed */
          case 0x81068:         /* ASL/LSL indexed (6809 illegal) */
            ASL(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30069:         /* ROL indexed */
          case 0x80069:         /* ROL indexed */
          case 0x81069:         /* ROL indexed (6809 illegal) */
            ROL(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x3006a:         /* DEC indexed */
          case 0x8006a:         /* DEC indexed */
          case 0x8006b:         /* DEC indexed (6809 illegal) */
          case 0x8106a:         /* DEC indexed (6809 illegal) */
          case 0x8106b:         /* DEC indexed (6809 illegal) */
            DEC(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x3006b:         /* TIM IM-indexed */
            TIM(p1, GET_IND_MA(p2, p3, p4), 3, 7, 7);
            break;

          case 0x3006c:         /* INC indexed */
          case 0x8006c:         /* INC indexed */
          case 0x8106c:         /* INC indexed (6809 illegal) */
            INC(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x3006d:         /* TST indexed */
          case 0x8006d:         /* TST indexed */
          case 0x8106d:         /* TST indexed (6809 illegal) */
            TST(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 5 + ec);
            break;

          case 0x3006e:         /* JMP indexed */
          case 0x8006e:         /* JMP indexed */
          case 0x8106e:         /* JMP indexed (6809 illegal) */
            JMP(GET_IND_MA(p1, p2, p3), 3 + ec, 3 + ec);
            break;

          case 0x3006f:         /* CLR indexed */
          case 0x8006f:         /* CLR indexed */
          case 0x8106f:         /* CLR indexed (6809 illegal) */
            CLR(GET_IND_MA(p1, p2, p3), 2 + ec, 6 + ec, 6 + ec);
            break;

          case 0x30070:         /* NEG extended */
          case 0x80070:         /* NEG extended */
          case 0x80071:         /* NEG extended (6809 illegal) */
          case 0x81070:         /* NEG extended (6809 illegal) */
          case 0x81071:         /* NEG extended (6809 illegal) */
            NEG((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30071:         /* OIM IM-extended */
            OIM(p1, (p2 << 8) | p3, 4, 7, 7);
            break;

          case 0x30072:         /* AIM IM-extended */
            AIM(p1, (p2 << 8) | p3, 4);
            break;

          case 0x80072:         /* NEG extended / COM extended (6809 illegal) */
          case 0x81072:         /* NEG extended / COM extended (6809 illegal) */
            if (!LOCAL_CARRY()) {
                NEG((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            } else {
                COM((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            }
            break;

          case 0x30073:         /* COM extended */
          case 0x80073:         /* COM extended */
          case 0x81073:         /* COM extended (6809 illegal) */
            COM((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30074:         /* LSR extended */
          case 0x80074:         /* LSR extended */
          case 0x80075:         /* LSR extended (6809 illegal) */
          case 0x81074:         /* LSR extended (6809 illegal) */
          case 0x81075:         /* LSR extended (6809 illegal) */
            LSR((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30075:         /* EIM IM-extended */
            EIM(p1, (p2 << 8) | p3, 4, 7, 7);
            break;

          case 0x30076:         /* ROR extended */
          case 0x80076:         /* ROR extended */
          case 0x81076:         /* ROR extended (6809 illegal) */
            ROR((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30077:         /* ASR extended */
          case 0x80077:         /* ASR extended */
          case 0x81077:         /* ASR extended (6809 illegal) */
            ASR((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30078:         /* ASL extended */
          case 0x80078:         /* ASL extended */
          case 0x81078:         /* ASL extended (6809 illegal) */
            ASL((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30079:         /* ROL extended */
          case 0x80079:         /* ROL extended */
          case 0x81079:         /* ROL extended (6809 illegal) */
            ROL((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x3007a:         /* DEC extended */
          case 0x8007a:         /* DEC extended */
          case 0x8007b:         /* DEC extended (6809 illegal) */
          case 0x8107a:         /* DEC extended (6809 illegal) */
          case 0x8107b:         /* DEC extended (6809 illegal) */
            DEC((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x3007b:         /* TIM IM-extended */
            TIM(p1, (p2 << 8) | p3, 4, 7, 7);
            break;

          case 0x3007c:         /* INC extended */
          case 0x8007c:         /* INC extended */
          case 0x8107c:         /* INC extended (6809 illegal) */
            INC((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x3007d:         /* TST extended */
          case 0x8007d:         /* TST extended */
          case 0x8107d:         /* TST extended (6809 illegal) */
            TST((p1 << 8) | p2, 3 + ec, 7 + ec, 5 + ec);
            break;

          case 0x3007e:         /* JMP extended */
          case 0x8007e:         /* JMP extended */
          case 0x8107e:         /* JMP extended (6809 illegal) */
            JMP((p1 << 8) | p2, 4 + ec, 3 + ec);
            break;

          case 0x3007f:         /* CLR extended */
          case 0x8007f:         /* CLR extended */
          case 0x8107f:         /* CLR extended (6809 illegal */
            CLR((p1 << 8) | p2, 3 + ec, 7 + ec, 6 + ec);
            break;

          case 0x30080:         /* SUBA immediate */
          case 0x80080:         /* SUBA immediate */
          case 0x81080:         /* SUBA immediate (6809 illegal) */
            SUB(reg_a, 0, p1, 8, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30081:         /* CMPA immediate */
          case 0x80081:         /* CMPA immediate */
          case 0x81081:         /* CMPA immediate (6809 illegal) */
            CMP(reg_a, 8, p1, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30082:         /* SBCA immediate */
          case 0x80082:         /* SBCA immediate */
          case 0x81082:         /* SBCA immediate */
            SUB(reg_a, LOCAL_CARRY(), p1, 8, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30083:         /* SUBD immediate */
          case 0x80083:         /* SUBD immediate */
            SUB(reg_d, 0, (p1 << 8) | p2, 16, 3, 4, 3);
            break;

          case 0x30084:         /* ANDA immediate */
          case 0x80084:         /* ANDA immediate */
          case 0x81084:         /* ANDA immediate (6809 illegal) */
            AND(reg_a, p1, 8, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30085:         /* BITA immediate */
          case 0x80085:         /* BITA immediate */
          case 0x81085:         /* BITA immediate (6809 illegal) */
            BIT(reg_a, 8, p1, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30086:         /* LDA immediate */
          case 0x80086:         /* LDA immediate */
          case 0x81086:         /* LDA immediate (6809 illegal) */
            LD(reg_a, 8, p1, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x80087:         /* SCC immediate (6809 illegal) */
          case 0x800c7:         /* SCC immediate (6809 illegal) */
          case 0x810c7:         /* SCC immediate (6809 illegal) */
            SCC();
            break;

          case 0x30088:         /* EORA immediate */
          case 0x80088:         /* EORA immediate */
          case 0x81088:         /* EORA immediate (6809 illegal) */
            EOR(reg_a, 8, p1, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x30089:         /* ADCA immediate */
          case 0x80089:         /* ADCA immediate */
          case 0x81089:         /* ADCA immediate (6809 illegal) */
            ADD(reg_a, p1, LOCAL_CARRY(), 8, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x3008a:         /* ORA immediate */
          case 0x8008a:         /* ORA immediate */
          case 0x8108a:         /* ORA immediate (6809 illegal) */
            OR(reg_a, 8, p1, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x3008b:         /* ADDA immediate */
          case 0x8008b:         /* ADDA immediate */
          case 0x8108b:         /* ADDA immediate (6809 illegal) */
            ADD(reg_a, p1, 0, 8, 2 + ec, 2 + ec, 2 + ec);
            break;

          case 0x3008c:         /* CMPX immediate */
          case 0x8008c:         /* CMPX immediate */
            CMP(reg_x, 16, (p1 << 8) | p2, 3, 4, 3);
            break;

          case 0x3008d:         /* BSR offset */
          case 0x8008d:         /* BSR offset */
          case 0x8108d:         /* BSR offset (6809 illegal) */
            BSR();
            break;

          case 0x3008e:         /* LDX immediate */
          case 0x8008e:         /* LDX immediate */
            LD(reg_x, 16, (p1 << 8) | p2, 3, 3, 3);
            break;

          case 0x8008f:         /* STX immediate (6809 illegal) */
          case 0x8108f:         /* STX immediate (6809 illegal) */
            STI(reg_x);
            break;

          case 0x30090:         /* SUBA direct */
          case 0x80090:         /* SUBA direct */
          case 0x81090:         /* SUBA direct (6809 illegal) */
            SUB(reg_a, 0, LOAD_DIRECT8(p1), 8, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30091:         /* CMPA direct */
          case 0x80091:         /* CMPA direct */
          case 0x81091:         /* CMPA direct (6809 illegal) */
            CMP(reg_a, 8, LOAD_DIRECT8(p1), 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30092:         /* SBCA direct */
          case 0x80092:         /* SBCA direct */
          case 0x81092:         /* SBCA direct (6809 illegal) */
            SUB(reg_a, LOCAL_CARRY(), LOAD_DIRECT8(p1), 8, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30093:         /* SUBD direct */
          case 0x80093:         /* SUBD direct */
            SUB(reg_d, 0, LOAD_DIRECT16(p1), 16, 2, 6, 4);
            break;

          case 0x30094:         /* ANDA direct */
          case 0x80094:         /* ANDA direct */
          case 0x81094:         /* ANDA direct (6809 illegal) */
            AND(reg_a, LOAD_DIRECT8(p1), 8, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30095:         /* BITA direct */
          case 0x80095:         /* BITA direct */
          case 0x81095:         /* BITA direct (6809 illegal) */
            BIT(reg_a, 8, LOAD_DIRECT8(p1), 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30096:         /* LDA direct */
          case 0x80096:         /* LDA direct */
          case 0x81096:         /* LDA direct (6809 illegal) */
            LD(reg_a, 8, LOAD_DIRECT8(p1), 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30097:         /* STA direct */
          case 0x80097:         /* STA direct */
          case 0x81097:         /* STA direct (6809 illegal) */
            ST(reg_a, 8, (reg_dpr << 8) | p2, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30098:         /* EORA direct */
          case 0x80098:         /* EORA direct */
          case 0x81098:         /* EORA direct (6809 illegal) */
            EOR(reg_a, 8, LOAD_DIRECT8(p1), 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x30099:         /* ADCA direct */
          case 0x80099:         /* ADCA direct */
          case 0x81099:         /* ADCA direct (6809 illegal) */
            ADD(reg_a, LOAD_DIRECT8(p1), LOCAL_CARRY(), 8, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x3009a:         /* ORA direct */
          case 0x8009a:         /* ORA direct */
          case 0x8109a:         /* ORA direct (6809 illegal) */
            OR(reg_a, 8, LOAD_DIRECT8(p1), 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x3009b:         /* ADDA direct */
          case 0x8009b:         /* ADDA direct */
          case 0x8109b:         /* ADDA direct */
            ADD(reg_a, LOAD_DIRECT8(p1), 0, 8, 2 + ec, 4 + ec, 3 + ec);
            break;

          case 0x3009c:         /* CMPX direct */
          case 0x8009c:         /* CMPX direct */
            CMP(reg_x, 16, LOAD_DIRECT16(p1), 2, 6, 4);
            break;

          case 0x3009d:         /* JSR direct */
          case 0x8009d:         /* JSR direct */
          case 0x8109d:         /* JSR direct (6809 illegal) */
            JSR((reg_dpr << 8) | p1, 2 + ec, 7 + ec, 6 + ec);
            break;

          case 0x3009e:         /* LDX direct */
          case 0x8009e:         /* LDX direct */
            LD(reg_x, 16, LOAD_DIRECT16(p1), 2, 5, 4);
            break;

          case 0x3009f:         /* STX direct */
          case 0x8009f:         /* STX direct */
            ST(reg_x, 16, (reg_dpr << 8) | p1, 2, 5, 4);
            break;

          case 0x300a0:         /* SUBA indexed */
          case 0x800a0:         /* SUBA indexed */
          case 0x810a0:         /* SUBA indexed (6809 illegal) */
            SUB(reg_a, 0, LOAD_IND8(), 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a1:         /* CMPA indexed */
          case 0x800a1:         /* CMPA indexed */
          case 0x810a1:         /* CMPA indexed */
            CMP(reg_a, 8, LOAD_IND8(), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a2:         /* SBCA indexed */
          case 0x800a2:         /* SBCA indexed */
          case 0x810a2:         /* SBCA indexed (6809 illegal) */
            SUB(reg_a, LOCAL_CARRY(), LOAD_IND8(), 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a3:         /* SUBD indexed */
          case 0x800a3:         /* SUBD indexed */
            SUB(reg_d, 0, LOAD_IND16(), 16, 2, 6, 5);
            break;

          case 0x300a4:         /* ANDA indexed */
          case 0x800a4:         /* ANDA indexed */
          case 0x810a4:         /* ANDA indexed (6809 illegal) */
            AND(reg_a, LOAD_IND8(), 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a5:         /* BITA indexed */
          case 0x800a5:         /* BITA indexed */
          case 0x810a5:         /* BITA indexed (6809 illegal) */
            BIT(reg_a, 8, LOAD_IND8(), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a6:         /* LDA indexed */
          case 0x800a6:         /* LDA indexed */
          case 0x810a6:         /* LDA indexed (6809 illegal) */
            LD(reg_a, 8, LOAD_IND8(), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a7:         /* STA indexed */
          case 0x800a7:         /* STA indexed */
          case 0x810a7:         /* STA indexed (6809 illegal) */
            ST(reg_a, 8, GET_IND_MA(p1, p2, p3), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a8:         /* EORA indexed */
          case 0x800a8:         /* EORA indexed */
          case 0x810a8:         /* EORA indexed (6809 illegal) */
            EOR(reg_a, 8, LOAD_IND8(), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300a9:         /* ADCA indexed */
          case 0x800a9:         /* ADCA indexed */
          case 0x810a9:         /* ADCA indexed (6809 illegal) */
            ADD(reg_a, LOAD_IND8(), LOCAL_CARRY(), 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300aa:         /* ORA indexed */
          case 0x800aa:         /* ORA indexed */
          case 0x810aa:         /* ORA indexed (6809 illegal) */
            OR(reg_a, 8, LOAD_IND8(), 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300ab:         /* ADDA indexed */
          case 0x800ab:         /* ADDA indexed */
          case 0x810ab:         /* ADDA indexed (6809 illegal) */
            ADD(reg_a, LOAD_IND8(), 0, 8, 2 + ec, 4 + ec, 4 + ec);
            break;

          case 0x300ac:         /* CMPX indexed */
          case 0x800ac:         /* CMPX indexed */
            CMP(reg_x, 16, LOAD_IND16(), 2, 6, 5);
            break;

          case 0x300ad:         /* JSR indexed */
          case 0x800ad:         /* JSR indexed */
          case 0x810ad:         /* JSR indexed (6809 illegal) */
            JSR(GET_IND_MA(p1, p2, p3), 2 + ec, 7 + ec, 6 + ec);
            break;

          case 0x300ae:         /* LDX indexed */
          case 0x800ae:         /* LDX indexed */
            LD(reg_x, 16, LOAD_IND16(), 2, 5, 5);
            break;

          case 0x300af:         /* STX indexed */
          case 0x800af:         /* STX indexed */
            ST(reg_x, 16, GET_IND_MA(p1, p2, p3), 2, 5, 5);
            break;

          case 0x300b0:         /* SUBA extended */
          case 0x800b0:         /* SUBA extended */
            SUB(reg_a, 0, LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300b1:         /* CMPA extended */
          case 0x800b1:         /* CMPA extended */
            CMP(reg_a, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300b2:         /* SBCA extended */
          case 0x800b2:         /* SBCA extended */
            SUB(reg_a, LOCAL_CARRY(), LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300b3:         /* SUBD extended */
          case 0x800b3:         /* SUBD extended */
            SUB(reg_d, 0, LOAD_EXT16(), 16, 3, 7, 5);
            break;

          case 0x300b4:         /* ANDA extended */
          case 0x800b4:         /* ANDA extended */
            AND(reg_a, LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300b5:         /* BITA extended */
          case 0x800b5:         /* BITA extended */
            BIT(reg_a, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300b6:         /* LDA extended */
          case 0x800b6:         /* LDA extended */
            LD(reg_a, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300b7:         /* STA extended */
          case 0x800b7:         /* STA extended */
            ST(reg_a, 8, (p1 << 8) | p2, 3, 5, 4);
            break;

          case 0x300b8:         /* EORA extended */
          case 0x800b8:         /* EORA extended */
            EOR(reg_a, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300b9:         /* ADCA extended */
          case 0x800b9:         /* ADCA extended */
            ADD(reg_a, LOAD_EXT8(), LOCAL_CARRY(), 8, 3, 5, 4);
            break;

          case 0x300ba:         /* ORA extended */
          case 0x800ba:         /* ORA extended */
            OR(reg_a, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300bb:         /* ADDA extended */
          case 0x800bb:         /* ADDA extended */
            ADD(reg_a, LOAD_EXT8(), 0, 8, 3, 5, 4);
            break;

          case 0x300bc:         /* CMPX extended */
          case 0x800bc:         /* CMPX extended */
            CMP(reg_x, 16, LOAD_EXT16(), 3, 7, 5);
            break;

          case 0x300bd:         /* JSR extended */
          case 0x800bd:         /* JSR extended */
            JSR((p1 << 8) | p2, 3, 8, 7);
            break;

          case 0x300be:         /* LDX extended */
          case 0x800be:         /* LDX extended */
            LD(reg_x, 16, LOAD_EXT16(), 3, 6, 5);
            break;

          case 0x300bf:         /* STX extended */
          case 0x800bf:         /* STX extended */
            ST(reg_x, 16, (p1 << 8) | p2, 3, 6, 5);
            break;

          case 0x300c0:         /* SUBB immediate */
          case 0x800c0:         /* SUBB immediate */
            SUB(reg_b, 0, p1, 8, 2, 2, 2);
            break;

          case 0x300c1:         /* CMPB immediate */
          case 0x800c1:         /* CMPB immediate */
            CMP(reg_b, 8, p1, 2, 2, 2);
            break;

          case 0x300c2:         /* SBCB immediate */
          case 0x800c2:         /* SBCB immediate */
            SUB(reg_b, LOCAL_CARRY(), p1, 8, 2, 2, 2);
            break;

          case 0x300c3:         /* ADDD immediate */
          case 0x800c3:         /* ADDD immediate */
            ADD(reg_d, (p1 << 8) | p2, 0, 16, 3, 4, 3);
            break;

          case 0x300c4:         /* ANDB immediate */
          case 0x800c4:         /* ANDB immediate */
            AND(reg_b, p1, 8, 2, 2, 2);
            break;

          case 0x300c5:         /* BITB immediate */
          case 0x800c5:         /* BITB immediate */
            BIT(reg_b, 8, p1, 2, 2, 2);
            break;

          case 0x300c6:         /* LDB immediate */
          case 0x800c6:         /* LDB immediate */
            LD(reg_b, 8, p1, 2, 2, 2);
            break;

          case 0x300c8:         /* EORB immediate */
          case 0x800c8:         /* EORB immediate */
            EOR(reg_b, 8, p1, 2, 2, 2);
            break;

          case 0x300c9:         /* ADCB immediate */
          case 0x800c9:         /* ADCB immediate */
            ADD(reg_b, p1, LOCAL_CARRY(), 8, 2, 2, 2);
            break;

          case 0x300ca:         /* ORB immediate */
          case 0x800ca:         /* ORB immediate */
            OR(reg_b, 8, p1, 2, 2, 2);
            break;

          case 0x300cb:         /* ADDB immediate */
          case 0x800cb:         /* ADDB immediate */
            ADD(reg_b, p1, 0, 8, 2, 2, 2);
            break;

          case 0x300cc:         /* LDD immediate */
          case 0x800cc:         /* LDD immediate */
            LD(reg_d, 16, (p1 << 8) | p2, 3, 3, 3);
            break;

          case 0x300cd:         /* LDQ immediate */
            LD(reg_q, 32, (p1 << 24) | (p2 << 16) | (p3 << 8) | p4, 5, 5, 5);
            break;

          case 0x300ce:         /* LDU immediate */
          case 0x800ce:         /* LDU immediate */
            LD(reg_usp, 16, (p1 << 8) | p2, 3, 3, 3);
            break;

          case 0x800cf:         /* STU immediate (6809 illegal) */
            STI(reg_u);
            break;

          case 0x300d0:         /* SUBB direct */
          case 0x800d0:         /* SUBB direct */
            SUB(reg_b, 0, LOAD_DIRECT8(p1), 8, 2, 4, 3);
            break;

          case 0x300d1:         /* CMPB direct */
          case 0x800d1:         /* CMPB direct */
            CMP(reg_b, 8, LOAD_DIRECT8(p1), 2, 4, 3);
            break;

          case 0x300d2:         /* SBCB direct */
          case 0x800d2:         /* SBCB direct */
            SUB(reg_b, LOCAL_CARRY(), LOAD_DIRECT8(p1), 8, 2, 4, 3);
            break;

          case 0x300d3:         /* ADDD direct */
          case 0x800d3:         /* ADDD direct */
            ADD(reg_d, LOAD_DIRECT16(p1), 0, 16, 2, 6, 4);
            break;

          case 0x300d4:         /* ANDB direct */
          case 0x800d4:         /* ANDB direct */
            AND(reg_b, LOAD_DIRECT8(p1), 8, 2, 4, 3);
            break;

          case 0x300d5:         /* BITB direct */
          case 0x800d5:         /* BITB direct */
            BIT(reg_b, 8, LOAD_DIRECT8(p1), 2, 4, 3);
            break;

          case 0x300d6:         /* LDB direct */
          case 0x800d6:         /* LDB direct */
            LD(reg_b, 8, LOAD_DIRECT8(p1), 2, 4, 3);
            break;

          case 0x300d7:         /* STB direct */
          case 0x800d7:         /* STB direct */
            ST(reg_b, 8, (reg_pbr << 8) | p1, 2, 4, 3);
            break;

          case 0x300d8:         /* EORB direct */
          case 0x800d8:         /* EORB direct */
            EOR(reg_b, 8, LOAD_DIRECT8(p1), 2, 4, 3);
            break;

          case 0x300d9:         /* ADCB direct */
          case 0x800d9:         /* ADCB direct */
            ADD(reg_b, LOAD_DIRECT8(p1), LOCAL_CARRY(), 8, 2, 4, 3);
            break;

          case 0x300da:         /* ORB direct */
          case 0x800da:         /* ORB direct */0
            OR(reg_b, 8, LOAD_DIRECT8(p1), 2, 4, 3);
            break;

          case 0x300db:         /* ADDB direct */
          case 0x800db:         /* ADDB direct */
            ADD(reg_b, LOAD_DIRECT8(p1), 0, 8, 2, 4, 3);
            break;

          case 0x300dc:         /* LDD direct */
          case 0x800dc:         /* LDD direct */
            LD(reg_d, 16, LOAD_DIRECT16(p1), 2, 5, 4);
            break;

          case 0x300dd:         /* STD direct */
          case 0x800dd:         /* STD direct */
            ST(reg_d, 16, (reg_dpr << 8) | p1, 2, 5, 4);
            break;

          case 0x300de:         /* LDU direct */
          case 0x800de:         /* LDU direct */
            LD(reg_usp, 16, LOAD_DIRECT16(p1), 2, 5, 4);
            break;

          case 0x300df:         /* STU direct */
          case 0x800df:         /* STU direct */
            ST(reg_usp, 16, (reg_dpr << 8) | p1, 2, 5, 4);
            break;

          case 0x300e0:         /* SUBB indexed */
          case 0x800e0:         /* SUBB indexed */
            SUB(reg_b, 0, LOAD_IND8(), 8, 2, 4, 4);
            break;

          case 0x300e1:         /* CMPB indexed */
          case 0x800e1:         /* CMPB indexed */
            CMP(reg_b, 8, LOAD_IND8(), 2, 4, 4);
            break;

          case 0x300e2:         /* SBCB indexed */
          case 0x800e2:         /* SBCB indexed */
            SUB(reg_b, LOCAL_CARRY(), LOAD_IND8(), 8, 2, 4, 4);
            break;

          case 0x300e3:         /* ADDD indexed */
          case 0x800e3:         /* ADDD indexed */
            ADD(reg_d, LOAD_IND16(), 0, 16, 2, 6, 5);
            break;

          case 0x300e4:         /* ANDB indexed */
          case 0x800e4:         /* ANDB indexed */
            AND(reg_b, LOAD_IND8(), 8, 2, 4, 4);
            break;

          case 0x300e5:         /* BITB indexed */
          case 0x800e5:         /* BITB indexed */
            BIT(reg_b, 8, LOAD_IND8(), 2, 4, 4);
            break;

          case 0x300e6:         /* LDB indexed */
          case 0x800e6:         /* LDB indexed */
            LD(reg_b, 8, LOAD_IND8(), 2, 4, 4);
            break;

          case 0x300e7:         /* STB indexed */
          case 0x800e7:         /* STB indexed */
            ST(reg_b, 8, GET_IND_MA(p1, p2, p3), 2, 4, 4);
            break;

          case 0x300e8:         /* EORB indexed */
          case 0x800e8:         /* EORB indexed */
            EOR(reg_b, 8, LOAD_IND8(), 2, 4, 4);
            break;

          case 0x300e9:         /* ADCB indexed */
          case 0x800e9:         /* ADCB indexed */
            ADD(reg_b, LOAD_IND8(), LOCAL_CARRY(), 8, 2, 4, 4);
            break;

          case 0x300ea:         /* ORB indexed */
          case 0x800ea:         /* ORB indexed */
            OR(reg_b, 8, LOAD_IND8(), 2, 4, 4);
            break;

          case 0x300eb:         /* ADDB indexed */
          case 0x800eb:         /* ADDB indexed */
            ADD(reg_b, LOAD_IND8(), 0, 8, 2, 4, 4);
            break;

          case 0x300ec:         /* LDD indexed */
          case 0x800ec:         /* LDD indexed */
            LD(reg_d, 16, LOAD_IND16(), 2, 5, 5);
            break;

          case 0x300ed:         /* STD indexed */
          case 0x800ed:         /* STD indexed */
            ST(reg_d, 16, GET_IND_MA(p1, p2, p3), 2, 5, 5);
            break;

          case 0x300ee:         /* LDU indexed */
          case 0x800ee:         /* LDU indexed */
            LD(reg_usp, 16, LOAD_IND16(), 2, 5, 5);
            break;

          case 0x300ef:         /* STU indexed */
          case 0x800ef:         /* STU indexed */
            ST(reg_usp, 16, GET_IND_MA(p1, p2, p3), 2, 5, 5);
            break;

          case 0x300f0:         /* SUBB extended */
          case 0x800f0:         /* SUBB extended */
            SUB(reg_b, 0, LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300f1:         /* CMPB extended */
          case 0x800f1:         /* CMPB extended */
            CMP(reg_b, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300f2:         /* SBCB extended */
          case 0x800f2:         /* SBCB extended */
            SUB(reg_b, LOCAL_CARRY(), LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300f3:         /* ADDD extended */
          case 0x800f3:         /* ADDD extended */
            ADD(reg_d, LOAD_EXT16(), 0, 16, 3, 7, 5);
            break;

          case 0x300f4:         /* ANDB extended */
          case 0x800f4:         /* ANDB extended */
            AND(reg_b, LOAD_EXT8(), 8, 3, 5, 4);
            break;

          case 0x300f5:         /* BITB extended */
          case 0x800f5:         /* BITB extended */
            BIT(reg_b, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300f6:         /* LDB extended */
          case 0x800f6:         /* LDB extended */
            LD(reg_b, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300f7:         /* STB extended */
          case 0x800f7:         /* STB extended */
            ST(reg_b, 8, (p1 << 8) | p2, 3, 5, 4);
            break;

          case 0x300f8:         /* EORB extended */
          case 0x800f8:         /* EORB extended */
            EOR(reg_b, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300f9:         /* ADCB extended */
          case 0x800f9:         /* ADCB extended */
            ADD(reg_b, LOAD_EXT8(), LOCAL_CARRY(), 8, 3, 5, 4);
            break;

          case 0x300fa:         /* ORB extended */
          case 0x800fa:         /* ORB extended */
            OR(reg_b, 8, LOAD_EXT8(), 3, 5, 4);
            break;

          case 0x300fb:         /* ADDB extended */
          case 0x800fb:         /* ADDB extended */
            ADD(reg_b, LOAD_EXT8(), 0, 8, 3, 5, 4);
            break;

          case 0x300fc:         /* LDD extended */
          case 0x800fc:         /* LDD extended */
            LD(reg_d, 16, LOAD_EXT16(), 3, 6, 5);
            break;

          case 0x300fd:         /* STD extended */
          case 0x800fd:         /* STD extended */
            ST(reg_d, 16, (p1 << 8) | p2, 3, 6, 5);
            break;

          case 0x300fe:         /* LDU extended */
          case 0x800fe:         /* LDU extended */
            LD(reg_usp, 16, LOAD_EXT16(), 3, 6, 5);
            break;

          case 0x300ff:         /* STU extended */
          case 0x800ff:         /* STU extended */
            ST(reg_usp, 16, (p1 << 8) | p2, 3, 6, 5);
            break;

          case 0x31021:         /* LBRN offset */
          case 0x81021:         /* LBRN offset */
            BRANCH_LONG(0, (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31022:         /* LBHI offset */
          case 0x81022:         /* LBHI offset */
            BRANCH_LONG(!(LOCAL_CARRY() | LOCAL_ZERO()), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31023:         /* LBLS offset */
          case 0x81023:         /* LBLS offset */
            BRANCH_LONG(LOCAL_CARRY() | LOCAL_ZERO(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31024:         /* LBHS/LBCC offset */
          case 0x81024:         /* LBHS/LBCC offset */
            BRANCH_LONG(!LOCAL_CARRY(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31025:         /* LBCS/LBLO offset */
          case 0x81025:         /* LBCS/LBLO offset */
            BRANCH_LONG(LOCAL_CARRY(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31026:         /* LBNE offset */
          case 0x81026:         /* LBNE offset */
            BRANCH_LONG(!LOCAL_ZERO(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31027:         /* LBEQ offset */
          case 0x81027:         /* LBEQ offset */
            BRANCH_LONG(LOCAL_ZERO(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31028:         /* LBVC offset */
          case 0x81028:         /* LBVC offset */
            BRANCH_LONG(!LOCAL_OVERFLOW(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31029:         /* LBVS offset */
          case 0x81029:         /* LBVS offset */
            BRANCH_LONG(LOCAL_OVERFLOW(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102a:         /* LBPL offset */
          case 0x8102a:         /* LBPL offset */
            BRANCH_LONG(!LOCAL_NEGATIVE(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102b:         /* LBMI offset */
          case 0x8102b:         /* LBMI offset */
            BRANCH_LONG(LOCAL_NEGATIVE(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102c:         /* LBGE offset */
          case 0x8102c:         /* LBGE offset */
            BRANCH_LONG(!(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102d:         /* LBLT offset */
          case 0x8102d:         /* LBLT offset */
            BRANCH_LONG(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW(), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102e:         /* LBGT offset */
          case 0x8102e:         /* LBGT offset */
            BRANCH_LONG((!LOCAL_ZERO() & !(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW())), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x3102f:         /* LBLE offset */
          case 0x8102f:         /* LBLE offset */
            BRANCH_LONG(LOCAL_ZERO() | (LOCAL_NEGATIVE ^ LOCAL_OVERFLOW()), (p1 << 8) | p2, 4, 5, 5);
            break;

          case 0x31030:         /* ADDR registers */
            R_WRAP(p1, ADDR_WRAP, 3, 4, 4);
            break;

          case 0x31031:         /* ADCR registers */
            R_WRAP(p1, ADCR_WRAP, 3, 4, 4);
            break;

          case 0x31032:         /* SUBR registers */
            R_WRAP(p1, SUBR_WRAP, 3, 4, 4);
            break;

          case 0x31033:         /* SBCR registers */
            R_WRAP(p1, SBCR_WRAP, 3, 4, 4);
            break;

          case 0x31034:         /* ANDR registers */
            R_WRAP(p1, ANDR_WRAP, 3, 4, 4);
            break;

          case 0x31035:         /* ORR registers */
            R_WRAP(p1, ORR_WRAP, 3, 4, 4);
            break;

          case 0x31036:         /* EORR registers */
            R_WRAP(p1, EORR_WRAP, 3, 4, 4);
            break;

          case 0x31037:         /* CMPR registers */
            R_WRAP(p1, CMPR_WRAP, 3, 4, 4);
            break;

          case 0x31038:         /* PSHSW */
            PSHSW();
            break;

          case 0x31039:         /* PULSW */
            PULSW();
            break;

          case 0x3103a:         /* PSHUW */
            PSHUW();
            break;

          case 0x3103b:         /* PULUW */
            PULUW();
            break;

          case 0x3103f:         /* SWI2 */
            SWI_REG(2, 2);
            break;

          case 0x31040:         /* NEGD */
            NEG_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x31043:         /* COMD */
            COM_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x31044:         /* LSRD */
            LSR_REG(reg_d, 2, 3, 2);
            break;

          case 0x31046:         /* RORD */
            ROR_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x31047:         /* ASRD */
            ASR_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x31048:         /* ASLD */
            ASL_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x31049:         /* ROLD */
            ROL_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x3104a:         /* DECD */
            DEC_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x3104c:         /* INCD */
            INC_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x3104d:         /* TSTD */
            TST_REG(reg_d, 16, 2, 3, 2);
            break;

          case 0x3104f:         /* CLRD */
            CLR_REG(reg_d, 2, 3, 2);
            break;

          case 0x31053:         /* COMW */
            COM_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x31054:         /* LSRW */
            LSR_REG(reg_w, 2, 3, 2);
            break;

          case 0x31056:         /* RORW */
            ROR_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x31059:         /* ROLW */
            ROL_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x3105a:         /* DECW */
            DEC_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x3105c:         /* INCW */
            INC_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x3105d:         /* TSTW */
            TST_REG(reg_w, 16, 2, 3, 2);
            break;

          case 0x3105f:         /* CLRW */
            CLR_REG(reg_w, 2, 3, 2);
            break;

          case 0x31080:         /* SUBW immediate */
            SUB(reg_w, 0, (p1 << 8) | p2, 16, 4, 5, 4);
            break;

          case 0x31081:         /* CMPW immediate */
            CMP(reg_w, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31082:         /* SBCD immediate */
            SUB(reg_d, LOCAL_CARRY(), (p1 << 8) | p2, 16, 4, 5, 4);
            break;

          case 0x31083:         /* CMPD immediate */
          case 0x81083:         /* CMPD immediate */
            CMP(reg_d, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31084:         /* ANDD immediate */
            AND(reg_d, (p1 << 8) | p2, 16, 4, 5, 4);
            break;

          case 0x31085:         /* BITD immediate */
            BIT(reg_d, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31086:         /* LDW immediate */
            LD(reg_w, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31088:         /* EORD immediate */
            EOR(reg_d, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31089:         /* ADCD immediate */
            ADD(reg_d, (p1 << 8) | p2, LOCAL_CARRY(), 16, 4, 5, 4);
            break;

          case 0x3108a:         /* ORD immediate */
            OR(reg_d, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x3108b:         /* ADDW immediate */
            ADD(reg_w, (p1 << 8) | p2, 0, 16, 4, 5, 4);
            break;

          case 0x3108c:         /* CMPY immediate */
          case 0x8108c:         /* CMPY immediate */
            CMP(reg_y, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x3108e:         /* LDY immediate */
          case 0x8108e:         /* LDY immediate */
            LD(reg_y, 16, (p1 << 8) | p2, 4, 5, 4);
            break;

          case 0x31090:         /* SUBW direct */
            SUB(reg_w, 0, LOAD_DIRECT16(p1), 16, 3, 7, 5);
            break;

          case 0x31091:         /* CMPW direct */
            CMP(reg_w, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x31092:         /* SBCD direct */
            SUB(reg_d, LOCAL_CARRY(), LOAD_DIRECT16(p1), 16, 3, 7, 5);
            break;

          case 0x31093:         /* CMPD direct */
          case 0x81093:         /* CMPD direct */
            CMP(reg_d, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x31094:         /* ANDD direct */
            AND(reg_d, LOAD_DIRECT_16(p1), 16, 3, 7, 5);
            break;

          case 0x31095:         /* BITD direct */
            BIT(reg_d, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x31096:         /* LDW direct */
            LD(reg_w, 16, LOAD_DIRECT16(p1), 3, 6, 5);
            break;

          case 0x31097:         /* STW direct */
            ST(reg_w, 16, (reg_dpr << 8) | p1, 3, 6, 5);
            break;

          case 0x31098:         /* EORD direct */
            EOR(reg_d, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x31099:         /* ADCD direct */
            ADD(reg_d, LOAD_DIRECT16(p1), LOCAL_CARRY(), 16, 3, 7, 5);
            break;

          case 0x3109a:         /* ORD direct */
            OR(reg_d, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x3109b:         /* ADDW direct */
            ADD(reg_w, LOAD_DIRECT16(p1), 0, 16, 3, 7, 5);
            break;

          case 0x3109c:         /* CMPY direct */
          case 0x8109c:         /* CMPY direct */
            CMP(reg_y, 16, LOAD_DIRECT16(p1), 3, 7, 5);
            break;

          case 0x3109e:         /* LDY direct */
          case 0x8109e:         /* LDY direct */
            LD(reg_y, 16, LOAD_DIRECT16(p1), 3, 6, 5);
            break;

          case 0x3109f:         /* STY direct */
          case 0x8109f:         /* STY direct */
            ST(reg_y, 16, (reg_dpr << 8) | p1, 3, 6, 5);
            break;

          case 0x310a0:         /* SUBW indexed */
            SUB(reg_w, 0, LOAD_IND16(), 16, 3, 7, 6);
            break;

          case 0x310a1:         /* CMPW indexed */
            CMP(reg_w, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310a2:         /* SBCD indexed */
            SUB(reg_d, LOCAL_CARRY(), LOAD_IND16(), 16, 3, 7, 6);
            break;

          case 0x310a3:         /* CMPD indexed */
          case 0x810a3:         /* CMPD indexed */
            CMP(reg_d, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310a4:         /* ANDD indexed */
            AND(reg_d, LOAD_IND16(), 16, 3, 7, 6);
            break;

          case 0x310a5:         /* BITD indexed */
            BIT(reg_d, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310a6:         /* LDW indexed */
            LD(reg_w, 16, LOAD_IND16(), 3, 6, 6);
            break;

          case 0x310a7:         /* STW indexed */
            ST(reg_w, 16, GET_IND_MA(p1, p2, p3), 3, 6, 6);
            break;

          case 0x310a8:         /* EORD indexed */
            EOR(reg_d, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310a9:         /* ADCD indexed */
            ADD(reg_d, LOAD_IND16(), LOCAL_CARRY(), 16, 3, 7, 6);
            break;

          case 0x310aa:         /* ORD indexed */
            OR(reg_d, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310ab:         /* ADDW indexed */
            ADD(reg_w, LOAD_IND16(), 0, 16, 3, 7, 6);
            break;

          case 0x310ac:         /* CMPY indexed */
          case 0x810ac:         /* CMPY indexed */
            CMP(reg_y, 16, LOAD_IND16(), 3, 7, 6);
            break;

          case 0x310ae:         /* LDY indexed */
          case 0x810ae:         /* LDY indexed */
            LD(reg_y, 16, LOAD_IND16(), 3, 6, 6);
            break;

          case 0x310af:         /* STY indexed */
          case 0x810af:         /* STY indexed */
            ST(reg_y, 16, GET_IND_MA(p1, p2, p3), 3, 6, 6);
            break;

          case 0x10b0:          /* SUBW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_w, 0, LOAD_EXT16(), 16, 4);
            break;

          case 0x10b1:          /* CMPW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_w, 16, LOAD_EXT16(), 4);
            break;

          case 0x10b2:          /* SBCD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_d, LOCAL_CARRY(), LOAD_EXT16(), 16, 4);
            break;

          case 0x10b3:          /* CMPD extended */
            CMP(reg_d, 16, LOAD_EXT16(), 4);
            break;

          case 0x10b4:          /* ANDD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            AND(reg_d, LOAD_EXT16(), 16, 4);
            break;

          case 0x10b5:          /* BITD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_d, 16, LOAD_EXT16(), 4);
            break;

          case 0x10b6:          /* LDW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_EXT16(), 4);
            break;

          case 0x10b7:          /* STW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_w, 16, (p1 << 8) | p2, 4);
            break;

          case 0x10b8:          /* EORD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            EOR(reg_d, 16, LOAD_EXT16(), 4);
            break;

          case 0x10b9:          /* ADCD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_d, LOAD_EXT16(), LOCAL_CARRY(), 16, 4);
            break;

          case 0x10ba:          /* ORD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 16, LOAD_EXT16(), 4);
            break;

          case 0x10bb:          /* ADDW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_w, LOAD_EXT16(), 0, 16, 4);
            break;

          case 0x10bc:          /* CMPY extended */
            CMP(reg_y, 16, LOAD_EXT16(), 4);
            break;

          case 0x10be:          /* LDY extended */
            LD(reg_y, 16, LOAD_EXT16(), 4);
            break;

          case 0x10bf:          /* STY extended */
            ST(reg_y, 16, (p1 << 8) | p2, 4);
            break;

          case 0x10ce:          /* LDS immediate */
            LD(reg_ssp, 16, (p1 << 8) | p2, 4);
            break;

          case 0x10dc:          /* LDQ direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_DIRECT32(p1), 3);
            break;

          case 0x10dd:          /* STQ direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_q, 32, (reg_dpr << 8) | p1, 3);
            break;

          case 0x10de:          /* LDS direct */
            LD(reg_ssp, 16, LOAD_DIRECT16(p1), 3);
            break;

          case 0x10df:          /* STS direct */
            ST(reg_ssp, 16, (reg_dpr << 8) | p1, 3);
            break;

          case 0x10ec:          /* LDQ indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_IND32(), 3);
            break;

          case 0x10ed:          /* STQ indexed */
            ST(reg_q, 32, GET_IND_MA(p1, p2, p3), 3);
            break;

          case 0x10ee:          /* LDS indexed */
            LD(reg_ssp, 16, LOAD_IND16(), 3);
            break;

          case 0x10ef:          /* STS indexed */
            ST(reg_ssp, 16, GET_IND_MA(p1, p2, p3), 3);
            break;

          case 0x10fc:          /* LDQ extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_EXT32(), 4);
            break;

          case 0x10fd:          /* STQ extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_q, 32, (p1 << 8) | p2, 4);
            break;

          case 0x10fe:          /* LDS extended */
            LD(reg_ssp, 16, LOAD_EXT16(), 4);
            break;

          case 0x10ff:          /* STS extended */
            ST(reg_ssp, 16, (p1 << 8) | p2, 4);
            break;

          case 0x1130:          /* BAND B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BAND(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1131:          /* BIAND B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BIAND(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1132:          /* BOR B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BOR(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1133:          /* BIOR B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BIOR(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1134:          /* BEOR B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BEOR(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1135:          /* BIEOR B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BIEOR(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1136:          /* LDBT B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LDBT(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1137:          /* STBT B-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            STBT(p1, (reg_dpr << 8) | p2);
            break;

          case 0x1138:          /* TFM R+,R+ */   /* FIXME: fix for 6809, 6309 only opcode */
            TFMPP(p1);
            break;

          case 0x1139:          /* TFM R-,R- */   /* FIXME: fix for 6809, 6309 only opcode */
            TFMMM(p1);
            break;

          case 0x113a:          /* TFM R+,R */   /* FIXME: fix for 6809, 6309 only opcode */
            TFMPC(p1);
            break;

          case 0x113b:          /* TFM R,R+ */   /* FIXME: fix for 6809, 6309 only opcode */
            TFMCP(p1);
            break;

          case 0x113c:          /* BITMD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_md, 8, p1, 3);
            break;

          case 0x113d:          /* LDMD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_md, 8, p1, 5);
            break;

          case 0x113f:          /* SWI3 */
            SWI_REG(3, 2)

          case 0x1143:          /* COME */   /* FIXME: fix for 6809, 6309 only opcode */
            COM_REG(reg_e, 8, 2);
            break;

          case 0x114a:          /* DECE */   /* FIXME: fix for 6809, 6309 only opcode */
            DEC_REG(reg_e, 8, 2);
            break;

          case 0x114c:          /* INCE */   /* FIXME: fix for 6809, 6309 only opcode */
            INC_REG(reg_e, 8, 2);
            break;

          case 0x114d:          /* TSTE */   /* FIXME: fix for 6809, 6309 only opcode */
            TST_REG(reg_e, 8, 2);
            break;

          case 0x114f:          /* CLRE */   /* FIXME: fix for 6809, 6309 only opcode */
            CLR_REG(reg_e, 2);
            break;

          case 0x1153:          /* COMF */   /* FIXME: fix for 6809, 6309 only opcode */
            COM_REG(reg_f, 8, 2);
            break;

          case 0x115a:          /* DECF */   /* FIXME: fix for 6809, 6309 only opcode */
            DEC_REG(reg_f, 8, 2);
            break;

          case 0x115c:          /* INCF */   /* FIXME: fix for 6809, 6309 only opcode */
            INC_REG(reg_f, 8, 2);
            break;

          case 0x115d:          /* TSTF */   /* FIXME: fix for 6809, 6309 only opcode */
            TST_REG(reg_f, 8, 2);
            break;

          case 0x115f:          /* CLRF */   /* FIXME: fix for 6809, 6309 only opcode */
            CLR_REG(reg_f, 2);
            break;

          case 0x1180:          /* SUBE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, p1, 8, 3);
            break;

          case 0x1181:          /* CMPE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, p1, 3);
            break;

          case 0x1183:          /* CMPU immediate */
            CMP(reg_usp, 16, (p1 << 8) | p2, 4);
            break;

          case 0x1186:          /* LDE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, p1, 3);
            break;

          case 0x118b:          /* ADDE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, p1, 0, 8, 3);
            break;

          case 0x118c:          /* CMPS immediate */
            CMP(reg_ssp, 16, (p1 << 8) | p2, 4);
            break;

          case 0x118d:          /* DIVD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVD(p1, 3);
            break;

          case 0x118e:          /* DIVQ immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVQ((p1 << 8) | p2, 4);
            break;

          case 0x118f:          /* MULD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            MULD((p1 << 8) | p2, 4);
            break;

          case 0x1190:          /* SUBE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_DIRECT8(p1), 8, 3);
            break;

          case 0x1191:          /* CMPE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_DIRECT8(p1), 3);
            break;

          case 0x1193:          /* CMPU direct */
            CMP(reg_usp, 16, LOAD_DIRECT16(p1), 3);
            break;

          case 0x1196:          /* LDE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_DIRECT8(p1), 3);
            break;

          case 0x1197:          /* STE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, (reg_dpr << 8) | p1, 3);
            break;

          case 0x119b:          /* ADDE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, LOAD_DIRECT8(p1), 0, 8, 3);
            break;

          case 0x119c:          /* CMPS direct */
            CMP(reg_ssp, 16, LOAD_DIRECT16(p1), 3);
            break;

          case 0x119d:          /* DIVD direct */
            DIVD(LOAD_DIRECT8(p1), 3);
            break;

          case 0x119e:          /* DIVQ direct */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVQ(LOAD_DIRECT16(p1), 3);
            break;

          case 0x119f:          /* MULD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            MULD(LOAD_DIRECT16(p1), 3);
            break;

          case 0x11a0:          /* SUBE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_IND8(), 8, 3);
            break;

          case 0x11a1:          /* CMPE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_IND8(), 3);
            break;

          case 0x11a3:          /* CMPU indexed */
            CMP(reg_usp, 16, LOAD_IND16(), 3);
            break;

          case 0x11a6:          /* LDE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_IND8(), 3);
            break;

          case 0x11a7:          /* STE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, GET_IND_MA(p1, p2, p3), 3);
            break;

          case 0x11ab:          /* ADDE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, LOAD_IND8(), 0, 8, 3);
            break;

          case 0x11ac:          /* CMPS indexed */
            CMP(reg_ssp, 16, LOAD_IND16(), 3);
            break;

          case 0x11ad:          /* DIVD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVD(LOAD_IND8(), 3);
            break;

          case 0x11ae:          /* DIVQ indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVQ(LOAD_IND16(), 3);
            break;

          case 0x11af:          /* MULD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            MULD(LOAD_IND16(), 3);
            break;

          case 0x11b0:          /* SUBE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_EXT8(), 8, 4);
            break;

          case 0x11b1:          /* CMPE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_EXT8(), 4);
            break;

          case 0x11b3:          /* CMPU extended */
            CMP(reg_usp, 16, LOAD_EXT16(), 4);
            break;

          case 0x11b6:          /* LDE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_EXT8(), 4);
            break;

          case 0x11b7:          /* STE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, (p1 << 8) | p2, 4);
            break;

          case 0x11bb:          /* ADDE extended */
            ADD(reg_e, LOAD_EXT8(), 0, 8, 4);
            break;

          case 0x11bc:          /* CMPS extended */
            CMP(reg_ssp, 16, LOAD_EXT16(), 4);
            break;

          case 0x11bd:          /* DIVD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVD(LOAD_EXT8(), 4);
            break;

          case 0x11be:          /* DIVQ extended */   /* FIXME: fix for 6809, 6309 only opcode */
            DIVQ(LOAD_EXT16(), 4);
            break;

          case 0x11bf:          /* MULD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            MULD(LOAD_EXT16(), 4);
            break;

          case 0x11c0:          /* SUBF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, p1, 8, 3);
            break;

          case 0x11c1:          /* CMPF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, p1, 3);
            break;

          case 0x11c6:          /* LDF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, p1, 3);
            break;

          case 0x11cb:          /* ADDF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, p1, 0, 8, 3);
            break;

          case 0x11d0:          /* SUBF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_DIRECT8(p1), 8, 3);
            break;

          case 0x11d1:          /* CMPF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_DIRECT8(p1), 3);
            break;

          case 0x11d6:          /* LDF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_DIRECT8(p1), 3);
            break;

          case 0x11d7:          /* STF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, (reg_dpr << 8) | p1, 3);
            break;

          case 0x11db:          /* ADDF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_DIRECT8(p1), 0, 8, 3);
            break;

          case 0x11e0:          /* SUBF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_IND8(), 8, 3);
            break;

          case 0x11e1:          /* CMPF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_IND8(), 3);
            break;

          case 0x11e6:          /* LDF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_IND8(), 3);
            break;

          case 0x11e7:          /* STF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, GET_IND_MA(p1, p2, p3), 3);
            break;

          case 0x11eb:          /* ADDF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_IND8(), 0, 8, 3);
            break;

          case 0x11f0:          /* SUBF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_EXT8(), 8, 4);
            break;

          case 0x11f1:          /* CMPF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_EXT8(), 4);
            break;

          case 0x11f6:          /* LDF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_EXT8());
            break;

          case 0x11f7:          /* STF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, (p1 << 8) | p2, 4);
            break;

          case 0x11fb:          /* ADDF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_EXT8(), 0, 8, 4);
            break;
        }
    }
}

