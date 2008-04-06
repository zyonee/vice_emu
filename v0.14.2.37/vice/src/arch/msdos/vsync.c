/*
 * vsync.c - End-of-frame handling for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#include <stdlib.h>
#include <time.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>

#include <allegro.h>
#undef EOF			/* Workaround for Allegro bug. */

#include "vsync.h"
#include "resources.h"
#include "cmdline.h"
#include "vmachine.h"
#include "interrupt.h"
#include "kbd.h"
#include "ui.h"
#include "true1541.h"
#include "sid.h"
#include "vmidas.h"
#include "joystick.h"
#include "autostart.h"
#include "kbdbuf.h"

#if defined(VIC20)
#include "vic.h"
#include "via.h"
#elif defined(PET)
#include "crtc.h"
#include "via.h"
#elif defined(CBM64)
#include "vicii.h"
#include "cia.h"
#endif

/* If this is #defined, the emulator always runs at full speed. */
#undef NO_SYNC

/* If this is disabled, we reprogram the timer the hard way, thus making
   the internal clock go crazy. (useful under DOSemu, where the MIDAS timer
   might not work) */
#define USE_MIDAS_TIMER

/* Maximum number of frames we can skip consecutively when adjusting the
   refresh rate dynamically.  */
#define MAX_SKIPPED_FRAMES	10

/* ------------------------------------------------------------------------- */

/* Relative speed of the emulation (%).  0 means "don't limit speed".  */
static int relative_speed;

/* Refresh rate.  0 means "auto".  */
static int refresh_rate;

/* "Warp mode".  If nonzero, attempt to run as fast as possible.  */
static int warp_mode_enabled;

/* FIXME: This should call `set_timers'.  */
static int set_relative_speed(resource_value_t v)
{
    relative_speed = (int) v;
    return 0;
}

static int set_refresh_rate(resource_value_t v)
{
    if ((int) v < 0)
        return -1;
    refresh_rate = (int) v;
    return 0;
}

static int set_warp_mode(resource_value_t v)
{
    warp_mode_enabled = (int) v;
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

/* Number of frames per second on the real machine.  */
static double refresh_frequency;

/* Number of clock cycles per seconds on the real machine.  */
static long cycles_per_sec;

/* Function to call at the end of every screen frame.  */
static void (*vsync_hook)(void);

/* ------------------------------------------------------------------------- */

/* Speed evaluation. */

static CLOCK speed_eval_prev_clk;

static double avg_speed_index;
static double avg_frame_rate;

static int speed_eval_suspended = 1;

void suspend_speed_eval(void)
{
    speed_eval_suspended = 1;
}

static void calc_avg_performance(int num_frames)
{
    static double prev_time;
    struct timeval tv;
    double curr_time;

    gettimeofday(&tv, NULL);
    curr_time = (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;
    if (!speed_eval_suspended) {
	CLOCK diff_clk;
	double speed_index;
	double frame_rate;

	diff_clk = clk - speed_eval_prev_clk;
	frame_rate = (double)num_frames / (curr_time - prev_time);
	speed_index = ((((double)diff_clk / (curr_time - prev_time))
			/ (double)cycles_per_sec)) * 100.0;
	avg_speed_index = speed_index;
	avg_frame_rate = frame_rate;
    }
    prev_time = curr_time;
    speed_eval_prev_clk = clk;
    speed_eval_suspended = 0;
}

double vsync_get_avg_frame_rate(void)
{
    if (speed_eval_suspended)
	return -1.0;
    else
	return avg_frame_rate;
}

double vsync_get_avg_speed_index(void)
{
    if (speed_eval_suspended)
	return -1.0;
    else
	return avg_speed_index;
}

/* ------------------------------------------------------------------------- */

/* This prevents the clock counters from overflowing. */
void vsync_prevent_clk_overflow(CLOCK sub)
{
    speed_eval_prev_clk -= sub;
}

/* ------------------------------------------------------------------------- */

#if defined(USE_MIDAS_TIMER) && !defined(NO_SYNC)

static volatile int elapsed_frames = 0;
static int timer_speed = -1;

static void MIDAS_CALL my_timer_callback(void)
{
    elapsed_frames++;
}
END_OF_FUNCTION(my_timer_callback)

inline static void set_timer_speed(void)
{
    int new_timer_speed;

    /* Force 100% speed if using automatic refresh rate and there is no speed
       limit, or sound playback is turned on. */
    if (relative_speed == 0 && refresh_rate == 0)
	new_timer_speed = 100;
    else
	new_timer_speed = relative_speed;

    if (new_timer_speed == timer_speed)
	return;

    timer_speed = new_timer_speed;
    if (timer_speed == 0) {
	if (!vmidas_remove_timer_callbacks())
	    fprintf(stderr, "%s: Warning: Could not remove timer callbacks.\n",
		    __FUNCTION__);
    } else {
	DWORD rate = (refresh_frequency * 1000 * timer_speed) / 100;

	/* printf("%s: setting MIDAS timer at %d\n", __FUNCTION__, rate); */
	if (vmidas_set_timer_callbacks(rate, FALSE,
				       my_timer_callback, NULL, NULL) < 0) {
	    fprintf(stderr, "%s: cannot set timer callback at %.2f Hz\n",
		    __FUNCTION__, (double)rate / 1000.0);
	    /* FIXME: is this necessary? */
	    vmidas_remove_timer_callbacks();
	    relative_speed = timer_speed = 0;
	}
    }
}

int do_vsync(int been_skipped)
{
    extern int _escape_requested;
    static long skip_counter = 0;
    static int num_skipped_frames = 0;
    static int frame_counter = 0;
    int skip_next_frame = 0;

    set_timer_speed();

    if (been_skipped)
	num_skipped_frames++;

    if (refresh_rate != 0) {

	/* Fixed refresh rate. */

	if (timer_speed != 0 && skip_counter >= elapsed_frames)
	    while (skip_counter >= elapsed_frames)
		/* Sleep... */;
	if (skip_counter < refresh_rate - 1) {
	    skip_next_frame = 1;
	    skip_counter++;
	} else {
	    skip_counter = elapsed_frames = 0;
	}
    } else {

	/* Automatic refresh rate adjustment. */

	if (timer_speed && skip_counter >= elapsed_frames) {
	    while (skip_counter >= elapsed_frames)
		/* Sleep... */ ;
	    elapsed_frames = 0;
	    skip_counter = 0;
	} else {
	    if (skip_counter < MAX_SKIPPED_FRAMES) {
		skip_counter++;
		skip_next_frame = 1;
	    } else {
		/* Give up, we are too slow. */
		skip_next_frame = 0;
		skip_counter = 0;
		elapsed_frames = 0;
	    }
	}
    }

    if (frame_counter >= refresh_frequency * 2) {
	calc_avg_performance(frame_counter + 1 - num_skipped_frames);
	num_skipped_frames = 0;
	frame_counter = 0;
    } else
	frame_counter++;

    kbd_flush_commands();

    if (_escape_requested) {
	_escape_requested = 0;
	maincpu_trigger_trap(ui_main);
    }

    kbd_buf_flush();

    return skip_next_frame;
}

#else  /* !USE_MIDAS_TIMER || NO_VSYNC */

int do_vsync(int been_skipped)
{
    extern int _escape_requested;
    static long skip_counter = 0;
    static long old_rawclock = 0;
    int skip_next_frame = 0;
    int curr_rawclock = rawclock();

#ifdef HAVE_TRUE1541
    do_drive();
#endif

#ifndef NO_SYNC
    /* Here we always do automatic refresh rate adjustment. */
    if (skip_counter >= (curr_rawclock - old_rawclock)) {
    	while (skip_counter >= (rawclock() - old_rawclock))
	    ;
	skip_counter = 0;
	old_rawclock = rawclock();
    } else {
	if (skip_counter < 10) {
	    skip_counter++;
	    skip_next_frame = 1;
	} else {
	    /* Give up, we are too slow. */
	    skip_next_frame = 0;
	    skip_counter = 0;
	    old_rawclock = curr_rawclock;
	}
    }
#else  /* NO_SYNC */
    skip_next_frame = 0;
#endif

    if (_escape_requested) {
	_escape_requested = 0;
	maincpu_trigger_trap(UiMain);
    }

    vsync_prevent_clk_overflow();

#if defined(CBM64) || defined(VIC20)
    joystick_update();
#endif

#ifdef AUTOSTART
    autostart_advance();
#endif

    return skip_next_frame;
}

#endif /* USE_MIDAS_TIMER && !NO_VSYNC */

/* -------------------------------------------------------------------------- */

void vsync_init(double hz, long cycles, void (*hook)(void))
{
#ifndef NO_SYNC
#if defined(USE_MIDAS_TIMER)
    LOCK_VARIABLE(elapsed_frames);
    LOCK_FUNCTION(my_timer_callback);
    vmidas_startup();
    vmidas_init();
#else
    outportb(0x40, (BYTE)(0x1234dd / refresh_frequency));
    outportb(0x40, (BYTE)((0x1234dd / refresh_frequency) >> 8));
#endif
#endif
    vsync_hook = hook;
    refresh_frequency = hz;
    cycles_per_sec = cycles;
    suspend_speed_eval();
    vsync_disable_timer();
}

int vsync_disable_timer(void)
{
    return 0;
}
