/*
 * joystick.c - Joystick support for Windows.
 *
 * Written by
 *  Tibor Biczo         (crown@mail.matav.hu)
 *  Ettore Perazzoli    (ettore@comm2000.it)
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

#include "vice.h"

#include "cmdline.h"
#include "resources.h"

#include <winerror.h>
#include <ddraw.h>
#include <dinput.h>
#include <mmsystem.h>

#include "joystick.h"
#include "winmain.h"
#include "ui.h"

int joystick_inited;

/* Notice that this has to be `int' to make resources work.  */
static int keyset1[9], keyset2[9];

/* ------------------------------------------------------------------------ */

/* Joystick devices.  */
static joystick_device_t joystick_device_1, joystick_device_2;

static int set_joystick_device_1(resource_value_t v)
{
    joystick_device_t dev = (joystick_device_t) v;

    joystick_device_1 = dev;
    return 0;
}

static int set_joystick_device_2(resource_value_t v)
{
    joystick_device_t dev = (joystick_device_t) v;

    joystick_device_2 = dev;
    return 0;
}

#define DEFINE_SET_KEYSET(num, dir)                             \
    static int set_keyset##num##_##dir##(resource_value_t v)    \
    {                                                           \
        keyset##num##[KEYSET_##dir##] = (int) v;                \
                                                                \
        return 0;                                               \
    }

DEFINE_SET_KEYSET(1, NW)
DEFINE_SET_KEYSET(1, N)
DEFINE_SET_KEYSET(1, NE)
DEFINE_SET_KEYSET(1, E)
DEFINE_SET_KEYSET(1, SE)
DEFINE_SET_KEYSET(1, S)
DEFINE_SET_KEYSET(1, SW)
DEFINE_SET_KEYSET(1, W)
DEFINE_SET_KEYSET(1, FIRE)

DEFINE_SET_KEYSET(2, NW)
DEFINE_SET_KEYSET(2, N)
DEFINE_SET_KEYSET(2, NE)
DEFINE_SET_KEYSET(2, E)
DEFINE_SET_KEYSET(2, SE)
DEFINE_SET_KEYSET(2, S)
DEFINE_SET_KEYSET(2, SW)
DEFINE_SET_KEYSET(2, W)
DEFINE_SET_KEYSET(2, FIRE)

static resource_t resources[] = {
    { "JoyDevice1", RES_INTEGER, (resource_value_t) JOYDEV_NONE,
      (resource_value_t *) &joystick_device_1, set_joystick_device_1 },
    { "JoyDevice2", RES_INTEGER, (resource_value_t) JOYDEV_NONE,
      (resource_value_t *) &joystick_device_2, set_joystick_device_2 },
    { "KeySet1NorthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_NW], set_keyset1_NW },
    { "KeySet1North", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_N], set_keyset1_N },
    { "KeySet1NorthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_NE], set_keyset1_NE },
    { "KeySet1East", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_E], set_keyset1_E },
    { "KeySet1SouthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_SE], set_keyset1_SE },
    { "KeySet1South", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_S], set_keyset1_S },
    { "KeySet1SouthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_SW], set_keyset1_SW },
    { "KeySet1West", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_W], set_keyset1_W },
    { "KeySet1Fire", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset1[KEYSET_FIRE], set_keyset1_FIRE },
    { "KeySet2NorthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_NW], set_keyset2_NW },
    { "KeySet2North", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_N], set_keyset2_N },
    { "KeySet2NorthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_NE], set_keyset2_NE },
    { "KeySet2East", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_E], set_keyset2_E },
    { "KeySet2SouthEast", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_SE], set_keyset2_SE },
    { "KeySet2South", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_S], set_keyset2_S },
    { "KeySet2SouthWest", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_SW], set_keyset2_SW },
    { "KeySet2West", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_W], set_keyset2_W },
    { "KeySet2Fire", RES_INTEGER, (resource_value_t) K_NONE,
      (resource_value_t *) &keyset2[KEYSET_FIRE], set_keyset2_FIRE },
    { NULL }
};

int joystick_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

static cmdline_option_t cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice1", NULL,
      "<number>", "Set input device for joystick #1" },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL,
      "JoyDevice2", NULL,
      "<number>", "Set input device for joystick #2" },
    { NULL }
};

int joystick_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

LPDIRECTINPUT               di;
int                         directinput_inited;
extern LPDIRECTINPUTDEVICE  di_mouse;
void mouse_set_format(void);

#if 0
static BOOL CALLBACK EnumCallBack(LPCDIDEVICEINSTANCE lpddi, LPVOID pvref)
{
    log_debug("DirectInput device found : %d",lpddi->dwDevType);
    log_debug(lpddi->tszProductName);
    log_debug(lpddi->tszInstanceName);
    return DIENUM_CONTINUE;
}
#endif

int joystick_init(void)
{
HRESULT result;

    result=DirectInputCreate(winmain_instance,0x0300,&di,NULL);
    if (result!=DI_OK) {
        di=NULL;
        di_mouse=NULL;
        return 0;
    }
    joystick_inited=1;

    /*  Init mouse device */
    result=IDirectInput_CreateDevice(di,(GUID *)&GUID_SysMouse,&di_mouse,NULL);
    if (result!=DI_OK) {
        di_mouse=NULL;
        return 0;
    }
    mouse_set_format();

//    IDirectInput_EnumDevices(di,0,EnumCallBack,0,0);
    return 0;
}

int joystick_close(void)
{
    if (di_mouse!=NULL) {
        IDirectInputDevice_Unacquire(di_mouse);
        IDirectInputDevice_Release(di_mouse);
        di_mouse=NULL;
    }
    if (di!=NULL) IDirectInput_Release(di);
    joystick_inited=0;
    return 0;
}

JOYINFOEX   joy_info;
JOYCAPS     joy_caps;

void joystick_update(void)
{
int         value;
MMRESULT    result;

    if ((joystick_device_1==JOYDEV_HW1) || (joystick_device_2==JOYDEV_HW1)) {
        value=0;
        joy_info.dwSize=sizeof(JOYINFOEX);
        joy_info.dwFlags=JOY_RETURNBUTTONS | JOY_RETURNCENTERED | JOY_RETURNX | JOY_RETURNY;
        result=joyGetPosEx(JOYSTICKID1,&joy_info);
        if (result==JOYERR_NOERROR) {
            result=joyGetDevCaps(JOYSTICKID1,&joy_caps,sizeof(JOYCAPS));
            if (result==JOYERR_NOERROR) {
                if (joy_info.dwXpos<=joy_caps.wXmin+(joy_caps.wXmax-joy_caps.wXmin)/4) {
                    value|=4;
                }
                if (joy_info.dwXpos>=joy_caps.wXmin+(joy_caps.wXmax-joy_caps.wXmin)/4*3) {
                    value|=8;
                }
                if (joy_info.dwYpos<=joy_caps.wYmin+(joy_caps.wYmax-joy_caps.wYmin)/4) {
                    value|=1;
                }
                if (joy_info.dwYpos>=joy_caps.wYmin+(joy_caps.wYmax-joy_caps.wYmin)/4*3) {
                    value|=2;
                }
                if (joy_info.dwButtons) {
                    value|=16;
                }
                if (joystick_device_1==JOYDEV_HW1) {
                    joystick_value[1]=value;
                }
                if (joystick_device_2==JOYDEV_HW1) {
                    joystick_value[2]=value;
                }
            }
        }
    }

    if ((joystick_device_1==JOYDEV_HW2) || (joystick_device_2==JOYDEV_HW2)) {
        value=0;
        joy_info.dwSize=sizeof(JOYINFOEX);
        joy_info.dwFlags=JOY_RETURNBUTTONS | JOY_RETURNCENTERED | JOY_RETURNX | JOY_RETURNY;
        result=joyGetPosEx(JOYSTICKID2,&joy_info);
        if (result==JOYERR_NOERROR) {
            result=joyGetDevCaps(JOYSTICKID2,&joy_caps,sizeof(JOYCAPS));
            if (result==JOYERR_NOERROR) {
                if (joy_info.dwXpos<=joy_caps.wXmin+(joy_caps.wXmax-joy_caps.wXmin)/4) {
                    value|=4;
                }
                if (joy_info.dwXpos>=joy_caps.wXmin+(joy_caps.wXmax-joy_caps.wXmin)/4*3) {
                    value|=8;
                }
                if (joy_info.dwYpos<=joy_caps.wYmin+(joy_caps.wYmax-joy_caps.wYmin)/4) {
                    value|=1;
                }
                if (joy_info.dwYpos>=joy_caps.wYmin+(joy_caps.wYmax-joy_caps.wYmin)/4*3) {
                    value|=2;
                }
                if (joy_info.dwButtons) {
                    value|=16;
                }
                if (joystick_device_1==JOYDEV_HW2) {
                    joystick_value[1]=value;
                }
                if (joystick_device_2==JOYDEV_HW2) {
                    joystick_value[2]=value;
                }
            }
        }
    }
}

/* Joystick-through-keyboard.  */

int handle_keyset_mapping(joystick_device_t device, int *set,
                          kbd_code_t kcode, int pressed)
{
    if (joystick_device_1 == device || joystick_device_2 == device) {
        BYTE value = 0;

        if (kcode == set[KEYSET_NW])    /* North-West */
            value = 5;
        else if (kcode == set[KEYSET_N]) /* North */
            value = 1;
        else if (kcode == set[KEYSET_NE]) /* North-East */
            value = 9;
        else if (kcode == set[KEYSET_E]) /* East */
            value = 8;
        else if (kcode == set[KEYSET_SE]) /* South-East */
            value = 10;
        else if (kcode == set[KEYSET_S]) /* South */
            value = 2;
        else if (kcode == set[KEYSET_SW]) /* South-West */
            value = 6;
        else if (kcode == set[KEYSET_W]) /* West */
            value = 4;
        else if (kcode == set[KEYSET_FIRE]) /* Fire */
            value = 16;
        else
            return 0;

        if (pressed) {
            if (joystick_device_1 == device)
                joystick_value[1] |= value;
            if (joystick_device_2 == device)
                joystick_value[2] |= value;
        } else {
            if (joystick_device_1 == device)
                joystick_value[1] &= ~value;
            if (joystick_device_2 == device)
                joystick_value[2] &= ~value;
        }
        return 1;
    }

    return 0;
}

int joystick_handle_key(kbd_code_t kcode, int pressed)
{
    int value = 0;

    /* The numpad case is handled specially because it allows users to use
       both `5' and `2' for "down".  */
    if (joystick_device_1 == JOYDEV_NUMPAD
        || joystick_device_2 == JOYDEV_NUMPAD) {

        switch (kcode) {
          case K_KP7:               /* North-West */
            value = 5;
            break;
          case K_KP8:               /* North */
            value = 1;
            break;
          case K_KP9:               /* North-East */
            value = 9;
            break;
          case K_KP6:               /* East */
            value = 8;
            break;
          case K_KP3:               /* South-East */
            value = 10;
            break;
          case K_KP2:               /* South */
          case K_KP5:
            value = 2;
            break;
          case K_KP1:               /* South-West */
            value = 6;
            break;
          case K_KP4:               /* West */
            value = 4;
            break;
          case K_KP0:
          case K_RIGHTCTRL:
            value = 16;
            break;
          default:
            /* (make compiler happy) */
            ;
        }

        if (pressed) {
            if (joystick_device_1 == JOYDEV_NUMPAD)
                joystick_value[1] |= value;
            if (joystick_device_2 == JOYDEV_NUMPAD)
                joystick_value[2] |= value;
        } else {
            if (joystick_device_1 == JOYDEV_NUMPAD)
                joystick_value[1] &= ~value;
            if (joystick_device_2 == JOYDEV_NUMPAD)
                joystick_value[2] &= ~value;
        }
    }

    /* (Notice we have to handle all the keysets even when one key is used
       more than once (the most intuitive behavior), so we use `|' instead of
       `||'.)  */
    return (value
            | handle_keyset_mapping(JOYDEV_KEYSET1, keyset1, kcode, pressed)
            | handle_keyset_mapping(JOYDEV_KEYSET2, keyset2, kcode, pressed));
}

void joystick_calibrate(HWND hwnd)
{
    IDirectInput_RunControlPanel(di,hwnd,0);
}