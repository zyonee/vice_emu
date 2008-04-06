/*
 * machine.h  - Interface to machine-specific implementations.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _MACHINE_H
#define _MACHINE_H

/* The following stuff must be defined once per every emulated CBM machine.  */

/* Name of the machine.  */
extern const char machine_name[];

/* A little handier way to identify the machine: */
#define VICE_MACHINE_NONE      0
#define VICE_MACHINE_C64       1
#define VICE_MACHINE_C128      2
#define VICE_MACHINE_VIC20     3
#define VICE_MACHINE_PET       4
#define VICE_MACHINE_CBM2      5
#define VICE_MACHINE_PLUS4     6

extern int machine_class;
extern int vsid_mode;
extern
#ifdef __OS2__
    const
#endif
int console_mode;

/* Initialize the machine's resources.  */
extern int machine_init_resources(void);

/* Initialize the machine's command-line options.  */
extern int machine_init_cmdline_options(void);

/* Initialize the machine.  */
extern int machine_init(void);

/* Reset the machine.  */
extern void machine_reset(void);
extern void machine_specific_reset(void);

/* Power-up the machine.  */
extern void machine_powerup(void);

/* Shutdown the emachine.  */
extern void machine_shutdown(void);

/* Set the state of the RESTORE key (!=0 means pressed); returns 1 if key had
   been used.  */
extern int machine_set_restore_key(int v);

/* Get the number of CPU cylces per second.  This is used in various parts.  */
extern long machine_get_cycles_per_second(void);

/* Set the screen refresh rate, as this is variable in the CRTC.  */
extern void machine_set_cycles_per_frame(long cpf);

/* Write a snapshot.  */
extern int machine_write_snapshot(const char *name, int save_roms,
                                  int save_disks);

/* Read a snapshot.  */
extern int machine_read_snapshot(const char *name);

/* handle pending interrupts - needed by libsid.a.  */
extern void machine_handle_pending_alarms(int num_write_cycles);

/* Autodetect PSID file.  */
extern int machine_autodetect_psid(const char *name);
extern void machine_play_psid(int tune);

/* Check the base address for the second sid chip.  */
extern int machine_sid2_check_range(unsigned int sid2_adr);

/* Change the timing parameters of the maching (for example PAL/NTSC).  */
extern void machine_change_timing(int timeval);

/* Get screenshot data.  */
struct screenshot_s;
struct video_canvas_s;
struct canvas_refresh_s;
extern int machine_screenshot(struct screenshot_s *screenshot, unsigned int wn);
extern int machine_canvas_screenshot(struct screenshot_s *screenshot,
                                     struct video_canvas_s *canvas);
extern int machine_canvas_async_refresh(struct canvas_refresh_s *ref,
                                        struct video_canvas_s *canvas);

#define JAM_NONE       0
#define JAM_RESET      1
#define JAM_HARD_RESET 2
#define JAM_MONITOR    3
unsigned int machine_jam(const char *format, ...);

extern void machine_video_refresh(void);

/* Update memory pointers if memory mapping has changed. */
extern void machine_update_memory_ptrs(void);

#endif /* _MACHINE_H */

