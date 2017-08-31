/*
 * uistatusbar.c - Native GTK3 UI statusbar stuff.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Michael C. Martin <mcmartin@gmail.com>
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
#include <gtk/gtk.h>

#include "not_implemented.h"

#include "drive.h"
#include "joyport.h"
#include "lib.h"
#include "types.h"
#include "uiapi.h"

#include "uistatusbar.h"

/*
 * Counters for warnings issued, the below counters keep track of warnings
 * that get issued many times due to vsync and other events.
 *
 * These vars are used in NOT_IMPLEMENTED_WARN_X_TIMES() macro calls.
 */
static int tape_status_msgs = 0;
static int tape_counter_msgs = 0;
static int tape_motor_msgs = 0;

#define MAX_STATUS_BARS 3

/* Global data that custom status bar widgets base their rendering
 * on. */
static struct ui_sb_state_s {
    /* TODO: does not cover two-unit drives */
    int drive_led_types[DRIVE_NUM];
    unsigned int current_drive_leds[DRIVE_NUM][2];
    int current_joyports[JOYPORT_MAX_PORTS];
} sb_state;

typedef struct ui_statusbar_s {
    GtkWidget *bar;
    GtkLabel *msg;
    /* TODO: Tape */
    GtkWidget *joysticks;
    GtkWidget *drives[DRIVE_NUM];
} ui_statusbar_t;

static ui_statusbar_t allocated_bars[MAX_STATUS_BARS];

void ui_statusbar_init(void)
{
    int i, j;

    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        allocated_bars[i].bar = NULL;
        allocated_bars[i].msg = NULL;
        allocated_bars[i].joysticks = NULL;
        for (j = 0; j < DRIVE_NUM; ++j) {
            allocated_bars[i].drives[j] = NULL;
        }
    }

    for (i = 0; i < DRIVE_NUM; ++i) {
        sb_state.drive_led_types[i] = 0;
        sb_state.current_drive_leds[i][0] = 0;
        sb_state.current_drive_leds[i][1] = 0;
    }

    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        sb_state.current_joyports[i] = 0;
    }
}

void ui_statusbar_shutdown(void)
{
    /* Any universal resources we allocate get cleaned up here */
}

static gboolean draw_drive_led_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int width, height, drive, i;
    double red = 0.0, green = 0.0, x, y, w, h;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    drive = *((int *)data);
    for (i = 0; i < 2; ++i) {
        int led_color = sb_state.drive_led_types[drive] & (1 << i);
        if (led_color) {
            green += sb_state.current_drive_leds[drive][i] / 1000.0;
        } else {
            red += sb_state.current_drive_leds[drive][i] / 1000.0;
        }
    }
    /* Cairo clamps these for us */
    cairo_set_source_rgb(cr, red, green, 0);
    /* LED is half text height and aims for a 2x1 aspect ratio */
    h = height / 2.0;
    w = 2.0 * h;
    x = (width / 2.0) - h;
    y = height / 4.0;
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    return FALSE;
}

static gboolean draw_joyport_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int width, height, val;
    int *port;
    double e, s, x, y;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    port = ((int *)data);
    val = port ? sb_state.current_joyports[*port] : 0;

    /* This widget "wants" to draw 6x6 squares inside a 20x20
     * space. We compute x and y offsets for a scaled square within
     * the real widget space, and then the actual widths for a square
     * edge (e) and the spaces between them (s). */

    if (width > height) {
        s = height / 20.0;
        x = (width - height) / 2.0;
        y = 0.0;
    } else {
        s = width / 20.0;
        y = (height - width) / 2.0;
        x = 0.0;
    }
    e = s * 5.0;

    /* Then we render the five squares. This seems like it could be
     * done more programatically, but enough changes each iteration
     * that we might as well unroll it. */

    /* Up: Bit 0 */
    cairo_set_source_rgb(cr, 0, (val&0x01) ? 1 : 0, 0);
    cairo_rectangle(cr, x + e + 2*s, y+s, e, e);
    cairo_fill(cr);
    /* Down: Bit 1 */
    cairo_set_source_rgb(cr, 0, (val&0x02) ? 1 : 0, 0);
    cairo_rectangle(cr, x + e + 2*s, y + 2*e + 3*s, e, e);
    cairo_fill(cr);
    /* Left: Bit 2 */
    cairo_set_source_rgb(cr, 0, (val&0x04) ? 1 : 0, 0);
    cairo_rectangle(cr, x + s, y + e + 2*s, e, e);
    cairo_fill(cr);
    /* Right: Bit 3 */
    cairo_set_source_rgb(cr, 0, (val&0x08) ? 1 : 0, 0);
    cairo_rectangle(cr, x + 2*e + 3*s, y + e + 2*s, e, e);
    cairo_fill(cr);
    /* Fire buttons: Bits 4-6. Each of the three notional fire buttons
     * controls the red, green, or blue color of the fire button
     * area. By default, we are using one-button joysticks and so this
     * region will be either black or red. */
    cairo_set_source_rgb(cr, (val&0x10) ? 1 : 0,
                             (val&0x20) ? 1 : 0,
                             (val&0x40) ? 1 : 0);
    cairo_rectangle(cr, x + e + 2*s, y + e + 2*s, e, e);
    cairo_fill(cr);
    return FALSE;
}

static void destroy_ancillary_index_cb(GtkWidget *drive, gpointer data)
{
    lib_free(data);
}

static GtkWidget *ui_drive_widget_create(int unit)
{
    GtkWidget *grid, *number, *track, *led;
    char drive_id[4];
    int *drive_index;

    grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_HORIZONTAL);
    snprintf(drive_id, 4, "%d:", unit+8);
    drive_id[3]=0;
    number = gtk_label_new(drive_id);
    track = gtk_label_new("18.5");
    led = gtk_drawing_area_new();
    gtk_widget_set_size_request(led, 30, 15);
    gtk_container_add(GTK_CONTAINER(grid), number);
    gtk_container_add(GTK_CONTAINER(grid), track);
    gtk_container_add(GTK_CONTAINER(grid), led);
    drive_index = lib_malloc(sizeof(int));
    *drive_index = unit;
    g_signal_connect(led, "destroy", G_CALLBACK(destroy_ancillary_index_cb), drive_index);
    g_signal_connect(led, "draw", G_CALLBACK(draw_drive_led_cb), drive_index);
    return grid;
}

static GtkWidget *ui_joystick_widget_create(void)
{
    GtkWidget *grid, *label;
    int i;
    grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_HORIZONTAL);
    label = gtk_label_new(_("Joysticks:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_container_add(GTK_CONTAINER(grid), label);
    /* TODO: At some point, it should be possible for the core to
     *       indicate how many joyports are going to actually be valid
     *       and worth displaying. At the moment, however, we merely
     *       assume "just ports 1 and 2". */
    for (i = 0; i < 2; ++i) {
        GtkWidget *joyport = gtk_drawing_area_new();
        int *port_index;
        gtk_widget_set_size_request(joyport,20,20);
        gtk_container_add(GTK_CONTAINER(grid), joyport);
        port_index = lib_malloc(sizeof(int));
        *port_index = i;
        g_signal_connect(joyport, "destroy", G_CALLBACK(destroy_ancillary_index_cb), port_index);
        g_signal_connect(joyport, "draw", G_CALLBACK(draw_joyport_cb), port_index);
    }
    return grid;
}

static void destroy_statusbar_cb(GtkWidget *sb, gpointer ignored)
{
    int i, j;

    /* TODO: Unreference all widgets to allow collection. This work
     *       must be done in concert with the changes to
     *       ui_statusbar_create(). */
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar == sb) {
            allocated_bars[i].bar = NULL;
            allocated_bars[i].msg = NULL;
            for (j = 0; j < DRIVE_NUM; ++j) {
                allocated_bars[i].drives[j] = NULL;
            }
        }
    }
}

GtkWidget *ui_statusbar_create(void)
{
    GtkWidget *sb, *msg, *joysticks;
    int i, j;

    TEMPORARY_IMPLEMENTATION();

    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar == NULL) {
            break;
        }
    }

    if (i >= MAX_STATUS_BARS) {
        /* Fatal error? */
        return NULL;
    }

    /* TODO: Create all elements independently and keep them
     *       referenced by this code, even when not part of any
     *       immediate display. Procedurally lay out all active
     *       devices based on system status. */
    sb = gtk_grid_new();
    /* First column: messages */
    msg = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(msg), 0.0);
    gtk_widget_set_hexpand(msg, TRUE);
    g_signal_connect(sb, "destroy", G_CALLBACK(destroy_statusbar_cb), NULL);
    allocated_bars[i].bar = sb;
    allocated_bars[i].msg = GTK_LABEL(msg);
    gtk_grid_attach(GTK_GRID(sb), msg, 0, 0, 1, 2);
    /* Second column: Tape and joysticks */
    /* TODO: tape */
    gtk_grid_attach(GTK_GRID(sb), gtk_separator_new(GTK_ORIENTATION_VERTICAL), 1, 0, 1, 2);
    joysticks = ui_joystick_widget_create();
    gtk_grid_attach(GTK_GRID(sb), joysticks, 2, 1, 1, 1);
    allocated_bars[i].joysticks = joysticks;
    /* Third column on: Drives */
    /* TODO: Only the ones that exist! */
    for (j = 0; j < DRIVE_NUM; ++j) {
        GtkWidget *drive = ui_drive_widget_create(j);
        int row = j % 2;
        int column = (j / 2) * 2 + 4;
        if (row == 0) {
            gtk_grid_attach(GTK_GRID(sb), gtk_separator_new(GTK_ORIENTATION_VERTICAL), column-1, 0, 1, 2);
        }
        gtk_grid_attach(GTK_GRID(sb), drive, column, row, 1, 1);
        allocated_bars[i].drives[j] = drive;
    }

    return sb;
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void ui_display_playback(int playback_status, char *version)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void ui_display_recording(int recording_status)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void ui_display_statustext(const char *text, int fade_out)
{
    /* TODO: handle fade out properly. This may work better with a
     *       GtkStatusBar and its per-message revocation methods, but
     *       based on other implementations, we're much better off
     *       pushing graphical updates to another function and using
     *       this to set up a timer fed by the speed display */
    int i;
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].msg) {
            gtk_label_set_text(allocated_bars[i].msg, text);
        }
    }

    TEMPORARY_IMPLEMENTATION();
}

void ui_display_volume(int vol)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void ui_display_joyport(uint8_t *joyport)
{
    int i;
    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        /* Compare the new value to the current one, set the new
         * value, and queue a redraw if and only if there was a
         * change. And yes, the input joystick ports are 1-indexed. I
         * don't know either. */
        if (sb_state.current_joyports[i] != joyport[i+1]) {
            int j;
            sb_state.current_joyports[i] = joyport[i+1];
            for (j = 0; j < MAX_STATUS_BARS; ++j) {
                if (allocated_bars[j].joysticks) {
                    GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(allocated_bars[j].joysticks), i+1, 0);
                    if (widget) {
                        gtk_widget_queue_draw(widget);
                    }
                }
            }
        }
    }
}

/* TODO: status display for TAPE emulation
 *
 * NOTE: newly written GUI code should be able to handle TWO independent tape
 *       drives. the PET emulation may make use of it some day.
 */

void ui_display_tape_control_status(int control)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void ui_display_tape_counter(int counter)
{
    NOT_IMPLEMENTED_WARN_X_TIMES(tape_counter_msgs, 3);
}

void ui_display_tape_motor_status(int motor)
{
    NOT_IMPLEMENTED_WARN_X_TIMES(tape_motor_msgs, 3);
}

void ui_set_tape_status(int tape_status)
{
    NOT_IMPLEMENTED_WARN_X_TIMES(tape_status_msgs, 3);
}

void ui_display_tape_current_image(const char *image)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

/* TODO: status display for DRIVE emulation
 *
 * NOTE: newly written GUI code should be able to use 4 drives, of which each
 *       can be a dual disk drive. (so it must handle 8 images total). currently
 *       the code does not make use of it yet, but it will in the future.
 */
void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int led_pwm2)
{
    int i;
    if (drive_number < 0 || drive_number > DRIVE_NUM-1) {
        /* TODO: Fatal error? */
        return;
    }
    sb_state.current_drive_leds[drive_number][0] = pwm1;
    sb_state.current_drive_leds[drive_number][1] = led_pwm2;
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar) {
            GtkWidget *drive, *led;
            drive = allocated_bars[i].drives[i];
            led = gtk_grid_get_child_at(GTK_GRID(drive), 2, 0);
            if (led) {
                gtk_widget_queue_draw(led);
            }
        }
    }
}

void ui_display_drive_track(unsigned int drive_number,
                            unsigned int drive_base,
                            unsigned int half_track_number)
{
    int i, unit = drive_base-8;
    if (unit < 0 || unit > DRIVE_NUM-1) {
        /* TODO: Fatal error? */
        return;
    }
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar) {
            GtkWidget *drive, *track;
            drive = allocated_bars[i].drives[i];
            track = gtk_grid_get_child_at(GTK_GRID(drive), 1, 0);
            if (track) {
                char track_str[16];
                snprintf(track_str, 16, "%.1lf", half_track_number / 2.0);
                track_str[15] = 0;
                gtk_label_set_text(GTK_LABEL(track), track_str);
            }
        }
    }
}

void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color)
{
    int i;

    for (i = 0; i < DRIVE_NUM; ++i) {
        if (state & 1) {
            sb_state.drive_led_types[i] = drive_led_color[i];
            sb_state.current_drive_leds[i][0] = 0;
            sb_state.current_drive_leds[i][1] = 0;
        }
        state >>= 1;
    }
    TEMPORARY_IMPLEMENTATION();
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
    char buf[256];
    snprintf(buf, 256, _("Attached %s to unit %d"), image, drive_number+8);
    buf[255]=0;
    ui_display_statustext(buf, 1);
}
