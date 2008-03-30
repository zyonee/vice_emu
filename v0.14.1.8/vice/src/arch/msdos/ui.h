/*
 * ui.h - A (very) simple user interface for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#ifndef _UI_DOS_H
#define _UI_DOS_H

#include "types.h"

extern int UiInit(int *argc, char **argv);
extern int UiInitFinish(void);
extern void UiError(const char *format, ...);
extern int UiJamDialog(const char *format, ...);
extern void UiShowText(const char *title, const char *text);
extern void UiUpdateMenus(void);
extern void UiAutoRepeatOn(void);
extern void UiAutoRepeatOff(void);
extern void UiMain(ADDRESS);
extern void UiToggleDriveStatus(int state);
extern void UiDisplayDriveTrack(double track_number);
extern void UiDisplayDriveLed(int status);

#endif
