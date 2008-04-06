/*
 * c128.c
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *
 * Based on the original work in VICE 0.11.0 by
 *  Jouko Valta <jopi@stekt.oulu.fi>
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

#include "attach.h"
#include "autostart.h"
#include "c128.h"
#include "c128mem.h"
#include "c128mmu.h"
#include "c128ui.h"
#include "c64cia.h"
#include "c64rsuser.h"
#include "c64tpi.h"
#include "ciatimer.h"
#include "clkguard.h"
#include "console.h"
#include "datasette.h"
#include "drive-cmdline-options.h"
#include "drive-resources.h"
#include "drive-snapshot.h"
#include "drive.h"
#include "drivecpu.h"
#include "iecdrive.h"
#include "interrupt.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mon.h"
#include "patchrom.h"
#include "reu.h"
#include "screenshot.h"
#include "serial.h"
#include "sid.h"
#include "snapshot.h"
#include "sound.h"
#include "tape.h"
#include "traps.h"
#include "types.h"
#include "utils.h"
#include "vicii.h"
#include "video.h"
#include "vdc.h"
#include "vsync.h"
#include "z80.h"
#include "z80mem.h"

#ifdef HAVE_RS232
#include "rs232.h"
#include "c64acia.h"
#include "rsuser.h"
#endif

#ifdef HAVE_PRINTER
#include "print.h"
#include "prdevice.h"
#include "pruser.h"
#endif

#ifdef HAVE_MOUSE
#include "mouse.h"
#endif

static void vsync_hook(void);

/* ------------------------------------------------------------------------- */

const char machine_name[] = "C128";

int machine_class = VICE_MACHINE_C128;

static trap_t c128_serial_traps[] = {
    {
	"SerialListen",
	0xE355,
	0xE5BA,
	{0x20, 0x73, 0xE5},
	serialattention
    },
    {
	"SerialSaListen",
	0xE37C,
	0xE5BA,
	{0x20, 0x73, 0xE5},
	serialattention
    },
    {
	"SerialSendByte",
	0xE38C,
	0xE5BA,
	{0x20, 0x73, 0xE5},
	serialsendbyte
    },
    {
	"SerialReceiveByte",
	0xE43E,
	0xE5BA,
	{0x20, 0x73, 0xE5},
	serialreceivebyte
    },
    {
	"Serial ready",
	0xE569,
	0xE572,
	{0xAD, 0x00, 0xDD},
	trap_serial_ready
    },
    {
	"Serial ready",
	0xE4F5,
	0xE572,
	{0xAD, 0x00, 0xDD},
	trap_serial_ready
    },

    { NULL }
};

/* Tape traps.  */
static trap_t c128_tape_traps[] = {
    {
        "TapeFindHeader",
        0xE8D3,
        0xE8D6,
        {0x20, 0xF2, 0xE9},
        tape_find_header_trap
    },
    {
        "TapeReceive",
        0xEA60,
        0xEE57,
        {0x20, 0x9B, 0xEE},
        tape_receive_trap
    },
    {
        NULL,
        0,
        0,
        {0, 0, 0},
        NULL
    }
};

static log_t c128_log = LOG_ERR;

static long cycles_per_sec = C128_PAL_CYCLES_PER_SEC;
static long cycles_per_rfsh = C128_PAL_CYCLES_PER_RFSH;
static double rfsh_per_sec = C128_PAL_RFSH_PER_SEC;

/* ------------------------------------------------------------------------ */

/* C128-specific resource initialization.  This is called before initializing
   the machine itself with `machine_init()'.  */
int machine_init_resources(void)
{
    if (traps_init_resources() < 0
        || vsync_init_resources() < 0
        || video_init_resources() < 0
        || c128_mem_init_resources() < 0
        || vic_ii_init_resources() < 0
        || vdc_init_resources() < 0
        || sound_init_resources() < 0
        || sid_init_resources() < 0
#ifdef HAVE_RS232
        || acia1_init_resources() < 0
        || rs232_init_resources() < 0
        || rsuser_init_resources() < 0
#endif
#ifdef HAVE_PRINTER
        || print_init_resources() < 0
        || prdevice_init_resources() < 0
        || pruser_init_resources() < 0
#endif
        /* FIXME: This is already done in main.c
         #ifdef HAVE_MOUSE
         || mouse_init_resources() < 0
         #endif
         */
        || kbd_init_resources() < 0
        || drive_init_resources() < 0
        || datasette_init_resources() < 0
        || mmu_init_resources() < 0
        || z80mem_init_resources() < 0)
        return -1;

    return 0;
}

/* C128-specific command-line option initialization.  */
int machine_init_cmdline_options(void)
{
    if (traps_init_cmdline_options() < 0
        || vsync_init_cmdline_options() < 0
        || video_init_cmdline_options() < 0
        || c128_mem_init_cmdline_options() < 0
        || vic_ii_init_cmdline_options() < 0
        || vdc_init_cmdline_options() < 0
        || sound_init_cmdline_options() < 0
        || sid_init_cmdline_options() < 0
#ifdef HAVE_RS232
        || acia1_init_cmdline_options() < 0
        || rs232_init_cmdline_options() < 0
        || rsuser_init_cmdline_options() < 0
#endif
#ifdef HAVE_PRINTER
        || print_init_cmdline_options() < 0
        || prdevice_init_cmdline_options() < 0
        || pruser_init_cmdline_options() < 0
#endif
        /* FIXME: This is done already in main.c
         #ifdef HAVE_MOUSE
         || mouse_init_cmdline_options() < 0
         #endif
         */
        || kbd_init_cmdline_options() < 0
        || drive_init_cmdline_options() < 0
        || datasette_init_cmdline_options() < 0
        || mmu_init_cmdline_options() < 0
        || z80mem_init_cmdline_options() < 0)
        return -1;

    return 0;
}

/* C128-specific initialization.  */
int machine_init(void)
{
    if (c128_log == LOG_ERR)
        c128_log = log_open("C128");

    maincpu_init();

    if (mem_load() < 0)
	return -1;

    if (z80mem_load() < 0)
        return -1;

    /* Setup trap handling.  */
    traps_init();

    /* Initialize serial traps.  */
    serial_init(c128_serial_traps);

    /* Initialize drives. */
    file_system_init();

#ifdef HAVE_RS232
    /* initialize RS232 handler */
    rs232_init();
    c64_rsuser_init();
#endif

#ifdef HAVE_PRINTER
    /* initialize print devices */
    print_init();
#endif

    /* Initialize the tape emulation.  */
    tape_init(0xb2, 0x90, 0x93, 0xa09, 0, 0xc1, 0xae, 0x34a, 0xd0,
              c128_tape_traps);

    /* Initialize the datasette emulation.  */
    datasette_init();

    /* Fire up the hardware-level drive emulation.  */
    drive_init(C128_PAL_CYCLES_PER_SEC, C128_NTSC_CYCLES_PER_SEC);

    /* Initialize autostart. FIXME: at least 0xa26 is only for 40 cols */
    autostart_init((CLOCK)
                   (3 * rfsh_per_sec * cycles_per_rfsh),
                   1, 0xa27, 0xe0, 0xec, 0xee);

    /* Initialize the VDC emulation.  */
    if (vdc_init() == NULL)
        return -1;

    /* Initialize the VIC-II emulation.  */
    if (vic_ii_init() == NULL)
        return -1;
    vic_ii_enable_extended_keyboard_rows(1);

    cia1_enable_extended_keyboard_rows(1);

    ciat_init_table();
    cia1_init();
    cia2_init();

    tpi_init();

#ifdef HAVE_RS232
    acia1_init();
#endif

    /* Initialize the keyboard.  */
    if (c128_kbd_init() < 0)
        return -1;

    /* Initialize the monitor.  */
    monitor_init(&maincpu_monitor_interface, drive0_monitor_interface_ptr,
                 drive1_monitor_interface_ptr);

    /* Initialize vsync and register our hook function.  */
    vsync_set_machine_parameter(rfsh_per_sec, cycles_per_sec);
    vsync_init(vsync_hook);

    /* Initialize sound.  Notice that this does not really open the audio
       device yet.  */
    sound_init(cycles_per_sec, cycles_per_rfsh);

    /* Initialize keyboard buffer.  */
    kbd_buf_init(842, 208, 10,
                 (CLOCK)(rfsh_per_sec * cycles_per_rfsh));

    /* Initialize the C128-specific part of the UI.  */
    c128_ui_init();

    /* Initialize the REU.  */
    reu_init();

#ifdef HAVE_MOUSE
    /* Initialize mouse support (if present).  */
    mouse_init();
#endif

    iec_init();

    mmu_init();

    return 0;
}

/* C128-specific reset sequence.  */
void machine_reset(void)
{
    serial_reset();

    cia1_reset();
    cia2_reset();
    sid_reset();
    tpi_reset();

#ifdef HAVE_RS232
    acia1_reset();

    rs232_reset();
    rsuser_reset();
#endif

#ifdef HAVE_PRINTER
    print_reset();
#endif

    vdc_reset();

    /* The VIC-II must be the *last* to be reset.  */
    vic_ii_reset();

    autostart_reset();
    drive_reset();
    datasette_reset();

    z80mem_initialize();
    z80_reset();
}

void machine_powerup(void)
{
    mem_powerup();
    /*vic_ii_reset_registers();*/
    maincpu_trigger_reset();
}

void machine_shutdown(void)
{
    /* Detach all disks.  */
    file_system_detach_disk_shutdown();

    /* and the tape */
    tape_detach_image();

    console_close_all();
}

void machine_handle_pending_alarms(int num_write_cycles)
{
     vic_ii_handle_pending_alarms_external(num_write_cycles);
}

/* ------------------------------------------------------------------------- */

/* This hook is called at the end of every frame.  */
static void vsync_hook(void)
{
    CLOCK sub;

    drive_vsync_hook();

    autostart_advance();

    sub = clk_guard_prevent_overflow(&maincpu_clk_guard);

    /* The drive has to deal both with our overflowing and its own one, so
       it is called even when there is no overflowing in the main CPU.  */
    /* FIXME: Do we have to check drive_enabled here?  */
    drive_prevent_clk_overflow(sub, 0);
    drive_prevent_clk_overflow(sub, 1);
}

int machine_set_restore_key(int v)
{
    maincpu_set_nmi(I_RESTORE, v ? 1 : 0);
    return 1;
}

/* ------------------------------------------------------------------------- */

long machine_get_cycles_per_second(void)
{
    return cycles_per_sec;
}

void machine_change_timing(int timeval)
{
    maincpu_trigger_reset();

    switch (timeval) {
      case DRIVE_SYNC_PAL:
        cycles_per_sec = C128_PAL_CYCLES_PER_SEC;
        cycles_per_rfsh = C128_PAL_CYCLES_PER_RFSH;
        rfsh_per_sec = C128_PAL_RFSH_PER_SEC;
        break;
      case DRIVE_SYNC_NTSC:
        cycles_per_sec = C128_NTSC_CYCLES_PER_SEC;
        cycles_per_rfsh = C128_NTSC_CYCLES_PER_RFSH;
        rfsh_per_sec = C128_NTSC_RFSH_PER_SEC;
        break;
      case DRIVE_SYNC_NTSCOLD:
        cycles_per_sec = C128_NTSCOLD_CYCLES_PER_SEC;
        cycles_per_rfsh = C128_NTSCOLD_CYCLES_PER_RFSH;
        rfsh_per_sec = C128_NTSC_OLDRFSH_PER_SEC;
        break;
      default:
        log_error(c128_log, "Unknown machine timing.");
    }

    vsync_set_machine_parameter(rfsh_per_sec, cycles_per_sec);
    sound_set_machine_parameter(cycles_per_sec, cycles_per_rfsh);
}

/* ------------------------------------------------------------------------- */

#define SNAP_MACHINE_NAME   "C128"
#define SNAP_MAJOR          0
#define SNAP_MINOR          0

int machine_write_snapshot(const char *name, int save_roms, int save_disks)
{
    snapshot_t *s;

    s = snapshot_create(name, ((BYTE)(SNAP_MAJOR)), ((BYTE)(SNAP_MINOR)),
                        SNAP_MACHINE_NAME);
    if (s == NULL)
        return -1;

    if (maincpu_write_snapshot_module(s) < 0
        || mem_write_snapshot_module(s, save_roms) < 0
        || cia1_write_snapshot_module(s) < 0
        || cia2_write_snapshot_module(s) < 0
        || sid_write_snapshot_module(s) < 0
        || drive_write_snapshot_module(s, save_disks, save_roms) < 0
        || vic_ii_write_snapshot_module(s) < 0) {
        snapshot_close(s);
        remove_file(name);
        return -1;
    }

    snapshot_close(s);
    return 0;
}

int machine_read_snapshot(const char *name)
{
    snapshot_t *s;
    BYTE minor, major;

    s = snapshot_open(name, &major, &minor, SNAP_MACHINE_NAME);
    if (s == NULL)
        return -1;

    if (major != SNAP_MAJOR || minor != SNAP_MINOR) {
        log_message(c128_log,
                    "Snapshot version (%d.%d) not valid: expecting %d.%d.",
                    major, minor, SNAP_MAJOR, SNAP_MINOR);
        goto fail;
    }

    vic_ii_prepare_for_snapshot();

    if (maincpu_read_snapshot_module(s) < 0
        || mem_read_snapshot_module(s) < 0
        || cia1_read_snapshot_module(s) < 0
        || cia2_read_snapshot_module(s) < 0
        || sid_read_snapshot_module(s) < 0
        || drive_read_snapshot_module(s) < 0
        || vic_ii_read_snapshot_module(s) < 0)
       goto fail;

    snapshot_close(s);

    return 0;

fail:
    if (s != NULL)
        snapshot_close(s);
    maincpu_trigger_reset();
    return -1;
}


/* ------------------------------------------------------------------------- */
int machine_autodetect_psid(const char *name)
{
  return -1;
}

void machine_play_psid(int tune)
{
}

int machine_screenshot(screenshot_t *screenshot, unsigned int wn)
{
  if (wn == 0)
      return vdc_screenshot(screenshot);
  if (wn == 1)
      return vic_ii_screenshot(screenshot);
  return -1;
}

int machine_canvas_screenshot(screenshot_t *screenshot, canvas_t *canvas)
{
  if (canvas == vic_ii_get_canvas())
      return vic_ii_screenshot(screenshot);
  if (canvas == vdc_get_canvas())
      return vdc_screenshot(screenshot);
  return -1;
}
