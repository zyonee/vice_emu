/*
 * uirs232.h
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _UI_RS232_H
#define _UI_RS232_H

#include "ui.h"

extern struct ui_menu_entry_s rs232_submenu[];

extern struct ui_menu_entry_s ser1_baud_submenu[];
extern struct ui_menu_entry_s ser2_baud_submenu[];
extern struct ui_menu_entry_s acia1_device_submenu[];
extern struct ui_menu_entry_s rsuser_device_submenu[];

extern UI_CALLBACK(set_rs232_device_file);
extern UI_CALLBACK(set_rs232_exec_file);
extern UI_CALLBACK(set_rs232_dump_file);

#endif

