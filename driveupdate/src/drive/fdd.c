/*
 * fdd.c - Floppy disk drive emulation
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
 *
 * Also used work from:
 *  Andreas Boose <viceteam@t-online.de>
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
#include <string.h>

#include "diskimage.h"
#include "lib.h"
#include "types.h"
#include "fdd.h"
#include "diskimage.h"
#include "drive.h"

static void fdd_image_writeback(fd_drive_t *drv, int free);
static void fdd_image_read(fd_drive_t *drv);
typedef void (*rotate_function_t)(fd_drive_t *drv);
static void fdd_rotate_null(fd_drive_t *drv);
static void fdd_rotate_gcr(fd_drive_t *drv);
static void fdd_rotate_mfm(fd_drive_t *drv);

struct fd_drive_s {
    int number;      /* number of fdd */
    int disk_change; /* disk change signal */
    int track;       /* track of head */
    int tracks;      /* total tracks which the head can move */
    int side;        /* side selected */
    int sides;       /* number of physical heads */
    int motor;       /* motor running */
    int write_gate;  /* read / write */
    int rate;        /* bit rate */
    int index_count; /* number of index pulses counted */
    int hrstep;      /* high resolution stepping */
    drive_t *drive;
    struct disk_image_s *image;
    rotate_function_t rotate;

    int headb; /* bit position of head */
    BYTE *headp; /* pointer to head position */
    CLOCK last_clk, clk;
    unsigned int read_shift_reg;
    BYTE write_shift_reg;
    BYTE read_latch, write_out;
    WORD read_latch_mfm, write_out_mfm;
    int bit_counter;
    int zero_count;
    DWORD seed, accum;
    int frequency; /* 1, 2, 4 MHz */
    int divider; /* 50000 for 2 MHz, 25000 for 1 MHz */
    int native; /* native byte ready mode */
    int soe, byte_ready, byte_ready_edge;
    struct disk_track_s *raw, *raw_cache[2][84];

    CLOCK attach_clk; /* Tick when the disk image was attached.  */
    CLOCK detach_clk; /* Tick when the disk image was detached.  */
    CLOCK attach_detach_clk; /* Tick when the disk image was attached, but an old image was just detached.  */
};

/* Overflow handling */
inline void fdd_overflow_callback(CLOCK sub, fd_drive_t *drv)
{
    drv->rotate(drv);
    if (drv->last_clk > (CLOCK)0)
        drv->last_clk -= sub;
    if (drv->attach_clk > (CLOCK)0)
        drv->attach_clk -= sub;
    if (drv->detach_clk > (CLOCK)0)
        drv->detach_clk -= sub;
    if (drv->attach_detach_clk > (CLOCK)0)
        drv->attach_detach_clk -= sub;
}

/* Create new instance */
fd_drive_t *fdd_init(int num, drive_t *drive)
{
    fd_drive_t *drv = lib_calloc(1, sizeof(fd_drive_t));
    drv->number = num;
    drv->disk_change = 1;
    drv->tracks = 80;
    drv->sides = 1;
    drv->drive = drive;
    drv->frequency = 1;
    drv->divider = 25000;
    drv->rotate = fdd_rotate_null;
    drv->write_gate = 1;

    switch (drive->type) {
    case DRIVE_TYPE_1551:
        drv->frequency = 2;
        drv->divider = 50000; /* 2 MHz */
        drv->soe = 1;
        drv->native = 3;
        /* fall through */
    case DRIVE_TYPE_1541:
    case DRIVE_TYPE_1541II:
    case DRIVE_TYPE_1570:
    case DRIVE_TYPE_2031:
        drv->tracks = 84;
        drv->track = 17 * 2;
        drv->rotate = fdd_rotate_gcr;
        break;
    case DRIVE_TYPE_1571:
    case DRIVE_TYPE_1571CR:
        drv->tracks = 70;
        drv->track = 17 * 2;
        drv->sides = 2;
        drv->rotate = fdd_rotate_gcr;
        break;
    case DRIVE_TYPE_1581:
        drv->frequency = 2;
        drv->divider = 50000; /* 2 MHz */
        drv->tracks = 82;
        drv->hrstep = 1;
        drv->sides = 2;
        drv->native = 3;
        drv->rotate = fdd_rotate_mfm;
        break;
    case DRIVE_TYPE_2000:
    case DRIVE_TYPE_4000:
        drv->frequency = 2;
        drv->divider = 50000; /* 2 MHz */
        drv->tracks = 82;
        drv->hrstep = 1;
        drv->sides = 2;
        drv->native = 3;
        drv->rotate = fdd_rotate_mfm;
        break;
    }
    drv->drive->current_half_track = (drv->track << drv->hrstep) + 2;
    return drv;
}

/* Shutdown, free */
void fdd_shutdown(fd_drive_t *drv)
{
    if (!drv) {
        return;
    }
    if (drv->image) {
        fdd_image_detach(drv);
    }
    lib_free(drv);
}

/* CPU reset */
void fdd_reset(fd_drive_t *drv, CLOCK clk)
{
    drv->clk = clk;
    drv->last_clk = clk;
    drv->byte_ready = 0;
    drv->byte_ready_edge = 0;
}

/* Adjust timing when maincpu changes speed */
void fdd_set_frequency(fd_drive_t *drv, int mhz)
{
    if (!drv) {
        return;
    }
    drv->rotate(drv);
    drv->frequency = mhz;
    drv->divider = 25000 * mhz;
}

/* Switch native mode for 1571 */
void fdd_set_native(fd_drive_t *drv, int mode)
{
    if (!drv || drv->native == !!mode) {
        return;
    }
    drv->rotate(drv);
    drv->frequency = mode ? 2 : 1;
    drv->divider = mode ? 50000 : 25000;
    drv->native ^= 1;
}

/* Attach disk image */
void fdd_image_attach(fd_drive_t *drv, struct disk_image_s *image)
{
    if (!drv) {
        return;
    }
    if (drv->image) {
        fdd_image_detach(drv);
    }
    drv->image = image;
    drv->disk_change = 1;
    fdd_image_read(drv);
    drv->headp = drv->raw->data;
    drv->headb = 0;

    drv->attach_clk = drv->clk;
    if (drv->detach_clk > (CLOCK)0)
        drv->attach_detach_clk = drv->clk;
}

/* Detach disk image */
void fdd_image_detach(fd_drive_t *drv)
{
    int i;
    disk_track_t *track;

    if (!drv) {
        return;
    }
    fdd_image_writeback(drv, 1);
    drv->image = NULL;

    for (i = 0; i < 84 * 2; i++) {
        track = drv->raw_cache[i % 2][i / 2];
        if (track) {
            lib_free(track->data);
            lib_free(track);
            drv->raw_cache[i % 2][i / 2] = NULL;
        }
    }
    drv->disk_change = 1;
    drv->headp = NULL;
    drv->headb = 0;

    drv->detach_clk = drv->clk;
}

/* Index sensor, active low */
inline int fdd_index(fd_drive_t *drv)
{
    if (!drv || !drv->raw) {
        return 1;
    }
    drv->rotate(drv);
    return drv->headp > drv->raw->data + (drv->raw->size / 32); /* ~6 ms */
}

/* Reset number of revolutions */
inline void fdd_index_count_reset(fd_drive_t *drv)
{
    if (!drv) {
        return;
    }
    drv->rotate(drv);
    drv->index_count = 0;
}

/* Number of revolutions */
inline int fdd_index_count(fd_drive_t *drv)
{
    if (!drv) {
        return 0;
    }
    drv->rotate(drv);
    return drv->index_count;
}

/* Track0 signal, active low */
inline int fdd_track0(fd_drive_t *drv)
{
    if (!drv) {
        return 1;
    }
    return !!drv->track;
}

/* Byte ready signal, active high */
inline int fdd_byte_ready(fd_drive_t *drv)
{
    if (!drv || !drv->soe) {
        return 0;
    }
    drv->rotate(drv);
    return drv->native ? drv->byte_ready : (!(~drv->bit_counter & 7));
}

/* Byte ready signal for 1551, active high */
inline int fdd_byte_ready_1551(fd_drive_t *drv)
{
    if (!drv) {
        return 1;
    }
    drv->rotate(drv);
    return drv->byte_ready;
}

/* Byte ready signal clear */
inline void fdd_byte_ready_clear(fd_drive_t *drv)
{
    if (!drv) {
        return;
    }
    drv->rotate(drv);
    drv->byte_ready = 0;
}

/* Byte ready edge for SO, active high impulse */
inline int fdd_byte_ready_edge(fd_drive_t *drv)
{
    if (!drv || !(drv->soe | drv->byte_ready_edge) || drv->native == 3) {
        return 0;
    }
    drv->clk = *drv->drive->clk;
    drv->rotate(drv);
    if (drv->byte_ready_edge) {
        drv->byte_ready_edge ^= 1;
        return 1;
    }
    return 0;
}

/* GCR sync detector, active low */
inline int fdd_sync(fd_drive_t *drv)
{
    if (!drv || !drv->write_gate) {
        return 1;
    }
    drv->rotate(drv);
    return !!(~drv->read_shift_reg & 0x3ff);
}

/* Output for writing */
inline void fdd_byte_write(fd_drive_t *drv, BYTE data)
{
    drv->rotate(drv);
    drv->write_out = data;
}

/* Read latch */
inline BYTE fdd_byte_read(fd_drive_t *drv)
{
    drv->rotate(drv);
    return drv->read_latch;
}

inline void fdd_mfm_write(fd_drive_t *drv, WORD data)
{
    drv->rotate(drv);
    drv->write_out_mfm = data;
}

inline WORD fdd_mfm_read(fd_drive_t *drv)
{
    drv->rotate(drv);
    return drv->read_latch_mfm;
}

inline int fdd_mfm_ready(fd_drive_t *drv)
{
    drv->rotate(drv);
    if (drv->byte_ready_edge) {
        drv->byte_ready_edge ^= 1;
        return 0;
    }
    return 1;
}

/* write protection, active low */
inline int fdd_write_protect(fd_drive_t *drv)
{
    if (!drv) {
        return 1;
    }

    /* Clear the write protection bit for the time the disk is pulled out on
       detach.  */
    if (drv->detach_clk != (CLOCK)0) {
        if (drv->clk - drv->detach_clk < DRIVE_DETACH_DELAY)
            return 0;
        drv->detach_clk = (CLOCK)0;
    }
    /* Set the write protection bit for the minimum time until a new disk
       can be inserted.  */
    if (drv->attach_detach_clk != (CLOCK)0) {
        if (drv->clk - drv->attach_detach_clk
            < DRIVE_ATTACH_DETACH_DELAY)
            return 1;
        drv->attach_detach_clk = (CLOCK)0;
    }
    /* Clear the write protection bit for the time the disk is put in on
       attach.  */
    if (drv->attach_clk != (CLOCK)0) {
        if (drv->clk - drv->attach_clk < DRIVE_ATTACH_DELAY)
            return 0;
        drv->attach_clk = (CLOCK)0;
    }

    if (!drv->image) {
        return 1;  /* No disk in drive, write protection is off. */
    } 

    return !drv->image->read_only;
}

inline int fdd_disk_change(fd_drive_t *drv)
{
    if (!drv) {
        return 0;
    }
    drv->rotate(drv);
    return drv->disk_change;
}

void fdd_flush(fd_drive_t *drv)
{
    if (!drv) {
        return;
    }
    fdd_image_writeback(drv, 0);
}

void fdd_step_pulse(fd_drive_t *drv, int dir)
{
    int size, pos;

    if (!drv) {
        return;
    }
    drv->rotate(drv);
    if (drv->image) {
        drv->disk_change = 0;
    }
    if (!drv->motor || (drv->track == 0 && dir)
        || (drv->track == drv->tracks - 1 && !dir)) {
        return;
    }
    if (drv->raw) {
        size = drv->raw->size;
        pos = (drv->headp - drv->raw->data) * 8 + drv->headb;
    } else {
        size = 1;
        pos = 0;
    }
    fdd_image_writeback(drv, 1);
    drv->track += dir ? -1 : 1;
    drv->drive->current_half_track = (drv->track << drv->hrstep) + 2;
    fdd_image_read(drv);
    if (drv->raw) {
        pos = pos * drv->raw->size / size;
        drv->headb = pos & 7;
        drv->headp = drv->raw->data + (pos >> 3);
    }
}

inline void fdd_set_side(fd_drive_t *drv, int side)
{
    int size, pos;

    if (!drv || drv->side == !!side || drv->sides <= !!side) {
        return;
    }
    drv->rotate(drv);
    if (drv->raw) {
        size = drv->raw->size;
        pos = (drv->headp - drv->raw->data) * 8 + drv->headb;
    } else {
        size = 1;
        pos = 0;
    }
    fdd_image_writeback(drv, 1);
    drv->side ^= 1;
    fdd_image_read(drv);
    if (drv->raw) {
        pos = pos * drv->raw->size / size;
        drv->headb = pos & 7;
        drv->headp = drv->raw->data + (pos >> 3);
    }
}

inline void fdd_set_motor(fd_drive_t *drv, int motor)
{
    if (!drv || drv->motor == !!motor) {
        return;
    }
    drv->rotate(drv);
    drv->motor ^= 1;
}

inline void fdd_set_rate(fd_drive_t *drv, int rate)
{
    if (!drv || drv->rate == (rate & 3)) {
        return;
    }
    drv->rotate(drv);
    drv->rate = rate & 3;
}

inline void fdd_set_write_gate(fd_drive_t *drv, int wg)
{
    if (!drv || drv->write_gate == !!wg) {
        return;
    }
    drv->rotate(drv);
    drv->write_gate ^= 1;   /* 0 writing, other reading */
}

inline void fdd_set_soe(fd_drive_t *drv, int soe)
{
    if (!drv || drv->soe == !!soe) {
        return;
    }
    drv->rotate(drv);
    drv->soe ^= 1;
    drv->byte_ready = drv->soe & !(~drv->bit_counter & 7);
    drv->byte_ready_edge |= drv->byte_ready;
}

void fdd_set_clk(fd_drive_t *drv, CLOCK clk)
{
    if (drv) {
        drv->clk = clk;
    }
}

inline static void write_next_bit(fd_drive_t *drv, int value)
{
    if (!drv->headp) {
        return;
    }

    drv->headb++;
    if (drv->headb >= 8) {
        drv->headb = 0;
        drv->headp++;
        if (drv->headp >= drv->raw->data + drv->raw->size) {
            drv->headp = drv->raw->data;
            drv->index_count++;
        }
    }

    if (value) {
        *drv->headp |= 0x80 >> drv->headb;
    } else {
        *drv->headp &= 0x7f7f >> drv->headb;
    }
    drv->raw->dirty = 1;
}

inline static int read_next_bit(fd_drive_t *drv)
{
    if (!drv->headp) {
        return 0;
    }

    drv->headb++;
    if (drv->headb >= 8) {
        drv->headb = 0;
        drv->headp++;
        if (drv->headp >= drv->raw->data + drv->raw->size) {
            drv->headp = drv->raw->data;
            drv->index_count++;
        }
    }

    return *drv->headp & (0x80 >> drv->headb);
}

inline static SDWORD RANDOM_nextInt(fd_drive_t *drv) {
    DWORD bits = drv->seed >> 15;
    drv->seed ^= drv->accum;
    drv->seed = drv->seed << 17 | bits;
    return (SDWORD) drv->seed;
}

static void fdd_rotate_null(fd_drive_t *drv) {
    drv->last_clk = drv->clk;
}

static void fdd_rotate_mfm(fd_drive_t *drv)
{
    CLOCK clk;

    if (!drv->motor || !drv->headp) {
        drv->last_clk = drv->clk;
        return;
    }

    clk = drv->clk - drv->last_clk;
    drv->last_clk = drv->clk;

    for (;;) {
        if (drv->accum < drv->divider * 8) {
            if (clk & ~0xffff) {
                drv->accum += 65536 * (drv->raw ? drv->raw->size : 6250);
                clk -= 65536;
            } else {
                drv->accum += clk * (drv->raw ? drv->raw->size : 6250);
                clk = 0;
                if (drv->accum < drv->divider * 8) {
                    break;
                }
            }
        }
        drv->accum -= drv->divider * 8;

        if (drv->write_gate) {
            drv->read_latch_mfm = drv->headp[0];
            if (drv->rate == drv->image->mfm.rate) {
                drv->read_latch_mfm |= drv->headp[drv->raw->size] << 8;
            }
        } else {
            drv->headp[0] = drv->write_out_mfm;
            drv->headp[drv->raw->size] = drv->write_out_mfm >> 8;
            drv->write_out_mfm = 0;
            drv->raw->dirty = 1;
        }
        drv->headp++;
        if (drv->headp >= drv->raw->data + drv->raw->size) {
            drv->headp = drv->raw->data;
            drv->index_count++;
        }
        drv->byte_ready_edge = 1;
    }
}

static void fdd_rotate_gcr(fd_drive_t *drv)
{
    CLOCK clk;

    if (!drv->motor) {
        drv->last_clk = drv->clk;
        return;
    }

    clk = drv->clk - drv->last_clk;
    drv->last_clk = drv->clk;

    for (;;) {
        if (drv->accum < drv->divider) {
            if (clk & ~0xffff) {
                drv->accum += 65536 * (drv->raw ? drv->raw->size : 6250);
                clk -= 65536;
            } else {
                drv->accum += clk * (drv->raw ? drv->raw->size : 6250);
                clk = 0;
                if (drv->accum < drv->divider) {
                    break;
                }
            }
        }
        drv->accum -= drv->divider;
        if (drv->write_gate) {
            drv->read_shift_reg <<= 1;
            drv->write_shift_reg <<= 1;
            if (~drv->read_shift_reg & 0x7fe) {
                drv->bit_counter++;
            }
            /* GCR=0 support.
             * 
             * In the absence of 1-bits (magnetic flux changes), the drive
             * will use a timer counter to count how many 0s it has read. Every
             * 4 read bits, it will detect a 1-bit, because it doesn't
             * distinguish between reset occuring from magnetic flux or regular
             * wraparound.
             * 
             * Random magnetic flux events can also occur after GCR data has been
             * quiet for a long time, for at least 4 bits. So the first value
             * read will always be 1. Afterwards, the 0-bit sequence lengths
             * vary randomly, but can never exceed 3.
             * 
             * Each time a random event happens, it tends to advance the bit counter
             * by half a clock, because the random event can occur at any time
             * and thus the expectation value is that it occurs at 50 % point
             * within the bitcells.
             * 
             * Additionally, the underlying disk rotation has no way to keep in sync
             * with the electronics, so the bitstream after a GCR=0 may or may not
             * be shifted with respect to the bit counter by the time drive
             * encounters it. This situation will persist until the next sync
             * sequence. There is no specific emulation for variable disk rotation,
             * this case is thought to be covered by the random event handling.
             * 
             * Here's some genuine 1541 patterns for reference:
             * 
             * 53 12 46 22 24 AA AA AA AA AA AA AA A8 AA AA AA
             * 53 11 11 11 14 AA AA AA AA AA AA AA A8 AA AA AA
             * 53 12 46 22 24 AA AA AA AA AA AA AA A8 AA AA AA
             * 53 12 22 24 45 2A AA AA AA AA AA AA AA 2A AA AA
             * 53 11 52 22 24 AA AA AA AA AA AA AA A8 AA AA AA
             */

            if (read_next_bit(drv)) {
                drv->zero_count = 0;
                drv->read_shift_reg++;
            }

            /* Simulate random magnetic flux events in our lame-ass emulation. */
            if (++ drv->zero_count > 8 && (drv->read_shift_reg & 0x3f) == 0x8 && RANDOM_nextInt(drv) > (1 << 30)) {
                drv->read_shift_reg |= 1;
                /*
                 * Simulate loss of sync against the underlying platter.
                 * Whenever 1-bits occur, there's a chance that they occured
                 * due to a random magnetic flux event, and can thus occur
                 * at any phase of the bit-cell clock.
                 * 
                 * It follows, therefore, that such events have a chance to
                 * advance the bit_counter by about 0,5 clocks each time they
                 * occur. Hence > 0 here, which filters out 50 % of events.
                 */
                if (drv->bit_counter < 7 && RANDOM_nextInt(drv) > 0) {
                    drv->bit_counter++;
                    drv->read_shift_reg <<= 1;
                }
            } else if ((drv->read_shift_reg & 0xf) == 0) {
                /* Simulate clock reset */
                drv->read_shift_reg++;
            }

            /* is sync? reset bit counter, don't move data, etc. */
            if (~drv->read_shift_reg & 0x3ff) {
                if (~drv->bit_counter & 7) {
                    drv->byte_ready &= drv->native;
                } else {
                    drv->byte_ready_edge |= drv->soe & ~drv->byte_ready;
                    drv->byte_ready |= drv->soe;
                    drv->write_shift_reg = drv->read_latch = (BYTE) drv->read_shift_reg;
                }
            } else {
                drv->bit_counter = 0;
            }
        } else {
        /* When writing, the first byte after transition is going to echo the
         * bits from the last read value.
         */
            drv->bit_counter++;

            drv->read_shift_reg <<= 1;
            if ((drv->read_shift_reg & 0xf) == 0) {
                drv->read_shift_reg++;
            }

            write_next_bit(drv, drv->write_shift_reg & 0x80);
            drv->write_shift_reg <<= 1;

            if (~drv->bit_counter & 7) {
                drv->byte_ready &= drv->native;
            } else {
                drv->byte_ready_edge |= drv->soe & ~drv->byte_ready;
                drv->byte_ready |= drv->soe;
                drv->write_shift_reg = drv->write_out;
            }
        }
    }
}

static void fdd_image_read(fd_drive_t *drv)
{
    if (!drv->image) {
        drv->raw = NULL;
        return;
    }

    drv->raw = drv->raw_cache[drv->side][drv->track];
    if (drv->raw) {
        return;
    }
    drv->raw = lib_calloc(1, sizeof(*drv->raw));
    drv->raw_cache[drv->side][drv->track] = drv->raw;
    disk_image_read_track(drv->image, drv->track, drv->side, drv->raw);
#if 0
    {
        int i;
        fprintf(stderr, "read %d:%d\n", drv->track, drv->side);
        for (i = 0; i < drv->raw->size; i++) {
            if (i % 20 == 0) fprintf(stderr, "%04x:", i);
            if (!(i & 3)) fprintf(stderr, " ");
            /*fprintf(stderr, "%c%02x", drv->raw->data[i + drv->raw->size] ? '*' : ' ', drv->raw->data[i]);*/
            fprintf(stderr, " %02x", drv->raw->data[i]);
            if (i % 20 == 19 || i == drv->raw->size - 1) fprintf(stderr, "\n");
        }
    }
#endif
    return;
}

static void fdd_image_writeback(fd_drive_t *drv, int free)
{
    if (!drv->image || !drv->raw)
        return;

    if (drv->raw->dirty) {
#if 0
    {
        int i;
        fprintf(stderr, "write %d:%d\n", drv->track, drv->side);
        for (i = 0; i < drv->raw->size; i++) {
            if (i % 20 == 0) fprintf(stderr, "%04x:", i);
            if (!(i & 3)) fprintf(stderr, " ");
            /*fprintf(stderr, "%c%02x", (drv->raw->data[i + drv->raw->size]) ? '*' : ' ', drv->raw->data[i]);*/
            fprintf(stderr, " %02x", drv->raw->data[i]);
            if (i % 20 == 19 || i == drv->raw->size - 1) fprintf(stderr, "\n");
        }
    }
#endif
        drv->raw->pinned = disk_image_write_track(drv->image, drv->track, drv->side, drv->raw);
        drv->raw->dirty = 0;
    }
    if (free && !drv->raw->pinned) {
        lib_free(drv->raw->data);
        lib_free(drv->raw);
        drv->raw_cache[drv->side][drv->track] = NULL;
        drv->raw = NULL;
    }
}
