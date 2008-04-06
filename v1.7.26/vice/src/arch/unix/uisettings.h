/*
 * uisettings.h - Implementation of common UI settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef _UI_SETTINGS_H
#define _UI_SETTINGS_H

#include "ui.h"

extern struct ui_menu_entry_s ui_performance_settings_menu[];
extern struct ui_menu_entry_s ui_video_settings_menu[];
extern struct ui_menu_entry_s ui_vic_video_settings_menu[];
extern struct ui_menu_entry_s video_settings_submenu[];
extern struct ui_menu_entry_s ui_fullscreen_settings_menu[];
extern struct ui_menu_entry_s ui_fullscreen_settings_submenu[];
extern struct ui_menu_entry_s ui_keyboard_settings_menu[];
extern struct ui_menu_entry_s ui_settings_settings_menu[];
extern UI_CALLBACK(ui_set_romset);
extern UI_CALLBACK(ui_load_romset);
extern UI_CALLBACK(ui_load_rom_file);
extern UI_CALLBACK(ui_unload_rom_file);
extern UI_CALLBACK(ui_dump_romset);

#endif

