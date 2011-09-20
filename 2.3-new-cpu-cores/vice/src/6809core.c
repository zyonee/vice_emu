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

#ifndef CLK_ADD
#define CLK_ADD(clock, amount) clock += amount
#endif

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
    } else {
        PC_INC(1);       /* postbyte fetch */
        PB_REG2VAR(b1, ma);
        switch (b1 & 0x1f) {
            case 0:      /* ,R+ */
                PB_REGADD(b1, 1);
                break;
            case 1:      /* ,R++ */
                PB_REGADD(b1, 2);
                break;
            case 2:      /* ,R- */
                PB_REGADD(b1, -1);
                break;
            case 3:      /* ,R-- */
                PB_REGADD(b1, -2);
                break;
            case 4:      /* ,R */
                break;
            case 5:      /* B,R */
                ma += (signed char)reg_b;
                break;
            case 6:      /* A,R */
                ma += (signed char)reg_a;
                break;
            case 7:      /* E,R */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed char)reg_e;
                break;
            case 8:      /* n,R (8bit offset) */
                ma += b2;
                PC_INC(1);
                break;
            case 9:      /* n,R (16bit offset) */
                ma += ((b2 << 8) | b3);
                PC_INC(2);
                break;
            case 10:     /* F,R */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed char)reg_f;
                break;
            case 11:     /* D,R */
                ma += (signed short)reg_d;
                break;
            case 12:     /* n,PC (8bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(1);
                ma = reg_pc + (signed char)b2;
                break;
            case 13:     /* n,PC (16bit offset) */   /* FIXME: check of pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((b2 << 8) | b3);
                break;
            case 14:     /* W,R */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed short)((reg_e << 8) | reg_f);
                break;
            case 15:   /* FIXME: fix for 6809, 6309 only modes */
                switch ((b1 & 0x60) >> 5) {
                    case 0:     /* ,W */
                        ma = reg_w;
                        break;
                    case 1:     /* n,W (16bit offset) */
                        ma = reg_w + (signed short)((b2 << 8) | b3);
                        PC_INC(2);
                        break;
                    case 2:     /* ,W++ */
                        ma = reg_w;
                        reg_w += 2;
                        break;
                    case 3:     /* ,W-- */
                        ma = reg_w;
                        reg_w -= 2;
                        break;
                }
                break;
            case 16:     /* FIXME: fix for 6809, 6309 only modes */
                switch ((p1 & 0x60) >> 5) {
                    case 0:     /* [,W] */
                        ma = LOAD(reg_w) << 8;
                        ma |= LOAD(reg_w + 1);
                        break;
                    case 1:     /* [n,W] (16bit offset) */
                        ma = LOAD(reg_w + (signed short)((b2 << 8) | b3)) << 8;
                        ma |= LOAD(reg_w + 1 + (signed short)((b2 << 8) | b3));
                        PC_INC(2);
                        break;
                    case 2:     /* [,W++] */
                        ma = LOAD(reg_w) << 8;
                        ma |= LOAD(reg_w + 1);
                        reg_w += 2;
                        break;
                    case 3:     /* [,W--] */
                        ma = LOAD(reg_w) << 8;
                        ma |= LOAD(reg_w + 1);
                        reg_w -= 2;
                        break;
                }
                break;
            case 17:     /* [,R++] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PB_REGADD(b1, 2);
                break;
            case 18:     /* FIXME: unknown */
                break;
            case 19:     /* [,R--] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PB_REGADD(b1, -2);
                break;
            case 20:     /* [,R] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 21:     /* [B,R] */
                ma += (signed char)reg_b;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 22:     /* [A,R] */
                ma += (signed char)reg_a;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 23:     /* [E,R] */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed char)reg_e;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 24:     /* [n,R] (8bit offset) */
                ma += b2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PC_INC(1);
                break;
            case 25:     /* [n,R] (16bit offset) */
                ma += ((b2 << 8) | b3);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PC_INC(2);
                break;
            case 26:     /* [F,R] */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed char)reg_f;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 27:     /* [D,R] */
                ma += (signed short)reg_d;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 28:     /* [n,PC] (8bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(1);
                ma = reg_pc + (signed char)b2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 29:     /* [n,PC] (16bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((b2 << 8) | b3);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 30:     /* [W,R] */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed short)((reg_e << 8) | reg_f);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 31:     /* [n] (16bit extended indirect) */
                ma = LOAD((b2 << 8) | b3) << 8;
                ma |= LOAD(((b2 << 8) | b3) + 1);
                PC_INC(2);
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

#define TFMPP_REG(r0, r1)   \
  do {                      \
      BYTE val;             \
                            \
      while (reg_w) {       \
          val = LOAD(r0++); \
          STORE(r1++, val); \
          reg_w--;          \
      }                     \
  } while (0)

#define TFMMM_REG(r0, r1)   \
  do {                      \
      BYTE val;             \
                            \
      while (reg_w) {       \
          val = LOAD(r0--); \
          STORE(r1--, val); \
          reg_w--;          \
      }                     \
  } while (0)

#define TFMPC_REG(r0, r1)   \
  do {                      \
      BYTE val;             \
                            \
      while (reg_w) {       \
          val = LOAD(r0++); \
          STORE(r1, val);   \
          reg_w--;          \
      }                     \
  } while (0)

#define TFMCP_REG(r0, r1)   \
  do {                      \
      BYTE val = LOAD(r0);  \
                            \
      while (reg_w) {       \
          STORE(r1++, val); \
      }                     \
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

#defime ABX()         \
  do {                \
      reg_a += reg_b; \
      PC_INC(1);      \
  } while (0)

#define ADD(RR, m, CC, bits)                       \
  do {                                             \
      unsigned int tmp;                            \
                                                   \
      if (bits == 8) {                             \
          tmp = (RR & 0xf) + (m & 0xf) + CC;       \
          LOCAL_SET_HALF_CARRY(BT(tmp, 4));        \
          tmp = (RR & 0x7f) + (m & 0x7f) + CC;     \
      } else {                                     \
          tmp = (RR & 0x7fff) + (m & 0x7fff) + CC; \
      }                                            \
      LOCAL_SET_OVERFLOW(BT(tmp, bits - 1));       \
      tmp = RR + m + CC;                           \
      LOCAL_SET_CARRY(BT(tmp, bits));              \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1));       \
      LOCAL_SET_ZERO(!tmp);                        \
      RR = tmp & (bits == 8) ? 0xff : 0xffff;      \
  } while (0)

#define AIM(m, ma, pc_inc)            \
  do {                                \
      BYTE tmp;                       \
                                      \
      tmp = LOAD(ma);                 \
      tmp &= m;                       \
      LOCAL_SET_OVERFLOW(0);          \
      LOCAL_SET_NEGATIVE(BT(tmp, 7)); \
      LOCAL_SET_ZERO(!tmp);           \
      STORE(ma, tmp);                 \
      PC_INC(pc_inc);                 \
  } while (0)

#define AND(RR, m, bits, pc_inc)            \
  do {                                      \
      RR &= m;                              \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      LOCAL_SET_OVERFLOW(0);                \
      PC_INC(pc_inc);                       \
  } while (0)

#define ASL_REG(RR, bits, pc_inc)                              \
  do {                                                         \
      LOCAL_SET_CARRY(BT(RR, bits - 1));                       \
      LOCAL_SET_OVERFLOW(BT(RR, bits - 1) ^ BT(RR, bits - 2)); \
      RR <<= 1;                                                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));                    \
      LOCAL_SET_ZERO(!RR);                                     \
      PC_INC(pc_inc);                                          \
  } while (0)

#define ASL(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      tmp = LOAD(ma);          \
      ASL_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define ASR_REG(RR, bits, pc_inc)                            \
  do {                                                       \
      LOCAL_SET_CARRY(BT(RR, 0));                            \
      RR = (RR & ((bits == 8) ? 0x80 : 0x8000)) | (RR >> 1); \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));                  \
      LOCAL_SET_ZERO(!RR);                                   \
      PC_INC(pc_inc);                                        \
  } while (0)

#define ASR(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      ASR_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
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
  } while (0)

#define BIT(RR, bits, m)                     \
  do {                                       \
      unsigned int tmp;                      \
                                             \
      tmp = RR & m;                          \
      LOCAL_SET_NEGATIVE(BT(tmp, bits - 1)); \
      LOCAL_SET_ZERO(!tmp);                  \
      LOCAL_SET_OVERFLOW(0);                 \
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
  } while (0)

#define BRANCH(cond, value, pc_inc)                  \
  do {                                               \
      unsigned int dest_addr = 0;                    \
                                                     \
      PC_INC(pc_inc);                                \
      if (cond) {                                    \
          dest_addr = reg_pc + (signed char)(value); \
          reg_pc = dest_addr & 0xffff;               \
      }                                              \
  } while (0)

#define BRANCH_LONG(cond, value, pc_inc)              \
  do {                                                \
      unsigned int dest_addr = 0;                     \
                                                      \
      PC_INC(pc_inc);                                 \
      if (cond) {                                     \
          dest_addr = reg_pc + (signed short)(value); \
          reg_pc = dest_addr & 0xffff;                \
      }                                               \
  } while (0)

#define BSR()                     \
  do {                            \
      PUSHS((reg_pc + 2) & 0xff); \
      PUSHS((reg_pc + 2) >> 8);   \
      BRANCH(1, p1, 2);           \
  } while (0)

#define LBSR()                           \
  do {                                   \
      PUSHS((reg_pc + 3) & 0xff);        \
      PUSHS((reg_pc + 3) >> 8);          \
      BRANCH_LONG(1, (p1 << 8) | p2, 3); \
  } while (0)

#define CLR_REG(RR, pc_inc) \
  do {                      \
      RR = 0;               \
      PC_INC(pc_inc);       \
  } while (0)

#define CLR(ma, pc_inc) \
  do {                  \
      STORE(ma, 0);     \
      PC_INC(pc_inc);   \
  } while (0)

#define COM_REG(RR, bits, pc_inc)           \
  do {                                      \
      RR = ~RR;                             \
      LOCAL_SET_CARRY(1);                   \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      PC_INC(pc_inc);                       \
  } while (0)

#define COM(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      COM_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define CMP(RR, bits, m)                                                 \
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
      PC_INC(1);                                                    \
  } while (0)

#define DEC_REG(RR, bits, pc_inc)           \
  do {                                      \
      if (bits == 8) {                      \
          LOCAL_SET_OVERFLOW(RR == 0x80);   \
      } else {                              \
          LOCAL_SET_OVERFLOW(RR == 0x8000); \
      }                                     \
      RR--;                                 \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      PC_INC(pc_inc);                       \
  } while (0)

#define DEC(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      DEC_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define EIM(m, ma, pc_inc)            \
  do {                                \
      BYTE tmp;                       \
                                      \
      tmp = LOAD(ma);                 \
      tmp ^= m;                       \
      LOCAL_SET_OVERFLOW(0);          \
      LOCAL_SET_NEGATIVE(BT(tmp, 7)); \
      LOCAL_SET_ZERO(!tmp);           \
      STORE(ma, tmp);                 \
      PC_INC(pc_inc);                 \
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
      PC_INC(2);                           \
  } while (0)

#define EOR(RR, bits, m)                    \
  do {                                      \
      RR ^= m;                              \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
  } while (0)

#define INC_REG(RR, bits, pc_inc)           \
  do {                                      \
      if (bits == 8) {                      \
          LOCAL_SET_OVERFLOW(RR == 0x7f);   \
      } else {                              \
          LOCAL_SET_OVERFLOW(RR == 0x7fff); \
      }                                     \
      RR += 1;                              \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      PC_INC(pc_inc);                       \
  } while (0)

#define INC(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      INC_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)


/* The 0x02 AIM opcode is also used to patch the ROM.  The function trap_handler()
   returns nonzero if this is not a patch, but a `real' AIM instruction. */

#define AIM_02()                                                \
  do {                                                          \
      DWORD trap_result;                                        \
      EXPORT_REGISTERS();                                       \
      if (!ROM_TRAP_ALLOWED()                                   \
          || (trap_result = ROM_TRAP_HANDLER()) == (DWORD)-1) { \
          REWIND_FETCH_OPCODE(CLK);                             \
          AIM(p1, (reg_dpr << 8) | p2, 3);                      \
      } else {                                                  \
          if (trap_result) {                                    \
             REWIND_FETCH_OPCODE(CLK);                          \
             SET_OPCODE(trap_result);                           \
             IMPORT_REGISTERS();                                \
             goto trap_skipped;                                 \
          } else {                                              \
             IMPORT_REGISTERS();                                \
          }                                                     \
      }                                                         \
  } while (0)
  
#define JMP(ma)    \
  do {             \
      reg_pc = ma; \
  } while (0)

#define JSR(ma)             \
  do {                      \
      PUSHS(reg_pc & 0xff); \
      PUSHS(reg_pc >> 8);   \
      reg_pc = ma;          \
  } while (0)

#define LD(RR, bits, m)                     \
  do {                                      \
      RR = m;                               \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_ZERO(!RR);                  \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
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
      BTM_VAR2REG(rnr, rr);
  } while (0)

#define LSR_REG(RR, pc_inc)       \
  do {                            \
      LOCAL_SET_CARRY(BT(RR, 0)); \
      RR >>= 1;                   \
      LOCAL_SET_NEGATIVE(0);      \
      LOCAL_SET_ZERO(!RR);        \
      PC_INC(pc_inc);             \
  } while (0)

#define LSR(ma, pc_inc)     \
  do {                      \
      BYTE val;             \
                            \
      val = LOAD(ma);       \
      LSR_REG(val, pc_inc); \
      STORE(ma, val);       \
  } while (0)

#define MUL()                        \
  do {                               \
      reg_d = reg_a * reg_b;         \
      LOCAL_SET_CARRY(BT(reg_b, 7)); \
      LOCAL_SET_ZERO(!reg_d);        \
      PC_INC(1);                     \
  } while (0)

#define NEG_REG(RR, bits, pc_inc)                       \
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
  } while (0)

#define NOP(pc_inc) \
  (PC_INC(pc_inc))

#define NEG(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      NEG_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define OIM(m, ma, pc_inc)            \
  do {                                \
      BYTE tmp;                       \
                                      \
      tmp = LOAD(ma);                 \
      tmp |= m;                       \
      LOCAL_SET_OVERFLOW(0);          \
      LOCAL_SET_NEGATIVE(BT(tmp, 7)); \
      LOCAL_SET_ZERO(!tmp);           \
      STORE(ma, tmp);                 \
      PC_INC(pc_inc);                 \
  } while (0)

#define OR(RR, bits, m, pc_inc)             \
  do {                                      \
      RR |= m;                              \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_ZERO(!RR);                  \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      PC_INC(pc_inc);                       \
  } while (0)

#define PSHS(m)                  \
  do {                           \
      if (m & 0x80) {            \
          PUSHS(reg_pc & 0xff);  \
          PUSHS(reg_pc >> 8);    \
      }                          \
      if (m & 0x40) {            \
          PUSHS(reg_usp & 0xff); \
          PUSHS(reg_usp >> 8);   \
      }                          \
      if (m & 0x20) {            \
          PUSHS(reg_y & 0xff);   \
          PUSHS(reg_y >> 8);     \
      }                          \
      if (m & 0x10) {            \
          PUSHS(reg_x & 0xff);   \
          PUSHS(reg_x >> 8);     \
      }                          \
      if (m & 8) {               \
          PUSHS(reg_dpr);        \
      }                          \
      if (m & 4) {               \
          PUSHS(reg_b);          \
      }                          \
      if (m & 2) {               \
          PUSHS(reg_a);          \
      }                          \
      if (m & 1) {               \
          PUSHS(reg_p);          \
      }                          \
      PC_INC(2);                 \
  } while (0)

#define PSHSW()     \
  do {              \
      PUSHS(reg_f); \
      PUSHS(reg_e); \
  } while (0)

#define PSHU(m)                  \
  do {                           \
      if (m & 0x80) {            \
          PUSHU(reg_pc & 0xff);  \
          PUSHU(reg_pc >> 8);    \
      }                          \
      if (m & 0x40) {            \
          PUSHU(reg_ssp & 0xff); \
          PUSHU(reg_ssp >> 8);   \
      }                          \
      if (m & 0x20) {            \
          PUSHU(reg_y & 0xff);   \
          PUSHU(reg_y >> 8);     \
      }                          \
      if (m & 0x10) {            \
          PUSHU(reg_x & 0xff);   \
          PUSHU(reg_x >> 8);     \
      }                          \
      if (m & 8) {               \
          PUSHU(reg_dpr);        \
      }                          \
      if (m & 4) {               \
          PUSHU(reg_b);          \
      }                          \
      if (m & 2) {               \
          PUSHU(reg_a);          \
      }                          \
      if (m & 1) {               \
          PUSHU(reg_p);          \
      }                          \
      PC_INC(2);                 \
  } while (0)

#define PSHUW()     \
  do {              \
      PUSHU(reg_f); \
      PUSHU(reg_e); \
  } while (0)

#define PULS(m)                   \
  do {                            \
      if (m & 1) {                \
          reg_p = PULLS();        \
      }                           \
      if (m & 2) {                \
          reg_a = PULLS();        \
      }                           \
      if (m & 4) {                \
          reg_b = PULLS();        \
      }                           \
      if (m & 8) {                \
          reg_dpr = PULLS();      \
      }                           \
      if (m & 0x10) {             \
          reg_x = PULLS() << 8;   \
          reg_x |= PULLS();       \
      }                           \
      if (m & 0x20) {             \
          reg_y = PULLS() << 8;   \
          reg_y |= PULLS();       \
      }                           \
      if (m & 0x40) {             \
          reg_usp = PULLS() << 8; \
          reg_usp |= PULLS();     \
      }                           \
      if (m & 0x80) {             \
          reg_pc = PULLS() << 8;  \
          reg_pc |= PULLS();      \
      }                           \
      PC_INC(2);                  \
  } while (0)

#define PULSW()        \
  do {                 \
      reg_e = PULLS(); \
      reg_f = PULLS(); \
  } while (0)

#define PULU(m)                   \
  do {                            \
      if (m & 1) {                \
          reg_p = PULLU();        \
      }                           \
      if (m & 2) {                \
          reg_a = PULLU();        \
      }                           \
      if (m & 4) {                \
          reg_b = PULLU();        \
      }                           \
      if (m & 8) {                \
          reg_dpr = PULLU();      \
      }                           \
      if (m & 0x10) {             \
          reg_x = PULLU() << 8;   \
          reg_x |= PULLU();       \
      }                           \
      if (m & 0x20) {             \
          reg_y = PULLU() << 8;   \
          reg_y |= PULLU();       \
      }                           \
      if (m & 0x40) {             \
          reg_ssp = PULLU() << 8; \
          reg_ssp |= PULLU();     \
      }                           \
      if (m & 0x80) {             \
          reg_pc = PULLU() << 8;  \
          reg_pc |= PULLU();      \
      }                           \
      PC_INC(2);                  \
  } while (0)

#define PULUW()        \
  do {                 \
      reg_e = PULLU(); \
      reg_f = PULLU(); \
  } while (0)

#define ROL_REG(RR, bits, pc_inc)                              \
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
  } while (0)

#define ROL(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      ROL_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define ROR_REG(RR, bits, pc_inc)           \
  do {                                      \
      unsigned int tmp;                     \
                                            \
      tmp = LOCAL_CARRY();                  \
      RR >>= 1;                             \
      RR |= (tmp << (bits - 1));            \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      PC_INC(pc_inc);                       \
  } while (0)

#define ROR(ma, pc_inc)        \
  do {                         \
      BYTE val;                \
                               \
      val = LOAD(ma);          \
      ROR_REG(val, 8, pc_inc); \
      STORE(ma, val);          \
  } while (0)

#define RTI()                     \
  do {                            \
      reg_p = PULLS();            \
      if (LOCAL_ENTIRE()) {       \
          reg_a = PULLS();        \
          reg_b = PULLS();        \
          if (!reg_emul) {        \
              reg_e = PULLS();    \
              reg_f = PULLS();    \
          }                       \
          reg_dpr = PULLS();      \
          reg_x = PULLS() << 8;   \
          reg_x |= PULLS();       \
          reg_y = PULLS() << 8;   \
          reg_y |= PULLS();       \
          reg_usp = PULLS() << 8; \
          reg_usp |= PULLS();     \
      }                           \
      reg_pc = PULLS() << 8;      \
      reg_pc |= PULLS();          \
  } while (0)

#define RTS()                \
  do {                       \
      reg_pc = PULLS() << 8; \
      reg_pc |= PULLS();     \
  } while (0)

#define SEX()                              \
  do {                                     \
      LOCAL_SET_NEGATIVE(BT(reg_b, 7));    \
      reg_a = LOCAL_NEGATIVE() ? 0xff : 0; \
      LOCAL_SET_ZERO(!reg_a);              \
      PC_INC(1);                           \
  } while (0)

#define ST(RR, bits, ma)                    \
  do {                                      \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      if (bits == 8) {                      \
          STORE(ma, RR);                    \
      }                                     \
      if (bits == 16) {                     \
          STORE(ma, RR >> 8);               \
          STORE(ma + 1, RR & 0xff);         \
      }                                     \
      if (bits == 32) {                     \
          STORE(ma, RR >> 24);              \
          STORE(ma + 1, (RR >> 16) & 0xff); \
          STORE(ma + 2, (RR >> 8) & 0xff);  \
          STORE(ma + 3, RR & 0xff);         \
      }                                     \
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
  } while (0)

#define SUB(RR, CC, m, bits, pc_inc)                                     \
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
  } while (0)

#define SWI_REG(nr)                   \
  do {                                \
      LOCAL_SET_ENTIRE(1);            \
      PUSHS(reg_pc & 0xff);           \
      PUSHS(reg_pc >> 8);             \
      PUSHS(reg_usp & 0xff);          \
      PUSHS(reg_usp >> 8);            \
      PUSHS(reg_y & 0xff);            \
      PUSHS(reg_y >> 8);              \
      PUSHS(reg_x & 0xff);            \
      PUSHS(reg_x >> 8);              \
      PUSHS(reg_dpr);                 \
      if (!reg_emul) {                \
          PUSHS(reg_f);               \
          PUSHS(reg_e);               \
      }                               \
      PUSHS(reg_b);                   \
      PUSHS(reg_a);                   \
      PUSHS(reg_p);                   \
      if (nr == 1) {                  \
          LOCAL_SET_FIRQ(1);          \
          LOCAL_SET_IRQ(1);           \
          reg_pc = LOAD(0xfffa) << 8; \
          reg_pc |= LOAD(0xfffb);     \
      }                               \
      if (nr == 2) {                  \
          reg_pc = LOAD(0xfff4) << 8; \
          reg_pc |= LOAD(0xfff5);     \
      }                               \
      if (nr == 3) {                  \
          reg_pc = LOAD(0xfff2) << 8; \
          reg_pc |= LOAD(0xfff3);     \
      }                               \
  } while (0)

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
      PC_INC(2);                           \
  } while (0)

#define TIM(m, ma, pc_inc)          \
  do {                              \
      LOCAL_SET_OVERFLOW(0);        \
      LOCAL_SET_NEGATIVE(BT(m, 7)); \
      LOCAL_SET_ZERO(!m);           \
      STORE(ma, m);                 \
      PC_INC(pc_inc);               \
  } while (0)

#define TST_REG(RR, bits, pc_inc)           \
  do {                                      \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
      LOCAL_SET_ZERO(!RR);                  \
      PC_INC(pc_inc);                       \
  while (0)

#define TST(ma, pc_inc) \
  (TST_REG(LOAD(ma), 8), pc_inc)

/* ------------------------------------------------------------------------- */

#define ADCR_WRAP(r0, r1, bits) \
  (ADD(r1, r0, LOCAL_CARRY(), bits))

#define ADDR_WRAP(r0, r1, bits) \
  (ADD(r1, r0, 0, bits))

#define ANDR_WRAP(r0, r1, bits, pc_inc) \
  (AND(r1, r0, bits, pc_inc))

#define CMPR_WRAP(r0, r1, bits) \
  (CMP(r1, bits, r0))

#define EORR_WRAP(r0, r1, bits) \
  (EOR(r1, bits, r0))

#define ORR_WRAP(r0, r1, bits, pc_inc) \
  (OR(r1, bits, r0, pc_inc))

#define SBCR_WRAP(r0, r1, bits) \
  (SUB(r1, LOCAL_CARRY(), r0, bits))

#define SUBR_WRAP(r0, r1, bits, pc_inc) \
  (SUB(r1, 0, r0, bits, pc_inc))

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
        SET_LAST_ADDR(reg_pc);
        FETCH_OPCODE(opcode, page);

trap_skipped:
        switch ((page << 8) | p0) {

          case 0x0000:          /* NEG direct */
            NEG(p1 | (reg_dpr << 8), 2);
            break;

          case 0x0001:          /* OIM IM-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            OIM(p1, (reg_dpr << 8) | p2, 3);
            break;

          case 0x0002:          /* AIM IM-direct, also used for traps, FIXME: fix for 6809, 6309 only opcode */
            STATIC_ASSERT(TRAP_OPCODE == 0x02);
            AIM_02();
            break;

          case 0x0003:          /* COM direct */
            COM((reg_dpr << 8) | p1, 2);
            break;

          case 0x0004:          /* LSR direct */
            LSR((reg_dpr << 8) | p1, 2);
            break;

          case 0x0005:          /* EIM IM-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            EIM(p1, (reg_dpr << 8) | p2, 3);
            break;

          case 0x0006:          /* ROR direct */
            ROR((reg_dpr << 8) | p1, 2);
            break;

          case 0x0007:          /* ASR direct */
            ASR((reg_dpr << 8) | p1, 2);
            break;

          case 0x0008:          /* ASL/LSL direct */
            ASL((reg_dpr << 8) | p1, 2);
            break;

          case 0x0009:          /* ROL direct */
            ROL((reg_dpr << 8) | p1, 2);
            break;

          case 0x000a:          /* DEC direct */
            DEC((reg_dpr << 8) | p1, 2);
            break;

          case 0x000b:          /* TIM IM-direct */   /* FIXME: fix for 6809, 6309 only opcode */
            TIM(p1, (reg_dpr << 8) | p2, 3);
            break;

          case 0x000c:          /* INC direct */
            INC((reg_dpr << 8) | p1, 2);
            break;

          case 0x000d:          /* TST direct */
            TST((reg_dpr << 8) | p1, 2);
            break;

          case 0x000e:          /* JMP direct */
            JMP((reg_dpr << 8) | p1);
            break;

          case 0x000f:          /* CLR direct */
            CLR((reg_dpr << 8) | p1, 2);
            break;

          case 0x0012:          /* NOP */
            NOP(1);
            break;

          case 0x13:            /* SLO ($nn),Y */
            SLO_IND_Y(p1);
            break;

          case 0x14:            /* NOOP $nn,X */
            NOOP(CLK_NOOP_ZERO_X, 2);
            break;

          case 0x0016:          /* LBRA offset */
            BRANCH_LONG(1, (p1 << 8) | p2, 3);
            break;

          case 0x0017:          /* LBSR offset */
            LBSR();
            break;

          case 0x0019:          /* DAA */
            DAA();
            break;

          case 0x001a:          /* ORCC immediate */
            OR(reg_p, 8, p1, 2);
            break;

          case 0x001c:          /* ANDCC immediate */
            AND(reg_p, p1, 8, 2);
            break;

          case 0x001d:          /* SEX (Sign EXtend, not the other thing ;) */
            SEX();
            break;

          case 0x001e:          /* EXG registers */
            EXG(p1);
            break;

          case 0x001f:          /* TFR registers */
            TFR(p1);
            break;

          case 0x0020:          /* BRA offset */
            BRANCH(1, p1, 2);
            break;

          case 0x0021:          /* BRN offset */
            BRANCH(0, p1, 2);
            break;

          case 0x0022:          /* BHI offset */
            BRANCH(!(LOCAL_CARRY() | LOCAL_ZERO()), p1, 2);
            break;

          case 0x0023:          /* BLS offset */
            BRANCH(LOCAL_CARRY() | LOCAL_ZERO(), p1, 2);
            break;

          case 0x0024:          /* BHS/BCC offset */
            BRANCH(!LOCAL_CARRY(), p1, 2);
            break;

          case 0x0025:          /* BLO/BCS offset */
            BRANCH(LOCAL_CARRY(), p1, 2);
            break;

          case 0x0026:          /* BNE offset */
            BRANCH(!LOCAL_ZERO(), p1, 2);
            break;

          case 0x0027:          /* BEQ offset */
            BRANCH(LOCAL_ZERO(), p1, 2);
            break;

          case 0x0028:          /* BVC offset */
            BRANCH(!LOCAL_OVERFLOW(), p1, 2);
            break;

          case 0x0029:          /* BVS offset */
            BRANCH(LOCAL_OVERFLOW(), p1, 2);
            break;

          case 0x002a:          /* BPL offset */
            BRANCH(!LOCAL_NEGATIVE(), p1, 2);
            break;

          case 0x002b:          /* BMI offset */
            BRANCH(LOCAL_NEGATIVE(), p1, 2);
            break;

          case 0x002c:          /* BGE offset */
            BRANCH(!(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), p1, 2);
            break;

          case 0x002d:          /* BLT offset */
            BRANCH(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW(), p1, 2);
            break;

          case 0x002e:          /* BGT offset */
            BRANCH((!LOCAL_ZERO() & !(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW())), p1, 2);
            break;

          case 0x002f:          /* BLE offset */
            BRANCH(LOCAL_ZERO() | (LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), p1, 2);
            break;

          case 0x30:            /* BMI $nnnn */
            BRANCH(LOCAL_SIGN(), p1);
            break;

          case 0x31:            /* AND ($nn),Y */
            AND(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0x33:            /* RLA ($nn),Y */
            RLA_IND_Y(p1);
            break;

          case 0x0034:          /* PSHS immediate */
            PSHS(p1);
            break;

          case 0x0035:          /* PULS immediate */
            PULS(p1);
            break;

          case 0x0036:          /* PSHU immediate */
            PSHU(p1);
            break;

          case 0x0037:          /* PULU immeditate */
            PULU(p1);
            break;

          case 0x0039:          /* RTS */
            RTS();
            break;

          case 0x003a:          /* ABX */
            ABX();
            break;

          case 0x003b:          /* RTI */
            RTI();
            break;

          case 0x003d:            /* MUL */
            MUL();
            break;

          case 0x003f:          /* SWI */
            SWI(1);
            break;

          case 0x0040:          /* NEGA */
            NEG_REG(reg_a, 8, 1);
            break;

          case 0x0043:          /* COMA */
            COM_REG(reg_a, 8, 1);
            break;

          case 0x0044:          /* LSRA */
            LSR_REG(reg_a, 1);
            break;

          case 0x0046:          /* RORA */
            ROR_REG(reg_a, 8, 1);
            break;

          case 0x0047:          /* ASRA */
            ASR_REG(reg_a, 8, 1);
            break;

          case 0x0048:          /* ASLA/LSLA */
            ASL_REG(reg_a, 8, 1);
            break;

          case 0x0049:          /* ROLA */
            ROL_REG(reg_a, 8, 1);
            break;

          case 0x004a:          /* DECA */
            DEC_REG(reg_a, 8, 1);
            break;

          case 0x004c:          /* INCA */
            INC_REG(reg_a, 8, 1);
            break;

          case 0x004d:          /* TSTA */
            TST_REG(reg_a, 8, 1);
            break;

          case 0x004f:          /* CLRA */
            CLR_REG(reg_a, 1);
            break;

          case 0x0050:          /* NEGB */
            NEG_REG(reg_b, 8, 1);
            break;

          case 0x0053:          /* COMB */
            COM_REG(reg_b, 8, 1);
            break;

          case 0x0054:          /* LSRB */
            LSR_REG(reg_b, 1);
            break;

          case 0x0056:          /* RORB */
            ROR_REG(reg_b, 8, 1);
            break;

          case 0x0057:          /* ASRB */
            ASR_REG(reg_b, 8, 1);
            break;

          case 0x0058:          /* ASLB/LSLB */
            ASL_REG(reg_b, 8, 1);
            break;

          case 0x0059:          /* ROLB */
            ROL_REG(reg_b, 8, 1);
            break;

          case 0x005a:          /* DECB */
            DEC_REG(reg_b, 8, 1);
            break;

          case 0x005c:          /* INCB */
            INC_REG(reg_b, 8, 1);
            break;

          case 0x005d:          /* TSTB */
            TST_REG(reg_b, 8, 1);
            break;

          case 0x005f:          /* CLRB */
            CLR_REG(reg_b, 1);
            break;

          case 0x0060:            /* NEG indexed */
            NEG(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0061:          /* OIM IM-indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            OIM(p1, GET_IND_MA(p2, p3, p4), 3);
            break;

          case 0x0062:          /* AIM IM-indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            AIM(p1, GET_IND_MA(p2, p3, p4), 3);
            break;

          case 0x0063:          /* COM indexed */
            COM(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0064:          /* LSR indexed */
            LSR(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0065:          /* EIM IM-indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            EIM(p1, GET_IND_MA(p2, p3, p4), 3);
            break;

          case 0x0066:          /* ROR indexed */
            ROR(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0067:          /* ASR indexed */
            ASR(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0068:          /* ASL/LSL indexed */
            ASL(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0069:            /* ROL indexed */
            ROL(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x006a:          /* DEC indexed */
            DEC(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x006b:          /* TIM IM-indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            TIM(p1, GET_IND_MA(p2, p3, p4), 3);
            break;

          case 0x006c:          /* INC indexed */
            INC(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x006d:          /* TST indexed */
            TST(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x006e:          /* JMP indexed */
            JMP(GET_IND_MA(p1, p2, p3));
            break;

          case 0x006f:          /* CLR indexed */
            CLR(GET_IND_MA(p1, p2, p3), 2);
            break;

          case 0x0070:          /* NEG extended */
            NEG((p1 << 8) | p2, 3);
            break;

          case 0x0071:          /* OIM IM-extended */   /* FIXME: fix for 6809, 6309 only opcode */
            OIM(p1, (p2 << 8) | p3, 4);
            break;

          case 0x0072:          /* AIM IM-extended */   /* FIXME: fix for 6809, 6309 only opcode */
            AIM(p1, (p2 << 8) | p3, 4);
            break;

          case 0x0073:          /* COM extended */
            COM((p1 << 8) | p2, 3);
            break;

          case 0x0074:          /* LSR extended */
            LSR((p1 << 8) | p2, 3);
            break;

          case 0x0075:          /* EIM IM-extended */   /* FIXME: fix for 6809, 6309 only opcode */
            EIM(p1, (p2 << 8) | p3, 4);
            break;

          case 0x0076:          /* ROR extended */
            ROR((p1 << 8) | p2, 3);
            break;

          case 0x0077:          /* ASR extended */
            ASR((p1 << 8) | p2, 3);
            break;

          case 0x0078:          /* ASL extended */
            ASL((p1 << 8) | p2, 3);
            break;

          case 0x0079:          /* ROL extended */
            ROL((p1 << 8) | p2, 3);
            break;

          case 0x007a:          /* DEC extended */
            DEC((p1 << 8) | p2, 3);
            break;

          case 0x007b:          /* TIM IM-extended */   /* FIXME: fix for 6809, 6309 only opcode */
            TIM(p1, (p2 << 8) | p3, 4);
            break;

          case 0x007c:          /* INC extended */
            INC((p1 << 8) | p2, 3);
            break;

          case 0x007d:          /* TST extended */
            TST((p1 << 8) | p2, 3);
            break;

          case 0x007e:          /* JMP extended */
            JMP((p1 << 8) | p2);
            break;

          case 0x007f:          /* CLR extended */
            CLR((p1 << 8) | p2, 3);
            break;

          case 0x0080:          /* SUBA immediate */
            SUB(reg_a, 0, p1, 8, 2);
            break;

          case 0x0081:          /* CMPA immediate */
            CMP(reg_a, 8, p1);
            break;

          case 0x0082:          /* SBCA immediate */
            SUB(reg_a, LOCAL_CARRY(), p1, 8);
            break;

          case 0x0083:          /* SUBD immediate */
            SUB(reg_d, 0, (p1 << 8) | p2, 16, 3);
            break;

          case 0x0084:          /* ANDA immediate */
            AND(reg_a, p1, 8, 2);
            break;

          case 0x0085:          /* BITA immediate */
            BIT(reg_a, 8, p1);
            break;

          case 0x0086:            /* LDA immediate */
            LD(reg_a, 8, p1);
            break;

          case 0x0088:          /* EORA immediate */
            EOR(reg_a, 8, p1);
            break;

          case 0x0089:          /* ADCA immediate */
            ADD(reg_a, p1, LOCAL_CARRY(), 8);
            break;

          case 0x008a:          /* ORA immediate */
            OR(reg_a, 8, p1, 2);
            break;

          case 0x008b:          /* ADDA immediate */
            ADD(reg_a, p1, 0, 8);
            break;

          case 0x008c:          /* CMPX immediate */
            CMP(reg_x, 16, (p1 << 8) | p2);
            break;

          case 0x008d:          /* BSR offset */
            BSR();
            break;

          case 0x008e:            /* LDX immediate */
            LD(reg_x, 16, (p1 << 8) | p2);
            break;

          case 0x0090:          /* SUBA direct */
            SUB(reg_a, 0, LOAD_DIRECT8(p1), 8, 2);
            break;

          case 0x0091:          /* CMPA direct */
            CMP(reg_a, 8, LOAD_DIRECT8(p1));
            break;

          case 0x0092:          /* SBCA direct */
            SUB(reg_a, LOCAL_CARRY(), LOAD_DIRECT8(p1), 8);
            break;

          case 0x0093:          /* SUBD direct */
            SUB(reg_d, 0, LOAD_DIRECT16(p1), 16, 2);
            break;

          case 0x0094:          /* ANDA direct */
            AND(reg_a, LOAD_DIRECT8(p1), 8, 2);
            break;

          case 0x0095:          /* BITA direct */
            BIT(reg_a, 8, LOAD_DIRECT8(p1));
            break;

          case 0x0096:            /* LDA direct */
            LD(reg_a, 8, LOAD_DIRECT8(p1));
            break;

          case 0x97:            /* SAX $nn,Y */
            SAX((p1 + reg_y) & 0xff, 0, CLK_ZERO_I_STORE, 2);
            break;

          case 0x0098:          /* EORA direct */
            EOR(reg_a, 8, LOAD_DIRECT8(p1));
            break;

          case 0x0099:          /* ADCA direct */
            ADD(reg_a, LOAD_DIRECT8(p1), LOCAL_CARRY(), 8);
            break;

          case 0x009a:          /* ORA direct */
            OR(reg_a, 8, LOAD_DIRECT8(p1), 2);
            break;

          case 0x009b:          /* ADDA direct */
            ADD(reg_a, LOAD_DIRECT8(p1), 0, 8);
            break;

          case 0x009c:          /* CMPX direct */
            CMP(reg_x, 16, LOAD_DIRECT16(p1));
            break;

          case 0x009d:          /* JSR direct */
            JSR((reg_dpr << 8) | p1);
            break;

          case 0x009e:            /* LDX direct */
            LD(reg_x, 16, LOAD_DIRECT16(p1));
            break;

          case 0x9f:            /* SHA $nnnn,Y */
            SHA_ABS_Y(p2);
            break;

          case 0x00a0:          /* SUBA indexed */
            SUB(reg_a, 0, LOAD_IND8(), 8, 2);
            break;

          case 0x00a1:          /* CMPA indexed */
            CMP(reg_a, 8, LOAD_IND8());
            break;

          case 0x00a2:          /* SBCA indexed */
            SUB(reg_a, LOCAL_CARRY(), LOAD_IND8(), 8, 2);
            break;

          case 0x00a3:          /* SUBD indexed */
            SUB(reg_d, 0, LOAD_IND16(), 16);
            break;

          case 0x00a4:          /* ANDA indexed */
            AND(reg_a, LOAD_IND8(), 8, 2);
            break;

          case 0x00a5:          /* BITA indexed */
            BIT(reg_a, 8, LOAD_IND8());
            break;

          case 0x00a6:            /* LDA indexed */
            LD(reg_a, 8, LOAD_IND8());
            break;

          case 0xa7:            /* LAX $nn */
            LAX(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x00a8:          /* EORA indexed */
            EOR(reg_a, 8, LOAD_IND8());
            break;

          case 0x00a9:          /* ADCA indexed */
            ADD(reg_a, LOAD_IND8(), LOCAL_CARRY(), 8);
            break;

          case 0x00aa:          /* ORA indexed */
            OR(reg_a, 8, LOAD_IND8(), 2);
            break;

          case 0x00ab:          /* ADDA indexed */
            ADD(reg_a, LOAD_IND8(), 0, 8);
            break;

          case 0x00ac:          /* CMPX indexed */
            CMP(reg_x, 16, LOAD_IND16());
            break;

          case 0x00ad:          /* JSR indexed */
            JSR(GET_IND_MA(p1, p2, p3));
            break;

          case 0x00ae:            /* LDX indexed */
            LD(reg_x, 16, LOAD_IND16());
            break;

          case 0xaf:            /* LAX $nnnn */
            LAX(LOAD(p2), 1, 3);
            break;

          case 0x00b0:            /* SUBA extended */
            SUB(reg_a, 0, LOAD_EXT8(), 8, 3);
            break;

          case 0x00b1:          /* CMPA extended */
            CMP(reg_a, 8, LOAD_EXT8());
            break;

          case 0x00b2:          /* SBCA extended */
            SUB(reg_a, LOCAL_CARRY(), LOAD_EXT8(), 8);
            break;

          case 0x00b3:          /* SUBD extended */
            SUB(reg_d, 0, LOAD_EXT16(), 16, 3);
            break;

          case 0x00b4:          /* ANDA extended */
            AND(reg_a, LOAD_EXT8(), 8, 3);
            break;

          case 0x00b5:          /* BITA extended */
            BIT(reg_a, 8, LOAD_EXT8());
            break;

          case 0x00b6:            /* LDA extended */
            LD(reg_a, 8, LOAD_EXT8());
            break;

          case 0xb7:            /* LAX $nn,Y */
            LAX(LOAD_ZERO_Y(p1), CLK_ZERO_I2, 2);
            break;

          case 0x00b8:          /* EORA extended */
            EOR(reg_a, 8, LOAD_EXT8());
            break;

          case 0x00b9:          /* ADCA extended */
            ADD(reg_a, LOAD_EXT8(), LOCAL_CARRY(), 8);
            break;

          case 0x00ba:          /* ORA extended */
            OR(reg_a, 8, LOAD_EXT8(), 3);
            break;

          case 0x00bb:          /* ADDA extended */
            ADD(reg_a, LOAD_EXT8(), 0, 8);
            break;

          case 0x00bc:          /* CMPX extended */
            CMP(reg_x, 16, LOAD_EXT16());
            break;

          case 0x00bd:          /* JSR extended */
            JSR((p1 << 8) | p2);
            break;

          case 0x00be:            /* LDX extended */
            LD(reg_x, 16, LOAD_EXT16());
            break;

          case 0xbf:            /* LAX $nnnn,Y */
            LAX(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x00c0:          /* SUBB immediate */
            SUB(reg_b, 0, p1, 8, 2);
            break;

          case 0x00c1:          /* CMPB immediate */
            CMP(reg_b, 8, p1);
            break;

          case 0x00c2:          /* SBCB immediate */
            SUB(reg_b, LOCAL_CARRY(), p1, 8);
            break;

          case 0x00c3:          /* ADDD immediate */
            ADD(reg_d, (p1 << 8) | p2, 0, 16);
            break;

          case 0x00c4:          /* ANDB immediate */
            AND(reg_b, p1, 8, 2);
            break;

          case 0x00c5:          /* BITB immediate */
            BIT(reg_b, 8, p1);
            break;

          case 0x00c6:            /* LDB immediate */
            LD(reg_b, 8, p1);
            break;

          case 0x00c8:          /* EORB immediate */
            EOR(reg_b, 8, p1);
            break;

          case 0x00c9:          /* ADCB immediate */
            ADD(reg_b, p1, LOCAL_CARRY(), 8);
            break;

          case 0x00ca:          /* ORB immediate */
            OR(reg_b, 8, p1, 2);
            break;

          case 0x00cb:          /* ADDB immediate */
            ADD(reg_b, p1, 0, 8);
            break;

          case 0x00cc:            /* LDD immediate */
            LD(reg_d, 16, (p1 << 8) | p2);
            break;

          case 0x00cd:            /* LDQ immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, (p1 << 24) | (p2 << 16) | (p3 << 8) | p4);
            break;

          case 0x00ce:            /* LDU immediate */
            LD(reg_usp, 16, (p1 << 8) | p2);
            break;

          case 0x00d0:          /* SUBB direct */
            SUB(reg_b, 0, LOAD_DIRECT8(p1), 8, 2);
            break;

          case 0x00d1:          /* CMPB direct */
            CMP(reg_b, 8, LOAD_DIRECT8(p1));
            break;

          case 0x00d2:          /* SBCB direct */
            SUB(reg_b, LOCAL_CARRY(), LOAD_DIRECT8(p1), 8);
            break;

          case 0x00d3:          /* ADDD direct */
            ADD(reg_d, LOAD_DIRECT16(p1), 0, 16);
            break;

          case 0x00d4:          /* ANDB direct */
            AND(reg_b, LOAD_DIRECT8(p1), 8, 2);
            break;

          case 0x00d5:          /* BITB direct */
            BIT(reg_b, 8, LOAD_DIRECT8(p1));
            break;

          case 0x00d6:            /* LDB direct */
            LD(reg_b, 8, LOAD_DIRECT8(p1));
            break;

          case 0xd7:            /* DCP $nn,X */
            DCP((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0x00d8:          /* EORB direct */
            EOR(reg_b, 8, LOAD_DIRECT8(p1));
            break;

          case 0x00d9:          /* ADCB direct */
            ADD(reg_b, LOAD_DIRECT8(p1), LOCAL_CARRY(), 8);
            break;

          case 0x00da:          /* ORB direct */
            OR(reg_b, 8, LOAD_DIRECT8(p1), 2);
            break;

          case 0x00db:          /* ADDB direct */
            ADD(reg_b, LOAD_DIRECT8(p1), 0, 8);
            break;

          case 0x00dc:          /* LDD direct */
            LD(reg_d, 16, LOAD_DIRECT16(p1));
            break;

          case 0xdd:            /* CMP $nnnn,X */
            CMP(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x00de:            /* LDU direct */
            LD(reg_u, 16, LOAD_DIRECT16(p1));
            break;

          case 0xdf:            /* DCP $nnnn,X */
            DCP(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x00e0:          /* SUBB indexed */
            SUB(reg_b, 0, LOAD_IND8(), 8, 2);
            break;

          case 0x00e1:          /* CMPB indexed */
            CMP(reg_b, 8, LOAD_IND8());
            break;

          case 0x00e2:          /* SBCB indexed */
            SUB(reg_b, LOCAL_CARRY(), LOAD_IND8(), 8);
            break;

          case 0x00e3:          /* ADDD indexed */
            ADD(reg_d, LOAD_IND16(), 0, 16);
            break;

          case 0x00e4:          /* ANDB indexed */
            AND(reg_b, LOAD_IND8(), 8, 2);
            break;

          case 0x00e5:          /* BITB indexed */
            BIT(reg_b, 8, LOAD_IND8());
            break;

          case 0x00e6:            /* LDB indexed */
            LD(reg_b, 8, LOAD_IND8());
            break;

          case 0xe7:            /* ISB $nn */
            ISB(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x00e8:          /* EORB indexed */
            EOR(reg_b, 8, LOAD_IND8());
            break;

          case 0x00e9:          /* ADCB indexed */
            ADD(reg_b, LOAD_IND8(), LOCAL_CARRY(), 8);
            break;

          case 0x00ea:          /* ORB indexed */
            OR(reg_b, 8, LOAD_IND8(), 2);
            break;

          case 0x00eb:          /* ADDB indexed */
            ADD(reg_b, LOAD_IND8(), 0, 8);
            break;

          case 0x00ec:            /* LDD indexed */
            LD(reg_d, 16, LOAD_IND16());
            break;

          case 0xed:            /* SBC $nnnn */
            SBC(LOAD(p2), 1, 3);
            break;

          case 0x00ee:            /* LDU indexed */
            LDU(reg_usp, 16, LOAD_IND16());
            break;

          case 0xef:            /* ISB $nnnn */
            ISB(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x00f0:          /* SUBB extended */
            SUB(reg_b, 0, LOAD_EXT8(), 8, 3);
            break;

          case 0x00f1:          /* CMPB extended */
            CMP(reg_b, 8, LOAD_EXT8());
            break;

          case 0x00f2:          /* SBCB extended */
            SUB(reg_b, LOCAL_CARRY(), LOAD_EXT8(), 8);
            break;

          case 0x00f3:          /* ADDD extended */
            ADD(reg_d, LOAD_EXT16(), 0, 16);
            break;

          case 0x00f4:          /* ANDB extended */
            AND(reg_b, LOAD_EXT8(), 8, 3);
            break;

          case 0x00f5:          /* BITB extended */
            BIT(reg_b, 8, LOAD_EXT8());
            break;

          case 0x00f6:          /* LDB extended */
            LD(reg_b, 8, LOAD_EXT8());
            break;

          case 0xf7:            /* ISB $nn,X */
            ISB((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x00f8:          /* EORB extended */
            EOR(reg_b, 8, LOAD_EXT8());
            break;

          case 0x00f9:          /* ADCB extended */
            ADD(reg_b, LOAD_EXT8(), LOCAL_CARRY(), 8);
            break;

          case 0x00fa:          /* ORB extended */
            OR(reg_b, 8, LOAD_EXT8(), 3);
            break;

          case 0x00fb:          /* ADDB extended */
            ADD(reg_b, LOAD_EXT8(), 0, 8);
            break;

          case 0x00fc:          /* LDD extended */
            LD(reg_d, 16, LOAD_EXT16());
            break;

          case 0xfd:            /* SBC $nnnn,X */
            SBC(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x00fe:          /* LDU extended */
            LD(reg_usp, 16, LOAD_EXT16());
            break;

          case 0xff:            /* ISB $nnnn,X */
            ISB(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x1021:          /* LBRN offset */
            BRANCH_LONG(0, (p1 << 8) | p2, 4);
            break;

          case 0x1022:          /* LBHI offset */
            BRANCH_LONG(!(LOCAL_CARRY() | LOCAL_ZERO()), (p1 << 8) | p2, 4);
            break;

          case 0x1023:          /* LBLS offset */
            BRANCH_LONG(LOCAL_CARRY() | LOCAL_ZERO(), (p1 << 8) | p2, 4);
            break;

          case 0x1024:          /* LBHS/LBCC offset */
            BRANCH_LONG(!LOCAL_CARRY(), (p1 << 8) | p2, 4);
            break;

          case 0x1025:          /* LBCS/LBLO offset */
            BRANCH_LONG(LOCAL_CARRY(), (p1 << 8) | p2, 4);
            break;

          case 0x1026:          /* LBNE offset */
            BRANCH_LONG(!LOCAL_ZERO(), (p1 << 8) | p2, 4);
            break;

          case 0x1027:          /* LBEQ offset */
            BRANCH_LONG(LOCAL_ZERO(), (p1 << 8) | p2, 4);
            break;

          case 0x1028:          /* LBVC offset */
            BRANCH_LONG(!LOCAL_OVERFLOW(), (p1 << 8) | p2, 4);
            break;

          case 0x1029:          /* LBVS offset */
            BRANCH_LONG(LOCAL_OVERFLOW(), (p1 << 8) | p2, 4);
            break;

          case 0x102a:          /* LBPL offset */
            BRANCH_LONG(!LOCAL_NEGATIVE(), (p1 << 8) | p2, 4);
            break;

          case 0x102b:          /* LBMI offset */
            BRANCH_LONG(LOCAL_NEGATIVE(), (p1 << 8) | p2, 4);
            break;

          case 0x102c:          /* LBGE offset */
            BRANCH_LONG(!(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW()), (p1 << 8) | p2, 4);
            break;

          case 0x102d:          /* LBLT offset */
            BRANCH_LONG(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW(), (p1 << 8) | p2, 4);
            break;

          case 0x102e:          /* LBGT offset */
            BRANCH_LONG((!LOCAL_ZERO() & !(LOCAL_NEGATIVE() ^ LOCAL_OVERFLOW())), (p1 << 8) | p2, 4);
            break;

          case 0x102f:          /* LBLE offset */
            BRANCH_LONG(LOCAL_ZERO() | (LOCAL_NEGATIVE ^ LOCAL_OVERFLOW()), (p1 << 8) | p2, 4);
            break;

          case 0x1030:          /* ADDR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, ADDR_WRAP);
            break;

          case 0x1031:          /* ADCR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, ADCR_WRAP);
            break;

          case 0x1032:          /* SUBR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, SUBR_WRAP, 3);
            break;

          case 0x1033:          /* SBCR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, SBCR_WRAP);
            break;

          case 0x1034:          /* ANDR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, ANDR_WRAP, 3);
            break;

          case 0x1035:          /* ORR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, ORR_WRAP, 3);
            break;

          case 0x1036:          /* EORR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, EORR_WRAP);
            break;

          case 0x1037:          /* CMPR registers */   /* FIXME: fix for 6809, 6309 only opcode */
            R_WRAP(p1, CMPR_WRAP);
            break;

          case 0x1038:          /* PSHSW */   /* FIXME: fix for 6809, 6309 only opcode */
            PSHSW();
            break;

          case 0x1039:          /* PULSW */   /* FIXME: fix for 6809, 6309 only opcode */
            PULSW();
            break;

          case 0x103a:          /* PSHUW */   /* FIXME: fix for 6809, 6309 only opcode */
            PSHUW();
            break;

          case 0x103b:          /* PULUW */   /* FIXME: fix for 6809, 6309 only opcode */
            PULUW();
            break;

          case 0x103f:          /* SWI2 */
            SWI(2);
            break;

          case 0x1040:          /* NEGD */   /* FIXME: fix for 6809, 6309 only opcode */
            NEG_REG(reg_d, 16, 2);
            break;

          case 0x1043:          /* COMD */   /* FIXME: fix for 6809, 6309 only opcode */
            COM_REG(reg_d, 16, 2);
            break;

          case 0x1044:          /* LSRD */   /* FIXME: fix for 6809, 6309 only opcode */
            LSR_REG(reg_d, 2);
            break;

          case 0x1046:          /* RORD */   /* FIXME: fix for 6809, 6309 only opcode */
            ROR_REG(reg_d, 16, 2);
            break;

          case 0x1047:          /* ASRD */   /* FIXME: fix for 6809, 6309 only opcode */
            ASR_REG(reg_d, 16, 2);
            break;

          case 0x1048:          /* ASLD */   /* FIXME: fix for 6809, 6309 only opcode */
            ASL_REG(reg_d, 16, 2);
            break;

          case 0x1049:          /* ROLD */   /* FIXME: fix for 6809, 6309 only opcode */
            ROL_REG(reg_d, 16, 2);
            break;

          case 0x104a:          /* DECD */   /* FIXME: fix for 6809, 6309 only opcode */
            DEC_REG(reg_d, 16, 2);
            break;

          case 0x104c:          /* INCD */   /* FIXME: fix for 6809, 6309 only opcode */
            INC_REG(reg_d, 16, 2);
            break;

          case 0x104d:          /* TSTD */   /* FIXME: fix for 6809, 6309 only opcode */
            TST_REG(reg_d, 16, 2);
            break;

          case 0x104f:          /* CLRD */   /* FIXME: fix for 6809, 6309 only opcode */
            CLR_REG(reg_d, 2);
            break;

          case 0x1053:          /* COMW */   /* FIXME: fix for 6809, 6309 only opcode */
            COM_REG(reg_w, 16, 2);
            break;

          case 0x1054:          /* LSRW */   /* FIXME: fix for 6809, 6309 only opcode */
            LSR_REG(reg_w, 2);
            break;

          case 0x1056:          /* RORW */   /* FIXME: fix for 6809, 6309 only opcode */
            ROR_REG(reg_w, 16, 2);
            break;

          case 0x1059:          /* ROLW */   /* FIXME: fix for 6809, 6309 only opcode */
            ROL_REG(reg_w, 16, 2);
            break;

          case 0x105a:          /* DECW */   /* FIXME: fix for 6809, 6309 only opcode */
            DEC_REG(reg_w, 16, 2);
            break;

          case 0x105c:          /* INCW */   /* FIXME: fix for 6809, 6309 only opcode */
            INC_REG(reg_w, 16, 2);
            break;

          case 0x105d:          /* TSTW */   /* FIXME: fix for 6809, 6309 only opcode */
            TST_REG(reg_w, 16, 2);
            break;

          case 0x105f:          /* CLRW */   /* FIXME: fix for 6809, 6309 only opcode */
            CLR_REG(reg_w, 2);
            break;

          case 0x1080:          /* SUBW immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_w, 0, (p1 << 8) | p2, 16, 4);
            break;

          case 0x1081:          /* CMPW immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_w, 16, (p1 << 8) | p2);
            break;

          case 0x1082:          /* SBCD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_d, LOCAL_CARRY(), (p1 << 8) | p2, 16);
            break;

          case 0x1083:          /* CMPD immediate */
            CMP(reg_d, 16, (p1 << 8) | p2);
            break;

          case 0x1084:          /* ANDD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            AND(reg_d, (p1 << 8) | p2, 16, 4);
            break;

          case 0x1085:          /* BITD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_d, 16, (p1 << 8) | p2);
            break;

          case 0x1086:          /* LDW immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, (p1 << 8) | p2);
            break;

          case 0x1088:          /* EORD immediate */   /* FIXME: fix for 6809,6309 only opcode */
            EOR(reg_d, 16, (p1 << 8) | p2);
            break;

          case 0x1089:          /* ADCD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_d, (p1 << 8) | p2, LOCAL_CARRY(), 16);
            break;

          case 0x108a:          /* ORD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 16, (p1 << 8) | p2, 4);
            break;

          case 0x108b:          /* ADDW immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_w, (p1 << 8) | p2, 0, 16);
            break;

          case 0x108c:          /* CMPY immediate */
            CMP(reg_y, 16, (p1 << 8) | p2);
            break;

          case 0x108e:          /* LDY immediate */
            LD(reg_y, 16, (p1 << 8) | p2);
            break;

          case 0x1090:          /* SUBW direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_w, 0, LOAD_DIRECT16(p1), 16, 3);
            break;

          case 0x1091:          /* CMPW direct */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_w, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1092:          /* SBCD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_d, LOCAL_CARRY(), LOAD_DIRECT16(p1), 16);
            break;

          case 0x1093:          /* CMPD direct */
            CMP(reg_d, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1094:          /* ANDD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            AND(reg_d, LOAD_DIRECT_16(p1), 16, 3);
            break;

          case 0x1095:          /* BITD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_d, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1096:          /* LDW direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1098:          /* EORD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            EOR(reg_d, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1099:          /* ADCD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_d, LOAD_DIRECT16(p1), LOCAL_CARRY(), 16);
            break;

          case 0x109a:          /* ORD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 16, LOAD_DIRECT16(p1), 3);
            break;

          case 0x109b:          /* ADDW direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_w, LOAD_DIRECT16(p1), 0, 16);
            break;

          case 0x109c:          /* CMPY direct */
            CMP(reg_y, 16, LOAD_DIRECT16(p1));
            break;

          case 0x109e:          /* LDY direct */
            LD(reg_y, 16, LOAD_DIRECT16(p1));
            break;

          case 0x10a0:          /* SUBW indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_w, 0, LOAD_IND16(), 16, 3);
            break;

          case 0x10a1:          /* CMPW indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_w, 16, LOAD_IND16());
            break;

          case 0x10a2:          /* SBCD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_d, LOCAL_CARRY(), LOAD_IND16(), 16);
            break;

          case 0x10a3:          /* CMPD indexed */
            CMP(reg_d, 16, LOAD_IND16());
            break;

          case 0x10a4:          /* ANDD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            AND(reg_d, LOAD_IND16(), 16, 3);
            break;

          case 0x10a5:          /* BITD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_d, 16, LOAD_IND16());
            break;

          case 0x10a6:          /* LDW indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_IND16());
            break;

          case 0x10a8:          /* EORD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            EOR(reg_d, 16, LOAD_IND16());
            break;

          case 0x10a9:          /* ADCD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_d, LOAD_IND16(), LOCAL_CARRY(), 16);
            break;

          case 0x10aa:          /* ORD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 16, LOAD_IND16(), 3);
            break;

          case 0x10ab:          /* ADDW indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_w, LOAD_IND16(), 0, 16);
            break;

          case 0x10ac:          /* CMPY indexed */
            CMP(reg_y, 16, LOAD_IND16());
            break;

          case 0x10ae:          /* LDY indexed */
            LD(reg_y, 16, LOAD_IND16());
            break;

          case 0x10b0:          /* SUBW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_w, 0, LOAD_EXT16(), 16, 4);
            break;

          case 0x10b1:          /* CMPW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_w, 16, LOAD_EXT16());
            break;

          case 0x10b2:          /* SBCD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_d, LOCAL_CARRY(), LOAD_EXT16(), 16);
            break;

          case 0x10b3:          /* CMPD extended */
            CMP(reg_d, 16, LOAD_EXT16());
            break;

          case 0x10b4:          /* ANDD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            AND(reg_d, LOAD_EXT16(), 16, 4);
            break;

          case 0x10b5:          /* BITD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            BIT(reg_d, 16, LOAD_EXT16());
            break;

          case 0x10b6:          /* LDW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_EXT16());
            break;

          case 0x10b8:          /* EORD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            EOR(reg_d, 16, LOAD_EXT16());
            break;

          case 0x10b9:          /* ADCD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_d, LOAD_EXT16(), LOCAL_CARRY(), 16);
            break;

          case 0x10ba:          /* ORD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 16, LOAD_EXT16(), 4);
            break;

          case 0x10bb:          /* ADDW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_w, LOAD_EXT16(), 0, 16);
            break;

          case 0x10bc:          /* CMPY extended */
            CMP(reg_y, 16, LOAD_EXT16());
            break;

          case 0x10be:          /* LDY extended */
            LD(reg_y, 16, LOAD_EXT16());
            break;

          case 0x10ce:          /* LDS immediate */
            LD(reg_ssp, 16, (p1 << 8) | p2);
            break;

          case 0x10dc:          /* LDQ direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_DIRECT32(p1));
            break;

          case 0x10de:          /* LDS direct */
            LD(reg_ssp, 16, LOAD_DIRECT16(p1));
            break;

          case 0x10ec:          /* LDQ indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_IND32());
            break;

          case 0x10ee:          /* LDS indexed */
            LD(reg_ssp, 16, LOAD_IND16());
            break;

          case 0x10fc:          /* LDQ extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, LOAD_EXT32());
            break;

          case 0x10fe:          /* LDS extended */
            LD(reg_ssp, 16, LOAD_EXT16());
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
            BIT(reg_md, 8, p1);
            break;

          case 0x113d:          /* LDMD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_md, 8, p1);
            break;

          case 0x113f:          /* SWI3 */
            SWI(3)

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
            CMP(reg_e, 8, p1);
            break;

          case 0x1183:          /* CMPU immediate */
            CMP(reg_usp, 16, (p1 << 8) | p2);
            break;

          case 0x1186:          /* LDE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, p1);
            break;

          case 0x118b:          /* ADDE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, p1, 0, 8);
            break;

          case 0x118c:          /* CMPS immediate */
            CMP(reg_ssp, 16, (p1 << 8) | p2);
            break;

          case 0x1190:          /* SUBE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_DIRECT8(p1), 8, 3);
            break;

          case 0x1191:          /* CMPE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_DIRECT8(p1));
            break;

          case 0x1193:          /* CMPU direct */
            CMP(reg_usp, 16, LOAD_DIRECT16(p1));
            break;

          case 0x1196:          /* LDE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_DIRECT8(p1));
            break;

          case 0x1197:          /* STE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, (reg_dpr << 8) | p1);
            break;

          case 0x119b:          /* ADDE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, LOAD_DIRECT8(p1), 0, 8);
            break;

          case 0x119c:          /* CMPS direct */
            CMP(reg_ssp, 16, LOAD_DIRECT16(p1));
            break;

          case 0x11a0:          /* SUBE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_IND8(), 8, 3);
            break;

          case 0x11a1:          /* CMPE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_IND8());
            break;

          case 0x11a3:          /* CMPU indexed */
            CMP(reg_usp, 16, LOAD_IND16());
            break;

          case 0x11a6:          /* LDE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_IND8());
            break;

          case 0x11a7:          /* STE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, GET_IND_MA(p1, p2, p3));
            break;

          case 0x11ab:          /* ADDE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_e, LOAD_IND8(), 0, 8);
            break;

          case 0x11ac:          /* CMPS indexed */
            CMP(reg_ssp, 16, LOAD_IND16());
            break;

          case 0x11b0:          /* SUBE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_e, 0, LOAD_EXT8(), 8, 4);
            break;

          case 0x11b1:          /* CMPE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_e, 8, LOAD_EXT8());
            break;

          case 0x11b3:          /* CMPU extended */
            CMP(reg_usp, 16, LOAD_EXT16());
            break;

          case 0x11b6:          /* LDE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_EXT8());
            break;

          case 0x11b7:          /* STE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_e, 8, (p1 << 8) | p2);
            break;

          case 0x11bb:          /* ADDE extended */
            ADD(reg_e, LOAD_EXT8(), 0, 8);
            break;

          case 0x11bc:          /* CMPS extended */
            CMP(reg_ssp, 16, LOAD_EXT16());
            break;

          case 0x11c0:          /* SUBF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, p1, 8, 3);
            break;

          case 0x11c1:          /* CMPF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, p1);
            break;

          case 0x11c6:          /* LDF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, p1);
            break;

          case 0x11cb:          /* ADDF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, p1, 0, 8);
            break;

          case 0x11d0:          /* SUBF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_DIRECT8(p1), 8, 3);
            break;

          case 0x11d1:          /* CMPF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_DIRECT8(p1));
            break;

          case 0x11d6:          /* LDF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_DIRECT8(p1));
            break;

          case 0x11d7:          /* STF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, (reg_dpr << 8) | p1);
            break;

          case 0x11db:          /* ADDF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_DIRECT8(p1), 0, 8);
            break;

          case 0x11e0:          /* SUBF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_IND8(), 8, 3);
            break;

          case 0x11e1:          /* CMPF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_IND8());
            break;

          case 0x11e6:          /* LDF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_IND8());
            break;

          case 0x11e7:          /* STF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, GET_IND_MA(p1, p2, p3));
            break;

          case 0x11eb:          /* ADDF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_IND8(), 0, 8);
            break;

          case 0x11f0:          /* SUBF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            SUB(reg_f, 0, LOAD_EXT8(), 8, 4);
            break;

          case 0x11f1:          /* CMPF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            CMP(reg_f, 8, LOAD_EXT8());
            break;

          case 0x11f6:          /* LDF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_EXT8());
            break;

          case 0x11f7:          /* STF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ST(reg_f, 8, (p1 << 8) | p2);
            break;

          case 0x11fb:          /* ADDF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            ADD(reg_f, LOAD_EXT8(), 0, 8);
            break;
        }
    }
}

