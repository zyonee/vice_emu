/*
 * ui.c - Common UI routines.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
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
#include "types.h"

#include <SDL/SDL.h>
#include <stdio.h>

#include "color.h"
#include "joy.h"
#include "kbd.h"
#include "interrupt.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include "uicolor.h"
#include "uimenu.h"
#include "vsync.h"


/* ----------------------------------------------------------------- */
/* ui.h */

void ui_display_speed(float percent, float framerate, int warp_flag)
{
/*fprintf(stderr,"%f%%/%f fps %s\n",percent,framerate,warp_flag?"(warp)":"");*/
}

void ui_display_paused(int flag){}
void ui_dispatch_next_event(void){}

/* SDL event handling */
ui_menu_action_t ui_dispatch_events(void)
{
    SDL_Event e;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                retval = sdlkbd_press(e.key.keysym.sym, e.key.keysym.mod);
                break;
            case SDL_KEYUP:
                sdlkbd_release(e.key.keysym.sym, e.key.keysym.mod);
                break;
            case SDL_JOYAXISMOTION:
                retval = sdljoy_axis_event(e.jaxis.which, e.jaxis.axis, e.jaxis.value);
                break;
            case SDL_JOYBUTTONDOWN:
                retval = sdljoy_button_event(e.jbutton.which, e.jbutton.button, 1);
                break;
            case SDL_JOYBUTTONUP:
                sdljoy_button_event(e.jbutton.which, e.jbutton.button, 0);
                break;
            case SDL_ACTIVEEVENT:
                if(e.active.state & SDL_APPACTIVE) {
                    if(e.active.gain) {
/*fprintf(stderr,"%s: activeevent %i,%i\n",__func__,e.active.state,e.active.gain);*/
                    } else {
                    }
                }
                break;
            default:
/*fprintf(stderr,"%s: %i\n",__func__,e.type);*/
                break;
        }
        /* When using the menu, pass every meaningful event to the caller */
        if ((sdl_menu_state) && (retval != MENU_ACTION_NONE))
            break;
    }
    return retval;
}

void ui_check_mouse_cursor(void){}

void archdep_ui_init(int argc, char *argv[])
{
fprintf(stderr,"%s\n",__func__);
}

void ui_message(const char* format, ...){}

static int is_paused = 0;

static void pause_trap(WORD addr, void *data)
{
    ui_display_paused(1);
    is_paused = 1;
    vsync_suspend_speed_eval();
    while (is_paused)
        ui_dispatch_next_event();
}

void ui_pause_emulation(int flag)
{
    if (flag) {
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        ui_display_paused(0);
        is_paused = 0;
    }
}

int ui_emulation_is_paused(void)
{
    return is_paused;
}

/* ----------------------------------------------------------------- */
/* uiapi.h */

static int set_ui_menukey(int val, void *param)
{
    sdl_ui_menukeys[(ui_menu_action_t)param] = val;
    return 0;
}

static const resource_int_t resources_int[] = {
    { "MenuKey", SDLK_F9, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[0], set_ui_menukey, (void *)MENU_ACTION_NONE },
    { "MenuKeyUp", SDLK_UP, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[1], set_ui_menukey, (void *)MENU_ACTION_UP },
    { "MenuKeyDown", SDLK_DOWN, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[2], set_ui_menukey, (void *)MENU_ACTION_DOWN },
    { "MenuKeySelect", SDLK_RETURN, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[3], set_ui_menukey, (void *)MENU_ACTION_SELECT },
    { "MenuKeyCancel", SDLK_BACKSPACE, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[4], set_ui_menukey, (void *)MENU_ACTION_CANCEL },
    { "MenuKeyExit", SDLK_ESCAPE, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[5], set_ui_menukey, (void *)MENU_ACTION_EXIT },
    { "MenuKeyMap", SDLK_m, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[6], set_ui_menukey, (void *)MENU_ACTION_MAP },
    { NULL },
};

/* Initialization  */
int ui_resources_init(void)
{
fprintf(stderr,"%s\n",__func__);
    return resources_register_int(resources_int);
}

void ui_resources_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

int ui_cmdline_options_init(void)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

int ui_init(int *argc, char **argv)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

int ui_init_finish(void)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

int ui_init_finalize(void)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

void ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

/* Print an error message.  */
void ui_error(const char *format,...){}

/* Display a mesage without interrupting emulation */
void ui_display_statustext(const char *text, int fade_out){}

/* Drive related UI.  */
void ui_enable_drive_status(ui_drive_enable_t state,
                            int *drive_led_color){}
void ui_display_drive_track(unsigned int drive_number,
                            unsigned int drive_base,
                            unsigned int half_track_number){}
/* The pwm value will vary between 0 and 1000.  */
void ui_display_drive_led(int drive_number, unsigned int pwm1,
                          unsigned int led_pwm2){}
void ui_display_drive_current_image(unsigned int drive_number,
                                    const char *image){}
int ui_extend_image_dialog(void)
{
    return 0;
}

/* Tape related UI */
void ui_set_tape_status(int tape_status){}
void ui_display_tape_motor_status(int motor){}
void ui_display_tape_control_status(int control){}
void ui_display_tape_counter(int counter){}
void ui_display_tape_current_image(const char *image){}

/* Show a CPU JAM dialog.  */
ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    return 0;
}

/* Update all menu entries.  */
void ui_update_menus(void){}

/* Recording UI */
void ui_display_playback(int playback_status, char *version){}
void ui_display_recording(int recording_status){}
void ui_display_event_time(unsigned int current, unsigned int total){}

/* Joystick UI */
void ui_display_joyport(BYTE *joyport){}

/* Volume UI */
void ui_display_volume(int vol){}

/* ----------------------------------------------------------------- */
/* uicolor.h */

int uicolor_alloc_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long *color_pixel,
                        BYTE *pixel_return)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

void uicolor_free_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long color_pixel)
{
fprintf(stderr,"%s\n",__func__);
}

void uicolor_convert_color_table(unsigned int colnr, BYTE *data,
                                 long color_pixel, void *c)
{
fprintf(stderr,"%s\n",__func__);
}

int uicolor_set_palette(struct video_canvas_s *c,
                        const struct palette_s *palette)
{
fprintf(stderr,"%s\n",__func__);
    return 0;
}

