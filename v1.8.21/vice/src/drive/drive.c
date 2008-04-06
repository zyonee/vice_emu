/*
 * drive.c - Hardware-level Commodore disk drive emulation.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *
 * Based on old code by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

/* TODO:
        - more accurate emulation of disk rotation.
        - different speeds within one track.
        - check for byte ready *within* `BVC', `BVS' and `PHP'.
        - serial bus handling might be faster.  */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include "attach.h"
#include "ciad.h"
#include "clkguard.h"
#include "diskimage.h"
#include "drive.h"
#include "drivecpu.h"
#include "driverom.h"
#include "drivetypes.h"
#include "fdc.h"
#include "gcr.h"
#include "iecdrive.h"
#include "log.h"
#include "machine.h"
#include "parallel.h"
#include "resources.h"
#include "riotd.h"
#include "serial.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "vdrive-bam.h"
#include "vdrive.h"
#include "viad.h"
#include "wd1770.h"

/* ------------------------------------------------------------------------- */

/* Drive specific variables.  */
drive_t drive[2];

drive_context_t drive0_context;
drive_context_t drive1_context;

/* Prototypes of functions called by resource management.  */
static int drive_check_image_format(unsigned int format, unsigned int dnr);

/* Generic drive logging goes here.  */
static log_t drive_log = LOG_ERR;

/* Pointer to the IEC bus structure.  */
static iec_info_t *iec_info;

/* If nonzero, at least one vaild drive ROM has already been loaded.  */
int rom_loaded = 0;

/* ------------------------------------------------------------------------- */

/* Speed (in bps) of the disk in the 4 disk areas.  */
static int rot_speed_bps[2][4] = { { 250000, 266667, 285714, 307692 },
                                   { 125000, 133333, 142857, 153846 } };

/* Number of bytes per track size.  */
static unsigned int raw_track_size[4] = { 6250, 6666, 7142, 7692 };

/* Clock speed of the PAL and NTSC versions of the connected computer.  */
static CLOCK pal_cycles_per_sec;
static CLOCK ntsc_cycles_per_sec;

static int drive_led_color[2];

#define GCR_OFFSET(track, sector)  ((track - 1) * NUM_MAX_BYTES_TRACK \
                                    + sector * NUM_BYTES_SECTOR_GCR)

static void initialize_rotation(int freq, unsigned int dnr);
static void drive_extend_disk_image(unsigned int dnr);
static void drive_set_half_track(int num, drive_t *dptr);
static void drive_clk_overflow_callback(CLOCK sub, void *data);
static void drive_set_clock_frequency(unsigned int type, unsigned int dnr);

/* ------------------------------------------------------------------------- */

/* Disk image handling. */

static void drive_read_image_d64_d71(unsigned int dnr)
{
    BYTE buffer[260], chksum;
    int i;
    unsigned int track, sector;

    if (!drive[dnr].image)
        return;

    buffer[258] = buffer[259] = 0;

    /* Since the D64/D71 format does not provide the actual track sizes or
       speed zones, we set them to standard values.  */
    if ((drive[dnr].image->type == DISK_IMAGE_TYPE_D64
        || drive[dnr].image->type == DISK_IMAGE_TYPE_D67
        || drive[dnr].image->type == DISK_IMAGE_TYPE_X64)
        && (drive[dnr].type == DRIVE_TYPE_1541
        || drive[dnr].type == DRIVE_TYPE_1541II
        || drive[dnr].type == DRIVE_TYPE_1551
        || drive[dnr].type == DRIVE_TYPE_2031)) {
        for (track = 0; track < MAX_TRACKS_1541; track++) {
            drive[dnr].gcr->track_size[track] =
                raw_track_size[disk_image_speed_map_1541(track)];
            memset(drive[dnr].gcr->speed_zone, disk_image_speed_map_1541(track),
                   NUM_MAX_BYTES_TRACK);
        }
    }
    if (drive[dnr].image->type == DISK_IMAGE_TYPE_D71
        || drive[dnr].type == DRIVE_TYPE_1571
        || drive[dnr].type == DRIVE_TYPE_2031) {
        for (track = 0; track < MAX_TRACKS_1571; track++) {
            drive[dnr].gcr->track_size[track] =
                raw_track_size[disk_image_speed_map_1571(track)];
            memset(drive[dnr].gcr->speed_zone, disk_image_speed_map_1571(track),
                   NUM_MAX_BYTES_TRACK);
        }
    }

    drive_set_half_track(drive[dnr].current_half_track, &drive[dnr]);

    for (track = 1; track <= drive[dnr].image->tracks; track++) {
        BYTE *ptr;
        unsigned int max_sector = 0;

        ptr = drive[dnr].gcr->data + GCR_OFFSET(track, 0);
        max_sector = disk_image_sector_per_track(drive[dnr].image->type,
                                                 track);
        /* Clear track to avoid read errors.  */
        memset(ptr, 0xff, NUM_MAX_BYTES_TRACK);

        for (sector = 0; sector < max_sector; sector++) {
            int rc;
            ptr = drive[dnr].gcr->data + GCR_OFFSET(track, sector);

            rc = disk_image_read_sector(drive[dnr].image, buffer + 1, track,
                                        sector);
            if (rc < 0) {
                log_error(drive[dnr].log,
                          "Cannot read T:%d S:%d from disk image.",
                          track, sector);
                          continue;
            }

            if (rc == 21) {
                ptr = drive[dnr].gcr->data + GCR_OFFSET(track, 0);
                memset(ptr, 0x00, NUM_MAX_BYTES_TRACK);
                break;
            }

            buffer[0] = (rc == 22) ? 0xff : 0x07;

            chksum = buffer[1];
            for (i = 2; i < 257; i++)
                chksum ^= buffer[i];
            buffer[257] = (rc == 23) ? chksum ^ 0xff : chksum;
            gcr_convert_sector_to_GCR(buffer, ptr, track, sector,
                                      drive[dnr].diskID1, drive[dnr].diskID2,
                                      (BYTE)(rc));
        }
    }
}

static int setID(unsigned int dnr)
{
    BYTE buffer[256];
    int rc;

    if (!drive[dnr].image)
        return -1;

    rc = disk_image_read_sector(drive[dnr].image, buffer, 18, 0);
    if (rc >= 0) {
        drive[dnr].diskID1 = buffer[0xa2];
        drive[dnr].diskID2 = buffer[0xa3];
    }

    return rc;
}

void drive_set_disk_id_memory(unsigned int dnr, BYTE *id)
{
   if (dnr == 0) {
       drive0_context.cpud.drive_ram[0x12] = id[0];
       drive0_context.cpud.drive_ram[0x13] = id[1];
   } else {
       drive1_context.cpud.drive_ram[0x12] = id[0];
       drive1_context.cpud.drive_ram[0x13] = id[1];
   }
}

/* ------------------------------------------------------------------------- */

/* Global clock counters.  */
CLOCK drive_clk[2];

/* Initialize the hardware-level drive emulation (should be called at least
   once before anything else).  Return 0 on success, -1 on error.  */
int drive_init(CLOCK pal_hz, CLOCK ntsc_hz)
{
    unsigned int track;
    int i, sync_factor;

    if (rom_loaded)
        return 0;

    drive_rom_init();

    pal_cycles_per_sec = pal_hz;
    ntsc_cycles_per_sec = ntsc_hz;

    drive[0].log = log_open("Drive 8");
    drive[1].log = log_open("Drive 9");
    drive_log = log_open("Drive");

    drive_clk[0] = 0L;
    drive_clk[1] = 0L;
    drive[0].clk = &drive_clk[0];
    drive[1].clk = &drive_clk[1];

    if (drive_rom_load_images() < 0) {
        resources_set_value("Drive8Type", (resource_value_t)DRIVE_TYPE_NONE);
        resources_set_value("Drive9Type", (resource_value_t)DRIVE_TYPE_NONE);
        return -1;
    }

    drive[0].drive_ram_expand2 = NULL;
    drive[0].drive_ram_expand4 = NULL;
    drive[0].drive_ram_expand6 = NULL;
    drive[0].drive_ram_expand8 = NULL;
    drive[0].drive_ram_expanda = NULL;
    drive[1].drive_ram_expand2 = NULL;
    drive[1].drive_ram_expand4 = NULL;
    drive[1].drive_ram_expand6 = NULL;
    drive[1].drive_ram_expand8 = NULL;
    drive[1].drive_ram_expanda = NULL;

    iec_info = iec_get_drive_port();
    /* Set IEC lines of disabled drives to `1'.  */
    if (iec_info != NULL) {
        iec_info->drive_bus = 0xff;
        iec_info->drive_data = 0xff;
        iec_info->drive2_bus = 0xff;
        iec_info->drive2_data = 0xff;
    }

    log_message(drive_log, "Finished loading ROM images.");
    rom_loaded = 1;

    if (drive_check_type(drive[0].type, 0) < 1)
        resources_set_value("Drive8Type", (resource_value_t)DRIVE_TYPE_NONE);
    if (drive_check_type(drive[1].type, 1) < 1)
        resources_set_value("Drive9Type", (resource_value_t)DRIVE_TYPE_NONE);

    drive_rom_setup_image(0);
    drive_rom_setup_image(1);

    clk_guard_add_callback(drive0_context.cpu.clk_guard,
                           drive_clk_overflow_callback, (void *)0);
    clk_guard_add_callback(drive1_context.cpu.clk_guard,
                           drive_clk_overflow_callback, (void *)1);

    for (i = 0; i < 2; i++) {
        drive[i].gcr = gcr_create_image();
        drive[i].byte_ready = 1;
        drive[i].GCR_dirty_track = 0;
        drive[i].GCR_write_value = 0x55;
        drive[i].GCR_track_start_ptr = drive[i].gcr->data;
        drive[i].GCR_current_track_size = 0;
        drive[i].attach_clk = (CLOCK)0;
        drive[i].detach_clk = (CLOCK)0;
        drive[i].attach_detach_clk = (CLOCK)0;
        drive[i].bits_moved = drive[i].accum = 0;
        drive[i].finish_byte = 0;
        drive[i].last_mode = 1;
        drive[i].rotation_last_clk = 0L;
        drive[i].have_new_disk = 0;
        drive[i].rotation_table_ptr = drive[i].rotation_table[0];
        drive[i].old_led_status = 0;
        drive[i].old_half_track = 0;
        drive[i].side = 0;
        drive[i].GCR_image_loaded = 0;
        drive[i].read_only = 0;
        drive[i].clock_frequency = 1;

        for (track = 0; track < MAX_TRACKS_1541; track++)
            drive[i].gcr->track_size[track] =
                raw_track_size[disk_image_speed_map_1541(track)];

        /* Position the R/W head on the directory track.  */
        drive_set_half_track(36, &drive[i]);
        drive_led_color[i] = DRIVE_ACTIVE_RED;
    }

    drive_rom_initialize_traps(0);
    drive_rom_initialize_traps(1);

    drive_set_clock_frequency(drive[0].type, 0);
    drive_set_clock_frequency(drive[1].type, 1);

    initialize_rotation(0, 0);
    initialize_rotation(0, 1);

    drive_cpu_init(&drive0_context, drive[0].type);
    drive_cpu_init(&drive1_context, drive[1].type);

    /* Make sure the sync factor is acknowledged correctly.  */
    resources_get_value("VideoStandard", (resource_value_t *)&sync_factor);
    resources_set_value("VideoStandard", (resource_value_t)sync_factor);

    /* Make sure the traps are moved as needed.  */
    if (drive[0].enable)
        drive_enable(0);
    if (drive[1].enable)
        drive_enable(1);

    return 0;
}

void drive_set_active_led_color(unsigned int type, unsigned int dnr)
{
    switch (type) {
      case DRIVE_TYPE_1541:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      case DRIVE_TYPE_1541II:
      case DRIVE_TYPE_1551:
        drive_led_color[dnr] = DRIVE_ACTIVE_GREEN;
        break;
      case DRIVE_TYPE_1571:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      case DRIVE_TYPE_1581:
        drive_led_color[dnr] = DRIVE_ACTIVE_GREEN;
        break;
      case DRIVE_TYPE_2031:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      case DRIVE_TYPE_2040:
      case DRIVE_TYPE_3040:
      case DRIVE_TYPE_4040:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      case DRIVE_TYPE_1001:
      case DRIVE_TYPE_8050:
      case DRIVE_TYPE_8250:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      default:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
    }
}

static void drive_set_clock_frequency(unsigned int type, unsigned int dnr)
{
    switch (type) {
      case DRIVE_TYPE_1541:
        drive[dnr].clock_frequency = 1;
        break;
      case DRIVE_TYPE_1541II:
        drive[dnr].clock_frequency = 1;
        break;
      case DRIVE_TYPE_1551:
        drive[dnr].clock_frequency = 2;
        break;
      case DRIVE_TYPE_1571:
        drive[dnr].clock_frequency = 1;
        break;
      case DRIVE_TYPE_1581:
        drive[dnr].clock_frequency = 2;
        break;
      case DRIVE_TYPE_2031:
        drive[dnr].clock_frequency = 1;
        break;
      case DRIVE_TYPE_2040:
      case DRIVE_TYPE_3040:
      case DRIVE_TYPE_4040:
      case DRIVE_TYPE_1001:
      case DRIVE_TYPE_8050:
      case DRIVE_TYPE_8250:
        drive[dnr].clock_frequency = 1;
        break;
      default:
        drive[dnr].clock_frequency = 1;
    }
}

int drive_set_disk_drive_type(unsigned int type, unsigned int dnr)
{
    int sync_factor;

    if (drive_rom_check_loaded(type) < 0)
        return -1;

    if (drive[dnr].byte_ready_active == 0x06)
        drive_rotate_disk(&drive[dnr]);

    drive_set_clock_frequency(type, dnr);

    initialize_rotation(0, dnr);
    drive[dnr].type = type;
    drive[dnr].side = 0;
    drive_rom_setup_image(dnr);
    resources_get_value("VideoStandard", (resource_value_t *)&sync_factor);
    resources_set_value("VideoStandard", (resource_value_t)sync_factor);
    drive_set_active_led_color(type, dnr);

    if (dnr == 0)
        drive_cpu_init(&drive0_context, type);
    if (dnr == 1)
        drive_cpu_init(&drive1_context, type);

    return 0;
}


/* Activate full drive emulation. */
int drive_enable(unsigned int dnr)
{
    int i, drive_true_emulation = 0;

    /* This must come first, because this might be called before the drive
       initialization.  */
    if (!rom_loaded)
        return -1;

    resources_get_value("DriveTrueEmulation",
                        (resource_value_t *)&drive_true_emulation);

    /* Always disable kernal traps. */
    if (drive_true_emulation)
        serial_remove_traps();
    else
        return 0;

    if (drive[dnr].type == DRIVE_TYPE_NONE)
        return 0;

    /* Recalculate drive geometry.  */
    if (drive[dnr].image != NULL)
        drive_attach_image(drive[dnr].image, dnr + 8);

    if (dnr == 0)
        drive_cpu_wake_up(&drive0_context);
    if (dnr == 1)
        drive_cpu_wake_up(&drive1_context);

    /* Make sure the UI is updated.  */
    for (i = 0; i < 2; i++) {
        if (drive[i].enable) {
            drive[i].old_led_status = -1;
            drive[i].old_half_track = -1;
        }
    }

    drive_set_active_led_color(drive[dnr].type, dnr);
    ui_enable_drive_status((drive[0].enable ? UI_DRIVE_ENABLE_0 : 0)
                           | ((drive[1].enable
                           || (drive[0].enable && DRIVE_IS_DUAL(drive[0].type))
                           ) ? UI_DRIVE_ENABLE_1 : 0),
                           drive_led_color);

    return 0;
}

/* Disable full drive emulation.  */
void drive_disable(unsigned int dnr)
{
    int i, drive_true_emulation = 0;

    /* This must come first, because this might be called before the true
       drive initialization.  */
    drive[dnr].enable = 0;
    iec_calculate_callback_index();

    resources_get_value("DriveTrueEmulation",
                        (resource_value_t *)&drive_true_emulation);

    if (rom_loaded && !drive_true_emulation)
        serial_install_traps();

    if (rom_loaded){
        if (dnr == 0)
            drive_cpu_sleep(&drive0_context);
        if (dnr == 1)
            drive_cpu_sleep(&drive1_context);
        /* Set IEC lines of disabled drives to `1'.  */
        if (dnr == 0 && iec_info != NULL) {
            iec_info->drive_bus = 0xff;
            iec_info->drive_data = 0xff;
        }
        if (dnr == 1 && iec_info != NULL) {
            iec_info->drive2_bus = 0xff;
            iec_info->drive2_data = 0xff;
        }
    if (dnr == 0)
        drive_gcr_data_writeback(0);
    if (dnr == 1)
        drive_gcr_data_writeback(1);
    }

    /* Make sure the UI is updated.  */
    for (i = 0; i < 2; i++) {
        if (drive[i].enable) {
            drive[i].old_led_status = -1;
            drive[i].old_half_track = -1;
        }
    }

    ui_enable_drive_status((drive[0].enable ? UI_DRIVE_ENABLE_0 : 0)
                           | ((drive[1].enable
                           || (drive[0].enable && DRIVE_IS_DUAL(drive[0].type))
                           ) ? UI_DRIVE_ENABLE_1 : 0),
                           drive_led_color);
/*
    ui_enable_drive_status((drive[0].enable ? UI_DRIVE_ENABLE_0 : 0)
                           | (drive[1].enable ? UI_DRIVE_ENABLE_1 : 0),
                           drive_led_color);
*/
}

void drive_reset(void)
{
    drive_cpu_reset(&drive0_context);
    drive_cpu_reset(&drive1_context);
}

/* ------------------------------------------------------------------------- */
/* Check if the drive type matches the disk image type.  */
static int drive_check_image_format(unsigned int format, unsigned int dnr)
{
    switch (format) {
      case DISK_IMAGE_TYPE_D64:
      case DISK_IMAGE_TYPE_G64:
      case DISK_IMAGE_TYPE_X64:
        if (drive[dnr].type != DRIVE_TYPE_1541
            && drive[dnr].type != DRIVE_TYPE_1541II
            && drive[dnr].type != DRIVE_TYPE_1551
            && drive[dnr].type != DRIVE_TYPE_1571
            && drive[dnr].type != DRIVE_TYPE_2031
            && drive[dnr].type != DRIVE_TYPE_2040 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_3040
            && drive[dnr].type != DRIVE_TYPE_4040)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D67:
        if (drive[dnr].type != DRIVE_TYPE_1541 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_1541II /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_1551 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_1571 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_2031 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_2040
            && drive[dnr].type != DRIVE_TYPE_3040 /* FIXME: only read compat */
            && drive[dnr].type != DRIVE_TYPE_4040) /* FIXME: only read compat */
            return -1;
        break;
      case DISK_IMAGE_TYPE_D71:
        if (drive[dnr].type != DRIVE_TYPE_1571)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D81:
        if (drive[dnr].type != DRIVE_TYPE_1581)
            return -1;
        break;
      case DISK_IMAGE_TYPE_D80:
      case DISK_IMAGE_TYPE_D82:
        if ((drive[dnr].type != DRIVE_TYPE_1001)
            && (drive[dnr].type != DRIVE_TYPE_8050)
            && (drive[dnr].type != DRIVE_TYPE_8250))
            return -1;
        break;
      default:
        return -1;
    }
    return 0;
}

/* Attach a disk image to the true drive emulation. */
int drive_attach_image(disk_image_t *image, unsigned int unit)
{
    unsigned int dnr;

    if (unit != 8 && unit != 9)
        return -1;

    dnr = unit - 8;

    if (drive_check_image_format(image->type, dnr) < 0)
        return -1;

    drive[dnr].read_only = image->read_only;
    drive[dnr].have_new_disk = 1;
    drive[dnr].attach_clk = drive_clk[dnr];
    if (drive[dnr].detach_clk > (CLOCK) 0)
        drive[dnr].attach_detach_clk = drive_clk[dnr];
    drive[dnr].ask_extend_disk_image = 1;

    switch(image->type) {
      case DISK_IMAGE_TYPE_D64:
        log_message(drive_log, "Unit %d: D64 disk image attached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D67:
        log_message(drive_log, "Unit %d: D67 disk image attached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D71:
        log_message(drive_log, "Unit %d: D71 disk image attached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_G64:
        log_message(drive_log, "Unit %d: G64 disk image attached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_X64:
        log_message(drive_log, "Unit %d: X64 disk image attached: %s.",
                    unit, image->name);
        break;
      default:
        return -1;
    }

    drive[dnr].image = image;
    drive[dnr].image->gcr = drive[dnr].gcr;

    if (drive[dnr].image->type == DISK_IMAGE_TYPE_G64) {
        if (disk_image_read_gcr_image(drive[dnr].image) < 0) {
            return -1;
        }
    } else {
        if (setID(dnr) >= 0) {
            drive_read_image_d64_d71(dnr);
            drive[dnr].GCR_image_loaded = 1;
            return 0;
        } else {
            return -1;
        }
    }
    drive[dnr].GCR_image_loaded = 1;

    return 0;
}

/* Detach a disk image from the true drive emulation. */
int drive_detach_image(disk_image_t *image, unsigned int unit)
{
    unsigned int dnr;

    if (unit != 8 && unit != 9)
        return -1;

    dnr = unit - 8;

    if (drive[dnr].image != NULL) {
        switch(image->type) {
          case DISK_IMAGE_TYPE_D64:
            log_message(drive_log, "Unit %d: D64 disk image detached: %s.",
                        unit, image->name);
            break;
          case DISK_IMAGE_TYPE_D67:
            log_message(drive_log, "Unit %d: D67 disk image detached: %s.",
                        unit, image->name);
            break;
          case DISK_IMAGE_TYPE_D71:
            log_message(drive_log, "Unit %d: D71 disk image detached: %s.",
                        unit, image->name);
            break;
          case DISK_IMAGE_TYPE_G64:
            log_message(drive_log, "Unit %d: G64 disk image detached: %s.",
                        unit, image->name);
            break;
          case DISK_IMAGE_TYPE_X64:
            log_message(drive_log, "Unit %d: X64 disk image detached: %s.",
                        unit, image->name);
            break;
          default:
            return -1;
        }

        drive_gcr_data_writeback(dnr);
        memset(drive[dnr].gcr->data, 0, MAX_GCR_TRACKS * NUM_MAX_BYTES_TRACK);
        drive[dnr].detach_clk = drive_clk[dnr];
        drive[dnr].GCR_image_loaded = 0;
        drive[dnr].read_only = 0;
        drive[dnr].image = NULL;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

/* Initialization.  */

static void initialize_rotation(int freq, unsigned int dnr)
{
    drive_initialize_rotation_table(freq, dnr);
    drive[dnr].bits_moved = drive[dnr].accum = 0;
}

void drive_initialize_rotation_table(int freq, unsigned int dnr)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        int speed = rot_speed_bps[freq][i];

        for (j = 0; j < ROTATION_TABLE_SIZE; j++) {
            double bits = (double)j * (double)speed / 1000000.0;

            drive[dnr].rotation_table[i][j].bits = (unsigned long)bits;
            drive[dnr].rotation_table[i][j].accum = (unsigned long)(((bits -
                                          (unsigned long)bits) * ACCUM_MAX));
        }
    }
}
/* ------------------------------------------------------------------------- */

/* Clock overflow handing.  */

static void drive_clk_overflow_callback(CLOCK sub, void *data)
{
    unsigned int drive_num;
    drive_t *d;

    drive_num = (unsigned int) data;
    d = &drive[drive_num];

    if (d->byte_ready_active == 0x06)
        drive_rotate_disk(&drive[drive_num]);
    d->rotation_last_clk -= sub;
    if (d->attach_clk > (CLOCK) 0)
        d->attach_clk -= sub;
    if (d->detach_clk > (CLOCK) 0)
        d->detach_clk -= sub;
    if (d->attach_detach_clk > (CLOCK) 0)
        d->attach_detach_clk -= sub;

    /* FIXME: Having to do this by hand sucks *big time*!  These should be in
       `drive_t'.  */
    switch (drive_num) {
      case 0:
        alarm_context_time_warp(drive0_context.cpu.alarm_context, sub, -1);
        interrupt_cpu_status_time_warp(&drive0_context.cpu.int_status, sub, -1);
        break;
      case 1:
        alarm_context_time_warp(drive1_context.cpu.alarm_context, sub, -1);
        interrupt_cpu_status_time_warp(&drive1_context.cpu.int_status, sub, -1);
        break;
      default:
        log_error(drive_log,
                  "Unexpected drive number %d in drive_clk_overflow_callback",
                  drive_num);
    }
}

CLOCK drive_prevent_clk_overflow(CLOCK sub, unsigned int dnr)
{
    /* FIXME: Having to do this by hand sucks *big time*!  */
    switch (dnr) {
      case 0:
        return drive_cpu_prevent_clk_overflow(&drive0_context, sub);
      case 1:
        return drive_cpu_prevent_clk_overflow(&drive1_context, sub);
      default:
        log_error(drive_log,
                  "Unexpected drive number %d in `drive_prevent_clk_overflow()'\n",
                  dnr);
        return 0;
    }
}

/*-------------------------------------------------------------------------- */

/* The following functions are time critical.  */

/* Return non-zero if the Sync mark is found.  It is required to
   call drive_rotate_disk() to update drive[].GCR_head_offset first.
   The return value corresponds to bit#7 of VIA2 PRB. This means 0x0
   is returned when sync is found and 0x80 is returned when no sync
   is found.  */
inline BYTE drive_sync_found(drive_t *dptr)
{
    BYTE val = dptr->GCR_track_start_ptr[dptr->GCR_head_offset];

    if (val != 0xff || dptr->last_mode == 0) {
        return 0x80;
    } else {
        unsigned int previous_head_offset;

        previous_head_offset = (dptr->GCR_head_offset > 0
                               ? dptr->GCR_head_offset - 1
                               : dptr->GCR_current_track_size - 1);

        if ((dptr->GCR_track_start_ptr[previous_head_offset] & 3) != 3) {
            if (dptr->shifter >= 2) {
                unsigned int next_head_offset;

                next_head_offset = ((dptr->GCR_head_offset
                                   < (dptr->GCR_current_track_size - 1))
                                   ? dptr->GCR_head_offset + 1 : 0);

                if ((dptr->GCR_track_start_ptr[next_head_offset] & 0xc0)
                    == 0xc0)
                    return 0;
            }
            return 0x80;
        }
        /* As the current rotation code cannot cope with non byte aligned
           writes, do not change `drive[].bits_moved'!  */
        /* dptr->bits_moved = 0; */
        return 0;
    }
}

/* Rotate the disk according to the current value of `drive_clk[]'.  If
   `mode_change' is non-zero, there has been a Read -> Write mode switch.  */
void drive_rotate_disk(drive_t *dptr)
{
    unsigned long new_bits;

    /* Calculate the number of bits that have passed under the R/W head since
       the last time.  */

    CLOCK delta = *(dptr->clk) - dptr->rotation_last_clk;

    new_bits = 0;
    while (delta > 0) {
        if (delta >= ROTATION_TABLE_SIZE) {
            struct _rotation_table *p = (dptr->rotation_table_ptr
                                         + ROTATION_TABLE_SIZE - 1);
            new_bits += p->bits;
            dptr->accum += p->accum;
            delta -= ROTATION_TABLE_SIZE - 1;
        } else {
            struct _rotation_table *p = dptr->rotation_table_ptr + delta;
            new_bits += p->bits;
            dptr->accum += p->accum;
            delta = 0;
        }
        if (dptr->accum >= ACCUM_MAX) {
            dptr->accum -= ACCUM_MAX;
            new_bits++;
        }
    }

    dptr->shifter = dptr->bits_moved + new_bits;

    if (dptr->shifter >= 8) {

        dptr->bits_moved += new_bits;
        dptr->rotation_last_clk = *(dptr->clk);

        if (dptr->finish_byte) {
            if (dptr->last_mode == 0) { /* write */
                dptr->GCR_dirty_track = 1;
                if (dptr->bits_moved >= 8) {
                    dptr->GCR_track_start_ptr[dptr->GCR_head_offset]
                        = dptr->GCR_write_value;
                    dptr->GCR_head_offset = ((dptr->GCR_head_offset + 1) %
                                             dptr->GCR_current_track_size);
                    dptr->bits_moved -= 8;
                }
            } else {            /* read */
                if (dptr->bits_moved >= 8) {
                    dptr->GCR_head_offset = ((dptr->GCR_head_offset + 1) %
                                             dptr->GCR_current_track_size);
                    dptr->bits_moved -= 8;
                    dptr->GCR_read = dptr->GCR_track_start_ptr[dptr->GCR_head_offset];
                }
            }

            dptr->finish_byte = 0;
            dptr->last_mode = dptr->read_write_mode;
        }

        if (dptr->last_mode == 0) {     /* write */
            dptr->GCR_dirty_track = 1;
            while (dptr->bits_moved >= 8) {
                dptr->GCR_track_start_ptr[dptr->GCR_head_offset]
                    = dptr->GCR_write_value;
                dptr->GCR_head_offset = ((dptr->GCR_head_offset + 1)
                                         % dptr->GCR_current_track_size);
                dptr->bits_moved -= 8;
            }
        } else {                /* read */
            dptr->GCR_head_offset = ((dptr->GCR_head_offset
                                      + dptr->bits_moved / 8)
                                     % dptr->GCR_current_track_size);
            dptr->bits_moved %= 8;
            dptr->GCR_read = dptr->GCR_track_start_ptr[dptr->GCR_head_offset];
        }

        dptr->shifter = dptr->bits_moved;

        /* The byte ready line is only set when no sync is found.  */
        if (drive_sync_found(dptr))
            dptr->byte_ready = 1;
    } /* if (dptr->shifter >= 8) */
}


/* Move the head to half track `num'.  */
static void drive_set_half_track(int num, drive_t *dptr)
{
    if ((dptr->type == DRIVE_TYPE_1541 || dptr->type == DRIVE_TYPE_1541II
        || dptr->type == DRIVE_TYPE_1541 || dptr->type == DRIVE_TYPE_2031)
        && num > 84)
        num = 84;
    if (dptr->type == DRIVE_TYPE_1571 && num > 140)
        num = 140;
    if (num < 2)
        num = 2;

    dptr->current_half_track = num;
    dptr->GCR_track_start_ptr = (dptr->gcr->data
                                + ((dptr->current_half_track / 2 - 1)
                                * NUM_MAX_BYTES_TRACK));

    if (dptr->GCR_current_track_size != 0)
        dptr->GCR_head_offset *= (dptr->gcr->track_size[dptr->current_half_track                                 / 2 - 1]) / dptr->GCR_current_track_size;
    else
        dptr->GCR_head_offset = 0;

    dptr->GCR_current_track_size =
        dptr->gcr->track_size[dptr->current_half_track / 2 - 1];
}

/* Return the write protect sense status. */
inline BYTE drive_write_protect_sense(drive_t *dptr)
{
    /* Set the write protection bit for the time the disk is pulled out on
       detach.  */
    if (dptr->detach_clk != (CLOCK)0) {
        if ((*(dptr->clk)) - dptr->detach_clk < DRIVE_DETACH_DELAY)
            return 0x10;
        dptr->detach_clk = (CLOCK)0;
    }
    /* Clear the write protection bit for the minimum time until a new disk
       can be inserted.  */
    if (dptr->attach_detach_clk != (CLOCK)0) {
        if ((*(dptr->clk)) - dptr->attach_detach_clk
            < DRIVE_ATTACH_DETACH_DELAY)
            return 0x0;
        dptr->attach_detach_clk = (CLOCK)0;
    }
    /* Set the write protection bit for the time the disk is put in on
       attach.  */
    if (dptr->attach_clk != (CLOCK)0) {
        if (((*(dptr->clk)) - dptr->attach_clk < DRIVE_ATTACH_DELAY))
            return 0x10;
        dptr->attach_clk = (CLOCK)0;
    }

    if (dptr->GCR_image_loaded == 0) {
        /* No disk in drive, write protection is on. */
        return 0x0;
    } else if (dptr->have_new_disk) {
        /* Disk has changed, make sure the drive sees at least one change in
           the write protect status. */
        dptr->have_new_disk = 0;
        return dptr->read_only ? 0x10 : 0x0;
    } else {
        return dptr->read_only ? 0x0 : 0x10;
    }
}

void drive_update_viad2_pcr(int pcrval, drive_t *dptr)
{
    dptr->read_write_mode = pcrval & 0x20;
    dptr->byte_ready_active = (dptr->byte_ready_active & ~0x02)
                              | (pcrval & 0x02);
}

BYTE drive_read_viad2_prb(drive_t *dptr)
{
    if (dptr->byte_ready_active == 0x06)
        drive_rotate_disk(dptr);
    return drive_sync_found(dptr) | drive_write_protect_sense(dptr);
}

/* End of time critical functions.  */
/*-------------------------------------------------------------------------- */

void drive_set_1571_side(int side, unsigned int dnr)
{
    unsigned int num;

    num  = drive[dnr].current_half_track;

    if (drive[dnr].byte_ready_active == 0x06)
        drive_rotate_disk(&drive[dnr]);

    drive_gcr_data_writeback(dnr);

    drive[dnr].side = side;
    if (num > 70)
        num -= 70;
    num += side * 70;

    drive_set_half_track(num, &drive[dnr]);
}

/* Increment the head position by `step' half-tracks. Valid values
   for `step' are `+1' and `-1'.  */
void drive_move_head(int step, unsigned int dnr)
{
    drive_gcr_data_writeback(dnr);
    if (drive[dnr].type == DRIVE_TYPE_1571) {
        if (drive[dnr].current_half_track + step == 71)
            return;
    }
    drive_set_half_track(drive[dnr].current_half_track + step, &drive[dnr]);
}

/* Hack... otherwise you get internal compiler errors when optimizing on
    gcc2.7.2 on RISC OS */
static void gcr_data_writeback2(BYTE *buffer, BYTE *offset, unsigned int dnr,
                                unsigned int track, unsigned int sector)
{
    int rc;

    gcr_convert_GCR_to_sector(buffer, offset,
                              drive[dnr].GCR_track_start_ptr,
                              drive[dnr].GCR_current_track_size);
    if (buffer[0] != 0x7) {
        log_error(drive[dnr].log,
                  "Could not find data block id of T:%d S:%d.",
                  track, sector);
    } else {
        rc = disk_image_write_sector(drive[dnr].image, buffer + 1, track,
                                     sector);
        if (rc < 0)
            log_error(drive[dnr].log,
                      "Could not update T:%d S:%d.", track, sector);
    }
}

void drive_gcr_data_writeback(unsigned int dnr)
{
    int extend;
    unsigned int track, sector, max_sector = 0;
    BYTE buffer[260], *offset;

    if (drive[dnr].image == NULL)
        return;

    track = drive[dnr].current_half_track / 2;

    if (!drive[dnr].GCR_dirty_track)
        return;

    if (drive[dnr].image->type == DISK_IMAGE_TYPE_G64) {
        BYTE *gcr_track_start_ptr;
        unsigned int gcr_current_track_size;

        gcr_current_track_size = drive[dnr].gcr->track_size[track - 1];

        gcr_track_start_ptr = drive[dnr].gcr->data
                              + ((track - 1) * NUM_MAX_BYTES_TRACK);

        disk_image_write_track(drive[dnr].image, track,
                               gcr_current_track_size,
                               drive[dnr].gcr->speed_zone,
                               gcr_track_start_ptr);
        drive[dnr].GCR_dirty_track = 0;
        return;
    }

    if (drive[dnr].image->type == DISK_IMAGE_TYPE_D64
        || drive[dnr].image->type == DISK_IMAGE_TYPE_X64) {
        if (track > EXT_TRACKS_1541)
            return;
        max_sector = disk_image_sector_per_track(DISK_IMAGE_TYPE_D64, track);
        if (track > drive[dnr].image->tracks) {
            switch (drive[dnr].extend_image_policy) {
              case DRIVE_EXTEND_NEVER:
                drive[dnr].ask_extend_disk_image = 1;
                return;
              case DRIVE_EXTEND_ASK:
                if (drive[dnr].ask_extend_disk_image == 1) {
                    extend = ui_extend_image_dialog();
                    if (extend == 0) {
                        drive[dnr].ask_extend_disk_image = 0;
                        return;
                    } else {
                        drive_extend_disk_image(dnr);
                    }
                } else {
                    return;
                }
                break;
              case DRIVE_EXTEND_ACCESS:
                drive[dnr].ask_extend_disk_image = 1;
                drive_extend_disk_image(dnr);
                break;
            }
        }
    }

    if (drive[dnr].image->type == DISK_IMAGE_TYPE_D71) {
        if (track > MAX_TRACKS_1571)
            return;
        max_sector = disk_image_sector_per_track(DISK_IMAGE_TYPE_D71, track);
    }

    drive[dnr].GCR_dirty_track = 0;

    for (sector = 0; sector < max_sector; sector++) {

        offset = gcr_find_sector_header(track, sector,
                                        drive[dnr].GCR_track_start_ptr,
                                        drive[dnr].GCR_current_track_size);
        if (offset == NULL) {
            log_error(drive[dnr].log,
                      "Could not find header of T:%d S:%d.",
                      track, sector);
        } else {
            offset = gcr_find_sector_data(offset,
                                          drive[dnr].GCR_track_start_ptr,
                                          drive[dnr].GCR_current_track_size);
            if (offset == NULL) {
                log_error(drive[dnr].log,
                          "Could not find data sync of T:%d S:%d.",
                          track, sector);
            } else {
                gcr_data_writeback2(buffer, offset, dnr, track, sector);
            }
        }
    }
}

static void drive_extend_disk_image(unsigned int dnr)
{
    int rc;
    unsigned int track, sector;
    BYTE buffer[256];

    drive[dnr].image->tracks = EXT_TRACKS_1541;
    memset(buffer, 0, 256);
    for (track = NUM_TRACKS_1541 + 1; track <= EXT_TRACKS_1541; track++) {
        for (sector = 0;
             sector < disk_image_sector_per_track(DISK_IMAGE_TYPE_D64, track);
             sector++) {
             rc = disk_image_write_sector(drive[dnr].image, buffer, track,
                                          sector);
             if (rc < 0)
                 log_error(drive[dnr].log,
                           "Could not update T:%d S:%d.", track, sector);
        }
    }
}

int drive_match_bus(unsigned int drive_type, unsigned int drv, int bus_map)
{
    if ( (drive_type == DRIVE_TYPE_NONE)
      || (DRIVE_IS_IEEE(drive_type) && (bus_map & IEC_BUS_IEEE))
      || ((!DRIVE_IS_IEEE(drive_type)) && (bus_map & IEC_BUS_IEC))
    ) {
        return 1;
    }
    return 0;
}

int drive_check_type(unsigned int drive_type, unsigned int dnr)
{
    if (!drive_match_bus(drive_type, dnr, iec_available_busses()))
        return 0;

    if (DRIVE_IS_DUAL(drive_type)) {
        if (dnr > 0) {
            /* A second dual drive is not supported.  */
            return 0;
        } else {
            if (drive[1].type != DRIVE_TYPE_NONE)
                /* Disable dual drive if second drive is enabled.  */
                return 0;
        }
    }

    /* If the first drive is dual no second drive is supported at all.  */
    if (DRIVE_IS_DUAL(drive[0].type) && dnr > 0)
        return 0;

    switch (drive_type) {
      case DRIVE_TYPE_NONE:
        return 1;
      case DRIVE_TYPE_1541:
        return rom1541_loaded;
      case DRIVE_TYPE_1541II:
        return rom1541ii_loaded;
      case DRIVE_TYPE_1551:
        return rom1551_loaded;
      case DRIVE_TYPE_1571:
        return rom1571_loaded;
      case DRIVE_TYPE_1581:
        return rom1581_loaded;
      case DRIVE_TYPE_2031:
        return rom2031_loaded;
      case DRIVE_TYPE_2040:
        return rom2040_loaded;
      case DRIVE_TYPE_3040:
        return rom3040_loaded;
      case DRIVE_TYPE_4040:
        return rom4040_loaded;
      case DRIVE_TYPE_1001:
      case DRIVE_TYPE_8050:
      case DRIVE_TYPE_8250:
        return rom1001_loaded;
      default:
        log_error(drive[dnr].log, "Unknown drive type %i.", drive_type);
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

/* Set the sync factor between the computer and the drive.  */

void drive_set_sync_factor(unsigned int factor)
{
    drive_cpu_set_sync_factor(&drive0_context,
                              drive[0].clock_frequency * factor);
    drive_cpu_set_sync_factor(&drive1_context,
                              drive[1].clock_frequency * factor);
}

void drive_set_pal_sync_factor(void)
{
    if (pal_cycles_per_sec != 0) {
        unsigned int new_sync_factor = (unsigned int)
                                       floor(65536.0 * (1000000.0 /
                                       ((double)pal_cycles_per_sec)));
        drive_set_sync_factor(new_sync_factor);
    }
}

void drive_set_ntsc_sync_factor(void)
{
    if (ntsc_cycles_per_sec != 0) {
        unsigned int new_sync_factor = (unsigned int)
                                       floor(65536.0 * (1000000.0 /
                                       ((double)ntsc_cycles_per_sec)));

        drive_set_sync_factor(new_sync_factor);
    }
}

void drive_set_1571_sync_factor(int new_sync, unsigned int dnr)
{
    int sync_factor;

    if (rom_loaded) {
        if (drive[dnr].byte_ready_active == 0x06)
            drive_rotate_disk(&drive[dnr]);
        initialize_rotation(new_sync ? 1 : 0, dnr);
        drive[dnr].clock_frequency = (new_sync) ? 2 : 1;
        resources_get_value("VideoStandard", (resource_value_t *)&sync_factor);
        resources_set_value("VideoStandard", (resource_value_t)sync_factor);
    }
}

/* ------------------------------------------------------------------------- */

/* Update the status bar in the UI.  */
void drive_update_ui_status(void)
{
    int i;

    if (console_mode || vsid_mode) {
        return;
    }

    /* Update the LEDs and the track indicators.  */
    for (i = 0; i < 2; i++) {
        if (drive[i].enable
            || ((i == 1) && drive[0].enable && DRIVE_IS_DUAL(drive[0].type))) {
            int my_led_status = 0;

            /* Actually update the LED status only if the `trap idle'
               idling method is being used, as the LED status could be
               incorrect otherwise.  */

            if (drive[i].idling_method != DRIVE_IDLE_SKIP_CYCLES)
                my_led_status = drive[i].led_status;

            if (my_led_status != drive[i].old_led_status) {
                ui_display_drive_led(i, my_led_status);
                drive[i].old_led_status = my_led_status;
            }

            if (drive[i].current_half_track != drive[i].old_half_track) {
                drive[i].old_half_track = drive[i].current_half_track;
#ifdef __riscos
                ui_display_drive_track_int(i, drive[i].current_half_track);
#else
                ui_display_drive_track(i, (i < 2 && drive[0].enable
                                       && DRIVE_IS_DUAL(drive[0].type))
                                       ? 0 : 8,
                                       ((float)drive[i].current_half_track
                                       / 2.0));
#endif
            }
        }
    }
}

/* This is called at every vsync.  */
void drive_vsync_hook(void)
{
    drive_update_ui_status();
    if (drive[0].idling_method != DRIVE_IDLE_SKIP_CYCLES && drive[0].enable)
        drive0_cpu_execute(clk);
    if (drive[1].idling_method != DRIVE_IDLE_SKIP_CYCLES && drive[1].enable)
        drive1_cpu_execute(clk);
    wd1770_vsync_hook();
}

/* ------------------------------------------------------------------------- */

/*
int reload_rom_1541(char *name) {
    char romsetnamebuffer[MAXPATHLEN];
    char *tmppath;

    if(dos_rom_name_1541)
        free(dos_rom_name_1541);
    if(name == NULL) {
        dos_rom_name_1541 = default_dos_rom_name_1541;
        drive_load_rom_images();
        return(1);
    }
    strcpy(romsetnamebuffer,"dos1541-");
    strncat(romsetnamebuffer,name,MAXPATHLEN - strlen(romsetnamebuffer) - 1);
    if ( sysfile_locate(romsetnamebuffer, &tmppath) ) {
        dos_rom_name_1541 = default_dos_rom_name_1541;
    } else {
        dos_rom_name_1541 = stralloc(romsetnamebuffer);
    }

    drive_load_rom_images();
    return(1);
}
*/

void drive0_parallel_set_atn(int state)
{
    drive0_via_set_atn(state);
    drive0_riot_set_atn(state);
}

void drive1_parallel_set_atn(int state)
{
    drive1_via_set_atn(state);
    drive1_riot_set_atn(state);
}

int drive_num_leds(unsigned int dnr)
{
    if (DRIVE_IS_OLDTYPE(drive[dnr].type)) {
        return 2;
    }

    if ((dnr == 1) && DRIVE_IS_DUAL(drive[0].type)) {
        return 2;
    }

    return 1;
}


static void drive_setup_context_for_drive(drive_context_t *drv, int number)
{
    drv->mynumber = number;
    drv->clk_ptr = &drive_clk[number];
    drv->drive_ptr = &drive[number];

    /* setup shared function pointers */
    if (number == 0) {
        drv->func.iec_write = iec_drive0_write;
        drv->func.iec_read = iec_drive0_read;
        drv->func.parallel_cable_write = parallel_cable_drive0_write;
        drv->func.parallel_set_bus = parallel_drv0_set_bus;
        drv->func.parallel_set_eoi = parallel_drv0_set_eoi;
        drv->func.parallel_set_dav = parallel_drv0_set_dav;
        drv->func.parallel_set_ndac = parallel_drv0_set_ndac;
        drv->func.parallel_set_nrfd = parallel_drv0_set_nrfd;
    } else {
        drv->func.iec_write = iec_drive1_write;
        drv->func.iec_read = iec_drive1_read;
        drv->func.parallel_cable_write = parallel_cable_drive1_write;
        drv->func.parallel_set_bus = parallel_drv1_set_bus;
        drv->func.parallel_set_eoi = parallel_drv1_set_eoi;
        drv->func.parallel_set_dav = parallel_drv1_set_dav;
        drv->func.parallel_set_ndac = parallel_drv1_set_ndac;
        drv->func.parallel_set_nrfd = parallel_drv1_set_nrfd;
    }

    drive_cpu_setup_context(drv);
    drive_via1_setup_context(drv);
    drive_via2_setup_context(drv);
    cia1571_setup_context(drv);
    cia1581_setup_context(drv);
    riot1_setup_context(drv);
    riot2_setup_context(drv);
}

void drive_setup_context(void)
{
    drive_setup_context_for_drive(&drive0_context, 0);
    drive_setup_context_for_drive(&drive1_context, 1);
}

