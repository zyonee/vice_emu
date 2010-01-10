/*
 * vicii-mem.c - Memory interface for the MOS6569 (VIC-II) emulation.
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
#include <stdlib.h>
#include <string.h>

#include "c64cia.h"
#include "debug.h"
#include "maincpu.h"
#include "mem.h"
#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "types.h"
#include "vicii-fetch.h"
#include "vicii-irq.h"
#include "vicii-resources.h"
#include "vicii-mem.h"
#include "vicii.h"
#include "viciitypes.h"


/* Unused bits in VIC-II registers: these are always 1 when read.  */
static int unused_bits_in_registers[0x40] =
{
    0x00 /* $D000 */ , 0x00 /* $D001 */ , 0x00 /* $D002 */ , 0x00 /* $D003 */ ,
    0x00 /* $D004 */ , 0x00 /* $D005 */ , 0x00 /* $D006 */ , 0x00 /* $D007 */ ,
    0x00 /* $D008 */ , 0x00 /* $D009 */ , 0x00 /* $D00A */ , 0x00 /* $D00B */ ,
    0x00 /* $D00C */ , 0x00 /* $D00D */ , 0x00 /* $D00E */ , 0x00 /* $D00F */ ,
    0x00 /* $D010 */ , 0x00 /* $D011 */ , 0x00 /* $D012 */ , 0x00 /* $D013 */ ,
    0x00 /* $D014 */ , 0x00 /* $D015 */ , 0x00 /* $D016 */ , 0xc0 /* $D017 */ ,
    0x01 /* $D018 */ , 0x70 /* $D019 */ , 0xf0 /* $D01A */ , 0x00 /* $D01B */ ,
    0x00 /* $D01C */ , 0x00 /* $D01D */ , 0x00 /* $D01E */ , 0x00 /* $D01F */ ,
    0x00 /* $D020 */ , 0x00 /* $D021 */ , 0x00 /* $D022 */ , 0x00 /* $D023 */ ,
    0x00 /* $D024 */ , 0xf0 /* $D025 */ , 0xf0 /* $D026 */ , 0xf0 /* $D027 */ ,
    0xf0 /* $D028 */ , 0xf0 /* $D029 */ , 0xf0 /* $D02A */ , 0xf0 /* $D02B */ ,
    0xf0 /* $D02C */ , 0xf0 /* $D02D */ , 0xf0 /* $D02E */ , 0xff /* $D02F */ ,
    0xff /* $D030 */ , 0xff /* $D031 */ , 0xff /* $D032 */ , 0xff /* $D033 */ ,
    0xff /* $D034 */ , 0xff /* $D035 */ , 0xff /* $D036 */ , 0xff /* $D037 */ ,
    0xff /* $D038 */ , 0xff /* $D039 */ , 0xff /* $D03A */ , 0xff /* $D03B */ ,
    0xff /* $D03C */ , 0xff /* $D03D */ , 0xff /* $D03E */ , 0xff /* $D03F */
};


inline static void store_sprite_x_position_lsb(const WORD addr, BYTE value)
{
    int n;

    if (value == vicii.regs[addr]) {
        return;
    }

    vicii.regs[addr] = value;
    n = addr >> 1;                /* Number of changed sprite.  */

    VICII_DEBUG_REGISTER(("Sprite #%d X position LSB: $%02X", n, value));

    vicii.sprite[n].x = (value | (vicii.regs[0x10] & (1 << n) ? 0x100 : 0));
}

inline static void store_sprite_y_position(const WORD addr, BYTE value)
{
    vicii.regs[addr] = value;
}

static inline void store_sprite_x_position_msb(const WORD addr, BYTE value)
{
    int i;
    BYTE b;

    VICII_DEBUG_REGISTER(("Sprite X position MSBs: $%02X", value));

    if (value == vicii.regs[addr]) {
        return;
    }

    vicii.regs[addr] = value;

    /* Recalculate the sprite X coordinates.  */
    for (i = 0, b = 0x01; i < 8; b <<= 1, i++) {
        vicii.sprite[i].x = (vicii.regs[2 * i] | (value & b ? 0x100 : 0));
    }
}


inline static void d011_store(BYTE value)
{
    int cycle;
    unsigned int line;

    cycle = VICII_RASTER_CYCLE(maincpu_clk);
    line = VICII_RASTER_Y(maincpu_clk);

    VICII_DEBUG_REGISTER(("Control register: $%02X", value));
    VICII_DEBUG_REGISTER(("$D011 tricks at cycle %d, line $%04X, "
                          "value $%02X", cycle, line, value));

    vicii.raster_irq_line &= 0xff;
    vicii.raster_irq_line |= (value & 0x80) << 1;

    vicii.ysmooth = value & 0x7;

    vicii.regs[0x11] = value;

    /* FIXME: save time.  */
    vicii_update_video_mode(cycle);
}

inline static void d012_store(BYTE value)
{
    VICII_DEBUG_REGISTER(("Raster compare register: $%02X", value));

    if (value == vicii.regs[0x12])
        return;

    vicii.regs[0x12] = value;

    vicii.raster_irq_line &= 0x100;
    vicii.raster_irq_line |= value;

    VICII_DEBUG_REGISTER(("Raster interrupt line set to $%04X",
                         vicii.raster_irq_line));
}

inline static void d015_store(const BYTE value)
{
    vicii.regs[0x15] = value;
}

inline static void d016_store(const BYTE value)
{
    raster_t *raster;
    int cycle;

    VICII_DEBUG_REGISTER(("Control register: $%02X", value));

    raster = &vicii.raster;
    cycle = VICII_RASTER_CYCLE(maincpu_clk);

    vicii.regs[0x16] = value;

    vicii_update_video_mode(cycle);
}
const int vicii_sprites_crunch_table[64] =
{
    1,   4,   3, /* 0 */
    4,   1,   0, /* 3 */
    -1,  0,   1, /* 6 */
    4,   3,   4, /* 9 */
    1,   8,   7, /* 12 */
    8,   1,   4, /* 15 */
    3,   4,   1, /* 18 */
    0,  -1,   0, /* 21 */
    1,   4,   3, /* 24 */
    4,   1,  -8, /* 27 */
    -9, -8,   1, /* 30 */
    4,   3,   4, /* 33 */
    1,   0,  -1, /* 36 */
    0,   1,   4, /* 39 */
    3,   4,   1, /* 42 */
    8,   7,   8, /* 45 */
    1,   4,   3, /* 48 */
    4,   1,   0, /* 51 */
    -1,  0,   1, /* 54 */
    4,   3,   4, /* 57 */
    1, -40, -41, /* 60 */
    0
};

inline static void d017_store(const BYTE value)
{
    int i, cycle;
    BYTE b, mcbase, tmp;

    if (value == vicii.regs[0x17]) {
        return;
    }

    cycle = VICII_RASTER_CYCLE(maincpu_clk);

    VICII_DEBUG_REGISTER(("Sprite Y Expand register: $%02X", value));

    for (i = 0, b = 0x01; i < 8; b <<= 1, i++) {
        if (!(value & b)) {
            /* sprite crunch */
            if (vicii.regs[0x17] & b && vicii.sprite[i].exp_flop && cycle == 15) {
#if 0
                mcbase = vicii.sprite[i].mcbase - 2;
                if (mcbase != 0x3f && (mcbase & 0x0f) > 0x0c) {
                    if ((mcbase & 0x30) == 0x30) {
                        mcbase &= ~0x20;
                    }
                    mcbase |= 0x10;
                    mcbase &= ~0x08;
                }
                tmp = mcbase & 3;
                if (tmp == 1) {
                    tmp = 2;
                }
                mcbase |= tmp << 1;
                mcbase = mcbase - 4 & 0x3f;
                vicii.sprite[i].mcbase = mcbase;
#else
                /* undo the progress made in cycle 14 in sprite_exp_inc() */
                vicii.sprite[i].mcbase -= 2;
                /* apply the magic crunch */
                vicii.sprite[i].mcbase += vicii_sprites_crunch_table[vicii.sprite[i].mcbase];
                /* undo the progress that will follow in cycle 15 in sprite_exp_inc() */
                vicii.sprite[i].mcbase--;
#endif
            }

            vicii.sprite[i].exp_flop = 1;
#ifdef DEBUG
            if (debug.maincpu_traceflg) {
                log_debug("D017_store. exp_flop=1, New mcbase for sprite %d: %d",i, vicii.sprite[i].mcbase);
            }
#endif
        }
    }

    vicii.regs[0x17] = value;
}

inline static void d018_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Memory register: $%02X", value));

    if (vicii.regs[0x18] == value) {
        return;
    }

    vicii.regs[0x18] = value;
    vicii_update_memory_ptrs(VICII_RASTER_CYCLE(maincpu_clk));
}

inline static void d019_store(const BYTE value)
{
    vicii.irq_status &= ~((value & 0xf) | 0x80);
    vicii_irq_set_line();

    VICII_DEBUG_REGISTER(("IRQ flag register: $%02X", vicii.irq_status));
}

inline static void d01a_store(const BYTE value)
{
    vicii.regs[0x1a] = value & 0xf;

    vicii_irq_set_line();

    VICII_DEBUG_REGISTER(("IRQ mask register: $%02X", vicii.regs[0x1a]));
}

inline static void d01b_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Sprite priority register: $%02X", value));

    vicii.regs[0x1b] = value;
}

inline static void d01c_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Sprite Multicolor Enable register: $%02X", value));

    vicii.regs[0x1c] = value;
}

inline static void d01d_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Sprite X Expand register: $%02X", value));

    vicii.regs[0x1d] = value;
}

inline static void collision_store(const WORD addr, const BYTE value)
{
    VICII_DEBUG_REGISTER(("(collision register, Read Only)"));
}

inline static void d020_store(BYTE value)
{
    VICII_DEBUG_REGISTER(("Border color register: $%02X", value));

    value &= 0x0f;

    vicii.regs[0x20] = value;

}

inline static void d021_store(BYTE value)
{
    VICII_DEBUG_REGISTER(("Background #0 color register: $%02X", value));

    value &= 0x0f;

    vicii.regs[0x21] = value;
}

inline static void ext_background_store(WORD addr, BYTE value)
{
    int char_num;

    value &= 0x0f;

    VICII_DEBUG_REGISTER(("Background color #%d register: $%02X",
                          addr - 0x21, value));

    vicii.regs[addr] = value;
}

inline static void d025_store(BYTE value)
{
    value &= 0xf;

    VICII_DEBUG_REGISTER(("Sprite multicolor register #0: $%02X", value));

    vicii.regs[0x25] = value;
}

inline static void d026_store(BYTE value)
{
    value &= 0xf;

    VICII_DEBUG_REGISTER(("Sprite multicolor register #1: $%02X", value));

    vicii.regs[0x26] = value;
}

inline static void sprite_color_store(WORD addr, BYTE value)
{
    value &= 0xf;

    VICII_DEBUG_REGISTER(("Sprite #%d color register: $%02X",
                         addr - 0x27, value));

    vicii.regs[addr] = value;
}

/* Store a value in a VIC-II register.  */
void REGPARM2 vicii_store(WORD addr, BYTE value)
{
    addr &= 0x3f;

#if 0
    vicii_handle_pending_alarms_external_write();

    /* This is necessary as we must be sure that the previous line has been
       updated and `current_line' is actually set to the current Y position of
       the raster.  Otherwise we might mix the changes for this line with the
       changes for the previous one.  */
    if (maincpu_clk >= vicii.draw_clk)
        vicii_raster_draw_alarm_handler(maincpu_clk - vicii.draw_clk, NULL);
#endif

    VICII_DEBUG_REGISTER(("WRITE $D0%02X at cycle %d of current_line $%04X",
                         addr, VICII_RASTER_CYCLE(maincpu_clk),
                         VICII_RASTER_Y(maincpu_clk)));

    switch (addr) {
      case 0x0:                   /* $D000: Sprite #0 X position LSB */
      case 0x2:                   /* $D002: Sprite #1 X position LSB */
      case 0x4:                   /* $D004: Sprite #2 X position LSB */
      case 0x6:                   /* $D006: Sprite #3 X position LSB */
      case 0x8:                   /* $D008: Sprite #4 X position LSB */
      case 0xa:                   /* $D00a: Sprite #5 X position LSB */
      case 0xc:                   /* $D00c: Sprite #6 X position LSB */
      case 0xe:                   /* $D00e: Sprite #7 X position LSB */
        store_sprite_x_position_lsb(addr, value);
        break;

      case 0x1:                   /* $D001: Sprite #0 Y position */
      case 0x3:                   /* $D003: Sprite #1 Y position */
      case 0x5:                   /* $D005: Sprite #2 Y position */
      case 0x7:                   /* $D007: Sprite #3 Y position */
      case 0x9:                   /* $D009: Sprite #4 Y position */
      case 0xb:                   /* $D00B: Sprite #5 Y position */
      case 0xd:                   /* $D00D: Sprite #6 Y position */
      case 0xf:                   /* $D00F: Sprite #7 Y position */
        store_sprite_y_position(addr, value);
        break;

      case 0x10:                  /* $D010: Sprite X position MSB */
        store_sprite_x_position_msb(addr, value);
        break;

      case 0x11:                  /* $D011: video mode, Y scroll, 24/25 line
                                     mode and raster MSB */
        d011_store(value);
        break;

      case 0x12:                  /* $D012: Raster line compare */
        d012_store(value);
        break;

      case 0x13:                  /* $D013: Light Pen X */
      case 0x14:                  /* $D014: Light Pen Y */
        break;

      case 0x15:                  /* $D015: Sprite Enable */
        d015_store(value);
        break;

      case 0x16:                  /* $D016 */
        d016_store(value);
        break;

      case 0x17:                  /* $D017: Sprite Y-expand */
        d017_store(value);
        break;

      case 0x18:                  /* $D018: Video and char matrix base
                                     address */
        d018_store(value);
        break;

      case 0x19:                  /* $D019: IRQ flag register */
        d019_store(value);
        break;

      case 0x1a:                  /* $D01A: IRQ mask register */
        d01a_store(value);
        break;

      case 0x1b:                  /* $D01B: Sprite priority */
        d01b_store(value);
        break;

      case 0x1c:                  /* $D01C: Sprite Multicolor select */
        d01c_store(value);
        break;

      case 0x1d:                  /* $D01D: Sprite X-expand */
        d01d_store(value);
        break;

      case 0x1e:                  /* $D01E: Sprite-sprite collision */
      case 0x1f:                  /* $D01F: Sprite-background collision */
        collision_store(addr, value);
        break;

      case 0x20:                  /* $D020: Border color */
        d020_store(value);
        break;

      case 0x21:                  /* $D021: Background #0 color */
        d021_store(value);
        break;

      case 0x22:                  /* $D022: Background #1 color */
      case 0x23:                  /* $D023: Background #2 color */
      case 0x24:                  /* $D024: Background #3 color */
        ext_background_store(addr, value);
        break;

      case 0x25:                  /* $D025: Sprite multicolor register #0 */
        d025_store(value);
        break;

      case 0x26:                  /* $D026: Sprite multicolor register #1 */
        d026_store(value);
        break;

      case 0x27:                  /* $D027: Sprite #0 color */
      case 0x28:                  /* $D028: Sprite #1 color */
      case 0x29:                  /* $D029: Sprite #2 color */
      case 0x2a:                  /* $D02A: Sprite #3 color */
      case 0x2b:                  /* $D02B: Sprite #4 color */
      case 0x2c:                  /* $D02C: Sprite #5 color */
      case 0x2d:                  /* $D02D: Sprite #6 color */
      case 0x2e:                  /* $D02E: Sprite #7 color */
        sprite_color_store(addr, value);
        break;

      case 0x2f:                  /* $D02F: Unused */
      case 0x30:                  /* $D030: Unused */
      case 0x31:                  /* $D031: Unused */
      case 0x32:                  /* $D032: Unused */
      case 0x33:                  /* $D033: Unused */
      case 0x34:                  /* $D034: Unused */
      case 0x35:                  /* $D035: Unused */
      case 0x36:                  /* $D036: Unused */
      case 0x37:                  /* $D037: Unused */
      case 0x38:                  /* $D038: Unused */
      case 0x39:                  /* $D039: Unused */
      case 0x3a:                  /* $D03A: Unused */
      case 0x3b:                  /* $D03B: Unused */
      case 0x3c:                  /* $D03C: Unused */
      case 0x3d:                  /* $D03D: Unused */
      case 0x3e:                  /* $D03E: Unused */
      case 0x3f:                  /* $D03F: Unused */
        VICII_DEBUG_REGISTER(("(unused)"));
        break;
    }
}


/* Helper function for reading from $D011/$D012.  */
inline static unsigned int read_raster_y(void)
{
    int raster_y;

    raster_y = vicii.raster_line;

    /* Line 0 is 62 cycles long, while line (SCREEN_HEIGHT - 1) is 64
       cycles long.  As a result, the counter is incremented one
       cycle later on line 0.  */
    if (raster_y == 0 && vicii.raster_cycle == 0) {
        raster_y = vicii.screen_height - 1;
    }

    return raster_y;
}

inline static BYTE d01112_read(WORD addr)
{
    unsigned int tmp = read_raster_y();

    VICII_DEBUG_REGISTER(("Raster Line register %svalue = $%04X",
                         (addr == 0x11 ? "(highest bit) " : ""), tmp));
    if (addr == 0x11)
        vicii.last_read = (vicii.regs[addr] & 0x7f) | ((tmp & 0x100) >> 1);
    else
        vicii.last_read = tmp & 0xff;

    return vicii.last_read;
}


inline static BYTE d019_read(void)
{
    vicii.last_read = vicii.irq_status | 0x70;
    return vicii.last_read;
}

inline static BYTE d01e_read(void)
{
    /* Remove the pending sprite-sprite interrupt, as the collision
       register is reset upon read accesses.  */
    vicii_irq_sscoll_clear();

    if (!vicii_resources.sprite_sprite_collisions_enabled) {
        VICII_DEBUG_REGISTER(("Sprite-sprite collision mask: $00 "
                             "(emulation disabled)"));
        vicii.sprite_sprite_collisions = 0;
        return 0;
    }

    vicii.regs[0x1e] = vicii.sprite_sprite_collisions;
    vicii.sprite_sprite_collisions = 0;
    VICII_DEBUG_REGISTER(("Sprite-sprite collision mask: $%02X",
                         vicii.regs[0x1e]));

    return vicii.regs[0x1e];
}

inline static BYTE d01f_read(void)
{
    /* Remove the pending sprite-background interrupt, as the collision
       register is reset upon read accesses.  */
    vicii_irq_sbcoll_clear();

    if (!vicii_resources.sprite_background_collisions_enabled) {
        VICII_DEBUG_REGISTER(("Sprite-background collision mask: $00 "
                             "(emulation disabled)"));
        vicii.sprite_background_collisions = 0;
        return 0;
    }

    vicii.regs[0x1f] = vicii.sprite_background_collisions;
    vicii.sprite_background_collisions = 0;
    VICII_DEBUG_REGISTER(("Sprite-background collision mask: $%02X",
                          vicii.regs[0x1f]));

#if defined (VICII_DEBUG_SB_COLLISIONS)
    log_message(vicii.log,
                "vicii.sprite_background_collisions reset by $D01F "
                "read at line 0x%X.",
                VICII_RASTER_Y(clk));
#endif

    return vicii.regs[0x1f];
}

/* Read a value from a VIC-II register.  */
BYTE REGPARM1 vicii_read(WORD addr)
{
    addr &= 0x3f;

#if 0
    /* Serve all pending events.  */
    vicii_handle_pending_alarms(0);
#endif

    VICII_DEBUG_REGISTER(("READ $D0%02X at cycle %d of current_line $%04X:",
                         addr, VICII_RASTER_CYCLE(maincpu_clk),
                         VICII_RASTER_Y(maincpu_clk)));

    /* Note: we use hardcoded values instead of `unused_bits_in_registers[]'
       here because this is a little bit faster.  */
    switch (addr) {
      case 0x0:                   /* $D000: Sprite #0 X position LSB */
      case 0x2:                   /* $D002: Sprite #1 X position LSB */
      case 0x4:                   /* $D004: Sprite #2 X position LSB */
      case 0x6:                   /* $D006: Sprite #3 X position LSB */
      case 0x8:                   /* $D008: Sprite #4 X position LSB */
      case 0xa:                   /* $D00a: Sprite #5 X position LSB */
      case 0xc:                   /* $D00c: Sprite #6 X position LSB */
      case 0xe:                   /* $D00e: Sprite #7 X position LSB */
        VICII_DEBUG_REGISTER(("Sprite #%d X position LSB: $%02X",
                             addr >> 1, vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x1:                   /* $D001: Sprite #0 Y position */
      case 0x3:                   /* $D003: Sprite #1 Y position */
      case 0x5:                   /* $D005: Sprite #2 Y position */
      case 0x7:                   /* $D007: Sprite #3 Y position */
      case 0x9:                   /* $D009: Sprite #4 Y position */
      case 0xb:                   /* $D00B: Sprite #5 Y position */
      case 0xd:                   /* $D00D: Sprite #6 Y position */
      case 0xf:                   /* $D00F: Sprite #7 Y position */
        VICII_DEBUG_REGISTER(("Sprite #%d Y position: $%02X",
                             addr >> 1, vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x10:                  /* $D010: Sprite X position MSB */
        VICII_DEBUG_REGISTER(("Sprite X position MSB: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x11:                /* $D011: video mode, Y scroll, 24/25 line mode
                                   and raster MSB */
      case 0x12:                /* $D012: Raster line compare */
        return d01112_read(addr);

      case 0x13:                  /* $D013: Light Pen X */
        VICII_DEBUG_REGISTER(("Light pen X: %d", vicii.light_pen.x));
        return vicii.light_pen.x;

      case 0x14:                  /* $D014: Light Pen Y */
        VICII_DEBUG_REGISTER(("Light pen Y: %d", vicii.light_pen.y));
        return vicii.light_pen.y;

      case 0x15:                  /* $D015: Sprite Enable */
        VICII_DEBUG_REGISTER(("Sprite Enable register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x16:                  /* $D016 */
        VICII_DEBUG_REGISTER(("$D016 Control register read: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr] | 0xc0;

      case 0x17:                  /* $D017: Sprite Y-expand */
        VICII_DEBUG_REGISTER(("Sprite Y Expand register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x18:                /* $D018: Video and char matrix base address */
        VICII_DEBUG_REGISTER(("Video memory address register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr] | 0x1;

      case 0x19:                  /* $D019: IRQ flag register */
        {
            BYTE tmp;

            tmp = d019_read();
            VICII_DEBUG_REGISTER(("Interrupt register: $%02X", tmp));

            return tmp;
        }

      case 0x1a:                  /* $D01A: IRQ mask register  */
        VICII_DEBUG_REGISTER(("Mask register: $%02X",
                             vicii.regs[addr] | 0xf0));
        return vicii.regs[addr] | 0xf0;

      case 0x1b:                  /* $D01B: Sprite priority */
        VICII_DEBUG_REGISTER(("Sprite Priority register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x1c:                  /* $D01C: Sprite Multicolor select */
        VICII_DEBUG_REGISTER(("Sprite Multicolor Enable register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x1d:                  /* $D01D: Sprite X-expand */
        VICII_DEBUG_REGISTER(("Sprite X Expand register: $%02X",
                             vicii.regs[addr]));
        return vicii.regs[addr];

      case 0x1e:                  /* $D01E: Sprite-sprite collision */
        return d01e_read();

      case 0x1f:                  /* $D01F: Sprite-background collision */
        return d01f_read();

      case 0x20:                  /* $D020: Border color */
        VICII_DEBUG_REGISTER(("Border Color register: $%02X",
                             vicii.regs[addr]));
        return (vicii.regs[addr] | 0xf0);

      case 0x21:                  /* $D021: Background #0 color */
      case 0x22:                  /* $D022: Background #1 color */
      case 0x23:                  /* $D023: Background #2 color */
      case 0x24:                  /* $D024: Background #3 color */
        VICII_DEBUG_REGISTER(("Background Color #%d register: $%02X",
                             addr - 0x21, vicii.regs[addr]));
        return (vicii.regs[addr] | 0xf0);

      case 0x25:                  /* $D025: Sprite multicolor register #0 */
      case 0x26:                  /* $D026: Sprite multicolor register #1 */
        VICII_DEBUG_REGISTER(("Multicolor register #%d: $%02X",
                             addr - 0x22, vicii.regs[addr]));
        return vicii.regs[addr] | 0xf0;

      case 0x27:                  /* $D027: Sprite #0 color */
      case 0x28:                  /* $D028: Sprite #1 color */
      case 0x29:                  /* $D029: Sprite #2 color */
      case 0x2a:                  /* $D02A: Sprite #3 color */
      case 0x2b:                  /* $D02B: Sprite #4 color */
      case 0x2c:                  /* $D02C: Sprite #5 color */
      case 0x2d:                  /* $D02D: Sprite #6 color */
      case 0x2e:                  /* $D02E: Sprite #7 color */
        VICII_DEBUG_REGISTER(("Sprite #%d color: $%02X",
                             addr - 0x22, vicii.regs[addr]));
        return vicii.regs[addr] | 0xf0;

      case 0x2f:                  /* $D02F: Unused */
      case 0x30:                  /* $D030: Unused */
      case 0x31:                  /* $D031: Unused */
      case 0x32:                  /* $D032: Unused */
      case 0x33:                  /* $D033: Unused */
      case 0x34:                  /* $D034: Unused */
      case 0x35:                  /* $D035: Unused */
      case 0x36:                  /* $D036: Unused */
      case 0x37:                  /* $D037: Unused */
      case 0x38:                  /* $D038: Unused */
      case 0x39:                  /* $D039: Unused */
      case 0x3a:                  /* $D03A: Unused */
      case 0x3b:                  /* $D03B: Unused */
      case 0x3c:                  /* $D03C: Unused */
      case 0x3d:                  /* $D03D: Unused */
      case 0x3e:                  /* $D03E: Unused */
      case 0x3f:                  /* $D03F: Unused */
      default:
        return 0xff;
    }
    return 0xff;  /* make compiler happy */
}

inline static BYTE d019_peek(void)
{
    return vicii.irq_status | 0x70;
}

BYTE REGPARM1 vicii_peek(WORD addr)
{
    addr &= 0x3f;

    switch (addr) {
      case 0x11:              /* $D011: video mode, Y scroll, 24/25 line mode
                                 and raster MSB */
        return (vicii.regs[addr] & 0x7f) | ((read_raster_y () & 0x100) >> 1);
      case 0x12:              /* $D012: Raster line LSB */
        return read_raster_y() & 0xff;
      case 0x13:              /* $D013: Light Pen X */
        return vicii.light_pen.x;
      case 0x14:              /* $D014: Light Pen Y */
        return vicii.light_pen.y;
      case 0x19:
        return d019_peek();
      case 0x1e:              /* $D01E: Sprite-sprite collision */
        return vicii.sprite_sprite_collisions;
      case 0x1f:              /* $D01F: Sprite-background collision */
        return vicii.sprite_background_collisions;
      default:
        return vicii.regs[addr] | unused_bits_in_registers[addr];
    }
}

