#include <dinput.h>

#include "joystick.h"
#include "maincpu.h"
#include "ui.h"
#include "winmain.h"

LPDIRECTINPUT di;

typedef struct _JoyAxis {
    struct _JoyAxis *next;
    DWORD           id;
    char            *name;
    DWORD           dwOffs;
} JoyAxis;

typedef struct _JoyButton {
    struct _JoyButton   *next;
    DWORD               id;
    char                *name;
    DWORD               dwOffs;
} JoyButton;

typedef struct _JoyInfo {
    GUID            guid;
    JoyAxis         *axes;
    JoyButton       *buttons;
    LPDIRECTINPUTDEVICE joystick_di_devices;
    LPDIRECTINPUTDEVICE2  joystick_di_devices2;
    int joystick_fire_speed;
    int joystick_fire_axis;
    int joystick_autofire_button;
    int joystick_fire_button;
} JoyInfo;

static BOOL CALLBACK EnumJoyAxes(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
JoyAxis *axis;
JoyInfo *joy;

//    log_debug("Axis, offset : %s %d", lpddoi->tszName, lpddoi->dwOfs);

    joy = (JoyInfo*)pvRef;

    //  Save info about axis
    axis = lib_malloc(sizeof(JoyAxis));
    axis->next = NULL;
    axis->id = DIDFT_GETINSTANCE(lpddoi->dwType);
    axis->name = lib_stralloc(lpddoi->tszName);
    axis->dwOffs = lpddoi->dwOfs;

    //  Link axis into list for this joystick
    if (joy->axes == NULL) {
        joy->axes = axis;
    } else {
        JoyAxis *s;
        s = joy->axes;
        while (s->next != NULL) {
            s = s->next;
        }
        s->next = axis;
    }
    return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumJoyButtons(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
JoyButton *button;
JoyInfo *joy;

//    log_debug("Button, offset : %s %d", lpddoi->tszName, lpddoi->dwOfs);

    joy = (JoyInfo*)pvRef;

    //  Save info about button
    button = lib_malloc(sizeof(JoyButton));
    button->next = NULL;
    button->id = DIDFT_GETINSTANCE(lpddoi->dwType);
    button->name = lib_stralloc(lpddoi->tszName);
    button->dwOffs = lpddoi->dwOfs;

    //  Link button into list for this joystick
    if (joy->buttons == NULL) {
        joy->buttons = button;
    } else {
        JoyButton *s;
        s = joy->buttons;
        while (s->next != NULL) {
            s = s->next;
        }
        s->next = button;
    }
    return DIENUM_CONTINUE;
}

static void joystick_release_axes(JoyAxis *axis)
{
    while (axis != NULL) {
        JoyAxis *next;

        next = axis->next;
        lib_free(axis->name);
        lib_free(axis);
        axis = next;
    }
}

static void joystick_release_buttons(JoyButton *button)
{
    while (button != NULL) {
        JoyButton *next;

        next = button->next;
        lib_free(button->name);
        lib_free(button);
        button = next;
    }
}

static void joystick_di5_update(int joy_no, void* dev)
{
    BYTE value;
    int i;
    DIPROPRANGE    prop;
    UINT amin;
    UINT amax;
    DWORD apos;
    DIJOYSTATE  js;
    JoyInfo *joy = (JoyInfo *)dev;
    JoyButton *button;
    int     afire_button;
    int		fire_button;

    value = 0;

    IDirectInputDevice2_Poll(joy->joystick_di_devices2);
    IDirectInputDevice_GetDeviceState(joy->joystick_di_devices, sizeof(DIJOYSTATE), &js);

    //  Get boundary values for X axis
    prop.diph.dwSize = sizeof(DIPROPRANGE);
    prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    prop.diph.dwObj = 0;    // Offset of X axis
    prop.diph.dwHow = DIPH_BYOFFSET;
    IDirectInputDevice_GetProperty(joy->joystick_di_devices, DIPROP_RANGE, (DIPROPHEADER*)&prop);
    if (js.lX <= prop.lMin + (prop.lMax - prop.lMin) / 4) {
        value |= 4;
    }
    if (js.lX >= prop.lMin + (prop.lMax - prop.lMin) / 4 * 3) {
        value |= 8;
    }

    //  Get boundary values for Y axis
    prop.diph.dwSize = sizeof(DIPROPRANGE);
    prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    prop.diph.dwObj = 4;    // Offset of Y axis
    prop.diph.dwHow = DIPH_BYOFFSET;
    IDirectInputDevice_GetProperty(joy->joystick_di_devices, DIPROP_RANGE, (DIPROPHEADER*)&prop);
    if (js.lY <= prop.lMin + (prop.lMax - prop.lMin) / 4) {
        value |= 1;
    }
    if (js.lY >= prop.lMin + (prop.lMax - prop.lMin) / 4 * 3) {
        value |= 2;
    }

    //  Find the joystick object

    afire_button = -1;
	fire_button = -1;
    if (joy->joystick_autofire_button > 0) {
        button = joy->buttons;
        i = 0;
        while (button && i < joy->joystick_autofire_button - 1) {
            button = button->next;
            i++;
        }
        if (button) {
            afire_button = button->dwOffs - 48;
        }
    }
    if ((afire_button >= 32) || (afire_button < -1)) afire_button = -1;
	if (joy->joystick_fire_button > 0) {
        button = joy->buttons;
        i = 0;
        while (button && i < joy->joystick_fire_button - 1) {
            button = button->next;
            i++;
        }
        if (button) {
            fire_button = button->dwOffs - 48;
        }
	}
    if ((fire_button >= 32) || (fire_button < -1)) fire_button = -1;

	//	If fire button is not in valid range [0..31] then it means every button is
	//	treated as fire button, otherwise the only one selected.
	if (fire_button != -1) {
		if ((fire_button != afire_button) && (js.rgbButtons[fire_button] & 0x80)) value |= 16;
	} else {
		for (i = 0; i < 32; i++) {
			if ((i != afire_button) && (js.rgbButtons[i] & 0x80)) value |= 16;
		}
	}
    if ((afire_button != -1) && (js.rgbButtons[afire_button] & 0x80)) {
        if (joy->joystick_fire_axis) {
            amin = 0;
            amax = 32;
            apos = 16;
            {
                //  Find axis
                JoyAxis *axis;

                axis = joy->axes;
                i = 0;
                while (axis && i < joy->joystick_fire_axis - 1) {
                    axis = axis->next;
                    i++;
                }
                if (axis) {
                    //  Get boundary values for axis
                    prop.diph.dwSize = sizeof(DIPROPRANGE);
                    prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
                    prop.diph.dwObj = axis->id;
                    prop.diph.dwHow = DIPH_BYID;
                    IDirectInputDevice_GetProperty(joy->joystick_di_devices, DIPROP_RANGE, (DIPROPHEADER*)&prop);
                    amin = prop.lMin;
                    amax = prop.lMax;
                    apos = *(DWORD*)(((BYTE*)&js) + axis->dwOffs);
                }
            }
            value |= maincpu_clk / (((amin + apos) * 0x2000)
                    / (amax - amin) + 1) & 16;
        } else {
            value |= (maincpu_clk / (joy->joystick_fire_speed
                    * 0x100)) & 16;
        }
    }
    joystick_set_value_absolute(joy_no, value);
}

int joystick_di_open(void* dev)
{
    JoyInfo *joy = (JoyInfo *)dev;

    IDirectInput_CreateDevice(di, &joy->guid, &joy->joystick_di_devices, NULL);
    IDirectInputDevice_QueryInterface(joy->joystick_di_devices, &IID_IDirectInputDevice2, (LPVOID*)&joy->joystick_di_devices2);
    IDirectInputDevice_SetDataFormat(joy->joystick_di_devices, &c_dfDIJoystick);
    IDirectInputDevice_SetCooperativeLevel(joy->joystick_di_devices, ui_active_window, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    IDirectInputDevice_Acquire(joy->joystick_di_devices);
    if (joy->axes) {
        joystick_release_axes(joy->axes);
        joy->axes = NULL;
    }
    IDirectInputDevice_EnumObjects(joy->joystick_di_devices, EnumJoyAxes, (LPVOID)joy, DIDFT_AXIS);
    if (joy->buttons) {
        joystick_release_buttons(joy->buttons);
        joy->buttons = NULL;
    }
    IDirectInputDevice_EnumObjects(joy->joystick_di_devices, EnumJoyButtons, (LPVOID)joy, DIDFT_BUTTON);
    return 1;
}

void joystick_di_close(void* dev)
{
    JoyInfo *joy = (JoyInfo *)dev;
    if (joy->joystick_di_devices) {
        IDirectInputDevice_Unacquire(joy->joystick_di_devices);
        if (joy->joystick_di_devices2) IDirectInputDevice2_Release(joy->joystick_di_devices2);
        IDirectInputDevice_Release(joy->joystick_di_devices);
    }
    joy->joystick_di_devices = NULL;
    joy->joystick_di_devices2 = NULL;
}

static joy_device_t directinput_joystick_device = {
    joystick_di5_update,
    joystick_di_open,
    joystick_di_close,
    NULL
} ;

static BOOL CALLBACK EnumCallBack(LPCDIDEVICEINSTANCE lpddi, LPVOID pvref)
{
    JoyInfo *new_joystick = lib_malloc(sizeof(JoyInfo));
    joy_data_t* dev = lib_malloc(sizeof(joy_data_t));

    memcpy(&new_joystick->guid, &lpddi->guidInstance, sizeof(GUID));
    new_joystick->axes = NULL;
    new_joystick->buttons = NULL;
    
    dev->name = lib_stralloc(lpddi->tszInstanceName);
    dev->priv = new_joystick;
    dev->device = &directinput_joystick_device;
    joystick_register_device(dev);

    return DIENUM_CONTINUE;
}

int joy_di_init(void)
{
    HRESULT result;

        result = DirectInputCreate(winmain_instance, 0x0500, &di, NULL);
        if (result != DI_OK) {
            di = NULL;
            return 0;
        }
        IDirectInput_EnumDevices(di, DIDEVTYPE_JOYSTICK, EnumCallBack, NULL, DIEDFL_ALLDEVICES);

    return 1;
}

