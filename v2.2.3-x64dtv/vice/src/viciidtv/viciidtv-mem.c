/*
 * viciidtv-mem.c - Memory interface for the VIC-II DTV emulation.
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "maindtvcpu.h"
#include "raster-sprite-status.h"
#include "raster-sprite.h"
#include "types.h"
#include "viciidtv-fetch.h"
#include "viciidtv-irq.h"
#include "viciidtv-resources.h"
#include "viciidtv-mem.h"
#include "vicii.h"
#include "viciidtvtypes.h"


/* Unused bits in VIC-II registers: these are always 1 when read.  */
static int unused_bits_in_registers[0x50] =
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
    0xff /* $D034 */ , 0xff /* $D035 */ , 0x00 /* $D036 */ , 0x00 /* $D037 */ ,
    0x00 /* $D038 */ , 0xf0 /* $D039 */ , 0x00 /* $D03A */ , 0x00 /* $D03B */ ,
    0x80 /* $D03C */ , 0x00 /* $D03D */ , 0xff /* $D03E */ , 0xfc /* $D03F */ ,
    0xf8 /* $D040 */ , 0x00 /* $D041 */ , 0x00 /* $D042 */ , 0x00 /* $D043 */ ,
    0x80 /* $D044 */ , 0xc0 /* $D045 */ , 0x00 /* $D046 */ , 0x00 /* $D047 */ ,
    0xf0 /* $D048 */ , 0x00 /* $D049 */ , 0x00 /* $D04A */ , 0xc0 /* $D04B */ ,
    0x00 /* $D04C */ , 0xc0 /* $D04D */ , 0x00 /* $D04E */ , 0xf0 /* $D04F */
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

inline static void update_raster_line(void)
{
    unsigned int new_line;

    new_line =  vicii.regs[0x12];
    new_line |= (vicii.regs[0x11] & 0x80) << 1;

    vicii.raster_irq_line = new_line;

    VICII_DEBUG_REGISTER(("Raster interrupt line set to $%04X",
                         vicii.raster_irq_line));
}

inline static void d011_store(BYTE value)
{
    VICII_DEBUG_REGISTER(("$D011 tricks at cycle %d, line $%04X, "
                          "value $%02X", vicii.raster_cycle, vicii.raster_line, value));

    vicii.ysmooth = value & 0x7;

    vicii.regs[0x11] = value;

    update_raster_line();

    vicii_update_video_mode();
}

inline static void d012_store(BYTE value)
{
    VICII_DEBUG_REGISTER(("Raster compare register: $%02X", value));

    if (value == vicii.regs[0x12]) {
        return;
    }

    vicii.regs[0x12] = value;

    update_raster_line();
}

inline static void d015_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Sprite Enable register: $%02X", value));

    vicii.regs[0x15] = value;
}

inline static void d016_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Control register: $%02X", value));

    vicii.regs[0x16] = value;

    vicii_update_video_mode();
}

inline static void d017_store(const BYTE value)
{
    int i, cycle;
    BYTE b;

    VICII_DEBUG_REGISTER(("Sprite Y Expand register: $%02X", value));

    if (value == vicii.regs[0x17]) {
        return;
    }

#if 0 /* TODO does sprite crunch work on DTV? */
    cycle = vicii.raster_cycle;

    for (i = 0, b = 0x01; i < 8; b <<= 1, i++) {
        if (!(value & b)) {
            /* sprite crunch */
            if ((vicii.regs[0x17] & b) && (cycle == VICII_PAL_CYCLE(15))) {
                BYTE mc = vicii.sprite[i].mc;
                BYTE mcbase = vicii.sprite[i].mcbase;

                /* 0x2a = 0b101010
                   0x15 = 0b010101 */
                vicii.sprite[i].mc = (0x2a & (mcbase & mc)) | (0x15 & (mcbase | mc));

                /* mcbase is set from mc on the following vicii_cycle() call */
            }
            vicii.sprite[i].exp_flop = 1;
        }
    }
#endif

    vicii.regs[0x17] = value;
}

inline static void d018_store(const BYTE value)
{
    VICII_DEBUG_REGISTER(("Memory register: $%02X", value));

    if (vicii.regs[0x18] == value) {
        return;
    }

    vicii.regs[0x18] = value;
    vicii_update_memory_ptrs();
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
    if (!vicii.extended_enable) {
        value = (vicii.regs[0x20] & 0xf0) | (value & 0x0f);
    }

    VICII_DEBUG_REGISTER(("Border color register: $%02X", value));

    vicii.regs[0x20] = value;
}

inline static void d021_store(BYTE value)
{
    if (!vicii.extended_enable) {
        value = (vicii.regs[0x21] & 0xf0) | (value & 0x0f);
    }

    VICII_DEBUG_REGISTER(("Background #0 color register: $%02X", value));

    vicii.regs[0x21] = value;
}

inline static void ext_background_store(WORD addr, BYTE value)
{
    if (!vicii.extended_enable) {
        value = (vicii.regs[addr] & 0xf0) | (value & 0x0f);
    }

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

/* HACK */
extern BYTE mem_ram[];

void viciidtv_update_colorram()
{
    vicii.color_ram_ptr = mem_ram
        + (vicii.regs[0x36] << 10)
        + ((vicii.regs[0x37] & 0x07) << 18); /* TODO test */
}

inline static void d036_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x36] = value;
        viciidtv_update_colorram();
    }

    VICII_DEBUG_REGISTER(("Color bank low: $%02x", value));
}

inline static void d037_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x37] = value;
        viciidtv_update_colorram();
    }
    VICII_DEBUG_REGISTER(("Color bank high: $%02x", value));
}

inline static void d038_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x38] = value;
        vicii.counta_mod &= 0xff00;
        vicii.counta_mod |= value;
    }

    VICII_DEBUG_REGISTER(("Linear Count A modulo low: $%02x", value));
}

inline static void d039_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x39] = value & 0xf;
        vicii.counta_mod &= 0x00ff;
        vicii.counta_mod |= ((value & 0xf) << 8);
    }

    VICII_DEBUG_REGISTER(("Linear Count A modulo high: $%02x", value));
}

inline static void d03a_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x3a] = value;
        vicii_update_memory_ptrs();
    }

    VICII_DEBUG_REGISTER(("Linear Count A Start low: $%02x", value));
}

inline static void d03b_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x3b] = value;
        vicii_update_memory_ptrs();
    }

    VICII_DEBUG_REGISTER(("Linear Count A Start middle: $%02x", value));
}

static void d03c_store(const BYTE value)
{
    if (!vicii.extended_enable) {
        return;
    }

    vicii.regs[0x3c] = value;
    vicii.badline_disable = value & 0x20 ? 1 : 0;
    vicii.colorfetch_disable = value & 0x10 ? 1: 0;
    vicii.overscan = value & 0x08 ? 1 : 0;
    vicii.high_color = value & 0x04 ? 1 : 0;
    vicii.border_off = value & 0x02 ? 1 : 0;

    vicii_update_video_mode();
    vicii_update_memory_ptrs();

    VICII_DEBUG_REGISTER(("VICIIDTV register 1: $%02x", value));
}

inline static void d03d_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x3d] = value & 0x1f;
        vicii_update_memory_ptrs();
    }

    VICII_DEBUG_REGISTER(("Graphics fetch bank: $%02x", value));
}

inline static void d03f_store(const BYTE value)
{
    if (vicii.extended_lockout) {
        return;
    }

    vicii.extended_enable = value & 0x01 ? 1 : 0;
    vicii.extended_lockout = value & 0x02 ? 1 : 0;

    vicii.regs[0x3f] = value;

    VICII_DEBUG_REGISTER(("Enable extended features: $%02x", value));
}

int vicii_extended_regs(void)
{
    return vicii.extended_enable;
}


inline static void d040_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x40] = value;
    }

    VICII_DEBUG_REGISTER(("VICIIDTV register 2: $%02x", value));
}

inline static void d044_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x44] = value;

        vicii.raster_irq_offset = value & 0x7f;
    }

    VICII_DEBUG_REGISTER(("CPU cycle/IRQ trigger cycle: $%02x", value));
}

inline static void d045_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x45] = value & 0x1f;
        viciidtv_update_colorram();
        vicii_update_memory_ptrs();
    }

    VICII_DEBUG_REGISTER(("Linear Count A Start high: $%02x", value));
}

inline static void d046_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x46] = value;
        vicii.counta_step = value;
    }

    VICII_DEBUG_REGISTER(("Linear Count A Step: $%02x", value));
}

inline static void d047_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x47] = value;
        vicii.countb_mod &= 0xff00;
        vicii.countb_mod |= value;
    }

    VICII_DEBUG_REGISTER(("Linear Count B modulo low: $%02x", value));
}

inline static void d048_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x48] = value & 0xf;
        vicii.countb_mod &= 0x00ff;
        vicii.countb_mod |= ((value & 0xf) << 8);
    }

    VICII_DEBUG_REGISTER(("Linear Count B modulo high: $%02x", value));
}

inline static void d049_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x49] = value;
    }

    VICII_DEBUG_REGISTER(("Linear Count B Start low: $%02x", value));
}

inline static void d04a_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x4a] = value;
    }

    VICII_DEBUG_REGISTER(("Linear Count B Start middle: $%02x", value));
}

inline static void d04b_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x4b] = value & 0x1f;
    }

    VICII_DEBUG_REGISTER(("Linear Count B Start high: $%02x", value));
}

inline static void d04c_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x4c] = value;
        vicii.countb_step = value;
    }

    VICII_DEBUG_REGISTER(("Linear Count B Step: $%02x", value));
}

inline static void d04d_store(const BYTE value)
{
    if (vicii.extended_enable) {
        vicii.regs[0x4d] = value & 0x1f;
    }

    VICII_DEBUG_REGISTER(("Sprite bank: $%02x", value));
}

/* DTV Palette registers at $d2xx */
void REGPARM2 vicii_palette_store(WORD addr, BYTE value)
{
    if (!vicii.extended_enable) {
        return;
    }

    if (vicii.dtvpalette[addr & 0xf] == value) {
        return;
    }

    vicii.dtvpalette[addr & 0xf] = value;
}

BYTE REGPARM1 vicii_palette_read(WORD addr)
{
    return 0;
}

/* Store a value in a VIC-II register.  */
void REGPARM2 vicii_store(WORD addr, BYTE value)
{
    if (vicii.extended_enable) {
        addr &= 0x7f;
    } else {
        addr &= 0x3f;
    }

    VICII_DEBUG_REGISTER(("WRITE $D0%02X at cycle %d of current_line $%04X",
                         addr, vicii.raster_cycle, vicii.raster_line));

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

      case 0x36:                  /* $D036: Color Bank Low */
        d036_store(value);
        break;
      case 0x37:                  /* $D037: Color Bank High */
        d037_store(value);
        break;
      case 0x38:                  /* $D038: Linear Count A Modulo Low */
        d038_store(value);
        break;
      case 0x39:                  /* $D039: Linear Count A Modulo High */
        d039_store(value);
        break;
      case 0x3a:                  /* $D03a: Linear Count A Start Low */
        d03a_store(value);
        break;
      case 0x3b:                  /* $D03B: Linear Count A Start Middle */
        d03b_store(value);
        break;
      case 0x3c:                  /* $D03C: VICIIDTV register 1 */
        d03c_store(value);
        break;
      case 0x3d:                  /* $D03D: Graphics fetch bank */
        d03d_store(value);
        break;
      case 0x3f:                  /* $D03F: Enable extended features register */
        d03f_store(value);
        break;
      case 0x40:                  /* $D040: VICIIDTV register 2 */
        d040_store(value);
        break;
      case 0x41:                  /* $D041: Burst rate modulus high */
      case 0x42:                  /* $D042: Burst rate modulus middle */
      case 0x43:                  /* $D043: Burst rate modulus low */
        VICII_DEBUG_REGISTER(("Burst rate modulus (ignored)"));
        break;
      case 0x44:                  /* $D044: CPU cycle/IRQ trigger cycle */
        d044_store(value);
        break;
      case 0x45:                  /* $D045: Linear Count A Start High */
        d045_store(value);
        break;
      case 0x46:                  /* $D046: Linear Count A Step */
        d046_store(value);
        break;
      case 0x47:                  /* $D047: Linear Count B Modulo Low */
        d047_store(value);
        break;
      case 0x48:                  /* $D048: Linear Count B Modulo High */
        d048_store(value);
        break;
      case 0x49:                  /* $D049: Linear Count B Start Low */
        d049_store(value);
        break;
      case 0x4a:                  /* $D04A: Linear Count B Start Middle */
        d04a_store(value);
        break;
      case 0x4b:                  /* $D04B: Linear Count B Start High */
        d04b_store(value);
        break;
      case 0x4c:                  /* $D04C: Linear Count B Step */
        d04c_store(value);
        break;
      case 0x4d:                  /* $D04D: Sprite bank */
        d04d_store(value);
        break;
      case 0x4e:                  /* $D04E: Scan line timing adjust */
      case 0x4f:                  /* $D04F: VICIIDTV register 3 */
        VICII_DEBUG_REGISTER(("Scan line/Saturation/Burst lock (ignored)"));
        break;
      default:
        VICII_DEBUG_REGISTER(("(unused)"));
        break;
    }
}


/* Helper function for reading from $D011/$D012.  */
inline static unsigned int read_raster_y(void)
{
    return vicii.raster_line;
}

inline static BYTE d01112_read(WORD addr)
{
    unsigned int tmp = read_raster_y();

    VICII_DEBUG_REGISTER(("Raster Line register %svalue = $%04X",
                         (addr == 0x11 ? "(highest bit) " : ""), tmp));
    if (addr == 0x11) {
        vicii.last_read = (vicii.regs[addr] & 0x7f) | ((tmp & 0x100) >> 1);
    } else {
        vicii.last_read = tmp & 0xff;
    }

    return vicii.last_read;
}


inline static BYTE d019_read(void)
{
    vicii.last_read = vicii.irq_status | 0x70;

    return vicii.last_read | ((vicii.last_read & 0xf) ? 0x80 : 0x00);
}

inline static BYTE d01e_read(void)
{
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

inline static BYTE d044_read(void)
{
    return vicii.raster_cycle | 0x80;
}

/* Read a value from a VIC-II register.  */
BYTE REGPARM1 vicii_read(WORD addr)
{
    if (vicii.extended_enable) {
        addr &= 0x7f;
    } else {
        addr &= 0x3f;
    }

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
        VICII_DEBUG_REGISTER(("Light pen X"));
        return 0;

      case 0x14:                  /* $D014: Light Pen Y */
        VICII_DEBUG_REGISTER(("Light pen Y"));
        return 0;

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
        return vicii.regs[addr];

      case 0x21:                  /* $D021: Background #0 color */
      case 0x22:                  /* $D022: Background #1 color */
      case 0x23:                  /* $D023: Background #2 color */
      case 0x24:                  /* $D024: Background #3 color */
        VICII_DEBUG_REGISTER(("Background Color #%d register: $%02X",
                             addr - 0x21, vicii.regs[addr]));
        return vicii.regs[addr];

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
      case 0x3e:                  /* $D03E: Unused */
        return 0xff;

      case 0x36:                  /* $D036: Color Bank Low */
        VICII_DEBUG_REGISTER(("Color Bank Low: $%02X",
                               vicii.regs[addr]));
        return 0xff; 

      case 0x37:                  /* $D037: Color Bank High */
        VICII_DEBUG_REGISTER(("Color Bank High: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x38:                  /* $D038: Linear Count A Modulo Low */
        VICII_DEBUG_REGISTER(("Linear Count A Modulo Low: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x39:                  /* $D039: Linear Count A Modulo High */
        VICII_DEBUG_REGISTER(("Linear Count A Modulo High: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x3a:                  /* $D03a: Linear Count A Start Low */
        VICII_DEBUG_REGISTER(("Linear Count A Start Low: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x3b:                  /* $D03B: Linear Count A Start Middle */
        VICII_DEBUG_REGISTER(("Linear Count A Start Middle: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x3c:                  /* $D03C: VICIIDTV register 1 */
        VICII_DEBUG_REGISTER(("VICIIDTV register 1: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x3d:                  /* $D03D: Graphics fetch bank */
        VICII_DEBUG_REGISTER(("Graphics fetch bank: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x3f:                  /* $D03F: Enable extended features register */
        VICII_DEBUG_REGISTER(("Enable extended features: $%02X",
                               vicii.regs[addr]));
        return 0xff;

      case 0x40:                  /* $D040: VICIIDTV register 2 */
        VICII_DEBUG_REGISTER(("VICIIDTV register 2: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x41:                  /* $D041: Burst rate modulus high */
        VICII_DEBUG_REGISTER(("Burst rate modulus high: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x42:                  /* $D042: Burst rate modulus middle */
        VICII_DEBUG_REGISTER(("Burst rate modulus middle: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x43:                  /* $D043: Burst rate modulus low */
        VICII_DEBUG_REGISTER(("Burst rate modulus low: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x44:                  /* $D044: CPU cycle/IRQ trigger cycle */
        VICII_DEBUG_REGISTER(("CPU cycle/IRQ trigger cycle: $%02X",
                             vicii.regs[addr]));
        return d044_read();

      case 0x45:                  /* $D045: Linear Count A Start High */
        VICII_DEBUG_REGISTER(("Linear Count A Start High: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x46:                  /* $D046: Linear Count A Step */
        VICII_DEBUG_REGISTER(("Linear Count A Step: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x47:                  /* $D047: Linear Count B Modulo Low */
        VICII_DEBUG_REGISTER(("Linear Count B Modulo Low: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x48:                  /* $D048: Linear Count B Modulo High */
        VICII_DEBUG_REGISTER(("Linear Count B Modulo High: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x49:                  /* $D049: Linear Count B Start Low */
        VICII_DEBUG_REGISTER(("Linear Count B Start Low: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4a:                  /* $D04A: Linear Count B Start Middle */
        VICII_DEBUG_REGISTER(("Linear Count B Start Middle: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4b:                  /* $D04B: Linear Count B Start High */
        VICII_DEBUG_REGISTER(("Linear Count B Start High: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4c:                  /* $D04C: Linear Count B Step */
        VICII_DEBUG_REGISTER(("Linear Count B Step: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4d:                  /* $D04D: Sprite bank */
        VICII_DEBUG_REGISTER(("Sprite bank: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4e:                  /* $D04E: Scan line timing adjust */
        VICII_DEBUG_REGISTER(("Scan line timing adjust: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      case 0x4f:                  /* $D04F: VICIIDTV register 3 */
        VICII_DEBUG_REGISTER(("VICIIDTV register 3: $%02X",
                             vicii.regs[addr]));
        return 0xff;

      default:
        return 0xff;
    }
    return 0xff;  /* make compiler happy */
}

inline static BYTE d019_peek(void)
{
    return vicii.irq_status | ((vicii.irq_status & 0xf) ? 0xf0 : 0x70);
}

BYTE REGPARM1 vicii_peek(WORD addr)
{
    addr &= 0x7f;

    switch (addr) {
      case 0x11:              /* $D011: video mode, Y scroll, 24/25 line mode
                                 and raster MSB */
        return (vicii.regs[addr] & 0x7f) | ((read_raster_y () & 0x100) >> 1);
      case 0x12:              /* $D012: Raster line LSB */
        return read_raster_y() & 0xff;
      case 0x13:              /* $D013: Light Pen X */
      case 0x14:              /* $D014: Light Pen Y */
        return 0;
      case 0x19:
        return d019_peek();
      case 0x1e:              /* $D01E: Sprite-sprite collision */
        return vicii.sprite_sprite_collisions;
      case 0x1f:              /* $D01F: Sprite-background collision */
        return vicii.sprite_background_collisions;
      default:
        if (addr > 0x4f) {
            return 0xff;
        }
        return vicii.regs[addr] | unused_bits_in_registers[addr];
    }
}

