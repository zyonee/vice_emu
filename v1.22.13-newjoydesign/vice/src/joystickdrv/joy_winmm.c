#include <windows.h>

#include "joystick.h"

typedef struct joy_winmm_priv_s {
    UINT uJoyID;
    JOYCAPS     joy_caps;
} joy_winmm_priv_t;

static void joystick_winmm_poll(int idx, void* dev)
{
    JOYINFOEX joy_info;
    joy_winmm_priv_t* priv = (joy_winmm_priv_t*)dev;
    BYTE value;

    MMRESULT result = joyGetPosEx(priv->uJoyID, &joy_info);
    if (result == JOYERR_NOERROR) {
        if (joy_info.dwXpos <= priv->joy_caps.wXmin
            + (priv->joy_caps.wXmax - priv->joy_caps.wXmin) / 4) {
            value |= 4;
        }
        if (joy_info.dwXpos >= priv->joy_caps.wXmin
            + (priv->joy_caps.wXmax - priv->joy_caps.wXmin) / 4 * 3) {
            value |= 8;
        }
        if (joy_info.dwYpos <= priv->joy_caps.wYmin
            + (priv->joy_caps.wYmax - priv->joy_caps.wYmin) / 4) {
            value |= 1;
        }
        if (joy_info.dwYpos >= priv->joy_caps.wYmin
            + (priv->joy_caps.wYmax - priv->joy_caps.wYmin) / 4 * 3) {
            value |= 2;
        }
        if (joy_info.dwButtons)
            value |= 16;
        joystick_set_value_absolute(idx + 1, value);
    }
}

static joy_device_t winmm_joystick_device =
{
    joystick_winmm_poll,
    NULL,
    NULL,
    NULL
};

int joy_winmm_init(void)
{
    UINT wNumDevs = joyGetNumDevs();
    UINT i;

    for (i = 0; i < wNumDevs && i < 16; i++)
    {
        joy_data_t* dev;
        joy_winmm_priv_t* priv = lib_malloc(sizeof(joy_winmm_priv_t));
        MMRESULT result = joyGetDevCaps(JOYSTICKID1, &priv->joy_caps, sizeof(JOYCAPS));
        if (result != JOYERR_NOERROR) {
            lib_free(priv);
        }
        else {
            priv->uJoyID = i;
            dev = lib_malloc(sizeof(joy_data_t));
            dev->name = lib_msprintf("Hardware joystick in port %u (Windows Multimedia)", i);
            dev->priv = priv;
            dev->device = &winmm_joystick_device;
            joystick_register_device(dev);
        }
    }
    return 1;
}

