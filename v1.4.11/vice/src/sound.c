/*
 * sound.c - General code for the sound interface.
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *
 * Resource and cmdline code by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "clkguard.h"
#include "cmdline.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "resources.h"
#include "sound.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "vsync.h"

/* ------------------------------------------------------------------------- */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* ------------------------------------------------------------------------- */

/* Resource handling -- Added by Ettore 98-04-26.  */

/* FIXME: We need sanity checks!  And do we really need all of these
   `sound_close()' calls?  */

static int playback_enabled;          /* app_resources.sound */
static int sample_rate;               /* app_resources.soundSampleRate */
static char *device_name=NULL;        /* app_resources.soundDeviceName */
static char *device_arg=NULL;         /* app_resources.soundDeviceArg */
static int buffer_size;               /* app_resources.soundBufferSize */
static int suspend_time;              /* app_resources.soundSuspendTime */
static int speed_adjustment_setting;  /* app_resources.soundSpeedAdjustment */
static int oversampling_factor;       /* app_resources.soundOversample */

/* I need this to serialize close_sound and enablesound/initsid in
   the OS/2 Multithreaded environment                              */
static int sdev_open           = FALSE;

/* I need this to serialize close_sound and enablesound/initsid in
   the OS/2 Multithreaded environment                              */
int sound_state_changed;

static int set_playback_enabled(resource_value_t v)
{
    if ((int)v) vsync_disable_timer();
    playback_enabled = (int)v;
    return 0;
}

static int set_sample_rate(resource_value_t v)
{
    sample_rate   = (int) v;
    sound_state_changed = TRUE;
    return 0;
}

static int set_device_name(resource_value_t v)
{
    string_set(&device_name, (char *) v);
    sound_close();
    return 0;
}

static int set_device_arg(resource_value_t v)
{
    string_set(&device_arg, (char *) v);
    sound_close();
    return 0;
}

static int set_buffer_size(resource_value_t v)
{
    buffer_size   = (int)v;
    sound_state_changed = TRUE;
    return 0;
}

static int set_suspend_time(resource_value_t v)
{
    suspend_time = (int)v;
    if (suspend_time < 0)
        suspend_time = 0;
    sound_close();
    return 0;
}

static int set_speed_adjustment_setting(resource_value_t v)
{
    speed_adjustment_setting = (int)v;
    sound_close();
    return 0;
}

static int set_oversampling_factor(resource_value_t v)
{
    oversampling_factor = (int)v;
    if (oversampling_factor < 0 || oversampling_factor > 3) {
        log_warning(LOG_DEFAULT,
                    "Invalid oversampling factor %d.  Forcing 3.",
                    oversampling_factor);
        oversampling_factor = 3;
    }
    sound_state_changed = TRUE;
    return 0;
}

static resource_t resources[] = {
    { "Sound", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &playback_enabled, set_playback_enabled },
    { "SoundSampleRate", RES_INTEGER,
      (resource_value_t) SOUND_SAMPLE_RATE,
      (resource_value_t *) &sample_rate, set_sample_rate },
    { "SoundDeviceName", RES_STRING, (resource_value_t) NULL,
      (resource_value_t *) &device_name, set_device_name },
    { "SoundDeviceArg", RES_STRING, (resource_value_t) NULL,
      (resource_value_t *) &device_arg, set_device_arg },
    { "SoundBufferSize", RES_INTEGER, (resource_value_t) SOUND_SAMPLE_BUFFER_SIZE,
      (resource_value_t *) &buffer_size, set_buffer_size },
    { "SoundSuspendTime", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &suspend_time, set_suspend_time },
    { "SoundSpeedAdjustment", RES_INTEGER, (resource_value_t) SOUND_ADJUST_FLEXIBLE,
      (resource_value_t *) &speed_adjustment_setting,
      set_speed_adjustment_setting },
    { "SoundOversample", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &oversampling_factor, set_oversampling_factor },
    { NULL }
};

int sound_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

/* Command-line options -- Added by Ettore 98-05-09.  */
static cmdline_option_t cmdline_options[] = {
    { "-sound", SET_RESOURCE, 0, NULL, NULL, "Sound", (resource_value_t) 1,
      NULL, "Enable sound playback" },
    { "+sound", SET_RESOURCE, 0, NULL, NULL, "Sound", (resource_value_t) 0,
      NULL, "Disable sound playback" },
    { "-soundrate", SET_RESOURCE, 1, NULL, NULL, "SoundSampleRate", NULL,
      "<value>", "Set sound sample rate to <value> Hz" },
    { "-soundbufsize", SET_RESOURCE, 1, NULL, NULL, "SoundBufferSize", NULL,
      "<value>", "Set sound buffer size to <value> msec" },
    { "-sounddev", SET_RESOURCE, 1, NULL, NULL, "SoundDeviceName", NULL,
      "<name>", "Specify sound driver" },
    { "-soundarg", SET_RESOURCE, 1, NULL, NULL, "SoundDeviceArg", NULL,
      "<args>", "Specify initialization parameters for sound driver" },
    { "-soundsync", SET_RESOURCE, 1, NULL, NULL, "SoundSpeedAdjustment", NULL,
      "<sync>", "Set sound speed adjustment (0: flexible, 1: adjusting, 2: exact)" },
    { NULL }
};

int sound_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* timing constants */
static unsigned int cycles_per_sec;
static unsigned int cycles_per_rfsh;
static double rfsh_per_sec;

/* Flag: Is warp mode enabled?  */
static int warp_mode_enabled;

#define BUFSIZE 32768
typedef struct
{
    /* sid itself */
    sound_t		*psid;
    /* number of clocks between each sample. used value */
    double		 clkstep;
    /* number of clocks between each sample. original value */
    double		 origclkstep;
    /* factor between those two clksteps */
    double		 clkfactor;
    /* time of last sample generated */
    double		 fclk;
    /* time of last write to sid. used for pdev->dump() */
    CLOCK		 wclk;
    /* sample buffer */
    SWORD		 buffer[BUFSIZE];
    /* sample buffer pointer */
    int			 bufptr;
    /* pointer to device structure in use */
    sound_device_t	*pdev;
    /* number of samples in a fragment */
    int			 fragsize;
    /* number of fragments in kernel buffer */
    int			 fragnr;
    /* number of samples in kernel buffer */
    int			 bufsize;
    /* return value of first pdev->bufferstatus() call to device */
    int			 firststatus;
    /* constants related to adjusting sound */
    int			 prevused;
    int			 prevfill;
    /* is the device suspended? */
    int			 issuspended;
    SWORD		 lastsample;
    /* nr of samples to oversame / real sample */
    DWORD		 oversamplenr;
    /* number of shift needed on oversampling */
    DWORD		 oversampleshift;
} snddata_t;

static snddata_t snddata;

/* device registration code */
static sound_device_t *sound_devices[32];

int sound_register_device(sound_device_t *pdevice)
{
    int					i;
    for (i = 0; sound_devices[i]; i++);
    sound_devices[i] = pdevice;
    return 0;
}


/* close sid device and show error dialog if needed */
static int closesound(const char *msg)
{
    if (snddata.pdev)
    {
	log_message(LOG_DEFAULT, "SOUND: Closing device `%s'", snddata.pdev->name);
	if (snddata.pdev->close)
	    snddata.pdev->close();
        snddata.pdev = NULL;
    }
    if (snddata.psid)
    {
	sound_machine_close(snddata.psid);
	snddata.psid = NULL;
    }
    if (msg)
    {
        suspend_speed_eval();
	if (msg[0])
	{
	    if (console_mode || vsid_mode)
	        log_message(LOG_DEFAULT, "SOUND: %s", msg);
	    else
	        ui_error(msg);
            playback_enabled = 0;
	    if (!console_mode && !vsid_mode)
	        ui_update_menus();
	}
    }
    snddata.prevused = snddata.prevfill = 0;
    return 1;
}

/* code to disable sid for a given number of seconds if needed */
static int disabletime;

static void suspendsound(const char *reason)
{
    disabletime = time(0);
    log_warning(LOG_DEFAULT, "SUSPEND: disabling sound for %d secs (%s)",
	 suspend_time, reason);
    closesound("");
}

static void enablesound(void)
{
    int	diff;
    if (!disabletime)
        return;
    diff = time(0) - disabletime;
    if (diff < 0 || diff >= suspend_time)
    {
        disabletime = 0;
    }
}

/* open sound device */
static int initsid(void)
{
    int i;
    sound_device_t *pdev;
    char *name;
    char *param;
    int speed;
    int fragsize;
    int fragnr;
    double bufsize;
    char err[1024];

    if (suspend_time > 0 && disabletime)
        return 1;

    name = device_name;
    if (name && !strcmp(name, ""))
	name = NULL;
    param = device_arg;
    if (param && !strcmp(param, ""))
	param = NULL;

    bufsize = ((buffer_size<100 || buffer_size>1000)?SOUND_SAMPLE_BUFFER_SIZE:buffer_size)/1000.0;
    speed   = (sample_rate<8000 || sample_rate>50000)?SOUND_SAMPLE_RATE:sample_rate;

        /* calculate optimal fragments */
    fragsize = speed / ((int)rfsh_per_sec);
    for (i = 1; 1 << i < fragsize; i++);
    fragsize = 1 << i;
    fragnr = (int)((speed * bufsize + fragsize - 1) / fragsize);
    if (fragnr < 3)
        fragnr = 3;

    for (i = 0; (pdev = sound_devices[i]); i++)
    {
	if ((name && pdev->name && !strcmp(pdev->name, name)) ||
	    (!name && pdev->name))
	{
	    if (pdev->init)
	    {
		sprintf(err, "SOUND(%s)", pdev->name);
		if (pdev->init(param, &speed, &fragsize, &fragnr, bufsize))
		{
		    sprintf(err, "Audio: initialization failed for device `%s'.",
			    pdev->name);
		    return closesound(err);
		}
	    }
	    snddata.issuspended = 0;
	    snddata.lastsample = 0;
	    snddata.pdev = pdev;
	    snddata.fragsize = fragsize;
	    snddata.fragnr = fragnr;
	    snddata.bufsize = fragsize*fragnr;
	    snddata.bufptr = 0;
	    log_message(LOG_DEFAULT,
		 "SOUND: Opened device `%s' speed %dHz fragsize %.3fs bufsize %.3fs",
		 pdev->name, speed, (double)fragsize / speed,
		 (double)snddata.bufsize / speed);
	    sample_rate = speed;
	    snddata.oversampleshift = oversampling_factor;
	    snddata.oversamplenr = 1 << snddata.oversampleshift;
	    snddata.psid = sound_machine_open((int)
                                              (speed * snddata.oversamplenr),
					      cycles_per_sec);
            if (!snddata.psid)
            {
                return closesound("Audio: Cannot initialize sound module");
            }
	    if (pdev->bufferstatus)
		snddata.firststatus = pdev->bufferstatus(1);
	    snddata.clkstep = (double)cycles_per_sec / speed;
	    if (snddata.oversamplenr > 1)
	    {
		snddata.clkstep /= snddata.oversamplenr;
		log_message(LOG_DEFAULT, "SOUND: Using %dx oversampling",
		     snddata.oversamplenr);
	    }
	    snddata.origclkstep = snddata.clkstep;
	    snddata.clkfactor = 1.0;
	    snddata.fclk = clk;
	    snddata.wclk = clk;

	    /* Set warp mode for non-realtime sound devices in psid mode. */
	    if (vsid_mode && !pdev->bufferstatus)
	        resources_set_value("WarpMode", (resource_value_t)1);

	    return 0;
	}
    }
    sprintf(err, "Audio: device '%s' not found or not supported.", name);
    return closesound(err);
}

/* run sid */
static int sound_run_sound(void)
{
    int	nr, i;
    /* XXX: implement the exact ... */
    if (!playback_enabled || (suspend_time > 0 && disabletime))
        return 1;
    if (!snddata.pdev)
    {
        i = initsid();
        if (i)
            return i;
        sdev_open = TRUE;
    }
#ifdef __riscos
    /* RISC OS vidc device uses a different approach... */
    SoundMachineReady = 1;
    if (SoundThreadActive != 0) return 0;
#endif
    nr = (int)((clk - snddata.fclk) / snddata.clkstep);
    if (!nr)
	return 0;
    if (snddata.bufptr + nr > BUFSIZE)
        return closesound("Audio: sound buffer overflow.");

    sound_machine_calculate_samples(snddata.psid,
				    snddata.buffer + snddata.bufptr,
				    nr);
    snddata.bufptr += nr;
    snddata.fclk   += nr*snddata.clkstep;
    return 0;
}

/* reset sid */
void sound_reset(void)
{
    snddata.fclk = clk;
    snddata.wclk = clk;
    snddata.bufptr = 0;		/* ugly hack! */
    if (snddata.psid)
	sound_machine_reset(snddata.psid, clk);
}

static void prevent_clk_overflow_callback(CLOCK sub, void *data)
{
    snddata.fclk -= sub;
    snddata.wclk -= sub;
    if (snddata.psid)
	sound_machine_prevent_clk_overflow(snddata.psid, sub);
}

#ifdef __riscos
void sound_synthesize(SWORD *buffer, int length)
{
    sound_machine_calculate_samples(snddata.psid, buffer, length);
    snddata.fclk += length * snddata.clkstep;
}
#endif

/* flush all generated samples from buffer to sounddevice. adjust sid runspeed
   to match real running speed of program */
int sound_flush(int relative_speed)
{
    int	i, nr, space, used, fill = 0, dir = 0;

    if (!playback_enabled || sound_state_changed)
    {
        if (sdev_open) sound_close();
        sound_state_changed = FALSE;
        return 0;
    }

    if (suspend_time > 0)
        enablesound();
    if (sound_run_sound())
        return 0;
    sound_resume();

    if (warp_mode_enabled && snddata.pdev->bufferstatus != NULL) {
      snddata.bufptr = 0;
      return 0;
    }

    if (snddata.pdev->flush)
    {
	char *state = sound_machine_dump_state(snddata.psid);
	i = snddata.pdev->flush(state);
	free(state);
	if (i)
	{
	    closesound("Audio: cannot flush.");
	    return 0;
	}
    }
    nr = snddata.bufptr -
	snddata.bufptr % (snddata.fragsize*snddata.oversamplenr);
    if (!nr)
	return 0;
    /* handle oversampling */
    if (snddata.oversamplenr > 1)
    {
	int j, newnr;
	newnr = nr >> snddata.oversampleshift;
        {
            SDWORD v;
            SWORD *sndbufj = &snddata.buffer[0];
            SWORD *sndbufi = &snddata.buffer[0];
            for (i = 0; i++ < newnr;)
            {
                for (v = j = 0; j++ < (int)(snddata.oversamplenr);)
                    v += *(sndbufj++);
                *(sndbufi++) = v >> snddata.oversampleshift;
            }
        }
        {
            SWORD *sndbufnewnr = &snddata.buffer[newnr];
            SWORD *sndbufnr    = &snddata.buffer[nr];
            for (i = 0; i++ < snddata.bufptr - nr;)
                *(sndbufnewnr++) = *(sndbufnr++);
        }
	snddata.bufptr -= (nr - newnr);
	nr = newnr;
    }
    /* adjust speed */
    if (snddata.pdev->bufferstatus)
    {
	space = snddata.pdev->bufferstatus(0);
	if (!snddata.firststatus)
	    space = snddata.bufsize - space;
	if (space < 0 || space > snddata.bufsize)
	{
	    log_warning(LOG_DEFAULT, "fragment problems %d %d %d",
		 space, snddata.bufsize - space, snddata.firststatus);

            closesound("Audio: fragment problems.");
	    return 0;
	}
	used = snddata.bufsize - space;
	/* buffer empty */
	if (used <= snddata.fragsize)
	{
	    SWORD		*p, v;
	    int			 j;
	    static int		 prev;
	    int			 now;
	    if (suspend_time > 0)
	    {
	        now = time(0);
		if (now == prev)
		{
		    suspendsound("buffer overruns");
		    return 0;
		}
		prev = now;
	    }
	    j = snddata.fragsize*snddata.fragnr - nr;
	    if (j > snddata.bufsize / 2
                && speed_adjustment_setting != SOUND_ADJUST_ADJUSTING
                && relative_speed)
	    {
		j = snddata.fragsize*(snddata.fragnr/2);
	    }
	    j *= sizeof(*p);
	    if (j > 0)
	    {
	        p = (short*)alloca(j);
		v = snddata.bufptr > 0 ? snddata.buffer[0] : 0;
		for (i = 0; i < j / (int)sizeof(*p); i++)
		    p[i] = (float)v*i/(j / (int)sizeof(*p));
		if (snddata.pdev->write(p, j/sizeof(*p)))
		{
		    closesound("Audio: write to sound device failed.");
		    return 0;
		}
		snddata.lastsample = v;
	    }
	    fill = j;
	}
	if (speed_adjustment_setting != SOUND_ADJUST_ADJUSTING) {
            if (relative_speed > 0)
	        snddata.clkfactor = relative_speed / 100.0;
	}
	else
	{
	    if (snddata.prevfill)
		snddata.prevused = used;
	    snddata.clkfactor *= 1.0 + 0.9*(used - snddata.prevused)/
		snddata.bufsize;
	}
	snddata.prevused = used;
	snddata.prevfill = fill;
	if (speed_adjustment_setting == SOUND_ADJUST_EXACT)
	{
	    /* finetune VICE timer */
	    static int lasttime = 0;
	    static int minspace = 0;
	    int t = time(0);
	    if (minspace > space - nr)
		minspace = space - nr;
	    if (t != lasttime)
	    {
		lasttime = t;
		if (minspace <= 0)
		    dir = -1;
		if (minspace > snddata.fragsize)
		    dir = 1;
		minspace = snddata.bufsize;
	    }
	}
	else
	    snddata.clkfactor *= 0.9 + (used+nr)*0.12/snddata.bufsize;
	snddata.clkstep = snddata.origclkstep * snddata.clkfactor;
	if (cycles_per_rfsh / snddata.clkstep >= snddata.bufsize)
	{
            if (suspend_time > 0)
	        suspendsound("running too slow");
	    else
            {
                closesound("Audio: running too slow.");
            }
	    return 0;
	}
	if (nr > space && nr < used)
	    nr = space;
    }
    if (snddata.pdev->write(snddata.buffer, nr))
    {
	closesound("Audio: write to sounddevice failed.");
	return 0;
    }
    snddata.lastsample = snddata.buffer[nr-1];
    snddata.bufptr -= nr;
    if (snddata.bufptr > 0)
    {
	for (i = 0; i < snddata.bufptr; i++)
	    snddata.buffer[i] = snddata.buffer[i + nr];
    }

    return dir;
}

/* close sid */
void sound_close(void)
{
    closesound(NULL);
    sdev_open     = FALSE;
    sound_state_changed = FALSE;
}

/* suspend sid (eg. before pause) */
void sound_suspend(void)
{
    int	i;
    SWORD *p, v;
    if (snddata.pdev)
    {
	if (snddata.pdev->write && !snddata.issuspended)
	{
	    p = (short*)xmalloc(snddata.fragsize*sizeof(SWORD));
	    if (!p)
		return;
	    v = snddata.lastsample;
	    for (i = 0; i < snddata.fragsize; i++)
		p[i] = v - (float)v * i / (int)snddata.fragsize;
	    i = snddata.pdev->write(p, snddata.fragsize);
	    free(p);
	    if (i)
		return;
	}
	if (snddata.pdev->suspend && !snddata.issuspended)
	{
	    if (snddata.pdev->suspend())
		return;
	}
	snddata.issuspended = 1;
    }
}

/* resume sid */
void sound_resume(void)
{
    if (snddata.pdev)
    {
        snddata.issuspended = (snddata.pdev->resume && snddata.issuspended == 1)
                              ? (snddata.pdev->resume() ? 1 : 0) : 0;
    }
}


/* initialize sid at program start -time */
void sound_init(unsigned int clock_rate, unsigned int ticks_per_frame)
{
    sound_state_changed = FALSE;

    cycles_per_sec  = clock_rate;
    cycles_per_rfsh = ticks_per_frame;
    rfsh_per_sec    = (1.0 / ((double)cycles_per_rfsh / (double)cycles_per_sec));

    sound_machine_init();

    clk_guard_add_callback(&maincpu_clk_guard, prevent_clk_overflow_callback,
                           NULL);

#if defined(HAVE_LINUX_SOUNDCARD_H) || defined(HAVE_MACHINE_SOUNDCARD_H)
    sound_init_uss_device();
#endif
#if defined(HAVE_DMEDIA_AUDIO_H)
    sound_init_sgi_device();
#endif
#if defined(HAVE_SYS_AUDIOIO_H)
    sound_init_sun_device();
#endif
#if defined(HAVE_SYS_AUDIO_H)
    sound_init_hpux_device();
#endif
#if defined(HAVE_LIBUMSOBJ) && defined(HAVE_UMS_UMSAUDIODEVICE_H) && defined(HAVE_UMS_UMSBAUDDEVICE_H)
    sound_init_aix_device();
#endif
#if defined(HAVE_SDL_AUDIO_H) && defined(HAVE_SDL_SLEEP_H)
    sound_init_sdl_device();
#endif

#ifdef __MSDOS__
#ifdef USE_MIDAS_SOUND
    sound_init_midas_device();
#else
    sound_init_sb_device();
#endif
#endif

#ifdef WIN32
    sound_init_dx_device();
#endif

#ifdef WINCE
    sound_init_ce_device();
#endif

#ifdef __riscos
    sound_init_vidc_device();
#endif

#ifdef OS2
    //    sound_init_mmos2_device();
    sound_init_dart_device();
//    sound_init_dart2_device();
#endif

#ifdef __BEOS__
	sound_init_beos_device();
#endif

    sound_init_dummy_device();
    sound_init_fs_device();
    sound_init_speed_device();
    sound_init_dump_device();
    sound_init_wav_device();

#if 0
    sound_init_test_device();	/* XXX: missing */
#endif
}

double sound_sample_position(void)
{
    return (snddata.clkstep==0) ? 0.0 : (clk-snddata.fclk)/snddata.clkstep;
}

int sound_read(ADDRESS addr)
{
    return sound_run_sound() ? -1 : sound_machine_read(snddata.psid, addr);
}

void sound_store(ADDRESS addr, BYTE val)
{
    int	i;
    if (sound_run_sound()) return;
    sound_machine_store(snddata.psid, addr, val);
    if (snddata.pdev->dump)
    {
        i = snddata.pdev->dump(addr, val, clk - snddata.wclk);
        snddata.wclk = clk;
        if (i)
            closesound("Audio: store to sounddevice failed.");
    }
}


void sound_set_warp_mode(int value)
{
    warp_mode_enabled = value;
}
