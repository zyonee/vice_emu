/*
 * c64iec.c - IEC bus handling for the C64.
 *
 * Written by
 *  Daniel Sladic (sladic@eecg.toronto.edu)
 *  Andreas Boose (boose@unixserv.rz.fh-hannover.de)
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
 *  Teemu Rantanen (tvr@cs.hut.fi)
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
#include "resources.h"
#include "viad.h"
#include "types.h"
#include "iecdrive.h"
#include "true1541.h"
#include "c64cia.h"

static iec_info_t iec_info;

static BYTE parallel_cable_cpu_value = 0xff;
static BYTE parallel_cable_drive_value = 0xff;

inline static void update_ports(void)
{
    iec_info.cpu_port = iec_info.cpu_bus & iec_info.drive_bus;
    iec_info.drive_port = (((iec_info.cpu_port >> 4) & 0x4)
		  | (iec_info.cpu_port >> 7)
		  | ((iec_info.cpu_bus << 3) & 0x80));
}

void iec_drive_write(BYTE data)
{
    iec_info.drive_bus = (((data << 3) & 0x40)
		 | ((data << 6) & ((~data ^ iec_info.cpu_bus) << 3) & 0x80));
    iec_info.drive_data = data;
    update_ports();
}

BYTE iec_drive_read(void)
{
    return iec_info.drive_port;
}


/* The C64 has all bus lines in one I/O byte in a CIA.  If this byte is read
   or modified, these routines are called.  */

void iec_cpu_write(BYTE data)
{
    if (!true1541_enabled) {
	iec_info.iec_fast_1541 = data;
	return;
    }

    true1541_cpu_execute();

    iec_info.cpu_bus = (((data << 2) & 0x80)
	       | ((data << 2) & 0x40)
	       | ((data << 1) & 0x10));

    /* FIXME: this is slow, we should avoid doing it when not necessary.  */
    set_atn(!(iec_info.cpu_bus & 0x10));

    iec_info.drive_bus = (((iec_info.drive_data << 3) & 0x40)
		 | ((iec_info.drive_data << 6)
		    & ((~iec_info.drive_data ^ iec_info.cpu_bus) << 3)
		    & 0x80));

    update_ports();
}

BYTE iec_cpu_read(void)
{
    if (!true1541_enabled)
	return ((iec_info.iec_fast_1541 & 0x30) << 2);

    true1541_cpu_execute();
    return iec_info.cpu_port;
}

iec_info_t *iec_get_drive_port(void)
{
    return &iec_info;
}

void parallel_cable_drive_write(BYTE data, int handshake)
{
    if (handshake)
	cia2_set_flag();
    parallel_cable_drive_value = data;
}

BYTE parallel_cable_drive_read(int handshake)
{
    if (handshake)
	cia2_set_flag();
    return parallel_cable_cpu_value & parallel_cable_drive_value;
}

void parallel_cable_cpu_write(BYTE data, int handshake)
{
    if (!true1541_enabled)
        return;

    true1541_cpu_execute();
    if (handshake)
	viaD1_signal(VIA_SIG_CB1, VIA_SIG_FALL);
    parallel_cable_cpu_value = data;
}

BYTE parallel_cable_cpu_read(void)
{
    if (!true1541_enabled)
        return 0;

    true1541_cpu_execute();
    viaD1_signal(VIA_SIG_CB1, VIA_SIG_FALL);
    return parallel_cable_cpu_value & parallel_cable_drive_value;
}


