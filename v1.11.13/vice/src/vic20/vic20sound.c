/*
 * vic20sound.c - Implementation of VIC20 sound code.
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include "maincpu.h"
#include "sound.h"
#include "types.h"
#include "utils.h"
#include "vic20sound.h"

/* argh */
static BYTE siddata[16];

/* noise magic */
#define NSHIFT(v, n) \
    (((v) << (n)) | \
    ((((v) >> (23 - (n))) ^ (v >> (18 - (n)))) & (( 1<< (n)) - 1)))
#define NVALUE(v) \
    (noiseLSB[v&0xff] | noiseMID[(v >> 8) & 0xff] | noiseMSB[(v >> 16) & 0xff])
#define NSEED 0x7ffff8

/* Noise tables */
#define NOISETABLESIZE 256
static BYTE noiseMSB[NOISETABLESIZE];
static BYTE noiseMID[NOISETABLESIZE];
static BYTE noiseLSB[NOISETABLESIZE];

/* needed data for one voice */
typedef struct voice_s
{
    /* counter value */
    DWORD f;
    /* counter step / sample */
    DWORD fs;

    /* noise shift register. Note! rv may be 0 to 15 shifts 'behind' the
       real noise shift register value. Remaining shifts are done when
       it is referenced */
    DWORD rv;
} voice_t;

/* needed data for SID */
struct sound_s
{
    /* number of voices */
    voice_t v[4];
    /* 4-bit volume value */
    BYTE vol;

    /* internal constant used for sample rate dependent calculations */
    DWORD speed1;
};

int sound_machine_calculate_samples(sound_t *psid, SWORD *pbuf, int nr,
                                    int interleave, int *delta_t)
{
    int i;
    DWORD o0, o1, o2, o3;

    for (i = 0; i < nr; i++) {
        /* addfptrs */
        psid->v[0].f += psid->v[0].fs;
        psid->v[1].f += psid->v[1].fs;
        psid->v[2].f += psid->v[2].fs;
        psid->v[3].f += psid->v[3].fs;
        /* noise */
        if (psid->v[3].f < psid->v[3].fs)
            psid->v[3].rv = NSHIFT(psid->v[3].rv, 16);
        /* voices */
        o0 = (psid->v[0].f & 0x80000000) >> 2;
        o1 = (psid->v[1].f & 0x80000000) >> 2;
        o2 = (psid->v[2].f & 0x80000000) >> 2;
        o3 = (DWORD)NVALUE(NSHIFT(psid->v[3].rv, psid->v[3].f >> 28)) << 22;
        /* sample */
        pbuf[i * interleave] = ((SDWORD)((o0 + o1 + o2 + o3) >> 20) - 0x800)
                               * psid->vol;
    }
    return 0;
}


/* SID initialization routine */
sound_t *sound_machine_open(int chipno)
{
    return (sound_t*)xcalloc(1, sizeof(sound_t));
}

int sound_machine_init(sound_t *psid, int speed, int cycles_per_sec)
{
    DWORD i;

    psid->speed1 = (cycles_per_sec << 8) / speed;
    psid->v[3].rv = NSEED;
    for (i = 0; i < NOISETABLESIZE; i++) {
        noiseLSB[i] = (BYTE)(((i >> (7 - 2)) & 0x04) | ((i >> (4 - 1)) &0x02)
                      | ((i >> (2 - 0)) & 0x01));
        noiseMID[i] = (BYTE)(((i >> (13 - 8 - 4)) & 0x10)
                      | ((i << (3- (11 - 8))) & 0x08));
        noiseMSB[i] = (BYTE)(((i << (7 - (22 - 16))) & 0x80)
                      | ((i << (6 - (20 - 16))) & 0x40)
                      | ((i << (5 - (16 - 16))) & 0x20));
    }
    for (i = 0; i < 16; i++)
        sound_machine_store(psid, (ADDRESS)i, siddata[i]);

    return 1;
}

void sound_machine_close(sound_t *psid)
{
    free(psid);
}

void vic_sound_reset(void)
{
    ADDRESS i;

    sound_reset();
    for (i = 10; i < 15; i++)
        vic_sound_store(i, 0);
}


/*
 * XXX: This is _really_ experimental
 */
#define VIC20FREQBASE    65535

void vic_sound_store(ADDRESS addr, BYTE value)
{
    addr &= 0x0f;
    siddata[addr] = value;
    sound_store(addr, value, 0);
}

void sound_machine_store(sound_t *psid, ADDRESS addr, BYTE value)
{
    DWORD freq;
    int sbase, shift, divide;

    switch (addr) {
      case 10:
      case 11:
      case 12:
      case 13:
        sbase = (addr - 10) * 4;
        shift = addr - 10;
        if (addr == 13)
            shift = 0;
        divide = 255 - value;
        /* XXX: ? */
        if (!divide)
            divide = 127;
        if (!(value & 0x80))
            freq = 0;
        else
            freq = VIC20FREQBASE * (1 << shift) / divide;
        psid->v[addr - 10].fs = psid->speed1 * freq;
        break;
      case 14:
        /* volume */
        psid->vol = value & 0x0f;
        break;
    }
}

void sound_machine_prevent_clk_overflow(sound_t *psid, CLOCK sub)
{
}

void sound_machine_reset(sound_t *psid, CLOCK cpu_clk)
{
}

BYTE sound_machine_read(sound_t *psid, ADDRESS addr)
{
    abort();
    return 0;
}

char *sound_machine_dump_state(sound_t *psid)
{
    return xmsprintf("#SID: clk=%d v=%d\n", maincpu_clk, psid->vol);
}

int sound_machine_cycle_based(void)
{
    return 0;
}

int sound_machine_channels(void)
{
    return 1;
}

void sound_machine_enable(int enable)
{

}

