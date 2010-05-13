/*
 * viciidtv.c - A cycle-exact event-driven VIC-II DTV emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
 *
 * DTV sections written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Daniel Kahlin <daniel@kahlin.net>
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

/* TODO: - speed optimizations;
   - faster sprites and registers.  */

/*
   Current (most important) known limitations:

   - sprite colors (and other attributes) cannot change in the middle of the
   raster line;

   Probably something else which I have not figured out yet...

 */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alarm.h"
#include "c64dtv.h"
#include "c64dtvblitter.h"
#include "c64dtvdma.h"
#include "clkguard.h"
#include "dma.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "maindtvcpu.h"

#ifdef WATCOM_COMPILE
#include "../mem.h"
#else
#include "mem.h"
#endif

#include "raster-line.h"
#include "raster-modes.h"
#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "resources.h"
#include "screenshot.h"
#include "types.h"
#include "viciidtv-cmdline-options.h"
#include "viciidtv-color.h"
#include "viciidtv-draw.h"
#include "viciidtv-fetch.h"
#include "viciidtv-irq.h"
#include "viciidtv-mem.h"
#include "viciidtv-sprites.h"
#include "viciidtv-resources.h"
#include "viciidtv-timing.h"
#include "vicii.h"
#include "viciidtvtypes.h"
#include "vsync.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"


void vicii_set_phi1_addr_options(WORD mask, WORD offset)
{
    vicii.vaddr_mask_phi1 = mask;
    vicii.vaddr_offset_phi1 = offset;

    VICII_DEBUG_REGISTER(("Set phi1 video addr mask=%04x, offset=%04x",
                         mask, offset));
    vicii_update_memory_ptrs_external();
}

void vicii_set_phi2_addr_options(WORD mask, WORD offset)
{
    vicii.vaddr_mask_phi2 = mask;
    vicii.vaddr_offset_phi2 = offset;

    VICII_DEBUG_REGISTER(("Set phi2 video addr mask=%04x, offset=%04x",
                         mask, offset));
    vicii_update_memory_ptrs_external();
}

void vicii_set_phi1_chargen_addr_options(WORD mask, WORD value)
{
    vicii.vaddr_chargen_mask_phi1 = mask;
    vicii.vaddr_chargen_value_phi1 = value;

    VICII_DEBUG_REGISTER(("Set phi1 chargen addr mask=%04x, value=%04x",
                         mask, value));
    vicii_update_memory_ptrs_external();
}

void vicii_set_phi2_chargen_addr_options(WORD mask, WORD value)
{
    vicii.vaddr_chargen_mask_phi2 = mask;
    vicii.vaddr_chargen_value_phi2 = value;

    VICII_DEBUG_REGISTER(("Set phi2 chargen addr mask=%04x, value=%04x",
                         mask, value));
    vicii_update_memory_ptrs_external();
}

void vicii_set_chargen_addr_options(WORD mask, WORD value)
{
    vicii.vaddr_chargen_mask_phi1 = mask;
    vicii.vaddr_chargen_value_phi1 = value;
    vicii.vaddr_chargen_mask_phi2 = mask;
    vicii.vaddr_chargen_value_phi2 = value;

    VICII_DEBUG_REGISTER(("Set chargen addr mask=%04x, value=%04x",
                         mask, value));
    vicii_update_memory_ptrs_external();
}

/* ---------------------------------------------------------------------*/

vicii_t vicii;

static void vicii_set_geometry(void);

void vicii_change_timing(machine_timing_t *machine_timing, int border_mode)
{
    vicii_timing_set(machine_timing, border_mode);

    if (vicii.initialized) {
        vicii_set_geometry();
        raster_mode_change();
    }
}

inline void vicii_handle_pending_alarms(int num_write_cycles)
{
    return;
}

void vicii_handle_pending_alarms_external(int num_write_cycles)
{
    if (vicii.initialized)
        vicii_handle_pending_alarms(num_write_cycles);
}

void vicii_handle_pending_alarms_external_write(void)
{
    /* WARNING: assumes `maincpu_rmw_flag' is 0 or 1.  */
    if (vicii.initialized)
        vicii_handle_pending_alarms(maincpu_rmw_flag + 1);
}

static void vicii_set_geometry(void)
{
    unsigned int width, height;

    width = vicii.screen_leftborderwidth + VICII_SCREEN_XPIX + vicii.screen_rightborderwidth;
    height = vicii.last_displayed_line - vicii.first_displayed_line + 1;

    raster_set_geometry(&vicii.raster,
                        width, height, /* canvas dimensions */
                        width, vicii.screen_height, /* screen dimensions */
                        width, height, /* gfx dimensions */
                        width/8, height/8, /* text dimensions */
                        0, 0, /* gfx position */
                        0, /* gfx area doesn't move */
                        vicii.first_displayed_line,
                        vicii.last_displayed_line,
                        0, /* extra offscreen border left */
                        0) /* extra offscreen border right */;

#ifdef __MSDOS__
    video_ack_vga_mode();
#endif

    vicii.raster.display_ystart = 0;
    vicii.raster.display_ystop = vicii.screen_height;
    vicii.raster.display_xstart = 0;
    vicii.raster.display_xstop = width;
}

static int init_raster(void)
{
    raster_t *raster;

    raster = &vicii.raster;
    video_color_set_canvas(raster->canvas);

    raster->sprite_status = NULL;
    raster_line_changes_init(raster);

    /* We only use the dummy mode for "drawing" to raster.
       Report only 1 video mode and set the idle mode to it. */
    if (raster_init(raster, 1) < 0) {
        return -1;
    }
    raster_modes_set_idle_mode(raster->modes, VICII_DUMMY_MODE);

    resources_touch("VICIIVideoCache");

    vicii_set_geometry();

    if (vicii_color_update_palette(raster->canvas) < 0) {
        log_error(vicii.log, "Cannot load palette.");
        return -1;
    }

    raster_set_title(raster, machine_name);

    if (raster_realize(raster) < 0) {
        return -1;
    }

    return 0;
}

static void vicii_sprites_init(void)
{
    int i;

    for (i = 0; i < VICII_NUM_SPRITES; i++) {
        vicii.sprite[i].data = 0;
        vicii.sprite[i].mc = 0;
        vicii.sprite[i].mcbase = 0;
        vicii.sprite[i].pointer = 0;
        vicii.sprite[i].exp_flop = 1;
        vicii.sprite[i].x = 0;
    }

    vicii.sprite_display_bits = 0;
    vicii.sprite_dma = 0;
}

/* Initialize the VIC-II emulation.  */
raster_t *vicii_init(unsigned int flag)
{
    vicii.log = log_open("VIC-II DTV");

    if (init_raster() < 0) {
        return NULL;
    }

    vicii_powerup();

    vicii.video_mode = -1;
    vicii_update_video_mode(0);
    vicii_update_memory_ptrs(0);

    vicii.raster_cycle = 0;
    vicii_draw_init();
    vicii_sprites_init();

    vicii.buf_offset = 0;

    vicii.initialized = 1;

    return &vicii.raster;
}

struct video_canvas_s *vicii_get_canvas(void)
{
    return vicii.raster.canvas;
}

/* Reset the VIC-II chip.  */
void vicii_reset(void)
{
    int i;

    raster_reset(&vicii.raster);

    vicii.raster_line = 0;
    vicii.raster_cycle = 6;

    vicii.raster_irq_line = 0;

    /* FIXME: I am not sure this is exact emulation.  */
    vicii.regs[0x11] = 0;
    vicii.regs[0x12] = 0;

    vicii.force_display_state = 0;

    /* Remove all the IRQ sources.  */
    vicii.regs[0x1a] = 0;

    vicii.store_clk = CLOCK_MAX;

    vicii.counta = 0;
    vicii.counta_mod = 0;
    vicii.counta_step = 0;
    vicii.countb = 0;
    vicii.countb_mod = 0;
    vicii.countb_step = 0;

    for (i = 0; i < 256; i++) {
        vicii.dtvpalette[i] = i;
    }

    vicii.dtvpalette[0] = 0;
    vicii.dtvpalette[1] = 0x0f;
    vicii.dtvpalette[2] = 0x36;
    vicii.dtvpalette[3] = 0xbe;
    vicii.dtvpalette[4] = 0x58;
    vicii.dtvpalette[5] = 0xdb;
    vicii.dtvpalette[6] = 0x86;
    vicii.dtvpalette[7] = 0xff;
    vicii.dtvpalette[8] = 0x29;
    vicii.dtvpalette[9] = 0x26;
    vicii.dtvpalette[10] = 0x3b;
    vicii.dtvpalette[11] = 0x05;
    vicii.dtvpalette[12] = 0x07;
    vicii.dtvpalette[13] = 0xdf;
    vicii.dtvpalette[14] = 0x9a;
    vicii.dtvpalette[15] = 0x0a;

    /* clear colors so that standard colors can be written without
       having extended_enable=1 */
    vicii.regs[0x20] = 0;
    vicii.regs[0x21] = 0;
    vicii.regs[0x22] = 0;
    vicii.regs[0x23] = 0;
    vicii.regs[0x24] = 0;

    vicii.regs[0x3c] = 0;

    vicii.regs[0x36] = 0x76;
    vicii.regs[0x37] = 0;

    /* clear count[ab] & other regs,
       fixes problem with DTVBIOS, gfxmodes & soft reset */
    vicii.regs[0x38] = 0;
    vicii.regs[0x39] = 0;
    vicii.regs[0x3a] = 0;
    vicii.regs[0x3b] = 0;
    vicii.regs[0x3d] = 0;
    vicii.regs[0x44] = 64;
    vicii.regs[0x45] = 0;
    vicii.regs[0x46] = 0;
    vicii.regs[0x47] = 0;
    vicii.regs[0x48] = 0;
    vicii.regs[0x49] = 0;
    vicii.regs[0x4a] = 0;
    vicii.regs[0x4b] = 0;
    vicii.regs[0x4c] = 0;
    vicii.regs[0x4d] = 0;

    vicii.extended_enable = 0;
    vicii.extended_lockout = 0;
    vicii.badline_disable = 0;
    vicii.colorfetch_disable = 0;
    vicii.border_off = 0;
    vicii.overscan = 0;
    vicii.color_ram_ptr = &mem_ram[0x01d800];

    vicii.raster_irq_offset = 64;
}

void vicii_reset_registers(void)
{
    WORD i;

    if (!vicii.initialized) {
        return;
    }

    vicii.extended_enable = 1;
    vicii.extended_lockout = 0;

    for (i = 0; i <= 0x3e; i++) {
        vicii_store(i, 0);
    }

    vicii_store(0x36, 0x76);

    for (i = 0x40; i <= 0x4f; i++) {
        vicii_store(i, 0);
    }

    vicii_store(0x3f, 0);
}

/* This /should/ put the VIC-II in the same state as after a powerup, if
   `reset_vicii()' is called afterwards.  But FIXME, as we are not really
   emulating everything correctly here; just $D011.  */
void vicii_powerup(void)
{
    memset(vicii.regs, 0, sizeof(vicii.regs));

    vicii.irq_status = 0;
    vicii.raster_irq_line = 0;

    vicii.ram_base_phi1 = mem_ram;
    vicii.ram_base_phi2 = mem_ram;

    vicii.vaddr_mask_phi1 = 0xffff;
    vicii.vaddr_mask_phi2 = 0xffff;
    vicii.vaddr_offset_phi1 = 0;
    vicii.vaddr_offset_phi2 = 0;

    vicii.allow_bad_lines = 0;
    vicii.sprite_sprite_collisions = vicii.sprite_background_collisions = 0;
    vicii.idle_state = 0;
    vicii.force_display_state = 0;
    vicii.memory_fetch_done = 0;
    vicii.memptr = 0;
    vicii.mem_counter = 0;
    vicii.bad_line = 0;
    vicii.ycounter_reset_checked = 0;
    vicii.force_black_overscan_background_color = 0;
    vicii.vbank_phi1 = 0;
    vicii.vbank_phi2 = 0;
    vicii.idle_data = 0;

    vicii_reset();

    vicii.raster.blank = 1;
    vicii.raster.display_ystart = vicii.row_24_start_line;
    vicii.raster.display_ystop = vicii.row_24_stop_line;

    vicii.raster.ysmooth = 0;
}

/* ---------------------------------------------------------------------*/

/* This hook is called whenever video bank must be changed.  */
static inline void vicii_set_vbanks(int vbank_p1, int vbank_p2)
{
    /* Warning: assumes it's called within a memory write access.
       FIXME: Change name?  */
    /* Also, we assume the bank has *really* changed, and do not do any
       special optimizations for the not-really-changed case.  */
    vicii_handle_pending_alarms(maincpu_rmw_flag + 1);
    vicii.vbank_phi1 = vbank_p1;
    vicii.vbank_phi2 = vbank_p2;
    vicii_update_memory_ptrs(VICII_RASTER_CYCLE(maincpu_clk));
}

/* Phi1 and Phi2 accesses */
void vicii_set_vbank(int num_vbank)
{
    int tmp = num_vbank << 14;
    vicii_set_vbanks(tmp, tmp);
}

/* Phi1 accesses */
void vicii_set_phi1_vbank(int num_vbank)
{
    vicii_set_vbanks(num_vbank << 14, vicii.vbank_phi2);
}

/* Phi2 accesses */
void vicii_set_phi2_vbank(int num_vbank)
{
    vicii_set_vbanks(vicii.vbank_phi1, num_vbank << 14);
}

/* ---------------------------------------------------------------------*/

/* Change the base of RAM seen by the VIC-II.  */
static inline void vicii_set_ram_bases(BYTE *base_p1, BYTE *base_p2)
{
    /* WARNING: assumes `maincpu_rmw_flag' is 0 or 1.  */
    vicii_handle_pending_alarms(maincpu_rmw_flag + 1);

    vicii.ram_base_phi1 = base_p1;
    vicii.ram_base_phi2 = base_p2;
    vicii_update_memory_ptrs(VICII_RASTER_CYCLE(maincpu_clk));
}

void vicii_set_ram_base(BYTE *base)
{
    vicii_set_ram_bases(base, base);
}

void vicii_set_phi1_ram_base(BYTE *base)
{
    vicii_set_ram_bases(base, vicii.ram_base_phi2);
}

void vicii_set_phi2_ram_base(BYTE *base)
{
    vicii_set_ram_bases(vicii.ram_base_phi1, base);
}


void vicii_update_memory_ptrs_external(void)
{
    if (vicii.initialized > 0)
        vicii_update_memory_ptrs(VICII_RASTER_CYCLE(maincpu_clk));
}

/* Set the memory pointers according to the values in the registers.  */
void vicii_update_memory_ptrs(unsigned int cycle)
{
    /* FIXME: This is *horrible*!  */
    static BYTE *old_screen_ptr, *old_bitmap_low_ptr, *old_bitmap_high_ptr;
    static BYTE *old_chargen_ptr;
    static int old_vbank_p1 = -1;
    static int old_vbank_p2 = -1;
    WORD screen_addr;             /* Screen start address.  */
    BYTE *char_base;              /* Pointer to character memory.  */
    BYTE *bitmap_low_base;        /* Pointer to bitmap memory (low part).  */
    BYTE *bitmap_high_base;       /* Pointer to bitmap memory (high part).  */
    int tmp, bitmap_bank;

    viciidtv_update_colorram();

    screen_addr = vicii.vbank_phi2 + ((vicii.regs[0x18] & 0xf0) << 6);

    screen_addr = (screen_addr & vicii.vaddr_mask_phi2)
                  | vicii.vaddr_offset_phi2;

    VICII_DEBUG_REGISTER(("Screen memory at $%04X", screen_addr));

    tmp = (vicii.regs[0x18] & 0xe) << 10;
    tmp = (tmp + vicii.vbank_phi1);
    tmp &= vicii.vaddr_mask_phi1;
    tmp |= vicii.vaddr_offset_phi1;

    bitmap_bank = tmp & 0xe000;
    bitmap_low_base = vicii.ram_base_phi1 + bitmap_bank;

    VICII_DEBUG_REGISTER(("Bitmap memory at $%04X", tmp & 0xe000));

    if ((screen_addr & vicii.vaddr_chargen_mask_phi2)
        != vicii.vaddr_chargen_value_phi2) {
        vicii.screen_base_phi2 = vicii.ram_base_phi2 + screen_addr;
    } else {
        vicii.screen_base_phi2 = mem_chargen_rom_ptr
                                 + (screen_addr & 0xc00);
    }

    if ((screen_addr & vicii.vaddr_chargen_mask_phi1)
        != vicii.vaddr_chargen_value_phi1) {
        vicii.screen_base_phi1 = vicii.ram_base_phi1 + screen_addr;
    } else {
        vicii.screen_base_phi1 = mem_chargen_rom_ptr
                                 + (screen_addr & 0xc00);
    }

    if ((tmp & vicii.vaddr_chargen_mask_phi1)
        != vicii.vaddr_chargen_value_phi1) {
        char_base = vicii.ram_base_phi1 + tmp;
    } else {
        char_base = mem_chargen_rom_ptr + (tmp & 0x0800);
    }

    if (((bitmap_bank + 0x1000) & vicii.vaddr_chargen_mask_phi1)
        != vicii.vaddr_chargen_value_phi1) {
        bitmap_high_base = bitmap_low_base + 0x1000;
    } else {
        bitmap_high_base = mem_chargen_rom_ptr;
    }

    switch (vicii.video_mode) {
        /* TODO other modes */
        case VICII_8BPP_PIXEL_CELL_MODE:
        case VICII_ILLEGAL_LINEAR_MODE:
            vicii.screen_base_phi2 = vicii.ram_base_phi2 + (vicii.regs[0x45]<<16) + (vicii.regs[0x3b]<<8) + vicii.regs[0x3a];
            break;
        default:
            vicii.screen_base_phi2 += (vicii.regs[0x45]<<16);
            char_base += (vicii.regs[0x3d]<<16);
            bitmap_low_base += (vicii.regs[0x3d]<<16);
            bitmap_high_base += (vicii.regs[0x3d]<<16);
            break;
    }

    tmp = VICII_RASTER_CHAR(cycle);

    if (tmp <= 0) {
        old_screen_ptr = vicii.screen_ptr = vicii.screen_base_phi2;
        old_bitmap_low_ptr = vicii.bitmap_low_ptr = bitmap_low_base;
        old_bitmap_high_ptr = vicii.bitmap_high_ptr = bitmap_high_base;
        old_chargen_ptr = vicii.chargen_ptr = char_base;
        old_vbank_p1 = vicii.vbank_phi1;
        old_vbank_p2 = vicii.vbank_phi2;
        /* vicii.vbank_ptr = vicii.ram_base + vicii.vbank; */
    } else if (tmp < VICII_SCREEN_TEXTCOLS) {
        if (vicii.screen_base_phi2 != old_screen_ptr) {
            raster_changes_foreground_add_ptr(&vicii.raster, tmp,
                                              (void *)&vicii.screen_ptr,
                                              (void *)vicii.screen_base_phi2);
            old_screen_ptr = vicii.screen_base_phi2;
        }

        if (bitmap_low_base != old_bitmap_low_ptr) {
            raster_changes_foreground_add_ptr(&vicii.raster,
                                              tmp,
                                              (void *)&vicii.bitmap_low_ptr,
                                              (void *)(bitmap_low_base));
            old_bitmap_low_ptr = bitmap_low_base;
        }

        if (bitmap_high_base != old_bitmap_high_ptr) {
            raster_changes_foreground_add_ptr(&vicii.raster,
                                              tmp,
                                              (void *)&vicii.bitmap_high_ptr,
                                              (void *)(bitmap_high_base));
            old_bitmap_high_ptr = bitmap_high_base;
        }

        if (char_base != old_chargen_ptr) {
            raster_changes_foreground_add_ptr(&vicii.raster,
                                              tmp,
                                              (void *)&vicii.chargen_ptr,
                                              (void *)char_base);
            old_chargen_ptr = char_base;
        }

        if (vicii.vbank_phi1 != old_vbank_p1) {
            old_vbank_p1 = vicii.vbank_phi1;
        }

        if (vicii.vbank_phi2 != old_vbank_p2) {
            old_vbank_p2 = vicii.vbank_phi2;
        }
    } else {
        if (vicii.screen_base_phi2 != old_screen_ptr) {
            raster_changes_next_line_add_ptr(&vicii.raster,
                                             (void *)&vicii.screen_ptr,
                                             (void *)vicii.screen_base_phi2);
            old_screen_ptr = vicii.screen_base_phi2;
        }

        if (bitmap_low_base != old_bitmap_low_ptr) {
            raster_changes_next_line_add_ptr(&vicii.raster,
                                             (void *)&vicii.bitmap_low_ptr,
                                             (void *)(bitmap_low_base));
            old_bitmap_low_ptr = bitmap_low_base;
        }

        if (bitmap_high_base != old_bitmap_high_ptr) {
            raster_changes_next_line_add_ptr(&vicii.raster,
                                             (void *)&vicii.bitmap_high_ptr,
                                             (void *)(bitmap_high_base));
            old_bitmap_high_ptr = bitmap_high_base;
        }

        if (char_base != old_chargen_ptr) {
            raster_changes_next_line_add_ptr(&vicii.raster,
                                             (void *)&vicii.chargen_ptr,
                                             (void *)char_base);
            old_chargen_ptr = char_base;
        }

        if (vicii.vbank_phi1 != old_vbank_p1) {
            old_vbank_p1 = vicii.vbank_phi1;
        }

        if (vicii.vbank_phi2 != old_vbank_p2) {
            old_vbank_p2 = vicii.vbank_phi2;
        }
    }
}

/* Set the video mode according to the values in registers $D011 and $D016 of
   the VIC-II chip.  */
void vicii_update_video_mode(unsigned int cycle)
{
    int new_video_mode;

    new_video_mode = ((vicii.regs[0x11] & 0x60)
                     | (vicii.regs[0x16] & 0x10)) >> 4;

    new_video_mode |= (((vicii.regs[0x3c] & 0x04)<<1)
                     | ((vicii.regs[0x3c] & 0x01)<<3));

    if (((new_video_mode) == VICII_8BPP_FRED_MODE)
       && ((vicii.regs[0x3c] & 0x04)==0)) {
         new_video_mode = VICII_8BPP_FRED2_MODE;
    }

    if (((new_video_mode) == VICII_8BPP_CHUNKY_MODE)
       && ((vicii.regs[0x3c] & 0x10)==0)) {
        if (vicii.regs[0x3c] & 0x04) {
            new_video_mode = VICII_8BPP_PIXEL_CELL_MODE;
        } else {
            new_video_mode = VICII_ILLEGAL_LINEAR_MODE;
        }
    }

    /* HACK to make vcache display gfx in chunky & the rest */
    if ((new_video_mode >= VICII_8BPP_CHUNKY_MODE)&&
        (new_video_mode <= VICII_8BPP_PIXEL_CELL_MODE)) {
            vicii.raster.dont_cache = 1;
    }

    viciidtv_update_colorram();

    if (new_video_mode != vicii.video_mode) {
        switch (new_video_mode) {
          case VICII_ILLEGAL_TEXT_MODE:
          case VICII_ILLEGAL_BITMAP_MODE_1:
          case VICII_ILLEGAL_BITMAP_MODE_2:
          case VICII_ILLEGAL_LINEAR_MODE:
            /* Force the overscan color to black.  */
            raster_changes_background_add_int
                (&vicii.raster, VICII_RASTER_X(cycle),
                &vicii.raster.idle_background_color, 0);
            raster_changes_background_add_int
                (&vicii.raster,
                VICII_RASTER_X(VICII_RASTER_CYCLE(maincpu_clk)),
                &vicii.raster.xsmooth_color, 0);
            vicii.get_background_from_vbuf = 0;
            vicii.force_black_overscan_background_color = 1;
            break;
          case VICII_HIRES_BITMAP_MODE:
            raster_changes_background_add_int
                (&vicii.raster, VICII_RASTER_X(cycle),
                &vicii.raster.idle_background_color, 0);
            raster_changes_background_add_int
                (&vicii.raster,
                VICII_RASTER_X(VICII_RASTER_CYCLE(maincpu_clk)),
                &vicii.raster.xsmooth_color,
                vicii.background_color_source & 0x0f);
            vicii.get_background_from_vbuf = VICII_HIRES_BITMAP_MODE;
            vicii.force_black_overscan_background_color = 1;
            break;
          case VICII_EXTENDED_TEXT_MODE:
            raster_changes_background_add_int
                (&vicii.raster, VICII_RASTER_X(cycle),
                &vicii.raster.idle_background_color,
                vicii.dtvpalette[vicii.regs[0x21]]);
            raster_changes_background_add_int
                (&vicii.raster,
                VICII_RASTER_X(VICII_RASTER_CYCLE(maincpu_clk)),
                &vicii.raster.xsmooth_color,
                vicii.dtvpalette[vicii.regs[0x21 + (vicii.background_color_source >> 6)]]);
            vicii.get_background_from_vbuf = VICII_EXTENDED_TEXT_MODE;
            vicii.force_black_overscan_background_color = 0;
            break;
          default:
            /* The overscan background color is given by the background
               color register.  */
            raster_changes_background_add_int
                (&vicii.raster, VICII_RASTER_X(cycle),
                &vicii.raster.idle_background_color,
                vicii.dtvpalette[vicii.regs[0x21]]);
            raster_changes_background_add_int
                (&vicii.raster,
                VICII_RASTER_X(VICII_RASTER_CYCLE(maincpu_clk)),
                &vicii.raster.xsmooth_color,
                vicii.dtvpalette[vicii.regs[0x21]]);
            vicii.get_background_from_vbuf = 0;
            vicii.force_black_overscan_background_color = 0;
            break;
        }

        {
            int pos;

            pos = VICII_RASTER_CHAR(cycle) - 1;

            raster_changes_background_add_int(&vicii.raster,
                                              VICII_RASTER_X(cycle),
                                              &vicii.raster.video_mode,
                                              new_video_mode);

            raster_changes_foreground_add_int(&vicii.raster, pos,
                                              &vicii.raster.last_video_mode,
                                              vicii.video_mode);

            raster_changes_foreground_add_int(&vicii.raster, pos,
                                              &vicii.raster.video_mode,
                                              new_video_mode);

            raster_changes_foreground_add_int(&vicii.raster, pos + 2,
                                              &vicii.raster.last_video_mode,
                                              -1);

        }

        vicii.video_mode = new_video_mode;
    }

#ifdef VICII_VMODE_DEBUG
    switch (new_video_mode) {
      case VICII_NORMAL_TEXT_MODE:
        VICII_DEBUG_VMODE(("Standard Text"));
        break;
      case VICII_MULTICOLOR_TEXT_MODE:
        VICII_DEBUG_VMODE(("Multicolor Text"));
        break;
      case VICII_HIRES_BITMAP_MODE:
        VICII_DEBUG_VMODE(("Hires Bitmap"));
        break;
      case VICII_MULTICOLOR_BITMAP_MODE:
        VICII_DEBUG_VMODE(("Multicolor Bitmap"));
        break;
      case VICII_EXTENDED_TEXT_MODE:
        VICII_DEBUG_VMODE(("Extended Text"));
        break;
      case VICII_ILLEGAL_TEXT_MODE:
        VICII_DEBUG_VMODE(("Illegal Text"));
        break;
      case VICII_ILLEGAL_BITMAP_MODE_1:
        VICII_DEBUG_VMODE(("Invalid Bitmap"));
        break;
      case VICII_ILLEGAL_BITMAP_MODE_2:
        VICII_DEBUG_VMODE(("Invalid Bitmap"));
        break;
      case VICII_8BPP_NORMAL_TEXT_MODE:
        VICII_DEBUG_VMODE(("8BPP Standard Text"));
        break;
      case VICII_8BPP_MULTICOLOR_TEXT_MODE:
        VICII_DEBUG_VMODE(("8BPP Multicolor Text"));
        break;
      case VICII_8BPP_HIRES_BITMAP_MODE:
        VICII_DEBUG_VMODE(("8BPP Hires Bitmap (?)"));
        break;
      case VICII_8BPP_MULTICOLOR_BITMAP_MODE:
        VICII_DEBUG_VMODE(("8BPP Multicolor Bitmap"));
        break;
      case VICII_8BPP_EXTENDED_TEXT_MODE:
        VICII_DEBUG_VMODE(("8BPP Extended Text"));
        break;
      case VICII_8BPP_CHUNKY_MODE:
        VICII_DEBUG_VMODE(("Chunky mode"));
        break;
      case VICII_8BPP_TWO_PLANE_BITMAP_MODE:
        VICII_DEBUG_VMODE(("Two plane bitmap"));
        break;
      case VICII_8BPP_FRED_MODE:
        VICII_DEBUG_VMODE(("FRED"));
        break;
      case VICII_8BPP_FRED2_MODE:
        VICII_DEBUG_VMODE(("FRED2"));
        break;
      case VICII_8BPP_PIXEL_CELL_MODE:
        VICII_DEBUG_VMODE(("8BPP Pixel Cell"));
        break;
      case VICII_ILLEGAL_LINEAR_MODE:
        VICII_DEBUG_VMODE(("Illegal Linear"));
        break;
      default:                    /* cannot happen */
        VICII_DEBUG_VMODE(("???"));
    }

    VICII_DEBUG_VMODE(("Mode enabled at line $%04X, cycle %d.",
                       VICII_RASTER_Y(maincpu_clk), cycle));
#endif
}

/* Redraw the current raster line.  This happens after the last cycle
   of each line.  */
void vicii_raster_draw_handler(void)
{
    BYTE prev_sprite_sprite_collisions;
    BYTE prev_sprite_background_collisions;
    int in_visible_area;

    prev_sprite_sprite_collisions = vicii.sprite_sprite_collisions;
    prev_sprite_background_collisions = vicii.sprite_background_collisions;

    in_visible_area = (vicii.raster.current_line
                      >= (unsigned int)vicii.first_displayed_line
                      && vicii.raster.current_line
                      <= (unsigned int)vicii.last_displayed_line);

    /* handle wrap if the first few lines are displayed in the visible lower border */
    if ((unsigned int)vicii.last_displayed_line >= vicii.screen_height) {
        in_visible_area |= vicii.raster.current_line
                          <= ((unsigned int)vicii.last_displayed_line - vicii.screen_height);
    }

    vicii.raster.xsmooth_shift_left = 0;

    vicii_sprites_reset_xshift();

    raster_line_emulate(&vicii.raster);

    if (vicii.raster.current_line == 0) {
        /* no vsync here for NTSC  */
        if ((unsigned int)vicii.last_displayed_line < vicii.screen_height) {
            raster_skip_frame(&vicii.raster,
                              vsync_do_vsync(vicii.raster.canvas,
                              vicii.raster.skip_frame));
        }
        vicii.memptr = 0;
        vicii.mem_counter = 0;
        vicii.raster.blank_off = 0;

        /* Clear color buffer on line 0 */
        memset(vicii.cbuf, 0, sizeof(vicii.cbuf));

        /* HACK to make vcache display gfx in chunky & the rest */
        if ((vicii.video_mode >= VICII_8BPP_CHUNKY_MODE)&&
            (vicii.video_mode <= VICII_8BPP_PIXEL_CELL_MODE)) {
                vicii.raster.dont_cache = 1;
        }

        /* HACK to fix greetings in 2008 */
        if(vicii.video_mode == VICII_8BPP_PIXEL_CELL_MODE) {
            vicii_update_memory_ptrs(VICII_RASTER_CYCLE(maincpu_clk));
        }

#ifdef __MSDOS__
        if ((unsigned int)vicii.last_displayed_line < vicii.screen_height) {
            if (vicii.raster.canvas->draw_buffer->canvas_width
                <= VICII_SCREEN_XPIX
                && vicii.raster.canvas->draw_buffer->canvas_height
                <= VICII_SCREEN_YPIX
                && vicii.raster.canvas->viewport->update_canvas)
                canvas_set_border_color(vicii.raster.canvas,
                                        vicii.raster.border_color);
        }
#endif
    }

    /* vsync for NTSC */
    if ((unsigned int)vicii.last_displayed_line >= vicii.screen_height
        && vicii.raster.current_line == vicii.last_displayed_line - vicii.screen_height + 1) {
        raster_skip_frame(&vicii.raster,
                          vsync_do_vsync(vicii.raster.canvas,
                          vicii.raster.skip_frame));
#ifdef __MSDOS__
        if (vicii.raster.canvas->draw_buffer->canvas_width
            <= VICII_SCREEN_XPIX
            && vicii.raster.canvas->draw_buffer->canvas_height
            <= VICII_SCREEN_YPIX
            && vicii.raster.canvas->viewport->update_canvas)
            canvas_set_border_color(vicii.raster.canvas,
                                    vicii.raster.border_color);
#endif
    }

    if ( (!vicii.overscan && vicii.raster.current_line == 48)
       || (vicii.overscan && vicii.raster.current_line == 10) ) {
        vicii.counta = vicii.regs[0x3a]
                     + (vicii.regs[0x3b]<<8)
                     + (vicii.regs[0x45]<<16);

        vicii.countb = vicii.regs[0x49]
                     + (vicii.regs[0x4a]<<8)
                     + (vicii.regs[0x4b]<<16);
    }

    if (in_visible_area) {
        if (!vicii.idle_state) {
            /* TODO should be done in cycle 57 */
            if ( !(VICII_MODULO_BUG(vicii.video_mode) && (vicii.raster.ycounter == 7)) ) {
                vicii.counta += vicii.counta_mod;
                vicii.countb += vicii.countb_mod;
            }

            /* TODO hack */
            if (!vicii.overscan) { 
                vicii.counta += vicii.counta_step*40;
                vicii.countb += vicii.countb_step*40;
            } else {
                /* faked overscan */
                vicii.counta += vicii.counta_step*48;
                vicii.countb += vicii.countb_step*48;
            }

            /* HACK to fix greetings in 2008 */
            if((vicii.video_mode == VICII_8BPP_PIXEL_CELL_MODE)&&(vicii.raster.ycounter == 7)) {
                vicii.screen_base_phi2 += vicii.counta_mod;
            }
        }

        /* `ycounter' makes the chip go to idle state when it reaches the
           maximum value.  */
        if (vicii.raster.ycounter == 7) {
            vicii.idle_state = 1;
            vicii.memptr = vicii.mem_counter;
        }
        if (!vicii.idle_state || vicii.bad_line) {
            vicii.raster.ycounter = (vicii.raster.ycounter + 1) & 0x7;
            vicii.idle_state = 0;
        }
        if (vicii.force_display_state) {
            vicii.idle_state = 0;
            vicii.force_display_state = 0;
        }
        vicii.raster.draw_idle_state = vicii.idle_state;
        vicii.bad_line = 0;
    }

    vicii.ycounter_reset_checked = 0;
    vicii.memory_fetch_done = 0;
    vicii.buf_offset = 0;

    if (vicii.raster.current_line == vicii.first_dma_line)
        vicii.allow_bad_lines = !vicii.raster.blank;

    /* As explained in Christian's article, only the first collision
       (i.e. the first time the collision register becomes non-zero) actually
       triggers an interrupt.  */
    if (vicii_resources.sprite_sprite_collisions_enabled
        && vicii.raster.sprite_status->sprite_sprite_collisions != 0
        && !prev_sprite_sprite_collisions) {
        vicii_irq_sscoll_set();
    }

    if (vicii_resources.sprite_background_collisions_enabled
        && vicii.raster.sprite_status->sprite_background_collisions
        && !prev_sprite_background_collisions) {
        vicii_irq_sbcoll_set();
    }
}

void vicii_set_canvas_refresh(int enable)
{
    raster_set_canvas_refresh(&vicii.raster, enable);
}

void vicii_shutdown(void)
{
    vicii_sprites_shutdown();
    raster_sprite_status_destroy(&vicii.raster);
    raster_shutdown(&vicii.raster);
}

void vicii_screenshot(screenshot_t *screenshot)
{
    raster_screenshot(&vicii.raster, screenshot);
}

void vicii_async_refresh(struct canvas_refresh_s *refresh)
{
    raster_async_refresh(&vicii.raster, refresh);
}

