/*
 * ted.c
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include <stdlib.h>
#include <string.h>

#include "alarm.h"
#include "clkguard.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "palette.h"
#include "plus4.h"
#include "plus4mem.h"
#include "raster-modes.h"
#include "resources.h"
#include "screenshot.h"
#include "snapshot.h"
#include "ted-cmdline-options.h"
#include "ted-color.h"
#include "ted-draw.h"
#include "ted-mem.h"
#include "ted-resources.h"
#include "ted-snapshot.h"
#include "ted-sound.h"
#include "ted-timer.h"
#include "ted.h"
#include "tedtypes.h"
#include "types.h"
#include "utils.h"
#include "vsync.h"
#ifdef __MSDOS__
#include "videoarch.h"
#endif
#include "video.h"
#ifdef USE_XF86_EXTENSIONS
#include "fullscreen.h"
#endif


/* FIXME: PAL/NTSC constants should be moved from drive.h */
#define DRIVE_SYNC_PAL     -1
#define DRIVE_SYNC_NTSC    -2
#define DRIVE_SYNC_NTSCOLD -3


ted_t ted;

static void ted_raster_irq_alarm_handler(CLOCK offset);
static void ted_exposure_handler(unsigned int width, unsigned int height);

static void clk_overflow_callback(CLOCK sub, void *unused_data)
{
    ted.raster_irq_clk -= sub;
    ted.last_emulate_line_clk -= sub;
    ted.fetch_clk -= sub;
    ted.draw_clk -= sub;
}

static void ted_change_timing(void)
{
    resource_value_t mode;

    resources_get_value("VideoStandard", &mode);

    switch ((int)mode) {
      case DRIVE_SYNC_NTSC:
        clk_guard_set_clk_base(&maincpu_clk_guard, PLUS4_NTSC_CYCLES_PER_RFSH);
        ted.screen_height = TED_NTSC_SCREEN_HEIGHT;
        ted.first_displayed_line = TED_NTSC_FIRST_DISPLAYED_LINE;
        ted.last_displayed_line = TED_NTSC_LAST_DISPLAYED_LINE;
        ted.row_25_start_line = TED_NTSC_25ROW_START_LINE;
        ted.row_25_stop_line = TED_NTSC_25ROW_STOP_LINE;
        ted.row_24_start_line = TED_NTSC_24ROW_START_LINE;
        ted.row_24_stop_line = TED_NTSC_24ROW_STOP_LINE;
        ted.screen_borderwidth = TED_SCREEN_NTSC_BORDERWIDTH;
        ted.screen_borderheight = TED_SCREEN_NTSC_BORDERHEIGHT;
        ted.cycles_per_line = TED_NTSC_CYCLES_PER_LINE;
        ted.draw_cycle = TED_NTSC_DRAW_CYCLE;
        ted.first_dma_line = TED_NTSC_FIRST_DMA_LINE;
        ted.last_dma_line = TED_NTSC_LAST_DMA_LINE;
        ted.offset = TED_NTSC_OFFSET;
        break;
      case DRIVE_SYNC_PAL:
      default:
        clk_guard_set_clk_base(&maincpu_clk_guard, PLUS4_PAL_CYCLES_PER_RFSH);
        ted.screen_height = TED_PAL_SCREEN_HEIGHT;
        ted.first_displayed_line = TED_PAL_FIRST_DISPLAYED_LINE;
        ted.last_displayed_line = TED_PAL_LAST_DISPLAYED_LINE;
        ted.row_25_start_line = TED_PAL_25ROW_START_LINE;
        ted.row_25_stop_line = TED_PAL_25ROW_STOP_LINE;
        ted.row_24_start_line = TED_PAL_24ROW_START_LINE;
        ted.row_24_stop_line = TED_PAL_24ROW_STOP_LINE;
        ted.screen_borderwidth = TED_SCREEN_PAL_BORDERWIDTH;
        ted.screen_borderheight = TED_SCREEN_PAL_BORDERHEIGHT;
        ted.cycles_per_line = TED_PAL_CYCLES_PER_LINE;
        ted.draw_cycle = TED_PAL_DRAW_CYCLE;
        ted.first_dma_line = TED_PAL_FIRST_DMA_LINE;
        ted.last_dma_line = TED_PAL_LAST_DMA_LINE;
        ted.offset = TED_PAL_OFFSET;
        break;
    }
}

inline void ted_handle_pending_alarms(int num_write_cycles)
{
    if (num_write_cycles != 0) {
        int f;

        /* Cycles can be stolen only during the read accesses, so we serve
           only the events that happened during them.  The last read access
           happened at `clk - maincpu_write_cycles()' as all the opcodes
           except BRK and JSR do all the write accesses at the very end.  BRK
           cannot take us here and we would not be able to handle JSR
           correctly anyway, so we don't care about them...  */

        /* Go back to the time when the read accesses happened and serve TED
           events.  */
        clk -= num_write_cycles;

        do {
            f = 0;
            if (clk > ted.fetch_clk) {
               ted_raster_fetch_alarm_handler(0);
               f = 1;
            }
            if (clk >= ted.draw_clk) {
                ted_raster_draw_alarm_handler((long)(clk - ted.draw_clk));
                f = 1;
            }
        }
        while (f);

        /* Go forward to the time when the last write access happens (that's
          the one we care about, as the only instructions that do two write
           accesses - except BRK and JSR - are the RMW ones, which store the
           old value in the first write access, and then store the new one in
           the second write access).  */
        clk += num_write_cycles;

      } else {
        int f;

        do {
            f = 0;
            if (clk >= ted.fetch_clk) {
                ted_raster_fetch_alarm_handler(0);
                f = 1;
            }
            if (clk >= ted.draw_clk) {
                ted_raster_draw_alarm_handler(0);
                f = 1;
            }
        }
        while (f);
    }
}


static void ted_set_geometry(void)
{
    unsigned int width, height;

    width = TED_SCREEN_XPIX + ted.screen_borderwidth * 2;
    height = ted.last_displayed_line - ted.first_displayed_line + 1;
#ifdef VIC_II_NEED_2X
#ifdef USE_XF86_EXTENSIONS
    if (fullscreen_is_enabled
        ? ted_resources.fullscreen_double_size_enabled
        : ted_resources.double_size_enabled)
#else
    if (ted_resources.double_size_enabled)
#endif
    {
        width *= 2;
        height *= 2;
        raster_set_pixel_size(&ted.raster, 2, 2, VIDEO_RENDER_PAL_2X2);
    }
#endif

    raster_set_geometry(&ted.raster,
                        TED_SCREEN_WIDTH, ted.screen_height,
                        TED_SCREEN_XPIX, TED_SCREEN_YPIX,
                        TED_SCREEN_TEXTCOLS, TED_SCREEN_TEXTLINES,
                        ted.screen_borderwidth, ted.row_25_start_line,
                        0,
                        ted.first_displayed_line,
                        ted.last_displayed_line,
                        0);
    raster_resize_viewport(&ted.raster, width, height);

#ifdef __MSDOS__
    video_ack_vga_mode();
#endif

}

static int init_raster(void)
{
    raster_t *raster;
    char *title;

    raster = &ted.raster;
    video_color_set_raster(raster);

    if (raster_init(raster, TED_NUM_VMODES, 0) < 0)
        return -1;
    raster_modes_set_idle_mode(raster->modes, TED_IDLE_MODE);
    raster_set_exposure_handler(raster, (void*)ted_exposure_handler);
    raster_enable_cache(raster, ted_resources.video_cache_enabled);
#ifdef VIC_II_NEED_2X
#ifdef USE_XF86_EXTENSIONS
    raster_enable_double_scan(raster, fullscreen_is_enabled
                              ? ted_resources.fullscreen_double_scan_enabled
                              : ted_resources.double_scan_enabled);
#else
    raster_enable_double_scan(raster, ted_resources.double_scan_enabled);
#endif
#else
    raster_enable_double_scan(raster, 0);
#endif
    raster_set_canvas_refresh(raster, 1);

    ted_set_geometry();

    if (ted_update_palette() < 0) {
        log_error(ted.log, "Cannot load palette.");
        return -1;
    }
    title = concat("VICE: ", machine_name, " emulator", NULL);
    raster_set_title(raster, title);
    free (title);

    if (raster_realize(raster) < 0)
        return -1;

    raster->display_ystart = ted.row_25_start_line;
    raster->display_ystop = ted.row_25_stop_line;
    raster->display_xstart = TED_40COL_START_PIXEL;
    raster->display_xstop = TED_40COL_STOP_PIXEL;

    return 0;
}

/* Emulate a matrix line fetch, `num' bytes starting from `offs'.  This takes
   care of the 10-bit counter wraparound.  */
inline void ted_fetch_matrix(int offs, int num)
{
    BYTE *p;
    int start_char;
    int c;

    /* Matrix fetches are done during Phi2, the fabulous "bad lines" */
    p = ted.screen_ptr;

    start_char = (ted.mem_counter + offs) & 0x3ff;
    c = 0x3ff - start_char + 1;

    if (c >= num) {
        memcpy(ted.vbuf + offs, p + start_char, num);
    } else {
        memcpy(ted.vbuf + offs, p + start_char, c);
        memcpy(ted.vbuf + offs + c, p, num - c);
    }
}

inline void ted_fetch_color(int offs, int num)
{
    BYTE *p;
    int start_char;
    int c, i;

    /* Matrix fetches are done during Phi2, the fabulous "bad lines" */
    p = ted.screen_ptr;

    start_char = (ted.mem_counter + offs) & 0x3ff;
    c = 0x3ff - start_char + 1;

    if (c >= num) {
        for (i = 0; i < num; i++)
            ted.cbuf[offs + i] = (ted.color_ptr)[start_char + i] & 0xff;
    } else {
        for (i = 0; i < c; i++)
            ted.cbuf[offs + i] = (ted.color_ptr)[start_char + i] & 0xff;
        for (i = 0; i < num - c; i++)
            ted.cbuf[offs + c + i] = (ted.color_ptr)[i] & 0xff;
    }
}

/* If we are on a bad line, do the DMA.  Return nonzero if cycles have been
   stolen.  */
inline static int do_matrix_fetch(CLOCK sub)
{
    if (!ted.memory_fetch_done) {
        raster_t *raster;

        raster = &ted.raster;

        ted.memory_fetch_done = 1;
        ted.mem_counter = ted.memptr;

        if ((raster->current_line & 7) == (unsigned int)raster->ysmooth
            && ted.allow_bad_lines
            && raster->current_line >= ted.first_dma_line
            && raster->current_line <= ted.last_dma_line) {
            ted_fetch_matrix(0, TED_SCREEN_TEXTCOLS);

            raster->draw_idle_state = 0;
            raster->ycounter = 0;

            ted.idle_state = 0;
            ted.idle_data_location = IDLE_NONE;
            ted.ycounter_reset_checked = 1;
            ted.memory_fetch_done = 2;

            maincpu_steal_cycles(ted.fetch_clk,
                                 TED_SCREEN_TEXTCOLS + 3 - sub);

            ted.bad_line = 1;
            return 1;
        }

        if (((raster->current_line + 1) & 7) == (unsigned int)raster->ysmooth
            && ted.allow_bad_lines
            && raster->current_line >= ted.first_dma_line
            && raster->current_line <= ted.last_dma_line) {
            ted_fetch_color(0, TED_SCREEN_TEXTCOLS);
/*
            raster->draw_idle_state = 0;
            raster->ycounter = 0;

            ted.idle_state = 0;
            ted.idle_data_location = IDLE_NONE;
            ted.ycounter_reset_checked = 1;
            ted.memory_fetch_done = 2;
*/
            maincpu_steal_cycles(ted.fetch_clk,
                                 TED_SCREEN_TEXTCOLS + 3 - sub);

            ted.bad_line = 1;
            return 1;
        }

    }

    return 0;
}

/* Initialize the TED emulation.  */
raster_t *ted_init(void)
{
    ted.log = log_open("TED");

    alarm_init(&ted.raster_fetch_alarm, maincpu_alarm_context,
               "TEDRasterFetch", ted_raster_fetch_alarm_handler);
    alarm_init(&ted.raster_draw_alarm, maincpu_alarm_context,
               "TEDRasterDraw", ted_raster_draw_alarm_handler);
    alarm_init(&ted.raster_irq_alarm, maincpu_alarm_context,
               "TEDRasterIrq", ted_raster_irq_alarm_handler);

    ted_change_timing();

    ted_timer_init();

    if (init_raster() < 0)
        return NULL;

    ted_powerup();

    ted_update_video_mode(0);
    ted_update_memory_ptrs(0);

    ted_draw_init();
#ifdef VIC_II_NEED_2X
#ifdef USE_XF86_EXTENSIONS
    ted_draw_set_double_size(fullscreen_is_enabled
                             ? ted_resources.fullscreen_double_size_enabled
                             : ted_resources.double_size_enabled);
#else
    ted_draw_set_double_size(ted_resources.double_size_enabled);
#endif
#else
    ted_draw_set_double_size(0);
#endif

    ted.initialized = 1;

    clk_guard_add_callback(&maincpu_clk_guard, clk_overflow_callback, NULL);

    ted_resize();

    return &ted.raster;
}

struct video_canvas_s *ted_get_canvas(void)
{
    return ted.raster.viewport.canvas;
}

/* Reset the TED chip.  */
void ted_reset(void)
{
    ted_change_timing();

    ted_timer_reset();
    ted_sound_reset();

    raster_reset(&ted.raster);

    ted_set_geometry();

    ted.last_emulate_line_clk = 0;

    ted.draw_clk = ted.draw_cycle;
    alarm_set(&ted.raster_draw_alarm, ted.draw_clk);

    ted.fetch_clk = TED_FETCH_CYCLE;
    alarm_set(&ted.raster_fetch_alarm, ted.fetch_clk);

    /* FIXME: I am not sure this is exact emulation.  */
    ted.raster_irq_line = 0;
    ted.raster_irq_clk = 0;

    /* Setup the raster IRQ alarm.  The value is `1' instead of `0' because we
       are at the first line, which has a +1 clock cycle delay in IRQs.  */
    alarm_set(&ted.raster_irq_alarm, 1);

    ted.force_display_state = 0;

    /* Remove all the IRQ sources.  */
    ted.regs[0x0a] = 0;

    ted.raster.display_ystart = ted.row_25_start_line;
    ted.raster.display_ystop = ted.row_25_stop_line;

    ted.cursor_visible = 0;
    ted.cursor_phase = 0;
}

void ted_reset_registers(void)
{
    ADDRESS i;

    for (i = 0; i <= 0x3f; i++)
        ted_store(i, 0);
}

void ted_powerup(void)
{
    memset(ted.regs, 0, sizeof(ted.regs));

    ted.irq_status = 0;
    ted.raster_irq_line = 0;
    ted.raster_irq_clk = 1;

    ted.allow_bad_lines = 0;
    ted.idle_state = 0;
    ted.force_display_state = 0;
    ted.memory_fetch_done = 0;
    ted.memptr = 0;
    ted.mem_counter = 0;
    ted.mem_counter_inc = 0;
    ted.bad_line = 0;
    ted.ycounter_reset_checked = 0;
    ted.force_black_overscan_background_color = 0;
    ted.idle_data = 0;
    ted.idle_data_location = IDLE_NONE;
    ted.last_emulate_line_clk = 0;

    ted_reset();

    ted.raster_irq_line = 0;

    ted.raster.blank = 1;
    ted.raster.display_ystart = ted.row_24_start_line;
    ted.raster.display_ystop = ted.row_24_stop_line;

    ted.raster.ysmooth = 0;
}

/* ---------------------------------------------------------------------*/

/* Handle the exposure event.  */
static void ted_exposure_handler(unsigned int width, unsigned int height)
{
    raster_resize_viewport(&ted.raster, width, height);
}

/* Make sure all the TED alarms are removed.  This just makes it easier to
   write functions for loading snapshot modules in other video chips without
   caring that the TED alarms are dispatched when they really shouldn't
   be.  */
void ted_prepare_for_snapshot(void)
{
    ted.fetch_clk = CLOCK_MAX;
    alarm_unset(&ted.raster_fetch_alarm);
    ted.draw_clk = CLOCK_MAX;
    alarm_unset(&ted.raster_draw_alarm);
    ted.raster_irq_clk = CLOCK_MAX;
    alarm_unset(&ted.raster_irq_alarm);
}

void ted_set_raster_irq(unsigned int line)
{
    if (line == ted.raster_irq_line && ted.raster_irq_clk != CLOCK_MAX)
        return;

    if (line < ted.screen_height) {
        unsigned int current_line = TED_RASTER_Y(clk);

        ted.raster_irq_clk = (TED_LINE_START_CLK(clk)
                             + TED_RASTER_IRQ_DELAY - INTERRUPT_DELAY
                             + (ted.cycles_per_line
                             * (line - current_line)));

        /* Raster interrupts on line 0 are delayed by 1 cycle.  */
        if (line == 0)
            ted.raster_irq_clk++;

        if (line <= current_line)
            ted.raster_irq_clk += (ted.screen_height
                                  * ted.cycles_per_line);
        alarm_set(&ted.raster_irq_alarm, ted.raster_irq_clk);
    } else {
        TED_DEBUG_RASTER(("TED: update_raster_irq(): "
                         "raster compare out of range ($%04X)!\n",
                         line));
        alarm_unset(&ted.raster_irq_alarm);
    }

    TED_DEBUG_RASTER(("TED: update_raster_irq(): "
                     "ted.raster_irq_clk = %ul, "
                     "line = $%04X, "
                     "ted.regs[0x0a] & 2 = %d\n",
                     ted.raster_irq_clk,
                     line,
                     ted.regs[0x0a] & 2));

    ted.raster_irq_line = line;
}

/* Set the memory pointers according to the values in the registers.  */
void ted_update_memory_ptrs(unsigned int cycle)
{
    /* FIXME: This is *horrible*!  */
    static BYTE *old_screen_ptr, *old_bitmap_ptr, *old_chargen_ptr;
    static BYTE *old_color_ptr;
    ADDRESS screen_addr, char_addr, bitmap_addr, color_addr;
    BYTE *screen_base;            /* Pointer to screen memory.  */
    BYTE *char_base;              /* Pointer to character memory.  */
    BYTE *bitmap_base;            /* Pointer to bitmap memory.  */
    BYTE *color_base;             /* Pointer to color memory.  */
    int tmp;
    unsigned int romsel;

    romsel = ted.regs[0x12] & 4;

    screen_addr = ((ted.regs[0x14] & 0xf8) << 8) | 0x400;
    screen_base = mem_get_tedmem_base((screen_addr >> 14) | romsel)
                  + (screen_addr& 0x3fff);

    TED_DEBUG_REGISTER(("\tVideo memory at $%04X\n", screen_addr));

    bitmap_addr = (ted.regs[0x12] & 0x38) << 10;
    bitmap_base = mem_get_tedmem_base((bitmap_addr >> 14) | romsel)
                  + (bitmap_addr & 0x3fff);

    TED_DEBUG_REGISTER(("\tBitmap memory at $%04X\n", bitmap_addr));

    char_addr = (ted.regs[0x13] & 0xfc) << 8;
    char_base = mem_get_tedmem_base((char_addr >> 14) | romsel)
                + (char_addr & 0x3fff);

    TED_DEBUG_REGISTER(("\tUser-defined character set at $%04X\n", char_addr));

    color_addr = ((ted.regs[0x14] & 0xf8) << 8);
    color_base = mem_get_tedmem_base((color_addr >> 14) | romsel)
                 + (color_addr & 0x3fff);

    TED_DEBUG_REGISTER(("\tColor memory at $%04X\n", color_addr));


    tmp = TED_RASTER_CHAR(cycle);

    if (ted.idle_data_location != IDLE_NONE) {
        if (ted.idle_data_location == IDLE_39FF)
            raster_add_int_change_foreground(&ted.raster,
                                             TED_RASTER_CHAR(cycle),
                                             &ted.idle_data,
                                             ram[0x39ff]);
        else
            raster_add_int_change_foreground(&ted.raster,
                                             TED_RASTER_CHAR(cycle),
                                             &ted.idle_data,
                                             ram[0x3fff]);
    }

    if (ted.raster.skip_frame || (tmp <= 0 && clk < ted.draw_clk)) {
        old_screen_ptr = ted.screen_ptr = screen_base;
        old_bitmap_ptr = ted.bitmap_ptr = bitmap_base;
        old_chargen_ptr = ted.chargen_ptr = char_base;
        old_color_ptr = ted.color_ptr = color_base;
    } else if (tmp < TED_SCREEN_TEXTCOLS) {
        if (screen_base != old_screen_ptr) {
            raster_add_ptr_change_foreground(&ted.raster, tmp,
                                             (void **)&ted.screen_ptr,
                                             (void *)screen_base);
            old_screen_ptr = screen_base;
        }

        if (bitmap_base != old_bitmap_ptr) {
            raster_add_ptr_change_foreground(&ted.raster,
                                             tmp,
                                             (void **)&ted.bitmap_ptr,
                                             (void *)(bitmap_base));
            old_bitmap_ptr = bitmap_base;
        }

        if (char_base != old_chargen_ptr) {
            raster_add_ptr_change_foreground(&ted.raster,
                                             tmp,
                                             (void **)&ted.chargen_ptr,
                                             (void *)char_base);
            old_chargen_ptr = char_base;
        }
        if (color_base != old_color_ptr) {
            raster_add_ptr_change_foreground(&ted.raster, tmp,
                                             (void **)&ted.color_ptr,
                                             (void *)color_base);
            old_color_ptr = color_base;
        }
    } else {
        if (screen_base != old_screen_ptr) {
            raster_add_ptr_change_next_line(&ted.raster,
                                            (void **)&ted.screen_ptr,
                                            (void *)screen_base);
            old_screen_ptr = screen_base;
        }
        if (bitmap_base != old_bitmap_ptr) {
            raster_add_ptr_change_next_line(&ted.raster,
                                            (void **)&ted.bitmap_ptr,
                                            (void *)(bitmap_base));
            old_bitmap_ptr = bitmap_base;
        }

        if (char_base != old_chargen_ptr) {
            raster_add_ptr_change_next_line(&ted.raster,
                                            (void **)&ted.chargen_ptr,
                                            (void *)char_base);
            old_chargen_ptr = char_base;
        }
        if (color_base != old_color_ptr) {
            raster_add_ptr_change_next_line(&ted.raster,
                                            (void **)&ted.color_ptr,
                                            (void *)color_base);
            old_color_ptr = color_base;
        }
    }
}

/* Set the video mode according to the values in registers 6 and 7 of TED */
void ted_update_video_mode(unsigned int cycle)
{
   static int old_video_mode = -1;
    int new_video_mode;

    new_video_mode = ((ted.regs[0x06] & 0x60)
                     | (ted.regs[0x07] & 0x10)) >> 4;

    if (new_video_mode != old_video_mode) {
        if (TED_IS_ILLEGAL_MODE(new_video_mode)) {
            /* Force the overscan color to black.  */
            raster_add_int_change_background
                (&ted.raster, TED_RASTER_X(cycle),
                &ted.raster.overscan_background_color,
                0);
            ted.force_black_overscan_background_color = 1;
        } else {
            /* The overscan background color is given by the background color
               register.  */
            if (ted.raster.overscan_background_color != ted.regs[0x15])
                raster_add_int_change_background
                    (&ted.raster, TED_RASTER_X(cycle),
                    &ted.raster.overscan_background_color,
                    ted.regs[0x15]);
            ted.force_black_overscan_background_color = 0;
        }

        {
            int pos;

            pos = TED_RASTER_CHAR(cycle);

            raster_add_int_change_foreground(&ted.raster, pos,
                                             &ted.raster.video_mode,
                                             new_video_mode);

            if (ted.idle_data_location != IDLE_NONE) {
                if (ted.regs[0x06] & 0x40)
                    raster_add_int_change_foreground
                        (&ted.raster, pos, (void *)&ted.idle_data,
                        ram[0x39ff]);
                else
                    raster_add_int_change_foreground
                        (&ted.raster, pos, (void *)&ted.idle_data,
                        ram[0x3fff]);
            }
        }

        old_video_mode = new_video_mode;
    }

#ifdef TED_VMODE_DEBUG
    switch (new_video_mode) {
      case TED_NORMAL_TEXT_MODE:
        TED_DEBUG_VMODE(("Standard Text"));
        break;
      case TED_MULTICOLOR_TEXT_MODE:
        TED_DEBUG_VMODE(("Multicolor Text"));
        break;
      case TED_HIRES_BITMAP_MODE:
        TED_DEBUG_VMODE(("Hires Bitmap"));
        break;
      case TED_MULTICOLOR_BITMAP_MODE:
        TED_DEBUG_VMODE(("Multicolor Bitmap"));
        break;
      case TED_EXTENDED_TEXT_MODE:
        TED_DEBUG_VMODE(("Extended Text"));
        break;
      case TED_ILLEGAL_TEXT_MODE:
        TED_DEBUG_VMODE(("Illegal Text"));
        break;
      case TED_ILLEGAL_BITMAP_MODE_1:
        TED_DEBUG_VMODE(("Invalid Bitmap"));
        break;
      case TED_ILLEGAL_BITMAP_MODE_2:
        TED_DEBUG_VMODE(("Invalid Bitmap"));
        break;
      default:                    /* cannot happen */
        TED_DEBUG_VMODE(("???"));
    }

    TED_DEBUG_VMODE((" Mode enabled at line $%04X, cycle %d.\n",
                    TED_RASTER_Y(clk), cycle));
#endif
}

/* Redraw the current raster line.  This happens at cycle TED_DRAW_CYCLE
   of each line.  */
void ted_raster_draw_alarm_handler(CLOCK offset)
{
    int in_visible_area;


    in_visible_area = (ted.raster.current_line >= ted.first_displayed_line
                      && ted.raster.current_line <= ted.last_displayed_line);

    raster_emulate_line(&ted.raster);

    if (ted.raster.current_line == 0) {
        raster_skip_frame(&ted.raster,
                          vsync_do_vsync(ted.raster.skip_frame));
        ted.memptr = 0;
        ted.mem_counter = 0;

        ted.cursor_phase = (ted.cursor_phase + 1) & 0x1f;
        ted.cursor_visible = ted.cursor_phase & 0x10;

#ifdef __MSDOS__
        if (ted.raster.viewport.width <= TED_SCREEN_XPIX
            && ted.raster.viewport.height <= TED_SCREEN_YPIX
            && ted.raster.viewport.update_canvas)
            canvas_set_border_color(ted.raster.viewport.canvas,
                                    ted.raster.border_color);
#endif
    }

    if (in_visible_area) {
        if (!ted.idle_state)
            ted.mem_counter = (ted.mem_counter
                              + ted.mem_counter_inc) & 0x3ff;
        ted.mem_counter_inc = TED_SCREEN_TEXTCOLS;
        /* `ycounter' makes the chip go to idle state when it reaches the
           maximum value.  */
        if (ted.raster.ycounter == 7) {
            ted.idle_state = 1;
            ted.memptr = ted.mem_counter;
        }
        if (!ted.idle_state || ted.bad_line) {
            ted.raster.ycounter = (ted.raster.ycounter + 1) & 0x7;
            ted.idle_state = 0;
        }
        if (ted.force_display_state) {
            ted.idle_state = 0;
            ted.force_display_state = 0;
        }
        ted.raster.draw_idle_state = ted.idle_state;
        ted.bad_line = 0;
    }

    ted.ycounter_reset_checked = 0;
    ted.memory_fetch_done = 0;

    if (ted.raster.current_line == ted.first_dma_line)
        ted.allow_bad_lines = !ted.raster.blank;

    if (ted.idle_state) {
        if (ted.regs[0x6] & 0x40) {
            ted.idle_data_location = IDLE_39FF;
            ted.idle_data = ram[0x39ff];
        } else {
            ted.idle_data_location = IDLE_3FFF;
            ted.idle_data = ram[0x3fff];
        }
    } else
      ted.idle_data_location = IDLE_NONE;

    /* Set the next draw event.  */
    ted.last_emulate_line_clk += ted.cycles_per_line;
    ted.draw_clk = ted.last_emulate_line_clk + ted.draw_cycle;
    alarm_set(&ted.raster_draw_alarm, ted.draw_clk);
}

inline static void handle_fetch_matrix(long offset, CLOCK sub,
                                       CLOCK *write_offset)
{
    raster_t *raster;

    *write_offset = 0;

    raster = &ted.raster;

    do_matrix_fetch(sub);

    if (raster->current_line < ted.first_dma_line) {
        ted.fetch_clk += ((ted.first_dma_line
                         - raster->current_line)
                         * ted.cycles_per_line);
    } else {
        if (raster->current_line >= ted.last_dma_line)
            ted.fetch_clk += ((ted.screen_height
                             - raster->current_line
                             + ted.first_dma_line)
                             * ted.cycles_per_line);
        else
            ted.fetch_clk += ted.cycles_per_line;
    }

    alarm_set(&ted.raster_fetch_alarm, ted.fetch_clk);

    return;
}

/* Handle matrix fetch events.  FIXME: could be made slightly faster.  */
void ted_raster_fetch_alarm_handler(CLOCK offset)
{
     CLOCK last_opcode_first_write_clk, last_opcode_last_write_clk;

    /* This kludgy thing is used to emulate the behavior of the 6510 when BA
       goes low.  When BA goes low, every read access stops the processor
       until BA is high again; write accesses happen as usual instead.  */

    if (offset > 0) {
        switch (OPINFO_NUMBER(last_opcode_info)) {
          case 0:
            /* In BRK, IRQ and NMI the 3rd, 4th and 5th cycles are write
               accesses, while the 1st, 2nd, 6th and 7th are read accesses.  */
            last_opcode_first_write_clk = clk - 5;
            last_opcode_last_write_clk = clk - 3;
            break;

          case 0x20:
            /* In JSR, the 4th and 5th cycles are write accesses, while the
               1st, 2nd, 3rd and 6th are read accesses.  */
            last_opcode_first_write_clk = clk - 3;
            last_opcode_last_write_clk = clk - 2;
            break;

          default:
            /* In all the other opcodes, all the write accesses are the last
               ones.  */
            if (maincpu_num_write_cycles() != 0) {
                last_opcode_last_write_clk = clk - 1;
                last_opcode_first_write_clk = clk - maincpu_num_write_cycles();
            } else {
                last_opcode_first_write_clk = (CLOCK) 0;
                last_opcode_last_write_clk = last_opcode_first_write_clk;
            }
            break;
        }
    } else { /* offset <= 0, i.e. offset == 0 */
        /* If we are called with no offset, we don't have to care about write
           accesses.  */
        last_opcode_first_write_clk = last_opcode_last_write_clk = 0;
    }

    {
        CLOCK sub;
        CLOCK write_offset;

        if (ted.fetch_clk < last_opcode_first_write_clk
            || ted.fetch_clk > last_opcode_last_write_clk)
            sub = 0;
        else
            sub = last_opcode_last_write_clk - ted.fetch_clk + 1;

        handle_fetch_matrix(offset, sub, &write_offset);
        last_opcode_first_write_clk += write_offset;
        last_opcode_last_write_clk += write_offset;
    }
}

/* If necessary, emulate a raster compare IRQ. This is called when the raster
   line counter matches the value stored in the raster line register.  */
static void ted_raster_irq_alarm_handler(CLOCK offset)
{
    ted.irq_status |= 0x2;
    if (ted.regs[0x0a] & 0x2) {
        maincpu_set_irq_clk(I_RASTER, 1, ted.raster_irq_clk);
        ted.irq_status |= 0x80;
        TED_DEBUG_RASTER(("TED: *** IRQ requested at line $%04X, "
                         "ted.raster_irq_line=$%04X, offset = %ld, cycle = %d.\n",
                         TED_RASTER_Y(clk), ted.raster_irq_line, offset,
                         TED_RASTER_CYCLE(clk)));
    }

    ted.raster_irq_clk += ted.screen_height * ted.cycles_per_line;
    alarm_set(&ted.raster_irq_alarm, ted.raster_irq_clk);
}


/* Set proper functions and constants for the current video settings.  */
void ted_resize(void)
{
    if (!ted.initialized)
        return;

#ifdef USE_XF86_EXTENSIONS
    if (!fullscreen_is_enabled)
#endif
        raster_enable_double_size(&ted.raster,
                                  ted_resources.double_size_enabled,
                                  ted_resources.double_size_enabled);

#ifdef VIC_II_NEED_2X
#ifdef USE_XF86_EXTENSIONS
    if (fullscreen_is_enabled
        ? ted_resources.fullscreen_double_size_enabled
        : ted_resources.double_size_enabled)
#else
    if (ted_resources.double_size_enabled)
#endif
#else
    if (0)
#endif
    {
        if (ted.raster.viewport.pixel_size.width == 1
            && ted.raster.viewport.canvas != NULL) {
          raster_set_pixel_size(&ted.raster, 2, 2, VIDEO_RENDER_PAL_2X2);
          raster_resize_viewport(&ted.raster,
                                 ted.raster.viewport.width * 2,
                                 ted.raster.viewport.height * 2);
        } else {
            raster_set_pixel_size(&ted.raster, 2, 2, VIDEO_RENDER_PAL_2X2);
        }

        ted_draw_set_double_size(1);
    } else {
        if (ted.raster.viewport.pixel_size.width == 2
            && ted.raster.viewport.canvas != NULL) {
            raster_set_pixel_size(&ted.raster, 1, 1, VIDEO_RENDER_PAL_1X1);
            raster_resize_viewport(&ted.raster,
                                   ted.raster.viewport.width / 2,
                                   ted.raster.viewport.height / 2);
        } else {
            raster_set_pixel_size(&ted.raster, 1, 1, VIDEO_RENDER_PAL_1X1);
        }

        ted_draw_set_double_size(0);
    }

#ifdef VIC_II_NEED_2X
#ifdef USE_XF86_EXTENSIONS
    if (fullscreen_is_enabled)
        raster_enable_double_scan(&ted.raster,
                                  ted_resources.fullscreen_double_scan_enabled);
    else
#endif
        raster_enable_double_scan(&ted.raster,
                                  ted_resources.double_scan_enabled);
#endif
}

void ted_free(void)
{
    raster_free(&ted.raster);
}

int ted_screenshot(screenshot_t *screenshot)
{
    return raster_screenshot(&ted.raster, screenshot);
}

void ted_video_refresh(void)
{
#ifdef USE_XF86_EXTENSIONS

  ted_resize();
  raster_enable_double_scan(&ted.raster,
                            fullscreen_is_enabled ?
                            ted_resources.fullscreen_double_scan_enabled :
                            ted_resources.double_scan_enabled);
#endif
}

