/*
 * vicii-draw-cycle.c - Cycle based rendering for the VIC-II emulation.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
 *
 * Based on code written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <string.h>

#include "types.h"
#include "vicii-draw-cycle.h"
#include "viciitypes.h"
#include "viewport.h"


#define GFX_MSK_LEFTBORDER_SIZE ((VICII_MAX_SPRITE_WIDTH - VICII_RASTER_X(0) \
                                  + vicii.screen_leftborderwidth ) / 8 + 1)

/* The following tables are used to speed up the drawing.  We do not use
   multi-dimensional arrays as we can optimize better this way...  */

/* foreground(4) | background(4) | nibble(4) -> 4 pixels.  */
static DWORD hr_table[16 * 16 * 16];

/* mc flag(1) | idx(2) | byte(8) -> index into double-pixel table.  */
static BYTE mc_table[2 * 4 * 256];
static BYTE mcmsktable[512];


/* These functions draw the background from `start_pixel' to `end_pixel'.  */

static void draw_std_background(unsigned int start_pixel,
                                unsigned int end_pixel)
{
    BYTE background_color;
    unsigned int gfxstart, gfxend;

    background_color = (BYTE)vicii.raster.background_color;

    if (VICII_IS_ILLEGAL_MODE(vicii.raster.video_mode))
        background_color = 0;

    gfxstart = vicii.raster.geometry->gfx_position.x + vicii.raster.xsmooth;
    gfxend = gfxstart + vicii.raster.geometry->gfx_size.width;

    if (start_pixel < gfxstart) {
        if (end_pixel < gfxstart) {
            memset(vicii.raster.draw_buffer_ptr + start_pixel,
                   vicii.raster.xsmooth_color,
                   end_pixel - start_pixel + 1);
        } else {
            if (end_pixel < gfxend) {
                memset(vicii.raster.draw_buffer_ptr + start_pixel,
                       vicii.raster.xsmooth_color,
                       gfxstart - start_pixel);
                memset(vicii.raster.draw_buffer_ptr + gfxstart,
                       background_color,
                       end_pixel - gfxstart + 1);
            } else {
                memset(vicii.raster.draw_buffer_ptr + start_pixel,
                       vicii.raster.xsmooth_color,
                       gfxstart - start_pixel);
                memset(vicii.raster.draw_buffer_ptr + gfxstart,
                       background_color,
                       gfxend - gfxstart);
                memset(vicii.raster.draw_buffer_ptr + gfxend,
                       vicii.raster.xsmooth_color,
                       end_pixel - gfxend + 1);
            }
        }
    } else {
        if (start_pixel < gfxend) {
            if (end_pixel < gfxend) {
                memset(vicii.raster.draw_buffer_ptr + start_pixel,
                       background_color,
                       end_pixel - start_pixel + 1);
            } else {
                memset(vicii.raster.draw_buffer_ptr + start_pixel,
                       background_color,
                       gfxend - start_pixel);
                memset(vicii.raster.draw_buffer_ptr + gfxend,
                       vicii.raster.xsmooth_color,
                       end_pixel - gfxend + 1);
            }
        } else {
            memset(vicii.raster.draw_buffer_ptr + start_pixel,
                       vicii.raster.xsmooth_color,
                       end_pixel - start_pixel + 1);
        }
    }

    if (vicii.raster.xsmooth_shift_right) {
        int pos;

        pos = (start_pixel - vicii.raster.geometry->gfx_position.x) / 8;

        if (pos >= 0 && pos < VICII_SCREEN_TEXTCOLS) {
            if (vicii.raster.video_mode == VICII_HIRES_BITMAP_MODE)
                background_color = vicii.vbuf[pos] & 0xf;
            if (vicii.raster.video_mode == VICII_EXTENDED_TEXT_MODE) {
                int bg_idx;

                bg_idx = vicii.vbuf[pos] >> 6;

                if (bg_idx > 0)
                    background_color = vicii.ext_background_color[bg_idx - 1];
            }
            if (VICII_IS_ILLEGAL_MODE(vicii.raster.video_mode))
                background_color = 0;
            memset(vicii.raster.draw_buffer_ptr + start_pixel + 8,
                   background_color, vicii.raster.xsmooth_shift_right);

        }
        vicii.raster.xsmooth_shift_right = 0;
    }
}

static void draw_idle_std_background(unsigned int start_pixel,
                                     unsigned int end_pixel)
{
    memset(vicii.raster.draw_buffer_ptr + start_pixel,
           vicii.raster.idle_background_color,
           end_pixel - start_pixel + 1);
}

/* If unaligned 32-bit access is not allowed, the graphics is stored in a
   temporary aligned buffer, and later copied to the real frame buffer.  This
   is ugly, but should be hopefully faster than accessing 8 bits at a time
   anyway.  */

#ifndef ALLOW_UNALIGNED_ACCESS
static DWORD _aligned_line_buffer[VICII_SCREEN_XPIX / 2 + 1];
static BYTE *const aligned_line_buffer = (BYTE *)_aligned_line_buffer;
#endif


/* Pointer to the start of the graphics area on the frame buffer.  */
#define GFX_PTR()                 \
    (vicii.raster.draw_buffer_ptr \
    + (vicii.screen_leftborderwidth + vicii.raster.xsmooth))

#ifdef ALLOW_UNALIGNED_ACCESS
#define ALIGN_DRAW_FUNC(name, xs, xe, gfx_msk_ptr) \
   name(GFX_PTR(), (xs), (xe), (gfx_msk_ptr))
#else
#define ALIGN_DRAW_FUNC(name, xs, xe, gfx_msk_ptr)           \
   do {                                                      \
       name(aligned_line_buffer, (xs), (xe), (gfx_msk_ptr)); \
       memcpy(GFX_PTR() + (xs) * 8,                          \
              aligned_line_buffer + (xs) * 8,                \
              ((xe) - (xs) + 1) * 8);                        \
   } while (0)
#endif


/* Standard text mode.  */

inline static void _draw_std_text(BYTE *p, unsigned int xs, unsigned int xe,
                                  BYTE *gfx_msk_ptr)
{
    DWORD *table_ptr;
    BYTE *msk_ptr;
    unsigned int i;

    table_ptr = hr_table + (vicii.raster.background_color << 4);
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    for (i = xs; i <= xe; i++) {
        DWORD *ptr = table_ptr + (vicii.cbuf[i] << 8);
        int d = (*(msk_ptr + i) = vicii.gbuf[i]);

        *((DWORD *)p + i * 2) = *(ptr + (d >> 4));
        *((DWORD *)p + i * 2 + 1) = *(ptr + (d & 0xf));
    }
}

#define DRAW_STD_TEXT_BYTE(p, b, f)       \
    do {                                  \
        if ((b) & 0x80) *(p) = (f);       \
        if ((b) & 0x40) *((p) + 1) = (f); \
        if ((b) & 0x20) *((p) + 2) = (f); \
        if ((b) & 0x10) *((p) + 3) = (f); \
        if ((b) & 0x08) *((p) + 4) = (f); \
        if ((b) & 0x04) *((p) + 5) = (f); \
        if ((b) & 0x02) *((p) + 6) = (f); \
        if ((b) & 0x01) *((p) + 7) = (f); \
    } while (0)                           \

static void draw_std_text_foreground(unsigned int start_char,
                                     unsigned int end_char)
{
    unsigned int i;
    BYTE *char_ptr, *msk_ptr, *p;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;
    p = GFX_PTR() + 8 * start_char;

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b, f;

        b = vicii.gbuf[i];
        
        if (vicii.raster.last_video_mode == VICII_EXTENDED_TEXT_MODE)
            b = char_ptr[(vicii.vbuf[i - vicii.buf_offset] & 0x3f) * 8];

        if (vicii.raster.last_video_mode == VICII_HIRES_BITMAP_MODE) {
            unsigned int j = ((vicii.memptr << 3) 
                                + vicii.raster.ycounter + i * 8) & 0x1fff;
            if (j & 0x1000)
                b = vicii.bitmap_high_ptr[j & 0xfff];
            else
                b = vicii.bitmap_low_ptr[j];
        }

        f = vicii.cbuf[i];
        
        if (vicii.raster.xsmooth_shift_left > 0) {
            b = (b >> vicii.raster.xsmooth_shift_left) 
                << vicii.raster.xsmooth_shift_left;
        }

        *(msk_ptr + i) = b;
        DRAW_STD_TEXT_BYTE(p, b, f);
    }
}

/* Hires Bitmap mode.  */

inline static void _draw_hires_bitmap(BYTE *p, unsigned int xs,
                                      unsigned int xe, BYTE *gfx_msk_ptr)
{
    BYTE *msk_ptr;
    unsigned int i;

    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    for (i = xs; i <= xe; i++) {
        DWORD *ptr = hr_table + (vicii.vbuf[i] << 4);
        int d;

        d = *(msk_ptr + i) = vicii.gbuf[i];
        *((DWORD *)p + i * 2) = *(ptr + (d >> 4));
        *((DWORD *)p + i * 2 + 1) = *(ptr + (d & 0xf));
    }
}

inline static void _draw_hires_bitmap_foreground(BYTE *p, unsigned int xs,
                                                 unsigned int xe,
                                                 BYTE *gfx_msk_ptr)
{
    BYTE *bmptr_low, *bmptr_high, *msk_ptr;
    BYTE bmval;
    unsigned int i, j;

    bmptr_low = vicii.bitmap_low_ptr;
    bmptr_high = vicii.bitmap_high_ptr;
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    for (j = ((vicii.memptr << 3)
        + vicii.raster.ycounter + xs * 8) & 0x1fff, i = xs;
        i <= xe; i++, j = (j + 8) & 0x1fff) {

        DWORD *ptr = hr_table + (vicii.vbuf[i] << 4);
        int d;

        if (vicii.raster.last_video_mode == VICII_ILLEGAL_BITMAP_MODE_1)
            j &= 0x19ff;

/*
        if (j & 0x1000)
            bmval = bmptr_high[j & 0xfff];
        else
            bmval = bmptr_low[j];
*/

        bmval = vicii.gbuf[i];

        if (vicii.raster.last_video_mode == VICII_NORMAL_TEXT_MODE) {
            BYTE *char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
            bmval = char_ptr[vicii.vbuf[i] * 8];
        } 

        d = *(msk_ptr + i) = bmval;

        *((DWORD *)p + i * 2) = *(ptr + (d >> 4));
        *((DWORD *)p + i * 2 + 1) = *(ptr + (d & 0xf));
    }
}

/* Multicolor text mode.  */

inline static void _draw_mc_text(BYTE *p, unsigned int xs, unsigned int xe,
                                 BYTE *gfx_msk_ptr)
{
    BYTE c[12];
    BYTE *char_ptr, *msk_ptr;
    WORD *ptmp;
    unsigned int i;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    c[1] = c[0] = vicii.raster.background_color;
    c[3] = c[2] = vicii.ext_background_color[0];
    c[5] = c[4] = vicii.ext_background_color[1];
    c[11] = c[8] = vicii.raster.background_color;

    ptmp = (WORD *)(p + xs * 8);

    for (i = xs; i <= xe; i++) {
        unsigned int d;

        d = vicii.gbuf[i] | ((vicii.cbuf[i] & 0x8) << 5);

        *(msk_ptr + i) = mcmsktable[d];

        c[10] = c[9] = c[7] = c[6] = vicii.cbuf[i] & 0x7;

        ptmp[0] = ((WORD *)c)[mc_table[d]];
        ptmp[1] = ((WORD *)c)[mc_table[0x200 + d]];
        ptmp[2] = ((WORD *)c)[mc_table[0x400 + d]];
        ptmp[3] = ((WORD *)c)[mc_table[0x600 + d]];
        ptmp += 4;
    }
}

/* FIXME: aligned/unaligned versions.  */
#define DRAW_MC_BYTE(p, b, f1, f2, f3)          \
    do {                                        \
        if ((b) & 0x80) {                       \
            if ((b) & 0x40)                     \
                *(p) = *((p) + 1) = (f3);       \
            else                                \
                *(p) = *((p) + 1) = (f2);       \
        } else if ((b) & 0x40)                  \
            *(p) = *((p) + 1) = (f1);           \
                                                \
        if ((b) & 0x20) {                       \
            if ((b) & 0x10)                     \
                *((p) + 2) = *((p) + 3) = (f3); \
            else                                \
                *((p) + 2) = *((p) + 3) = (f2); \
        } else if ((b) & 0x10)                  \
            *((p) + 2) = *((p) + 3) = (f1);     \
                                                \
        if ((b) & 0x08) {                       \
            if ((b) & 0x04)                     \
                *((p) + 4) = *((p) + 5) = (f3); \
            else                                \
                *((p) + 4) = *((p) + 5) = (f2); \
        } else if ((b) & 0x04)                  \
            *((p) + 4) = *((p) + 5) = (f1);     \
                                                \
        if ((b) & 0x02) {                       \
            if ((b) & 0x01)                     \
                *((p) + 6) = *((p) + 7) = (f3); \
            else                                \
                *((p) + 6) = *((p) + 7) = (f2); \
        } else if ((b) & 0x01)                  \
            *((p) + 6) = *((p) + 7) = (f1);     \
    } while (0)

static void draw_mc_text_foreground(unsigned int start_char,
                                    unsigned int end_char)
{
    BYTE *char_ptr, *msk_ptr;
    BYTE c1, c2;
    BYTE *p;
    unsigned int i;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;
    c1 = vicii.ext_background_color[0];
    c2 = vicii.ext_background_color[1];
    p = GFX_PTR() + 8 * start_char;

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b, c;

        c = vicii.cbuf[i - vicii.buf_offset];
        if (vicii.raster.last_video_mode == VICII_MULTICOLOR_BITMAP_MODE) {
            unsigned int j = ((vicii.memptr << 3) 
                                + vicii.raster.ycounter + i * 8) & 0x1fff;
            if (j & 0x1000)
                b = vicii.bitmap_high_ptr[j & 0xfff];
            else
                b = vicii.bitmap_low_ptr[j];
        } else {
            b = vicii.gbuf[i];
        }

        if (c & 0x8) {
            BYTE c3;
            BYTE orig_background = *p;

            c3 = c & 0x7;
            DRAW_MC_BYTE(p, b, c1, c2, c3);
            *(msk_ptr + i) = mcmsktable[0x100 + b];

            if (vicii.raster.xsmooth_shift_left > 0) {
                int j;
                
                for (j = 0; j < vicii.raster.xsmooth_shift_left; j++)
                    *(p + 7 - j) = orig_background;

                *(msk_ptr + i) = (BYTE)((mcmsktable[0x100 + b]
                                 >> vicii.raster.xsmooth_shift_left)
                                 << vicii.raster.xsmooth_shift_left);
            }

        } else {
            BYTE c3;

            if (vicii.raster.xsmooth_shift_left > 0) {
                b = (b >> vicii.raster.xsmooth_shift_left) 
                    << vicii.raster.xsmooth_shift_left;
            }

            c3 = c;
            DRAW_STD_TEXT_BYTE(p, b, c3);
            *(msk_ptr + i) = b;
        }
    }
}

/* Multicolor Bitmap Mode.  */

inline static void _draw_mc_bitmap(BYTE *p, unsigned int xs, unsigned int xe,
                                   BYTE *gfx_msk_ptr)
{
    BYTE *colptr, *msk_ptr, *ptmp;
    BYTE c[4];
    unsigned int i;

    colptr = vicii.cbuf;
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    c[0] = vicii.raster.background_color;

    ptmp = p + xs * 8;

    for (i = xs; i <= xe; i++) {
        unsigned int d;

        d = vicii.gbuf[i];

        *(msk_ptr + i) = mcmsktable[d | 0x100];

        c[1] = vicii.vbuf[i] >> 4;
        c[2] = vicii.vbuf[i] & 0xf;
        c[3] = colptr[i];

        ptmp[1] = ptmp[0] = c[mc_table[0x100 + d]];
        ptmp[3] = ptmp[2] = c[mc_table[0x300 + d]];
        ptmp[5] = ptmp[4] = c[mc_table[0x500 + d]];
        ptmp[7] = ptmp[6] = c[mc_table[0x700 + d]];
        ptmp += 8;
    }
}

static void draw_mc_bitmap_foreground(unsigned int start_char,
                                      unsigned int end_char)
{
    BYTE *p, *bmptr_low, *bmptr_high, *msk_ptr;
    unsigned int i, j;

    p = GFX_PTR() + 8 * start_char;
    bmptr_low = vicii.bitmap_low_ptr;
    bmptr_high = vicii.bitmap_high_ptr;
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;

    for (j = ((vicii.memptr << 3)
        + vicii.raster.ycounter + 8 * start_char) & 0x1fff,
        i = start_char; i <= end_char; j = (j + 8) & 0x1fff, i++, p += 8) {

        BYTE c1, c2, c3;
        BYTE b;
        BYTE orig_background = *p;

        c1 = vicii.vbuf[i] >> 4;
        c2 = vicii.vbuf[i] & 0xf;
        c3 = vicii.cbuf[i];

        if (vicii.raster.last_video_mode == VICII_ILLEGAL_BITMAP_MODE_2)
            j &= 0x19ff;

/*
        if (j & 0x1000)
            b = bmptr_high[j & 0xfff];
        else
            b = bmptr_low[j];
*/
        b = vicii.gbuf[i];

        if (vicii.raster.last_video_mode == VICII_MULTICOLOR_TEXT_MODE
            || vicii.raster.last_video_mode == VICII_ILLEGAL_TEXT_MODE) {
            BYTE *char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
            b = char_ptr[vicii.vbuf[i - vicii.buf_offset] * 8];
        }

        *(msk_ptr + i) = mcmsktable[0x100 + b];
        DRAW_MC_BYTE(p, b, c1, c2, c3);

        if (vicii.raster.xsmooth_shift_left > 0) {
            int j;

            for (j = 0; j < vicii.raster.xsmooth_shift_left; j++)
                *(p + 7 - j) = orig_background;

            *(msk_ptr + i) = (BYTE)((mcmsktable[0x100 + b]
                             >> vicii.raster.xsmooth_shift_left)
                             << vicii.raster.xsmooth_shift_left);
        }
    }
}

/* Extended Text Mode.  */

inline static void _draw_ext_text(BYTE *p, unsigned int xs, unsigned int xe,
                                  BYTE *gfx_msk_ptr)
{
    BYTE *msk_ptr;
    unsigned int i;

    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    for (i = xs; i <= xe; i++) {
        DWORD *ptr;
        int bg_idx;
        int d;

        ptr = hr_table + (vicii.cbuf[i] << 8);
        bg_idx = vicii.vbuf[i] >> 6;
        d = vicii.gbuf[i];

        if (bg_idx == 0)
            ptr += vicii.raster.background_color << 4;
        else
            ptr += vicii.ext_background_color[bg_idx - 1] << 4;

        *(msk_ptr + i) = d;
        *((DWORD *)p + 2 * i) = *(ptr + (d >> 4));
        *((DWORD *)p + 2 * i + 1) = *(ptr + (d & 0xf));
    }
}

static void draw_ext_text_foreground(unsigned int start_char,
                                     unsigned int end_char)
{
    unsigned int i;
    BYTE *char_ptr, *msk_ptr, *p;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;
    p = GFX_PTR() + 8 * start_char;

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b, f;
        int bg_idx;

        b = char_ptr[(vicii.vbuf[i - vicii.buf_offset] & 0x3f) * 8];

        if (vicii.raster.last_video_mode == VICII_ILLEGAL_BITMAP_MODE_1) {
            unsigned int j = ((vicii.memptr << 3) 
                                + vicii.raster.ycounter + i * 8) & 0x19ff;
            if (j & 0x1000)
                b = vicii.bitmap_high_ptr[j & 0xfff];
            else
                b = vicii.bitmap_low_ptr[j];
        }

        f = vicii.cbuf[i - vicii.buf_offset];
        bg_idx = vicii.vbuf[i - vicii.buf_offset] >> 6;

        if (vicii.raster.xsmooth_shift_left > 0) {
            b = (b >> vicii.raster.xsmooth_shift_left) 
                << vicii.raster.xsmooth_shift_left;
        }

        if (bg_idx > 0) {
            p[7] = p[6] = p[5] = p[4] = p[3] = p[2] = p[1] = p[0] =
                vicii.ext_background_color[bg_idx - 1];
        }

        *(msk_ptr + i) = b;
        DRAW_STD_TEXT_BYTE(p, b, f);
    }
}

/* Illegal text mode.  */

inline static void _draw_illegal_text(BYTE *p, unsigned int xs,
                                      unsigned int xe, BYTE *gfx_msk_ptr)
{
    BYTE *char_ptr, *msk_ptr;
    unsigned int i;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    memset(p + 8 * xs, 0, (xe - xs + 1) * 8);

    for (i = xs; i <= xe; i++) {
        unsigned int d;

        d = vicii.gbuf[i]  | ((vicii.cbuf[i] & 0x8) << 5);

        *(msk_ptr + i) = mcmsktable[d];
    }
}

static void draw_illegal_text_foreground(unsigned int start_char,
                                         unsigned int end_char)
{
    BYTE *char_ptr, *msk_ptr, *p;
    unsigned int i;

    char_ptr = vicii.chargen_ptr + vicii.raster.ycounter;
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;
    p = GFX_PTR() + 8 * start_char;

    memset(p, 0, (end_char - start_char + 1) * 8);

    for (i = start_char; i <= end_char; i++) {
        unsigned int d;

        d = (*(char_ptr + (vicii.vbuf[i - vicii.buf_offset] & 0x3f) * 8))
            | ((vicii.cbuf[i - vicii.buf_offset] & 0x8) << 5);

        *(msk_ptr + i) = mcmsktable[d];
    }
}

/* Illegal bitmap mode 1.  */

inline static void _draw_illegal_bitmap_mode1(BYTE *p, unsigned int xs,
                                              unsigned int xe,
                                              BYTE *gfx_msk_ptr)
{
    BYTE *msk_ptr;
    unsigned int i;

    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    memset(p + 8 * xs, 0, (xe - xs + 1) * 8);

    for (i = xs; i <= xe; i++) {
        *(msk_ptr + i) = vicii.gbuf[i];
    }
}

/* Illegal bitmap mode 2.  */

inline static void _draw_illegal_bitmap_mode2(BYTE *p, unsigned int xs,
                                              unsigned int xe,
                                              BYTE *gfx_msk_ptr)
{
    BYTE *msk_ptr;
    unsigned int i;
    
    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;
    
    memset(p + 8 * xs, 0, (xe - xs + 1) * 8);
    
    for (i = xs; i <= xe; i++) {
        *(msk_ptr + i) = mcmsktable[vicii.gbuf[i] | 0x100];
    }
}


/* Idle state.  */

inline static void _draw_idle(BYTE *p, unsigned int xs, unsigned int xe,
                              BYTE *gfx_msk_ptr)
{
    BYTE *msk_ptr;
    BYTE d = 0;
    unsigned int i;

    msk_ptr = gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE;

    /* FIXME this is hideous and possibly broken */

    if (VICII_IS_TEXT_MODE(vicii.raster.video_mode)) {
        /* The foreground color is always black (0).  */
        unsigned int offs;
        DWORD c1, c2;

        offs = vicii.raster.idle_background_color << 4;
        for (i = xs; i <= xe; i++) {
            if (!vicii.raster.blank_enabled) {
                d = vicii.gbuf[i];
            }
            *(msk_ptr + i) = d;

            c1 = *(hr_table + offs + (d >> 4));
            c2 = *(hr_table + offs + (d & 0xf));

            *((DWORD *)(p + i * 8)) = c1;
            *((DWORD *)(p + i * 8 + 4)) = c2;
        }
    } else {
        if (vicii.raster.video_mode == VICII_MULTICOLOR_BITMAP_MODE) {
            /* FIXME: Could be optimized */
            BYTE *ptmp;
            BYTE c[4];

            c[0] = vicii.raster.background_color;
            c[1] = 0;
            c[2] = 0;
            c[3] = 0;

            ptmp = p + xs * 8;

            for (i = xs; i <= xe; i++) {
                if (!vicii.raster.blank_enabled) {
                    d = vicii.gbuf[i];
                }
                *(msk_ptr + i) = mcmsktable[d | 0x100];

                ptmp[1] = ptmp[0] = c[mc_table[0x100 + d]];
                ptmp[3] = ptmp[2] = c[mc_table[0x300 + d]];
                ptmp[5] = ptmp[4] = c[mc_table[0x500 + d]];
                ptmp[7] = ptmp[6] = c[mc_table[0x700 + d]];
                ptmp += 8;
            }
        } else {
            memset(p + xs * 8, 0, (xe + 1 - xs) * 8);
            if (vicii.raster.video_mode == VICII_ILLEGAL_BITMAP_MODE_2) {
                for (i = xs; i <= xe; i++) {
                    if (!vicii.raster.blank_enabled) {
                        d = vicii.gbuf[i];
                    }
                    *(msk_ptr + i) = mcmsktable[d | 0x100];
                }
            } else {
                for (i = xs; i <= xe; i++) {
                    if (!vicii.raster.blank_enabled) {
                        d = vicii.gbuf[i];
                    }
                    *(msk_ptr + i) = d;
                }
            }
        }
    }
}

static void draw_idle_foreground(unsigned int start_char,
                                 unsigned int end_char)
{
    BYTE *p, *msk_ptr;
    BYTE d = 0;
    unsigned int i;

    p = GFX_PTR();
    msk_ptr = vicii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE;

    for (i = start_char; i <= end_char; i++) {
        if (!vicii.raster.blank_enabled) {
            d = vicii.gbuf[i];
        }

        if (vicii.raster.xsmooth_shift_left > 0) {
            d = (d >> vicii.raster.xsmooth_shift_left) 
                << vicii.raster.xsmooth_shift_left;
        }

        DRAW_STD_TEXT_BYTE(p + i * 8, d, 0);
        *(msk_ptr + i) = d;
    }
}

/* Initialize the drawing tables.  */
static void init_drawing_tables(void)
{
    DWORD i;
    unsigned int f, b;
    const BYTE tmptable[4] = { 0, 4, 5, 3 };

    for (i = 0; i <= 0xf; i++) {
        for (f = 0; f <= 0xf; f++) {
            for (b = 0; b <= 0xf; b++) {
                BYTE fp, bp;
                BYTE *p;
                int offset;

                fp = f;
                bp = b;
                offset = (f << 8) | (b << 4);
                p = (BYTE *)(hr_table + offset + i);

                *p = i & 0x8 ? fp : bp;
                *(p + 1) = i & 0x4 ? fp : bp;
                *(p + 2) = i & 0x2 ? fp : bp;
                *(p + 3) = i & 0x1 ? fp : bp;
            }
        }
    }

    for (i = 0; i <= 0xff; i++) {
        mc_table[i + 0x100] = (BYTE)(i >> 6);
        mc_table[i + 0x300] = (BYTE)((i >> 4) & 0x3);
        mc_table[i + 0x500] = (BYTE)((i >> 2) & 0x3);
        mc_table[i + 0x700] = (BYTE)(i & 0x3);
        mc_table[i] = tmptable[i >> 6];
        mc_table[i + 0x200] = tmptable[(i >> 4) & 0x3];
        mc_table[i + 0x400] = tmptable[(i >> 2) & 0x3];
        mc_table[i + 0x600] = tmptable[i & 0x3];
        mcmsktable[i + 0x100] = (BYTE)0;
        mcmsktable[i + 0x100] |= (BYTE)(((i >> 6) & 0x2) ? 0xc0 : 0);
        mcmsktable[i + 0x100] |= (BYTE)(((i >> 4) & 0x2) ? 0x30 : 0);
        mcmsktable[i + 0x100] |= (BYTE)(((i >> 2) & 0x2) ? 0x0c : 0);
        mcmsktable[i + 0x100] |= (BYTE)((i & 0x2) ? 0x03 : 0);
        mcmsktable[i] = (BYTE)i;
    }
}

void vicii_draw_cycle(void)
{
    int cycle, i;
    BYTE vbuf, cbuf, gbuf;

    cycle = vicii.raster_cycle;
    vbuf = 0;
    cbuf = 0;
    gbuf = 0;

    /* reset rendering on raster cycle 0 */
    if (cycle == 0) {
        vicii.dbuf_offset = 0;
    }
    i = vicii.dbuf_offset;
    /* guard */
    if (i >= 65*8) 
        return;
    

    if (cycle >= 14 && cycle <= 53) {
        BYTE bg, c1, c2, c3;

        bg = vicii.regs[0x21];
        
        vbuf = vicii.vbuf[cycle - 14];
        cbuf = vicii.cbuf[cycle - 14];
        gbuf = vicii.gbuf[cycle - 14];

        switch (vicii.video_mode) {

        case VICII_NORMAL_TEXT_MODE:
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, cbuf);
            break;

        case VICII_MULTICOLOR_TEXT_MODE:
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = cbuf & 0x07;
            if (cbuf & 0x08) {
                DRAW_MC_BYTE(&vicii.dbuf[i], gbuf, c1, c2, c3);
            } else {
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, c3);
            }
            break;

        case VICII_HIRES_BITMAP_MODE:
            c1 = vbuf & 0x0f;
            c2 = vbuf >> 4;
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c1);
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, c2);
            break;

        case VICII_MULTICOLOR_BITMAP_MODE:
            c1 = vbuf >> 4;
            c2 = vbuf & 0x0f;
            c3 = cbuf;
            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
            DRAW_MC_BYTE(&vicii.dbuf[i], gbuf, c1, c2, c3);
            break;
            
        case VICII_EXTENDED_TEXT_MODE:
            c1 = vicii.ext_background_color[0];
            c2 = vicii.ext_background_color[1];
            c3 = vicii.ext_background_color[2];
            switch (vbuf & 0xc0) {
            case 0x00:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, bg);
                break;
            case 0x40:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c1);
                break;
            case 0x80:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c2);
                break;
            case 0xc0:
                DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c3);
                break;
            }

            DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], gbuf, cbuf);
            break;

        }
    } else {
        /* border */
        BYTE c = vicii.regs[0x20];
        DRAW_STD_TEXT_BYTE(&vicii.dbuf[i], 0xff, c+1);
    }
    
    vicii.dbuf_offset += 8;
}


void vicii_draw_cycle_init(void)
{
    init_drawing_tables();

    memset(vicii.dbuf, 0, 65*8); /* this should really use a define */
    vicii.dbuf_offset = 0;
}

