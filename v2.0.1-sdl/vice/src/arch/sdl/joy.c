/*
 * joy.c - SDL joystick support.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 *  Ulmer Lionel <ulmer@poly.polytechnique.fr>
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
#include <stdlib.h>

#include "archdep.h"
#include "cmdline.h"
#include "joy.h"
#include "joystick.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "sysfile.h"
#include "util.h"
#include "uimenu.h"

#define DEFAULT_JOYSTICK_THRESHOLD 10000
#define DEFAULT_JOYMAPFILE "sdl_joymap.vjm"

static log_t sdljoy_log = LOG_ERR;

/* Joystick mapping filename */
static char *joymap_file = NULL;

/* (Used by `keyboard.c').  */
int joystick_port_map[2];

/* Total number of joysticks */
static int num_joysticks;

/* Joystick threshold (0..32767) */
static int joystick_threshold;

/* Different types of joystick input */
typedef enum {
    AXIS = 0,
    BUTTON = 1,
    HAT = 2,
    BALL = 3,
    NUM_INPUT_TYPES
} sdljoystick_input_t;

/* Number of actions per input source */
static const int input_mult[NUM_INPUT_TYPES] = { 
    2, /* Axis: actions for positive and negative */
    1, /* Button */
    4, /* Hat: actions for all 4 directions */
    1  /* Ball */
};

/* Actions to perform on joystick input */
typedef enum {
    NONE = 0,
    /* Joystick movement or button press */
    JOYSTICK = 1,
    /* Keyboard key press */
    KEYBOARD = 2,
    /* (De)Activate virtual keyboard */
    VIRTUAL_KBD = 3,
    /* (De)Activate UI */
    UI_ACTIVATE = 4,
    /* Call UI function */
    UI_FUNCTION = 5
} sdljoystick_action_t;

/* Input mapping for each direction/button/etc */
struct sdljoystick_mapping_s {
    /* Action to perform */
    sdljoystick_action_t action;

    union {
        /* joy[0] = port number (0,1), joy[1] = pin number, joy[3] = previous value*/
        BYTE joy[3];
        /* key[0] = row, key[1] = column */
        BYTE key[2];
        /* TODO */
        int ui_function;
    } value;
};
typedef struct sdljoystick_mapping_s sdljoystick_mapping_t;

struct sdljoystick_s {
    SDL_Joystick *joyptr;
    const char *name;
    int input_max[NUM_INPUT_TYPES];
    sdljoystick_mapping_t *input[NUM_INPUT_TYPES];
};
typedef struct sdljoystick_s sdljoystick_t;

static sdljoystick_t *sdljoystick = NULL;

/* ------------------------------------------------------------------------- */

/* Resources.  */

static int joymap_file_set(const char *val, void *param)
{
    if (util_string_set(&joymap_file, val))
        return 0;

    return joy_arch_mapping_load(joymap_file);
}

static int joyport1select(int val, void *param)
{
    joystick_port_map[0] = val;
    return 0;
}

static int joyport2select(int val, void *param)
{
    joystick_port_map[1] = val;
    return 0;
}

static int set_joystick_threshold(int val, void *param)
{
    joystick_threshold = val;
    return 0;
}


static const resource_string_t resources_string[] = {
    { "JoyMapFile", DEFAULT_JOYMAPFILE, RES_EVENT_NO, NULL,
      &joymap_file, joymap_file_set, (void *)0 },
    { NULL },
};

static const resource_int_t resources_int[] = {
    { "JoyDevice1", 0, RES_EVENT_NO, NULL,
      &joystick_port_map[0], joyport1select, NULL },
    { "JoyDevice2", 0, RES_EVENT_NO, NULL,
      &joystick_port_map[1], joyport2select, NULL },
    { "JoyThreshold", DEFAULT_JOYSTICK_THRESHOLD, RES_EVENT_NO, NULL,
      &joystick_threshold, set_joystick_threshold, NULL },
    { NULL },
};

/* Command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL, "JoyDevice1", NULL,
      "<0-4>", N_("Set device for joystick port 1") },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL, "JoyDevice2", NULL,
      "<0-4>", N_("Set device for joystick port 2") },
    { "-joymap", SET_RESOURCE, 1, NULL, NULL, "JoyMapFile", NULL,
      N_("<name>"), N_("Specify name of joystick map file") },
    { "-joythreshold", SET_RESOURCE, 1, NULL, NULL, "JoyThreshold", NULL,
      "<0-32767>", N_("Set joystick threshold") },
    { NULL },
};

int joystick_arch_init_resources(void)
{
fprintf(stderr,"%s\n",__func__);
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}

int joystick_init_cmdline_options(void)
{
fprintf(stderr,"%s\n",__func__);
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/**********************************************************
 * Generic high level joy routine                         *
 **********************************************************/
int joy_arch_init(void)
{
    int i, axis, button, hat, ball;
    sdljoystick_input_t j;
    SDL_Joystick *joy;

    sdljoystick = NULL;

    sdljoy_log = log_open("SDLJoystick");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
        log_error(sdljoy_log,"Subsystem init failed!");
        return -1;
    }

    num_joysticks = SDL_NumJoysticks();

    if(num_joysticks == 0) {
        log_message(sdljoy_log,"No joysticks found");
        return 0;
    }

    log_message(sdljoy_log,"%i joysticks found",num_joysticks);

    sdljoystick = (sdljoystick_t *)lib_malloc(sizeof(sdljoystick_t) * num_joysticks);

    for(i=0; i<num_joysticks; ++i) {
        joy = sdljoystick[i].joyptr = SDL_JoystickOpen(i);
        if(joy) {
            sdljoystick[i].name = SDL_JoystickName(i);
            axis = sdljoystick[i].input_max[AXIS] = SDL_JoystickNumAxes(joy);
            button = sdljoystick[i].input_max[BUTTON] = SDL_JoystickNumButtons(joy);
            hat = sdljoystick[i].input_max[HAT] = SDL_JoystickNumHats(joy);
            ball = sdljoystick[i].input_max[BALL] = SDL_JoystickNumBalls(joy);

            for(j=AXIS; j<NUM_INPUT_TYPES; ++j) {
                if(sdljoystick[i].input_max[j] > 0) {
                    sdljoystick[i].input[j] = (sdljoystick_mapping_t *)lib_malloc(sizeof(sdljoystick_mapping_t) * sdljoystick[i].input_max[j] * input_mult[j]);
                } else {
                    sdljoystick[i].input[j] = NULL;
                }
            }

            joy_arch_init_default_mapping(i);

            log_message(sdljoy_log,"Device %i \"%s\" (%i axes, %i buttons, %i hats, %i balls)",
                        i, sdljoystick[i].name, axis, button, hat, ball);
        } else {
            log_warning(sdljoy_log,"Couldn't open joystick %i",i);
        }
    }

    joy_arch_mapping_load(joymap_file);

    SDL_JoystickEventState(SDL_ENABLE);
    return 0;
}

void joystick_close(void)
{
    int i;
    sdljoystick_input_t j;

fprintf(stderr,"%s\n",__func__);

/*    joy_arch_mapping_dump(joymap_file);*/

    lib_free(joymap_file);
    joymap_file = NULL;

    if(sdljoystick == NULL)
        return;

    for(i=0; i<num_joysticks; ++i) {
        if(sdljoystick[i].joyptr)
            SDL_JoystickClose(sdljoystick[i].joyptr);

        for(j=AXIS; j<NUM_INPUT_TYPES; ++j) {
            lib_free(sdljoystick[i].input[j]);
            sdljoystick[i].input[j] = NULL;
        }
    }

    lib_free(sdljoystick);
    sdljoystick = NULL;
}

/* ------------------------------------------------------------------------- */

void joy_arch_init_default_mapping(int joynum)
{
    int i, joyport, pin, state = 0;

    SDL_JoystickUpdate();

    for(i=0; i<sdljoystick[joynum].input_max[AXIS]*input_mult[AXIS]; ++i) {
        joyport = ((1+joynum+((i&4)>>2))&1);
        pin = 8>>(i&3);

        /* Poll each joystick axis once */
        if((i % input_mult[AXIS]) == 0) {
            state = SDL_JoystickGetAxis(sdljoystick[joynum].joyptr, i/input_mult[AXIS]);
        }

        /* Check that the default joystick value is within the threshold.
           Some devices have axes that are +/-32767 when idle; mapping
           those to NONE (by default) avoids some problems. */
        if((state > joystick_threshold)||(state < -joystick_threshold)) {
            log_warning(sdljoy_log, "Axis %i exceeds threshold, mapping to NONE", i/input_mult[AXIS]);
            sdljoystick[joynum].input[AXIS][i++].action = NONE;
            sdljoystick[joynum].input[AXIS][i].action = NONE;
        } else {
            sdljoystick[joynum].input[AXIS][i].action = JOYSTICK;
            sdljoystick[joynum].input[AXIS][i].value.joy[0] = joyport;
            sdljoystick[joynum].input[AXIS][i].value.joy[1] = pin;
        }
    }

    for(i=0; i<sdljoystick[joynum].input_max[BUTTON]*input_mult[BUTTON]; ++i) {
        switch(i&3) {
            case 0:
            case 1:
            default:
                joyport = ((1+joynum+(i&1))&1);
                pin = 1<<4;
                sdljoystick[joynum].input[BUTTON][i].action = JOYSTICK;
                sdljoystick[joynum].input[BUTTON][i].value.joy[0] = joyport;
                sdljoystick[joynum].input[BUTTON][i].value.joy[1] = pin;
                break;
            case 2:
                sdljoystick[joynum].input[BUTTON][i].action = UI_ACTIVATE;
                break;
            case 3:
                sdljoystick[joynum].input[BUTTON][i].action = VIRTUAL_KBD;
                break;
        }
    }

    for(i=0; i<sdljoystick[joynum].input_max[HAT]*input_mult[HAT]; ++i) {
        sdljoystick[joynum].input[HAT][i].action = NONE;
    }

    for(i=0; i<sdljoystick[joynum].input_max[BALL]*input_mult[BALL]; ++i) {
        sdljoystick[joynum].input[BALL][i].action = NONE;
    }
}

int joy_arch_mapping_dump(const char *filename)
{
fprintf(stderr,"%s\n",__func__);
    FILE *fp;
    int i, k;
    sdljoystick_input_t j;
    sdljoystick_action_t t;
    
    if (filename == NULL)
        return -1;

    fp = fopen(filename, MODE_WRITE_TEXT);
 
    if (fp == NULL)
        return -1;

    fprintf(fp, "# VICE joystick mapping file\n"
            "#\n"
            "# A joystick map is read in as patch to the current map.\n"
            "#\n"
            "# File format:\n"
            "# - comment lines start with '#'\n"
            "# - keyword lines start with '!keyword'\n"
            "# - normal line has 'joynum inputtype inputindex action'\n"
            "#\n"              
            "# Keywords and their lines are:\n"
            "# '!CLEAR'    clear all mappings\n"
            "#\n"
            "# inputtype:\n"
            "# 0      axis\n"
            "# 1      button\n"
            "# 2      hat\n"
            "# 3      ball\n"
            "#\n"
            "# Note that each axis has 2 inputindex entries and each hat has 4.\n"
            "#\n"
            "# action [action_parameters]:\n"
            "# 0              none\n"
            "# 1 port pin     joystick (pin: 1/2/4/8/16 = u/d/l/r/fire)\n"
            "# 2 row col      keyboard\n"
            "# 3              virtual keyboard\n"
            "# 4              UI activate\n"
            "# 5 TODO         UI function\n"
            "#\n\n"
        );

    fprintf(fp, "!CLEAR\n\n");

    for(i=0; i<num_joysticks; ++i) {
        fprintf(fp, "# %s\n", sdljoystick[i].name);
        for(j=AXIS; j<NUM_INPUT_TYPES; ++j) {
            for(k=0; k<sdljoystick[i].input_max[j]*input_mult[j]; ++k) {
                t = sdljoystick[i].input[j][k].action;
                fprintf(fp, "%i %i %i %i", i, j, k, t);
                switch(t) {
                    case JOYSTICK:
                        fprintf(fp, " %i %i", 
                                sdljoystick[i].input[j][k].value.joy[0],
                                sdljoystick[i].input[j][k].value.joy[1]
                                );
                        break;
                    case KEYBOARD:
                        fprintf(fp, " %i %i", 
                                sdljoystick[i].input[j][k].value.key[0],
                                sdljoystick[i].input[j][k].value.key[1]
                                );
                        break;
                    case UI_FUNCTION:
                        fprintf(fp, " %i", 
                                sdljoystick[i].input[j][k].value.ui_function
                                );
                        break;
                    default:
                        break;
                }
                fprintf(fp, "\n");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n\n");
    }

    fclose(fp);

    return 0;
}

static void joy_arch_keyword_clear(void)
{
    int i, k;
    sdljoystick_input_t j;

    for(i=0; i<num_joysticks; ++i) {
        for(j=AXIS; j<NUM_INPUT_TYPES; ++j) {
            for(k=0; k<sdljoystick[i].input_max[j]*input_mult[j]; ++k) {
                sdljoystick[i].input[j][k].action = NONE;
            }
        }
    }
}

static void joy_arch_parse_keyword(char *buffer)
{
    char *key;
    
    key = strtok(buffer + 1, " \t:");
    
    if (!strcmp(key, "CLEAR")) {
        joy_arch_keyword_clear();
    }
}

static void joy_arch_parse_entry(char *buffer)
{
    char *p;
    int joynum, inputindex, data1, data2;
    sdljoystick_input_t inputtype;
    sdljoystick_action_t action;
 
    p = strtok(buffer, " \t:");

    joynum = atoi(p);
    
    if (joynum >= num_joysticks) {
        log_error(sdljoy_log, _("Could not find joystick %i!"), joynum);
        return;
    }

    p = strtok(NULL, " \t,");
    if (p != NULL) {
        inputtype = (sdljoystick_input_t)atoi(p);
        p = strtok(NULL, " \t,");
        if (p != NULL) {
            inputindex = atoi(p);
            p = strtok(NULL, " \t");
            if (p != NULL) {
                action = (sdljoystick_action_t)atoi(p);

                switch(action) {
                    case JOYSTICK:
                    case KEYBOARD:
                        p = strtok(NULL, " \t");
                        data1 = atoi(p);
                    /* fall through */
                    case UI_FUNCTION:
                        p = strtok(NULL, " \t");
                        data2 = atoi(p);
                        break;
                    default:
                        break;
                }

                if(inputindex < sdljoystick[joynum].input_max[inputtype]*input_mult[inputtype]) {
                    sdljoystick[joynum].input[inputtype][inputindex].action = action;

                    switch(action) {
                        case JOYSTICK:
                            sdljoystick[joynum].input[inputtype][inputindex].value.joy[0] = data1;
                            sdljoystick[joynum].input[inputtype][inputindex].value.joy[1] = data2;
                            break;
                        case KEYBOARD:
                            sdljoystick[joynum].input[inputtype][inputindex].value.key[0] = data1;
                            sdljoystick[joynum].input[inputtype][inputindex].value.key[1] = data2;
                            break;
                        case UI_FUNCTION:
                            sdljoystick[joynum].input[inputtype][inputindex].value.ui_function = data2;
                            break;
                        default:
                            break;
                    }
                } else {
                    log_warning(sdljoy_log, _("inputindex %i too large for inputtype %i, joynum %i!"), inputindex, inputtype, joynum);
                }
            }
        }
    }
}

int joy_arch_mapping_load(const char *filename)
{
    FILE *fp;
    char *complete_path;
    char buffer[1000];

fprintf(stderr,"%s, %s\n",__func__, filename);

    /* Silently ignore keymap load on resource & cmdline init */
    if (sdljoystick == NULL)
        return 0;

    if (filename == NULL)
        return -1;

    fp = sysfile_open(filename, &complete_path, MODE_READ_TEXT);

    if (fp == NULL) {
        log_warning(sdljoy_log, _("Failed to open `%s'."), filename);
        lib_free(complete_path);
        return -1;
    }

    log_message(sdljoy_log, _("Loading joystick map `%s'."), complete_path);

    lib_free(complete_path);

    do {
        buffer[0] = 0;
        if (fgets(buffer, 999, fp)) {
        char *p;

            if (strlen(buffer) == 0)
                break;

            buffer[strlen(buffer) - 1] = 0; /* remove newline */
        /* remove comments */
        if((p = strchr(buffer, '#')))
            *p=0;

            switch(*buffer) {
              case 0:
                break;
              case '!':
                /* keyword handling */
                joy_arch_parse_keyword(buffer);
                break;
              default:
                /* table entry handling */
                joy_arch_parse_entry(buffer);
                break;
            }
        }
    } while (!feof(fp));
    fclose(fp);

    return 0;
}

/* ------------------------------------------------------------------------- */

ui_menu_action_t sdljoy_perform_event(sdljoystick_mapping_t *event, int value)
{
    BYTE t;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    if(sdl_menu_state) {
        if((value)&&(event->action == JOYSTICK)) {
            switch(event->value.joy[1]) {
                case 0x01:
                    retval = MENU_ACTION_UP;
                    break;
                case 0x02:
                    retval = MENU_ACTION_DOWN;
                    break;
                case 0x04:
                    retval = MENU_ACTION_CANCEL;
                    break;
                case 0x10:
                    retval = MENU_ACTION_SELECT;
                    break;
                default:
                    break;
            }
        }
        return retval;
    }

    switch(event->action) {
        case JOYSTICK:
            t = event->value.joy[0];
            if(joystick_port_map[t] == JOYDEV_JOYSTICK) {
                if(value) {
                    joystick_set_value_or(t+1, event->value.joy[1]);
                } else {
                    joystick_set_value_and(t+1, ~(event->value.joy[1]));
                }
            }
            break;
        case KEYBOARD:
            keyboard_set_keyarr(event->value.key[0], event->value.key[1], value);
            break;
        case VIRTUAL_KBD:
        case UI_ACTIVATE:
        case UI_FUNCTION:
        case NONE:
        default:
            break;
    }

    return retval;
}

ui_menu_action_t sdljoy_axis_event(Uint8 joynum, Uint8 axis, Sint16 value)
{
    BYTE cur, prev;
    int index;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    if(value < -joystick_threshold) {
        cur = 2;
    } else if(value > joystick_threshold) {
        cur = 1;
    } else {
        cur = 0;
    }

    index = axis*input_mult[AXIS];
    prev = sdljoystick[joynum].input[AXIS][index].value.joy[2];

    if(cur == prev)
        return retval;

    if(cur == 1) {
        if(prev == 2) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index+1]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 1);
    } else if(cur == 2) {
        if(prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index+1]), 1);
    } else {
        if(prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 0);
        } else {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index+1]), 0);
        }
    }

    sdljoystick[joynum].input[AXIS][index].value.joy[2] = cur;
    return retval;
}

ui_menu_action_t sdljoy_button_event(Uint8 joynum, Uint8 button, Uint8 value)
{
    return sdljoy_perform_event(&(sdljoystick[joynum].input[BUTTON][button]), value);
}

