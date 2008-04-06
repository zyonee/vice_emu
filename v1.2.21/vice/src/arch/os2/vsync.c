/*
 * vsync.c - End-of-frame handling for Vice/2
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

/* This does what has to be done at the end of each screen frame (50 times per
   second on PAL machines). */
#define INCL_DOSPROFILE
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES

#include "vice.h"

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vsync.h"
#include "ui.h"
#include "ui_status.h"
#include "interrupt.h"
#include "maincpu.h"
#include "log.h"
#include "kbdbuf.h"
#include "sound.h"
#include "resources.h"
#include "cmdline.h"
#include "kbd.h"
#include "archdep.h"
#include "clkguard.h"

#include "usleep.h"

#ifdef HAS_JOYSTICK
#include "joystick.h"
#endif

/* ------------------------------------------------------------------------- */

/* Relative speed of the emulation (%).  0 means "don't limit speed".  */
static int relative_speed;

/* Refresh rate.  0 means "auto".  */
static int refresh_rate;

/* "Warp mode".  If nonzero, attempt to run as fast as possible.  */
static int warp_mode_enabled;

/* o if Emulator is paused */
static int emulator_paused=FALSE;

/* FIXME: This should call `set_timers'.  */
static int set_relative_speed(resource_value_t v)
{
    relative_speed = (int) v;
    return 0;
}

static int set_refresh_rate(resource_value_t v)
{
    if ((int) v < 0) return -1;
    refresh_rate = (int) v;
    return 0;
}

static int set_warp_mode(resource_value_t v)
{
    warp_mode_enabled = (int) v;
    sound_set_warp_mode(warp_mode_enabled);
    return 0;
}

/* Vsync-related resources.  */
static resource_t resources[] = {
    { "Speed", RES_INTEGER, (resource_value_t) 100,
      (resource_value_t *) &relative_speed, set_relative_speed },
    { "RefreshRate", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &refresh_rate, set_refresh_rate },
    { "WarpMode", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &warp_mode_enabled, set_warp_mode },
    { NULL }
};

int vsync_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* Vsync-related command-line options.  */
static cmdline_option_t cmdline_options[] = {
    { "-speed", SET_RESOURCE, 1, NULL, NULL, "Speed", NULL,
      "<percent>", "Limit emulation speed to specified value" },
    { "-refresh", SET_RESOURCE, 1, NULL, NULL, "RefreshRate", NULL,
      "<value>", "Update every <value> frames (`0' for automatic)" },
    { "-warp", SET_RESOURCE, 0, NULL, NULL, "WarpMode", (resource_value_t) 1,
      NULL, "Enable warp mode" },
    { "+warp", SET_RESOURCE, 0, NULL, NULL, "WarpMode", (resource_value_t) 0,
      NULL, "Disable warp mode" },
    { NULL }
};

int vsync_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Maximum number of frames we can skip consecutively when adjusting the
   refresh rate dynamically.  */
#define MAX_SKIPPED_FRAMES	10

/* Number of frames per second on the real machine.  */
static double refresh_frequency;

/* Number of clock cycles per seconds on the real machine.  */
static long cycles_per_sec;

/* Function to call at the end of every screen frame.  */
static void (*vsync_hook)(void);

/* ------------------------------------------------------------------------- */

static volatile int elapsed_frames = 0;
static int timer_disabled = 1;
static int timer_speed    = 0;
static int timer_patch    = 0;

static ULONG ulTmrFreq;  // Hertz (almost 1.2MHz at my PC)

inline ULONG gettime(void)
{
    QWORD qwTmrTime;
    DosTmrQueryTime(&qwTmrTime);
    return qwTmrTime.ulLo;
}

static ULONG timer_ticks;
static ULONG timer_time;

void update_elapsed_frames(int want)
{
    ULONG now;
    static ULONG old_now;
    static int pending;

    if (timer_disabled) return;
    if (!want && timer_patch > 0) {
        timer_patch--;
        elapsed_frames++;
    }

    now=gettime();

    /* problems with now<timer_time und now overflow  */
    if (now<old_now) timer_time = 0; /* (old_now-now) */

    while (now>timer_time) {
        if (!pending) {
            if (timer_patch < 0) timer_patch++;
            else                 elapsed_frames++;
        }
        else pending--;
        timer_time += timer_ticks;
    }

    if (want == 1 && !pending) {
        if (timer_patch < 0) timer_patch++;
        else                 elapsed_frames++;
        pending++;
    }

    old_now = now;
}

static int set_timer_speed(int speed)
{
    if (speed > 0) {
        timer_ticks = ((100*ulTmrFreq)/(refresh_frequency*speed));
        timer_time=gettime();
        update_elapsed_frames(-1);
        elapsed_frames = 0;
    }
    else {
        speed       = 0;
        timer_ticks = 0;
    }

    timer_speed    = speed;
    timer_disabled = speed ? 0 : 1;
    return 0;
}

static void timer_sleep(void)
{
    int	old;

    if (timer_disabled) return;
    old = elapsed_frames;
    do {
        update_elapsed_frames(1);
        if (old == elapsed_frames) DosSleep(1);
    } while (old == elapsed_frames);
}

static void patch_timer(int patch)
{
    timer_patch += patch;
}

int vsync_disable_timer(void)
{
/*    log_message(LOG_DEFAULT,"disable timer %i",timer_disabled);
    if (!timer_disabled)
        return set_timer_speed(0);
    else */return 0;
}

/* ------------------------------------------------------------------------- */

static int speed_eval_suspended = 1;
static CLOCK speed_eval_prev_clk;

static void clk_overflow_callback(CLOCK amount, void *data)
{
    speed_eval_prev_clk -= amount;
}

/* This should be called whenever something that has nothing to do with the
   emulation happens, so that we don't display bogus speed values. */
void suspend_speed_eval(void)
{
    //    sound_suspend();
    speed_eval_suspended = 1;
}

void vsync_set_machine_parameter(double refresh_rate, long cycles)
{
    refresh_frequency = refresh_rate;
    cycles_per_sec    = cycles;
}

void vsync_init(void (*hook)(void))
{
    vsync_disable_timer();
    suspend_speed_eval();

    /* What's this - from unix */
    clk_guard_add_callback(&maincpu_clk_guard, clk_overflow_callback, NULL);

    DosTmrQueryFreq(&ulTmrFreq);
    vsync_hook        = hook;
}

static void display_speed(int num_frames)
{
    static ULONG prev_time;
    /* problems with now<timer_time und now overflow         */
    /* if (curr_time<prev_time) prev_time = 0; (old_now-now) */
    if (!speed_eval_suspended) {
        ULONG curr_time = gettime();
        float diff_clk    = clk - speed_eval_prev_clk;
        float time_diff   = (double)(curr_time - prev_time)/ulTmrFreq;
	float speed_index = diff_clk/(time_diff*cycles_per_sec);
	float frame_rate  = num_frames/time_diff;

        ui_display_speed(speed_index*100, frame_rate);

        prev_time = curr_time;
    }
    else prev_time = gettime();
    speed_eval_prev_clk  = clk;
    speed_eval_suspended = 0;
}

void vsync_prevent_clk_overflow(CLOCK sub)
{
    speed_eval_prev_clk -= sub;
}

/* OS/2 functions to handle emulator paused */

void emulator_pause()
{
    suspend_speed_eval();
    emulator_paused = TRUE;
}

void emulator_resume()
{
    emulator_paused=FALSE;
}

int isEmulatorPaused()
{
    return emulator_paused;
}


/* ------------------------------------------------------------------------- */

/* This is called at the end of each screen frame.  It flushes the audio buffer
   and keeps control of the emulation speed.  */
int do_vsync(int been_skipped)
{
    static unsigned short frame_counter = USHRT_MAX;
    static unsigned short num_skipped_frames;
    static int skip_counter;
    int skip_next_frame = 0;

    while (emulator_paused) DosSleep(1);

    vsync_hook();

    if (been_skipped) num_skipped_frames++;

    if (timer_speed != relative_speed) {
	frame_counter = USHRT_MAX;
        set_timer_speed(relative_speed);
    }

    if (warp_mode_enabled) {
        /* "Warp Mode".  Just skip as many frames as possible and do not
         limit the maximum speed at all.  */
        if (skip_counter < MAX_SKIPPED_FRAMES) {
            skip_next_frame = 1;
            skip_counter++;
        }
        else skip_counter = elapsed_frames = 0;
        sound_flush(0);
    }
    else
    {
        if (refresh_rate)
        {   // Fixed refresh rate.
            update_elapsed_frames(0);
            if (timer_speed && skip_counter >= elapsed_frames)
                timer_sleep();
            if (skip_counter < refresh_rate - 1)
            {
                skip_next_frame = 1;
                skip_counter++;
            }
            else
            {
                skip_counter = elapsed_frames = 0;
                // this is for better system response if CPU usage is 100%
                DosSleep(1);
            }
            patch_timer(sound_flush(relative_speed));
        }
        else
        {   // Dynamically adjusted refresh rate.
            update_elapsed_frames(0);
            if (skip_counter >= elapsed_frames)
            {
                elapsed_frames = -1;
                timer_sleep();
                skip_counter = 0;
            }
            else
                if (skip_counter < MAX_SKIPPED_FRAMES)
                {
                    skip_next_frame = 1;
                    skip_counter++;
                }
                else
                {
                    skip_counter = elapsed_frames = 0;
                    // this is for better system response if CPU usage is 100%
                    DosSleep(1);
                }
            patch_timer(sound_flush(relative_speed));
        }
    }
    if (frame_counter >= refresh_frequency * 2)
    {
        display_speed(frame_counter + 1 - num_skipped_frames);
	num_skipped_frames = 0;
	frame_counter = 0;
        // this is for better system response in warp_mode
        // seems that it doesn't really make it slower
        if (warp_mode_enabled) DosSleep(1);
    }
    else frame_counter++;

#ifdef HAS_JOYSTICK
    joystick_update();
#endif

    return skip_next_frame;
}
