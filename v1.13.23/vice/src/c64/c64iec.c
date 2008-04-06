/*
 * c64iec.c - IEC bus handling for the C64.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "c64.h"
#include "c64cart.h"
#include "c64iec.h"
#include "cartridge.h"
#include "drive.h"
#include "drivecpu.h"
#include "drivetypes.h"
#include "iecdrive.h"
#include "types.h"


/* Status of the IEC bus signals.  */
extern iec_info_t iec_info;


void c64iec_init(void)
{
    memset(&iec_info, 0xff, sizeof(iec_info_t));
    iec_info.drive_port = 0x85;
}

void iec_update_cpu_bus(BYTE data)
{
    iec_info.cpu_bus = (((data << 2) & 0x80)
                       | ((data << 2) & 0x40)
                       | ((data << 1) & 0x10));
}

void iec_update_ports(void)
{
    iec_info.cpu_port = iec_info.cpu_bus & iec_info.drive_bus
                        & iec_info.drive2_bus;
    iec_info.drive_port = iec_info.drive2_port = (((iec_info.cpu_port >> 4)
                          & 0x4)
                          | (iec_info.cpu_port >> 7)
                          | ((iec_info.cpu_bus << 3) & 0x80));
}

void iec_update_ports_embedded(void)
{
    iec_update_ports();
}

void iec_drive0_write(BYTE data)
{
    iec_info.drive_bus = (((data << 3) & 0x40)
                         | ((data << 6) & ((~data ^ iec_info.cpu_bus) << 3)
                         & 0x80));
    iec_info.drive_data = data;
    iec_update_ports();
}

void iec_drive1_write(BYTE data)
{
    iec_info.drive2_bus = (((data << 3) & 0x40)
                          | ((data << 6) & ((~data ^ iec_info.cpu_bus) << 3)
                          & 0x80));
    iec_info.drive2_data = data;
    iec_update_ports();
}

BYTE iec_drive0_read(void)
{
    return iec_info.drive_port;
}

BYTE iec_drive1_read(void)
{
    return iec_info.drive2_port;
}

#if 0
BYTE iec_cpu_read(void)
{
    if (!(drive_context[0]->drive->enable)
        && !(drive_context[1]->drive->enable))
        return (iec_info.iec_fast_1541 & 0x30) << 2;

    drivecpu_execute_all(maincpu_clk);

    return iec_info.cpu_port;
}
#endif

iec_info_t *iec_get_drive_port(void)
{
    return &iec_info;
}

/* This function is called from ui_update_menus() */
int iec_available_busses(void)
{
    return IEC_BUS_IEC
        | ((c64cart_type == CARTRIDGE_IEEE488) ? IEC_BUS_IEEE : 0);
}

