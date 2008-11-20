/*
 * ser-eeprom.h
 *
 * Written by
 *  Groepaz/Hitmen <groepaz@gmx.net>
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

#ifndef SER_EEPROM
#define SER_EEPROM

#include "types.h"
#include <stdio.h>

void eeprom_data_readadvance(void);
BYTE eeprom_data_readbyte(void);
BYTE eeprom_data_readbit(void);
BYTE eeprom_data_read(void);
void eeprom_cmd_reset(void);
void eeprom_cmd_write(BYTE value);
void eeprom_seq_reset(void);
void eeprom_seq_write(BYTE value);
int  eeprom_execute_command(int eeprom_mode);
void eeprom_port_write(int clk,int data,int ddr,int status);
FILE *eeprom_open_image(char *name);
void eeprom_close_image(void);

#endif
