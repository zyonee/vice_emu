/*
 * rotation.c
 *
 * Written by
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

#include "drive.h"
#include "drivetypes.h"
#include "rotation.h"
#include "types.h"
#include "diskimage.h"
#include "lib.h"

struct rotation_s {
    DWORD accum;
    CLOCK rotation_last_clk, *clk;

    unsigned int last_read_data;
    BYTE last_write_data;
    BYTE gcr_read, gcr_write;
    int bit_counter;
    int zero_count;
    int head;

    int frequency; /* 50000 for 2Mhz, 25000 for 1MHz */
    int rate;

    DWORD seed;
};
typedef struct rotation_s rotation_t;

void rotation_init(rotation_t *rotation, int freq)
{
    rotation->frequency = freq ? 50000 : 25000;
    rotation->accum = 0;
}

void rotation_reset(rotation_t *rotation)
{
    rotation->head = 0;
    rotation->gcr_read = 0;
    rotation->gcr_write = 0;
    rotation->last_read_data = 0;
    rotation->last_write_data = 0;
    rotation->bit_counter = 0;
    rotation->accum = 0;
    rotation->seed = 0;
    rotation->rotation_last_clk = *(rotation->clk);
}

void rotation_speed_zone_set(rotation_t *rotation, int rate)
{
    rotation->rate = rate;
}

void rotation_table_get(DWORD *rotation_table_ptr)
{
#if 0
    unsigned int dnr;
    drive_t *drive;

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;

        rotation_table_ptr[dnr] = rotation[dnr].speed_zone;

        drive->snap_accum = rotation[dnr].accum;
        drive->snap_rotation_last_clk = rotation[dnr].rotation_last_clk;
        drive->snap_last_read_data = rotation[dnr].last_read_data;
        drive->snap_last_write_data = rotation[dnr].last_write_data;
        drive->snap_bit_counter = rotation[dnr].bit_counter;
        drive->snap_zero_count = rotation[dnr].zero_count;
        drive->snap_seed = rotation[dnr].seed;
    }
#endif
}

void rotation_table_set(DWORD *rotation_table_ptr)
{
#if 0
    unsigned int dnr;
    drive_t *drive;

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;

        rotation[dnr].speed_zone = rotation_table_ptr[dnr];

        rotation[dnr].accum = drive->snap_accum;
        rotation[dnr].rotation_last_clk = drive->snap_rotation_last_clk;
        rotation[dnr].last_read_data = drive->snap_last_read_data;
        rotation[dnr].last_write_data = drive->snap_last_write_data;
        rotation[dnr].bit_counter = drive->snap_bit_counter;
        rotation[dnr].zero_count = drive->snap_zero_count;
        rotation[dnr].seed = drive->snap_seed;
    }
#endif
}

void rotation_overflow_callback(CLOCK sub, rotation_t *rotation)
{
    rotation->rotation_last_clk -= sub;
}

inline static void write_next_bit(drive_t *dptr, int value)
{
    int off = dptr->rotation->head;
    int byte_offset = off >> 3;
    int bit = off & 7;

    if (!dptr->raw) {
        return;
    }

    off++;
    if (off >= (dptr->raw->size << 3)) {
        off = 0;
    }
    dptr->rotation->head = off;

    if (value) {
        dptr->raw->data[byte_offset] |= 0x80 >> bit;
    } else {
        dptr->raw->data[byte_offset] &= ~(0x80 >> bit);
    }
    dptr->raw->dirty = 1;
}

inline static int read_next_bit(drive_t *dptr)
{
    int off = dptr->rotation->head;
    int byte_offset = off >> 3;
    int bit = off & 7;

    if (!dptr->raw) {
        return 0;
    }

    off++;
    if (off >= (dptr->raw->size << 3)) {
        off = 0;
    }
    dptr->rotation->head = off;

    return dptr->raw->data[byte_offset] & (0x80 >> bit);
}

inline static SDWORD RANDOM_nextInt(rotation_t *rptr) {
    DWORD bits = rptr->seed >> 15;
    rptr->seed ^= rptr->accum;
    rptr->seed = rptr->seed << 17 | bits;
    return (SDWORD) rptr->seed;
}

void rotation_begins(rotation_t *rotation) {
    rotation->rotation_last_clk = *(rotation->clk);
}

/* Rotate the disk according to the current value of `drive_clk[]'.  If
   `mode_change' is non-zero, there has been a Read -> Write mode switch.  */
void rotation_rotate_disk(drive_t *dptr)
{
    rotation_t *rptr;
    int bit;
    CLOCK clk;

    if ((dptr->byte_ready_active & 4) == 0) {
        return;
    }

    rptr = dptr->rotation;

    /* Calculate the number of bits that have passed under the R/W head since
       the last time.  */
    clk = *(dptr->clk) - rptr->rotation_last_clk;
    rptr->rotation_last_clk = *(dptr->clk);

    for (;;) {
        if (rptr->accum < rptr->frequency) {
            if (clk < 65536) {
                rptr->accum += clk * (dptr->raw ? dptr->raw->size : 6250);
                clk = 0;
                if (rptr->accum < rptr->frequency) {
                    break;
                }
            } else {
                rptr->accum += 65536 * (dptr->raw ? dptr->raw->size : 6250);
                clk -= 65536;
            }
        }
        rptr->accum -= rptr->frequency;
        if (dptr->read_write_mode) {
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

            bit = read_next_bit(dptr);
            rptr->last_read_data = ((rptr->last_read_data << 1) & 0x3fe);
            
            if (bit) {
                rptr->zero_count = 0;
                rptr->last_read_data |= 1;
            }

            /* Simulate random magnetic flux events in our lame-ass emulation. */
            if (++ rptr->zero_count > 8 && (rptr->last_read_data & 0x3f) == 0x8 && RANDOM_nextInt(rptr) > (1 << 30)) {
                rptr->last_read_data |= 1;
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
                if (rptr->bit_counter < 7 && RANDOM_nextInt(rptr) > 0) {
                    rptr->bit_counter ++;
                    rptr->last_read_data = (rptr->last_read_data << 1) & 0x3fe;
                }
            } else if ((rptr->last_read_data & 0xf) == 0) {
                /* Simulate clock reset */
                rptr->last_read_data |= 1;
            }
            rptr->last_write_data <<= 1;

            /* is sync? reset bit counter, don't move data, etc. */
            if (rptr->last_read_data == 0x3ff) {
                rptr->bit_counter = 0;
            } else {
                if (++ rptr->bit_counter == 8) {
                    rptr->bit_counter = 0;
                    rptr->gcr_read = (BYTE) rptr->last_read_data;
                    /* tlr claims that the write register is loaded at every
                     * byte boundary, and since the bus is shared, it's reasonable
                     * to guess that it would be loaded with whatever was last read. */
                    rptr->last_write_data = rptr->gcr_read;
                    if ((dptr->byte_ready_active & 2) != 0) {
                        dptr->byte_ready_edge = 1;
                        dptr->byte_ready_level = 1;
                    }
                }
            }
        } else {
        /* When writing, the first byte after transition is going to echo the
         * bits from the last read value.
         */
            rptr->last_read_data = (rptr->last_read_data << 1) & 0x3fe;
            if ((rptr->last_read_data & 0xf) == 0) {
                rptr->last_read_data |= 1;
            }

            write_next_bit(dptr, rptr->last_write_data & 0x80);
            rptr->last_write_data <<= 1;

            if (++ rptr->bit_counter == 8) {
                rptr->bit_counter = 0;
                rptr->last_write_data = rptr->gcr_write;
                if ((dptr->byte_ready_active & 2) != 0) {
                   dptr->byte_ready_edge = 1;
                   dptr->byte_ready_level = 1;
                }
            }
        }
    }
}

/* Return non-zero if the Sync mark is found.  It is required to
   call rotation_rotate_disk() to update drive[].GCR_head_offset first.
   The return value corresponds to bit#7 of VIA2 PRB. This means 0x0
   is returned when sync is found and 0x80 is returned when no sync
   is found.  */
BYTE rotation_sync_found(drive_t *dptr)
{
    if (dptr->read_write_mode == 0 || dptr->attach_clk != (CLOCK)0)
        return 0x80;

    return dptr->rotation->last_read_data == 0x3ff ? 0 : 0x80;
}

BYTE rotation_byte_read(drive_t *dptr)
{
    if (dptr->attach_clk != (CLOCK)0) {
        if (*(dptr->clk) - dptr->attach_clk < DRIVE_ATTACH_DELAY)
            dptr->rotation->gcr_read = 0;
        else
            dptr->attach_clk = (CLOCK)0;
    } else if (dptr->attach_detach_clk != (CLOCK)0) {
        if (*(dptr->clk) - dptr->attach_detach_clk < DRIVE_ATTACH_DETACH_DELAY)
            dptr->rotation->gcr_read = 0;
        else
            dptr->attach_detach_clk = (CLOCK)0;
    } else {
        rotation_rotate_disk(dptr);
    }
    return dptr->rotation->gcr_read;
}

inline void rotation_byte_write(drive_t *drive, BYTE data)
{
    if (drive->rotation->gcr_write != data) {
        rotation_rotate_disk(drive);
        drive->rotation->gcr_write = data;
    }
}

void rotation_reposition(drive_t *drive)
{
    drive->rotation->head = drive->rotation->head % (drive->raw->size << 3);
}

rotation_t *rotation_new(drive_t *drive)
{
    rotation_t *rotation;

    rotation = (rotation_t *)lib_calloc(1, sizeof(rotation_t));
    rotation->clk = drive->clk;

    return rotation;
}

void rotation_destroy(rotation_t *rotation)
{
    lib_free(rotation);
}
