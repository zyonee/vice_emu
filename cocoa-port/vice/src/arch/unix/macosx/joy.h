/*
 * joy.h - Joystick support for Mac OS X.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#ifndef _JOY_H
#define _JOY_H

#ifdef HAS_JOYSTICK

/* NOTE: We use the HID Utilites Library provided by Apple for free

   http://developer.apple.com/samplecode/HID_Utilities_Source/index.html

   Make sure to install this (static) library first!
*/
#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

/* axis map */
struct axis_map
{
  const char *name;
  int tag;
};
typedef struct axis_map axis_map_t;

extern axis_map_t joy_axis_map[];

/* calibration data for an axis */
struct calibration
{
  int  min_threshold;
  int  max_threshold;
};
typedef struct calibration calibration_t;

#define JOYSTICK_DESCRIPTOR_MAX_BUTTONS  32
#define JOYSTICK_DESCRIPTOR_MAX_AXIS     6

/* describe a joystick HID device */
struct joystick_descriptor 
{
  /* resources/settings for joystick */
  char         *query_vid_pid;    /* query by vid & pid */
  char         *query_serial;     /* query for serial number */
  
  char         *x_axis_name;      /* set x axis */
  char         *y_axis_name;      /* sety y axis */
    
  char         *button_mapping;   /* set button mapping */
  int          x_threshold;
  int          y_threshold;
  
  /* filled in from HID utils after setup */
  pRecDevice   device;
  
  pRecElement  x_axis;
  pRecElement  y_axis;
  
  pRecElement  fire_button;
  pRecElement  alt_fire_button;
  pRecElement  left_button;
  pRecElement  right_button;
  pRecElement  up_button;
  pRecElement  down_button;
  
  /* fill list of all buttons and axis */
  int          num_buttons;
  pRecElement  buttons[JOYSTICK_DESCRIPTOR_MAX_BUTTONS];
  int          num_axis;
  pRecElement  axis[JOYSTICK_DESCRIPTOR_MAX_AXIS];
  
  /* calibration */
  calibration_t x_calib;
  calibration_t y_calib;
};
typedef struct joystick_descriptor joystick_descriptor_t;

/* access HID joystick A,B */
extern joystick_descriptor_t joy_a;
extern joystick_descriptor_t joy_b;

/* virtual joystick mapping */ 
extern int joystick_port_map[2];

#define JOYDEV_NONE         0
#define JOYDEV_NUMPAD       1
#define JOYDEV_KEYSET1      2
#define JOYDEV_KEYSET2      3
#define JOYDEV_HID_0        4
#define JOYDEV_HID_1        5

/* functions */
extern int joy_arch_init(void);
extern void joystick_close(void);
extern void joystick(void);

/* readjust axis mapping of joystick after change of x/y_axis_name or threshold */
extern void setup_axis_mapping(joystick_descriptor_t *joy);
/* readjust button mapping of joystick after change of button_mapping */
extern void setup_button_mapping(joystick_descriptor_t *joy);
/* readjust joystick setup after change of query_vid_pid and/or query_serial */
extern void assign_joysticks_from_device_list(void);
/* reload device list */
extern void reload_device_list(void);

#endif

#endif

