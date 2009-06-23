/*
 * audioin.c - General code for the sound interface.
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

#ifdef HAVE_AUDIOIN

#include <stdio.h>
#include <portaudio.h>

#include "machine.h"

#define PAUDIO_SAMPLE_RATE 44100

int paudio_open = 0;

static signed short *audio_in_buffer;
static unsigned int max_buffer = 0;
static PaStream *stream = NULL;
static PaError err;
static int numdevs;
static int filling_current_frame = 0;
static PaStreamParameters inputParameters;

static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    int i;
    short *buffer = (short *)inputBuffer;

    filling_current_frame = 1;
    for (i = 0; i < max_buffer; i++) {
        audio_in_buffer[i] = buffer[i];
    }
    filling_current_frame = 0;

    return paContinue;
}

signed char audio_in_get_sample(void)
{
    unsigned int current_sample = (unsigned int)(max_buffer * machine_get_frame_cycle() / machine_get_cycles_per_frame());

    while (filling_current_frame == 1) {}
    return (signed char)(audio_in_buffer[current_sample] >> 8);
}

void audio_in_close_default_stream(void)
{
    if (stream != NULL) {
        if (Pa_IsStreamActive(stream) == 1) {
            Pa_StopStream(stream);
        }
        Pa_CloseStream(stream);
        stream = NULL;
    }
    Pa_Terminate();

    if (audio_in_buffer != NULL) {
        lib_free(audio_in_buffer);
    }
    paudio_open = 0;
}

int audio_in_open_default_stream(void)
{
    unsigned int refresh;

    err = Pa_Initialize();

    if (err != paNoError) {
        return -1;
    }

    numdevs = Pa_GetDeviceCount();
    if (numdevs < 0) {
        audio_in_close_default_stream();
        return -1;
    }

    refresh = (unsigned int)(1.0 / ((double)machine_get_cycles_per_frame() / (double)machine_get_cycles_per_second()));

    if (refresh == 49) {
        refresh = 50;
    }

    if (refresh == 59) {
        refresh = 60;
    }

    max_buffer = PAUDIO_SAMPLE_RATE / refresh;

    audio_in_buffer = (char *)lib_malloc(max_buffer * 2);

    inputParameters.device = Pa_GetDefaultInputDevice();
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream,
                        &inputParameters,
                        NULL,
                        PAUDIO_SAMPLE_RATE,
                        max_buffer,
                        paClipOff,
                        recordCallback,
                        NULL );

    if (err != paNoError) {
         audio_in_close_default_stream();
         return -1;
    }

    err = Pa_StartStream( stream );
    if (err != paNoError) {
        audio_in_close_default_stream();
        return -1;
    }

    paudio_open = 1;

    return 0;
}

void audio_in_refresh(void)
{
    if (paudio_open == 1) {
        audio_in_close_default_stream();
        audio_in_open_default_stream();
    }
}
#endif
