/*
 * joy.c - SDL joystick support.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 *  Ulmer Lionel <ulmer@poly.polytechnique.fr>
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

#include <SDL/SDL.h>
#include <stdio.h>

#include "cmdline.h"
#include "joy.h"
#include "joystick.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "types.h"

#define JOYSTICK_THRESHOLD 10000

static log_t sdljoy_log = LOG_ERR;

/* (Used by `keyboard.c').  */
int joystick_port_map[2];

/* Total number of joysticks */
static int num_joysticks;

/* Actions to perform on joystick input */
typedef enum {
    NONE = 0,
    /* Joystick movement or button press */
    JOYSTICK,
    /* Keyboard key press */
    KEYBOARD,
    /* (De)Activate virtual keyboard */
    VIRTUAL_KBD,
    /* (De)Activate UI */
    UI_ACTIVATE,
    /* Call UI function */
    UI_FUNCTION
} sdljoystick_action_t;

/* State and input mapping for each direction/button/etc */
struct sdljoystick_input_s {
    /* Previous value */
    int previous;
    /* Action to perform */
    sdljoystick_action_t action;
    union {
        /* joy[0] = port number (0,1), joy[1] = pin number */
        BYTE joy[2];
        /* key[0] = row, key[1] = column */
        BYTE key[2];
        /* TODO */
        int ui_function;
    } value;
};
typedef struct sdljoystick_input_s sdljoystick_input_t;

struct sdljoystick_s {
    SDL_Joystick *joyptr;
    const char *name;
    int axis_max;
    int button_max;
    int hat_max;
    int ball_max;
    sdljoystick_input_t *axis;
    sdljoystick_input_t *button;
    sdljoystick_input_t *hat;
    sdljoystick_input_t *ball;
};
typedef struct sdljoystick_s sdljoystick_t;

static sdljoystick_t *sdljoystick;

/* Resources.  */

static int joyport1select(int val, void *param)
{
    joystick_port_map[0] = val;
    return 0;
}

static int joyport2select(int val, void *param)
{
    joystick_port_map[1] = val;
    return 0;
}

static const resource_int_t resources_int[] = {
    { "JoyDevice1", 0, RES_EVENT_NO, NULL,
      &joystick_port_map[0], joyport1select, NULL },
    { "JoyDevice2", 0, RES_EVENT_NO, NULL,
      &joystick_port_map[1], joyport2select, NULL },
    { NULL },
};

/* Command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL, "JoyDevice1", NULL,
      "<0-4>", N_("Set device for joystick port 1") },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL, "JoyDevice2", NULL,
      "<0-4>", N_("Set device for joystick port 2") },
    { NULL },
};

int joystick_arch_init_resources(void)
{
fprintf(stderr,"%s\n",__func__);
    return resources_register_int(resources_int);
}

int joystick_init_cmdline_options(void)
{
fprintf(stderr,"%s\n",__func__);
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/**********************************************************
 * Generic high level joy routine                         *
 **********************************************************/
int joy_arch_init(void)
{
    int i, axis, button, hat, ball;
    SDL_Joystick *joy;

    sdljoystick = NULL;

    sdljoy_log = log_open("SDLJoystick");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
        log_error(sdljoy_log,"Subsystem init failed!");
        return -1;
    }

    num_joysticks = SDL_NumJoysticks();

    if(num_joysticks == 0) {
        log_message(sdljoy_log,"No joysticks found");
        return 0;
    }

    log_message(sdljoy_log,"%i joysticks found",num_joysticks);

    sdljoystick = (sdljoystick_t *)lib_malloc(sizeof(sdljoystick_t) * num_joysticks);

    for(i=0; i<num_joysticks; ++i) {
        joy = sdljoystick[i].joyptr = SDL_JoystickOpen(i);
        if(joy) {
            sdljoystick[i].name = SDL_JoystickName(i);
            axis = sdljoystick[i].axis_max = SDL_JoystickNumAxes(joy);
            button = sdljoystick[i].button_max = SDL_JoystickNumButtons(joy);
            hat = sdljoystick[i].hat_max = SDL_JoystickNumHats(joy);
            ball = sdljoystick[i].ball_max = SDL_JoystickNumBalls(joy);

            if(axis > 0) {
                sdljoystick[i].axis = (sdljoystick_input_t *)lib_malloc(sizeof(sdljoystick_input_t) * axis * 2);
            } else {
                sdljoystick[i].axis = NULL;
            }

            if(button > 0) {
                sdljoystick[i].button = (sdljoystick_input_t *)lib_malloc(sizeof(sdljoystick_input_t) * button);
            } else {
                sdljoystick[i].button = NULL;
            }

            if(hat > 0) {
                sdljoystick[i].hat = (sdljoystick_input_t *)lib_malloc(sizeof(sdljoystick_input_t) * hat);
            } else {
                sdljoystick[i].hat = NULL;
            }

            if(ball > 0) {
                sdljoystick[i].ball = (sdljoystick_input_t *)lib_malloc(sizeof(sdljoystick_input_t) * ball);
            } else {
                sdljoystick[i].ball = NULL;
            }

            joy_arch_init_default_mapping(i);

            log_message(sdljoy_log,"\"%s\" (%i axes, %i buttons, %i hats, %i balls)",
                        sdljoystick[i].name,  axis, button, hat, ball);
        } else {
            log_warning(sdljoy_log,"Couldn't open joystick %i",i);
        }
    }

    SDL_JoystickEventState(SDL_ENABLE);
    return 0;
}

void joystick_close(void)
{
    int i;
fprintf(stderr,"%s\n",__func__);

    if(sdljoystick == NULL)
        return;

    for(i=0; i<num_joysticks; ++i) {
        if(sdljoystick[i].joyptr)
            SDL_JoystickClose(sdljoystick[i].joyptr);
        lib_free(sdljoystick[i].axis);
        lib_free(sdljoystick[i].button);
        lib_free(sdljoystick[i].hat);
        lib_free(sdljoystick[i].ball);
    }

    lib_free(sdljoystick);
}

/* ------------------------------------------------------------------------- */

void joy_arch_init_default_mapping(int joynum)
{
    int i, joyport, pin;

    for(i=0; i<sdljoystick[joynum].axis_max*2; ++i) {
        joyport = ((1+joynum+(i&4))&1);
        pin = 8>>(i&3);
        sdljoystick[joynum].axis[i].action = JOYSTICK;
        sdljoystick[joynum].axis[i].value.joy[0] = joyport;
        sdljoystick[joynum].axis[i].value.joy[1] = pin;
    }

    for(i=0; i<sdljoystick[joynum].button_max; ++i) {
        switch(i&3) {
            case 0:
            case 1:
            default:
                joyport = ((1+joynum+(i&1))&1);
                pin = 1<<4;
                sdljoystick[joynum].button[i].action = JOYSTICK;
                sdljoystick[joynum].button[i].value.joy[0] = joyport;
                sdljoystick[joynum].button[i].value.joy[1] = pin;
                break;
            case 2:
                sdljoystick[joynum].button[i].action = UI_ACTIVATE;
                break;
            case 3:
                sdljoystick[joynum].button[i].action = VIRTUAL_KBD;
                break;
        }
    }

    for(i=0; i<sdljoystick[joynum].hat_max; ++i) {
        sdljoystick[joynum].button[i].action = NONE;
    }

    for(i=0; i<sdljoystick[joynum].ball_max; ++i) {
        sdljoystick[joynum].button[i].action = NONE;
    }
}

/* ------------------------------------------------------------------------- */

void sdljoy_perform_event(sdljoystick_input_t *event, int value)
{
    BYTE t;

    switch(event->action) {
        case JOYSTICK:
            t = event->value.joy[0];
            if(joystick_port_map[t] == JOYDEV_JOYSTICK) {
                if(value) {
                    joystick_set_value_or(t+1, event->value.joy[1]);
                } else {
                    joystick_set_value_and(t+1, ~(event->value.joy[1]));
                }
            }
            break;
        case KEYBOARD:
            keyboard_set_keyarr(event->value.key[0], event->value.key[1], value);
            break;
        case VIRTUAL_KBD:
        case UI_ACTIVATE:
        case UI_FUNCTION:
        case NONE:
        default:
            break;
    }
}

void sdljoy_axis_event(Uint8 joynum, Uint8 axis, Sint16 value)
{
    int cur, prev, index;

    if(value < -JOYSTICK_THRESHOLD) {
        cur = -1;
    } else if(value > JOYSTICK_THRESHOLD) {
        cur = 1;
    } else {
        cur = 0;
    }

    index = axis*2;
    prev = sdljoystick[joynum].axis[index].previous;

    if(cur == prev)
        return;

    if(cur == 1) {
        if(prev == -1) {
            sdljoy_perform_event(&(sdljoystick[joynum].axis[index+1]), 0);
        }
        sdljoy_perform_event(&(sdljoystick[joynum].axis[index]), 1);
    } else if(cur == -1) {
        if(prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].axis[index]), 0);
        }
        sdljoy_perform_event(&(sdljoystick[joynum].axis[index+1]), 1);
    } else {
        if(prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].axis[index]), 0);
        } else {
            sdljoy_perform_event(&(sdljoystick[joynum].axis[index+1]), 0);
        }
    }

    sdljoystick[joynum].axis[index].previous = cur;
}

void sdljoy_button_event(Uint8 joynum, Uint8 button, Uint8 value)
{
    sdljoy_perform_event(&(sdljoystick[joynum].button[button]), value);
}

