/*
 * main.c - VICE startup.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
 *  Teemu Rantanen   (tvr@cs.hut.fi)
 *  Vesa-Matti Puro  (vmp@lut.fi)
 *  Jarkko Sonninen  (sonninen@lut.fi)
 *  Jouko Valta      (jopi@stekt.oulu.fi)
 *  Andr� Fachat     (a.fachat@physik.tu-chemnitz.de)
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

#ifdef __hpux
#ifndef _INCLUDE_POSIX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif  /* __hpux */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef __hpux
#define _INCLUDE_XOPEN_SOURCE
#define _XPG2
#include <limits.h>
#undef  _INCLUDE_XOPEN_SOURCE
#undef  _XPG2
#else
#include <limits.h>
#endif

#ifdef __GO32__
#include <fcntl.h>
#endif

#include "machine.h"
#include "maincpu.h"
#include "serial.h"
#include "interrupt.h"
#include "sound.h"
#include "ui.h"
#include "vsync.h"
#include "video.h"
#include "kbd.h"
#include "resources.h"
#include "sysfile.h"
#include "mon.h"
#include "autostart.h"
#include "findpath.h"
#include "utils.h"
#include "joystick.h"
#include "attach.h"
#include "cmdline.h"
#include "fsdevice.h"

/* ------------------------------------------------------------------------- */

char *progname;
char *boot_path;

static RETSIGTYPE break64(int sig);
static void exit64(void);

/* ------------------------------------------------------------------------- */

#ifdef __MSDOS__

#include <dir.h>

static char *orig_workdir;

static void restore_workdir(void)
{
    if (orig_workdir)
	chdir(orig_workdir);
}

static void preserve_workdir(void)
{
    orig_workdir = getcwd(NULL, GET_PATH_MAX);
    atexit(restore_workdir);
}

/* Warning!  This must be called *once*.  */
static void set_boot_path(const char *prg_path)
{
    fname_split(prg_path, &boot_path, NULL);
}

#else  /* __MSDOS__ */

/* Warning!  This must be called *once*.  */
static void set_boot_path(const char *prg_path)
{
    boot_path = findpath(prg_path, getenv("PATH"), X_OK);

    /* Remove the program name.  */
    *strrchr(boot_path, '/') = '\0';
}

#endif /* __MSDOS__ */

/* ------------------------------------------------------------------------- */

/* These are the command-line options for the initialization sequence.  */

static char *autostart_string;
static char *startup_disk_images[4];
static char *startup_tape_image;

static int cmdline_help(const char *param, void *extra_param)
{
    cmdline_show_help();
    exit(0);
}

static int cmdline_default(const char *param, void *extra_param)
{
    resources_set_defaults();
    return 0;
}

static int cmdline_autostart(const char *param, void *extra_param)
{
    if (autostart_string != NULL)
        free(autostart_string);
    autostart_string = stralloc(param);
    return 0;
}

static int cmdline_attach(const char *param, void *extra_param)
{
    int unit = (int) extra_param;

    switch (unit) {
      case 1:
        if (startup_tape_image != NULL)
            free(startup_tape_image);
        startup_tape_image = stralloc(param);
        break;
      case 8:
      case 9:
      case 10:
      case 11:
        if (startup_disk_images[unit - 8] != NULL)
            free(startup_disk_images);
        startup_disk_images[unit - 8] = stralloc(param);
        break;
      default:
        fprintf(stderr, "cmdline_attach(): unexpected unit number %d?!\n",
                unit);
    }

    return 0;
}

static cmdline_option_t cmdline_options[] = {
    { "-help", CALL_FUNCTION, 1, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-default", CALL_FUNCTION, 1, cmdline_default, NULL, NULL, NULL,
      NULL, "Restore default (factory) settings" },
    { "-autostart", CALL_FUNCTION, 1, cmdline_autostart, NULL, NULL, NULL,
      "<name>", "Attach and autostart tape/disk image <name>" },
    { "-1", CALL_FUNCTION, 1, cmdline_attach, (void *) 1, NULL, NULL,
      "<name>", "Attach <name> as a tape image" },
    { "-8", CALL_FUNCTION, 1, cmdline_attach, (void *) 8, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #8" },
    { "-9", CALL_FUNCTION, 1, cmdline_attach, (void *) 9, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #9" },
    { "-10", CALL_FUNCTION, 1, cmdline_attach, (void *) 10, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #10" },
    { "-11", CALL_FUNCTION, 1, cmdline_attach, (void *) 11, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #11" },
    { NULL }
};

/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    if (atexit (exit64) < 0) {
	perror ("atexit");
	return -1;
    }

#ifdef __GO32__
    /* Set the default file mode.  */
    _fmode = O_BINARY;
#endif
#ifdef __MSDOS__
    /* Avoid exiting to a different directory than the one we were called
       from.  */
    preserve_workdir();
#endif

    progname = strrchr(argv[0], '/');
    if (!progname)
	progname = argv[0];
    else
	progname++;

    set_boot_path(argv[0]);
    printf("boot_path = `%s'\n", boot_path);

    /* Initialize system file locator.  */
    sysfile_init(boot_path, machine_name);

    /* VICE boot sequence.  */
    printf ("\n*** VICE Version %s ***\n", VERSION);
    printf ("Welcome to %s, the free portable Commodore %s Emulator.\n\n",
	    progname, machine_name);
    printf ("Written by\n"
	    "E. Perazzoli, T. Rantanen, A. Fachat, D. Sladic,\n"
            "A. Boose, J. Valta and J. Sonninen.\n\n");
    printf ("This is free software with ABSOLUTELY NO WARRANTY.\n");
    printf ("See the \"About VICE\" command for more info.\n\n");

    /* Initialize the user interface.  UiInit() might need to handle the
       command line somehow, so we call it before parsing the options.
       (e.g. under X11, the `-display' option is handled independently).  */
    if (ui_init(&argc, argv) < 0) {
        fprintf(stderr, "Cannot initialize the UI.\n");
	exit (-1);
    }

    /* Initialize resource handling.  */
    if (resources_init(machine_name)) {
        fprintf(stderr, "Cannot initialize resource handling.\n");
        exit(-1);
    }
    if (sysfile_init_resources() < 0) {
        fprintf(stderr,
                "Cannot initialize resources for the system file locator.\n");
        exit(-1);
    }
    if (ui_init_resources() < 0) {
        fprintf(stderr, "Cannot initialize UI-specific resources.\n");
        exit(-1);
    }
    if (file_system_init_resources() < 0) {
        fprintf(stderr, "Cannot initialize file system-specific resources.\n");
        exit(-1);
    }
    if (joystick_init_resources() < 0) {
        fprintf(stderr, "Cannot initialize joystick-specific resources.\n");
        exit(-1);
    }

    /* Initialize file system device-specific resources.  */
    if (fsdevice_init_resources() < 0) {
        fprintf(stderr, "Cannot initialize file system device-specific
                        resources.\n");
        exit(-1);
    }
    if (machine_init_resources() < 0) {
        fprintf(stderr, "Cannot initialize machine-specific resources.\n");
        exit(-1);
    }

    /* Set factory defaults.  */
    resources_set_defaults();

    /* Load the user's default configuration file.  */
    if (resources_load(NULL) == -1) {
	fprintf(stderr,
		"Couldn't find user's configuration file: "
		"using default settings.\n");
	/* The resource file might contain errors, and thus certain resources
	   might have been initialized anyway.  */
	resources_set_defaults();
    }

    /* Initialize command-line options.  */
    cmdline_init();
    if (cmdline_register_options(cmdline_options) < 0) {
        fprintf(stderr, "Cannot initialize main command-line options.\n");
        exit(-1);
    }
    if (sysfile_init_cmdline_options() < 0) {
        fprintf(stderr, "Cannot initialize command-line options for system file locator.\n");
        exit(-1);
    }
    if (joystick_init_cmdline_options() < 0) {
        fprintf(stderr, "Cannot initialize joystick-specific command-line options.\n");
        exit(-1);
    }
    if (ui_init_cmdline_options() < 0) {
        fprintf(stderr, "Cannot initialize UI-specific command-line options.\n");
        exit(-1);
    }
    if (machine_init_cmdline_options() < 0) {
        fprintf(stderr, "Cannot initialize machine-specific command-line options.\n");
        exit(-1);
    }
    if (cmdline_parse(&argc, argv) < 0) {
        fprintf(stderr, "Error parsing command-line options, bailing out.\n");
        exit(-1);
    }

    /* The last orphan option is the same as `-autostart'.  */
    if (argc >= 1 && autostart_string == NULL) {
        autostart_string = stralloc(argv[1]);
        argc--, argv++;
    }

    if (argc > 1) {
        int i;

        fprintf(stderr, "Extra arguments on command-line:\n");
        for (i = 1; i < argc; i++)
            fprintf(stderr, "\t%s\n", argv[i]);
        exit(-1);
    }

    putchar('\n');

    /* Complete the GUI initialization (after loading the resources and
       parsing the command-line) if necessary.  */
    if (ui_init_finish() < 0)
	exit(-1);

    putchar('\n');

#ifdef __MSDOS__
    signal(SIGINT,   SIG_IGN);
#else
    signal(SIGINT,   break64);
#endif

    signal(SIGSEGV,  break64);
    signal(SIGHUP,   break64);
    signal(SIGQUIT,  break64);
    signal(SIGILL,   break64);
    signal(SIGTERM,  break64);
    signal(SIGPIPE,  break64);

    /* Initialize real joystick.  */
    joystick_init();

    if (video_init() < 0)
	exit (-1);

    /* Machine-specific initialization.  */
    if (machine_init() < 0) {
        fprintf(stderr, "Machine initialization failed.\n");
        exit(1);
    }

    /* Handle general-purpose command-line options.  */

    /* `-autostart' */
    if (autostart_string != NULL) {
        FILE *autostart_fd;
        char *autostart_prg;
        char *autostart_file;
        char *tmp;

        /* Check for image:prg -format.  */
        tmp = strrchr(autostart_string, ':');
        if (tmp) {
            autostart_file = stralloc(autostart_string);
            autostart_prg = strrchr(autostart_file, ':');
            *autostart_prg++ = '\0';
            autostart_fd = fopen(autostart_file, "r");
            /* Does the image exist?  */
            if (autostart_fd) {
                fclose(autostart_fd);
                autostart_autodetect(autostart_file, autostart_prg);
            }
            else
                autostart_autodetect(autostart_string, NULL);
            free(autostart_file);
        } else {
            autostart_autodetect(autostart_string, NULL);
        }
    }

    /* `-8', `-9', `-10' and `-11': Attach specified disk image.  */
    {
        int i;

        for (i = 0; i < 4; i++) {
            if (startup_disk_images[i] != NULL
                && file_system_attach_disk(i + 8, startup_disk_images[i]) < 0)
                fprintf(stderr, "Cannot attach disk image `%s' to unit %d.\n",
                        startup_disk_images[i], i + 8);
        }
    }

    /* `-1': Attach specified tape image.  */
    if (startup_tape_image
	&& serial_select_file(DT_TAPE, 1, startup_tape_image) < 0)
        fprintf(stderr, "Cannot attach tape image `%s'.\n",
                startup_tape_image);

    putchar ('\n');

    /* Let's go...  */
    maincpu_trigger_reset();
    mainloop(0);

    printf("perkele!\n");
    exit(0);   /* never reached */
}

static RETSIGTYPE break64(int sig)
{
#ifdef SYS_SIGLIST_DECLARED
    fprintf(stderr, "Received signal %d (%s).\n", sig, sys_siglist[sig]);
#else
    fprintf(stderr, "Received signal %d.\n", sig);
#endif

    exit (-1);
}

static void exit64(void)
{
    /* Disable SIGINT.  This is done to prevent the user from keeping C-C
       pressed and thus breaking the cleanup process, which might be
       dangerous.  */
    signal(SIGINT, SIG_IGN);

    printf("\nExiting...\n");

    machine_shutdown();
    video_free();
    sound_close();

#ifndef __MSDOS__
    joystick_close();
#endif

    putchar ('\n');
}

void end64()
{
    exit (-1);
}

