/*
 * main_exit.c - VICE shutdown.
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

#include <stdio.h>
#include <signal.h>

#include "main_exit.h"

#include "log.h"

void main_exit(void)
{
    /* Disable SIGINT.  This is done to prevent the user from keeping C-c
       pressed and thus breaking the cleanup process, which might be
       dangerous.  */
    signal(SIGINT, SIG_IGN);

    log_message(LOG_DEFAULT, "\nExiting...");
    //---    resources_set_value("Sound", (resource_value_t)FALSE);
    //---    DosSleep(500);

    //---    machine_shutdown();
    //    video_free();
    //    sound_close(); // Be sure sound device is closed.
    // Maybe we need some DosSleep(500)...

    //---#ifdef HAS_JOYSTICK
    //---    joystick_close();
    //---#endif
}

