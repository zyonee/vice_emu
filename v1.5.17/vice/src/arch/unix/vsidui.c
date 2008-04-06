/*
 * vsidui.c - Implementation of the C64-specific part of the UI.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
 * based on c64ui.c written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#define VSIDUI 1
#include "videoarch.h"

#include <stdio.h>
#include <stdlib.h>

#include "attach.h"
#include "c64mem.h"
#include "c64ui.h"
#include "drive.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "psid.h"
#include "resources.h"
#include "uicommands.h"
#include "uimenu.h"
#include "uisettings.h"
#include "utils.h"
#include "vsync.h"

static log_t vsid_log = LOG_ERR;
static void vsid_create_menus(void);

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(PSIDTune)

static ui_menu_entry_t ui_tune_menu[] = {
  { N_("Tunes"),
    NULL, NULL, NULL },
  { NULL }
};

static UI_CALLBACK(psid_load)
{
  char *filename;
  ui_button_t button;

  filename = ui_select_file(_("Load PSID file"), NULL, False, NULL,
			    "*.[psPS]*", &button, False, NULL);

  suspend_speed_eval();

  switch (button) {
  case UI_BUTTON_OK:
    if (machine_autodetect_psid(filename) < 0) {
      log_error(vsid_log, _("`%s' is not a valid PSID file."), filename);
      return;
    }
    machine_play_psid(0);
    maincpu_trigger_reset();
    vsid_create_menus();
    break;
  default:
    /* Do nothing special.  */
    break;
  }
  if (filename != NULL)
     free(filename);
}

static UI_CALLBACK(psid_tune)
{
  int tune = *((int *)UI_MENU_CB_PARAM);
  machine_play_psid(tune);
  suspend_speed_eval();
  maincpu_trigger_reset();
}


static ui_menu_entry_t ui_load_commands_menu[] = {
  { N_("Load PSID file..."),
    (ui_callback_t)psid_load, NULL, NULL,
    XK_l, UI_HOTMOD_META },
  { NULL }
};


/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(SoundBufferSize)

static ui_menu_entry_t set_sound_buffer_size_submenu[] = {
  { N_("*3.00 sec"), (ui_callback_t) radio_SoundBufferSize,
    (ui_callback_data_t) 3000, NULL },
  { N_("*1.00 sec"), (ui_callback_t) radio_SoundBufferSize,
    (ui_callback_data_t) 1000, NULL },
  { N_("*0.50 sec"), (ui_callback_t) radio_SoundBufferSize,
    (ui_callback_data_t) 500, NULL },
  { N_("*0.10 sec"), (ui_callback_t) radio_SoundBufferSize,
    (ui_callback_data_t) 100, NULL },
  { NULL }
};

UI_MENU_DEFINE_TOGGLE(Sound)

static ui_menu_entry_t sound_settings_submenu[] = {
  { N_("*Enable sound playback"),
    (ui_callback_t) toggle_Sound, NULL, NULL },
  { "--" },
  { N_("Sample rate"),
    NULL, NULL, set_sound_sample_rate_submenu },
  { N_("Buffer size"),
    NULL, NULL, set_sound_buffer_size_submenu },
  { N_("Oversample"),
    NULL, NULL, set_sound_oversample_submenu },
  { NULL },
};

static ui_menu_entry_t ui_sound_settings_menu[] = {
  { N_("Sound settings"),
    NULL, NULL, sound_settings_submenu },
  { NULL }
};

static ui_menu_entry_t psid_menu[] = {
  { N_("SID settings"),
    NULL, NULL, sid_submenu },
  { NULL }
};


/* ------------------------------------------------------------------------- */

extern int num_checkmark_menu_items;

static void vsid_create_menus(void)
{
  static ui_menu_entry_t tune_menu[256];
  static ui_window_t wl = NULL, wr = NULL;
  static int tunes = 0;
  int default_tune;
  int i;
  char *buf;

  buf = stralloc(_("*Default Tune"));

  /* Free previously allocated memory. */
  for (i = 0; i <= tunes; i++) {
    free(tune_menu[i].string);
  }

  /* Get number of tunes in current PSID. */
  tunes = psid_tunes(&default_tune);

  /* Build tune menu. */
  for (i = 0; i <= tunes; i++) {
    tune_menu[i].string =
      (ui_callback_data_t) stralloc(buf);
    tune_menu[i].callback =
      (ui_callback_t) radio_PSIDTune;
    tune_menu[i].callback_data =
      (ui_callback_data_t) i;
    tune_menu[i].sub_menu = NULL;
    tune_menu[i].hotkey_keysym = i < 10 ? XK_0 + i : 0;
    tune_menu[i].hotkey_modifier =
      (ui_hotkey_modifier_t) i < 10 ? UI_HOTMOD_META : 0;
    free(buf);
    buf = xmsprintf(_("*Tune %d"), i + 1);
  }

  free(buf);

  tune_menu[i].string =
    (ui_callback_data_t) NULL;

  ui_tune_menu[0].sub_menu = tune_menu;

  num_checkmark_menu_items = 0;

  if (wl) {
      ui_destroy_widget(wl);
  }
  if (wr) {
      ui_destroy_widget(wr);
  }

  ui_set_left_menu(wl = ui_menu_create("LeftMenu",
				       ui_load_commands_menu,
				       ui_tune_menu,
				       ui_menu_separator,
				       ui_tool_commands_menu,
				       ui_menu_separator,
				       ui_help_commands_menu,
				       ui_menu_separator,
				       ui_run_commands_menu,
				       ui_menu_separator,
				       ui_exit_commands_menu,
				       NULL));

  ui_set_right_menu(wr = ui_menu_create("RightMenu",
					ui_sound_settings_menu,
					ui_menu_separator,
					psid_menu,
					NULL));

  ui_update_menus();
}

int vsid_ui_init(void)
{
  ui_set_application_icon(icon_data);

  vsid_create_menus();
#ifdef LATER
  ui_set_topmenu();
#endif

  return 0;
}

void vsid_ui_display_name(const char *name)
{
    log_message(LOG_DEFAULT, "Name: %s", name);
}

void vsid_ui_display_author(const char *author)
{
    log_message(LOG_DEFAULT, "Author: %s", author);
}

void vsid_ui_display_copyright(const char *copyright)
{
    log_message(LOG_DEFAULT, "Copyright by: %s", copyright);
}

void vsid_ui_display_sync(int sync)
{
    log_message(LOG_DEFAULT, "Using %s sync", sync==DRIVE_SYNC_PAL?"PAL":"NTSC");
}

void vsid_ui_set_default_tune(int nr)
{
    log_message(LOG_DEFAULT, "Default Tune: %i", nr);
}

void vsid_ui_display_tune_nr(int nr)
{
    log_message(LOG_DEFAULT, "Playing Tune: %i", nr);
}

void vsid_ui_display_nr_of_tunes(int count)
{
    log_message(LOG_DEFAULT, "Number of Tunes: %i", count);
}

void vsid_ui_display_time(unsigned int sec)
{
}
