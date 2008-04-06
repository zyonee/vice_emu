/*
 * wd1770.c - WD1770 emulation for the 1571 and 1581 disk drives.
 *
 * Written by
 *  Andreas Boose (boose@linux.rz.fh-hannover.de)
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

/* FIXME: wd1770 support is far from being complete.  */

#define WD_DEBUG

#include "vice.h"

#include <stdio.h>

#include "interrupt.h"
#include "wd1770.h"

/*-----------------------------------------------------------------------*/

static void store_wd1770(ADDRESS addr, BYTE byte, int dnr);
static BYTE read_wd1770(ADDRESS addr, int dnr);
static void reset_wd1770(int dnr);

static void wd1770_command_restore(BYTE command, int dnr);
static void wd1770_command_seek(BYTE command, int dnr);
static void wd1770_command_step(BYTE command, int dnr);
static void wd1770_command_stepin(BYTE command, int dnr);
static void wd1770_command_stepout(BYTE command, int dnr);
static void wd1770_command_readsector(BYTE command, int dnr);
static void wd1770_command_writesector(BYTE command, int dnr);
static void wd1770_command_readaddress(BYTE command, int dnr);
static void wd1770_command_forceint(BYTE command, int dnr);
static void wd1770_command_readtrack(BYTE command, int dnr);
static void wd1770_command_writetrack(BYTE command, int dnr);

/* wd1770 disk controller structure.  */
static wd1770_t wd1770[2];

/*-----------------------------------------------------------------------*/
/* WD1770 external interface.  */

void REGPARM2 store_wd1770d0(ADDRESS addr, BYTE byte)
{
    store_wd1770(addr & 3, byte, 0);
}

BYTE REGPARM1 read_wd1770d0(ADDRESS addr)
{
    return read_wd1770(addr & 3, 0);
}

void reset_wd1770d0(void)
{
    reset_wd1770(0);
}

void REGPARM2 store_wd1770d1(ADDRESS addr, BYTE byte)
{
    store_wd1770(addr & 3, byte, 1);
}

BYTE REGPARM1 read_wd1770d1(ADDRESS addr)
{
    return read_wd1770(addr & 3, 1);
}

void reset_wd1770d1(void)
{
    reset_wd1770(1);
}

/* Clock overflow handling.  */

void wd1770d0_prevent_clk_overflow(CLOCK sub)
{
    if (wd1770[0].busy_clk > (CLOCK) 0)
        wd1770[0].busy_clk -= sub;
    if (wd1770[0].motor_spinup_clk > (CLOCK) 0)
        wd1770[0].motor_spinup_clk -= sub;
}

void wd1770d1_prevent_clk_overflow(CLOCK sub)
{
    if (wd1770[1].busy_clk > (CLOCK) 0)
        wd1770[1].busy_clk -= sub;
    if (wd1770[0].motor_spinup_clk > (CLOCK) 0)
        wd1770[0].motor_spinup_clk -= sub;
}

/*-----------------------------------------------------------------------*/
/* WD1770 register read/write access.  */

static void store_wd1770(ADDRESS addr, BYTE byte, int dnr)
{
#ifdef WD_DEBUG
    printf("WD WRITE ADDR: %i DATA:%x\n",addr,byte);
#endif
    wd1770[dnr].busy_clk = drive_clk[dnr];

    switch (addr) {
      case 0:
        switch (byte >> 5) {
          case 0:
            if (byte & 0x10)
                wd1770_command_seek(byte, dnr);
            else
                wd1770_command_restore(byte, dnr);
            break;
          case 1:
            wd1770_command_step(byte, dnr);
            break;
          case 2:
            wd1770_command_stepin(byte, dnr);
            break;
          case 3:
            wd1770_command_stepout(byte, dnr);
            break;
          case 4:
            wd1770_command_readsector(byte, dnr);
            break;
          case 5:
            wd1770_command_writesector(byte, dnr);
            break;
          case 6:
            if (byte & 0x10)
                wd1770_command_forceint(byte, dnr);
            else
                wd1770_command_readaddress(byte, dnr);
            break;
          case 7:
            if (byte & 0x10)
                wd1770_command_writetrack(byte, dnr);
            else
                wd1770_command_readtrack(byte, dnr);
            break;
        }
        break;
      case 1:
      case 2:
      case 3:
        wd1770[dnr].reg[addr] = byte;
        break;
    }
}
/* extern int drive0_traceflg;*/
static BYTE read_wd1770(ADDRESS addr, int dnr)
{
    BYTE tmp = 0;

    switch (addr) {
      case WD1770_STATUS:
        wd1770[dnr].reg[addr] &= 0xfc;
        if (wd1770[dnr].data_buffer_index >= 0)
            wd1770[dnr].reg[WD1770_STATUS] |= 3;
        if (wd1770[dnr].busy_clk != (CLOCK)0) {
            if (drive_clk[dnr] - wd1770[dnr].busy_clk < 100)
                wd1770[dnr].reg[WD1770_STATUS] |= 1;
            else
                wd1770[dnr].busy_clk = (CLOCK)0;
        }
        if (wd1770[dnr].motor_spinup_clk != (CLOCK)0
            && (wd1770[dnr].reg[WD1770_STATUS] & 0x80)) {
            wd1770[dnr].reg[WD1770_STATUS] &= 0xdf;
            if (drive_clk[dnr] - wd1770[dnr].motor_spinup_clk > 50) {
                wd1770[dnr].reg[WD1770_STATUS] |= 0x20;
                wd1770[dnr].motor_spinup_clk = (CLOCK)0;
            }
        }
        tmp = wd1770[dnr].reg[WD1770_STATUS];
        break; 
      case 1:
      case 2:
        tmp = wd1770[dnr].reg[addr];
        break;
      case 3:
        if (wd1770[dnr].data_buffer_index < 0)
            tmp = wd1770[dnr].reg[addr];
        else
            tmp = wd1770[dnr].data_buffer[wd1770[dnr].data_buffer_index--];
        break;
    }
    /*drive0_traceflg = 1;*/
#ifdef WD_DEBUG
    printf("WD READ ADDR: %i DATA:%x\n",addr,tmp);
#endif
    return tmp;
}

static void reset_wd1770(int dnr)
{
    int i;

    wd1770[dnr].busy_clk = (CLOCK)0;
    wd1770[dnr].motor_spinup_clk = (CLOCK)0;
    wd1770[dnr].current_track = 20;
    wd1770[dnr].data_buffer_index = -1;

    for (i = 0; i < 4; i++)
        /* FIXME: Just a wild guess.  */
        wd1770[dnr].reg[i] = 0;

}

/*-----------------------------------------------------------------------*/
/* WD1770 commands, common routines.  No need to inline these functions, as
   the real WD1770 needs ages to execute commands anyway.  */

static void wd1770_update_track_register(BYTE command, int dnr)
{
    if (command & 0x10)
        wd1770[dnr].reg[WD1770_TRACK] = wd1770[dnr].current_track;
}

static void wd1770_motor_control(BYTE command, int dnr)
{    
    if (command & 0x08) {
        if ((wd1770[dnr].reg[WD1770_STATUS] & 0x80) == 0)
            wd1770[dnr].motor_spinup_clk = drive_clk[dnr];
        wd1770[dnr].reg[WD1770_STATUS] |= 0x80;
    } else {
        /* Clear motor and spinup flag.  */
        wd1770[dnr].reg[WD1770_STATUS] &= 0x5f;
        wd1770[dnr].motor_spinup_clk = (CLOCK)0;
    }
}

/*-----------------------------------------------------------------------*/
/* WD1770 commands.  */

static void wd1770_command_restore(BYTE command, int dnr)
{
    wd1770[dnr].current_track = 0;
    wd1770_update_track_register(0x10, dnr);
    wd1770_motor_control(command, dnr);
}

static void wd1770_command_seek(BYTE command, int dnr)
{

}

static void wd1770_command_step(BYTE command, int dnr)
{

}

static void wd1770_command_stepin(BYTE command, int dnr)
{
    if (wd1770[dnr].current_track < 79)
        wd1770[dnr].current_track++;
    wd1770_update_track_register(command, dnr);
    wd1770_motor_control(command, dnr);
}

static void wd1770_command_stepout(BYTE command, int dnr)
{
    if (wd1770[dnr].current_track > 0)
        wd1770[dnr].current_track--;
    wd1770_update_track_register(command, dnr);
    wd1770_motor_control(command, dnr);
}

static void wd1770_command_readsector(BYTE command, int dnr)
{

}

static void wd1770_command_writesector(BYTE command, int dnr)
{

}

static void wd1770_command_readaddress(BYTE command, int dnr)
{
    wd1770[dnr].data_buffer[0] = 0xff;
    wd1770[dnr].data_buffer[1] = 0xff;
    wd1770[dnr].data_buffer[2] = 0x2;
    wd1770[dnr].data_buffer[3] = 0x1;
    wd1770[dnr].data_buffer[4] = 0x0;
    wd1770[dnr].data_buffer[5] = 0x0;

    wd1770[dnr].data_buffer_index = 5;
}

static void wd1770_command_forceint(BYTE command, int dnr)
{
    /* Abort any command immediately.  Clear status bits.  */
    wd1770[dnr].reg[WD1770_STATUS] = 0;
}

static void wd1770_command_readtrack(BYTE command, int dnr)
{

}

static void wd1770_command_writetrack(BYTE command, int dnr)
{

}

