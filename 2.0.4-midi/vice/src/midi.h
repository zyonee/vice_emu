/*
 * midi.h - MIDI emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andr. Fachat <a.fachat@physik.tu-chemnitz.de>
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

#ifndef _MIDI_H
#define _MIDI_H

#include "types.h"

extern void midi_init(void);
extern void midi_reset(void);

extern BYTE REGPARM1 midi_read(WORD a);
extern void REGPARM2 midi_store(WORD a, BYTE b);

extern int midi_resources_init(void);
extern void midi_resources_shutdown(void);
extern int midi_cmdline_options_init(void);

/* Emulated interfaces */
#define MIDI_MODE_SEQUENTIAL 0  /* Sequential Circuits Inc. */
#define MIDI_MODE_PASSPORT   1  /* Passport & Syntech */
#define MIDI_MODE_DATEL      2  /* DATEL/Siel/JMS */
#define MIDI_MODE_NAMESOFT   3  /* Namesoft */

#endif
