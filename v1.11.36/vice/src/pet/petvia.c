/*
 * petvia.c - VIA emulation in the PET.
 *
 * Written by
 *  Andre' Fachat <fachat@physik.tu-chemnitz.de>
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

#define mycpu maincpu
#define myclk maincpu_clk
#define myvia via
#define myvia_init via_init

#define I_MYVIAFL I_VIAFL
#define MYVIA_INT VIA_INT
#define MYVIA_NAME "Via"

#define mycpu_rmw_flag maincpu_rmw_flag
#define mycpu_int_status maincpu_int_status
#define mycpu_alarm_context maincpu_alarm_context
#define mycpu_clk_guard maincpu_clk_guard

#define myvia_reset via_reset
#define myvia_store via_store
#define myvia_read via_read
#define myvia_peek via_peek

#define myvia_log via_log
#define myvia_signal via_signal
#define myvia_prevent_clk_overflow via_prevent_clk_overflow
#define myvia_snapshot_read_module via_snapshot_read_module
#define myvia_snapshot_write_module via_snapshot_write_module

#include "vice.h"

#include "crtc.h"
#include "datasette.h"
#include "drive.h"
#include "drivecpu.h"
#include "keyboard.h"
#include "maincpu.h"
#include "parallel.h"
#include "petsound.h"
#include "petvia.h"
#include "printer.h"
#include "types.h"
#include "viacore.h"

void myvia_signal(int line, int edge);

                /* switching PET charrom with CA2 */
#define VIA_SET_CA2(byte) crtc_set_chargen_offset(byte ? 256 : 0); 

                /* switching userport strobe with CB2 */
#define VIA_SET_CB2(byte) printer_interface_userport_write_strobe(byte);

#define via_set_int             maincpu_set_irq
#define VIA_INT                 IK_IRQ

/* #define VIA_TIMER_DEBUG */

static char snap_module_name[] = "VIA";

static void undump_pra(BYTE byte)
{
    printer_interface_userport_write_data(byte);
}

inline static void store_pra(BYTE byte, BYTE myoldpa, WORD addr)
{
        printer_interface_userport_write_data(byte);
}

static void undump_prb(BYTE byte)
{
    parallel_cpu_set_nrfd((BYTE)(!(byte & 0x02)));
    parallel_cpu_restore_atn((BYTE)(!(byte & 0x04)));
}

inline static void store_prb(BYTE byte, BYTE myoldpb, WORD addr)
{
    if ((addr==VIA_DDRB) && (via[addr] & 0x20)) {
        log_warning(via_log, "PET: Killer POKE! might kill a real PET!\n");
    }
    parallel_cpu_set_nrfd((BYTE)(!(byte & 0x02)));
    parallel_cpu_set_atn((BYTE)(!(byte & 0x04)));
    if ((byte ^ myoldpb) & 0x8)
        datasette_toggle_write_bit((~via[VIA_DDRB] | byte) & 0x8);
}

static void undump_pcr(BYTE byte)
{
#if 0
    register BYTE tmp = byte;
    /* first set bit 1 and 5 to the real output values */
    if ((tmp & 0x0c) != 0x0c)
        tmp |= 0x02;
    if ((tmp & 0xc0) != 0xc0)
        tmp |= 0x20;
    crtc_set_char(byte & 2); /* switching PET charrom with CA2 */
                             /* switching userport strobe with CB2 */
#endif
}

inline static void store_pcr(BYTE byte, WORD addr)
{
#if 0
    if (byte != via[VIA_PCR]) {
        register BYTE tmp = byte;
        /* first set bit 1 and 5 to the real output values */
        if((tmp & 0x0c) != 0x0c)
            tmp |= 0x02;
        if((tmp & 0xc0) != 0xc0)
            tmp |= 0x20;
        crtc_set_char(byte & 2); /* switching PET charrom with CA2 */
                                 /* switching userport strobe with CB2 */
        printer_interface_userport_write_strobe(byte & 0x20);
    }
#endif
}

static void undump_acr(BYTE byte)
{
        store_petsnd_onoff(via[VIA_T2LL] ? (((byte & 0x1c)==0x10)?1:0) : 0);
}

inline void static store_acr(BYTE byte)
{
        store_petsnd_onoff(via[VIA_T2LL] ? (((byte & 0x1c)==0x10)?1:0) : 0);
}

inline void static store_sr(BYTE byte)
{
        store_petsnd_sample(byte);
}

inline void static store_t2l(BYTE byte)
{
    store_petsnd_rate(2*byte+4);
    if (!byte) {
        store_petsnd_onoff(0);
    } else {
        store_petsnd_onoff(((via[VIA_ACR] & 0x1c)==0x10)?1:0);
    }
}

static void res_via(void)
{
    /* set IEC output lines */
    parallel_cpu_set_atn(0);
    parallel_cpu_set_nrfd(0);

    printer_interface_userport_write_data(0xff);
    printer_interface_userport_write_strobe(1);
}

inline static BYTE read_pra(WORD addr)
{
    BYTE byte;
    byte = 255;
    /* VIA PA is connected to the userport pins C-L */
    byte &= (joystick_value[1] & 1) ? ~0x80 : 0xff;
    byte &= (joystick_value[1] & 2) ? ~0x40 : 0xff;
    byte &= (joystick_value[1] & 4) ? ~0x20 : 0xff;
    byte &= (joystick_value[1] & 8) ? ~0x10 : 0xff;
    byte &= (joystick_value[1] & 16)? ~0xc0 : 0xff;
    byte &= (joystick_value[2] & 1) ? ~0x08 : 0xff;
    byte &= (joystick_value[2] & 2) ? ~0x04 : 0xff;
    byte &= (joystick_value[2] & 4) ? ~0x02 : 0xff;
    byte &= (joystick_value[2] & 8) ? ~0x01 : 0xff;
    byte &= (joystick_value[2] & 16)? ~0x0c : 0xff;

    /* joystick always pulls low, even if high output, so no
       masking with DDRA */
    /*return ((j & ~via[VIA_DDRA]) | (via[VIA_PRA] & via[VIA_DDRA]));*/
    return byte;
}

inline static BYTE read_prb(void)
{
    BYTE byte;

    if (drive[0].enable)
        drive0_cpu_execute(maincpu_clk);
    if (drive[1].enable)
        drive1_cpu_execute(maincpu_clk);

    /* read parallel IEC interface line states */
    byte = 255
           - (parallel_nrfd ? 64:0)
           - (parallel_ndac ? 1:0)
           - (parallel_dav ? 128:0);
    /* vertical retrace */
    byte -= crtc_offscreen() ? 32:0;

    /* none of the load changes output register value -> std. masking */
    byte = ((byte & ~via[VIA_DDRB]) | (via[VIA_PRB] & via[VIA_DDRB]));
    return byte;
}

void printer_interface_userport_set_busy(int b)
{
    via_signal(VIA_SIG_CA1, b ? VIA_SIG_RISE : VIA_SIG_FALL);
}

#include "viacore.c"

