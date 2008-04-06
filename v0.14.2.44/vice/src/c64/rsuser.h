

/*
 * pruser.h - Printer device for userport.
 *
 * Written by
 *  Andr� Fachat        (a.fachat@physik.tu-chemnitz.de)
 *
 * Patches by
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

#ifndef _RSUSER_H_
#define _RSUSER_H_

#define	RTS_OUT		0x02
#define	DTR_OUT		0x04

#define	DCD_IN		0x10
#define	CTS_IN		0x40
#define	DSR_IN		0x80

void rsuser_init(void);

int rsuser_init_resources(void);
int rsuser_init_cmdline_options(void);

int rsuser_enabled;

void userport_serial_write_sr(BYTE);
void userport_serial_write_ctrl(int);
BYTE userport_serial_read_ctrl();

int int_rsuser(long offset);

#endif

