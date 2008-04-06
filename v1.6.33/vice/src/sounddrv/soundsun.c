/*
 * soundsun.c - Implementation of the Sun/Solaris/NetBSD sound device
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *
 * NetBSD patch by
 *  Krister Walfridsson <cato@df.lth.se>
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
#include <sys/uio.h>
#include <unistd.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <sys/audioio.h>

#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/ioctl.h>       /* For ioctl and _IOWR */
#include <string.h>          /* For memset */
#endif

#include "log.h"
#include "sound.h"

static int sun_bufferspace(void);

static int sun_fd = -1;
static int sun_8bit = 0;
static int sun_bufsize = 0;
static int sun_written = 0;

static int toulaw8(SWORD data)
{
    int			v, s, a;

    a = data / 8;

    v = (a < 0 ? -a : a);
    s = (a < 0 ? 0 : 0x80);

    if (v >= 4080)
        a = 0;
    else if (v >= 2032)
        a = 0x0f - (v - 2032) / 128;
    else if (v >= 1008)
        a = 0x1f - (v - 1008) / 64;
    else if (v >= 496)
        a = 0x2f - (v - 496) / 32;
    else if (v >= 240)
        a = 0x3f - (v - 240) / 16;
    else if (v >= 112)
        a = 0x4f - (v - 112) / 8;
    else if (v >= 48)
        a = 0x5f - (v - 48) / 4;
    else if (v >= 16)
        a = 0x6f - (v - 16) / 2;
    else
        a = 0x7f - v;

    a |= s;

    return a;
}


static int sun_init(const char *param, int *speed,
		    int *fragsize, int *fragnr, double bufsize)
{
    int			st;
    struct audio_info	info;

    if (!param)
	param = "/dev/audio";
    sun_fd = open(param, O_WRONLY, 0777);
    if (sun_fd < 0)
	return 1;
    AUDIO_INITINFO(&info);
    info.play.sample_rate = *speed;
    info.play.channels = 1;
    info.play.precision = 16;
    info.play.encoding = AUDIO_ENCODING_LINEAR;
    st = ioctl(sun_fd, AUDIO_SETINFO, &info);
    if (st < 0)
    {
	AUDIO_INITINFO(&info);
	info.play.sample_rate = 8000;
	info.play.channels = 1;
	info.play.precision = 8;
	info.play.encoding = AUDIO_ENCODING_ULAW;
	st = ioctl(sun_fd, AUDIO_SETINFO, &info);
	if (st < 0)
	    goto fail;
	sun_8bit = 1;
	*speed = 8000;
	log_message(LOG_DEFAULT, "Playing 8 bit ulaw at 8000Hz");
    }
    sun_bufsize = (*fragsize)*(*fragnr);
    sun_written = 0;
    return 0;
fail:
    close(sun_fd);
    sun_fd = -1;
    return 1;
}

static int sun_write(SWORD *pbuf, size_t nr)
{
    int			total, i, now;
    if (sun_8bit)
    {
	/* XXX: ugly to change contents of the buffer */
	for (i = 0; i < nr; i++)
	    ((char *)pbuf)[i] = toulaw8(pbuf[i]);
	total = nr;
    }
    else
	total = nr*sizeof(SWORD);
    for (i = 0; i < total; i += now)
    {
	now = write(sun_fd, (char *)pbuf + i, total - i);
	if (now <= 0)
	    return 1;
    }
    sun_written += nr;

    return 0;
}

static int sun_bufferspace(void)
{
    int			st;
    struct audio_info	info;
    /* ioctl(fd, AUDIO_GET_STATUS, &info) yields number of played samples
       in info.play.samples. */
    st = ioctl(sun_fd, AUDIO_GETINFO, &info);
    if (st < 0)
	return -1;
#if defined(__NetBSD__) || defined(__OpenBSD__)
    if (!sun_8bit)
	return sun_bufsize - (sun_written - info.play.samples / sizeof(SWORD));
#endif
    return sun_bufsize - (sun_written - info.play.samples);
}

static void sun_close(void)
{
    close(sun_fd);
    sun_fd = -1;
    sun_8bit = 0;
    sun_bufsize = 0;
    sun_written = 0;
}


static sound_device_t sun_device =
{
#if defined(__NetBSD__) || defined(__OpenBSD__)
    "netbsd",
#else
    "sun",
#endif
    sun_init,
    sun_write,
    NULL,
    NULL,
    sun_bufferspace,
    sun_close,
    NULL,
    NULL
};

int sound_init_sun_device(void)
{
    return sound_register_device(&sun_device);
}
