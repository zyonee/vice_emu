/*
 * drive.c - Hardware-level disk drive emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * Based on old code by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  André Fachat <fachat@physik.tu-chemnitz.de>
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
#include <assert.h>

#include "attach.h"
#include "diskconstants.h"
#include "diskimage.h"
#include "drive-check.h"
#include "drive-overflow.h"
#include "drive.h"
#include "drivecpu.h"
#include "fdccpu.h"
#include "driveimage.h"
#include "drivesync.h"
#include "driverom.h"
#include "drivetypes.h"
#include "iecbus.h"
#include "iecdrive.h"
#include "lib.h"
#include "log.h"
#include "machine-drive.h"
#include "machine.h"
#include "maincpu.h"
#include "resources.h"
#include "types.h"
#include "uiapi.h"
#include "ds1216e.h"
#include "fdd.h"


drive_context_t *drive_context[DRIVE_NUM];

/* Generic drive logging goes here.  */
static log_t drive_log = LOG_ERR;

/* If nonzero, at least one vaild drive ROM has already been loaded.  */
int rom_loaded = 0;

/* ------------------------------------------------------------------------- */

static int drive_led_color[DRIVE_NUM];

/* ------------------------------------------------------------------------- */

void drive_set_disk_memory(BYTE *id, unsigned int track, unsigned int sector,
                           struct drive_context_s *drv)
{
    drive_t *drive;

    drive = drv->drive;

    if (drive->type == DRIVE_TYPE_1541
        || drive->type == DRIVE_TYPE_1541II
        || drive->type == DRIVE_TYPE_1570
        || drive->type == DRIVE_TYPE_1571
        || drive->type == DRIVE_TYPE_1571CR) {
        drv->cpud->drive_ram[0x12] = id[0];
        drv->cpud->drive_ram[0x13] = id[1];
        drv->cpud->drive_ram[0x16] = id[0];
        drv->cpud->drive_ram[0x17] = id[1];
        drv->cpud->drive_ram[0x18] = track;
        drv->cpud->drive_ram[0x19] = sector;
        drv->cpud->drive_ram[0x22] = track;
    }
}

void drive_set_last_read(unsigned int track, unsigned int sector, BYTE *buffer,
                         struct drive_context_s *drv)
{
    drive_t *drive;

    drive = drv->drive;

    if (drive->type == DRIVE_TYPE_1541
        || drive->type == DRIVE_TYPE_1541II
        || drive->type == DRIVE_TYPE_1570
        || drive->type == DRIVE_TYPE_1571
        || drive->type == DRIVE_TYPE_1571CR) {
        memcpy(&(drv->cpud->drive_ram[0x0400]), buffer, 256);
    }
}

/* ------------------------------------------------------------------------- */

/* Global clock counters.  */
CLOCK drive_clk[DRIVE_NUM];
CLOCK drive_fdcclk[DRIVE_NUM];

/* Initialize the hardware-level drive emulation (should be called at least
   once before anything else).  Return 0 on success, -1 on error.  */
int drive_init(void)
{
    unsigned int dnr;
    drive_t *drive;

    if (rom_loaded)
        return 0;

    driverom_init();
    drive_image_init();

    drive_log = log_open("Drive");

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        char *logname;

        drive = drive_context[dnr]->drive;
        logname = lib_msprintf("Drive %i", dnr + 8);
        drive->log = log_open(logname);
        lib_free(logname);

        drive_clk[dnr] = 0L;
        drive->clk = &drive_clk[dnr];
        drive_fdcclk[dnr] = 0L;
        drive->fdcclk = &drive_fdcclk[dnr];
        drive->mynumber = dnr;
    }

    if (driverom_load_images() < 0) {
        resources_set_int("Drive8Type", DRIVE_TYPE_NONE);
        resources_set_int("Drive9Type", DRIVE_TYPE_NONE);
        return -1;
    }

    log_message(drive_log, "Finished loading ROM images.");
    rom_loaded = 1;

    drive_overflow_init();

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;
        drive->drive_ram_expand2 = NULL;
        drive->drive_ram_expand4 = NULL;
        drive->drive_ram_expand6 = NULL;
        drive->drive_ram_expand8 = NULL;
        drive->drive_ram_expanda = NULL;

        machine_drive_port_default(drive_context[dnr]);

        if (drive_check_type(drive->type, dnr) < 1)
            resources_set_int_sprintf("Drive%iType", DRIVE_TYPE_NONE, dnr + 8);

        machine_drive_rom_setup_image(dnr);

        drive->rtc_offset = (time_t)0; /* TODO: offset */
        drive->ds1216 = ds1216e_init(&drive->rtc_offset);
        drive->ds1216->hours12 = 1;
    }

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;
        drive->fdds[0] = fdd_init(1, drive);
        drive->old_led_status = 0;
        drive->old_half_track = 0;
        drive->clock_frequency = 1;
        drive->led_last_change_clk = *(drive->clk);
        drive->led_last_uiupdate_clk = *(drive->clk);
        drive->led_active_ticks = 0;

        /* Position the R/W head on the directory track.  */
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
    }

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;
        driverom_initialize_traps(drive, 1);

        drivesync_clock_frequency(drive->type, drive);

        drivecpu_init(drive_context[dnr], drive->type);
        fdccpu_init(drive_context[dnr], drive->type);

        /* Make sure the sync factor is acknowledged correctly.  */
        drivesync_factor(drive_context[dnr]);

        /* Make sure the traps are moved as needed.  */
        if (drive->enable)
            drive_enable(drive_context[dnr]);
    }
    return 0;
}

void drive_shutdown(void)
{
    unsigned int dnr;

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        fdccpu_shutdown(drive_context[dnr]);
        drivecpu_shutdown(drive_context[dnr]);
        fdd_shutdown(drive_context[dnr]->drive->fdds[0]);
        ds1216e_destroy(drive_context[dnr]->drive->ds1216);
    }

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        lib_free(drive_context[dnr]->drive);
        lib_free(drive_context[dnr]);
    }
}

void drive_set_active_led_color(unsigned int type, unsigned int dnr)
{
    switch (type) {
      case DRIVE_TYPE_1541:
      case DRIVE_TYPE_1551:
      case DRIVE_TYPE_1570:
      case DRIVE_TYPE_1571:
      case DRIVE_TYPE_1571CR:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      case DRIVE_TYPE_1541II:
      case DRIVE_TYPE_1581:
      case DRIVE_TYPE_2000:
      case DRIVE_TYPE_4000:
        drive_led_color[dnr] = DRIVE_ACTIVE_GREEN;
        break;
      case DRIVE_TYPE_2031:
      case DRIVE_TYPE_2040:
      case DRIVE_TYPE_3040:
      case DRIVE_TYPE_4040:
      case DRIVE_TYPE_1001:
      case DRIVE_TYPE_8050:
      case DRIVE_TYPE_8250:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
        break;
      default:
        drive_led_color[dnr] = DRIVE_ACTIVE_RED;
    }
}

int drive_set_disk_drive_type(unsigned int type, struct drive_context_s *drv)
{
    unsigned int dnr;

    dnr = drv->mynumber;

    if (machine_drive_rom_check_loaded(type) < 0)
        return -1;

    drivesync_clock_frequency(type, drv->drive);

    drv->drive->type = type;
    machine_drive_rom_setup_image(dnr);
    drivesync_factor(drv);
    drive_set_active_led_color(type, dnr);

    drivecpu_init(drv, type);
    fdccpu_init(drv, type);

    return 0;
}


/* Activate full drive emulation. */
int drive_enable(drive_context_t *drv)
{
    int i, drive_true_emulation = 0;
    unsigned int dnr;
    drive_t *drive;
    unsigned int enabled_drives = 0;

    dnr = drv->mynumber;
    drive = drv->drive;

    /* This must come first, because this might be called before the drive
       initialization.  */
    if (!rom_loaded)
        return -1;

    resources_get_int("DriveTrueEmulation", &drive_true_emulation);

    /* Always disable kernal traps. */
    if (!drive_true_emulation)
        return 0;

    if (drive->type == DRIVE_TYPE_NONE)
        return 0;

    fdccpu_wake_up(drv);
    drivecpu_wake_up(drv);

    /* Make sure the UI is updated.  */
    for (i = 0; i < DRIVE_NUM; i++) {
        unsigned int the_drive;
 
        the_drive = 1 << i;
        if (drive_context[i]->drive->enable) {
            enabled_drives |= the_drive;
            drive_context[i]->drive->old_led_status = -1;
            drive_context[i]->drive->old_half_track = -1;
        }
    }

    /* FIXME: this doesn't care about dual drives anymore */
    drive_set_active_led_color(drive->type, dnr);
    ui_enable_drive_status(enabled_drives,
                           drive_led_color);
    return 0;
}

/* Disable full drive emulation.  */
void drive_disable(drive_context_t *drv)
{
    int i, drive_true_emulation = 0;
    drive_t *drive;
    unsigned int enabled_drives = 0;

    drive = drv->drive;

    /* This must come first, because this might be called before the true
       drive initialization.  */
    drive->enable = 0;

    resources_get_int("DriveTrueEmulation", &drive_true_emulation);

    if (rom_loaded) {
        drivecpu_sleep(drv);
        fdccpu_sleep(drv);
        machine_drive_port_default(drv);

        fdd_flush(drive->fdds[0]);
    }

    /* Make sure the UI is updated.  */
    for (i = 0; i < DRIVE_NUM; i++) {
        unsigned int the_drive;

        the_drive = 1 << i;
        if (drive_context[i]->drive->enable) {
            enabled_drives |= the_drive;
            drive_context[i]->drive->old_led_status = -1;
            drive_context[i]->drive->old_half_track = -1;
        }
    }

    ui_enable_drive_status(enabled_drives,
                           drive_led_color);
}

void drive_reset(void)
{
    unsigned int dnr;
    drive_t *drive;

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive = drive_context[dnr]->drive;

        drivecpu_reset(drive_context[dnr]);
        fdccpu_reset(drive_context[dnr]);

        drive->led_last_change_clk = *(drive->clk);
        drive->led_last_uiupdate_clk = *(drive->clk);
        drive->led_active_ticks = 0;
    }
}

void drive_gcr_data_writeback_all(void)
{
    drive_t *drive;
    unsigned int i;

    for (i = 0; i < DRIVE_NUM; i++) {
        drive = drive_context[i]->drive;
        fdd_flush(drive->fdds[0]);
    }
}

/* ------------------------------------------------------------------------- */

static void drive_led_update(drive_t *drive)
{
    int my_led_status = 0;
    CLOCK led_period;
    unsigned int led_pwm;

    /* Actually update the LED status only if the `trap idle'
       idling method is being used, as the LED status could be
       incorrect otherwise.  */

    if (drive->idling_method != DRIVE_IDLE_SKIP_CYCLES)
        my_led_status = drive->led_status;

    /* Update remaining led clock ticks. */
    if (drive->led_status & 1)
        drive->led_active_ticks += *(drive->clk)
                                   - drive->led_last_change_clk;
    drive->led_last_change_clk = *(drive->clk);

    led_period = *(drive->clk) - drive->led_last_uiupdate_clk;
    drive->led_last_uiupdate_clk = *(drive->clk);

    if (led_period == 0)
        return;

    if (drive->led_active_ticks > led_period) {
    /* during startup it has been observer that led_pwm > 1000, 
       which potentially breaks several UIs */
    /* this also happens when the drive is reset from UI
       and the LED was on */
        led_pwm = 1000;
    } else {
        led_pwm = drive->led_active_ticks * 1000 / led_period;
    }
    assert(led_pwm <= MAX_PWM);
    if (led_pwm > MAX_PWM) {
        led_pwm = MAX_PWM;
    }
    
    drive->led_active_ticks = 0;

    if (led_pwm != drive->led_last_pwm
        || my_led_status != drive->old_led_status) {
        ui_display_drive_led(drive->mynumber, led_pwm,
                             (my_led_status & 2) ? 1000 : 0);
        drive->led_last_pwm = led_pwm;
        drive->old_led_status = my_led_status;
    }
}

/* Update the status bar in the UI.  */
void drive_update_ui_status(void)
{
    int i;

    if (console_mode || (machine_class == VICE_MACHINE_VSID)) {
        return;
    }

    /* Update the LEDs and the track indicators.  */
    for (i = 0; i < DRIVE_NUM; i++) {
        drive_t *drive;

        drive = drive_context[i]->drive;
        if (drive->enable
            || ((i == 1) && drive_context[0]->drive->enable
            && drive_check_dual(drive_context[0]->drive->type))) {
            drive_led_update(drive);

            if (drive->current_half_track != drive->old_half_track) {
                drive->old_half_track = drive->current_half_track;
                ui_display_drive_track(i, (i < 2
                    && drive_context[0]->drive->enable
                    && drive_check_dual(drive_context[0]->drive->type))
                    ? 0 : 8, drive->current_half_track);
            }
        }
    }
}

int drive_num_leds(unsigned int dnr)
{
    if (drive_check_old(drive_context[dnr]->drive->type)) {
        return 2;
    }

    if ((dnr == 1) && drive_check_dual(drive_context[0]->drive->type)) {
        return 2;
    }

    if (drive_context[dnr]->drive->type == DRIVE_TYPE_2000
        || drive_context[dnr]->drive->type == DRIVE_TYPE_4000) {
        return 2;
    }

    return 1;
}

/* This is called at every vsync.  */
void drive_vsync_hook(void)
{
    unsigned int dnr;

    drive_update_ui_status();

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive_t *drive;

        drive = drive_context[dnr]->drive;
        if (drive->idling_method != DRIVE_IDLE_SKIP_CYCLES
            && drive->enable) {
            drivecpu_execute(drive_context[dnr], maincpu_clk);
            if (drive->true_fdc && drive_check_old(drive->type)) {
                fdccpu_execute(drive_context[dnr], *(drive_context[dnr]->clk_ptr));
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

static void drive_setup_context_for_drive(drive_context_t *drv,
                                          unsigned int dnr)
{
    drv->mynumber = dnr;
    drv->drive = lib_calloc(1, sizeof(drive_t));
    drv->clk_ptr = &drive_clk[dnr];
    drv->fdcclk_ptr = &drive_fdcclk[dnr];

    drivecpu_setup_context(drv);
    fdccpu_setup_context(drv);
    machine_drive_setup_context(drv);
}

void drive_setup_context(void)
{
    unsigned int dnr;

    for (dnr = 0; dnr < DRIVE_NUM; dnr++) {
        drive_context[dnr] = lib_calloc(1, sizeof(drive_context_t));
        drive_setup_context_for_drive(drive_context[dnr], dnr);
    }
}
