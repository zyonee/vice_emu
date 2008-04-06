/*
 * dlg-drive.c - The drive-dialog.
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

#define INCL_WINBUTTONS
#define INCL_WINDIALOGS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS

#include "ui_status.h"

#include "vice.h"
#include "dialogs.h"

#include <string.h>

#include "log.h"
#include "drive.h"
#include "attach.h"
#include "fliplist.h"
#include "resources.h"

/* Dialog procedures                                                */
/*----------------------------------------------------------------- */

static int toggle_drive_res(char *format, int drive)
{
    char tmp[80];
    sprintf(tmp, format, drive+8);
    return toggle(tmp);
}

static int get_drive_res(char *format, int drive)
{
    char tmp[80];
    int val;
    sprintf(tmp, format, drive+8);
    resources_get_value(tmp, (resource_value_t*) &val);
    return val;
}

static void set_drive_res(char *format, int drive, int val)
{
    char tmp[80];
    sprintf(tmp, format, drive+8);
    resources_set_value(tmp, (resource_value_t) val);
}

#define nDRIVES 9

const char driveName[nDRIVES][28] = {
    "None",
    "1541, 5\"1/4, SS",
    "1541-II, 5\"1/4 SS",
    "1571, 5\"1/4, DS",
    "1581, 3\"1/2, DS",
    "2031, 5\"1/4, SS, IEEE488",
    "1001, single drive, 1M/disk",
    "8050, dual drive, 0.5M/disk",
    "8250, dual drive, 1M/disk"
};

const int driveRes[nDRIVES] = {
    DRIVE_TYPE_NONE,
    DRIVE_TYPE_1541,
    DRIVE_TYPE_1541II,
    DRIVE_TYPE_1571,
    DRIVE_TYPE_1581,
    DRIVE_TYPE_2031,
    DRIVE_TYPE_1001,
    DRIVE_TYPE_8050,
    DRIVE_TYPE_8250
};

static int driveTypeNo(int type)
{
    int i;
    for (i=0; i<nDRIVES; i++)
        if (driveRes[i]==type)
            return i;
    return -1;
}

static MRESULT EXPENTRY pm_drive(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    static int drive = 0;
    static int first = TRUE;

    //    static char image_hist[10][CCHMAXPATH];

    switch (msg)
    {
    case WM_INITDLG:
        setDlgOpen(DLGO_DRIVE);
        first = TRUE;
        break;
    case WM_CLOSE:
        delDlgOpen(DLGO_DRIVE);
        break;
    case WM_PAINT:
        {
            if (first) {
                int val, i=0;
                while (i<10 && ui_status.imageHist[i][0])
                    WinLboxInsertItem(hwnd, CBS_IMAGE, ui_status.imageHist[i++]);
                WinLboxInsertItem(hwnd, CBS_IMAGE, "");
                for (i=0; i<nDRIVES; i++)
                    WinLboxInsertItem(hwnd, CBS_TYPE, driveName[i]);
                resources_get_value("DriveTrueEmulation", (resource_value_t*) &val);
                WinCheckButton(hwnd, CB_TRUEDRIVE, val);
                resources_get_value("VideoStandard", (resource_value_t*) &val);
                WinCheckButton(hwnd, val==DRIVE_SYNC_PAL?RB_PAL:RB_NTSC, 1);
                WinCheckButton(hwnd, RB_DRIVE8|drive, 1);
                WinSendMsg(hwnd, WM_SWITCH, (void*)drive, 0);
                first=FALSE;
            }
        }
        break;
    case WM_COMMAND:
        switch (LONGFROMMP(mp1))
        {
        case DID_CLOSE:
            delDlgOpen(DLGO_DRIVE);
            break;
        case PB_CREATE:
            create_dialog(hwnd);
            return FALSE;
        case PB_ATTACH:
            attach_dialog(hwnd, drive+8);
            return FALSE;
        case PB_DETACH:
            file_system_detach_disk(drive+8);
            return FALSE;
        }
        break;
    case WM_CONTROL:
        {
            switch (SHORT1FROMMP(mp1))
            {
            case RB_DRIVE8:
            case RB_DRIVE9:
            case RB_DRIVE10:
            case RB_DRIVE11:
                WinSendMsg(hwnd, WM_SWITCH, (void*)(SHORT1FROMMP(mp1)&0x3), 0);
                break;
            case CB_TRUEDRIVE:
                toggle("DriveTrueEmulation");
                break;
            case CB_CONVERTP00:
                WinEnableControl(hwnd, CB_HIDENONP00,
                                 toggle_drive_res("FSDevice%dConvertP00", drive));
                break;
            case CB_SAVEP00:
                toggle_drive_res("FileDevice%dSaveP00", drive);
                break;
            case CB_ALLOWACCESS:
                toggle_drive_res("FileSystemDevice%d", drive);
                break;
            case CB_HIDENONP00:
                toggle_drive_res("FSDevice%dHideCBMFiles", drive);
                break;
            case CB_PARALLEL:
                if (drive==0 || drive==1)
                    toggle_drive_res("Drive%dParallelCable", drive);
                break;
            case RB_NEVER:
            case RB_ASK:
            case RB_ALWAYS:
                if (drive==0 || drive==1)
                    set_drive_res("Drive%dExtendImagePolicy", drive,
                                  SHORT1FROMMP(mp1)&0x3);
                break;
            case RB_NONE:
            case RB_TRAP:
            case RB_SKIP:
                if (drive==0 || drive==1)
                    set_drive_res("Drive%dIdleMethod", drive,
                                  SHORT1FROMMP(mp1)&0x3);
                break;
            case RB_PAL:
                resources_set_value("VideoStandard",
                                    (resource_value_t*) DRIVE_SYNC_PAL);
                break;
            case RB_NTSC:
                resources_set_value("VideoStandard",
                                    (resource_value_t*) DRIVE_SYNC_NTSC);
                break;
           case CBS_IMAGE:
                {
                    if (SHORT2FROMMP(mp1)==CBN_ENTER)
                    {
                        char psz[CCHMAXPATH];
                        WinLboxQuerySelectedItemText(hwnd, CBS_IMAGE, psz, CCHMAXPATH);

                        if (strlen(psz))
                        {
                            if (file_system_attach_disk(drive+8, psz))
                                WinMessageBox(HWND_DESKTOP, hwnd,
                                              "Cannot attach specified file.",
                                              "VICE/2 Error", 0, MB_OK);
                        }
                        else
                            file_system_detach_disk(drive+8);
                    }
                }
                return FALSE;
            case CBS_TYPE:
                log_debug("CBS_TYPE %d %d", SHORT2FROMMP(mp1)==CBN_ENTER, drive);
                if (SHORT2FROMMP(mp1)==CBN_ENTER && (drive==0 || drive==1))
                {
                    log_debug("set_type: %d %d",
                              WinLboxQuerySelectedItem(hwnd, CBS_TYPE),
                              driveRes[WinLboxQuerySelectedItem(hwnd, CBS_TYPE)]);
                    set_drive_res("Drive%dType", drive,
                                  driveRes[WinLboxQuerySelectedItem(hwnd, CBS_TYPE)]);
                }
                return FALSE;
            }
        }
        break;
    case WM_DRIVEIMAGE:
        {
            int pos;
            for (pos=0; pos<9; pos++) WinLboxDeleteItem(hwnd, CBS_IMAGE, 0);
            pos=0;
            while (pos<10 && ui_status.imageHist[pos][0])
                WinLboxInsertItem(hwnd, CBS_IMAGE, ui_status.imageHist[pos++]);
            WinLboxInsertItem(hwnd, CBS_IMAGE, "");
            if (drive==(int)mp2)
                WinLboxSelectItem(hwnd, CBS_IMAGE, *((char*)mp1)?0:pos);
        }
        return FALSE;
    case WM_SWITCH:
        drive = (int)mp1;
        {
            int drive89=(drive==0 || drive==1);
            if (drive89)
            {
                WinCheckButton(hwnd, RB_NEVER |
                               get_drive_res("Drive%dExtendImagePolicy",
                                             drive), 1);
                WinCheckButton(hwnd, RB_NONE |
                               get_drive_res("Drive%dIdleMethod",
                                             drive), 1);
            }
            else
            {
                WinCheckButton(hwnd, RB_NEVER,  0);
                WinCheckButton(hwnd, RB_ASK,    0);
                WinCheckButton(hwnd, RB_ALWAYS, 0);
                WinCheckButton(hwnd, RB_NONE,   0);
                WinCheckButton(hwnd, RB_SKIP,   0);
                WinCheckButton(hwnd, RB_TRAP,   0);
            }
            WinCheckButton(hwnd, CB_PARALLEL, drive89 ?
                           (get_drive_res("Drive%dParallelCable", drive)!=0):0);
            WinLboxSelectItem(hwnd, CBS_TYPE, drive89 ?
                              driveTypeNo(get_drive_res("Drive%dType", drive)):0);
            WinEnableControl(hwnd, CB_PARALLEL, drive89);
            WinEnableControl(hwnd, RB_NEVER,    drive89);
            WinEnableControl(hwnd, RB_ASK,      drive89);
            WinEnableControl(hwnd, RB_ALWAYS,   drive89);
            WinEnableControl(hwnd, RB_NONE,     drive89);
            WinEnableControl(hwnd, RB_SKIP,     drive89);
            WinEnableControl(hwnd, RB_TRAP,     drive89);
            WinEnableControl(hwnd, CBS_TYPE,    drive89);
            WinCheckButton(hwnd, CB_SAVEP00,
                           get_drive_res("FSDevice%dSaveP00", drive)!=0);
            WinCheckButton(hwnd, CB_ALLOWACCESS,
                           get_drive_res("FileSystemDevice%d", drive)!=0);
            WinCheckButton(hwnd, CB_HIDENONP00,
                           get_drive_res("FSDevice%dHideCBMFiles", drive)!=0);
            {
                int conv=(get_drive_res("FSDevice%dConvertP00", drive)!=0);
                WinCheckButton  (hwnd, CB_CONVERTP00, conv);
                WinEnableControl(hwnd, CB_HIDENONP00, conv);
            }
            {
                char tmp[CCHMAXPATH];
                int max=WinLboxQueryCount(hwnd, CBS_IMAGE);
                int pos=-1;
                do WinLboxQueryItem(hwnd, CBS_IMAGE, ++pos, tmp, CCHMAXPATH);
                while (pos<max && strcmp(ui_status.lastImage[drive], tmp));
                WinLboxSelectItem(hwnd, CBS_IMAGE, pos);
            }
        }
        return FALSE;

    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
}

/* call to open dialog                                              */
/*----------------------------------------------------------------- */

HWND hwndDrive;

void drive_dialog(HWND hwnd)
{
    if (dlgOpen(DLGO_DRIVE)) return;
    hwndDrive=WinLoadDlg(HWND_DESKTOP, hwnd, pm_drive, NULLHANDLE,
                         DLG_DRIVE, NULL);
}


