/*
 * joystick.h - Common joystick emulation.
 *
 * Written by
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

#ifndef _JOYSTICK_H
#define _JOYSTICK_H

#include "types.h"

struct snapshot_s;

extern int joystick_init(void);
extern int joystick_init_resources(void);

extern int joystick_kbd_update_set(signed long key);
extern int joystick_kbd_update_clr(signed long key);
extern void joystick_joypad_clear(void);

extern void joystick_set_value_absolute(unsigned int joyport, BYTE value);
extern void joystick_set_value_or(unsigned int joyport, BYTE value);
extern void joystick_set_value_and(unsigned int joyport, BYTE value);
extern void joystick_clear(unsigned int joyport);
extern void joystick_clear_all(void);

extern void joystick_event_playback(CLOCK offset, void *data);
extern void joystick_event_delayed_playback(void *data);
extern void joystick_register_delay(unsigned int delay);

extern int joystick_snapshot_write_module(struct snapshot_s *s);
extern int joystick_snapshot_read_module(struct snapshot_s *s);

typedef struct joy_device_s {
    void (*poll)   (int port, void* dev);
    int  (*open)   (void* dev);
    void (*close)  (void* dev);
    void (*destroy)(void* dev);
} joy_device_t;

typedef struct joy_data_s {
    char* name;
    void* priv;
    joy_device_t* device;
} joy_data_t;

#define MAX_HW_JOY_DRIVERS 16

#define JOYDEV_NONE         0
#define JOYDEV_NUMPAD       1
#define JOYDEV_KEYSET1      2
#define JOYDEV_KEYSET2      3
#define JOYDEV_HW_BASE      4

extern void joystick_register_device(joy_data_t* device);
extern unsigned int joystick_device_num(void);
extern const char* joystick_device_name(unsigned int i);
extern void joystick_close(void);
extern void joystick_update(void);
extern int joystick_kbd_update_set(signed long key);
extern int joystick_kbd_update_clr(signed long key);

#endif

