/*
 * Copyright 2001 by Arto Salmi and Joze Fabcic
 * Copyright 2006, 2007 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of GCC6809.
 * This file is part of VICE.
 *
 * VICE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * VICE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with VICE; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "vice.h"

#include <stdarg.h>

#include "6809.h"
#include "alarm.h"
#include "h6809regs.h"
#include "interrupt.h"
#include "monitor.h"
#include "petmem.h"

#define CLK maincpu_clk
#define CPU_INT_STATUS maincpu_int_status
#define ALARM_CONTEXT maincpu_alarm_context

/* ------------------------------------------------------------------------- */
/* Hook for additional delay.  */

#ifndef CPU_DELAY_CLK
#define CPU_DELAY_CLK
#endif

#ifndef CPU_REFRESH_CLK
#define CPU_REFRESH_CLK
#endif

#ifndef DMA_ON_RESET
#define DMA_ON_RESET
#endif

#ifndef DMA_FUNC
#define DMA_FUNC
#endif

#define CALLER  e_comp_space
#define LAST_OPCODE_ADDR iPC
#define GLOBAL_REGS     h6809_regs

h6809_regs_t h6809_regs;

/* Export the local version of the registers.  */
#define EXPORT_REGISTERS()            \
  do {                                \
      GLOBAL_REGS.reg_x  = get_x();   \
      GLOBAL_REGS.reg_y  = get_y();   \
      GLOBAL_REGS.reg_u  = get_u();   \
      GLOBAL_REGS.reg_s  = get_s();   \
      GLOBAL_REGS.reg_pc = get_pc();  \
      GLOBAL_REGS.reg_dp = get_dp();  \
      GLOBAL_REGS.reg_cc = get_cc();  \
      GLOBAL_REGS.reg_a  = get_a();   \
      GLOBAL_REGS.reg_b  = get_b();   \
  } while (0)

/* Import the public version of the registers.  */
#define IMPORT_REGISTERS()         \
  do {                             \
      set_x( GLOBAL_REGS.reg_x );  \
      set_y( GLOBAL_REGS.reg_y );  \
      set_u( GLOBAL_REGS.reg_u );  \
      set_s( GLOBAL_REGS.reg_s );  \
      set_pc(GLOBAL_REGS.reg_pc);  \
      set_dp(GLOBAL_REGS.reg_dp);  \
      set_cc(GLOBAL_REGS.reg_cc);  \
      set_a (GLOBAL_REGS.reg_a );  \
      set_b (GLOBAL_REGS.reg_b );  \
  } while (0)
#define JUMP(pc)

#define DO_INTERRUPT(int_kind)  do {                                  \
        BYTE ik = (int_kind);                                         \
        if (ik & (IK_TRAP | IK_RESET)) {                              \
            if (ik & IK_TRAP) {                                       \
                EXPORT_REGISTERS();                                   \
                interrupt_do_trap(CPU_INT_STATUS, (WORD)PC);          \
                IMPORT_REGISTERS();                                   \
                if (CPU_INT_STATUS->global_pending_int & IK_RESET)    \
                    ik |= IK_RESET;                                   \
            }                                                         \
            if (ik & IK_RESET) {                                      \
                interrupt_ack_reset(CPU_INT_STATUS);                  \
                cpu6809_reset();                                      \
                DMA_ON_RESET;                                         \
            }                                                         \
        }                                                             \
        if (ik & (IK_MONITOR | IK_DMA)) {                             \
            if (ik & IK_MONITOR) {                                    \
                if (monitor_force_import(CALLER))                     \
                    IMPORT_REGISTERS();                               \
                if (monitor_mask[CALLER])                             \
                    EXPORT_REGISTERS();                               \
                if (monitor_mask[CALLER] & (MI_BREAK)) {              \
                    if (monitor_check_breakpoints(CALLER,             \
                            (WORD)PC)) {                              \
                        monitor_startup(CALLER);                      \
                        IMPORT_REGISTERS();                           \
                    }                                                 \
                }                                                     \
                if (monitor_mask[CALLER] & (MI_STEP)) {               \
                    monitor_check_icount((WORD)PC);                   \
                    IMPORT_REGISTERS();                               \
                }                                                     \
                if (monitor_mask[CALLER] & (MI_WATCH)) {              \
                    monitor_check_watchpoints(LAST_OPCODE_ADDR, (WORD)PC); \
                    IMPORT_REGISTERS();                               \
                }                                                     \
            }                                                         \
            if (ik & IK_DMA) {                                        \
                EXPORT_REGISTERS();                                   \
                DMA_FUNC;                                             \
                interrupt_ack_dma(CPU_INT_STATUS);                    \
                IMPORT_REGISTERS();                                   \
                JUMP(PC);                                             \
            }                                                         \
        }                                                             \
        if (ik & IK_NMI) {                                            \
            request_nmi(0);                                           \
        } else if (ik & IK_IRQ) {                                     \
            req_irq(0);                                               \
        }                                                             \
    } while (0)

#ifdef LAST_OPCODE_ADDR
#define SET_LAST_ADDR(x) LAST_OPCODE_ADDR = (x)
#else
#error "please define LAST_OPCODE_ADDR"
#endif

union regs {
    DWORD reg_l;
    WORD reg_s[2];
    BYTE reg_c[4];
} regs6309;

#define Q regs6309.reg_l
#ifndef WORDS_BIGENDIAN
#define W regs6309.reg_s[0]
#define D regs6309.reg_s[1]
#define F regs6309.reg_c[0]
#define E regs6309.reg_c[1]
#define B regs6309.reg_c[2]
#define A regs6309.reg_c[3]
#else
#define W regs6309.reg_s[1]
#define D regs6309.reg_s[0]
#define F regs6309.reg_c[3]
#define E regs6309.reg_c[2]
#define B regs6309.reg_c[1]
#define A regs6309.reg_c[0]
#endif

static WORD X, Y, S, U, DP, PC, iPC;
static BYTE EFI;
static DWORD H, N, Z, OV, C;

#ifdef H6309
static BYTE MD;
static WORD V;

#define MD_NATIVE        0x01	/* if 1, execute in 6309 mode */
#define MD_FIRQ_LIKE_IRQ 0x02	/* if 1, FIRQ acts like IRQ */
#define MD_ILL 0x40		/* illegal instruction */
#define MD_DBZ 0x80		/* divide by zero */

#define H6309_NATIVE_MODE() (MD & 1)
#endif /* H6309 */


static WORD ea = 0;
static unsigned int irqs_pending = 0;
static unsigned int firqs_pending = 0;
static unsigned int cc_changed = 0;

static WORD *index_regs[4] = { &X, &Y, &U, &S };

extern void nmi(void);
extern void irq(void);
extern void firq(void);

void request_nmi(unsigned int source)
{
    /* If the interrupt is not masked, generate
     * IRQ immediately.  Else, mark it pending and
     * we'll check it later when the flags change.
     */
    nmi();
}

void req_irq(unsigned int source)
{
    /* If the interrupt is not masked, generate
     * IRQ immediately.  Else, mark it pending and
     * we'll check it later when the flags change.
     */
    irqs_pending |= (1 << source);
    if (!(EFI & I_FLAG)) {
        irq();
    }
}

void release_irq(unsigned int source)
{
    irqs_pending &= ~(1 << source);
}


void request_firq (unsigned int source)
{
    /* If the interrupt is not masked, generate
     * IRQ immediately.  Else, mark it pending and
     * we'll check it later when the flags change.
     */
    firqs_pending |= (1 << source);
    if (!(EFI & F_FLAG)) {
        firq();
    }
}

void release_firq(unsigned int source)
{
    firqs_pending &= ~(1 << source);
}

void sim_error(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "m6809-run: (at PC=%04X) ", iPC);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

static inline BYTE imm_byte(void)
{
    BYTE val = read8(PC);

    PC++;
    return val;
}

static inline WORD imm_word(void)
{
    WORD val = read16(PC);

    PC += 2;
    return val;
}

#ifdef H6309
static inline DWORD imm_dword(void)
{
    DWORD val = read32(PC);

    PC += 4;
    return val;
}
#endif

static void WRMEM(WORD addr, BYTE data)
{
    write8(addr, data);
    CLK++;
}

static void WRMEM16(WORD addr, WORD data)
{
    write8(addr, (BYTE)(data >> 8));
    CLK++;
    write8((WORD)(addr + 1), (BYTE)(data & 0xff));
    CLK++;
}

static BYTE RDMEM(WORD addr)
{
    BYTE val = read8(addr);

    CLK++;
    return val;
}

static WORD RDMEM16(WORD addr)
{
    WORD val = read8(addr) << 8;

    CLK++;
    val |= read8((WORD)(addr + 1));
    CLK++;
    return val;
}

#define read_stack  read8

static void write_stack(WORD addr, BYTE data)
{
    write8(addr, data);
    CLK++;
}

static void write_stack16(WORD addr, WORD data)
{
    write8((WORD)(addr + 1), (BYTE)(data & 0xff));
    CLK++;
    write8(addr, (BYTE)(data >> 8));
    CLK++;
}

static BYTE read_stack(WORD addr)
{
    BYTE val = read8(addr);

    CLK++;
    return val;
}

static WORD read_stack16(WORD addr)
{
    WORD val = read8(addr) << 8;

    CLK++;
    val |= read8((WORD)(addr + 1));
    return val;
}

static void direct(void)
{
    ea = RDMEM(PC) | DP;
    PC++;
}

static void indexed(void)
{
    BYTE post = imm_byte();
    WORD *R = index_regs[(post >> 5) & 0x3];

    if (post & 0x80) {
        switch (post & 0x1f) {
            case 0x00:	/* ,R+ */
                ea = *R;
                *R += 1;
                CLK_ADD(2, 1);
                break;
            case 0x01:	/* ,R++ */
                ea = *R;
                *R += 2;
                CLK_ADD(3, 2);
                break;
            case 0x02:	/* ,-R */
                *R -= 1;
                ea = *R;
                CLK_ADD(2, 1);
                break;
            case 0x03:	/* ,--R */
                *R -= 2;
                ea = *R;
                CLK_ADD(3, 2);
                break;
            case 0x04:	/* ,R */
                ea = *R;
                break;
            case 0x05:	/* B,R */
                ea = *R + (INT8)B;
                CLK++;
                break;
            case 0x06:	/* A,R */
                ea = *R + (INT8)A;
                CLK++;
                break;
#ifdef FULL6809
            case 0x07:	/* ,R (UNDOC) */
                ea = *R;
                break;
#endif
#ifdef H6309
            case 0x07:	/* E,R */
                ea = *R + (INT8)E;
                CLK++;
                break;
#endif
            case 0x08:	/* 8bit,R */
                ea = *R + (INT8)imm_byte();
                break;
            case 0x09:	/* 16bit,R */
                ea = *R + imm_word();
                CLK_ADD(2, 1);
                break;
#ifdef FULL6809
            case 0x0a:	/* ,PC | 0xff (UNDOC) */
                ea = PC | 0xff;
                break;
#endif
#ifdef H6309
            case 0x0a:	/* F,R */
                ea = *R + (INT8)F;
                CLK++;
                break;
#endif
            case 0x0b:	/* D,R */
                ea = *R + D;
                CLK_ADD(4, 2);
                break;
            case 0x0c:	/* 8bit,PC */
                ea = (INT8)imm_byte();
                ea += PC;
                break;
            case 0x0d:	/* 16bit,PC */
                ea = imm_word();
                ea += PC;
                CLK_ADD(3, 1);
                break;
#ifdef H6309
            case 0x0e:	/* W,R */
                ea = *R + W;
                CLK_ADD(4, 1);
                break;
#endif
#ifdef FULL6809
            case 0x0f:	/* ,extended (UNDOC) */
                ea = imm_word();
                break;
#endif
#ifdef H6309
            case 0x0f:	/* relative to W */
                switch (post & 0x60) {
                    case 0x00:	/* ,W */
                        ea = W;
                        break;
                    case 0x20:	/* 16bit,W */
                        ea = imm_word();
                        ea += W;
                        CLK_ADD(3, 0);
                        break;
                    case 0x40:	/* ,W++ */
                        ea = W;
                        W += 2;
                        CLK_ADD(3, 1);
                        break;
                    case 0x60:	/* ,--W */
                        W -= 2;
                        ea = W;
                        CLK_ADD(3, 1);
                        break;
                }
                break;
#endif
#ifdef FULL6809
            case 0x10:	/* [,R+] (UNDOC) */
                ea = *R;
                *R += 1;
                CLK++;
                ea = RDMEM16(ea);
                break;
#endif
#ifdef H6309
            case 0x10:	/* indirect relative to W */
                switch (post & 0x60) {
                    case 0x00:	/* [,W] */
                        ea = W;
                        ea = RDMEM16(ea);
                        break;
                    case 0x20:	/* [16bit,W] */
                        ea = imm_word();
                        ea += W;
                        CLK++;
                        ea = RDMEM16(ea);
                        break;
                    case 0x40:	/* [,W++] */
                        ea = W;
                        W += 2;
                        CLK++;
                        ea = RDMEM16(ea);
                        break;
                     case 0x60:	/* [,--W] */
                        W -= 2;
                        ea = W;
                        CLK++;
                        ea = RDMEM16(ea);
                        break;
                }
                break;
#endif
            case 0x11:	/* [,R++] */
                ea = *R;
                *R += 2;
                CLK += 4;
                ea = RDMEM16(ea);
                break;
#if defined(FULL6809) || defined(H6309)
            case 0x12:	/* [,-R] (UNDOC) */
                *R -= 1;
                ea = *R;
                CLK += 3;
                ea = RDMEM16(ea);
                break;
#endif
            case 0x13:	/* [,--R] */
                *R -= 2;
                ea = *R;
                CLK += 4;
                ea = RDMEM16(ea);
                break;
            case 0x14:	/* [,R] */
                ea = *R;
                CLK++;
                ea = RDMEM16(ea);
                break;
            case 0x15:	/* [B,R] */
                ea = *R + (INT8)B;
                CLK += 2;
                ea = RDMEM16(ea);
                break;
            case 0x16:	/* [A,R] */
                ea = *R + (INT8)A;
                CLK += 2;
                ea = RDMEM16(ea);
                break;
#ifdef FULL6809
            case 0x17:	/* [,R] (UNDOC) */
                ea = *R;
                CLK++;
                ea = RDMEM16(ea);
                break;
#endif
#ifdef H6309
            case 0x17:	/* [E,R] */
                ea = *R + (INT8)E;
                ea = RDMEM16(ea);
                break;
#endif
            case 0x18:	/* [8bit,R] */
                ea = *R + (INT8)imm_byte();
                CLK++;
                ea = RDMEM16(ea);
                break;
            case 0x19:	/* [16bit,R] */
                ea = *R + imm_word();
                CLK += 3;
                ea = RDMEM16(ea);
                break;
#ifdef FULL6809
            case 0x1a:	/* [,PC | 0xff] (UNDOC) */
                ea = PC | 0xff;
                CLK++;
                ea = RDMEM16(ea);
                break;
#endif
#ifdef H6309
            case 0x1a:	/* [F,R] */
                ea = *R + (INT8)F;
                ea = RDMEM16(ea);
                break;
#endif
            case 0x1b:	/* [D,R] */
                ea = *R + D;
                CLK += 2;
                ea = RDMEM16(ea);
                break;
            case 0x1c:	/* [8bit,PC] */
                ea = (INT8)imm_byte();
                ea += PC;
                CLK++;
                ea = RDMEM16(ea);
                break;
            case 0x1d:	/* [16bit,PC] */
                ea = imm_word();
                ea += PC;
                CLK += 4;
                ea = RDMEM16(ea);
                break;
#ifdef H6309
            case 0x1e:	/* [W,R] */
                ea = *R + W;
                CLK += 2;
                ea = RDMEM16(ea);
                break;
#endif
            case 0x1f:	/* [16bit] */
                ea = imm_word();
                CLK++;
                ea = RDMEM16(ea);
                break;
            default:
                ea = 0;
                sim_error("invalid index post $%02X\n", post);
                break;
        }
    } else {
        if (post & 0x10) {
            ea = *R + (post | 0xfff0);
        } else {
            ea = *R + (post & 0x000f);
        }
        CLK++;
    }
}

static void extended(void)
{
    ea = RDMEM16(PC);
    PC += 2;
}

/* external register functions */

static BYTE get_a(void)
{
    return A;
}

static BYTE get_b(void)
{
    return B;
}

static BYTE get_dp(void)
{
    return DP >> 8;
}

static WORD get_x(void)
{
    return X;
}

static WORD get_y(void)
{
    return Y;
}

static WORD get_s(void)
{
    return S;
}

static WORD get_u(void)
{
    return U;
}

static WORD get_pc(void)
{
    return PC;
}

static WORD get_d(void)
{
    return D;
}

static BYTE get_flags(void)
{
    return EFI;
}

#ifdef H6309
static BYTE get_e(void)
{
    return E;
}

static BYTE get_f(void)
{
    return F;
}

static WORD get_w(void)
{
    return W;
}

static DWORD get_q(void)
{
    return Q;
}

static WORD get_v(void)
{
    return V;
}

static BYTE get_md(void)
{
    return (MD & (MD_ILL | MD_DBZ));
}
#endif

static void set_a(BYTE val)
{
    A = val;
}

static void set_b(BYTE val)
{
    B = val;
}

static void set_dp(BYTE val)
{
    DP = val << 8;
}

static void set_x(WORD val)
{
    X = val;
}

static void set_y(WORD val)
{
    Y = val;
}

static void set_s(WORD val)
{
    S = val;
}

static void set_u(WORD val)
{
    U = val;
}

static void set_pc(WORD val)
{
    PC = val;
}

static void set_d(WORD val)
{
    D = val;
}

#ifdef H6309
static void set_e(BYTE val)
{
    E = val;
}

static void set_f(BYTE val)
{
    F = val;
}

static void set_w(WORD val)
{
    W = val;
}

static void set_q(DWORD val)
{
    Q = val;
}

static void set_v(WORD val)
{
    V = val;
}

static void set_md(BYTE val)
{
    MD = (MD & (MD_ILL | MD_DBZ)) | (val & (MD_NATIVE | MD_FIRQ_LIKE_IRQ));
}
#endif


/* handle condition code register */

static BYTE get_cc(void)
{
    BYTE res = EFI & (E_FLAG | F_FLAG | I_FLAG);

    if (H & 0x10) {
        res |= H_FLAG;
    }
    if (N & 0x80) {
        res |= N_FLAG;
    }
    if (Z == 0) {
        res |= Z_FLAG;
    }
    if (OV & 0x80) {
        res |= V_FLAG;
    }
    if (C != 0) {
        res |= C_FLAG;
    }
    return res;
}

static void set_cc(BYTE arg)
{
    EFI = arg & (E_FLAG | F_FLAG | I_FLAG);
    H = (arg & H_FLAG ? 0x10 : 0);
    N = (arg & N_FLAG ? 0x80 : 0);
    Z = (~arg) & Z_FLAG;
    OV = (arg & V_FLAG ? 0x80 : 0);
    C = arg & C_FLAG;
    cc_changed = 1;
}

static void cc_modified(void)
{
    /* Check for pending interrupts */
    if (firqs_pending && !(EFI & F_FLAG)) {
        firq();
    } else if (irqs_pending && !(EFI & I_FLAG)) {
        irq();
    }
    cc_changed = 0;
}

/* Undocumented: When the 6809 transfers an 8bit register
   to a 16bit register the LSB is filled with the 8bit
   register value and the MSB is filled with 0xff.
 */

static WORD get_reg(BYTE nro)
{
    WORD val = 0xffff;

    switch (nro) {
        case 0:
            val = D;
            break;
        case 1:
            val = X;
            break;
        case 2:
            val = Y;
            break;
        case 3:
            val = U;
            break;
        case 4:
            val = S;
            break;
        case 5:
            val = PC;
            break;
#ifdef H6309
        case 6:
            val = W;
            break;
        case 7:
            val = V;
            break;
#endif
        case 8:
            val = A & 0xffff;
            break;
        case 9:
            val = B & 0xffff;
            break;
        case 10:
            val = get_cc() & 0xffff;
            break;
        case 11:
            val = (DP >> 8) & 0xffff;
            break;
#ifdef H6309
        case 14:
            val = E & 0xffff;
            break;
        case 15:
            val = F & 0xffff;
            break;
#endif
    }
    return val;
}

static void set_reg(BYTE nro, WORD val)
{
    switch (nro) {
        case 0:
            D = val;
            break;
        case 1:
            X = val;
            break;
        case 2:
            Y = val;
            break;
        case 3:
            U = val;
            break;
        case 4:
            S = val;
            break;
        case 5:
            PC = val;
            break;
#ifdef H6309
        case 6:
            W = val;
            break;
        case 7:
            V = val;
            break;
#endif
        case 8:
            A = val & 0xff;
            break;
        case 9:
            B = val & 0xff;
            break;
        case 10:
            set_cc((BYTE)(val & 0xff));
            break;
        case 11:
            DP = (val & 0xff) << 8;
            break;
#ifdef H6309
        case 14:
            E = val & 0xff;
            break;
        case 15:
            F = val & 0xff;
            break;
#endif
    }
}

/* 8-Bit Accumulator and Memory Instructions */

static BYTE adc(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    WORD res = arg + val + (C != 0);

    C = (res >> 1) & 0x80;
    N = Z = res &= 0xff;
    OV = H = arg ^ val ^ res ^ C;

    CLK_ADD(CLK6809, CLK6309);

    return (BYTE)res;
}

static BYTE add(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    WORD res = arg + val;

    C = (res >> 1) & 0x80;
    N = Z = res &= 0xff;
    OV = H = arg ^ val ^ res ^ C;

    CLK_ADD(CLK6809, CLK6309);

    return (BYTE)res;
}

static BYTE and(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    BYTE res = arg & val;

    N = Z = res;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE asl(BYTE arg, int CLK6809, CLK6309)		/* same as lsl */
{
    WORD res = arg << 1;

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = arg ^ res;
    CLK_ADD(CLK6809, CLK6309);

    return (BYTE)res;
}

static BYTE asr(BYTE arg, int CLK6809, int CLK6309)
{
    WORD res = (INT8)arg;

    C = res & 1;
    N = Z = res = (res >> 1) & 0xff;
    CLK_ADD(CLK6809, CLK6309);

    return (BYTE)res;
}

static void bit(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    N = Z = arg & val;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);
}

static BYTE clr(BYTE arg, int CLK6809, int CLK6309)
{
    C = N = Z = OV = 0;
    CLK_ADD(CLK6809, CLK6309);

    return 0;
}

static void cmp(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    WORD res = arg - val;

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = (arg ^ val) & (arg ^ res);

    CLK_ADD(CLK6809, CLK6309);
}

static BYTE com(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = ~arg;

    N = Z = res;
    OV = 0;
    C = 1;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static void daa(void)
{
    WORD res = A;
    BYTE msn = res & 0xf0;
    BYTE lsn = res & 0x0f;

    if (lsn > 0x09 || (H & 0x10)) {
        res += 0x06;
    }
    if (msn > 0x80 && lsn > 0x09) {
        res += 0x60;
    }
    if (msn > 0x90 || (C != 0)) {
        res += 0x60;
    }

    C |= (res & 0x100);
    N = Z = res &= 0xff;
    A = (BYTE)res;

    CLK_ADD(1, 0);
}

static BYTE dec(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = arg - 1;

    N = Z = res;
    OV = arg & ~res;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE eor(BYTE arg, BYTE val)
{
    BYTE res = arg ^ val;

    N = Z = res;
    OV = 0;

    return res;
}

static BYTE eor_new(BYTE arg, BYTE val, int CLK6809, int CLK6309)
{
    BYTE res = arg ^ val;

    N = Z = res;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static void exg(void)
{
    WORD tmp1 = 0xff;
    WORD tmp2 = 0xff;
    BYTE post = imm_byte();

    if (((post ^ (post << 4)) & 0x80) == 0) {
        tmp1 = get_reg((BYTE)(post >> 4));
        tmp2 = get_reg((BYTE)(post & 15));
    }

    set_reg((BYTE)(post & 15), tmp1);
    set_reg((BYTE)(post >> 4), tmp2);

    CLK += 8;
}

static void exg_new(void)
{
    WORD tmp1 = 0xff;
    WORD tmp2 = 0xff;
    BYTE post = imm_byte();

    if (((post ^ (post << 4)) & 0x80) == 0) {
        tmp1 = get_reg((BYTE)(post >> 4));
        tmp2 = get_reg((BYTE)(post & 15));
    }

    set_reg((BYTE)(post & 15), tmp1);
    set_reg((BYTE)(post >> 4), tmp2);

    CLK_ADD(6, 3);
}

static BYTE inc(BYTE arg)
{
    BYTE res = arg + 1;

    N = Z = res;
    OV = ~arg & res;
    CLK += 2;

    return res;
}

static BYTE inc_new(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = arg + 1;

    N = Z = res;
    OV = ~arg & res;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE ld(BYTE arg)
{
    N = Z = arg;
    OV = 0;

    return arg;
}

static BYTE lsr(BYTE arg)
{
    BYTE res = arg >> 1;

    N = 0;
    Z = res;
    C = arg & 1;
    CLK += 2;

    return res;
}

static BYTE lsr_new(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = arg >> 1;

    N = 0;
    Z = res;
    C = arg & 1;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static void mul(void)
{
    WORD res = A * B;

    Z = D = res;
    C = res & 0x80;
    CLK_ADD(10, 9);
}

static BYTE neg(BYTE arg)
{
    BYTE res = ~arg + 1;

    C = N = Z = res;
    OV = res & arg;
    CLK += 2;

    return res;
}

static BYTE neg_new(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = ~arg + 1;

    C = N = Z = res;
    OV = res & arg;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE or(BYTE arg, BYTE val)
{
    BYTE res = arg | val;

    N = Z = res;
    OV = 0;

    return res;
}

static BYTE or_new(BYTE arg, BYTE val, int CLK6809, CLK6309)
{
    BYTE res = arg | val;

    N = Z = res;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE rol(BYTE arg)
{
    WORD res = (arg << 1) + (C != 0);

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = arg ^ res;
    CLK += 2;

    return (BYTE)res;
}

static BYTE rol_new(BYTE arg, int CLK6809, int CLK6309)
{
    WORD res = (arg << 1) + (C != 0);

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = arg ^ res;
    CLK_ADD(CLK6809, CLK6309);

    return (BYTE)res;
}

static BYTE ror(BYTE arg)
{
    BYTE res = (arg >> 1) | ((C != 0) << 7);

    C = arg & 1;
    N = Z = res;
    CLK += 2;

    return res;
}

static BYTE ror_new(BYTE arg, int CLK6809, int CLK6309)
{
    BYTE res = (arg >> 1) | ((C != 0) << 7);

    C = arg & 1;
    N = Z = res;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static BYTE sbc(BYTE arg, BYTE val)
{
    WORD res = arg - val - (C != 0);

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = (arg ^ val) & (arg ^ res);

    return (BYTE)res;
}

static void st(BYTE arg)
{
    N = Z = arg;
    OV = 0;

    WRMEM(ea, arg);
}

static BYTE sub(BYTE arg, BYTE val)
{
    WORD res = arg - val;

    C = res & 0x100;
    N = Z = res &= 0xff;
    OV = (arg ^ val) & (arg ^ res);

    return (BYTE)res;
}

static void tst(BYTE arg)
{
    N = Z = arg;
    OV = 0;
    CLK += 2;
}

static void tst_new(BYTE arg, int CLK6809, CLK6309)
{
    N = Z = arg;
    OV = 0;
    CLK_ADD(CLK6809, CLK6309);
}

static void tfr(void)
{
    WORD tmp1 = 0xff;
    BYTE post = imm_byte();

    if (((post ^ (post << 4)) & 0x80) == 0) {
        tmp1 = get_reg ((BYTE)(post >> 4));
    }

    set_reg((BYTE)(post & 15), tmp1);

    CLK += 6;
}

static void tfr_new(void)
{
    WORD tmp1 = 0xff;
    BYTE post = imm_byte();

    if (((post ^ (post << 4)) & 0x80) == 0) {
        tmp1 = get_reg ((BYTE)(post >> 4));
    }

    set_reg((BYTE)(post & 15), tmp1);

    CLK_ADD(4, 2);
}

/* 16-Bit Accumulator Instructions */

static void abx(void)
{
    X += B;
    CLK_ADD(2, 0);
}

static WORD add16(WORD arg, WORD val, int CLK6809, CLK6309)
{
    DWORD res = arg + val;

    C = (res >> 1) & 0x8000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = ((arg ^ res) & (val ^ res)) >> 8;

    CLK_ADD(CLK6809, CLK6309);

    return (WORD)res;
}

static void cmp16(WORD arg, WORD val, int CLK6809, int CLK6309)
{
    DWORD res = arg - val;

    C = res & 0x10000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = ((arg ^ val) & (arg ^ res)) >> 8;

    CLK_ADD(CLK6809, CLK6309);
}

static WORD ld16(WORD arg)
{
    Z = arg;
    N = arg >> 8;
    OV = 0;

    return arg;
}

static void sex(void)
{
    D = (INT8)B;

    Z = D;
    N = D >> 8;
    CLK += 2;
}

static void sex_new(void)
{
    D = (INT8)B;

    Z = D;
    N = D >> 8;

    CLK_ADD(1, 0);
}

static void st16(WORD arg)
{
    Z = arg;
    N = arg >> 8;
    OV = 0;
    WRMEM16(ea, arg);
}

static WORD sub16(WORD arg, WORD val)
{
    DWORD res = arg - val;

    C = res & 0x10000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = ((arg ^ val) & (arg ^ res)) >> 8;

    return (WORD)res;
}

/* stack instructions */

static void pshs(void)
{
    BYTE post = imm_byte();

    CLK_ADD(3, 2);

    if (post & 0x80) {
        S -= 2;
        write_stack16(S, PC);
    }
    if (post & 0x40) {
        S -= 2;
        write_stack16(S, U);
    }
    if (post & 0x20) {
        S -= 2;
        write_stack16(S, Y);
    }
    if (post & 0x10) {
        S -= 2;
        write_stack16(S, X);
    }
    if (post & 0x08) {
        S--;
        write_stack(S, (BYTE)(DP >> 8));
    }
    if (post & 0x04) {
        S--;
        write_stack(S, B);
    }
    if (post & 0x02) {
        S--;
        write_stack(S, A);
    }
    if (post & 0x01) {
        S--;
        write_stack(S, get_cc());
    }
}

static void pshu(void)
{
    BYTE post = imm_byte();

    CLK_ADD(3, 2);

    if (post & 0x80) {
        U -= 2;
        write_stack16(U, PC);
    }
    if (post & 0x40) {
        U -= 2;
        write_stack16(U, S);
    }
    if (post & 0x20) {
        U -= 2;
        write_stack16(U, Y);
    }
    if (post & 0x10) {
        U -= 2;
        write_stack16(U, X);
    }
    if (post & 0x08) {
        U--;
        write_stack(U, (BYTE)(DP >> 8));
    }
    if (post & 0x04) {
        U--;
        write_stack(U, B);
    }
    if (post & 0x02) {
        U--;
        write_stack(U, A);
    }
    if (post & 0x01) {
        U--;
        write_stack(U, get_cc());
    }
}

static void puls(void)
{
    BYTE post = imm_byte();

    CLK_ADD(3, 2);

    if (post & 0x01) {
        set_cc(read_stack(S++));
    }
    if (post & 0x02) {
        A = read_stack(S++);
    }
    if (post & 0x04) {
        B = read_stack(S++);
    }
    if (post & 0x08) {
        DP = read_stack(S++) << 8;
    }
    if (post & 0x10) {
        X = read_stack16(S);
        S += 2;
    }
    if (post & 0x20) {
        Y = read_stack16(S);
        S += 2;
    }
    if (post & 0x40) {
        U = read_stack16(S);
        S += 2;
    }
    if (post & 0x80) {
        PC = read_stack16(S);
        S += 2;
    }
}

static void pulu(void)
{
    BYTE post = imm_byte();

    CLK_ADD(3, 2);

    if (post & 0x01) {
        set_cc(read_stack(U++));
    }
    if (post & 0x02) {
        A = read_stack(U++);
    }
    if (post & 0x04) {
        B = read_stack(U++);
    }
    if (post & 0x08) {
        DP = read_stack(U++) << 8;
    }
    if (post & 0x10) {
        X = read_stack16(U);
        U += 2;
    }
    if (post & 0x20) {
        Y = read_stack16(U);
        U += 2;
    }
    if (post & 0x40) {
        S = read_stack16(U);
        U += 2;
    }
    if (post & 0x80) {
        PC = read_stack16(U);
        U += 2;
    }
}

/* Miscellaneous Instructions */

static void nop(void)
{
    CLK += 2;
}

static void jsr(void)
{
    S -= 2;
    write_stack16(S, PC);
    PC = ea;
}

static void rti(void)
{
    CLK += 3;
    set_cc(read_stack(S));
    S++;

    if ((EFI & E_FLAG) != 0) {
        A = read_stack(S++);
        B = read_stack(S++);
#ifdef H6309
        if (H6309_NATIVE_MODE()) {
            E = read_stack(S++);
            F = read_stack(S++);
        }
#endif
        DP = read_stack(S++) << 8;
        X = read_stack16(S);
        S += 2;
        Y = read_stack16(S);
        S += 2;
        U = read_stack16(S);
        S += 2;
    }
    PC = read_stack16(S);
    S += 2;
}

static void rts(void)
{
    CLK_ADD(2, 1);
    PC = read_stack16(S);
    S += 2;
}

void nmi(void)
{
    EFI |= E_FLAG;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
#endif
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());
    EFI |= I_FLAG;

    PC = read16(0xfffc);
}

void irq(void)
{
    EFI |= E_FLAG;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
#endif
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());
    EFI |= I_FLAG;

    PC = read16(0xfff8);
    irqs_pending = 0;
}

void firq(void)
{
    EFI &= ~E_FLAG;
    S -= 2;
    write_stack16(S--, PC);
#ifdef H6309
    if (MD & MD_FIRQ_LIKE_IRQ) {
        S -= 2;
        write_stack16(S, U);
        S -= 2;
        write_stack16(S, Y);
        S -= 2;
        write_stack16(S--, X);
        write_stack(S--, (BYTE)(DP >> 8));
        if (H6309_NATIVE_MODE()) {
            write_stack(S--, F);
            write_stack(S--, E);
        }
        write_stack(S--, B);
        write_stack(S--, A);
    }
#endif
    write_stack(S, get_cc());

    EFI |= (I_FLAG | F_FLAG);

    PC = read16(0xfff6);
    firqs_pending = 0;
}

void swi(void)
{
    CLK += 6;
    EFI |= E_FLAG;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
#endif
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());
    EFI |= (I_FLAG | F_FLAG);

    PC = read16(0xfffa);
}

void swi2(void)
{
    CLK += 6;
    EFI |= E_FLAG;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
#endif
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());

    PC = read16(0xfff4);
}

void swi3(void)
{
    CLK += 6;
    EFI |= E_FLAG;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
#endif
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());

    PC = read16(0xfff2);
}

#ifdef H6309
void opcode_trap(void)
{
    CLK += 6;
    EFI |= E_FLAG;
    MD &= MD_ILL;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());

    PC = read16(0xfff0);
}

void div0_trap(void)
{
    CLK += 6;
    EFI |= E_FLAG;
    MD &= MD_DBZ;
    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
    if (H6309_NATIVE_MODE()) {
        write_stack(S--, F);
        write_stack(S--, E);
    }
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());

    PC = read16(0xfff0);
}
#endif

void cwai(void)
{
    sim_error("CWAI - not supported yet!");
}

/* FIXME: cycle count */
void sync(void)
{
    CLK += 4;
    sim_error("SYNC - not supported yet!");
}

static void orcc(void)
{
    BYTE tmp = imm_byte();

    set_cc((BYTE)(get_cc() | tmp));
    CLK += 3;
}

static void orcc_new(void)
{
    BYTE tmp = imm_byte();

    set_cc((BYTE)(get_cc() | tmp));
}

static void andcc(void)
{
    BYTE tmp = imm_byte();

    set_cc((BYTE)(get_cc() & tmp));
    CLK++;
}

/* Branch Instructions */

#define cond_HI() ((Z != 0) && (C == 0))
#define cond_LS() ((Z == 0) || (C != 0))
#define cond_HS() (C == 0)
#define cond_LO() (C != 0)
#define cond_NE() (Z != 0)
#define cond_EQ() (Z == 0)
#define cond_VC() ((OV & 0x80) == 0)
#define cond_VS() ((OV & 0x80) != 0)
#define cond_PL() ((N & 0x80) == 0)
#define cond_MI() ((N & 0x80) != 0)
#define cond_GE() (((N^OV) & 0x80) == 0)
#define cond_LT() (((N^OV) & 0x80) != 0)
#define cond_GT() ((((N^OV) & 0x80) == 0) && (Z != 0))
#define cond_LE() ((((N^OV) & 0x80) != 0) || (Z == 0))

static void bra(void)
{
    INT8 tmp = (INT8)imm_byte();
    PC += tmp;
    CLK++;
}

static void branch(unsigned cond)
{
    if (cond) {
        bra();
    } else {
        imm_byte();
        CLK++;
    }
}

static void long_bra(void)
{
    WORD tmp = imm_word();

    PC += tmp;
}

static void long_branch(unsigned cond)
{
    if (cond) {
        long_bra();
        CLK += 6;
    } else {
        PC += 2;
        CLK += 5;
    }
}

static void long_bsr(void)
{
    WORD tmp = imm_word();

    ea = PC + tmp;
    S -= 2;
    write_stack16(S, PC);
#ifdef H6309
    if (H6309_NATIVE_MODE()) {
        CLK += 5;
    } else
#endif
    CLK += 7;
    PC = ea;
}

static void long_bsr_new(void)
{
    WORD tmp = imm_word();

    ea = PC + tmp;
    S -= 2;
    write_stack16(S, PC);
    PC = ea;
}

static void bsr(void)
{
    INT8 tmp = (INT8)imm_byte();

    ea = PC + tmp;
    S -= 2;
    write_stack16(S, PC);
    CLK_ADD(3, 2);
    PC = ea;
}

/* Undocumented 6809 specific code */
#ifdef FULL6809

/* FIXME: cycle count */
void hcf(void)
{
    sim_error("HCF - not supported yet!");
}

void ccrs(void)
{
    DWORD tmp_c = (OV != 0);
    DWORD tmp_h = ((EFI & I_FLAG) != 0);

    set_cc(0);
    C = tmp_c;
    H = tmp_h << 4;
    CLK += 2;
}

void scc(BYTE arg)
{
    N = 0x80;
    Z = OV = 0;
}

void st_imm(WORD arg)
{
    WRMEM(PC++, (BYTE)(arg & 0xff));
    N = 0x80;
    Z = OV = 0;
    /* TODO: cycle count */
}

void swires(void)
{
    CLK += 6;

    S -= 2;
    write_stack16(S, PC);
    S -= 2;
    write_stack16(S, U);
    S -= 2;
    write_stack16(S, Y);
    S -= 2;
    write_stack16(S--, X);
    write_stack(S--, (BYTE)(DP >> 8));
    write_stack(S--, B);
    write_stack(S--, A);
    write_stack(S, get_cc());
    EFI |= (I_FLAG | F_FLAG);

    PC = read16(0xfffe);
}
#endif

/* 6309 specific code */
#ifdef H6309
static BYTE tim(BYTE val, int CLK6309)
{
    OV = 0;
    N = Z = val;

    CLK += CLK6309;

    return val;
}

static void pshsw(void)
{
    /* TODO: cycle count */

    S -= 2;
    write_stack16(S, W);
}

static void pshuw(void)
{
    /* TODO: cycle count */

    U -= 2;
    write_stack16(U, W);
}

static void pulsw(void)
{
    /* TODO: cycle count */

    W = read_stack16(S);
    S += 2;
}

static void puluw(void)
{
    /* TODO: cycle count */

    W = read_stack16(U);
    U += 2;
}

static WORD neg16(WORD arg)
{
    WORD res = ~arg + 1;

    C = Z = res;
    N = res >> 8;
    OV = (res & arg) >> 8;
    /* TODO: cycle count */

    return res;
}

static WORD com16(WORD arg, int CLK6809, CLK6309)
{
    WORD res = ~arg;

    Z = res;
    N = res >> 8;
    OV = 0;
    C = 1;
    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static WORD lsr16(WORD arg)
{
    WORD res = arg >> 1;

    N = 0;
    Z = res;
    C = arg & 1;
    /* TODO: cycle count */

    return res;
}

static WORD ror16(WORD arg)
{
    WORD res = (arg >> 1) | ((C != 0) << 15);

    C = arg & 1;
    Z = res;
    N = res >> 8;
    /* TODO: cycle count */

    return res;
}

static WORD asr16(WORD arg, int CLK6809, int CLK6309)
{
    DWORD res = (SWORD)arg;

    C = res & 1;
    Z = res = (res >> 1) & 0xffff;
    N = res >> 8;
    CLK_ADD(CLK6809, CLK6309);

    return (WORD)res;
}

static WORD asl16(WORD arg, int CLK6809, int CLK6309)		/* same as lsl16 */
{
    DWORD res = arg << 1;

    C = res & 0x10000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = (arg ^ res) >> 8;
    CLK_ADD(CLK6809, CLK6309);

    return (WORD)res;
}

static WORD rol16(WORD arg)
{
    DWORD res = (arg << 1) + (C != 0);

    C = res & 0x10000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = (arg ^ res) >> 8;
    /* TODO: cycle count */

    return (WORD)res;
}

static WORD dec16(WORD arg, int CLK6809, int CLK6309)
{
    WORD res = arg - 1;

    Z = res;
    N = res >> 8;
    OV = (arg & ~res) >> 8;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static WORD inc16(WORD arg)
{
    WORD res = arg + 1;

    Z = res;
    N = res >> 8;
    OV = (~arg & res) >> 8;
    /* TODO: cycle count */

    return res;
}

static void tst16(WORD arg)
{
    Z = arg;
    N = arg >> 8;
    OV = 0;
    /* TODO: cycle count */
}

static WORD clr16(WORD arg, int CLK6809, CLK6309)
{
    C = N = Z = OV = 0;
    CLK_ADD(CLK6809, CLK6309);

    return 0;
}

static WORD sbc16(WORD arg, WORD val)
{
    DWORD res = arg - val - (C != 0);

    C = res & 0x10000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = ((arg ^ val) & (arg ^ res)) >> 8;

    return (WORD)res;
}

static WORD and16(WORD arg, WORD val, int CLK6809, int CLK6309)
{
    WORD res = arg & val;

    Z = res;
    N = res >> 8;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);

    return res;
}

static void bit16(WORD arg, WORD val, int CLK6809, int CLK6309)
{
    WORD res = arg & val;

    Z = res;
    N = res >> 8;
    OV = 0;

    CLK_ADD(CLK6809, CLK6309);
}

static WORD eor16(WORD arg, WORD val)
{
    WORD res = arg ^ val;

    Z = res;
    N = res >> 8;
    OV = 0;

    return res;
}

static WORD adc16(WORD arg, WORD val, int CLK6809, int CLK6309)
{
    DWORD res = arg + val + (C != 0);

    C = (res >> 1) & 0x8000;
    Z = res &= 0xffff;
    N = res >> 8;
    OV = H = (arg ^ val ^ res ^ C) >> 8;

    CLK_ADD(CLK6809, CLK6309);

    return (WORD)res;
}

static WORD or16(WORD arg, WORD val)
{
    WORD res = arg | val;

    Z = res;
    N = res >> 8;
    OV = 0;

    return res;
}

static BYTE get_breg(BYTE rnr)
{
    switch (rnr) {
        case 0:
            return get_cc();
        case 1:
            return A;
        case 2:
            return B;
    }
    return 0;
}

static void set_breg(BYTE rnr, BYTE arg)
{
    switch (rnr) {
        case 0:
            set_cc(arg);
            break;
        case 1:
            A = arg;
            break;
        case 2:
            B = arg;
            break;
    }
}

static BYTE band(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = stmp & dtmp;

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static BYTE beor(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = stmp ^ dtmp;

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static BYTE biand(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = !(stmp & dtmp);

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static BYTE bieor(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = !(stmp ^ dtmp);

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static BYTE bior(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = !(stmp | dtmp);

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static BYTE bor(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;
    BYTE dtmp = (tmp & (1 << dbit)) ? 1 : 0;
    BYTE atmp = stmp | dtmp;

    tmp = (tmp & ~(1 << dbit)) | (atmp << dbit);

    OV = 0;
    N = Z = tmp;

    CLK_ADD(1, 0);

    return tmp;
}

static void ldbt(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (tmp & (1 << sbit)) ? 1 : 0;

    tmp = (tmp & ~(1 << dbit)) | (stmp << dbit);

    OV = 0;
    N = Z = tmp;

    /* TODO: cycle count */

    set_breg(rnr, tmp);
}

static BYTE stbt(BYTE rnr, BYTE arg)
{
    BYTE rr = get_breg(rnr);
    BYTE tmp = arg;
    BYTE sbit = (rnr >> 3) & 7;
    BYTE dbit = rnr & 7;
    BYTE stmp = (rr & (1 << sbit)) ? 1 : 0;

    tmp = (tmp & ~(1 << dbit)) | (stmp << dbit);

    OV = 0;
    N = Z = tmp;

    /* TODO: cycle count */

    return tmp;
}

/* reg type: 0=illegal, 1=byte, 2=word */
static int tfm_reg_type(BYTE rnr)
{
    switch (rnr) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
            return 2;
        case 0x8:
        case 0x9:
        case 0xe:
        case 0xf:
            return 1;
    }
    return 0;
}

static WORD tfm_get_reg(BYTE rnr)
{
    switch (rnr) {
        case 0x0:
            return D;
        case 0x1:
            return X;
        case 0x2:
            return Y;
        case 0x3:
            return U;
        case 0x4:
            return S;
        case 0x8:
            return (WORD)A;
        case 0x9:
            return (WORD)B;
        case 0xe:
            return (WORD)E;
        case 0xf:
            return (WORD)F;
    }
    return 0;
}

static void tfm_set_reg(BYTE rnr, WORD val)
{
    switch (rnr) {
        case 0x0:
            D = val;
            break;
        case 0x1:
            X = val;
            break;
        case 0x2:
            Y = val;
            break;
        case 0x3:
            U = val;
            break;
        case 0x4:
            S = val;
            break;
        case 0x8:
            A = (BYTE)val;
            break;
        case 0x9:
            B = (BYTE)val;
            break;
        case 0xe:
            E = (BYTE)val;
            break;
        case 0xf:
            F = (BYTE)val;
            break;
    }
}

static void tfmpp(BYTE rnr)
{
    BYTE val;
    BYTE r0_nr = rnr >> 4;
    BYTE r1_nr = rnr & 7;
    WORD r0 = tfm_get_reg(r0_nr);
    WORD r1 = tfm_get_reg(r1_nr);
    int r0_type = tfm_reg_type(r0_nr);
    int r1_type = tfm_reg_type(r1_nr);

    if (r0_type && r1_type) {
        while (W) {
            val = RDMEM(r0++);
            WRMEM(r1++, val);
            if (r0_type == 1) {
                r0 &= 0xff;
            }
            if (r1_type == 1) {
                r1 &= 0xff;
            }
            W--;
            /* TODO: cycle count */
        }
        tfm_set_reg(r0_nr, r0);
        tfm_set_reg(r1_nr, r1);
        /* TODO: cycle count */
    } else {
        opcode_trap();
    }
}

static void tfmmm(BYTE rnr)
{
    BYTE val;
    BYTE r0_nr = rnr >> 4;
    BYTE r1_nr = rnr & 7;
    WORD r0 = tfm_get_reg(r0_nr);
    WORD r1 = tfm_get_reg(r1_nr);
    int r0_type = tfm_reg_type(r0_nr);
    int r1_type = tfm_reg_type(r1_nr);

    if (r0_type && r1_type) {
        while (W) {
            val = RDMEM(r0--);
            WRMEM(r1--, val);
            if (r0_type == 1) {
                r0 &= 0xff;
            }
            if (r1_type == 1) {
                r1 &= 0xff;
            }
            W--;
            /* TODO: cycle count */
        }
        tfm_set_reg(r0_nr, r0);
        tfm_set_reg(r1_nr, r1);
        /* TODO: cycle count */
    } else {
        opcode_trap();
    }
}

static void tfmpc(BYTE rnr)
{
    BYTE val;
    BYTE r0_nr = rnr >> 4;
    BYTE r1_nr = rnr & 7;
    WORD r0 = tfm_get_reg(r0_nr);
    WORD r1 = tfm_get_reg(r1_nr);
    int r0_type = tfm_reg_type(r0_nr);
    int r1_type = tfm_reg_type(r1_nr);

    if (r0_type && r1_type) {
        while (W) {
            val = RDMEM(r0--);
            WRMEM(r1, val);
            if (r0_type == 1) {
                r0 &= 0xff;
            }
            W--;
            /* TODO: cycle count */
        }
        tfm_set_reg(r0_nr, r0);
        /* TODO: cycle count */
    } else {
        opcode_trap();
    }
}

static void tfmcp(BYTE rnr)
{
    BYTE val;
    BYTE r0_nr = rnr >> 4;
    BYTE r1_nr = rnr & 7;
    WORD r0 = tfm_get_reg(r0_nr);
    WORD r1 = tfm_get_reg(r1_nr);
    int r0_type = tfm_reg_type(r0_nr);
    int r1_type = tfm_reg_type(r1_nr);

    if (r0_type && r1_type) {
        while (W) {
            val = RDMEM(r0);
            WRMEM(r1--, val);
            if (r1_type == 1) {
                r1 &= 0xff;
            }
            W--;
            /* TODO: cycle count */
        }
        tfm_set_reg(r1_nr, r1);
        /* TODO: cycle count */
    } else {
        opcode_trap();
    }
}

static void divd(BYTE m, int CLK6809, int CLK6309)
{
    SWORD val, bak;

    if (m) {
        bak = (SWORD)D;
        val = bak / (INT8)m;
        A = bak % (INT8)m;
        B = val & 0xff;
        Z = N = B;
        C = B & 1;
        if (bak < 0) {
          Z = 0x80;
        }
        OV = 0;
        if ((val > 127) || (val < -128)) {
            OV = 0x80;
            if ((val > 255) || (val < -256)) {
                N = (WORD)(bak) >> 8;
                Z = bak;
                D = abs(bak);
            }
        }
    } else {
        div0_trap();
    }

    CLK_ADD(CLK6809, CLK6309);
}

static void divq(WORD m, int CLK6809, int CLK6309)
{
    SDWORD val, bak;

    if (m) {
        bak = (SDWORD)Q;
        val = bak / (SWORD)m;
        D = bak % (SWORD)m;
        W = val & 0xffff;
        N = W >> 8;
        Z = W;
        C = W & 1;
        if (bak < 0) {
            N = 0x80;
        }
        OV = 0;
        if ((val > 32768) || (val < -32767)) {
            OV = 0x80;
            if ((val > 65536) || (val < -65535)) {
                N = (DWORD)(bak) >> 24;
                Z = bak;
                Q = abs(bak);
            }
        }
    } else {
        div0_trap();
    }

    CLK_ADD(CLK6809, CLK6309);
}

static void muld(WORD m)
{
    Q = D * m;
    C = OV = 0;
    N = D >> 8;
    Z = D;
    /* TODO: cycle count */
}

static void sexw(void)
{
    W = (INT8)F;

    Z = W;
    N = W >> 8;
}

static DWORD ld32(DWORD arg)
{
    Z = arg;
    N = arg >> 24;
    OV = 0;

    return arg;
}
#endif

#ifdef H6309
#define CLK_ADD(a, b) CLK += (!H6309_NATIVE_MODE()) ? a, b
#else
#define CLK_ADD(a, b) CLK += a
#endif

/* Execute 6809 code for a certain number of cycles. */
void h6809_mainloop (struct interrupt_cpu_status_s *maincpu_int_status, alarm_context_t *maincpu_alarm_context)
{
    WORD opcode;
    BYTE fetch;
#ifdef H6309
    BYTE post_byte;
#endif

    do {

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
                release_irq(0);
            }

            pending_interrupt = CPU_INT_STATUS->global_pending_int;
            if (pending_interrupt != IK_NONE) {
                DO_INTERRUPT(pending_interrupt);
                if (!(CPU_INT_STATUS->global_pending_int & IK_IRQ)
                    && CPU_INT_STATUS->global_pending_int & IK_IRQPEND) {
                        CPU_INT_STATUS->global_pending_int &= ~IK_IRQPEND;
                }
                CPU_DELAY_CLK
#ifndef CYCLE_EXACT_ALARM
                while (CLK >= alarm_context_next_pending_clk(ALARM_CONTEXT)) {
                    alarm_context_dispatch(ALARM_CONTEXT, CLK);
                    CPU_DELAY_CLK
                }
#endif
            }
        }

        SET_LAST_ADDR(PC);
        fetch = imm_byte();

        if (fetch == 0x10 || fetch == 0x11) {
            opcode = fetch << 8;
            fetch = imm_byte();
            while (fetch == 0x10 || fetch == 0x11) {
                fetch = imm_byte();
            }
            opcode |= fetch;
        } else {
            opcode = fetch;
        }

        switch (opcode) {
            case 0x003a:	/* ABX */
#ifdef FULL6809
            case 0x103a:	/* ABX (UNDOC) */
            case 0x113a:	/* ABX (UNDOC) */
#endif
                abx();
                break;

            case 0x0089:	/* ADCA immediate */
#ifdef FULL6809
            case 0x1089:	/* ADCA immediate (UNDOC) */
            case 0x1189:	/* ADCA immediate (UNDOC) */
#endif
                A = adc(A, imm_byte(), 0, 0);
                break;

            case 0x0099:	/* ADCA direct */
#ifdef FULL6809
            case 0x1099:	/* ADCA direct (UNDOC) */
            case 0x1199:	/* ADCA direct (UNDOC) */
#endif
                direct();
                A = adc(A, RDMEM(ea), 1, 0);
                break;

            case 0x00b9:	/* ADCA extended */
#ifdef FULL6809
            case 0x10b9:	/* ADCA extended (UNDOC) */
            case 0x11b9:	/* ADCA extended (UNDOC) */
#endif
                extended();
                A = adc(A, RDMEM(ea), 1, 0);
                break;

            case 0x00a9:	/* ADCA indexed */
#ifdef FULL6809
            case 0x10a9:	/* ADCA indexed (UNDOC) */
            case 0x11a9:	/* ADCA indexed (UNDOC) */
#endif
                indexed();
                A = adc(A, RDMEM(ea), 1, 1);
                break;

            case 0x00c9:	/* ADCB immediate */
#ifdef FULL6809
            case 0x10c9:	/* ADCB immediate (UNDOC) */
            case 0x11c9:	/* ADCB immediate (UNDOC) */
#endif
                B = adc(B, imm_byte(), 0, 0);
                break;

            case 0x00d9:	/* ADCB direct */
#ifdef FULL6809
            case 0x10d9:	/* ADCB direct (UNDOC) */
            case 0x11d9:	/* ADCB direct (UNDOC) */
#endif
                direct();
                B = adc(B, RDMEM(ea), 1, 0);
                break;

            case 0x00f9:	/* ADCB extended */
#ifdef FULL6809
            case 0x10f9:	/* ADCB extended (UNDOC) */
            case 0x11f9:	/* ADCB extended (UNDOC) */
#endif
                extended();
                B = adc(B, RDMEM(ea), 1, 0);
                break;

            case 0x00e9:	/* ADCB indexed */
#ifdef FULL6809
            case 0x10e9:	/* ADCB indexed (UNDOC) */
            case 0x11e9:	/* ADCB indexed (UNDOC) */
#endif
                indexed();
                B = adc(B, RDMEM(ea), 1, 1);
                break;

#ifdef H6309
            case 0x1089:	/* ADCD immediate */
                D = adc16(D, imm_word(), 1, 0);
                break;

            case 0x1099:	/* ADCD direct */
                direct();
                D = adc16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10b9:	/* ADCD extended */
                extended();
                D = adc16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10a9:	/* ADCD indexed */
                indexed();
                D = adc16(D, RDMEM16(ea), 2, 1);
                break;

            case 0x1031:	/* ADCR post */
                post_byte = imm_byte();
                set_reg((BYTE)(post_byte & 0x0f), adc16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f)), 1, 1));
                break;
#endif

            case 0x008b:	/* ADDA immediate */
#ifdef FULL6809
            case 0x108b:	/* ADDA immediate (UNDOC) */
            case 0x118b:	/* ADDA immediate (UNDOC) */
#endif
                A = add(A, imm_byte(), 0, 0);
                break;

            case 0x009b:	/* ADDA direct */
#ifdef FULL6809
            case 0x109b:	/* ADDA direct (UNDOC) */
            case 0x119b:	/* ADDA direct (UNDOC) */
#endif
                direct();
                A = add(A, RDMEM(ea), 1, 0);
                break;

            case 0x00bb:	/* ADDA extended */
#ifdef FULL6809
            case 0x10bb:	/* ADDA extended (UNDOC) */
            case 0x11bb:	/* ADDA extended (UNDOC) */
#endif
                extended();
                A = add(A, RDMEM(ea), 1, 0);
                break;

            case 0x00ab:	/* ADDA indexed */
#ifdef FULL6809
            case 0x10ab:	/* ADDA indexed (UNDOC) */
            case 0x11ab:	/* ADDA indexed (UNDOC) */
#endif
                indexed();
                A = add(A, RDMEM(ea), 1, 1);
                break;

            case 0x00cb:	/* ADDB immediate */
#ifdef FULL6809
            case 0x10cb:	/* ADDB immediate (UNDOC) */
            case 0x11cb:	/* ADDB immediate (UNDOC) */
#endif
                B = add(B, imm_byte(), 0, 0);
                break;

            case 0x00db:	/* ADDB direct */
#ifdef FULL6809
            case 0x10db:	/* ADDB direct (UNDOC) */
            case 0x11db:	/* ADDB direct (UNDOC) */
#endif
                direct();
                B = add(B, RDMEM(ea), 1, 0);
                break;

            case 0x00fb:	/* ADDB extended */
#ifdef FULL6809
            case 0x10fb:	/* ADDB extended (UNDOC) */
            case 0x11fb:	/* ADDB extended (UNDOC) */
#endif
                extended();
                B = add(B, RDMEM(ea), 1, 0);
                break;

            case 0x00eb:	/* ADDB indexed */
#ifdef FULL6809
            case 0x10eb:	/* ADDB indexed (UNDOC) */
            case 0x11eb:	/* ADDB indexed (UNDOC) */
#endif
                indexed();
                B = add(B, RDMEM(ea), 1, 1);
                break;

            case 0x00c3:	/* ADDD immediate */
#ifdef FULL6809
            case 0x10c3:	/* ADDD immediate (UNDOC) */
            case 0x11c3:	/* ADDD immediate (UNDOC) */
#endif
                D = add16(D, imm_word(), 1, 0);
                break;

            case 0x00d3:	/* ADDD direct */
#ifdef FULL6809
            case 0x10d3:	/* ADDD direct (UNDOC) */
            case 0x11d3:	/* ADDD direct (UNDOC) */
#endif
                direct();
                D = add16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x00f3:	/* ADDD extended */
#ifdef FULL6809
            case 0x10f3:	/* ADDD extended (UNDOC) */
            case 0x11f3:	/* ADDD extended (UNDOC) */
#endif
                extended();
                D = add16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x00e3:	/* ADDD indexed */
#ifdef FULL6809
            case 0x10e3:	/* ADDD indexed (UNDOC) */
            case 0x11e3:	/* ADDD indexed (UNDOC) */
#endif
                indexed();
                D = add16(D, RDMEM16(ea), 2, 1);
                break;

#ifdef H6309
            case 0x118b:	/* ADDE immediate */
                E = add(E, imm_byte(), 0, 0);
                break;

            case 0x119b:	/* ADDE direct */
                direct();
                E = add(E, RDMEM(ea), 1, 0);
                break;

            case 0x11bb:	/* ADDE extended */
                extended();
                E = add(E, RDMEM(ea), 1, 0);
                break;

            case 0x11ab:	/* ADDE indexed */
                indexed();
                E = add(E, RDMEM(ea), 2, 2);
                break;

            case 0x11cb:	/* ADDF immediate */
                F = add(F, imm_byte(), 0, 0);
                break;

            case 0x11db:	/* ADDF direct */
                direct();
                F = add(F, RDMEM(ea), 1, 0);
                break;

            case 0x11fb:	/* ADDF extended */
                extended();
                F = add(F, RDMEM(ea), 1, 0);
                break;

            case 0x11eb:	/* ADDF indexed */
                indexed();
                F = add(F, RDMEM(ea), 2, 2);
                break;

            case 0x1030:	/* ADDR post */
                post_byte = imm_byte();
                set_reg((BYTE)(post_byte & 0x0f), add16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f)), 1, 1));
                break;

            case 0x108b:	/* ADDW immediate */
                W = add16(W, imm_word(), 1, 0);
                break;

            case 0x109b:	/* ADDW direct */
                direct();
                W = add16(W, RDMEM16(ea), 2, 0);
                break;

            case 0x10bb:	/* ADDW extended */
                extended();
                W = add16(W, RDMEM16(ea), 2, 0);
                break;

            case 0x10ab:	/* ADDW indexed */
                indexed();
                W = add16(W, RDMEM16(ea), 2, 1);
                break;

            case 0x0002:	/* AIM post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, and(RDMEM(ea), post_byte, 1, 1));
                break;

            case 0x0072:	/* AIM post, extended */
                post_byte = imm_byte();
                extended();
                WRMEM(ea, and(RDMEM(ea), post_byte, 1, 1));
                break;

            case 0x0062:	/* AIM post, indexed */
                post_byte = imm_byte();
                indexed();
                WRMEM(ea, and(RDMEM(ea), post_byte, 2, 2));
                break;
#endif

            case 0x0084:	/* ANDA immediate */
#ifdef FULL6809
            case 0x1084:	/* ANDA immediate (UNDOC) */
            case 0x1184:	/* ANDA immediate (UNDOC) */
#endif
                A = and(A, imm_byte(), 0, 0);
                break;

            case 0x0094:	/* ANDA direct */
#ifdef FULL6809
            case 0x1094:	/* ANDA direct (UNDOC) */
            case 0x1194:	/* ANDA direct (UNDOC) */
#endif
                direct();
                A = and(A, RDMEM(ea), 1, 0);
                break;

            case 0x00b4:	/* ANDA extended */
#ifdef FULL6809
            case 0x10b4:	/* ANDA extended (UNDOC) */
            case 0x11b4:	/* ANDA extended (UNDOC) */
#endif
                extended();
                A = and(A, RDMEM(ea), 1, 0);
                break;

            case 0x00a4:	/* ANDA indexed */
#ifdef FULL6809
            case 0x10a4:	/* ANDA indexed (UNDOC) */
            case 0x11a4:	/* ANDA indexed (UNDOC) */
#endif
                indexed();
                A = and(A, RDMEM(ea), 1, 1);
                break;

            case 0x00c4:	/* ANDB immediate */
#ifdef FULL6809
            case 0x10c4:	/* ANDB immediate (UNDOC) */
            case 0x11c4:	/* ANDB immediate (UNDOC) */
#endif
                B = and(B, imm_byte(), 0, 0);
                break;

            case 0x00d4:	/* ANDB direct */
#ifdef FULL6809
            case 0x10d4:	/* ANDB direct (UNDOC) */
            case 0x11d4:	/* ANDB direct (UNDOC) */
#endif
                direct();
                B = and(B, RDMEM(ea), 1, 0);
                break;

            case 0x00f4:	/* ANDB extended */
#ifdef FULL6809
            case 0x10f4:	/* ANDB extended (UNDOC) */
            case 0x11f4:	/* ANDB extended (UNDOC) */
#endif
                extended();
                B = and(B, RDMEM(ea), 1, 0);
                break;

            case 0x00e4:	/* ANDB indexed */
#ifdef FULL6809
            case 0x10e4:	/* ANDB indexed (UNDOC) */
            case 0x11e4:	/* ANDB indexed (UNDOC) */
#endif
                indexed();
                B = and(B, RDMEM(ea), 1, 1);
                break;

            case 0x001c:	/* ANDCC immediate */
#ifdef FULL6809
            case 0x101c:	/* ANDCC immediate (UNDOC) */
            case 0x111c:	/* ANDCC immediate (UNDOC) */
#endif
                andcc();
                break;

#ifdef FULL6809
            case 0x0038:	/* ANDCC immediate (+1 extra cycle) (UNDOC) */
            case 0x1038:	/* ANDCC immediate (+1 extra cycle) (UNDOC) */
            case 0x1138:	/* ANDCC immediate (+1 extra cycle) (UNDOC) */
                andcc();
                CLK++;
                break;
#endif

#ifdef H6309
            case 0x1084:	/* ANDD immediate */
                D = and16(D, imm_word(), 1, 0);
                break;

            case 0x1094:	/* ANDD direct */
                direct();
                D = and16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10b4:	/* ANDD extended */
                extended();
                D = and16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10a4:	/* ANDD indexed */
                indexed();
                D = and16(D, RDMEM16(ea), 2, 1);
                break;

            case 0x1034:	/* ANDR post */
                post_byte = imm_byte();
                set_reg((BYTE)(post_byte & 0x0f), and16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f)), 1, 1));
                break;
#endif

            case 0x0048:	/* ASLA/LSLA */
#ifdef FULL6809
            case 0x1048:	/* ASLA/LSLA (UNDOC) */
            case 0x1148:	/* ASLA/LSLA (UNDOC) */
#endif
                A = asl(A, 1, 0);
                break;

            case 0x0058:	/* ASLB/LSLB */
#ifdef FULL6809
            case 0x1058:	/* ASLB/LSLB (UNDOC) */
            case 0x1158:	/* ASLB/LSLB (UNDOC) */
#endif
                B = asl(B, 1, 0);
                break;

#ifdef H6309
            case 0x1048:	/* ASLD/LSLD */
                D = asl16(D, 1, 0);
                break;
#endif

            case 0x0008:	/* ASL/LSL direct */
#ifdef FULL6809
            case 0x1008:	/* ASL/LSL direct (UNDOC) */
            case 0x1108:	/* ASL/LSL direct (UNDOC) */
#endif
                direct();
                WRMEM(ea, asl(RDMEM(ea), 2, 1));
                break;

            case 0x0078:	/* ASL/LSL extended */
#ifdef FULL6809
            case 0x1078:	/* ASL/LSL extended (UNDOC) */
            case 0x1178:	/* ASL/LSL extended (UNDOC) */
#endif
                extended();
                WRMEM(ea, asl(RDMEM(ea), 2, 1));
                break;

            case 0x0068:	/* ASL/LSL indexed */
#ifdef FULL6809
            case 0x1068:	/* ASL/LSL indexed (UNDOC) */
            case 0x1168:	/* ASL/LSL indexed (UNDOC) */
#endif
                indexed();
                WRMEM(ea, asl(RDMEM(ea), 2, 2));
                break;

            case 0x0047:	/* ASRA */
#ifdef FULL6809
            case 0x1047:	/* ASRA (UNDOC) */
            case 0x1147:	/* ASRA (UNDOC) */
#endif
                A = asr(A, 1, 0);
                break;

            case 0x0057:	/* ASRB */
#ifdef FULL6809
            case 0x1057:	/* ASRB (UNDOC) */
            case 0x1157:	/* ASRB (UNDOC) */
#endif
                B = asr(B, 1, 0);
                break;

#ifdef H6309
            case 0x1047:	/* ASRD */
                D = asr16(D, 1, 0);
                break;
#endif

            case 0x0007:	/* ASR direct */
#ifdef FULL6809
            case 0x1007:	/* ASR direct (UNDOC) */
            case 0x1107:	/* ASR direct (UNDOC) */
#endif
                direct();
                WRMEM(ea, asr(RDMEM(ea), 2, 1));
                break;

            case 0x0077:	/* ASR extended */
#ifdef FULL6809
            case 0x1077:	/* ASR extended (UNDOC) */
            case 0x1177:	/* ASR extended (UNDOC) */
#endif
                extended();
                WRMEM(ea, asr(RDMEM(ea), 2, 1));
                break;

            case 0x0067:	/* ASR indexed */
#ifdef FULL6809
            case 0x1067:	/* ASR indexed (UNDOC) */
            case 0x1167:	/* ASR indexed (UNDOC) */
#endif
                indexed();
                WRMEM(ea, asr(RDMEM(ea), 2, 2));
                break;

#ifdef H6309
            case 0x1130:	/* BAND post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, band(post_byte, RDMEM(ea)));
                break;
#endif

#ifdef H6309
            case 0x1134:	/* BEOR post,direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, beor(post_byte, RDMEM(ea)));
                break;
#endif

            case 0x0027:	/* BEQ */
#ifdef FULL6809
            case 0x1127:	/* BEQ (UNDOC) */
#endif
                branch(cond_EQ());
                break;

            case 0x002c:	/* BGE */
#ifdef FULL6809
            case 0x112c:	/* BGE (UNDOC) */
#endif
                branch(cond_GE());
                break;

            case 0x002e:	/* BGT */
#ifdef FULL6809
            case 0x112e:	/* BGT (UNDOC) */
#endif
                branch(cond_GT());
                break;

            case 0x0022:	/* BHI */
#ifdef FULL6809
            case 0x1122:	/* BHI (UNDOC) */
#endif
                branch(cond_HI());
                break;

            case 0x0024:	/* BHS */
#ifdef FULL6809
            case 0x1124:	/* BHS (UNDOC) */
#endif
                branch(cond_HS());
                break;

#ifdef H6309
            case 0x1131:	/* BIAND post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, biand(post_byte, RDMEM(ea)));
                break;

            case 0x1135:	/* BIEOR post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, bieor(post_byte, RDMEM(ea)));
                break;

            case 0x1133:	/* BIOR post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, bior(post_byte, RDMEM(ea)));
                break;
#endif

            case 0x0085:	/* BITA immediate */
#ifdef FULL6809
            case 0x1085:	/* BITA immediate (UNDOC) */
            case 0x1185:	/* BITA immediate (UNDOC) */
#endif
                bit(A, imm_byte(), 0, 0);
                break;

            case 0x0095:	/* BITA direct */
#ifdef FULL6809
            case 0x1095:	/* BITA direct (UNDOC) */
            case 0x1195:	/* BITA direct (UNDOC) */
#endif
                direct();
                bit(A, RDMEM(ea), 1, 0);
                break;

            case 0x00b5:	/* BITA extended */
#ifdef FULL6809
            case 0x10b5:	/* BITA extended (UNDOC) */
            case 0x11b5:	/* BITA extended (UNDOC) */
#endif
                extended();
                bit(A, RDMEM(ea), 1, 0);
                break;

            case 0x00a5:	/* BITA indexed */
#ifdef FULL6809
            case 0x10a5:	/* BITA indexed (UNDOC) */
            case 0x11a5:	/* BITA indexed (UNDOC) */
#endif
                indexed();
                bit(A, RDMEM(ea), 1, 1);
                break;

            case 0x00c5:	/* BITB immediate */
#ifdef FULL6809
            case 0x10c5:	/* BITB immediate (UNDOC) */
            case 0x11c5:	/* BITB immediate (UNDOC) */
#endif
                bit(B, imm_byte(), 0, 0);
                break;

            case 0x00d5:	/* BITB direct */
#ifdef FULL6809
            case 0x10d5:	/* BITB direct (UNDOC) */
            case 0x11d5:	/* BITB direct (UNDOC) */
#endif
                direct();
                bit(B, RDMEM(ea), 1, 0);
                break;

            case 0x00f5:	/* BITB extended */
#ifdef FULL6809
            case 0x10f5:	/* BITB extended (UNDOC) */
            case 0x11f5:	/* BITB extended (UNDOC) */
#endif
                extended();
                bit(B, RDMEM(ea), 1, 0);
                break;

            case 0x00e5:	/* BITB indexed */
#ifdef FULL6809
            case 0x10e5:	/* BITB indexed (UNDOC) */
            case 0x11e5:	/* BITB indexed (UNDOC) */
#endif
                indexed();
                bit(B, RDMEM(ea), 1, 1);
                break;

#ifdef H6309
            case 0x1085:	/* BITD immediate */
                bit16(D, imm_word(), 1, 0);
                break;

            case 0x1095:	/* BITD direct */
                direct();
                bit16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10b5:	/* BITD extended */
                extended();
                bit16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10a5:	/* BITD indexed */
                indexed();
                bit16(D, RDMEM16(ea), 2, 1);
                break;
#endif

            case 0x002f:	/* BLE */
#ifdef FULL6809
            case 0x112f:	/* BLE (UNDOC) */
#endif
                branch(cond_LE());
                break;

            case 0x0025:	/* BLO */
#ifdef FULL6809
            case 0x1125:	/* BLO (UNDOC) */
#endif
                branch(cond_LO());
                break;

            case 0x0023:	/* BLS */
#ifdef FULL6809
            case 0x1123:	/* BLS (UNDOC) */
#endif
                branch(cond_LS());
                break;

            case 0x002d:	/* BLT */
#ifdef FULL6809
            case 0x112d:	/* BLT (UNDOC) */
#endif
                branch(cond_LT());
                break;

            case 0x002b:	/* BMI */
#ifdef FULL6809
            case 0x112b:	/* BMI (UNDOC) */
#endif
                branch(cond_MI());
                break;

            case 0x0026:	/* BNE */
#ifdef FULL6809
            case 0x1126:	/* BNE (UNDOC) */
#endif
                branch(cond_NE());
                break;

#ifdef H6309
            case 0x1132:	/* BOR post,direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, bor(post_byte, RDMEM(ea)));
                break;
#endif

            case 0x002a:	/* BPL */
#ifdef FULL6809
            case 0x112a:	/* BPL (UNDOC) */
#endif
                branch(cond_PL());
                break;

            case 0x0020:	/* BRA */
#ifdef FULL6809
            case 0x1120:	/* BRA (UNDOC) */
#endif
                bra();
                break;

#ifdef FULL6809
            case 0x1121:	/* BRN (UNDOC) */
#endif
                imm_byte();
                CLK++;
                break;

            case 0x008d:	/* BSR */
#ifdef FULL6809
            case 0x108d:	/* BSR (UNDOC) */
            case 0x118d:	/* BSR (UNDOC) */
#endif
                bsr();
                break;

            case 0x0028:	/* BVC */
#ifdef FULL6809
            case 0x1128:	/* BVC (UNDOC) */
#endif
                branch(cond_VC());
                break;

            case 0x0029:	/* BVS */
#ifdef FULL6809
            case 0x1129:	/* BVS (UNDOC) */
#endif
                branch(cond_VS());
                break;

            case 0x004f:	/* CLRA */
#ifdef FULL6809
            case 0x004e:	/* CLRA (UNDOC) */
            case 0x104e:	/* CLRA (UNDOC) */
            case 0x104f:	/* CLRA (UNDOC) */
            case 0x114e:	/* CLRA (UNDOC) */
            case 0x114f:	/* CLRA (UNDOC) */
#endif
                A = clr(A, 1, 0);
                break;

            case 0x005f:	/* CLRB */
#ifdef FULL6809
            case 0x005e:	/* CLRB (UNDOC) */
            case 0x105e:	/* CLRB (UNDOC) */
            case 0x105f:	/* CLRB (UNDOC) */
            case 0x115e:	/* CLRB (UNDOC) */
            case 0x115f:	/* CLRB (UNDOC) */
#endif
                B = clr(B, 1, 0);
                break;

#ifdef H6309
            case 0x104f:	/* CLRD */
                D = clr16(D, 1, 0);
                break;

            case 0x114f:	/* CLRE */
                E = clr(E, 1, 0);
                break;

            case 0x115f:	/* CLRF */
                F = clr(F, 1, 0);
                break;

            case 0x105f:	/* CLRW */
                W = clr16(W, 1, 0);
                break;
#endif

            case 0x000f:	/* CLR direct */
#ifdef FULL6809
            case 0x100f:	/* CLR direct (UNDOC) */
            case 0x110f:	/* CLR direct (UNDOC) */
#endif
                direct();
                WRMEM(ea, clr(RDMEM(ea), 2, 1));
                break;

            case 0x007f:	/* CLR extended */
#ifdef FULL6809
            case 0x107f:	/* CLR extended (UNDOC) */
            case 0x117f:	/* CLR extended (UNDOC) */
#endif
                extended();
                WRMEM(ea, clr(RDMEM(ea), 2, 1));
                break;

            case 0x006f:	/* CLR indexed */
#ifdef FULL6809
            case 0x106f:	/* CLR indexed (UNDOC) */
            case 0x116f:	/* CLR indexed (UNDOC) */
#endif
                indexed();
                WRMEM(ea, clr(RDMEM(ea), 2, 2));
                break;

            case 0x0081:	/* CMPA immediate */
#ifdef FULL6809
            case 0x1081:	/* CMPA immediate (UNDOC) */
            case 0x1181:	/* CMPA immediate (UNDOC) */
#endif
                cmp(A, imm_byte(), 0, 0);
                break;

            case 0x0091:	/* CMPA direct */
#ifdef FULL6809
            case 0x1091:	/* CMPA direct (UNDOC) */
            case 0x1191:	/* CMPA direct (UNDOC) */
#endif
                direct();
                cmp(A, RDMEM(ea), 1, 0);
                break;

            case 0x00b1:	/* CMPA extended */
#ifdef FULL6809
            case 0x10b1:	/* CMPA extended (UNDOC) */
            case 0x11b1:	/* CMPA extended (UNDOC) */
#endif
                extended();
                cmp(A, RDMEM(ea), 1, 0);
                break;

            case 0x00a1:	/* CMPA indexed */
#ifdef FULL6809
            case 0x10a1:	/* CMPA indexed (UNDOC) */
            case 0x11a1:	/* CMPA indexed (UNDOC) */
#endif
                indexed();
                cmp(A, RDMEM(ea), 1, 1);
                break;

            case 0x00c1:	/* CMPB immediate */
#ifdef FULL6809
            case 0x10c1:	/* CMPB immediate (UNDOC) */
            case 0x11c1:	/* CMPB immediate (UNDOC) */
#endif
                cmp(B, imm_byte(), 0, 0);
                break;

            case 0x00d1:	/* CMPB direct */
#ifdef FULL6809
            case 0x10d1:	/* CMPB direct (UNDOC) */
            case 0x11d1:	/* CMPB direct (UNDOC) */
#endif
                direct();
                cmp(B, RDMEM(ea), 1, 0);
                break;

            case 0x00f1:	/* CMPB extended */
#ifdef FULL6809
            case 0x10f1:	/* CMPB extended (UNDOC) */
            case 0x11f1:	/* CMPB extended (UNDOC) */
#endif
                extended();
                cmp(B, RDMEM(ea), 1, 0);
                break;

            case 0x00e1:	/* CMPB indexed */
#ifdef FULL6809
            case 0x10e1:	/* CMPB indexed (UNDOC) */
            case 0x11e1:	/* CMPB indexed (UNDOC) */
#endif
                indexed();
                cmp(B, RDMEM(ea), 1, 1);
                break;

            case 0x1083:	/* CMPD immediate */
                cmp16(D, imm_word(), 1, 0);
                break;

            case 0x1093:	/* CMPD direct */
                direct();
                cmp16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10b3:	/* CMPD extended */
                extended();
                cmp16(D, RDMEM16(ea), 2, 0);
                break;

            case 0x10a3:	/* CMPD indexed */
                indexed();
                cmp16(D, RDMEM16(ea), 2, 1);
                break;

#ifdef H6309
            case 0x1181:	/* CMPE immediate */
                cmp(E, imm_byte(), 0, 0);
                break;

            case 0x1191:	/* CMPE direct */
                direct();
                cmp(E, RDMEM(ea), 1, 0);
                break;

            case 0x11b1:	/* CMPE extended */
                extended();
                cmp(E, RDMEM(ea), 1, 0);
                break;

            case 0x11a1:	/* CMPE indexed */
                indexed();
                cmp(E, RDMEM(ea), 1, 1);
                break;

            case 0x11c1:	/* CMPF immediate */
                cmp(F, imm_byte(), 0, 0);
                break;

            case 0x11d1:	/* CMPF direct */
                direct();
                cmp(F, RDMEM(ea), 1, 0);
                break;

            case 0x11f1:	/* CMPF extended */
                extended();
                cmp(F, RDMEM(ea), 1, 0);
                break;

            case 0x11e1:	/* CMPF indexed */
                indexed();
                cmp(F, RDMEM(ea), 1, 1);
                break;
#endif

            case 0x118c:	/* CMPS immediate */
                cmp16(S, imm_word(), 1, 0);
                break;

            case 0x119c:	/* CMPS direct */
                direct();
                cmp16(S, RDMEM16(ea), 2, 0);
                break;

            case 0x11bc:	/* CMPS extended */
                extended();
                cmp16(S, RDMEM16(ea), 2, 0);
                break;

            case 0x11ac:	/* CMPS indexed */
                indexed();
                cmp16(S, RDMEM16(ea), 2, 1);
                break;

            case 0x1183:	/* CMPU immediate */
                cmp16(U, imm_word(), 1, 0);
                break;

            case 0x1193:	/* CMPU direct */
                direct();
                cmp16(U, RDMEM16(ea), 2, 0);
                break;

            case 0x11b3:	/* CMPU extended */
                extended();
                cmp16(U, RDMEM16(ea), 2, 0);
                break;

            case 0x11a3:	/* CMPU indexed */
                indexed();
                cmp16(U, RDMEM16(ea), 2, 1);
                break;

#ifdef H6309
            case 0x1081:	/* CMPW immediate */
                cmp16(W, imm_word(), 1, 0);
                break;

            case 0x1091:	/* CMPW direct */
                direct();
                cmp16(W, RDMEM16(ea), 2, 0);
                break;

            case 0x10b1:	/* CMPW extended */
                extended();
                cmp16(W, RDMEM16(ea), 2, 0);
                break;

            case 0x10a1:	/* CMPW indexed */
                indexed();
                cmp16(W, RDMEM16(ea), 2, 1);
                break;
#endif

            case 0x008c:	/* CMPX immediate */
                cmp16(X, imm_word(), 1, 0);
                break;

            case 0x009c:	/* CMPX direct */
                direct();
                cmp16(X, RDMEM16(ea), 2, 0);
                break;

            case 0x00bc:	/* CMPX extended */
                extended();
                cmp16(X, RDMEM16(ea), 2, 0);
                break;

            case 0x00ac:	/* CMPX indexed */
                indexed();
                cmp16(X, RDMEM16(ea), 2, 1);
                break;

            case 0x108c:	/* CMPY immediate */
                cmp16(Y, imm_word(), 1, 0);
                break;

            case 0x109c:	/* CMPY direct */
                direct();
                cmp16(Y, RDMEM16(ea), 2, 0);
                break;

            case 0x10bc:	/* CMPY extended */
                extended();
                cmp16(Y, RDMEM16(ea), 2, 0);
                break;

            case 0x10ac:	/* CMPY indexed */
                indexed();
                cmp16(Y, RDMEM16(ea), 2, 1);
                break;

            case 0x0043:	/* COMA */
#ifdef FULL6809
            case 0x1043:	/* COMA (UNDOC) */
            case 0x1143:	/* COMA (UNDOC) */
#endif
                A = com(A, 1, 0);
                break;

            case 0x0042:	/* COMA/NEGA (UNDOC) */
#ifdef FULL6809
            case 0x1042:	/* COMA/NEGA (UNDOC) */
            case 0x1142:	/* COMA/NEGA (UNDOC) */
                if (C) {
                    A = com(A, 1, 0);
                } else {
                    A = neg(A, 1, 0);
                }
                break;
#endif

            case 0x0053:	/* COMB */
#ifdef FULL6809
            case 0x1053:	/* COMB (UNDOC) */
            case 0x1153:	/* COMB (UNDOC) */
#endif
                B = com(B, 1, 0);
                break;

#ifdef FULL6809
            case 0x0052:	/* COMB/NEGB (UNDOC) */
            case 0x1052:	/* COMB/NEGB (UNDOC) */
            case 0x1152:	/* COMB/NEGB (UNDOC) */
                if (C) {
                    B = com(B, 1, 0);
                } else {
                    B = neg(B, 1, 0);
                }
                break;
#endif

#ifdef H6309
            case 0x1043:	/* COMD */
                D = com16(D, 1, 0);
                break;
#endif

#ifdef H6309
            case 0x1143:	/* COME */
                E = com(E, 1, 0);
                break;

            case 0x1153:	/* COMF */
                F = com(F, 1, 0);
                break;

            case 0x1053:	/* COMW */
                W = com16(W, 1, 0);
                break;
#endif

            case 0x0003:	/* COM direct */
#ifdef H6309
            case 0x1003:	/* COM direct (UNDOC) */
            case 0x1103:	/* COM direct (UNDOC) */
#endif
                direct();
                WRMEM(ea, com(RDMEM(ea), 2, 1));
                break;

#ifdef FULL6809
            case 0x0002:	/* COM/NEG direct (UNDOC) */
            case 0x1002:	/* NEG/COM direct (UNDOC) */
            case 0x1102:	/* NEG/COM direct (UNDOC) */
                direct();
                if (C) {
                    WRMEM(ea, com(RDMEM(ea), 2, 1));
                } else {
                    WRMEM(ea, neg(RDMEM(ea), 2, 1));
                }
                break;
#endif

            case 0x0073:	/* COM extended */
#ifdef FULL6809
            case 0x1073:	/* COM extended (UNDOC) */
            case 0x1173:	/* COM extended (UNDOC) */
#endif
                extended();
                WRMEM(ea, com(RDMEM(ea), 2, 1));
                break;

#ifdef FULL6809
            case 0x0072:	/* COM/NEG extended (UNDOC) */
            case 0x1072:	/* COM/NEG extended (UNDOC) */
            case 0x1172:	/* COM/NEG extended (UNDOC) */
                extended();
                if (C) {
                    WRMEM(ea, com(RDMEM(ea), 2, 1));
                } else {
                    WRMEM(ea, neg(RDMEM(ea), 2, 1));
                }
                break;
#endif

            case 0x0063:	/* COM indexed */
#ifdef FULL6809
            case 0x1063:	/* COM indexed (UNDOC) */
            case 0x1163:	/* COM indexed (UNDOC) */
#endif
                indexed();
                WRMEM(ea, com(RDMEM(ea), 1, 1));
                break;

#ifdef FULL6809
            case 0x0062:	/* COM/NEG indexed (UNDOC) */
            case 0x1062:	/* COM/NEG indexed (UNDOC) */
            case 0x1162:	/* COM/NEG indexed (UNDOC) */
                indexed();
                if (C) {
                    WRMEM(ea, com(RDMEM(ea), 1, 1));
                } else {
                    WRMEM(ea, neg(RDMEM(ea), 1, 1));
                }
                break;
#endif

            case 0x003c:	/* CWAI */
#ifdef FULL6809
            case 0x103c:	/* CWAI (UNDOC) */
            case 0x113c:	/* CWAI (UNDOC) */
#endif
                cwai();
                break;

            case 0x0019:	/* DAA */
#ifdef FULL6809
            case 0x1019:	/* DAA (UNDOC) */
            case 0x1119:	/* DAA (UNDOC) */
#endif
                daa();
                break;

            case 0x004a:	/* DECA */
#ifdef FULL6809
            case 0x004b:	/* DECA (UNDOC) */
            case 0x104a:	/* DECA (UNDOC) */
            case 0x104b:	/* DECA (UNDOC) */
            case 0x114a:	/* DECA (UNDOC) */
            case 0x114b:	/* DECA (UNDOC) */
#endif
                A = dec(A, 1, 0);
                break;

            case 0x005a:	/* DECB */
#ifdef FULL6809
            case 0x005b:	/* DECB (UNDOC) */
            case 0x105a:	/* DECB (UNDOC) */
            case 0x105b:	/* DECB (UNDOC) */
            case 0x115a:	/* DECB (UNDOC) */
            case 0x115b:	/* DECB (UNDOC) */
#endif
                B = dec(B, 1, 0);
                break;

#ifdef H6309
            case 0x104a:	/* DECD */
                D = dec16(D, 1, 0);
                break;

            case 0x114a:	/* DECE */
                E = dec(E, 1, 0);
                break;

            case 0x115a:	/* DECF */
                F = dec(F, 1, 0);
                break;

            case 0x105a:	/* DECW */
                W = dec16(W, 1, 0);
                break;
#endif

            case 0x000a:	/* DEC direct */
#ifdef FULL6809
            case 0x000b:	/* DEC direct (UNDOC) */
            case 0x100a:	/* DEC direct (UNDOC) */
            case 0x100b:	/* DEC direct (UNDOC) */
            case 0x110a:	/* DEC direct (UNDOC) */
            case 0x110b:	/* DEC direct (UNDOC) */
#endif
                direct();
                WRMEM(ea, dec(RDMEM(ea), 2, 1));
                break;

            case 0x007a:	/* DEC extended */
#ifdef FULL6809
            case 0x007b:	/* DEC extended (UNDOC) */
            case 0x107a:	/* DEC extended (UNDOC) */
            case 0x107b:	/* DEC extended (UNDOC) */
            case 0x117a:	/* DEC extended (UNDOC) */
            case 0x117b:	/* DEC extended (UNDOC) */
#endif
                extended();
                WRMEM(ea, dec(RDMEM(ea), 2, 1));
                break;

            case 0x006a:	/* DEC indexed */
#ifdef FULL6809
            case 0x006b:	/* DEC indexed (UNDOC) */
            case 0x106a:	/* DEC indexed (UNDOC) */
            case 0x106b:	/* DEC indexed (UNDOC) */
            case 0x116a:	/* DEC indexed (UNDOC) */
            case 0x116b:	/* DEC indexed (UNDOC) */
#endif
                indexed();
                WRMEM(ea, dec(RDMEM(ea), 2, 2));
                break;

#ifdef H6309
            case 0x118d:	/* DIVD immediate */
                divd(imm_byte(), 22, 22);
                break;

            case 0x119d:	/* DIVD direct */
                direct();
                divd(RDMEM(ea), 23, 22);
                break;

            case 0x11bd:	/* DIVD extended */
                extended();
                divd(RDMEM(ea), 23, 22);
                break;

            case 0x11ad:	/* DIVD indexed */
                indexed();
                divd(RDMEM(ea), 23, 23);
                break;

            case 0x118e: /* DIVQ immediate */
                divq(imm_word(), 30, 30);
                break;

            case 0x119e:	/* DIVQ direct */
                direct();
                divq(RDMEM16(ea), 31, 30);
                break;

            case 0x11be:	/* DIVQ extended */
                extended();
                divq(RDMEM16(ea), 31, 30);
                break;

            case 0x11ae:	/* DIVQ indexed */
                indexed();
                divq(RDMEM16(ea), 31, 31);
                break;

            case 0x0005:	/* EIM post, direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, eor(RDMEM(ea), post_byte, 1, 1));
                break;

            case 0x0075:	/* EIM extended */
                post_byte = imm_byte();
                extended();
                WRMEM(ea, eor(RDMEM(ea), post_byte, 1, 1));
                break;

            case 0x0065:	/* EIM indexed */
                post_byte = imm_byte();
                indexed();
                WRMEM(ea, eor(RDMEM(ea), post_byte, 2, 2));
                break;
#endif

            case 0x0088:	/* EORA immediate */
#ifdef FULL6809
            case 0x1088:	/* EORA immediate (UNDOC) */
            case 0x1188:	/* EORA immediate (UNDOC) */
#endif
                A = eor(A, imm_byte(), 0, 0);
                break;

            case 0x0098:	/* EORA direct */
#ifdef FULL6809
            case 0x1098:	/* EORA direct (UNDOC) */
            case 0x1198:	/* EORA direct (UNDOC) */
#endif
                direct();
                A = eor(A, RDMEM(ea), 1, 0);
                break;

            case 0x00b8:	/* EORA extended */
#ifdef FULL6809
            case 0x10b8:	/* EORA extended (UNDOC) */
            case 0x11b8:	/* EORA extended (UNDOC) */
#endif
                extended();
                A = eor(A, RDMEM(ea), 1, 0);
                break;

            case 0x00a8:	/* EORA indexed */
#ifdef FULL6809
            case 0x10a8:	/* EORA indexed (UNDOC) */
            case 0x11a8:	/* EORA indexed (UNDOC) */
#endif
                indexed();
                A = eor(A, RDMEM(ea), 1, 1);
                break;

            case 0x00c8:	/* EORB immediate */
#ifdef FULL6809
            case 0x10c8:	/* EORB immediate (UNDOC) */
            case 0x11c8:	/* EORB immediate (UNDOC) */
#endif
                B = eor(B, imm_byte(), 0, 0);
                break;

        }


--------------------------------------------------------------------------------------

#if 0
        switch (opcode) {
            case 0x0000:	/* NEG direct */
                direct();
                WRMEM(ea, neg_new(RDMEM(ea), 2, 1));
                break;
#ifdef FULL6809
            case 0x0001:	/* NEG direct (UNDOC) */
                direct();
                WRMEM(ea, neg_new(RDMEM(ea), 2 , 2));
                break;
#endif
#ifdef H6309
            case 0x0001:	/* OIM post,direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, or_new(RDMEM(ea), post_byte, 1, 1));
                break;
#endif
            case 0x0004:	/* LSR direct */
                direct();
                WRMEM(ea, lsr_new(RDMEM(ea), 2, 1));
                break;
#ifdef FULL6809
            case 0x0005:	/* LSR direct (UNDOC) */
                direct();
                WRMEM(ea, lsr_new(RDMEM(ea), 2, 2));
                break;
#endif
            case 0x0006:	/* ROR direct */
                direct();
                WRMEM(ea, ror_new(RDMEM(ea), 2, 1));
                break;
            case 0x0009:	/* ROL direct */
                direct();
                WRMEM(ea, rol_new(RDMEM(ea), 2, 1));
                break;
#ifdef H6309
            case 0x000b:	/* TIM post,direct */
                post_byte = imm_byte();
                direct();
                WRMEM(ea, tim(post_byte, 1));
                break;
#endif
            case 0x000c:	/* INC direct */
                direct();
                WRMEM(ea, inc_new(RDMEM(ea), 2, 1));
                break;
            case 0x000d:	/* TST direct */
                direct();
                tst_new(RDMEM(ea), 3, 1);
                break;
            case 0x000e:	/* JMP direct */
                direct();
                CLK_ADD(1, 0);
                PC = ea;
                break;
            case 0x10:
                {
                    page_10:
                        opcode = imm_byte();
                        switch (opcode) {
#ifdef FULL6809
                            case 0x1000:	/* NEG direct (UNDOC) */
                            case 0x1001:	/* NEG direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x1004:	/* LSR direct (UNDOC) */
                            case 0x1005:	/* LSR direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x1006:	/* ROR direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x1009:	/* ROL direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x100c:	/* INC direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x100d:	/* TST direct (UNDOC) */
                                direct();
                                CLK += 4;
                                tst(RDMEM(ea));
                                break;
                            case 0x100e:	/* JMP direct (UNDOC) */
                                direct();
                                CLK += 3;
                                PC = ea;
                                break;
#endif
                            case 0x10:	/* ignore further prefix bytes (UNDOC) */
                            case 0x11:	/* ignore further prefix bytes (UNDOC) */
                                goto page_10;
#ifdef FULL6809
                            case 0x12:	/* NOP (UNDOC) */
                                nop();
                                break;
                            case 0x13:	/* SYNC (UNDOC) */
                                sync();
                                break;
                            case 0x14:	/* HCF (UNDOC) */
                            case 0x15:	/* HCF (UNDOC) */
                                hcf();
                                break;
                            case 0x16:	/* LBRA (UNDOC) */
                                long_bra();
                                CLK += 5;
                                break;
                            case 0x17:	/* LBSR (UNDOC) */
                                long_bsr();
                                break;
                            case 0x18:	/* CCRS (UNDOC) */
                                ccrs();
                                break;
                            case 0x1a:	/* ORCC immediate (UNDOC) */
                                orcc();
                                break;
                            case 0x1b:	/* NOP (UNDOC) */
                                nop();
                                break;
                            case 0x1d:	/* SEX (UNDOC) */
                                sex();
                                break;
                            case 0x1e:	/* EXG post (UNDOC) */
                                exg();
                                break;
                            case 0x1f:	/* TFR post (UNDOC) */
                                tfr();
                                break;
                            case 0x20:	/* LBRA (UNDOC) */
                                long_bra();
                                CLK += 5;
                                break;
#endif
                            case 0x21:	/* LBRN */
                                CLK += 5;
                                PC += 2;
                                break;
                            case 0x22:	/* LBHI */
                                long_branch(cond_HI());
                                break;
                            case 0x23:	/* LBLS */
                                long_branch(cond_LS());
                                break;
                            case 0x24:	/* LBCC/LBHS */
                                long_branch(cond_HS());
                                break;
                            case 0x25:	/* LBCS/LBLO */
                                long_branch(cond_LO());
                                break;
                            case 0x26:	/* LBNE */
                                long_branch(cond_NE());
                                break;
                            case 0x27:	/* LBEQ */
                                long_branch(cond_EQ());
                                break;
                            case 0x28:	/* LBVC */
                                long_branch(cond_VC());
                                break;
                            case 0x29:	/* LBVS */
                                long_branch(cond_VS());
                                break;
                            case 0x2a:	/* LBPL */
                                long_branch(cond_PL());
                                break;
                            case 0x2b:	/* LBMI */
                                long_branch(cond_MI());
                                break;
                            case 0x2c:	/* LBGE */
                                long_branch(cond_GE());
                                break;
                            case 0x2d:	/* LBLT */
                                long_branch(cond_LT());
                                break;
                            case 0x2e:	/* LBGT */
                                long_branch(cond_GT());
                                break;
                            case 0x2f:	/* LBLE */
                                long_branch(cond_LE());
                                break;
#ifdef FULL6809
                            case 0x30:	/* LEAX indexed (UNDOC) */
                                indexed();
                                Z = X = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x31:	/* LEAY indexed (UNDOC) */
                                indexed();
                                Z = Y = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x32:	/* LEAS indexed (UNDOC) */
                                indexed();
                                S = ea;
                                break;
#endif
#ifdef H6309
                            case 0x32:	/* SUBR post */
                                post_byte = imm_byte();
                                set_reg((BYTE)(post_byte & 0x0f), sub16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f))));
                                /* TODO: cycle count */
                                break;
#endif
#ifdef FULL6809
                            case 0x33:	/* LEAU indexed (UNDOC) */
                                indexed();
                                U = ea;
                                break;
#endif
#ifdef H6309
                            case 0x33:	/* SBCR post */
                                post_byte = imm_byte();
                                set_reg((BYTE)(post_byte & 0x0f), sbc16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f))));
                                /* TODO: cycle count */
                                break;
#endif
#ifdef FULL6809
                            case 0x34:	/* PSHS post (UNDOC) */
                                pshs();
                                break;
#endif
#ifdef FULL6809
                            case 0x35:	/* PULS post (UNDOC) */
                                puls();
                                break;
#endif
#ifdef H6309
                            case 0x35:	/* ORR post */
                                post_byte = imm_byte();
                                set_reg((BYTE)(post_byte & 0x0f), or16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f))));
                                /* TODO: cycle count */
                                break;
#endif
#ifdef FULL6809
                            case 0x36:	/* PSHU post (UNDOC) */
                                pshu();
                                break;
#endif
#ifdef H6309
                            case 0x36:	/* EORR post */
                                post_byte = imm_byte();
                                set_reg((BYTE)(post_byte & 0x0f), eor16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f))));
                                /* TODO: cycle count */
                                break;
#endif
#ifdef FULL6809
                            case 0x37:	/* PULU post (UNDOC) */
                                pulu();
                                break;
#endif
#ifdef H6309
                            case 0x37:	/* CMPR R,R */
                                post_byte = imm_byte();
                                cmp16(get_reg((BYTE)(post_byte >> 4)), get_reg((BYTE)(post_byte & 0x0f)));
                                /* TODO: cycle count */
                                break;
#endif
#ifdef H6309
                            case 0x38:	/* PSHSW */
                                pshsw();
                                break;
#endif
#ifdef FULL6809
                            case 0x39:	/* RTS (UNDOC) */
                                rts();
                                break;
#endif
#ifdef H6309
                            case 0x39:	/* PULSW */
                                pulsw();
                                break;
#endif
#ifdef H6309
                            case 0x3a:	/* PSHUW */
                                pshuw();
                                break;
#endif
#ifdef FULL6809
                            case 0x3b:	/* RTI (UNDOC) */
                                rti();
                                break;
#endif
#ifdef H6309
                            case 0x3b:	/* PULUW */
                                puluw();
                                break;
#endif
#ifdef FULL6809
                            case 0x3d:	/* MUL (UNDOC) */
                                mul();
                                break;
                            case 0x3e:	/* SWIRES (UNDOC) */
                                swires();
                                break;
#endif
                            case 0x3f:	/* SWI2 */
                                swi2();
                                break;
#ifdef FULL6809
                            case 0x40:	/* NEGA (UNDOC) */
                                A = neg(A);
                                break;
#endif
#ifdef H6309
                            case 0x40:	/* NEGD */
                                D = neg16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x41:	/* NEGA (UNDOC) */
                                A = neg(A);
                                break;
#endif
#ifdef FULL6809
                            case 0x44:	/* LSRA (UNDOC) */
                                A = lsr(A);
                                break;
#endif
#ifdef H6309
                            case 0x44:	/* LSRD */
                                D = lsr16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x45:	/* LSRA (UNDOC) */
                                A = lsr(A);
                                break;
                            case 0x46:	/* RORA (UNDOC) */
                                A = ror(A);
                                break;
#endif
#ifdef H6309
                            case 0x46:	/* RORD */
                                D = ror16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x49:	/* ROLA (UNDOC) */
                                A = rol(A);
                                break;
#endif
#ifdef H6309
                            case 0x49:	/* ROLD */
                                D = rol16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x4c:	/* INCA (UNDOC) */
                                A = inc(A);
                                break;
#endif
#ifdef H6309
                            case 0x4c:	/* INCD */
                                D = inc16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x4d:	/* TSTA (UNDOC) */
                                tst(A);
                                break;
#endif
#ifdef H6309
                            case 0x4d:	/* TSTD */
                                tst16(D);
                                break;
#endif
#ifdef FULL6809
                            case 0x50:	/* NEGB (UNDOC) */
                            case 0x51:	/* NEGB (UNDOC) */
                                B = neg(B);
                                break;
#endif
#ifdef FULL6809
                            case 0x54:	/* LSRB (UNDOC) */
                                B = lsr(B);
                                break;
#endif
#ifdef H6309
                            case 0x54:	/* LSRW */
                                W = lsr16(W);
                                break;
#endif
#ifdef FULL6809
                            case 0x55:	/* LSRB (UNDOC) */
                                B = lsr(B);
                                break;
                            case 0x56:	/* RORB (UNDOC) */
                                B = ror(B);
                                break;
#endif
#ifdef H6309
                            case 0x56:	/* RORW */
                                W = ror16(W);
                                break;
#endif
#ifdef FULL6809
                            case 0x59:	/* ROLB (UNDOC) */
                                B = rol(B);
                                break;
#endif
#ifdef H6309
                            case 0x59:	/* ROLW */
                                W = rol16(W);
                                break;
#endif
#ifdef FULL6809
                            case 0x5c:	/* INCB (UNDOC) */
                                B = inc(B);
                                break;
#endif
#ifdef H6309
                            case 0x5c:	/* INCW */
                                W = inc16(W);
                                break;
#endif
#ifdef FULL6809
                            case 0x5d:	/* TSTB (UNDOC) */
                                tst(B);
                                break;
#endif
#ifdef H6309
                            case 0x5d:	/* TSTW */
                                tst16(W);
                                break;
#endif
#ifdef FULL6809
                            case 0x60:	/* NEG indexed (UNDOC) */
                            case 0x61:	/* NEG indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x64:	/* LSR indexed (UNDOC) */
                            case 0x65:	/* LSR indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x66:	/* ROR indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x69:	/* ROL indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x6c:	/* INC indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x6d:	/* TST indexed (UNDOC) */
                                indexed();
                                tst(RDMEM(ea));
                                break;
                            case 0x6e:	/* JMP indexed (UNDOC) */
                                indexed();
                                CLK++;
                                PC = ea;
                                break;
                            case 0x70:	/* NEG extended (UNDOC) */
                            case 0x71:	/* NEG extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x74:	/* LSR extended (UNDOC) */
                            case 0x75:	/* LSR extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x76:	/* ROR extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x79:	/* ROL extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x7c:	/* INC extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x7d:	/* TST extended (UNDOC) */
                                extended();
                                CLK += 5;
                                tst(RDMEM(ea));
                                break;
                            case 0x7e:	/* JMP extended (UNDOC) */
                                extended();
                                CLK += 4;
                                PC = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x80:	/* SUBA immediate (UNDOC) */
                                CLK += 2;
                                A = sub(A, imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x80:	/* SUBW immediate */
                                /* TODO: cycle count */
                                W = sub16(W, imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x82:	/* SBCA immediate (UNDOC) */
                                CLK += 2;
                                A = sbc(A, imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x82:	/* SBCD immediate */
                                /* TODO: cycle count */
                                D = sbc16(D, imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x86:	/* LDA immediate (UNDOC) */
                                CLK += 2;
                                A = ld(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x86:	/* LDW immediate */
                                /* TODO: cycle count */
                                W = ld16(imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x87:	/* SCC immediate (UNDOC) */
                                /* TODO: cycle count */
                                scc(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x88:	/* EORD immediate */
                                /* TODO: cycle count */
                                D = eor16(D, imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x8a:	/* ORA immediate (UNDOC) */
                                CLK += 2;
                                A = or(A, imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x8a:	/* ORD immediate */
                                /* TODO: cycle count */
                                D = or16(D, imm_word());
                                break;
#endif
                            case 0x8e:	/* LDY immediate (UNDOC) */
                                CLK += 4;
                                Y = ld16(imm_word());
                                break;
#ifdef FULL6809
                            case 0x8f:	/* STX immediate (UNDOC) */
                                /* TODO: cycle count */
                                st_imm(X);
                                break;
                            case 0x90:	/* SUBA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x90:	/* SUBW direct */
                                direct();
                                /* TODO: cycle count */
                                W = sub16(W, RDMEM16(ea));                             
                                break;
#endif
#ifdef FULL6809
                            case 0x92:	/* SBCA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x92:	/* SBCD direct */
                                direct();
                                /* TODO: cycle count */
                                D = sbc16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x96:	/* LDA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x96:	/* LDW direct */
                                direct();
                                /* TODO: cycle count */
                                W = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x97:	/* STA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0x97:	/* STW direct */
                                direct();
                                /* TODO: cycle count */
                                st16(W);
                                break;
#endif
#ifdef H6309
                            case 0x98:	/* EORD direct */
                                direct();
                                /* TODO: cycle count */
                                D = eor16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x9a:	/* ORA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x9a:	/* ORD direct */
                                direct();
                                /* TODO: cycle count */
                                D = or16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x9d:	/* JSR direct (undoc) */
                                direct();
                                CLK += 7;
                                jsr();
                                break;
#endif
                            case 0x9e:	/* LDY direct */
                                direct();
                                CLK += 5;
                                Y = ld16(RDMEM16(ea));
                                break;
                            case 0x9f:	/* STY direct */
                                direct();
                                CLK += 5;
                                st16(Y);
                                break;
#ifdef FULL6809
                            case 0xa0:	/* SUBA indexed (UNDOC) */
                                indexed();
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xa0:	/* SUBW indexed */
                                indexed();
                                W = sub16(W, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa2:	/* SBCA indexed (UNDOC) */
                                indexed();
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xa2:	/* SBCD indexed */
                                indexed();
                                D = sbc16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa6:	/* LDA indexed (UNDOC) */
                                indexed();
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xa6:	/* LDW indexed */
                                indexed();
                                W = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa7:	/* STA indexed (UNDOC) */
                                indexed();
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0xa7:	/* STW indexed */
                                indexed();
                                st16(W);
                                break;
#endif
#ifdef H6309
                            case 0xa8:	/* EORD indexed */
                                indexed();
                                D = eor16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xaa:	/* ORA indexed (UNDOC) */
                                indexed();
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xaa:	/* ORD indexed */ 
                                indexed();
                                D = or16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xad:	/* JSR indexed (UNDOC) */
                                indexed();
                                CLK += 3;
                                jsr();
                                break;
#endif
                            case 0xae:	/* LDY indexed */
                                CLK++;
                                indexed();
                                Y = ld16(RDMEM16(ea));
                                break;
                            case 0xaf:	/* STY indexed */
                                CLK++;
                                indexed();
                                st16(Y);
                                break;
#ifdef FULL6809
                            case 0xb0:	/* SUBA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xb0:	/* SUBW extended */
                                extended();
                                /* TODO: cycle count */
                                W = sub16(W, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb2:	/* SBCA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xb2:	/* SBCD extended */
                                extended();
                                /* TODO: cycle count */
                                D = sbc16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb6:	/* LDA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xb6:	/* LDW extended */
                                extended();
                                /* TODO: cycle count */
                                W = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb7:	/* STA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0xb7:	/* STW extended */
                                extended();
                                /* TODO: cycle count */
                                st16(W);
                                break;
#endif
#ifdef H6309
                            case 0xb8:	/* EORD extended */
                                extended();
                                /* TODO: cycle count */
                                D = eor16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xba:	/* ORA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xba:	/* ORD extended */
                                extended();
                                /* TODO: cycle count */
                                D = or16(D, RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xbd:	/* JSR extended (UNDOC) */
                                extended();
                                CLK += 8;
                                jsr();
                                break;
#endif
                            case 0xbe:	/* LDY extended */
                                extended();
                                CLK += 6;
                                Y = ld16(RDMEM16(ea));
                                break;
                            case 0xbf:	/* STY extended */
                                extended();
                                CLK += 6;
                                st16(Y);
                                break;
#ifdef FULL6809
                            case 0xc0:	/* SUBB immediate (UNDOC) */
                                CLK += 2;
                                B = sub(B, imm_byte());
                                break;
                            case 0xc2:	/* SBCB immediate (UNDOC) */
                                CLK += 2;
                                B = sbc(B, imm_byte());
                                break;
                            case 0xc6:	/* LDB immediate (UNDOC) */
                                CLK += 2;
                                B = ld(imm_byte());
                                break;
                            case 0xc7:	/* SCC immediate (UNDOC) */
                                /* TODO: cycle count */
                                scc(imm_byte());
                                break;
                            case 0xca:	/* ORB immediate (UNDOC) */
                                CLK += 2;
                                B = or(B, imm_byte());
                                break;
                            case 0xcc:	/* LDD immediate (UNDOC) */
                                CLK += 3;
                                D = ld16(imm_word());
                                break;
                            case 0xcd:	/* HCF (UNDOC) */
                                hcf();
                                break;
#endif
                            case 0xce:	/* LDS immediate */
                                CLK += 4;
                                S = ld16(imm_word());
                                break;
#ifdef FULL6809
                            case 0xcf:	/* STU immediate (UNDOC) */
                                /* TODO: cycle count */
                                st_imm(U);
                            case 0xd0:	/* SUBB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = sub(B, RDMEM(ea));
                                break;
                            case 0xd2:	/* SBCB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xd6:	/* LDB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = ld(RDMEM(ea));
                                break;
                            case 0xd7:	/* STB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st(B);
                                break;
                            case 0xd8:	/* EORB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xda:	/* ORB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = or(B, RDMEM(ea));
                                break;
                            case 0xdc:	/* LDD direct (UNDOC) */
                                direct();
                                CLK += 4;
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xdd:	/* STD direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st16(D);
                                break;
#endif
                            case 0xde:	/* LDS direct */
                                direct();
                                CLK += 5;
                                S = ld16(RDMEM16(ea));
                                break;
                            case 0xdf:	/* STS direct */
                                direct();
                                CLK += 5;
                                st16(S);
                                break;
#ifdef FULL6809
                            case 0xe0:	/* SUBB indexed (UNDOC) */
                                indexed();
                                B = sub(B, RDMEM(ea));
                                break;
                            case 0xe2:	/* SBCB indexed (UNDOC) */
                                indexed();
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xe6:	/* LDB indexed (UNDOC) */
                                indexed();
                                B = ld(RDMEM(ea));
                                break;
                            case 0xe7:	/* STB indexed (UNDOC) */
                                indexed();
                                st(B);
                                break;
                            case 0xe8:	/* EORB indexed (UNDOC) */
                                indexed();
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xea:	/* ORB indexed (UNDOC) */
                                indexed();
                                B = or(B, RDMEM(ea));
                                break;
                            case 0xec:	/* LDD indexed (UNDOC) */
                                indexed();
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xed:	/* STD indexed (UNDOC) */
                                indexed();
                                st16(D);
                                break;
#endif
                            case 0xee:	/* LDS indexed */
                                CLK++;
                                indexed();
                                S = ld16(RDMEM16(ea));
                                break;
                            case 0xef:	/* STS indexed */
                                CLK++;
                                indexed();
                                st16(S);
                                break;
#ifdef FULL6809
                            case 0xf0:	/* SUBB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = sub(B, RDMEM(ea));
                                break;
                            case 0xf2:	/* SBCB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xf6:	/* LDB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = ld(RDMEM(ea));
                                break;
                            case 0xf7:	/* STB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st(B);
                                break;
                            case 0xf8:	/* EORB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xfa:	/* ORB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = or(B, RDMEM(ea));
                                break;
                            case 0xfc:	/* LDD extended (UNDOC) */
                                extended();
                                CLK += 5;
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xfd:	/* STD extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st16(D);
                                break;
#endif
                            case 0xfe:	/* LDS extended */
                                extended();
                                CLK += 6;
                                S = ld16(RDMEM16(ea));
                                break;
                            case 0xff:	/* STS extended */
                                extended();
                                CLK += 6;
                                st16(S);
                                break;
#ifdef H6309
                            default:	/* 6309 illegal opcode trap */
                                opcode_trap();
                                break;
#else
#ifndef FULL6809
                            default:
                                sim_error("invalid opcode (1) at %X\n", iPC);
                                break;
#endif
#endif
                        }
                }
                break;
            case 0x11:
                {
                    page_11:
                        opcode = imm_byte();

                        switch (opcode) {
#ifdef FULL6809
                            case 0x00:	/* NEG direct (UNDOC) */
                            case 0x01:	/* NEG direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x04:	/* LSR direct (UNDOC) */
                            case 0x05:	/* LSR direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x06:	/* ROR direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x09:	/* ROL direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x0c:	/* INC direct (UNDOC) */
                                direct();
                                CLK += 4;
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x0d:	/* TST direct (UNDOC) */
                                direct();
                                CLK += 4;
                                tst(RDMEM(ea));
                                break;
                            case 0x0e:	/* JMP direct (UNDOC) */
                                direct();
                                CLK += 3;
                                PC = ea;
                                break;
#endif
                            case 0x10:	/* ignore further prefix bytes (UNDOC) */
                            case 0x11:	/* ignore further prefix bytes (UNDOC) */
                                goto page_11;
#ifdef FULL6809
                            case 0x12:	/* NOP (UNDOC) */
                                nop();
                                break;
                            case 0x13:	/* SYNC (UNDOC) */
                                sync();
                                break;
                            case 0x14:	/* HCF (UNDOC) */
                            case 0x15:	/* HCF (UNDOC) */
                                hcf();
                                break;
                            case 0x16:	/* LBRA (UNDOC) */
                                long_bra();
                                CLK += 5;
                                break;
                            case 0x17:	/* LBSR (UNDOC) */
                                long_bsr();
                                break;
                            case 0x18:	/* CCRS (UNDOC) */
                                ccrs();
                                break;
                            case 0x1a:	/* ORCC immediate (UNDOC) */
                                orcc();
                                break;
                            case 0x1b:	/* NOP (UNDOC) */
                                nop();
                                break;
                            case 0x1d:	/* SEX (UNDOC) */
                                sex();
                                break;
                            case 0x1e:	/* EXG post (UNDOC) */
                                exg();
                                break;
                            case 0x1f:	/* TFR post (UNDOC) */
                                tfr();
                                break;
                            case 0x30:	/* LEAX indexed (UNDOC) */
                                indexed();
                                Z = X = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x31:	/* LEAY indexed (UNDOC) */
                                indexed();
                                Z = Y = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x32:	/* LEAS indexed (UNDOC) */
                                indexed();
                                S = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x33:	/* LEAU indexed (UNDOC) */
                                indexed();
                                U = ea;
                                break;
#endif
#ifdef FULL6809
                            case 0x34:	/* PSHS post (UNDOC) */
                                pshs();
                                break;
#endif
#ifdef FULL6809
                            case 0x35:	/* PULS post (UNDOC) */
                                puls();
                                break;
#endif
#ifdef FULL6809
                            case 0x36:	/* PSHU post (UNDOC) */
                                pshu();
                                break;
#endif
#ifdef H6309
                            case 0x36:	/* LDBT post,direct */
                                post_byte = imm_byte();
                                direct();
                                /* TODO: cycle count */
                                ldbt(post_byte, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x37:	/* PULU post (UNDOC) */
                                pulu();
                                break;
#endif
#ifdef H6309
                            case 0x37:	/* STBT post,direct */
                                post_byte = imm_byte();
                                direct();
                                /* TODO: cycle count */
                                WRMEM(ea, stbt(post_byte, RDMEM(ea)));
                                break;
#endif
#ifdef H6309
                            case 0x38:	/* TFM R+,R+ */
                                tfmpp(imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x39:	/* RTS (UNDOC) */
                                rts();
                                break;
#endif
#ifdef H6309
                            case 0x39:	/* TFM R-,R- */
                                tfmmm(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x3a:	/* TFM R+,R */
                                tfmpc(imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x3b:	/* RTI (UNDOC) */
                                rti();
                                break;
#endif
#ifdef H6309
                            case 0x3b:	/* TFM R,R+ */
                                tfmcp(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x3c:	/* BITMD immediate */
                                /* TODO: cycle count */
                                bit(get_md(), imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x3d:	/* MUL (UNDOC) */
                                mul();
                                break;
#endif
#ifdef H6309
                            case 0x3d:	/* LDMD immediate */
                                /* TODO: cycle count */
                                set_md(ld(imm_byte()));
                                break;
#endif
#ifdef FULL6809
                            case 0x3e:	/* SWIRES (UNDOC) */
                                swires();
                                break;
#endif
                            case 0x3f:	/* SWI3 */
                                swi3();
                                break;
#ifdef FUL6809
                            case 0x40:	/* NEGA (UNDOC) */
                            case 0x41:	/* NEGA (UNDOC) */
                                A = neg(A);
                                break;
#endif
#ifdef FULL6809
                            case 0x44:	/* LSRA (UNDOC) */
                            case 0x45:	/* LSRA (UNDOC) */
                                A = lsr(A);
                                break;
                            case 0x46:	/* RORA (UNDOC) */
                                A = ror(A);
                                break;
                            case 0x49:	/* ROLA (UNDOC) */
                                A = rol(A);
                                break;
#endif
#ifdef FULL6809
                            case 0x4c:	/* INCA (UNDOC) */
                                A = inc(A);
                                break;
#endif
#ifdef H6309
                            case 0x4c:	/* INCE */
                                E = inc(E);
                                break;
#endif
#ifdef FULL6809
                            case 0x4d:	/* TSTA (UNDOC) */
                                tst(A);
                                break;
#endif
#ifdef H6309
                            case 0x4d:	/* TSTE */
                                tst(E);
                                break;
#endif
#ifdef FULL6809
                            case 0x50:	/* NEGB (UNDOC) */
                            case 0x51:	/* NEGB (UNDOC) */
                                B = neg(B);
                                break;
#endif
#ifdef FULL6809
                            case 0x54:	/* LSRB (UNDOC) */
                            case 0x55:	/* LSRB (UNDOC) */
                                B = lsr(B);
                                break;
                            case 0x56:	/* RORB (UNDOC) */
                                B = ror(B);
                                break;
                            case 0x59:	/* ROLB (UNDOC) */
                                B = rol(B);
                                break;
#endif
#ifdef FULL6809
                            case 0x5c:	/* INCB (UNDOC) */
                                B = inc(B);
                                break;
#endif
#ifdef H6309
                            case 0x5c:	/* INCF */
                                F = inc(F);
                                break;
#endif
#ifdef FULL6809
                            case 0x5d:	/* TSTB (UNDOC) */
                                tst(B);
                                break;
#endif
#ifdef H6309
                            case 0x5d:	/* TSTF */
                                tst(F);
                                break;
#endif
#ifdef FULL6809
                            case 0x60:	/* NEG indexed (UNDOC) */
                            case 0x61:	/* NEG indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x64:	/* LSR indexed (UNDOC) */
                            case 0x65:	/* LSR indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x66:	/* ROR indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x69:	/* ROL indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x6c:	/* INC indexed (UNDOC) */
                                indexed();
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x6d:	/* TST indexed (UNDOC) */
                                indexed();
                                tst(RDMEM(ea));
                                break;
                            case 0x6e:	/* JMP indexed (UNDOC) */
                                indexed();
                                CLK++;
                                PC = ea;
                                break;
                            case 0x70:	/* NEG extended (UNDOC) */
                            case 0x71:	/* NEG extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, neg(RDMEM(ea)));
                                break;
                            case 0x74:	/* LSR extended (UNDOC) */
                            case 0x75:	/* LSR extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, lsr(RDMEM(ea)));
                                break;
                            case 0x76:	/* ROR extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, ror(RDMEM(ea)));
                                break;
                            case 0x79:	/* ROL extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, rol(RDMEM(ea)));
                                break;
                            case 0x7c:	/* INC extended (UNDOC) */
                                extended();
                                CLK += 5;
                                WRMEM(ea, inc(RDMEM(ea)));
                                break;
                            case 0x7d:	/* TST extended (UNDOC) */
                                extended();
                                CLK += 5;
                                tst(RDMEM(ea));
                                break;
                            case 0x7e:	/* JMP extended (UNDOC) */
                                extended();
                                CLK += 4;
                                PC = ea;
                                break;
                            case 0x80:	/* SUBA immediate (UNDOC) */
                                CLK += 2;
                                A = sub(A, imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x80:	/* SUBE immediate */
                                /* TODO: cycle count */
                                E = sub(E, imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x82:	/* SBCA immediate (UNDOC) */
                                CLK += 2;
                                A = sbc(A, imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x86:	/* LDA immediate (UNDOC) */
                                CLK += 2;
                                A = ld(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0x86:	/* LDE immediate */
                                /* TODO: cycle count */
                                E = ld(imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x87:	/* SCC immediate (UNDOC) */
                                /* TODO: cycle count */
                                scc(imm_byte());
                                break;
                            case 0x8a:	/* ORA immediate (UNDOC) */
                                CLK += 2;
                                A = or(A, imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0x8e:	/* LDX immediate (UNDOC) */
                                CLK += 3;
                                X = ld16(imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x8f:	/* STX immediate (UNDOC) */
                                /* TODO: cycle count */
                                st_imm(X);
                                break;
#endif
#ifdef H6309
                            case 0x8f: /* MULD immediate */
                                muld(imm_word());
                                break;
#endif
#ifdef FULL6809
                            case 0x90:	/* SUBA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x90:	/* SUBE direct */
                                direct();
                                /* TODO: cycle count */
                                E = sub(E, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x92:	/* SBCA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x96:	/* LDA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0x96:	/* LDE direct */
                                direct();
                                /* TODO: cycle count */
                                E = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x97:	/* STA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0x97:	/* STE direct */
                                direct();
                                /* TODO: cycle count */
                                st(E);
                                break;
#endif
#ifdef FULL6809
                            case 0x9a:	/* ORA direct (UNDOC) */
                                direct();
                                CLK += 4;
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x9d:	/* JSR direct (UNDOC) */
                                direct();
                                CLK += 7;
                                jsr();
                                break;
#endif
#ifdef FULL6809
                            case 0x9e:	/* LDX direct (UNDOC) */
                                direct();
                                CLK += 4;
                                X = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0x9f:	/* STX direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st16(X);
                                break;
#endif
#ifdef H6309
                            case 0x9f:	/* MULD direct */
                                direct();
                                /* TODO: cycle count */
                                muld(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa0:	/* SUBA indexed (UNDOC) */
                                indexed();
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xa0:	/* SUBE indexed */
                                indexed();
                                E = sub(E, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa2:	/* SBCA indexed (UNDOC) */
                                indexed();
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa6:	/* LDA indexed (UNDOC) */
                                indexed();
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xa6:	/* LDE indexed */
                                indexed();
                                E = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xa7:	/* STA indexed (UNDOC) */
                                indexed();
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0xa7:	/* STE indexed */
                                indexed();
                                st(E);
                                break;
#endif
#ifdef FULL6809
                            case 0xaa:	/* ORA indexed (UNDOC) */
                                indexed();
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xad:	/* JSR indexed (UNDOC) */
                                indexed();
                                CLK += 3;
                                jsr();
                                break;
#endif
#ifdef FULL6809
                            case 0xae:	/* LDX indexed (UNDOC) */
                                indexed();
                                X = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xaf:	/* STX indexed (UNDOC) */
                                indexed();
                                st16(X);
                                break;
#endif
#ifdef H6309
                            case 0xaf:	/* MULD indexed */
                                indexed();
                                muld(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb0:	/* SUBA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = sub(A, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xb0:	/* SUBE extended */
                                extended();
                                /* TODO: cycle count */
                                E = sub(E, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb2:	/* SBCA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = sbc(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb6:	/* LDA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xb6:	/* LDE extended */
                                extended();
                                /* TODO: cycle count */
                                E = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xb7:	/* STA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st(A);
                                break;
#endif
#ifdef H6309
                            case 0xb7:	/* STE extended */
                                extended();
                                /* TODO: cycle count */
                                st(E);
                                break;
#endif
#ifdef FULL6809
                            case 0xba:	/* ORA extended (UNDOC) */
                                extended();
                                CLK += 5;
                                A = or(A, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xbd:	/* JSR extended (UNDOC) */
                                extended();
                                CLK += 8;
                                jsr();
                                break;
#endif
#ifdef FULL6809
                            case 0xbe:	/* LDX extended (UNDOC) */
                                extended();
                                CLK += 5;
                                X = ld16(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xbf:	/* STX extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st16(X);
                                break;
#endif
#ifdef H6309
                            case 0xbf:	/* MULD extended */
                                extended();
                                /* TODO: cycle count */
                                muld(RDMEM16(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xc0:	/* SUBB immediate (UNDOC) */
                                CLK += 2;
                                B = sub(B, imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0xc0:	/* SUBF immediate */
                                /* TODO: cycle count */
                                F = sub(F, imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0xc2:	/* SBCB immediate (UNDOC) */
                                CLK += 2;
                                B = sbc(B, imm_byte());
                                break;
                            case 0xc6:	/* LDB immediate (UNDOC) */
                                CLK += 2;
                                B = ld(imm_byte());
                                break;
#endif
#ifdef H6309
                            case 0xc6:	/* LDF immediate */
                                /* TODO: cycle count */
                                F = ld(imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0xc7:	/* SCC immediate (UNDOC) */
                                /* TODO: cycle count */
                                scc(imm_byte());
                                break;
                            case 0xca:	/* ORB immediate (UNDOC) */
                                CLK += 2;
                                B = or(B, imm_byte());
                                break;
#endif
#ifdef FULL6809
                            case 0xcc:	/* LDD immediate (UNDOC) */
                                CLK += 3;
                                D = ld16(imm_word());
                                break;
                            case 0xcd:	/* HCF (UNDOC) */
                                hcf();
                                break;
                            case 0xce:	/* LDU immediate (UNDOC) */
                                CLK += 3;
                                U = ld16(imm_word());
                                break;
                            case 0xcf:	/* STU immediate (UNDOC) */
                                /* TODO: cycle count */
                                st_imm(U);
                                break;
                            case 0xd0:	/* SUBB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = sub(B, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xd0:	/* SUBF direct */
                                direct();
                                /* TODO: cycle count */
                                F = sub(F, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xd2:	/* SBCB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xd6:	/* LDB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xd6:	/* LDF direct */
                                direct();
                                /* TODO: cycle count */
                                F = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xd7:	/* STB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st(B);
                                break;
#endif
#ifdef H6309
                            case 0xd7:	/* STF direct */
                                direct();
                                /* TODO: cycle count */
                                st(F);
                                break;
#endif
#ifdef FULL6809
                            case 0xd8:	/* EORB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xda:	/* ORB direct (UNDOC) */
                                direct();
                                CLK += 4;
                                B = or(B, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xdc:	/* LDD direct (UNDOC) */
                                direct();
                                CLK += 4;
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xdd:	/* STD direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st16(D);
                                break;
                            case 0xde:	/* LDU direct (UNDOC) */
                                direct();
                                CLK += 4;
                                U = ld16(RDMEM16(ea));
                                break;
                            case 0xdf:	/* STU direct (UNDOC) */
                                direct();
                                CLK += 4;
                                st16(U);
                                break;
                            case 0xe0:	/* SUBB indexed (UNDOC) */
                                indexed();
                                B = sub(B, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xe0:	/* SUBF indexed */
                                indexed();
                                F = sub(F, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xe2:	/* SBCB indexed (UNDOC) */
                                indexed();
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xe6:	/* LDB indexed (UNDOC) */
                                indexed();
                                B = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xe6:	/* LDF indexed */
                                indexed();
                                F = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xe7:	/* STB indexed (UNDOC) */
                                indexed();
                                st(B);
                                break;
#endif
#ifdef H6309
                            case 0xe7:	/* STF indexed (UNDOC) */
                                indexed();
                                st(F);
                                break;
#endif
#ifdef FULL6809
                            case 0xe8:	/* EORB indexed (UNDOC) */
                                indexed();
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xea:	/* ORB indexed (UNDOC) */
                                indexed();
                                B = or(B, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xec:	/* LDD indexed (UNDOC) */
                                indexed ();
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xed:	/* STD indexed (UNDOC) */
                                indexed();
                                st16(D);
                                break;
                            case 0xee:	/* LDU indexed (UNDOC) */
                                indexed();
                                U = ld16(RDMEM16(ea));
                                break;
                            case 0xef:	/* STU indexed (UNDOC) */
                                indexed();
                                st16(U);
                                break;
                            case 0xf0:	/* SUBB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = sub(B, RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xf0:	/* SUBF extended */
                                extended();
                                /* TODO: cycle count */
                                F = sub(F, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xf2:	/* SBCB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = sbc(B, RDMEM(ea));
                                break;
                            case 0xf6:	/* LDB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = ld(RDMEM(ea));
                                break;
#endif
#ifdef H6309
                            case 0xf6:	/* LDF extended */
                                extended();
                                /* TODO: cycle count */
                                F = ld(RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xf7:	/* STB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st(B);
                                break;
#endif
#ifdef H6309
                            case 0xf7:	/* STF extended */
                                extended();
                                /* TODO: cycle count */
                                st(F);
                                break;
#endif
#ifdef FULL6809
                            case 0xf8:	/* EORB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = eor(B, RDMEM(ea));
                                break;
                            case 0xfa:	/* ORB extended (UNDOC) */
                                extended();
                                CLK += 5;
                                B = or(B, RDMEM(ea));
                                break;
#endif
#ifdef FULL6809
                            case 0xfc:	/* LDD extended (UNDOC) */
                                extended();
                                CLK += 5;
                                D = ld16(RDMEM16(ea));
                                break;
                            case 0xfd:	/* STD extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st16(D);
                                break;
                            case 0xfe:	/* LDU extended (UNDOC) */
                                extended();
                                CLK += 5;
                                U = ld16(RDMEM16(ea));
                                break;
                            case 0xff:	/* STU extended (UNDOC) */
                                extended();
                                CLK += 5;
                                st16(U);
                                break;
#endif
#ifdef H6309
                             default:		/* 6309 illegal opcode trap */
                                 opcode_trap();
                                 break;
#else
#ifndef FULL6809
                             default:
                                 sim_error("invalid opcode (2) at %X\n", iPC);
                                 break;
#endif
#endif
                         }
                }
                break;
            case 0x12:	/* NOP */
                CLK_ADD(1, 0);
                break;
            case 0x13:	/* SYNC */
                sync();
                break;
#ifdef FULL6809
            case 0x14:	/* HCF (UNDOC) */
                hcf();
                break;
#endif
#ifdef H6309
            case 0x14:	/* SEXW */
                CLK += 3;
                sexw();
                break;
#endif
#ifdef FULL6809
            case 0x15:	/* HCF (UNDOC) */
                hcf();
                break;
#endif
            case 0x16:	/* LBRA */
                long_bra();
                CLK_ADD(2, 1);
                break;
            case 0x17:	/* LBSR */
                long_bsr_new();
                CLK_ADD(4, 2);
                break;
#ifdef FULL6809
            case 0x18:	/* CCRS (UNDOC) */
                ccrs();
                break;
#endif
            case 0x1a:	/* ORCC immediate */
                orcc_new();
                CLK_ADD(1, 0);
                break;
#ifdef FULL6809
            case 0x1b:	/* NOP (UNDOC) */
                CLK++;
                break;
#endif
            case 0x1d:	/* SEX */
                sex_new();
                break;
            case 0x1e:	/* EXG post */
                exg_new();
                break;
            case 0x1f:	/* TFR post */
                tfr_new();
                break;
            case 0x21:	/* BRN */
                RDMEM(PC++);
                CLK++;
                break;
            case 0x30:	/* LEAX indexed */
                indexed();
                Z = X = ea;
                CLK += 2;
                break;
            case 0x31:	/* LEAY indexed */
                indexed();
                Z = Y = ea;
                CLK += 2;
                break;
            case 0x32:	/* LEAS indexed */
                indexed();
                S = ea;
                CLK += 2;
                break;
            case 0x33:	/* LEAU indexed */
                indexed();
                U = ea;
                CLK += 2;
                break;
            case 0x34:	/* PSHS post */
                pshs();
                break;
            case 0x35:	/* PULS post */
                puls();
                break;
            case 0x36:	/* PSHU post */
                pshu();
                break;
            case 0x37:	/* PULU post */
                pulu();
                break;
            case 0x39:	/* RTS */
                rts();
                break;
            case 0x3b:	/* RTI */
                rti();
                break;
            case 0x3d:	/* MUL */
                mul();
                break;
#ifdef FULL6809
            case 0x3e:	/* SWIRES (UNDOC) */
                swires();
                break;
#endif
            case 0x3f:	/* SWI */
                swi();
                break;
            case 0x40:	/* NEGA */
                A = neg_new(A, 1, 0);
                break;
#ifdef FULL6809
            case 0x41:	/* NEGA (UNDOC) */
                A = neg_new(A, 1, 1);
                break;
#endif
            case 0x44:	/* LSRA */
                A = lsr_new(A, 1, 0);
                break;
#ifdef FULL6809
            case 0x45:	/* LSRA (UNDOC) */
                A = lsr_new(A, 1, -1);
                break;
#endif
            case 0x46:	/* RORA */
                A = ror_new(A, 1, 0);
                break;
            case 0x49:	/* ROLA */
                A = rol_new(A, 1, 0);
                break;
            case 0x4c:	/* INCA */
                A = inc_new(A, 1, 0);
                break;
            case 0x4d:	/* TSTA */
                tst_new(A, 1, 0);
                break;
            case 0x50:	/* NEGB */
                B = neg(B);
                break;
#ifdef FULL6809
            case 0x51:	/* NEGB (UNDOC) */
                B = neg(B);
                break;
#endif
            case 0x54:	/* LSRB */
                B = lsr(B);
                break;
#ifdef FULL6809
            case 0x55:	/* LSRB (UNDOC) */
                B = lsr(B);
                break;
#endif
            case 0x56:	/* RORB */
                B = ror(B);
                break;
            case 0x59:	/* ROLB */
                B = rol(B);
                break;
            case 0x5c:	/* INCB */
                B = inc(B);
                break;
            case 0x5d:	/* TSTB */
                tst(B);
                break;
            case 0x60:	/* NEG indexed */
                indexed();
                WRMEM(ea, neg(RDMEM(ea)));
                break;
#ifdef FULL6809
            case 0x61:	/* NEG indexed (UNDOC) */
                indexed();
                WRMEM(ea, neg(RDMEM(ea)));
                break;
#endif
#ifdef H6309
            case 0x61:	/* OIM indexed */
                post_byte = imm_byte();
                indexed();
                WRMEM(ea, or(RDMEM(ea), post_byte));
                break;
#endif
            case 0x64:	/* LSR indexed */
                indexed();
                WRMEM(ea, lsr(RDMEM(ea)));
                break;
#ifdef FULL6809
            case 0x65:	/* LSR indexed (UNDOC) */
                indexed();
                WRMEM(ea, lsr(RDMEM(ea)));
                break;
#endif
            case 0x66:	/* ROR indexed */
                indexed();
                WRMEM(ea, ror(RDMEM(ea)));
                break;
            case 0x69:	/* ROL indexed */
                indexed();
                WRMEM(ea, rol(RDMEM(ea)));
                break;
#ifdef H6309
            case 0x6b:	/* TIM indexed */
                post_byte = imm_byte();
                indexed();
                WRMEM(ea, tim(post_byte));
                break;
#endif
            case 0x6c:	/* INC indexed */
                indexed();
                WRMEM(ea, inc(RDMEM(ea)));
                break;
            case 0x6d:	/* TST indexed */
                indexed();
                tst(RDMEM(ea));
                break;
            case 0x6e:	/* JMP indexed */
                indexed();
                CLK++;
                PC = ea;
                break;
            case 0x70:	/* NEG extended */
                extended();
                CLK += 5;
                WRMEM(ea, neg(RDMEM(ea)));
                break;
#ifdef FULL6809
            case 0x71:	/* NEG extended (UNDOC) */
                extended();
                CLK += 5;
                WRMEM(ea, neg(RDMEM(ea)));
                break;
#endif
#ifdef H6309
            case 0x71:	/* OIM extended */
                post_byte = imm_byte();
                extended();
                /* TODO: cycle count */
                WRMEM(ea, or(RDMEM(ea), post_byte));
                break;
#endif
            case 0x74:	/* LSR extended */
                extended();
                CLK += 5;
                WRMEM(ea, lsr(RDMEM(ea)));
                break;
#ifdef FULL6809
            case 0x75:	/* LSR extended (UNDOC) */
                extended();
                CLK += 5;
                WRMEM(ea, lsr(RDMEM(ea)));
                break;
#endif
            case 0x76:	/* ROR extended */
                extended();
                CLK += 5;
                WRMEM(ea, ror(RDMEM(ea)));
                break;
            case 0x79:	/* ROL extended */
                extended();
                CLK += 5;
                WRMEM(ea, rol(RDMEM(ea)));
                break;
#ifdef H6309
            case 0x7b:	/* TIM extended */
                post_byte = imm_byte();
                extended();
                /* TODO: cycle count */
                WRMEM(ea, tim(post_byte));
                break;
#endif
            case 0x7c:	/* INC extended */
                extended();
                CLK += 5;
                WRMEM(ea, inc(RDMEM(ea)));
                break;
            case 0x7d:	/* TST extended */
                extended();
                CLK += 5;
                tst(RDMEM(ea));
                break;
            case 0x7e:	/* JMP extended */
                extended();
                CLK += 4;
                PC = ea;
                break;
            case 0x80:	/* SUBA immediate */
                CLK += 2;
                A = sub(A, imm_byte());
                break;
            case 0x82:	/* SBCA immediate */
                CLK += 2;
                A = sbc(A, imm_byte());
                break;
            case 0x83:	/* SUBD immediate */
                CLK += 4;
                D = sub16(D, imm_word());
                break;
            case 0x86:	/* LDA immediate */
                CLK += 2;
                A = ld(imm_byte());
                break;
#ifdef FULL6809
            case 0x87:	/* SCC immediate (UNDOC) */
                /* TODO: cycle count */
                scc(imm_byte());
                break;
#endif
            case 0x8a:	/* ORA immediate */
                CLK += 2;
                A = or(A, imm_byte());
                break;
            case 0x8e:	/* LDX immediate */
                CLK += 3;
                X = ld16(imm_word());
                break;
#ifdef FULL6809
            case 0x8f:	/* STX immediate (UNDOC) */
                /* TODO: cycle count */
                st_imm(X);
                break;
#endif
            case 0x90:	/* SUBA direct */
                direct();
                CLK += 4;
                A = sub(A, RDMEM(ea));
                break;
            case 0x92:	/* SBCA direct */
                direct();
                CLK += 4;
                A = sbc(A, RDMEM(ea));
                break;
            case 0x93:	/* SUBD direct */
                direct();
                CLK += 4;
                D = sub16(D, RDMEM16(ea));
                CLK++;
                break;
            case 0x96:	/* LDA direct */
                direct();
                CLK += 4;
                A = ld(RDMEM(ea));
                break;
            case 0x97:	/* STA direct */
                direct();
                CLK += 4;
                st(A);
                break;
            case 0x9a:	/* ORA direct */
                direct();
                CLK += 4;
                A = or(A, RDMEM(ea));
                break;
            case 0x9d:	/* JSR direct */
                direct();
                CLK += 7;
                jsr();
                break;
            case 0x9e:	/* LDX direct */
                direct();
                CLK += 4;
                X = ld16(RDMEM16(ea));
                break;
            case 0x9f:	/* STX direct */
                direct();
                CLK += 4;
                st16(X);
                break;
            case 0xa0:	/* SUBA indexed */
                indexed();
                A = sub(A, RDMEM(ea));
                break;
            case 0xa2:	/* SBCA indexed */
                indexed();
                A = sbc(A, RDMEM(ea));
                break;
            case 0xa3:	/* SUBD indexed */
                indexed();
                D = sub16(D, RDMEM16(ea));
                CLK++;
                break;
            case 0xa6:	/* LDA indexed */
                indexed();
                A = ld(RDMEM(ea));
                break;
            case 0xa7:	/* STA indexed */
                indexed();
                st(A);
                break;
            case 0xaa:	/* ORA indexed */
                indexed();
                A = or(A, RDMEM(ea));
                break;
            case 0xad:	/* JSR indexed */
                indexed();
                CLK += 3;
                jsr();
                break;
            case 0xae:	/* LDX indexed */
                indexed();
                X = ld16(RDMEM16(ea));
                break;
            case 0xaf:	/* STX indexed */
                indexed();
                st16(X);
                break;
            case 0xb0:	/* SUBA extended */
                extended();
                CLK += 5;
                A = sub(A, RDMEM(ea));
                break;
            case 0xb2:	/* SBCA extended */
                extended();
                CLK += 5;
                A = sbc(A, RDMEM(ea));
                break;
            case 0xb3:	/* SUBD extended */
                extended();
                CLK += 5;
                D = sub16(D, RDMEM16(ea));
                CLK++;
                break;
            case 0xb6:	/* LDA extended */
                extended();
                CLK += 5;
                A = ld(RDMEM(ea));
                break;
            case 0xb7:	/* STA extended */
                extended();
                CLK += 5;
                st(A);
                break;
            case 0xba:	/* ORA extended */
                extended();
                CLK += 5;
                A = or(A, RDMEM(ea));
                break;
            case 0xbd:	/* JSR extended */
                extended();
                CLK += 8;
                jsr();
                break;
            case 0xbe:	/* LDX extended */
                extended();
                CLK += 5;
                X = ld16(RDMEM16(ea));
                break;
            case 0xbf:	/* STX extended */
                extended();
                CLK += 5;
                st16(X);
                break;
            case 0xc0:	/* SUBB immediate */
                CLK += 2;
                B = sub(B, imm_byte());
                break;
            case 0xc2:	/* SBCB immediate */
                CLK += 2;
                B = sbc(B, imm_byte());
                break;
            case 0xc6:	/* LDB immediate */
                CLK += 2;
                B = ld(imm_byte());
                break;
#ifdef FULL6809
            case 0xc7:	/* SCC immediate (UNDOC) */
                /* TODO: cycle count */
                scc(imm_byte());
                break;
#endif
            case 0xca:	/* ORB immediate */
                CLK += 2;
                B = or(B, imm_byte());
                break;
            case 0xcc:	/* LDD immediate */
                CLK += 3;
                D = ld16(imm_word());
                break;
#ifdef FULL6809
            case 0xcd:	/* HCF (UNDOC) */
                hcf();
                break;
#endif
#ifdef H6309
            case 0xcd:	/* LDQ immediate */
                /* TODO: cycle count */
                Q = ld32(imm_dword());
                break;
#endif
            case 0xce:	/* LDU immediate */
                CLK += 3;
                U = ld16(imm_word());
                break;
#ifdef FULL6809
            case 0xcf:	/* STU immediate (UNDOC) */
                /* TODO: cycle count */
                st_imm(U);
                break;
#endif
            case 0xd0:	/* SUBB direct */
                direct();
                CLK += 4;
                B = sub(B, RDMEM(ea));
                break;
            case 0xd2:	/* SBCB direct */
                direct();
                CLK += 4;
                B = sbc(B, RDMEM(ea));
                break;
            case 0xd6:	/* LDB direct */
                direct();
                CLK += 4;
                B = ld(RDMEM(ea));
                break;
            case 0xd7:	/* STB direct */
                direct();
                CLK += 4;
                st(B);
                break;
            case 0xd8:	/* EORB direct */
                direct();
                CLK += 4;
                B = eor(B, RDMEM(ea));
                break;
            case 0xda:	/* ORB direct */
                direct();
                CLK += 4;
                B = or(B, RDMEM(ea));
                break;
            case 0xdc:	/* LDD direct */
                direct();
                CLK += 4;
                D = ld16(RDMEM16(ea));
                break;
            case 0xdd:	/* STD direct */
                direct();
                CLK += 4;
                st16(D);
                break;
            case 0xde:	/* LDU direct */
                direct();
                CLK += 4;
                U = ld16(RDMEM16(ea));
                break;
            case 0xdf:	/* STU direct */
                direct();
                CLK += 4;
                st16(U);
                break;
            case 0xe0:	/* SUBB indexed */
                indexed();
                B = sub(B, RDMEM(ea));
                break;
            case 0xe2:	/* SBCB indexed */
                indexed();
                B = sbc(B, RDMEM(ea));
                break;
            case 0xe6:	/* LDB indexed */
                indexed();
                B = ld(RDMEM(ea));
                break;
            case 0xe7:	/* STB indexed */
                indexed();
                st(B);
                break;
            case 0xe8:	/* EORB indexed */
                indexed();
                B = eor(B, RDMEM(ea));
                break;
            case 0xea:	/* ORB indexed */
                indexed();
                B = or(B, RDMEM(ea));
                break;
            case 0xec:	/* LDD indexed */
                indexed();
                D = ld16(RDMEM16(ea));
                break;
            case 0xed:	/* STD indexed */
                indexed();
                st16(D);
                break;
            case 0xee:	/* LDU indexed */
                indexed();
                U = ld16(RDMEM16(ea));
                break;
            case 0xef:	/* STU indexed */
                indexed();
                st16(U);
                break;
            case 0xf0:	/* SUBB extended */
                extended();
                CLK += 5;
                B = sub(B, RDMEM(ea));
                break;
            case 0xf2:	/* SBCB extended */
                extended();
                CLK += 5;
                B = sbc(B, RDMEM(ea));
                break;
            case 0xf6:	/* LDB extended */
                extended();
                CLK += 5;
                B = ld(RDMEM(ea));
                break;
            case 0xf7:	/* STB extended */
                extended();
                CLK += 5;
                st(B);
                break;
            case 0xf8:	/* EORB extended */
                extended();
                CLK += 5;
                B = eor(B, RDMEM(ea));
                break;
            case 0xfa:	/* ORB extended */
                extended();
                CLK += 5;
                B = or(B, RDMEM(ea));
                break;
            case 0xfc:	/* LDD extended */
                extended();
                CLK += 5;
                D = ld16(RDMEM16(ea));
                break;
            case 0xfd:	/* STD extended */
                extended();
                CLK += 5;
                st16(D);
                break;
            case 0xfe:	/* LDU extended */
                extended();
                CLK += 5;
                U = ld16(RDMEM16(ea));
                break;
            case 0xff:	/* STU extended */
                extended();
                CLK += 5;
                st16(U);
                break;
#ifdef H6309
            default:		/* 6309 illegal opcode trap */
                 opcode_trap();
                 break;
#else
#ifndef FULL6809
            default:
                CLK += 2;
                sim_error ("invalid opcode '%02X'\n", opcode);
                break;
#endif
#endif
        }
#endif

        if (cc_changed) {
            cc_modified();
        }

    } while (1);

cpu_exit:
   return;
}

void cpu6809_reset (void)
{
    X = Y = S = U = DP = 0;
    H = N = OV = C = 0;
    A = B = 0;
    Z = 1;
    EFI = F_FLAG | I_FLAG;
#ifdef H6309
    MD = E = F = 0;
#endif

    PC = read16(0xfffe);
}
