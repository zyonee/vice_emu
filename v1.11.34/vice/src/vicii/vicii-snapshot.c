/*
 * vicii-snapshot.c - Snapshot functionality for the MOS 6569 (VIC-II)
 * emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <stdio.h>

#include "alarm.h"
#include "interrupt.h"
#include "log.h"
#include "mem.h"
#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "snapshot.h"
#include "types.h"
#include "vicii-irq.h"
#include "vicii-snapshot.h"
#include "vicii-sprites.h"
#include "vicii.h"
#include "viciitypes.h"

/*

   This is the format of the VIC-II snapshot module.

   Name               Type   Size   Description

   AllowBadLines      BYTE   1      flag: if true, bad lines can happen
   BadLine            BYTE   1      flag: this is a bad line
   Blank              BYTE   1      flag: draw lines in border color
   ColorBuf           BYTE   40     character memory buffer (loaded at bad line)
   ColorRam           BYTE   1024   contents of color RAM
   IdleState          BYTE   1      flag: idle state enabled
   LPTrigger          BYTE   1      flag: light pen has been triggered
   LPX                BYTE   1      light pen X
   LPY                BYTE   1      light pen Y
   MatrixBuf          BYTE   40     video matrix buffer (loaded at bad line)
   NewSpriteDmaMask   BYTE   1      value for SpriteDmaMask after drawing
                                    sprites
   RamBase            DWORD  1      pointer to the start of RAM seen by the VIC
   RasterCycle        BYTE   1      current vic_ii.raster cycle
   RasterLine         WORD   1      current vic_ii.raster line
   Registers          BYTE   64     VIC-II registers
   SbCollMask         BYTE   1      sprite-background collisions so far
   SpriteDmaMask      BYTE   1      sprites having DMA turned on
   SsCollMask         BYTE   1      sprite-sprite collisions so far
   VBank              BYTE   1      location of memory bank
   Vc                 WORD   1      internal VIC-II counter
   VcAdd              BYTE   1      value to add to Vc at the end of this line
                                    (vic_ii.mem_counter_inc)
   VcBase             WORD   1      internal VIC-II memory pointer
   VideoInt           BYTE   1      status of VIC-II IRQ (vic_ii.irq_status)

   [Sprite section: (repeat 8 times)]

   SpriteXMemPtr      BYTE   1      sprite memory pointer
   SpriteXMemPtrInc   BYTE   1      value to add to the MemPtr after fetch
   SpriteXExpFlipFlop BYTE   1      sprite expansion flip-flop

   [Alarm section]
   FetchEventTick     DWORD  1      ticks for the next "fetch" (DMA) event
   FetchEventType     BYTE   1      type of event (0: matrix, 1: sprite check, 2: sprite fetch)

 */

static char snap_module_name[] = "VIC-II";
#define SNAP_MAJOR 1
#define SNAP_MINOR 1

int vicii_snapshot_write_module(snapshot_t *s)
{
    int i;
    snapshot_module_t *m;
    BYTE color_ram[0x400];
    unsigned int current_line;

    /* FIXME: Dispatch all events?  */

    m = snapshot_module_create (s, snap_module_name, SNAP_MAJOR, SNAP_MINOR);
    if (m == NULL)
        return -1;

    mem_color_ram_to_snapshot(color_ram);

    current_line = VIC_II_RASTER_Y(maincpu_clk);

    if (0
        /* AllowBadLines */
        || SMW_B(m, (BYTE)vic_ii.allow_bad_lines) < 0
        /* BadLine */
        || SMW_B(m, (BYTE)vic_ii.bad_line) < 0
        /* Blank */
        || SMW_B(m, (BYTE)vic_ii.raster.blank_enabled) < 0
        /* ColorBuf */
        || SMW_BA(m, vic_ii.cbuf, 40) < 0
        /* ColorRam */
        || SMW_BA(m, color_ram, 1024) < 0
        /* IdleState */
        || SMW_B(m, vic_ii.idle_state) < 0
        /* LPTrigger */
        || SMW_B(m, (BYTE)vic_ii.light_pen.triggered) < 0
        /* LPX */
        || SMW_B(m, (BYTE)vic_ii.light_pen.x) < 0
        /* LPY */
        || SMW_B(m, (BYTE)vic_ii.light_pen.y) < 0
        /* MatrixBuf */
        || SMW_BA(m, vic_ii.vbuf, 40) < 0
        /* NewSpriteDmaMask */
        || SMW_B(m, vic_ii.raster.sprite_status->new_dma_msk) < 0
        /* RamBase */
        || SMW_DW(m, (DWORD)(vic_ii.ram_base_phi1 - mem_ram)) < 0
        /* RasterCycle */
        || SMW_B(m, (BYTE)(VIC_II_RASTER_CYCLE(maincpu_clk))) < 0
        /* RasterLine */
        || SMW_W(m, (WORD)current_line) < 0)
        goto fail;

    for (i = 0; i < 0x40; i++)
        /* Registers */
        if (SMW_B(m, (BYTE)vic_ii.regs[i]) < 0)
            goto fail;

    if (0
        /* SbCollMask */
        || SMW_B(m, (BYTE)vic_ii.sprite_background_collisions) < 0
        /* SpriteDmaMask */
        || SMW_B(m, (BYTE)vic_ii.raster.sprite_status->dma_msk) < 0
        /* SsCollMask */
        || SMW_B(m, (BYTE)vic_ii.sprite_sprite_collisions) < 0
        /* VBank */
        || SMW_W(m, (WORD)vic_ii.vbank_phi1) < 0
        /* Vc */
        || SMW_W(m, (WORD)vic_ii.mem_counter) < 0
        /* VcInc */
        || SMW_B(m, (BYTE)vic_ii.mem_counter_inc) < 0
        /* VcBase */
        || SMW_W(m, (WORD)vic_ii.memptr) < 0
        /* VideoInt */
        || SMW_B(m, (BYTE)vic_ii.irq_status) < 0)
        goto fail;

    for (i = 0; i < 8; i++) {
        if (0
            /* SpriteXMemPtr */
            || SMW_B(m,
                (BYTE)vic_ii.raster.sprite_status->sprites[i].memptr) < 0
            /* SpriteXMemPtrInc */
            || SMW_B(m,
                (BYTE)vic_ii.raster.sprite_status->sprites[i].memptr_inc) < 0
            /* SpriteXExpFlipFlop */
            || SMW_B(m,
                (BYTE)vic_ii.raster.sprite_status->sprites[i].exp_flag) < 0)
            goto fail;
    }

    if (0
        /* FetchEventTick */
        || SMW_DW(m, vic_ii.fetch_clk - maincpu_clk) < 0
        /* FetchEventType */
        || SMW_B(m, (BYTE)vic_ii.fetch_idx) < 0)
        goto fail;

  /* Added in version 1.1 of the snapshot module */
  /* using "ram_base-ram" is F***ing bullshit - what when external memory
     is not mapped anywhere in ram[]? We should rather use some more generic
     configuration info. But as we use it above in V1.0... :-(
     AF 16jan2001 */
    if (0
        /* RamBase */
        || SMW_DW(m, (DWORD)(vic_ii.ram_base_phi2 - mem_ram)) < 0
        /* VBank */
        || SMW_W(m, (WORD)vic_ii.vbank_phi2) < 0)
        goto fail;

    return snapshot_module_close(m);

fail:
    if (m != NULL)
        snapshot_module_close(m);

    return -1;
}

int vicii_snapshot_read_module(snapshot_t *s)
{
    BYTE major_version, minor_version;
    int i;
    snapshot_module_t *m;
    BYTE color_ram[0x400];

    m = snapshot_module_open(s, snap_module_name,
                             &major_version, &minor_version);
    if (m == NULL)
        return -1;

    if (major_version > SNAP_MAJOR || minor_version > SNAP_MINOR) {
        log_error(vic_ii.log,
                  "Snapshot module version (%d.%d) newer than %d.%d.",
                  major_version, minor_version,
                  SNAP_MAJOR, SNAP_MINOR);
        goto fail;
    }

    /* FIXME: initialize changes?  */

    if (0
        /* AllowBadLines */
        || SMR_B_INT(m, &vic_ii.allow_bad_lines) < 0
        /* BadLine */
        || SMR_B_INT(m, &vic_ii.bad_line) < 0 
        /* Blank */
        || SMR_B_INT(m, &vic_ii.raster.blank_enabled) < 0
        /* ColorBuf */
        || SMR_BA(m, vic_ii.cbuf, 40) < 0
        /* ColorRam */
        || SMR_BA(m, color_ram, 1024) < 0
        /* IdleState */
        || SMR_B_INT(m, &vic_ii.idle_state) < 0
        /* LPTrigger */
        || SMR_B_INT(m, &vic_ii.light_pen.triggered) < 0
        /* LPX */
        || SMR_B_INT(m, &vic_ii.light_pen.x) < 0
        /* LPY */
        || SMR_B_INT(m, &vic_ii.light_pen.y) < 0
        /* MatrixBuf */
        || SMR_BA(m, vic_ii.vbuf, 40) < 0
        /* NewSpriteDmaMask */
        || SMR_B(m, &vic_ii.raster.sprite_status->new_dma_msk) < 0)
        goto fail;

    mem_color_ram_from_snapshot(color_ram);

    {
        DWORD RamBase;

        if (SMR_DW(m, &RamBase) < 0)
            goto fail;
        vic_ii.ram_base_phi1 = mem_ram + RamBase;
    }

    /* Read the current raster line and the current raster cycle.  As they
       are a function of `clk', this is just a sanity check.  */
    {
        WORD RasterLine;
        BYTE RasterCycle;

        if (SMR_B(m, &RasterCycle) < 0
            || SMR_W(m, &RasterLine) < 0)
            goto fail;

        if (RasterCycle != (BYTE)VIC_II_RASTER_CYCLE(maincpu_clk)) {
            log_error(vic_ii.log,
                      "Not matching raster cycle (%d) in snapshot; should be %d.",
                      RasterCycle, VIC_II_RASTER_CYCLE(maincpu_clk));
            goto fail;
        }

        if (RasterLine != (WORD)VIC_II_RASTER_Y(maincpu_clk)) {
            log_error(vic_ii.log,
                      "VIC-II: Not matching raster line (%d) in snapshot; should be %d.",
                      RasterLine, VIC_II_RASTER_Y(maincpu_clk));
            goto fail;
        }
    }

    for (i = 0; i < 0x40; i++)
        if (SMR_B_INT(m, &vic_ii.regs[i]) < 0 /* Registers */ )
            goto fail;

    if (0
        /* SbCollMask */
        || SMR_B(m, &vic_ii.sprite_background_collisions) < 0
        /* SpriteDmaMask */
        || SMR_B(m, &vic_ii.raster.sprite_status->dma_msk) < 0
        /* SsCollMask */
        || SMR_B(m, &vic_ii.sprite_sprite_collisions) < 0
        /* VBank */
        || SMR_W_INT(m, &vic_ii.vbank_phi1) < 0
        /* Vc */
        || SMR_W_INT(m, &vic_ii.mem_counter) < 0
        /* VcInc */
        || SMR_B_INT(m, &vic_ii.mem_counter_inc) < 0
        /* VcBase */
        || SMR_W_INT(m, &vic_ii.memptr) < 0
        /* VideoInt */
        || SMR_B_INT(m, &vic_ii.irq_status) < 0)
        goto fail;

    for (i = 0; i < 8; i++) {
        if (0
            /* SpriteXMemPtr */
            || SMR_B_INT(m,
                &vic_ii.raster.sprite_status->sprites[i].memptr) < 0
            /* SpriteXMemPtrInc */
            || SMR_B_INT(m,
                &vic_ii.raster.sprite_status->sprites[i].memptr_inc) < 0
            /* SpriteXExpFlipFlop */
            || SMR_B_INT(m,
                &vic_ii.raster.sprite_status->sprites[i].exp_flag) < 0
            )
            goto fail;
    }

    /* FIXME: Recalculate alarms and derived values.  */

    vicii_irq_set_raster_line(vic_ii.regs[0x12]
                              | ((vic_ii.regs[0x11] & 0x80) << 1));

    /* compatibility with older versions */
    vic_ii.ram_base_phi2 = vic_ii.ram_base_phi1;
    vic_ii.vbank_phi2 = vic_ii.vbank_phi1;

    vicii_update_memory_ptrs(VIC_II_RASTER_CYCLE(maincpu_clk));

    /* Update sprite parameters.  We had better do this manually, or the
       VIC-II emulation could be quite upset.  */
    {
        BYTE msk;

        for (i = 0, msk = 0x1; i < 8; i++, msk <<= 1) {
            raster_sprite_t *sprite;
            int tmp;

            sprite = vic_ii.raster.sprite_status->sprites + i;

            /* X/Y coordinates.  */
            tmp = vic_ii.regs[i * 2] + ((vic_ii.regs[0x10] & msk) ? 0x100 : 0);

            /* (-0xffff makes sure it's updated NOW.) */
            vicii_sprites_set_x_position(i, tmp, -0xffff);

            sprite->y = (int)vic_ii.regs[i * 2 + 1];
            sprite->x_expanded = (int)(vic_ii.regs[0x1d] & msk);
            sprite->y_expanded = (int)(vic_ii.regs[0x17] & msk);
            sprite->multicolor = (int)(vic_ii.regs[0x1c] & msk);
            sprite->in_background = (int)(vic_ii.regs[0x1b] & msk);
            sprite->color = (int) vic_ii.regs[0x27 + i] & 0xf;
            sprite->dma_flag = (int)(vic_ii.raster.sprite_status->new_dma_msk
                               & msk);
        }
    }

    vic_ii.sprite_fetch_msk = vic_ii.raster.sprite_status->new_dma_msk;
    vic_ii.sprite_fetch_clk = (VIC_II_LINE_START_CLK(maincpu_clk)
                              + vic_ii.sprite_fetch_cycle);

    vic_ii.raster.xsmooth = vic_ii.regs[0x16] & 0x7;
    vic_ii.raster.ysmooth = vic_ii.regs[0x11] & 0x7;
    vic_ii.raster.current_line = VIC_II_RASTER_Y(maincpu_clk); /* FIXME? */

    vic_ii.raster.sprite_status->visible_msk = vic_ii.regs[0x15];

    /* Update colors.  */
    vic_ii.raster.border_color = vic_ii.regs[0x20] & 0xf;
    vic_ii.raster.background_color = vic_ii.regs[0x21] & 0xf;
    vic_ii.ext_background_color[0] = vic_ii.regs[0x22] & 0xf;
    vic_ii.ext_background_color[1] = vic_ii.regs[0x23] & 0xf;
    vic_ii.ext_background_color[2] = vic_ii.regs[0x24] & 0xf;
    vic_ii.raster.sprite_status->mc_sprite_color_1 = vic_ii.regs[0x25] & 0xf;
    vic_ii.raster.sprite_status->mc_sprite_color_2 = vic_ii.regs[0x26] & 0xf;

    vic_ii.raster.blank = !(vic_ii.regs[0x11] & 0x10);

    if (VIC_II_IS_ILLEGAL_MODE (vic_ii.raster.video_mode)) {
        vic_ii.raster.overscan_background_color = 0;
        vic_ii.force_black_overscan_background_color = 1;
    } else {
        vic_ii.raster.overscan_background_color
            = vic_ii.raster.background_color;
        vic_ii.force_black_overscan_background_color = 0;
    }

    if (vic_ii.regs[0x11] & 0x8) {
        vic_ii.raster.display_ystart = vic_ii.row_25_start_line;
        vic_ii.raster.display_ystop = vic_ii.row_25_stop_line;
    } else {
        vic_ii.raster.display_ystart = vic_ii.row_24_start_line;
        vic_ii.raster.display_ystop = vic_ii.row_24_stop_line;
    }

    if (vic_ii.regs[0x16] & 0x8) {
        vic_ii.raster.display_xstart = VIC_II_40COL_START_PIXEL;
        vic_ii.raster.display_xstop = VIC_II_40COL_STOP_PIXEL;
    } else {
        vic_ii.raster.display_xstart = VIC_II_38COL_START_PIXEL;
        vic_ii.raster.display_xstop = VIC_II_38COL_STOP_PIXEL;
    }

    /* `vic_ii.raster.draw_idle_state', `vic_ii.raster.open_right_border' and
       `vic_ii.raster.open_left_border' should be needed, but they would only
       affect the current vic_ii.raster line, and would not cause any
       difference in timing.  So who cares.  */

    /* FIXME: `vic_ii.ycounter_reset_checked'?  */
    /* FIXME: `vic_ii.force_display_state'?  */

    vic_ii.memory_fetch_done = 0; /* FIXME? */

    vicii_update_video_mode(VIC_II_RASTER_CYCLE(maincpu_clk));

    vic_ii.draw_clk = maincpu_clk + (vic_ii.draw_cycle
                      - VIC_II_RASTER_CYCLE(maincpu_clk));
    vic_ii.last_emulate_line_clk = vic_ii.draw_clk - vic_ii.cycles_per_line;
    alarm_set(vic_ii.raster_draw_alarm, vic_ii.draw_clk);

    {
        DWORD dw;
        BYTE b;

        if (0
            || SMR_DW(m, &dw) < 0  /* FetchEventTick */
            || SMR_B(m, &b) < 0    /* FetchEventType */
            )
            goto fail;

        vic_ii.fetch_clk = maincpu_clk + dw;
        vic_ii.fetch_idx = b;

        alarm_set(vic_ii.raster_fetch_alarm, vic_ii.fetch_clk);
    }

    if (vic_ii.irq_status & 0x80)
        interrupt_set_irq_noclk(maincpu_int_status, vic_ii.int_num, 1);

    /* added in version 1.1 of snapshot format */
    if (minor_version > 0) {
        DWORD RamBase;

        if (0
            || SMR_DW(m, &RamBase) < 0
            || SMR_W_INT(m, &vic_ii.vbank_phi2) < 0 /* VBank */
            )
            goto fail;
        vic_ii.ram_base_phi2 = mem_ram + RamBase;

        vicii_update_memory_ptrs(VIC_II_RASTER_CYCLE(maincpu_clk));
    }

    raster_force_repaint(&vic_ii.raster);
    return 0;

fail:
    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

