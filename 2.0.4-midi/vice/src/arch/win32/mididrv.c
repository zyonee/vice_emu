/*
 * mididrv.c - MIDI emulation for win32.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
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

#undef DEBUG

#include "vice.h"

#include <stdio.h>
#include <string.h>

#if __GNUC__>2 || (__GNUC__==2 && __GNUC_MINOR__>=91)
#include <windows.h>
#include <mmsystem.h>
#endif

#include "log.h"
#include "mididrv.h"
#include "types.h"

/* ------------------------------------------------------------------------- */

static HMIDIIN handle_in = 0;
static HMIDIOUT handle_out = 0;

static log_t mididrv_log = LOG_ERR;

static void CALLBACK midi_callback(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

static DWORD uglydata = 0;

#define OUT_BUF_LEN 3
static int out_index=0;
static BYTE out_buf[OUT_BUF_LEN];

#define IN_BUF_LEN 1024
static volatile int in_wi=0;
static volatile int in_ri=0;
static BYTE in_buf[IN_BUF_LEN];

/* ------------------------------------------------------------------------- */

void mididrv_init(void)
{
    if(mididrv_log == LOG_ERR) {
        mididrv_log = log_open("MIDIdrv");
    }
}

/* opens a MIDI-In device, returns handle */
int mididrv_in_open(const char *dev)
{
    int n=9;
    MMRESULT ret;
#ifdef DEBUG
    log_message(mididrv_log, "in_open");
#endif
    if(handle_in) {
        mididrv_in_close();
    }

    if(dev == NULL) {
        return -1;
    }

    ret = midiInOpen(&handle_in, n, (DWORD)midi_callback, 0, CALLBACK_FUNCTION);
    if(ret != MMSYSERR_NOERROR) {
        log_error(mididrv_log, "Cannot open MIDI-In device #%d!",
                  n);
        handle_in = 0;
        return -1;
    }

    /* can theoretically return MMSYSERR_INVALHANDLE */
    ret = midiInStart(handle_in);

    return (DWORD)handle_in;
}

/* opens a MIDI-Out device, returns handle */
int mididrv_out_open(const char *dev)
{
    int n=0;
    MMRESULT ret;
#ifdef DEBUG
    log_message(mididrv_log, "out_open");
#endif
    if(handle_out) {
        mididrv_out_close();
    }

    if(dev == NULL) {
        return -1;
    }

    ret = midiOutOpen(&handle_out, n, 0, 0, CALLBACK_NULL);
    if(ret != MMSYSERR_NOERROR) {
        log_error(mididrv_log, "Cannot open MIDI-Out device #%d!",
                  n);
        handle_out = 0;
        return -1;
    }

    /* reset buffer */
    out_index=0;

    return (DWORD)handle_out;
}

/* closes the MIDI-In device*/
void mididrv_in_close(void)
{
    MMRESULT ret;
#ifdef DEBUG
    log_message(mididrv_log, "in_close");
#endif
    if(!handle_in) {
        log_error(mididrv_log, "Attempt to close MIDI-In device that wasn't open!");
        return;
    }
    /* can theoretically return MMSYSERR_INVALHANDLE */
    ret = midiInReset(handle_in);

    ret = midiInClose(handle_in);
    if (ret != MMSYSERR_NOERROR) {
        log_error(mididrv_log, "Couldn't close MIDI-In device.");
    }
    handle_in = 0;
}

/* closes the MIDI-Out device*/
void mididrv_out_close(void)
{
    MMRESULT ret;
#ifdef DEBUG
    log_message(mididrv_log, "out_close");
#endif
    if(!handle_out) {
        log_error(mididrv_log, "Attempt to close MIDI-Out device that wasn't open!");
        return;
    }
    /* can theoretically return MMSYSERR_INVALHANDLE */
    ret = midiOutReset(handle_out);

    ret = midiOutClose(handle_out);
    if (ret != MMSYSERR_NOERROR) {
        log_error(mididrv_log, "Couldn't close MIDI-Out device.");
    }
    handle_out = 0;
}

/* sends a byte to MIDI-Out */
void mididrv_out(BYTE b)
{
    MMRESULT ret;
    int thres;

#ifdef DEBUG
    log_message(mididrv_log, "out %02x", b);
#endif

    out_buf[out_index]=b;
    out_index++;
    if (out_index >= OUT_BUF_LEN) {
        out_index=0;
        log_error(mididrv_log, "MIDI-Out overrun.");
    }

    switch (out_buf[0] & 0xf0) {
    case 0x80: /* Note Off */
    case 0x90: /* Note On */
    case 0xa0: /* Polyphonic Aftertouch */
    case 0xb0: /* Control Change */
    case 0xe0: /* Pitch Wheel */
        thres=3;
        break;
    case 0xc0: /* Program Change */
    case 0xd0: /* Channel Aftertouch */
        thres=2;
        break;
    default: /* running status */
        thres=2;
        break;
    }

    /* flush when enough bytes have been queued */
    if (out_index >=thres) {
        DWORD data;
        out_index=0;
        data = out_buf[0] | (out_buf[1] << 8) | (out_buf[2] << 16);
#ifdef DEBUG
        log_message(mididrv_log, "flushing out %06x", data);
#endif
        ret = midiOutShortMsg(handle_out, data);
        if (ret != MMSYSERR_NOERROR) {
            log_error(mididrv_log, "Failed to output data on MIDI-Out device.");
        }
    }

    return;
}


static void CALLBACK midi_callback(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    int len;
    int i;
    switch (uMsg) {
    case MIM_DATA:
#ifdef DEBUG
        log_message(mididrv_log, "MIDI callback got %08x", dwParam1);
#endif
        switch (dwParam1 & 0xf0) {
        case 0x80: /* Note Off */
        case 0x90: /* Note On */
        case 0xa0: /* Polyphonic Aftertouch */
        case 0xb0: /* Control Change */
        case 0xe0: /* Pitch Wheel */
            len=3;
            break;
        case 0xc0: /* Program Change */
        case 0xd0: /* Channel Aftertouch */
            len=2;
            break;
        default: /* running status */
            len=2;
            break;
        }
        for (i=0; i<len; i++) {
            in_buf[in_wi+i] = dwParam1 & 0xff;
            dwParam1 >>= 8;
        }
        in_wi += len;
        break;

        /* Received all or part of some System Exclusive message */
    case MIM_LONGDATA:
        break;

        /* Process these messages if you desire */
    case MIM_OPEN:
    case MIM_CLOSE:
    case MIM_ERROR:
    case MIM_LONGERROR:
    case MIM_MOREDATA:
        break;
    }
}


/* gets a byte from MIDI-In, returns !=0 if byte received, byte in *b. */
int mididrv_in(BYTE *b)
{
    if(!handle_in) {
        log_error(mididrv_log, "Attempt to read from closed MIDI-In port!");
        return -1;
    }

    if (((in_wi-in_ri) % IN_BUF_LEN) != 0) {
        *b=in_buf[in_ri];
        in_ri++;
#ifdef DEBUG
        log_message(mididrv_log, "in got %02x", *b);
#endif
        return 1;
    }
    return 0;
}
/* eof */
