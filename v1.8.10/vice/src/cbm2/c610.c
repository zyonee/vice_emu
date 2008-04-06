/*
 * c610.c
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#include "asm.h"
#include "attach.h"
#include "autostart.h"
#include "c610-cmdline-options.h"
#include "c610-resources.h"
#include "c610-snapshot.h"
#include "c610.h"
#include "c610acia.h"
#include "c610cia.h"
#include "c610mem.h"
#include "c610tpi.h"
#include "c610ui.h"
#include "ciatimer.h"
#include "clkguard.h"
#include "cmdline.h"
#include "console.h"
#include "crtc.h"
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
#include "mem.h"
#include "mon.h"
#include "printer.h"
#include "resources.h"
#include "screenshot.h"
#include "serial.h"
#include "sid-cmdline-options.h"
#include "sid-resources.h"
#include "sid-snapshot.h"
#include "sid.h"
#include "sound.h"
#include "snapshot.h"
#include "tape.h"
#include "traps.h"
#include "types.h"
#include "utils.h"
#include "via.h"
#include "vicii.h"
#include "video.h"
#include "vsync.h"

#ifdef HAVE_RS232
#include "rs232.h"
#endif

#define C500_POWERLINE_CYCLES_PER_IRQ   (C500_PAL_CYCLES_PER_RFSH)

static void machine_vsync_hook(void);

static long cbm2_cycles_per_sec = C610_PAL_CYCLES_PER_SEC;
static double cbm2_rfsh_per_sec = C610_PAL_RFSH_PER_SEC;
static long cbm2_cycles_per_rfsh = C610_PAL_CYCLES_PER_RFSH;

const char machine_name[] = "CBM-II";

int machine_class = VICE_MACHINE_CBM2;

static log_t c610_log = LOG_ERR;

extern BYTE rom[];

int isC500 = 0;

/* ------------------------------------------------------------------------- */

static int c500_snapshot_write_module(snapshot_t *p);
static int c500_snapshot_read_module(snapshot_t *p);

/* ------------------------------------------------------------------------- */

int cbm2_is_c500 (void)
{
    return isC500;
}

/* ------------------------------------------------------------------------- */

/* CBM-II-specific resource initialization.  This is called before initializing
   the machine itself with `machine_init()'.  */
int machine_init_resources(void)
{
    if (traps_resources_init() < 0
        || vsync_resources_init() < 0
        || video_resources_init(VIDEO_RESOURCES_PAL) < 0
        || c610_resources_init() < 0
        || crtc_resources_init() < 0
        || vic_ii_resources_init() < 0
        || sound_resources_init() < 0
        || sid_resources_init() < 0
        || drive_resources_init() < 0
        || datasette_resources_init() < 0
        || acia1_resources_init() < 0
#ifdef HAVE_RS232
        || rs232_resources_init() < 0
#endif
        || printer_resources_init() < 0
        || pet_kbd_resources_init() < 0
        )
        return -1;
    return 0;
}

/* CBM-II-specific command-line option initialization.  */
int machine_init_cmdline_options(void)
{
    if (traps_cmdline_options_init() < 0
        || vsync_cmdline_options_init() < 0
        || video_init_cmdline_options() < 0
        || c610_cmdline_options_init() < 0
        || crtc_cmdline_options_init() < 0
        || vic_ii_cmdline_options_init() < 0
        || sound_cmdline_options_init() < 0
        || sid_cmdline_options_init() < 0
        || drive_cmdline_options_init() < 0
        || datasette_cmdline_options_init() < 0
        || acia1_cmdline_options_init() < 0
#ifdef HAVE_RS232
        || rs232_cmdline_options_init() < 0
#endif
        || printer_cmdline_options_init() < 0
        || pet_kbd_cmdline_options_init() < 0
        )
        return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
/* provide the 50(?)Hz IRQ signal for the standard IRQ */

#define SIGNAL_VERT_BLANK_OFF   tpi1_set_int(0, 1);

#define SIGNAL_VERT_BLANK_ON    tpi1_set_int(0, 0);

/* ------------------------------------------------------------------------- */
/* for the C500 there is a powerline IRQ... */

static alarm_t c500_powerline_clk_alarm;
static CLOCK c500_powerline_clk = 0;

void c500_powerline_clk_alarm_handler (CLOCK offset) {

    c500_powerline_clk += C500_POWERLINE_CYCLES_PER_IRQ;

    SIGNAL_VERT_BLANK_OFF

    alarm_set (&c500_powerline_clk_alarm, c500_powerline_clk);

    SIGNAL_VERT_BLANK_ON

}

static void c500_powerline_clk_overflow_callback(CLOCK sub, void *data)
{
    c500_powerline_clk -= sub;
}

/* ------------------------------------------------------------------------- */
/* ... while the other CBM-II use the CRTC retrace signal. */

static void cbm2_crtc_signal(unsigned int signal) {
    if (signal) {
        SIGNAL_VERT_BLANK_ON
    } else {
        SIGNAL_VERT_BLANK_OFF
    }
}

/* ------------------------------------------------------------------------- */

void c610_monitor_init(void)
{
    monitor_cpu_type_t asm6502;
    monitor_cpu_type_t *asmarray[2];

    asm6502_init(&asm6502);

    asmarray[0] = &asm6502;
    asmarray[1] = NULL;

    /* Initialize the monitor.  */
    monitor_init(&maincpu_monitor_interface, drive0_get_monitor_interface_ptr(),
                 drive1_get_monitor_interface_ptr(), asmarray);
}

/* CBM-II-specific initialization.  */
int machine_init(void)
{
    c610_log = log_open("CBM2");

    cbm2_init_ok = 1;

    maincpu_init();

    /* Setup trap handling - must be before mem_load() */
    traps_init();

    if (mem_load() < 0)
        return -1;

    /* No traps installed on the CBM-II.  */
    if (serial_init(NULL, 0xa4) < 0)
        return -1;

    /* Initialize drives. */
    file_system_init();

#ifdef HAVE_RS232
    rs232_init();
#endif

    /* initialize print devices */
    printer_init();

    if (! isC500) {
        /* Initialize the CRTC emulation.  */
        if (crtc_init() == NULL)
            return -1;
        crtc_set_retrace_callback(cbm2_crtc_signal);
        crtc_set_retrace_type(0);
        crtc_set_hw_options( 1, 0x7ff, 0x1000, 512, -0x2000);
    } else {
        /* Initialize the VIC-II emulation.  */
        if (vic_ii_init() == NULL)
            return -1;

        /*
        c500_set_phi1_bank(15);
        c500_set_phi2_bank(15);
        */

        alarm_init(&c500_powerline_clk_alarm, maincpu_alarm_context,
                   "C500PowerlineClk", c500_powerline_clk_alarm_handler);
        clk_guard_add_callback(&maincpu_clk_guard,
                                c500_powerline_clk_overflow_callback, NULL);

        cbm2_cycles_per_sec = C500_PAL_CYCLES_PER_SEC;
        cbm2_rfsh_per_sec = C500_PAL_RFSH_PER_SEC;
        cbm2_cycles_per_rfsh = C500_PAL_CYCLES_PER_RFSH;
    }

    ciat_init_table();
    cia1_init();
    acia1_init();
    tpi1_init();
    tpi2_init();

    /* Initialize the keyboard.  */
    if (c610_kbd_init() < 0)
        return -1;

    /* Initialize the datasette emulation.  */
    datasette_init();

    /* Fire up the hardware-level 1541 emulation.  */
    drive_init(cbm2_cycles_per_sec, C610_NTSC_CYCLES_PER_SEC);

    c610_monitor_init();

    /* Initialize vsync and register our hook function.  */
    vsync_init(machine_vsync_hook);
    vsync_set_machine_parameter(cbm2_rfsh_per_sec, cbm2_cycles_per_sec);

    /* Initialize sound.  Notice that this does not really open the audio
       device yet.  */
    sound_init(cbm2_cycles_per_sec, cbm2_cycles_per_rfsh);

    /* Initialize the CBM-II-specific part of the UI.  */
    c610_ui_init();

    iec_init();


    return 0;
}

/* CBM-II-specific initialization.  */
void machine_specific_reset(void)
{
    serial_reset();

    acia1_reset();
    cia1_reset();
    tpi1_reset();
    tpi2_reset();

    sid_reset();

    if (!isC500) {
        crtc_reset();
    } else {
        c500_powerline_clk = clk + C500_POWERLINE_CYCLES_PER_IRQ;
        alarm_set (&c500_powerline_clk_alarm, c500_powerline_clk);
        vic_ii_reset();
    }
    printer_reset();

#ifdef HAVE_RS232
    rs232_reset();
#endif

    autostart_reset();
    drive_reset();
    datasette_reset();

    mem_reset();
}

void machine_powerup(void)
{
    mem_powerup();
    maincpu_trigger_reset();
}

void machine_shutdown(void)
{
    /* Detach all disks.  */
    file_system_detach_disk_shutdown();

    /* and the tape */
    tape_detach_image();

    console_close_all();

    /* close the video chip(s) */
    if (isC500) {
        vic_ii_free();
    } else {
        crtc_free();
    }
}

void machine_handle_pending_alarms(int num_write_cycles)
{
}

/* ------------------------------------------------------------------------- */

/* This hook is called at the end of every frame.  */
static void machine_vsync_hook(void)
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

/* Dummy - no restore key.  */
int machine_set_restore_key(int v)
{
    return 0;   /* key not used -> lookup in keymap */
}

/* ------------------------------------------------------------------------- */

long machine_get_cycles_per_second(void)
{
    return cbm2_cycles_per_sec;
}

void machine_change_timing(int timeval)
{

}

/* Set the screen refresh rate, as this is variable in the CRTC */
void machine_set_cycles_per_frame(long cpf) {

    cbm2_cycles_per_rfsh = cpf;
    cbm2_rfsh_per_sec = ((double) cbm2_cycles_per_sec) / ((double) cpf);

    vsync_set_machine_parameter(cbm2_rfsh_per_sec, cbm2_cycles_per_sec);
}

/* ------------------------------------------------------------------------- */

/* #define SNAP_MACHINE_NAME   "C610" */
#define SNAP_MAJOR          0
#define SNAP_MINOR          0

int machine_write_snapshot(const char *name, int save_roms, int save_disks)
{
    snapshot_t *s;

    s = snapshot_create(name, SNAP_MAJOR, SNAP_MINOR, machine_name);
    if (s == NULL) {
        perror(name);
        return -1;
    }
    if (maincpu_snapshot_write_module(s) < 0
        || c610_snapshot_write_module(s, save_roms) < 0
        || ((!isC500) && crtc_snapshot_write_module(s) < 0)
        || cia1_snapshot_write_module(s) < 0
        || tpi1_snapshot_write_module(s) < 0
        || tpi2_snapshot_write_module(s) < 0
        || acia1_snapshot_write_module(s) < 0
        || sid_snapshot_write_module(s) < 0
        || drive_snapshot_write_module(s, save_disks, save_roms) < 0
        || (isC500 && vic_ii_snapshot_write_module(s) < 0)
        || (isC500 && c500_snapshot_write_module(s) < 0)
        ) {
        snapshot_close(s);
        util_file_remove(name);
        return -1;
    }

    snapshot_close(s);
    return 0;
}

int machine_read_snapshot(const char *name)
{
    snapshot_t *s;
    BYTE minor, major;

    s = snapshot_open(name, &major, &minor, machine_name);
    if (s == NULL) {
        return -1;
    }

    if (major != SNAP_MAJOR || minor != SNAP_MINOR) {
        log_error(c610_log,
                  "Snapshot version (%d.%d) not valid: expecting %d.%d.",
                  major, minor, SNAP_MAJOR, SNAP_MINOR);
        goto fail;
    }

    if (isC500) {
        vic_ii_prepare_for_snapshot();
    }

    if (maincpu_snapshot_read_module(s) < 0
        || ((!isC500) && crtc_snapshot_read_module(s) < 0)
        || (isC500 && vic_ii_snapshot_read_module(s) < 0)
        || (isC500 && c500_snapshot_read_module(s) < 0)
        || c610_snapshot_read_module(s) < 0
        || cia1_snapshot_read_module(s) < 0
        || tpi1_snapshot_read_module(s) < 0
        || tpi2_snapshot_read_module(s) < 0
        || acia1_snapshot_read_module(s) < 0
        || sid_snapshot_read_module(s) < 0
        || drive_snapshot_read_module(s) < 0
        )
        goto fail;

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
        return crtc_screenshot(screenshot);
    if (wn == 1)
        return vic_ii_screenshot(screenshot);
    return -1;
}

int machine_canvas_screenshot(screenshot_t *screenshot,
                              struct video_canvas_s *canvas)
{
    if (canvas == vic_ii_get_canvas())
        return vic_ii_screenshot(screenshot);
    if (canvas == crtc_get_canvas())
        return crtc_screenshot(screenshot);
    return -1;
}

/*-----------------------------------------------------------------------*/

/*
 * C500 extra data (state of 50Hz clk)
 */
#define C500DATA_DUMP_VER_MAJOR   0
#define C500DATA_DUMP_VER_MINOR   0

/*
 * DWORD        IRQCLK          CPU clock ticks until next 50 Hz IRQ
 *
 */

static const char module_name[] = "C500DATA";

static int c500_snapshot_write_module(snapshot_t *p)
{
    snapshot_module_t *m;

    m = snapshot_module_create(p, module_name, C500DATA_DUMP_VER_MAJOR,
                               C500DATA_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    snapshot_module_write_dword(m, c500_powerline_clk - clk);

    snapshot_module_close(m);

    return 0;
}

static int c500_snapshot_read_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    snapshot_module_t *m;
    DWORD dword;

    m = snapshot_module_open(p, module_name, &vmajor, &vminor);
    if (m == NULL)
        return -1;

    if (vmajor != C500DATA_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_read_dword(m, &dword);
    c500_powerline_clk = clk + dword;
    alarm_set(&c500_powerline_clk_alarm, c500_powerline_clk);

    snapshot_module_close(m);

    return 0;
}

void machine_video_refresh(void)
{
    if (isC500) {
        vic_ii_video_refresh();
    } else {
        crtc_video_refresh();
    }
}

int machine_sid2_check_range(unsigned int sid2_adr)
{
    if (sid2_adr >= 0xda20 && sid2_adr <= 0xdae0)
        return 0;

    return -1;
}

