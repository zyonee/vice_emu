/*
 * dlg-joystick.c - The joystick-dialog.
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

#define INCL_WININPUT     // WM_CHAR
#define INCL_WINBUTTONS
#define INCL_WINDIALOGS
#define INCL_WINSTDSPIN
#define INCL_WINFRAMEMGR  // WM_TANSLATEACCEL
#include "vice.h"

#include <os2.h>

#include "dialogs.h"
#include "dlg-joystick.h"

#include <ctype.h>        // isprint
#include <stdlib.h>       // free

#include "log.h"
#include "utils.h"        // xmsprintf
#include "joystick.h"
#include "resources.h"
#include "snippets\pmwin2.h"

#ifdef HAS_JOYSTICK

#define JOY_ALL (CB_JOY11|CB_JOY12|CB_JOY21|CB_JOY22)
#define JOY_PORT1 0x100
#define JOY_PORT2 0x200

#define JOYDEV_ALL (JOYDEV_HW1|JOYDEV_HW2|JOYDEV_NUMPAD| \
                    JOYDEV_KEYSET1|JOYDEV_KEYSET2)

extern int number_joysticks;

static HWND hwndCalibrate=NULLHANDLE;
static HWND hwndKeyset   =NULLHANDLE;

static MRESULT EXPENTRY pm_joystick(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    const int ID_ON  = 1;
    const int ID_OFF = 2;

    switch (msg)
    {
    case WM_INITDLG:
        {
            int joy1, joy2;
            //
            // disable controls of non existing joysticks
            // remark: I think this cannot change while runtime
            //
            if (!(number_joysticks&JOYDEV_HW1))
            {
                WinEnableControl(hwnd, CB_JOY11, 0);
                WinEnableControl(hwnd, CB_JOY12, 0);
            }
            if (!(number_joysticks&JOYDEV_HW2))
            {
                WinEnableControl(hwnd, CB_JOY21, 0);
                WinEnableControl(hwnd, CB_JOY22, 0);
            }
            if (number_joysticks==0)
                WinEnableControl(hwnd, ID_CALIBRATE, 0);

            resources_get_value("JoyDevice1", (resource_value_t*) &joy1);
            resources_get_value("JoyDevice2", (resource_value_t*) &joy2);
            WinSendMsg(hwnd, WM_SETCBS, (void*)joy1, (void*)joy2);
        }
        break;

    case WM_DESTROY:
    case WM_CLOSE:
        {
            if (WinIsWindowVisible(hwndCalibrate))
                WinSendMsg(hwndCalibrate, WM_CLOSE, 0, 0);

            if (WinIsWindowVisible(hwndKeyset))
                WinSendMsg(hwndKeyset, WM_CLOSE, 0, 0);
        }
        break;

    case WM_COMMAND:
        switch(LONGFROMMP(mp1))
        {
        case DID_CLOSE:
        {
            if (WinIsWindowVisible(hwndCalibrate))
                WinSendMsg(hwndCalibrate, WM_CLOSE, 0, 0);

            if (WinIsWindowVisible(hwndKeyset))
                WinSendMsg(hwndKeyset, WM_CLOSE, 0, 0);
        }
            break;

        case ID_SWAP:
            {
                int joy1, joy2;

                resources_get_value("JoyDevice1", (resource_value_t*) &joy1);
                resources_get_value("JoyDevice2", (resource_value_t*) &joy2);

                resources_set_value("JoyDevice1", (resource_value_t) joy2);
                resources_set_value("JoyDevice2", (resource_value_t) joy1);

                WinSendMsg(hwnd, WM_SETCBS,  (void*)joy2, (void*)joy1);
            }
            return FALSE;
        case ID_CALIBRATE:
            calibrate_dialog(hwnd);
            return FALSE;;
        case ID_KEYSET:
            keyset_dialog(hwnd);
            return FALSE;;
        }
        break;
    case WM_CONTROL:
        {
            int button =SHORT1FROMMP(mp1);
            int state=WinQueryButtonCheckstate(hwnd, button);
            int port = button & JOY_PORT1;
            int joya, joyb;
            resources_get_value(port?"JoyDevice1":"JoyDevice2",
                                (resource_value_t*) &joya);
            resources_get_value(port?"JoyDevice2":"JoyDevice1",
                                (resource_value_t*) &joyb);
            if (state)
                joya |= button & JOYDEV_ALL;
            else
                joya &= ~(button & JOYDEV_ALL);

            resources_set_value(port?"JoyDevice1":"JoyDevice2",
                                (resource_value_t) joya);
            WinSendMsg(hwnd, WM_SETDLGS,
                       (void*)(port?joya:joyb), (void*)(port?joyb:joya));

        }
        break;
    case WM_SETDLGS:
        {
            HWND hwnd2;

            int joy1 = (int)mp1;
            int joy2 = (int)mp2;
            int joys = joy1 | joy2;

            if (WinIsWindowVisible(hwndCalibrate))
                WinSendMsg(hwndCalibrate, WM_SETJOY,
                           (MPARAM)(joys&JOYDEV_HW1),
                           (MPARAM)(joys&JOYDEV_HW2));

            if (WinIsWindowVisible(hwndKeyset))
                WinSendMsg(hwndKeyset, WM_SETKEY,
                           (MPARAM)(joys&JOYDEV_KEYSET1),
                           (MPARAM)(joys&JOYDEV_KEYSET2));
        }
        break;
    case WM_SETCBS:
        WinCheckButton(hwnd, CB_JOY11,   (JOYDEV_HW1     & (int)mp1) ? 1 : 0);
        WinCheckButton(hwnd, CB_JOY12,   (JOYDEV_HW1     & (int)mp2) ? 1 : 0);
        WinCheckButton(hwnd, CB_JOY21,   (JOYDEV_HW2     & (int)mp1) ? 1 : 0);
        WinCheckButton(hwnd, CB_JOY22,   (JOYDEV_HW2     & (int)mp2) ? 1 : 0);
        WinCheckButton(hwnd, CB_NUMJOY1, (JOYDEV_NUMPAD  & (int)mp1) ? 1 : 0);
        WinCheckButton(hwnd, CB_NUMJOY2, (JOYDEV_NUMPAD  & (int)mp2) ? 1 : 0);
        WinCheckButton(hwnd, CB_KS1JOY1, (JOYDEV_KEYSET1 & (int)mp1) ? 1 : 0);
        WinCheckButton(hwnd, CB_KS1JOY2, (JOYDEV_KEYSET1 & (int)mp2) ? 1 : 0);
        WinCheckButton(hwnd, CB_KS2JOY1, (JOYDEV_KEYSET2 & (int)mp1) ? 1 : 0);
        WinCheckButton(hwnd, CB_KS2JOY2, (JOYDEV_KEYSET2 & (int)mp2) ? 1 : 0);
        break;
    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
}

static MRESULT EXPENTRY pm_calibrate(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    static int joy1 = TRUE;

    switch (msg)
    {
    case WM_INITDLG:
        {
            int j1, j2;

            resources_get_value("JoyDevice1", (resource_value_t*) &j1);
            resources_get_value("JoyDevice2", (resource_value_t*) &j2);
            WinSendMsg(hwnd, WM_PROCESS,
                       (void*)!!((j1|j2)&JOYDEV_HW1),
                       (void*)!!((j1|j2)&JOYDEV_HW2));
        }
        break;

    case WM_COMMAND:
        {
            int cmd = LONGFROMMP(mp1);
            switch (cmd)
            {
            case ID_START:
            case ID_RESET:
            case ID_STOP:
                if (joy1) set_joyA_autoCal(NULL,   (void*)(cmd!=ID_STOP));
                else      set_joyB_autoCal(NULL,   (void*)(cmd!=ID_STOP));
                WinSendMsg(hwnd, WM_ENABLECTRL, 0, (void*)(cmd!=ID_STOP));
                WinSendMsg(hwnd, WM_FILLSPB, 0, 0);
                return FALSE;
            }
            break;
        }
    case WM_CONTROL:
        {
            int ctrl = SHORT1FROMMP(mp1);
            switch (ctrl)
            {
            case RB_JOY1:
            case RB_JOY2:
                if (!(ctrl==RB_JOY1 && joy1) && !(ctrl==RB_JOY2 && !joy1))
                {
                    joy1 = !joy1;
                    WinSendMsg(hwnd, WM_ENABLECTRL, 0, (void*)(get_joy_autoCal(joy1?0:1)));
                    WinSendMsg(hwnd, WM_FILLSPB, 0, 0);
                }
                break;
            case SPB_UP:
            case SPB_DOWN:
            case SPB_LEFT:
            case SPB_RIGHT:
                if (SHORT2FROMMP(mp1)==SPBN_ENDSPIN)
                {
                    const ULONG val = WinGetSpinVal((HWND)mp2);
                    resources_set_value(ctrl==SPB_UP  ? (joy1?"joyAup"  :"joyBup")  :
                                        ctrl==SPB_DOWN? (joy1?"joyAdown":"joyBdown"):
                                        ctrl==SPB_LEFT? (joy1?"joyAleft":"joyBleft"):
                                        (joy1?"joyAright":"joyBright"), (resource_value_t)val);
                }
                break;
            }
            break;
        }
    case WM_PROCESS:
        if (((int)mp1^(int)mp2)&1)
            joy1 = (int)mp1;

        WinCheckButton(hwnd, joy1?RB_JOY1:RB_JOY2, 1);
        WinEnableControl(hwnd, RB_JOY1, (ULONG)mp1);
        WinEnableControl(hwnd, RB_JOY2, (ULONG)mp2);

        WinSendMsg(hwnd, WM_ENABLECTRL, (void*)(!mp1 && !mp2),
                   (void*)get_joy_autoCal(joy1?0:1));
        WinSendMsg(hwnd, WM_FILLSPB, 0, 0);
        break;
    case WM_SETJOY:
        {
            ULONG state1 = mp1?1:0;
            ULONG state2 = mp2?1:0;
            WinEnableControl(hwnd, RB_JOY1, state1);
            WinEnableControl(hwnd, RB_JOY2, state2);
            WinSendMsg(hwnd, WM_PROCESS, (void*)state1, (void*)state2);
        }
        break;
    case WM_ENABLECTRL:
        WinEnableControl(hwnd, ID_START,  mp1?FALSE: !mp2);
        WinEnableControl(hwnd, ID_STOP,   mp1?FALSE:!!mp2);
        WinEnableControl(hwnd, ID_RESET,  mp1?FALSE:!!mp2);
        WinEnableControl(hwnd, SPB_UP,    mp1?FALSE: !mp2);
        WinEnableControl(hwnd, SPB_DOWN,  mp1?FALSE: !mp2);
        WinEnableControl(hwnd, SPB_LEFT,  mp1?FALSE: !mp2);
        WinEnableControl(hwnd, SPB_RIGHT, mp1?FALSE: !mp2);
        return FALSE;
    case WM_FILLSPB:
        {
            int val;
            resources_get_value(joy1?"JoyAup":"JoyBup", (resource_value_t *) &val);
            WinSetDlgSpinVal(hwnd, SPB_UP, val);
            resources_get_value(joy1?"JoyAdown":"JoyBdown", (resource_value_t *) &val);
            WinSetDlgSpinVal(hwnd, SPB_DOWN, val);
            resources_get_value(joy1?"JoyAleft":"JoyBleft", (resource_value_t *) &val);
            WinSetDlgSpinVal(hwnd, SPB_LEFT, val);
            resources_get_value(joy1?"JoyAright":"JoyBright", (resource_value_t *) &val);
            WinSetDlgSpinVal(hwnd, SPB_RIGHT,val);
        }
        return FALSE;
    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
}

static MRESULT EXPENTRY pm_keyset(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    static int set1 = TRUE;

    static int id = 0;

    switch (msg)
    {
    case WM_INITDLG:
        {
            int j1, j2;

            resources_get_value("JoyDevice1", (resource_value_t*) &j1);
            resources_get_value("JoyDevice2", (resource_value_t*) &j2);
            WinSendMsg(hwnd, WM_KPROCESS,
                       (void*)!!((j1|j2)&JOYDEV_KEYSET1),
                       (void*)!!((j1|j2)&JOYDEV_KEYSET2));

            log_debug("Keyset: %x", hwnd);
        }
        break;

    case WM_CONTROL:
        {
            int ctrl = SHORT1FROMMP(mp1);
            switch (ctrl)
            {
            case RB_SET1:
            case RB_SET2:
                if (!(ctrl==RB_SET1 && set1) && !(ctrl==RB_SET2 && !set1))
                {
                    set1 = !set1;
                    WinSendMsg(hwnd, WM_KENABLECTRL, 0, 0);
                    WinSendMsg(hwnd, WM_KFILLSPB, 0, 0);
                }
                break;
            case SPB_N:
            case SPB_NE:
            case SPB_E:
            case SPB_SE:
            case SPB_S:
            case SPB_SW:
            case SPB_W:
            case SPB_NW:
            case SPB_FIRE:
                if (SHORT2FROMMP(mp1)==SPBN_SETFOCUS)
                    id = ctrl;
                break;
            }
            break;
        }
    case WM_KPROCESS:
        if (((int)mp1^(int)mp2)&1)
            set1 = (int)mp1;

        WinCheckButton(hwnd, set1?RB_SET1:RB_SET2, 1);
        WinEnableControl(hwnd, RB_SET1, (ULONG)mp1);
        WinEnableControl(hwnd, RB_SET2, (ULONG)mp2);

        WinSendMsg(hwnd, WM_KENABLECTRL, (void*)(!mp1 && !mp2), NULL);
        WinSendMsg(hwnd, WM_KFILLSPB, 0, 0);
        break;
    case WM_TRANSLATEACCEL:
        {
            if (SHORT1FROMMP(((QMSG*)mp1)->mp1)&KC_ALT)
                break;
            else
                return FALSE;
        }
    case WM_CHAR:
        {
            char *out;
            CHAR usScancode = CHAR4FROMMP(mp1);   //fsFlags&KC_SCANCODE
            //USHORT fsFlags    = SHORT1FROMMP(mp1);
            //CHAR   usKeycode  = SHORT1FROMMP(mp2);  //fsFlags&KC_CHAR
            //CHAR   usVK       = SHORT2FROMMP(mp2);  //fsFlags&KC_VIRTUALKEY VK_TAB/LEFT/RIGHT/UP/DOWN

            switch (id)
            {
            case SPB_N:
                resources_set_value(set1?"KeySet1North":"KeySet2North",
                                    (resource_value_t) usScancode);
                break;
            case SPB_NE:
                resources_set_value(set1?"KeySet1NorthEast":"KeySet2NorthEast",
                                    (resource_value_t) usScancode);
                break;
            case SPB_E:
                resources_set_value(set1?"KeySet1East":"KeySet2East",
                                    (resource_value_t) usScancode);
                break;
           case SPB_SE:
                resources_set_value(set1?"KeySet1SouthEast":"KeySet2SouthEast",
                                    (resource_value_t) usScancode);
                break;
            case SPB_S:
                resources_set_value(set1?"KeySet1South":"KeySet2South",
                                    (resource_value_t) usScancode);
                break;
            case SPB_SW:
                resources_set_value(set1?"KeySet1SouthWest":"KeySet2SouthWest",
                                    (resource_value_t) usScancode);
                break;
            case SPB_W:
                resources_set_value(set1?"KeySet1West":"KeySet2West",
                                    (resource_value_t) usScancode);
                break;
            case SPB_NW:
                resources_set_value(set1?"KeySet1NorthWest":"KeySet2NorthWest",
                                    (resource_value_t) usScancode);
                break;
            case SPB_FIRE:
                resources_set_value(set1?"KeySet1Fire":"KeySet2Fire",
                                    (resource_value_t) usScancode);
                break;
            default:
                return FALSE;
            }
            out = xmsprintf("#%d", usScancode);
            WinSendDlgMsg(hwnd, id, SPBM_SETARRAY, &out, 1);
            WinSetDlgSpinVal(hwnd, id, 0);
            free(out);
        }
        break;

    case WM_SETKEY:
        {
            ULONG state1 = mp1?1:0;
            ULONG state2 = mp2?1:0;

            WinEnableControl(hwnd, RB_SET1, state1);
            WinEnableControl(hwnd, RB_SET2, state2);
            WinSendMsg(hwnd, WM_KPROCESS, (void*)state1, (void*)state2);
        }
        break;

    case WM_KENABLECTRL:
        WinEnableControl(hwnd, SPB_N,  mp1?0:1);
        WinEnableControl(hwnd, SPB_NE, mp1?0:1);
        WinEnableControl(hwnd, SPB_E,  mp1?0:1);
        WinEnableControl(hwnd, SPB_SE, mp1?0:1);
        WinEnableControl(hwnd, SPB_S,  mp1?0:1);
        WinEnableControl(hwnd, SPB_SW, mp1?0:1);
        WinEnableControl(hwnd, SPB_W,  mp1?0:1);
        WinEnableControl(hwnd, SPB_NW, mp1?0:1);
        WinEnableControl(hwnd, SPB_FIRE, mp1?0:1);
        return FALSE;

    case WM_KFILLSPB:
        {
            int val;
            char *msg;
            resources_get_value(set1?"KeySet1North":"KeySet2North",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_N, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_N, 0);
            free(msg);
            resources_get_value(set1?"KeySet1NorthEast":"KeySet2NorthEast",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_NE, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_NE, 0);
            free(msg);
            resources_get_value(set1?"KeySet1East":"KeySet2East",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_E, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_E, 0);
            free(msg);
            resources_get_value(set1?"KeySet1SouthEast":"KeySet2SouthEast",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_SE, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_SE, 0);
            free(msg);
            resources_get_value(set1?"KeySet1South":"KeySet2South",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_S, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_S, 0);
            free(msg);
            resources_get_value(set1?"KeySet1SouthWest":"KeySet2SouthWest",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_SW, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_SW, 0);
            free(msg);
            resources_get_value(set1?"KeySet1West":"KeySet2West",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_W, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_W, 0);
            free(msg);
            resources_get_value(set1?"KeySet1NorthWest":"KeySet2NorthWest",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_NW, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_NW, 0);
            free(msg);
            resources_get_value(set1?"KeySet1Fire":"KeySet2Fire",
                                (resource_value_t*) &val);
            msg=xmsprintf("#%d", val);
            WinSendDlgMsg(hwnd, SPB_FIRE, SPBM_SETARRAY, &msg, 1);
            WinSetDlgSpinVal(hwnd, SPB_FIRE, 0);
            free(msg);
        }
        return FALSE;
    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
}
#endif

/* call to open dialog                                              */
/*----------------------------------------------------------------- */

#ifdef HAS_JOYSTICK
void joystick_dialog(HWND hwnd)
{
    static HWND hwnd2 = NULLHANDLE;

    if (WinIsWindowVisible(hwnd2))
        return;

    hwnd2 = WinLoadStdDlg(hwnd, pm_joystick, DLG_JOYSTICK, NULL);
}

void calibrate_dialog(HWND hwnd)
{
    if (WinIsWindowVisible(hwndCalibrate))
        return;

    hwndCalibrate = WinLoadStdDlg(hwnd, pm_calibrate, DLG_CALIBRATE, NULL);
}

void keyset_dialog(HWND hwnd)
{
    if (WinIsWindowVisible(hwndKeyset))
        return;

    hwndKeyset = WinLoadStdDlg(hwnd, pm_keyset, DLG_KEYSET, NULL);
}
#endif

