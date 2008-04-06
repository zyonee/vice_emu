/*
 * c610mem.h - CBM-II memory handling.
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#ifndef _C610MEM_H
#define _C610MEM_H

#include "types.h"

#define C610_RAM_SIZE		0x100000	/* maximum 1M */
#define C610_ROM_SIZE		0x10000		/* complete bank 15 */
#define C610_CHARGEN_ROM_SIZE	0x2000

extern int c610_mem_init_resources(void);
extern int c610_mem_init_cmdline_options(void);

extern void set_bank_exec(int val);
extern void set_bank_ind(int val);
extern int cbm2_set_model(const char *model, void *extra);
extern const char *cbm2_get_model(void);

extern int cbm2_init_ok;

extern void mem_reset(void);

extern void cbm2_set_tpi1ca(int);
extern void cbm2_set_tpi1cb(int);
extern void cbm2_set_tpi2pc(BYTE);

extern void c500_set_phi1_bank(int b);
extern void c500_set_phi2_bank(int b);

#endif

