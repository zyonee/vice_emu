/*
 * ui.h - A user interface for OS/2.
 *
 * Written by
 *  Thomas Bretz <tbretz@gsi.de>
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

#ifndef _UI_STATUS_H
#define _UI_STATUS_H


#undef BYTE
#undef ADDRESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
#undef ADDRESS
#define ADDRESS WORD

#include "ui.h"

typedef struct _ui_status
{
    HPS   hps;
    RECTL rectl;
    UINT  step;
    UINT  width, height; // in steps
    UINT  xOffset, yOffset;
    BOOL  init;
    float lastSpeed;
    float lastFps;
    float lastTrack[4];
    CHAR  lastImage[4][CCHMAXPATH];
    CHAR  imageHist[10][CCHMAXPATH];
    int   lastTapeMotor;
    int   lastTapeStatus;
    int   lastTapeCounter;
    int   lastTapeCtrlStat;
    ui_drive_enable_t lastDriveState;

} ui_status_t;

extern ui_status_t ui_status;
extern int         PM_winActive;
extern HMTX        hmtxKey;

void PM_status(void *unused);
void ui_open_status_window(void);
void ui_draw_status_window(HWND hwnd);

void ui_set_rectl_lrtb(RECTL *rectl, int nr, int left, int right, int top, int bottom);
void ui_set_rectl_lrth(RECTL *rectl, int nr, int left, int right, int top, int height);
void ui_set_rectl_lwtb(RECTL *rectl, int nr, int left, int width, int top, int bottom);
void ui_set_rectl_lwth(RECTL *rectl, int nr, int left, int width, int top, int height);

void ui_display_speed(float spd, float fps);

#endif
