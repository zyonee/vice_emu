/*
 * main.c - VICE startup.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Vesa-Matti Puro <vmp@lut.fi>
 *  Jarkko Sonninen <sonninen@lut.fi>
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *  Andr� Fachat <a.fachat@physik.tu-chemnitz.de>
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

#include "vice.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "archdep.h"
#include "attach.h"
#include "autostart.h"
#include "cartridge.h"
#include "charsets.h"
#include "cmdline.h"
#include "console.h"
#include "diskimage.h"
#include "drive.h"
#include "drivecpu.h"
#include "fsdevice.h"
#include "fullscreen.h"
#include "interrupt.h"
#include "joystick.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "main_exit.h"
#include "palette.h"
#include "resources.h"
#include "sysfile.h"
#include "screenshot.h"
#include "tape.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "vdrive.h"
#include "version.h"
#include "video.h"

#ifdef HAVE_MOUSE
#include "mouse.h"
#endif

/* ------------------------------------------------------------------------- */

int vsid_mode = 0;
#ifdef __OS2__
const
#endif
int console_mode = 0;
static int init_done;

/* ------------------------------------------------------------------------- */

/* These are the command-line options for the initialization sequence.  */

static char *autostart_string;
static char *startup_disk_images[4];
static char *startup_tape_image;

static int cmdline_help(const char *param, void *extra_param)
{
    cmdline_show_help(NULL);
    exit(0);

    return 0;   /* OSF1 cc complains */
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

#ifndef __OS2__
static int cmdline_console(const char *param, void *extra_param)
{
    console_mode = 1;
    return 0;
}
#endif

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
            free(startup_disk_images[unit - 8]);
        startup_disk_images[unit - 8] = stralloc(param);
        break;
      default:
        archdep_startup_log_error("cmdline_attach(): unexpected unit number %d?!\n",
                                  unit);
    }

    return 0;
}

static cmdline_option_t vsid_cmdline_options[] = {
    { "-help", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-?", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
#ifndef __OS2__
    { "-console", CALL_FUNCTION, 0, cmdline_console, NULL, NULL, NULL,
      NULL, "Console mode (for playing music)" },
    { "-core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Allow production of core dumps" },
    { "+core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Do not produce core dumps" },
#else
    { "-debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Don't call exception handler" },
    { "+debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Call exception handler (default)" },
#endif
    { NULL }
};

static cmdline_option_t cmdline_options[] = {
    { "-help", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-?", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-default", CALL_FUNCTION, 0, cmdline_default, NULL, NULL, NULL,
      NULL, "Restore default (factory) settings" },
    { "-autostart", CALL_FUNCTION, 1, cmdline_autostart, NULL, NULL, NULL,
      "<name>", "Attach and autostart tape/disk image <name>" },
    { "-1", CALL_FUNCTION, 1, cmdline_attach, (void *)1, NULL, NULL,
      "<name>", "Attach <name> as a tape image" },
    { "-8", CALL_FUNCTION, 1, cmdline_attach, (void *)8, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #8" },
    { "-9", CALL_FUNCTION, 1, cmdline_attach, (void *)9, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #9" },
    { "-10", CALL_FUNCTION, 1, cmdline_attach, (void *)10, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #10" },
    { "-11", CALL_FUNCTION, 1, cmdline_attach, (void *)11, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #11" },
#ifdef __OS2__
    { "-debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Don't call exception handler" },
    { "+debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Call exception handler (default)" },
#else
    { "-console", CALL_FUNCTION, 0, cmdline_console, NULL, NULL, NULL,
      NULL, "Console mode (for playing music)" },
    { "-core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Allow production of core dumps" },
    { "+core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Do not produce core dumps" },
#endif
    { NULL }
};

/* ------------------------------------------------------------------------- */

static int do_core_dumps = 0;

static int set_do_core_dumps(resource_value_t v, void *param)
{
    do_core_dumps = (int) v;
    return 0;
}

static resource_t resources[] =
{
    {"DoCoreDump", RES_INTEGER, (resource_value_t) 0,
    (resource_value_t *) &do_core_dumps, set_do_core_dumps, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */

static int init_resources(void)
{
    if (resources_init(machine_name)) {
        archdep_startup_log_error("Cannot initialize resource handling.\n");
        return -1;
    }
    if (log_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize log resource handling.\n");        return -1;
    }
    if (resources_register(resources) < 0) {
        archdep_startup_log_error("Cannot initialize main resources.\n");
        return -1;
    }
    if (sysfile_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize resources for the system file locator.\n");
        return -1;
    }
    if (ui_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize UI-specific resources.\n");        return -1;
    }
    if (file_system_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize file system-specific resources.\n");
        return -1;
    }
    /* Initialize file system device-specific resources.  */
    if (fsdevice_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize file system device-specific resources.\n");
        return -1;
    }
    if (machine_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize machine-specific resources.\n");
        return -1;
    }
    if (joystick_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize joystick-specific resources.\n");
        return -1;
    }
#ifdef HAVE_MOUSE
    if (mouse_init_resources() < 0) {
        archdep_startup_log_error("Cannot initialize mouse-specific resources.\n");
        return -1;
    }
#endif
    return 0;
}

static int init_cmdline_options(void)
{
    cmdline_option_t* main_cmdline_options =
        vsid_mode ? vsid_cmdline_options : cmdline_options;

    if (cmdline_init()) {
        archdep_startup_log_error("Cannot initialize resource handling.\n");
        return -1;
    }
    if (log_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize log command-line option handling.\n");
        return -1;
    }
    if (cmdline_register_options(main_cmdline_options) < 0) {
        archdep_startup_log_error("Cannot initialize main command-line options.\n");
        return -1;
    }
    if (sysfile_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize command-line options for system file locator.\n");
        return -1;
    }
    if (ui_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize UI-specific command-line options.\n");
        return -1;
    }
    if (file_system_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize Attach-specific command-line options.\n");
        return -1;
    }
    if (machine_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize machine-specific command-line options.\n");
        return -1;
    }

    if (vsid_mode) {
        return 0;
    }

    if (fsdevice_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize file system-specific command-line options.\n");
        return -1;
    }
    if (joystick_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize joystick-specific command-line options.\n");
        return -1;
    }
#ifdef HAVE_MOUSE
    if (mouse_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize mouse-specific command-line options.\n");
        return -1;
    }
#endif
    if (kbd_buf_init_cmdline_options() < 0) {
        archdep_startup_log_error("Cannot initialize keyboard buffer-specific command-line options.\n");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

static char *hexstring_to_byte(const char *s, BYTE *value_return)
{
    int count;
    char c;

    for (*value_return = 0, count = 0; count < 2 && *s != 0; s++) {
        c = toupper(*s);
        if (c >= 'A' && c <= 'F') {
            *value_return <<= 4;
            *value_return += c - 'A' + 10;
        } else if (isdigit (c)) {
            *value_return <<= 4;
            *value_return += c - '0';
        } else {
            return (char *) s;
        }
    }

    return (char *) s;
}

/* This is a helper function for the `-autostart' command-line option.  It
   replaces all the $[0-9A-Z][0-9A-Z] patterns in `string' and returns it.  */
static char *replace_hexcodes(char *s)
{
    unsigned int len, dest_len;
    char *p;
    char *new_s;

    len = strlen(s);
    new_s = (char*)xmalloc(len + 1);

    p = s;
    dest_len = 0;
    while (1) {
        char *p1;

        p1 = strchr (p, '$');
        if (p1 != NULL) {
            char *new_p;

            new_p = hexstring_to_byte(p1 + 1,
                                      (BYTE *)(new_s + dest_len + (p1 - p)));
            if (p1 != p) {
                memcpy(new_s + dest_len, p, (size_t)(p1 - p));
                dest_len += p1 - p;
                dest_len++;
            }

            p = new_p;
        } else {
            break;
        }
    }

    strcpy((char *)(new_s + dest_len), p);
    return new_s;
}

/* ------------------------------------------------------------------------- */

/* This is the main program entry point.  When not compiling for Windows,
   this is `main()'; on Windows we have to #define the name to something
   different because the standard entry point is `WinMain()' there.  */

int MAIN_PROGRAM(int argc, char **argv)
{
    int i;

    /* Check for -console and -vsid before initializing the user interface.
       -console => no user interface
       -vsid    => user interface in separate process */
    for (i = 0; i < argc; i++) {
#ifndef __OS2__
        if (strcmp(argv[i], "-console") == 0) {
            console_mode = 1;
        } else
#endif
        if (strcmp(argv[i], "-vsid") == 0) {
            vsid_mode = 1;
        }
    }

#ifdef ENABLE_NLS
    /* gettext stuff, not needed in Gnome, but here I can
       overrule the default locale path */
    setlocale(LC_ALL, "");
    bindtextdomain (PACKAGE, NLS_LOCALEDIR);
    textdomain(PACKAGE);
#endif

    archdep_startup(&argc, argv);

#ifndef __riscos
    if (atexit (main_exit) < 0) {
        perror ("atexit");
        return -1;
    }
#endif

    drive_setup_context();

    /* Initialize system file locator.  */
    sysfile_init(machine_name);

    if (init_resources() < 0 || init_cmdline_options() < 0)
        return -1;

    /* Set factory defaults.  */
    resources_set_defaults();

    /* Initialize the user interface.  `ui_init()' might need to handle the
       command line somehow, so we call it before parsing the options.
       (e.g. under X11, the `-display' option is handled independently).  */
    if (!console_mode && ui_init(&argc, argv) < 0) {
        archdep_startup_log_error("Cannot initialize the UI.\n");
        return -1;
    }

    /* Load the user's default configuration file.  */
    if (vsid_mode) {
        resources_set_value("Drive8Type", (resource_value_t)0);
        resources_set_value("Sound", (resource_value_t)1);
#ifdef HAVE_RESID
        resources_set_value("SidUseResid", (resource_value_t)1);
#endif
        resources_set_value("SidModel", (resource_value_t)0);
        resources_set_value("SidFilters", (resource_value_t)1);
        resources_set_value("SoundSampleRate", (resource_value_t)44100);
        resources_set_value("SoundSpeedAdjustment", (resource_value_t)2);
        resources_set_value("SoundBufferSize", (resource_value_t)1000);
        resources_set_value("SoundSuspendTime", (resource_value_t)0);
    } else {
        int retval = resources_load(NULL);

        if (retval < 0) {
            /* The resource file might contain errors, and thus certain
               resources might have been initialized anyway.  */
            resources_set_defaults();
        }
    }

    if (cmdline_parse(&argc, argv) < 0) {
        archdep_startup_log_error("Error parsing command-line options, bailing out. For help use '-help'\n");
        return -1;
    }

    /* The last orphan option is the same as `-autostart'.  */
    if (argc >= 1 && autostart_string == NULL) {
        autostart_string = stralloc(argv[1]);
        argc--, argv++;
    }

    if (argc > 1)
    {
        int len = 0, j;

        for (j = 1; j < argc; j++)
            len += strlen(argv[j]);

        {
            char *txt = xcalloc(1, len + argc + 1);
            for (j = 1; j < argc; j++)
                strcat(strcat(txt, " "),argv[j]);
            archdep_startup_log_error("Extra arguments on command-line: %s\n",
                                      txt);
            free(txt);
        }
        return -1;
    }

    if (log_init() < 0) {
        archdep_startup_log_error("Cannot startup logging system.\n");
    }

    /* VICE boot sequence.  */
    log_message(LOG_DEFAULT, "*** VICE Version %s ***", VERSION);
    log_message(LOG_DEFAULT, " ");
    log_message(LOG_DEFAULT, "Welcome to %s, the free portable Commodore %s Emulator.",
                archdep_program_name(), machine_name);
    log_message(LOG_DEFAULT, " ");
    log_message(LOG_DEFAULT, "Written by");
    log_message(LOG_DEFAULT, "D. Sladic, A. Boose, D. Lem, T. Biczo, A. Dehmel,
T. Bretz,");
    log_message(LOG_DEFAULT, "A. Matthies, M. Pottendorfer, M. Brenner, S. Trikaliotis.");
    log_message(LOG_DEFAULT, " ");
    log_message(LOG_DEFAULT, "This is free software with ABSOLUTELY NO WARRANTY.");
    log_message(LOG_DEFAULT, "See the \"About VICE\" command for more info.");
    log_message(LOG_DEFAULT, " ");

    /* Complete the GUI initialization (after loading the resources and
       parsing the command-line) if necessary.  */
    if (!console_mode && ui_init_finish() < 0)
        return -1;

    if (!console_mode && video_init() < 0)
        return -1;

    archdep_setup_signals(do_core_dumps);

    /* Check for PSID here since we don't want to allow autodetection
       in autostart.c. ROM image loading should also be skipped. */
    if (vsid_mode)
    {
        if (autostart_string
            && machine_autodetect_psid(autostart_string) == -1)
        {
            log_error(LOG_DEFAULT, "`%s' is not a valid PSID file.",
                      autostart_string);
            return -1;
        }
    }

    if (!vsid_mode)
    {
        /* Initialize real joystick.  */
#ifdef HAS_JOYSTICK
        joystick_init();
#endif

        palette_init();
        screenshot_init();

        drive0_cpu_early_init();
        drive1_cpu_early_init();
    }

    /* Machine-specific initialization.  */
    if (machine_init() < 0) {
        log_error(LOG_DEFAULT, "Machine initialization failed.");
        return -1;
    }

    /* FIXME: what's about uimon_init??? */
    if (!vsid_mode && console_init() < 0) {
        log_error(LOG_DEFAULT, "Console initialization failed.");
        return -1;
    }

    keyboard_init();

    if (!vsid_mode)
    {
        disk_image_init();
        vdrive_init();

        /* Handle general-purpose command-line options.  */

        /* `-autostart' */
        if (autostart_string != NULL) {
            FILE *autostart_fd;
            char *autostart_prg_name;
            char *autostart_file;
            char *tmp;

            /* Check for image:prg -format.  */
            tmp = strrchr(autostart_string, ':');
            if (tmp) {
                autostart_file = stralloc(autostart_string);
                autostart_prg_name = strrchr(autostart_file, ':');
                *autostart_prg_name++ = '\0';
                autostart_fd = fopen(autostart_file, MODE_READ);
                /* Does the image exist?  */
                if (autostart_fd) {
                    char *name;

                    fclose(autostart_fd);
                    petconvstring(autostart_prg_name, 0);
                    name = replace_hexcodes(autostart_prg_name);
                    autostart_autodetect(autostart_file, name, 0);
                    free(name);
                } else
                    autostart_autodetect(autostart_string, NULL, 0);
                free(autostart_file);
            } else {
                autostart_autodetect(autostart_string, NULL, 0);
            }
        }

        /* `-8', `-9', `-10' and `-11': Attach specified disk image.  */
        {
            for (i = 0; i < 4; i++) {
                if (startup_disk_images[i] != NULL
                    && file_system_attach_disk(i + 8, startup_disk_images[i])
                    < 0)
                    log_error(LOG_DEFAULT,
                              "Cannot attach disk image `%s' to unit %d.",
                              startup_disk_images[i], i + 8);
            }
        }

        /* `-1': Attach specified tape image.  */
        if (startup_tape_image && tape_attach_image(startup_tape_image) < 0)
            log_error(LOG_DEFAULT, "Cannot attach tape image `%s'.",
                      startup_tape_image);

    }
    ui_init_finalize();

    init_done = 1;

    /* Let's go...  */
    maincpu_trigger_reset();

#ifdef USE_XF86_EXTENSIONS
    if (!(console_mode || vsid_mode))
        fullscreen_mode_init();
#endif

    maincpu_mainloop();

    log_error(LOG_DEFAULT, "perkele!");

    return 0;
}

