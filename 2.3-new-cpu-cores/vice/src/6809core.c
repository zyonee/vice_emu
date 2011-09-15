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
static WORD GET_IND_MA(void)
{
    WORD ma = 0;

    if (!(p1 & 0x80)) {
        /* n,R (5bit offset) */
        PB_REG2VAR(p1, ma);
        ma += (p1 & 0x1f);
    } else {
        PC_INC(1);       /* postbyte fetch */
        PB_REG2VAR(p1, ma);
        switch (p1 & 0x1f) {
            case 0:      /* ,R+ */
                PB_REGADD(p1, 1);
                break;
            case 1:      /* ,R++ */
                PB_REGADD(p1, 2);
                break;
            case 2:      /* ,R- */
                PB_REGADD(p1, -1);
                break;
            case 3:      /* ,R-- */
                PB_REGADD(p1, -2);
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
                ma += p2;
                PC_INC(1);
                break;
            case 9:      /* n,R (16bit offset) */
                ma += ((p2 << 8) | p3);
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
                ma = reg_pc + (signed char)p2;
                break;
            case 13:     /* n,PC (16bit offset) */   /* FIXME: check of pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((p2 << 8) | p3);
                break;
            case 14:     /* W,R */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed short)((reg_e << 8) | reg_f);
                break;
            case 15:   /* FIXME: fix for 6809, 6309 only modes */
                switch ((p1 & 0x60) >> 5) {
                    case 0:     /* ,W */
                        ma = reg_w;
                        break;
                    case 1:     /* n,W (16bit offset) */
                        ma = reg_w + (signed short)((p2 << 8) | p3);
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
                        ma = LOAD(reg_w + (signed short)((p2 << 8) | p3)) << 8;
                        ma |= LOAD(reg_w + 1 + (signed short)((p2 << 8) | p3));
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
                PB_REGADD(p1, 2);
                break;
            case 18:     /* FIXME: unknown */
                break;
            case 19:     /* [,R--] */
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PB_REGADD(p1, -2);
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
                ma += p2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                PC_INC(1);
                break;
            case 25:     /* [n,R] (16bit offset) */
                ma += ((p2 << 8) | p3);
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
                ma = reg_pc + (signed char)p2;
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 29:     /* [n,PC] (16bit offset) */   /* FIXME: check if pc offset is correct */
                PC_INC(2);
                ma = reg_pc + (signed short)((p2 << 8) | p3);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 30:     /* [W,R] */   /* FIXME: fix for 6809, 6309 only mode */
                ma += (signed short)((reg_e << 8) | reg_f);
                ma = (LOAD(ma) << 8) | LOAD(ma + 1);
                break;
            case 31:     /* [n] (16bit extended indirect) */
                ma = LOAD((p2 << 8) | p3) << 8;
                ma |= LOAD(((p2 << 8) | p3) + 1);
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

#define LOAD_EXT8(addr) \
   (LOAD(addr))

#define LOAD_EXT16(addr) \
   ((LOAD_EXT8(addr) << 8) | (LOAD_EXT8(addr + 1)))

#define LOAD_EXT32(addr) \
   ((LOAD_EXT16(addr) << 16) | (LOAD_EXT16(addr + 2)))

static BYTE LOAD_IND8(void)
{
    return LOAD(GET_IND_MA());
}

static WORD LOAD_IND16(void)
{
    WORD ma = GET_IND_MA();
    WORD retval;

    retval = LOAD(ma) << 8;
    retval |= LOAD(ma + 1);
    return retval;
}

static DWORD LOAD_IND32(void)
{
    WORD ma = GET_IND_MA();
    WORD retval;

    retval = LOAD(ma) << 24;
    retval |= (LOAD(ma + 1) << 16);
    retval |= (LOAD(ma + 2) << 8);
    retval |= LOAD(ma + 3);
    return retval;
}

/* ------------------------------------------------------------------------- */

#define BT(val, bit) (val & (1 < bit)) ? 1 : 0

/* ------------------------------------------------------------------------- */

/* Opcodes.  */

/* The 0x02 JAM opcode is also used to patch the ROM.  The function trap_handler()
   returns nonzero if this is not a patch, but a `real' JAM instruction. */

#define JAM_02()                                                \
  do {                                                          \
      DWORD trap_result;                                        \
      EXPORT_REGISTERS();                                       \
      if (!ROM_TRAP_ALLOWED()                                   \
          || (trap_result = ROM_TRAP_HANDLER()) == (DWORD)-1) { \
          REWIND_FETCH_OPCODE(CLK);                             \
          JAM();                                                \
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
  
#define JMP(addr) \
  do {            \
      JUMP(addr); \
  } while (0)

#define LD(RR, bits, m)                     \
  do {                                      \
      RR = m;                               \
      LOCAL_SET_OVERFLOW(0);                \
      LOCAL_SET_ZERO(!RR);                  \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1)); \
  } while (0)

#define LSR_REG(RR)               \
  do {                            \
      LOCAL_SET_CARRY(BT(RR, 0)); \
      RR >>= 1;                   \
      LOCAL_SET_NEGATIVE(0);      \
      LOCAL_SET_ZERO(!RR);        \
  } while (0)

#define LSR(ma)       \
  do {                \
      BYTE val;       \
                      \
      val = LOAD(ma); \
      LSR_REG(val);   \
      STORE(ma, val); \
  } while (0)

#define MUL()                        \
  do {                               \
      reg_d = reg_a * reg_b;         \
      LOCAL_SET_CARRY(BT(reg_b, 7)); \
      LOCAL_SET_ZERO(!reg_d);        \
  } while (0)

#define NEG_REG(RR, bits)                               \
  do {                                                  \
      unsigned int tmp;                                 \
                                                        \
      LOCAL_SET_OVERFLOW(RR == (1 << (bits -1)));       \
      tmp = ((~RR) & (bits == 8) ? 0xff : 0xffff) + 1;  \        
      RR = tmp & (bits == 8) ? 0xff " 0xffff;           \
      LOCAL_SET_NEGATIVE(BT(RR, bits - 1));             \
      LOCAL_SET_ZERO(!RR);                              \
      LOCAL_SET_CARRY(RR);                              \
  } while (0)

#define NEG(ma)        \
  do {                 \
      BYTE val;        \
                       \
      val = LOAD(ma);  \
      NEG_REG(val, 8); \
      STORE(ma, val);  \
  } while (0)

#define OR(RR, bits8, m)                  \
  do {                                    \
      RR |= m;                            \
      LOCAL_SET_OVERFLOW(0);              \
      LOCAL_SET_ZERO(!RR);                \
      if (bits8) {                        \
          LOCAL_SET_NEGATIVE(BT(RR, 7));  \
      } else {                            \
          LOCAL_SET_NEGATIVE(BT(RR, 15)); \
      }                                   \
  } while (0)

#define NOOP(clk_inc, pc_inc) \
    (CLK_ADD(CLK, (clk_inc)), INC_PC(pc_inc))

#define NOOP_IMM(pc_inc) INC_PC(pc_inc)

#define ROL_REG(RR, bits)                                      \
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
  } while (0)

#define ROL(ma)        \
  do {                 \
      BYTE val;        \
                       \
      val = LOAD(ma);  \
      ROL_REG(val, 8); \
      STORE(ma, val);  \
  } while (0)

/* ------------------------------------------------------------------------- */

#define FETCH_OPCODE(o, page)                                                                                      \
    do {                                                                                                           \
        o = LOAD(reg_pc);                                                                                          \
        page = 0;                                                                                                  \
        if (o == 0x10) {                                                                                           \
            page = 1;                                                                                              \
        }                                                                                                          \
        if (o == 0x11) {                                                                                           \
            page = 2;                                                                                              \
        }                                                                                                          \
        if (page) {                                                                                                \
            o = LOAD(reg_pc + 1) | (LOAD(reg_pc + 2) << 8)  | (LOAD(reg_pc + 3) << 16) | (LOAD(reg_pc + 4) << 24); \
        } else {                                                                                                   \
            o = o | (LOAD(reg_pc + 1) << 8)  | (LOAD(reg_pc + 2) << 16) | (LOAD(reg_pc + 3) << 24);                \
        }                                                                                                          \
  } while (0)

#define p0 (opcode & 0xff)
#define p1 ((opcode >> 8) & 0xff)
#define p2 ((opcode >> 16) & 0xff)
#define p3 ((opcode >> 24) & 0xff)

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

          case 0x0000:            /* NEG direct */
            NEG(p1 | (reg_dpr << 8));
            break;

          case 0x01:            /* ORA ($nn,X) */
            ORA(LOAD_IND_X(p1), 1, 2);
            break;

          case 0x02:            /* JAM - also used for traps */
            STATIC_ASSERT(TRAP_OPCODE == 0x02);
            JAM_02();
            break;

          case 0x22:            /* JAM */
          case 0x52:            /* JAM */
          case 0x62:            /* JAM */
          case 0x72:            /* JAM */
          case 0x92:            /* JAM */
          case 0xb2:            /* JAM */
          case 0xd2:            /* JAM */
          case 0xf2:            /* JAM */
            REWIND_FETCH_OPCODE(CLK);
            JAM();
            break;

          case 0x03:            /* SLO ($nn,X) */
            SLO(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0x0004:          /* LSR direct */
            LSR((reg_dpr << 8) | p1);
            break;

          case 0x05:            /* ORA $nn */
            ORA(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x06:            /* ASL $nn */
            ASL(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x07:            /* SLO $nn */
            SLO(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x08:            /* PHP */
#ifdef DRIVE_CPU
            if (drivecpu_byte_ready())
                LOCAL_SET_OVERFLOW(1);
#endif
            PHP();
            break;

          case 0x0009:          /* ROL direct */
            ROL((reg_dpr << 8) | p1);
            break;

          case 0x0a:            /* ASL A */
            ASL_A();
            break;

          case 0x0b:            /* ANC #$nn */
            ANC(p1, 2);
            break;

          case 0x0c:            /* NOOP $nnnn */
            NOOP_ABS();
            break;

          case 0x0d:            /* ORA $nnnn */
            ORA(LOAD(p2), 1, 3);
            break;

          case 0x0e:            /* ASL $nnnn */
            ASL(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x0f:            /* SLO $nnnn */
            SLO(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x10:            /* BPL $nnnn */
            BRANCH(!LOCAL_SIGN(), p1);
            break;

          case 0x11:            /* ORA ($nn),Y */
            ORA(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0x13:            /* SLO ($nn),Y */
            SLO_IND_Y(p1);
            break;

          case 0x14:            /* NOOP $nn,X */
          case 0x34:            /* NOOP $nn,X */
          case 0xd4:            /* NOOP $nn,X */
          case 0xf4:            /* NOOP $nn,X */
            NOOP(CLK_NOOP_ZERO_X, 2);
            break;

          case 0x15:            /* ORA $nn,X */
            ORA(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x16:            /* ASL $nn,X */
            ASL((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x17:            /* SLO $nn,X */
            SLO((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x18:            /* CLC */
            CLC();
            break;

          case 0x19:            /* ORA $nnnn,Y */
            ORA(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x001a:            /* ORCC immediate */
            OR(reg_p, 1, p1);
            break;

          case 0x3a:            /* NOOP */
          case 0x5a:            /* NOOP */
          case 0x7a:            /* NOOP */
            NOOP_IMM(1);
            break;

          case 0x1b:            /* SLO $nnnn,Y */
            SLO(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
            break;

          case 0x1c:            /* NOOP $nnnn,X */
          case 0x3c:            /* NOOP $nnnn,X */
          case 0x5c:            /* NOOP $nnnn,X */
          case 0x7c:            /* NOOP $nnnn,X */
            NOOP_ABS_X();
            break;

          case 0x1d:            /* ORA $nnnn,X */
            ORA(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x1e:            /* ASL $nnnn,X */
            ASL(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x1f:            /* SLO $nnnn,X */
            SLO(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x20:            /* JSR $nnnn */
            JSR();
            break;

          case 0x21:            /* AND ($nn,X) */
            AND(LOAD_IND_X(p1), 1, 2);
            break;

          case 0x23:            /* RLA ($nn,X) */
            RLA(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0x24:            /* BIT $nn */
            BIT(LOAD_ZERO(p1), 2);
            break;

          case 0x25:            /* AND $nn */
            AND(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x26:            /* ROL $nn */
            ROL(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x27:            /* RLA $nn */
            RLA(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x28:            /* PLP */
            PLP();
            break;

          case 0x29:            /* AND #$nn */
            AND(p1, 0, 2);
            break;

          case 0x2a:            /* ROL A */
            ROL_A();
            break;

          case 0x2b:            /* ANC #$nn */
            ANC(p1, 2);
            break;

          case 0x2c:            /* BIT $nnnn */
            BIT(LOAD(p2), 3);
            break;

          case 0x2d:            /* AND $nnnn */
            AND(LOAD(p2), 1, 3);
            break;

          case 0x2e:            /* ROL $nnnn */
            ROL(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x2f:            /* RLA $nnnn */
            RLA(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
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

          case 0x35:            /* AND $nn,X */
            AND(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x36:            /* ROL $nn,X */
            ROL((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x37:            /* RLA $nn,X */
            RLA((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x38:            /* SEC */
            SEC();
            break;

          case 0x39:            /* AND $nnnn,Y */
            AND(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x3b:            /* RLA $nnnn,Y */
            RLA(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
            break;

          case 0x003d:            /* MUL */
            MUL();
            break;

          case 0x3e:            /* ROL $nnnn,X */
            ROL(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x3f:            /* RLA $nnnn,X */
            RLA(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x0040:            /* NEGA */
            NEG_REG(reg_a, 8);
            break;

          case 0x41:            /* EOR ($nn,X) */
            EOR(LOAD_IND_X(p1), 1, 2);
            break;

          case 0x43:            /* SRE ($nn,X) */
            SRE(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0x0044:          /* LSRA */
            LSR_REG(reg_a);
            break;

          case 0x45:            /* EOR $nn */
            EOR(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x46:            /* LSR $nn */
            LSR(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x47:            /* SRE $nn */
            SRE(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x48:            /* PHA */
            PHA();
            break;

          case 0x0049:          /* ROLA */
            ROL_REG(reg_a, 8);
            break;

          case 0x4a:            /* LSR A */
            LSR_A();
            break;

          case 0x4b:            /* ASR #$nn */
            ASR(p1, 2);
            break;

          case 0x4c:            /* JMP $nnnn */
            JMP(p2);
            break;

          case 0x4d:            /* EOR $nnnn */
            EOR(LOAD(p2), 1, 3);
            break;

          case 0x4e:            /* LSR $nnnn */
            LSR(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x4f:            /* SRE $nnnn */
            SRE(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x0050:            /* NEGB */
            NEG_REG(reg_b, 8);
            break;

          case 0x51:            /* EOR ($nn),Y */
            EOR(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0x53:            /* SRE ($nn),Y */
            SRE_IND_Y(p1);
            break;

          case 0x0054:          /* LSRB */
            LSR_REG(reg_b);
            break;

          case 0x55:            /* EOR $nn,X */
            EOR(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x56:            /* LSR $nn,X */
            LSR((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x57:            /* SRE $nn,X */
            SRE((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x58:            /* CLI */
            CLI();
            break;

          case 0x0059:          /* ROLB */
            ROL_REG(reg_b, 8);
            break;

          case 0x5b:            /* SRE $nnnn,Y */
            SRE(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
            break;

          case 0x5d:            /* EOR $nnnn,X */
            EOR(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x5e:            /* LSR $nnnn,X */
            LSR(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x5f:            /* SRE $nnnn,X */
            SRE(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x0060:            /* NEG indexed */
            NEG(GET_IND_MA());
            break;

          case 0x61:            /* ADC ($nn,X) */
            ADC(LOAD_IND_X(p1), 1, 2);
            break;

          case 0x63:            /* RRA ($nn,X) */
            RRA(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0x0064:          /* LSR indexed */
            LSR(GET_IND_MA());
            break;

          case 0x65:            /* ADC $nn */
            ADC(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x66:            /* ROR $nn */
            ROR(p1, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x67:            /* RRA $nn */
            RRA(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x68:            /* PLA */
            PLA();
            break;

          case 0x0069:            /* ROL indexed */
            ROL(GET_IND_MA());
            break;

          case 0x6a:            /* ROR A */
            ROR_A();
            break;

          case 0x6b:            /* ARR #$nn */
            ARR(p1, 2);
            break;

          case 0x6c:            /* JMP ($nnnn) */
            JMP_IND();
            break;

          case 0x6d:            /* ADC $nnnn */
            ADC(LOAD(p2), 1, 3);
            break;

          case 0x6e:            /* ROR $nnnn */
            ROR(p2, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x6f:            /* RRA $nnnn */
            RRA(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0x0070:            /* NEG extended */
            NEG((p1 << 8) | p2);
            break;

          case 0x71:            /* ADC ($nn),Y */
            ADC(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0x73:            /* RRA ($nn),Y */
            RRA_IND_Y(p1);
            break;

          case 0x0074:            /* LSR extended */
            LSR((p1 << 8) | p2);
            break;

          case 0x75:            /* ADC $nn,X */
            ADC(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x76:            /* ROR $nn,X */
            ROR((p1 + reg_x) & 0xff, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x77:            /* RRA $nn,X */
            RRA((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0x78:            /* SEI */
            SEI();
            break;

          case 0x0079:          /* ROL extended */
            ROL((p1 << 8) | p2);
            break;

          case 0x7b:            /* RRA $nnnn,Y */
            RRA(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
            break;

          case 0x7d:            /* ADC $nnnn,X */
            ADC(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x7e:            /* ROR $nnnn,X */
            ROR(p2, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x7f:            /* RRA $nnnn,X */
            RRA(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x80:            /* NOOP #$nn */
          case 0x82:            /* NOOP #$nn */
          case 0x89:            /* NOOP #$nn */
          case 0xc2:            /* NOOP #$nn */
          case 0xe2:            /* NOOP #$nn */
            NOOP_IMM(2);
            break;

          case 0x81:            /* STA ($nn,X) */
            STA(LOAD_ZERO_ADDR(p1 + reg_x), 3, 1, 2, STORE_ABS);
            break;

          case 0x83:            /* SAX ($nn,X) */
            SAX(LOAD_ZERO_ADDR(p1 + reg_x), 3, 1, 2);
            break;

          case 0x84:            /* STY $nn */
            STY_ZERO(p1, 1, 2);
            break;

          case 0x85:            /* STA $nn */
            STA_ZERO(p1, 1, 2);
            break;

          case 0x0086:            /* LDA immediate */
            LD(reg_a, 8, p1);
            break;

          case 0x87:            /* SAX $nn */
            SAX_ZERO(p1, 1, 2);
            break;

          case 0x88:            /* DEY */
            DEY();
            break;

          case 0x008a:            /* ORA immediate */
            OR(reg_a, 1, p1);
            break;

          case 0x8b:            /* ANE #$nn */
            ANE(p1, 2);
            break;

          case 0x8c:            /* STY $nnnn */
            STY(p2, 1, 3);
            break;

          case 0x8d:            /* STA $nnnn */
            STA(p2, 0, 1, 3, STORE_ABS);
            break;

          case 0x008e:            /* LDX immediate */
            LD(reg_x, 16, (p1 << 8) | p2);
            break;

          case 0x8f:            /* SAX $nnnn */
            SAX(p2, 0, 1, 3);
            break;

          case 0x90:            /* BCC $nnnn */
            BRANCH(!LOCAL_CARRY(), p1);
            break;

          case 0x91:            /* STA ($nn),Y */
            STA_IND_Y(p1);
            break;

          case 0x93:            /* SHA ($nn),Y */
            SHA_IND_Y(p1);
            break;

          case 0x94:            /* STY $nn,X */
            STY_ZERO(p1 + reg_x, CLK_ZERO_I_STORE, 2);
            break;

          case 0x95:            /* STA $nn,X */
            STA_ZERO(p1 + reg_x, CLK_ZERO_I_STORE, 2);
            break;

          case 0x0096:            /* LDA direct */
            LD(reg_a, 8, LOAD_DIRECT8(p1));
            break;

          case 0x97:            /* SAX $nn,Y */
            SAX((p1 + reg_y) & 0xff, 0, CLK_ZERO_I_STORE, 2);
            break;

          case 0x98:            /* TYA */
            TYA();
            break;

          case 0x99:            /* STA $nnnn,Y */
            STA(p2, 0, CLK_ABS_I_STORE2, 3, STORE_ABS_Y);
            break;

          case 0x009a:            /* ORA direct */
            OR(reg_a, 1, LOAD_DIRECT8(p1));
            break;

          case 0x9b:            /* SHS $nnnn,Y */
#ifdef C64DTV
            NOOP_ABS_Y();
#else
            SHS_ABS_Y(p2);
#endif
            break;

          case 0x9c:            /* SHY $nnnn,X */
            SHY_ABS_X(p2);
            break;

          case 0x9d:            /* STA $nnnn,X */
            STA(p2, 0, CLK_ABS_I_STORE2, 3, STORE_ABS_X);
            break;

          case 0x009e:            /* LDX direct */
            LD(reg_x, 16, LOAD_DIRECT16(p1));
            break;

          case 0x9f:            /* SHA $nnnn,Y */
            SHA_ABS_Y(p2);
            break;

          case 0xa0:            /* LDY #$nn */
            LDY(p1, 0, 2);
            break;

          case 0xa1:            /* LDA ($nn,X) */
            LDA(LOAD_IND_X(p1), 1, 2);
            break;

          case 0xa2:            /* LDX #$nn */
            LDX(p1, 0, 2);
            break;

          case 0xa3:            /* LAX ($nn,X) */
            LAX(LOAD_IND_X(p1), 1, 2);
            break;

          case 0xa4:            /* LDY $nn */
            LDY(LOAD_ZERO(p1), 1, 2);
            break;

          case 0xa5:            /* LDA $nn */
            LDA(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x00a6:            /* LDA indexed */
            LD(reg_a, 8, LOAD_IND8());
            break;

          case 0xa7:            /* LAX $nn */
            LAX(LOAD_ZERO(p1), 1, 2);
            break;

          case 0xa8:            /* TAY */
            TAY();
            break;

          case 0xa9:            /* LDA #$nn */
            LDA(p1, 0, 2);
            break;

          case 0x00aa:            /* ORA indexed */
            OR(reg_a, 1, LOAD_IND8());
            break;

          case 0xab:            /* LXA #$nn */
            LXA(p1, 2);
            break;

          case 0xac:            /* LDY $nnnn */
            LDY(LOAD(p2), 1, 3);
            break;

          case 0xad:            /* LDA $nnnn */
            LDA(LOAD(p2), 1, 3);
            break;

          case 0x00ae:            /* LDX indexed */
            LD(reg_x, 16, LOAD_IND16());
            break;

          case 0xaf:            /* LAX $nnnn */
            LAX(LOAD(p2), 1, 3);
            break;

          case 0xb0:            /* BCS $nnnn */
            BRANCH(LOCAL_CARRY(), p1);
            break;

          case 0xb1:            /* LDA ($nn),Y */
            LDA(LOAD_IND_Y_BANK(p1), 1, 2);
            break;

          case 0xb3:            /* LAX ($nn),Y */
            LAX(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0xb4:            /* LDY $nn,X */
            LDY(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0xb5:            /* LDA $nn,X */
            LDA(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x00b6:            /* LDA extended */
            LD(reg_a, 8, LOAD_EXT8(((p1 << 8) | p2));
            break;

          case 0xb7:            /* LAX $nn,Y */
            LAX(LOAD_ZERO_Y(p1), CLK_ZERO_I2, 2);
            break;

          case 0xb8:            /* CLV */
            CLV();
            break;

          case 0xb9:            /* LDA $nnnn,Y */
            LDA(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x00ba:            /* ORA extended */
            OR(reg_a, 1, LOAD_EXT8((p1 << 8) | p2));
            break;

          case 0xbb:            /* LAS $nnnn,Y */
            LAS(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0xbc:            /* LDY $nnnn,X */
            LDY(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0xbd:            /* LDA $nnnn,X */
            LDA(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x00be:            /* LDX extended */
            LD(reg_x, 16, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0xbf:            /* LAX $nnnn,Y */
            LAX(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0xc0:            /* CPY #$nn */
            CPY(p1, 0, 2);
            break;

          case 0xc1:            /* CMP ($nn,X) */
            CMP(LOAD_IND_X(p1), 1, 2);
            break;

          case 0xc3:            /* DCP ($nn,X) */
            DCP(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0xc4:            /* CPY $nn */
            CPY(LOAD_ZERO(p1), 1, 2);
            break;

          case 0xc5:            /* CMP $nn */
            CMP(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x00c6:            /* LDB immediate */
            LD(reg_b, 8, p1);
            break;

          case 0xc7:            /* DCP $nn */
            DCP(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0xc8:            /* INY */
            INY();
            break;

          case 0xc9:            /* CMP #$nn */
            CMP(p1, 0, 2);
            break;

          case 0x00ca:            /* ORB immediate */
            OR(reg_b, 1, p1);
            break;

          case 0xcb:            /* SBX #$nn */
            SBX(p1, 2);
            break;

          case 0x00cc:            /* LDD immediate */
            LD(reg_d, 16, (p1 << 8) | p2);
            break;

          case 0x00cd:            /* LDQ immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_q, 32, (p1 << 24) | (p2 << 16) | (p3 << 8) | LOAD(reg_pc + 4));
            break;

          case 0x00ce:            /* LDU immediate */
            LD(reg_usp, 16, (p1 << 8) | p2);
            break;

          case 0xcf:            /* DCP $nnnn */
            DCP(p2, 0, CLK_ABS_RMW2, 3, LOAD_ABS, STORE_ABS);
            break;

          case 0xd0:            /* BNE $nnnn */
            BRANCH(!LOCAL_ZERO(), p1);
            break;

          case 0xd1:            /* CMP ($nn),Y */
            CMP(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0xd3:            /* DCP ($nn),Y */
            DCP_IND_Y(p1);
            break;

          case 0xd5:            /* CMP $nn,X */
            CMP(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x00d6:            /* LDB direct */
            LD(reg_b, 8, LOAD_DIRECT8(p1));
            break;

          case 0xd7:            /* DCP $nn,X */
            DCP((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0xd8:            /* CLD */
            CLD();
            break;

          case 0xd9:            /* CMP $nnnn,Y */
            CMP(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x00da:          /* ORB direct */
            OR(reg_b, 1, LOAD_DIRECT8(p1));
            break;

          case 0xdb:            /* DCP $nnnn,Y */
            DCP(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
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

          case 0xe0:            /* CPX #$nn */
            CPX(p1, 0, 2);
            break;

          case 0xe1:            /* SBC ($nn,X) */
            SBC(LOAD_IND_X(p1), 1, 2);
            break;

          case 0xe3:            /* ISB ($nn,X) */
            ISB(LOAD_ZERO_ADDR(p1 + reg_x), 3, CLK_IND_X_RMW, 2, LOAD_ABS, STORE_ABS);
            break;

          case 0xe4:            /* CPX $nn */
            CPX(LOAD_ZERO(p1), 1, 2);
            break;

          case 0xe5:            /* SBC $nn */
            SBC(LOAD_ZERO(p1), 1, 2);
            break;

          case 0x00e6:            /* LDB indexed */
            LD(reg_b, 8, LOAD_IND8());
            break;

          case 0xe7:            /* ISB $nn */
            ISB(p1, 0, CLK_ZERO_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0xe8:            /* INX */
            INX();
            break;

          case 0xe9:            /* SBC #$nn */
            SBC(p1, 0, 2);
            break;

          case 0x00ea:            /* ORB indexed */
            OR(reg_b, 1, LOAD_IND8());
            break;

          case 0xeb:            /* USBC #$nn (same as SBC) */
            SBC(p1, 0, 2);
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

          case 0xf0:            /* BEQ $nnnn */
            BRANCH(LOCAL_ZERO(), p1);
            break;

          case 0xf1:            /* SBC ($nn),Y */
            SBC(LOAD_IND_Y(p1), 1, 2);
            break;

          case 0xf3:            /* ISB ($nn),Y */
            ISB_IND_Y(p1);
            break;

          case 0xf5:            /* SBC $nn,X */
            SBC(LOAD_ZERO_X(p1), CLK_ZERO_I2, 2);
            break;

          case 0x00f6:            /* LDB extended */
            LD(reg_b, 8, LOAD_EXT8((p1 << 8) | p2));
            break;

          case 0xf7:            /* ISB $nn,X */
            ISB((p1 + reg_x) & 0xff, 0, CLK_ZERO_I_RMW, 2, LOAD_ZERO, STORE_ABS);
            break;

          case 0xf8:            /* SED */
            SED();
            break;

          case 0xf9:            /* SBC $nnnn,Y */
            SBC(LOAD_ABS_Y(p2), 1, 3);
            break;

          case 0x00fa:          /* ORB extended */
            OR(reg_b, 1, LOAD_EXT8((p1 << 8) | p2));
            break;

          case 0xfb:            /* ISB $nnnn,Y */
            ISB(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_Y_RMW, STORE_ABS_Y_RMW);
            break;

          case 0x00fc:          /* LDD extended */
            LD(reg_d, 16, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0xfd:            /* SBC $nnnn,X */
            SBC(LOAD_ABS_X(p2), 1, 3);
            break;

          case 0x00fe:          /* LDU extended */
            LD(reg_usp, 16, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0xff:            /* ISB $nnnn,X */
            ISB(p2, 0, CLK_ABS_I_RMW2, 3, LOAD_ABS_X_RMW, STORE_ABS_X_RMW);
            break;

          case 0x1040:          /* NEGD */   /* FIXME: fix for 6809, 6309 only opcode */
            NEG_REG(reg_d, 16);
            break;

          case 0x1044:          /* LSRD */   /* FIXME: fix for 6809, 6309 only opcode */
            LSR_REG(reg_d);
            break;

          case 0x1049:          /* ROLD */   /* FIXME: fix for 6809, 6309 only opcode */
            ROL_REG(reg_d, 16);
            break;

          case 0x1054:          /* LSRW */   /* FIXME: fix for 6809, 6309 only opcode */
            LSR_REG(reg_w);
            break;

          case 0x1059:          /* ROLW */   /* FIXME: fix for 6809, 6309 only opcode */
            ROL_REG(reg_w, 16);
            break;

          case 0x1086:          /* LDW immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, (p1 << 8) | p2);
            break;

          case 0x108a:          /* ORD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 0, (p1 << 8) | p2);
            break;

          case 0x108e:          /* LDY immediate */
            LD(reg_y, 16, (p1 << 8) | p2);
            break;

          case 0x1096:          /* LDW direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_DIRECT16(p1));
            break;

          case 0x109a:          /* ORD direct */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 0, LOAD_DIRECT16(p1));
            break;

          case 0x109e:          /* LDY direct */
            LD(reg_y, 16, LOAD_DIRECT16(p1));
            break;

          case 0x10a6:          /* LDW indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_IND16());
            break;

          case 0x10aa:          /* ORD indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 0, LOAD_IND16());
            break;

          case 0x10ae:          /* LDY indexed */
            LD(reg_y, 16, LOAD_IND16());
            break;

          case 0x10b6:          /* LDW extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_w, 16, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0x10ba:          /* ORD extended */   /* FIXME: fix for 6809, 6309 only opcode */
            OR(reg_d, 0, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0x10be:          /* LDY extended */
            LD(reg_y, 16, LOAD_EXT16((p1 << 8) | p2));
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
            LD(reg_q, 32, LOAD_EXT32((p1 << 8) | p2));
            break;

          case 0x10fe:          /* LDS extended */
            LD(reg_ssp, 16, LOAD_EXT16((p1 << 8) | p2));
            break;

          case 0x113d:          /* LDMD immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_md, 8, p1);
            break;

          case 0x1186:          /* LDE immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, p1);
            break;

          case 0x1196:          /* LDE direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_DIRECT8(p1));
            break;

          case 0x11a6:          /* LDE indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_IND8());
            break;

          case 0x11b6:          /* LDE extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_e, 8, LOAD_EXT8((p1 << 8) | p2));
            break;

          case 0x11c6:          /* LDF immediate */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, p1);
            break;

          case 0x11d6:          /* LDF direct */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_DIRECT8(p1));
            break;

          case 0x11e6:          /* LDF indexed */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_IND8());
            break;

          case 0x11f6:          /* LDF extended */   /* FIXME: fix for 6809, 6309 only opcode */
            LD(reg_f, 8, LOAD_EXT8((p1 << 8) | p2));
            break;
        }
    }
}

