/*
 * vic20.c - IEC bus handling for the VIC20.
 *
 * Written by
 *  Daniel Sladic (sladic@eecg.toronto.edu)
 *  Andr� Fachat (fachat@physik.tu-chemnitz.de)
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

#ifndef _VIC20IEC_H
#define _VIC20IEC_H

#include "types.h"
#include "iecdrive.h"

extern BYTE iec_pa_read(void);
extern void iec_pa_write(BYTE data);
void iec_pcr_write(BYTE data);

/* These are dummies.  */
extern void parallel_cable_cpu_write(BYTE data, int handshake);
extern BYTE parallel_cable_cpu_read(void);

extern iec_info_t *iec_get_drive_port(void);

#endif
