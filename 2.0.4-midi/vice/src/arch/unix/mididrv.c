/*
 * mididrv.c - MIDI emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  André Fachat <a.fachat@physik.tu-chemnitz.de>
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

#undef        DEBUG

#include "vice.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#if defined(MINIX_SUPPORT) || defined(OPENSERVER6_COMPILE)
#include <sys/select.h>
#endif

#if defined(__QNX__) && !defined(__QNXNTO__)
#include <sys/select.h>
#endif

#include "cmdline.h"
#include "log.h"
#include "mididrv.h"
#include "types.h"

/* ------------------------------------------------------------------------- */

static int fd_in = -1;
static int fd_out = -1;

static log_t mididrv_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

void mididrv_init(void)
{
    if(mididrv_log == LOG_ERR) {
        mididrv_log = log_open("MIDIdrv");
    }
}

/* opens a MIDI-In device, returns handle */
int mididrv_in_open(void)
{
    static const char *infile = "/dev/snd/midiC1D0"; /* FIXME */
#ifdef DEBUG
    log_message(mididrv_log, "in_open");
#endif
    if(fd_in >= 0) {
        mididrv_in_close();
    }

     fd_in = open(infile, O_RDONLY);
     if(fd_in < 0) {
         log_error(mididrv_log, "Cannot open file \"%s\": %s",
                   infile, strerror(errno));
         return -1;
     }

    return fd_in;
}

/* opens a MIDI-Out device, returns handle */
int mididrv_out_open(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "out_open");
#endif
    /* TODO */
    return -1;
}

/* closes the MIDI-In device*/
void mididrv_in_close(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "in_close");
#endif
    if(fd_in < 0) {
        log_error(mididrv_log, "Attempt to close invalid fd %d.", fd_in);
        return;
    }
    close(fd_in);
    fd_in = -1;
}

/* closes the MIDI-Out device*/
void mididrv_out_close(void)
{
#ifdef DEBUG
    log_error(mididrv_log, "out_close");
#endif
    if(fd_out < 0) {
        log_error(mididrv_log, "Attempt to close invalid fd %d.", fd_out);
        return;
    }
    close(fd_out);
    fd_out = -1;
}

/* sends a byte to MIDI-Out */
void mididrv_out(BYTE b)
{
    ssize_t n;
#ifdef DEBUG
    log_message(mididrv_log, "out %02x", b);
#endif
    if(fd_out < 0) {
        log_error(mididrv_log, "Attempt to write to invalid fd %d.", fd_out);
        return;
    }

    do {
        n = write(fd_out, &b, 1);
        if(n < 0)
            log_error(mididrv_log, "Error writing: %s.", strerror(errno));
    } while(n != 1);

    return;
}

/* gets a byte from MIDI-In, returns !=0 if byte received, byte in *b. */
int mididrv_in(BYTE *b)
{
    int ret;
    size_t n;
    fd_set rdset;
    struct timeval ti;

    if(fd_in < 0) {
        log_error(mididrv_log, "Attempt to read from invalid fd %d.", fd_in);
        return -1;
    }

    FD_ZERO(&rdset);
    FD_SET(fd_in, &rdset);
    ti.tv_sec = ti.tv_usec = 0;

#ifndef MINIXVMD
    /* for now this change will break MIDI support on Minix-vmd
       till I can implement the same functionality using the
       poll() function */

    ret = select(fd_in + 1, &rdset, NULL, NULL, &ti);
#endif

    if(ret && (FD_ISSET(fd_in, &rdset))) {
        n = read(fd_in, b, 1);
        if(n) {
#ifdef DEBUG
            log_message(mididrv_log, "in got %02x", *b);
#endif
            return 1;
        }
    }
    return 0;
}

