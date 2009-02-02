#ifndef DINPUT_HANDLE_H
#define DINPUT_HANDLE_H

#include "config.h"

#ifdef HAVE_DINPUT
#define DIRECTINPUT_VERSION     0x0500
#include <dinput.h>

LPDIRECTINPUT get_directinput_handle();
#endif

#endif