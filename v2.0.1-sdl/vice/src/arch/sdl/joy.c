/*
 * joy.c - Linux/BSD joystick support.
 *
 * Written by
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 *  Ulmer Lionel <ulmer@poly.polytechnique.fr>
 *
 * Patches by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *
 * NetBSD support by
 *  Krister Walfridsson <cato@df.lth.se>
 *
 * 1.1.xxx Linux API by
 *   Luca Montecchiani  <m.luca@usa.net> (http://i.am/m.luca)
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
#include "log.h"
#include "resources.h"
#include "types.h"

static log_t sdljoy_log = LOG_ERR;

/* (Used by `kbd.c').  */
int joystick_port_map[2];

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
      "<0-8>", N_("Set device for joystick port 1") },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL, "JoyDevice2", NULL,
      "<0-8>", N_("Set device for joystick port 2") },
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
    int num, i;
    SDL_Joystick *joy;

    sdljoy_log = log_open("SDLJoystick");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
        log_error(sdljoy_log,"Subsystem init failed!");
        return -1;
    }

    num = SDL_NumJoysticks();

    if(num == 0) {
        log_message(sdljoy_log,"No joysticks found");
        return 0;
    }

    log_message(sdljoy_log,"%i joysticks found",num);

    for(i=0; i<num; ++i) {
        joy = SDL_JoystickOpen(i);
        if(joy) {
            log_message(sdljoy_log,"%s, (%i axes, %i buttons)",
                        SDL_JoystickName(i),
                        SDL_JoystickNumAxes(joy),
                        SDL_JoystickNumButtons(joy)
                        );
        } else {
            log_warning(sdljoy_log,"Couldn't open joystick %i",i);
        }
    }

    SDL_JoystickEventState(SDL_ENABLE);
    return 0;
}

void joystick_close(void)
{
fprintf(stderr,"%s\n",__func__);
}

void joystick(void)
{
}

