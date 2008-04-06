/*
 * vicii-draw.c - Rendering for the MOS6569 (VIC-II) emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "raster-cache.h"
#include "raster-modes.h"
#include "types.h"
#include "vicii-draw.h"
#include "vicii.h"
#include "viciitypes.h"

#define GFX_MSK_LEFTBORDER_SIZE   ((VIC_II_MAX_SPRITE_WIDTH +           \
                                    vic_ii.screen_borderwidth) / 8 + 1)
#define GFX_MSK_SIZE              ((VIC_II_SCREEN_WIDTH                 \
                                    + VIC_II_MAX_SPRITE_WIDTH) / 8 + 1)

/* The following tables are used to speed up the drawing.  We do not use
   multi-dimensional arrays as we can optimize better this way...  */

/* foreground(4) | background(4) | nibble(4) -> 4 pixels.  */
static DWORD hr_table[16 * 16 * 16];

/* mc flag(1) | idx(2) | byte(8) -> index into double-pixel table.  */
static WORD mc_table[2 * 4 * 256];
static WORD mcmsktable[512];


/* These functions draw the background from `start_pixel' to `end_pixel'.  */

static void draw_std_background(int start_pixel, int end_pixel)
{
    BYTE background_color;
    unsigned int gfxstart, gfxend;

    background_color = (BYTE)vic_ii.raster.background_color;

    gfxstart = vic_ii.raster.geometry.gfx_position.x + vic_ii.raster.xsmooth;
    gfxend = gfxstart + vic_ii.raster.geometry.gfx_size.width;

    if (start_pixel < gfxstart) {
        if (end_pixel < gfxstart) {
            vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                       vic_ii.raster.xsmooth_color,
                       end_pixel - start_pixel + 1);
        } else {
            if (end_pixel < gfxend) {
                vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                           vic_ii.raster.xsmooth_color,
                           gfxstart - start_pixel + 1);
                vid_memset(vic_ii.raster.draw_buffer_ptr + gfxstart,
                           background_color,
                           end_pixel - gfxstart + 1);
            } else {
                vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                           vic_ii.raster.xsmooth_color,
                           gfxstart - start_pixel + 1);
                vid_memset(vic_ii.raster.draw_buffer_ptr + gfxstart,
                           background_color,
                           gfxend - gfxstart + 1);
                vid_memset(vic_ii.raster.draw_buffer_ptr + gfxend,
                           vic_ii.raster.xsmooth_color,
                           end_pixel - gfxend + 1);
            }
        }
    } else {
        if (start_pixel < gfxend) {
            if (end_pixel < gfxend) {
                vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                           background_color,
                           end_pixel - start_pixel + 1);
            } else {
                vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                           background_color,
                           gfxend - start_pixel + 1);
                vid_memset(vic_ii.raster.draw_buffer_ptr + gfxend,
                           vic_ii.raster.xsmooth_color,
                           end_pixel - gfxend + 1);
            }
        } else {
            vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
                       vic_ii.raster.xsmooth_color,
                       end_pixel - start_pixel + 1);
        }
    }

    if (vic_ii.raster.xsmooth_shift_right) {
        int pos;

        pos = (start_pixel - vic_ii.raster.geometry.gfx_position.x) / 8;

        if (pos >= 0 && pos < VIC_II_SCREEN_TEXTCOLS) {
            if (vic_ii.raster.video_mode == VIC_II_HIRES_BITMAP_MODE)
                background_color = vic_ii.vbuf[pos] & 0xf;
            if (vic_ii.raster.video_mode == VIC_II_EXTENDED_TEXT_MODE) {
                int bg_idx;

                bg_idx = vic_ii.vbuf[pos] >> 6;

                if (bg_idx > 0)
                    background_color = vic_ii.ext_background_color[bg_idx - 1];
            }
            vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel + 8,
                       background_color,
                       vic_ii.raster.xsmooth_shift_right);

        }
        vic_ii.raster.xsmooth_shift_right = 0;
    }
}

static void draw_idle_std_background(int start_pixel, int end_pixel)
{
    vid_memset(vic_ii.raster.draw_buffer_ptr + start_pixel,
               vic_ii.raster.overscan_background_color,
               end_pixel - start_pixel + 1);
}

/* If unaligned 32-bit access is not allowed, the graphics is stored in a
   temporary aligned buffer, and later copied to the real frame buffer.  This
   is ugly, but should be hopefully faster than accessing 8 bits at a time
   anyway.  */

#ifndef ALLOW_UNALIGNED_ACCESS
static DWORD _aligned_line_buffer[VIC_II_SCREEN_XPIX / 2 + 1];
static BYTE *const aligned_line_buffer = (BYTE *)_aligned_line_buffer;
#endif


/* Pointer to the start of the graphics area on the frame buffer.  */
#define GFX_PTR()                  \
    (vic_ii.raster.draw_buffer_ptr \
    + (vic_ii.screen_borderwidth + vic_ii.raster.xsmooth))

#ifdef ALLOW_UNALIGNED_ACCESS
#define ALIGN_DRAW_FUNC(name, xs, xe, gfx_msk_ptr) \
   name(GFX_PTR(), (xs), (xe), (gfx_msk_ptr))
#else
#define ALIGN_DRAW_FUNC(name, xs, xe, gfx_msk_ptr)           \
   do {                                                      \
       name(aligned_line_buffer, (xs), (xe), (gfx_msk_ptr)); \
       vid_memcpy(GFX_PTR() + (xs) * 8,                      \
                  aligned_line_buffer + (xs) * 8,            \
                  ((xe) - (xs) + 1) * 8);                    \
   } while (0)
#endif


/* FIXME: in the cache, we store the foreground bitmap values for the
   characters, but we do not use them when drawing and this is slow!  */

/* Standard text mode.  */

static int get_std_text(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    int r;

    if (vic_ii.raster.background_color != cache->background_data[0]
        || cache->chargen_ptr != vic_ii.chargen_ptr) {
        cache->background_data[0] = vic_ii.raster.background_color;
        cache->chargen_ptr = vic_ii.chargen_ptr;
        rr = 1;
    }

    r = raster_cache_data_fill_text(cache->foreground_data,
                                    vic_ii.vbuf,
                                    vic_ii.chargen_ptr,
                                    8,   /* FIXME */
                                    VIC_II_SCREEN_TEXTCOLS,
                                    vic_ii.raster.ycounter,
                                    xs, xe,
                                    rr);

    r |= raster_cache_data_fill(cache->color_data_1,
                                vic_ii.cbuf,
                                VIC_II_SCREEN_TEXTCOLS,
                                1,
                                xs, xe,
                                rr);

    if (!r) {
        vic_ii.sprite_sprite_collisions |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
            |= cache->sprite_background_collisions;
    }

    return r;
}

inline static void _draw_std_text(BYTE *p, int xs, int xe, BYTE *gfx_msk_ptr)
{
    DWORD *table_ptr;
    BYTE *char_ptr;
    unsigned int i;

    table_ptr = hr_table + (vic_ii.raster.background_color << 4);
    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;

    for (i = xs; i <= xe; i++) {
        DWORD *ptr = table_ptr + (vic_ii.cbuf[i] << 8);
        int d = (*(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i)
                = *(char_ptr + vic_ii.vbuf[i] * 8));

        *((DWORD *)p + i * 2) = *(ptr + (d >> 4));
        *((DWORD *)p + i * 2 + 1) = *(ptr + (d & 0xf));
    }
}

static void draw_std_text_cached(raster_cache_t *cache,
                                 int xs,
                                 int xe)
{
    ALIGN_DRAW_FUNC(_draw_std_text, xs, xe, cache->gfx_msk);
}

static void draw_std_text(void)
{
    ALIGN_DRAW_FUNC(_draw_std_text, 0,
                    VIC_II_SCREEN_TEXTCOLS - 1, vic_ii.raster.gfx_msk);
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

static void draw_std_text_foreground(int start_char, int end_char)
{
    unsigned int i;
    BYTE *char_ptr;
    BYTE *p;

    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;
    p = (vic_ii.raster.draw_buffer_ptr + vic_ii.screen_borderwidth
        + vic_ii.raster.xsmooth + 8 * start_char);

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b;
        BYTE f;

        b = char_ptr[vic_ii.vbuf[i] * 8];
        f = vic_ii.cbuf[i];
        
        if (i == end_char) {
            if (vic_ii.raster.xsmooth_shift_left > 0) {
                b = (b >> vic_ii.raster.xsmooth_shift_left) 
                    << vic_ii.raster.xsmooth_shift_left;
            }
        }

        *(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE + i) = b;
        DRAW_STD_TEXT_BYTE(p, b, f);
    }
}

/* Hires Bitmap mode.  */

static int get_hires_bitmap(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    int r;

    r = raster_cache_data_fill_nibbles(cache->color_data_1,
                                       cache->background_data,
                                       vic_ii.vbuf,
                                       VIC_II_SCREEN_TEXTCOLS,
                                       1,
                                       xs, xe,
                                       rr);
    r |= raster_cache_data_fill(cache->foreground_data,
                                (vic_ii.bitmap_ptr + vic_ii.memptr * 8
                                + vic_ii.raster.ycounter),
                                VIC_II_SCREEN_TEXTCOLS,
                                8,
                                xs, xe,
                                rr);

    if (!r) {
        vic_ii.sprite_sprite_collisions
            |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
            |= cache->sprite_background_collisions;
    }

    return r;
}

inline static void _draw_hires_bitmap(BYTE *p, int xs, int xe,
                                      BYTE *gfx_msk_ptr)
{
    BYTE *bmptr;
    unsigned int i, j;

    bmptr = vic_ii.bitmap_ptr;

    for (j = ((vic_ii.memptr << 3)
        + vic_ii.raster.ycounter + xs * 8) & 0x1fff, i = xs;
        i <= xe; i++, j = (j + 8) & 0x1fff) {

        DWORD *ptr = hr_table + (vic_ii.vbuf[i] << 4);
        int d;

        d = *(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i) = bmptr[j];
        *((DWORD *)p + i * 2) = *(ptr + (d >> 4));
        *((DWORD *)p + i * 2 + 1) = *(ptr + (d & 0xf));
    }
}

static void draw_hires_bitmap(void)
{
    ALIGN_DRAW_FUNC(_draw_hires_bitmap, 0, VIC_II_SCREEN_TEXTCOLS - 1,
                    vic_ii.raster.gfx_msk);
}

static void draw_hires_bitmap_cached(raster_cache_t *cache, int xs, int xe)
{
    ALIGN_DRAW_FUNC(_draw_hires_bitmap, xs, xe, cache->gfx_msk);
}

static void draw_hires_bitmap_foreground(int start_char, int end_char)
{
    ALIGN_DRAW_FUNC(_draw_hires_bitmap, start_char, end_char,
                    vic_ii.raster.gfx_msk);
}

/* Multicolor text mode.  */

static int get_mc_text(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    int r;

    if (vic_ii.raster.background_color != cache->background_data[0]
        || cache->color_data_1[0] != vic_ii.ext_background_color[0]
        || cache->color_data_1[1] != vic_ii.ext_background_color[1]
        || cache->chargen_ptr != vic_ii.chargen_ptr) {
        cache->background_data[0] = vic_ii.raster.background_color;
        cache->color_data_1[0] = vic_ii.ext_background_color[0];
        cache->color_data_1[1] = vic_ii.ext_background_color[1];
        cache->chargen_ptr = vic_ii.chargen_ptr;
        rr = 1;
    }

    r = raster_cache_data_fill_text(cache->foreground_data,
                                    vic_ii.vbuf,
                                    vic_ii.chargen_ptr,
                                    8,   /* FIXME */
                                    VIC_II_SCREEN_TEXTCOLS,
                                    vic_ii.raster.ycounter,
                                    xs, xe,
                                    rr);
    r |= raster_cache_data_fill(cache->color_data_3,
                                vic_ii.cbuf,
                                VIC_II_SCREEN_TEXTCOLS,
                                1,
                                xs, xe,
                                rr);

    if (!r) {
        vic_ii.sprite_sprite_collisions
          |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
          |= cache->sprite_background_collisions;
    }

    return r;
}

inline static void _draw_mc_text(BYTE *p, int xs, int xe, BYTE *gfx_msk_ptr)
{
    BYTE c[12];
    BYTE *char_ptr;
	WORD *ptmp;
    unsigned int i;

    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;

    c[1] = c[0] = vic_ii.raster.background_color;
    c[3] = c[2] = vic_ii.ext_background_color[0];
    c[5] = c[4] = vic_ii.ext_background_color[1];
    c[11] = c[8] = vic_ii.raster.background_color;

    ptmp = (WORD *)(p + xs * 8);

    for (i = xs; i <= xe; i++) {
        unsigned int d = (*(char_ptr + vic_ii.vbuf[i] * 8))
                         | ((vic_ii.cbuf[i] & 0x8) << 5);

        *(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i) = mcmsktable[d];

        c[10] = c[9] = c[7] = c[6] = vic_ii.cbuf[i] & 0x7;

        ptmp[0] = ((WORD *)c)[mc_table[        d]];
        ptmp[1] = ((WORD *)c)[mc_table[0x200 + d]];
        ptmp[2] = ((WORD *)c)[mc_table[0x400 + d]];
        ptmp[3] = ((WORD *)c)[mc_table[0x600 + d]];
		ptmp += 4;
    }
}

static void draw_mc_text(void)
{
    ALIGN_DRAW_FUNC(_draw_mc_text,
                    0, VIC_II_SCREEN_TEXTCOLS - 1,
                    vic_ii.raster.gfx_msk);
}

static void draw_mc_text_cached(raster_cache_t *cache, int xs, int xe)
{
    ALIGN_DRAW_FUNC(_draw_mc_text, xs, xe, cache->gfx_msk);
}

/* FIXME: aligned/unaligned versions.  */
#define DRAW_MC_BYTE(p, b, f1, f2, f3)      \
    do {                                    \
        if ((b) & 0x80) {                   \
          if ((b) & 0x40)                   \
            *(p) = *((p) + 1) = (f3);       \
          else                              \
            *(p) = *((p) + 1) = (f2);       \
        } else if ((b) & 0x40)              \
            *(p) = *((p) + 1) = (f1);       \
                                            \
        if ((b) & 0x20) {                   \
          if ((b) & 0x10)                   \
            *((p) + 2) = *((p) + 3) = (f3); \
          else                              \
            *((p) + 2) = *((p) + 3) = (f2); \
        } else if ((b) & 0x10)              \
            *((p) + 2) = *((p) + 3) = (f1); \
                                            \
        if ((b) & 0x08) {                   \
          if ((b) & 0x04)                   \
            *((p) + 4) = *((p) + 5) = (f3); \
          else                              \
            *((p) + 4) = *((p) + 5) = (f2); \
        } else if ((b) & 0x04)              \
            *((p) + 4) = *((p) + 5) = (f1); \
                                            \
        if ((b) & 0x02) {                   \
          if ((b) & 0x01)                   \
            *((p) + 6) = *((p) + 7) = (f3); \
          else                              \
            *((p) + 6) = *((p) + 7) = (f2); \
        } else if ((b) & 0x01)              \
            *((p) + 6) = *((p) + 7) = (f1); \
    } while (0)

static void draw_mc_text_foreground(int start_char, int end_char)
{
    BYTE *char_ptr;
    BYTE c1, c2;
    BYTE *p;
    unsigned int i;

    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;
    c1 = vic_ii.ext_background_color[0];
    c2 = vic_ii.ext_background_color[1];
    p = (vic_ii.raster.draw_buffer_ptr + vic_ii.screen_borderwidth
        + vic_ii.raster.xsmooth + 8 * start_char);

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b, c;

        b = *(char_ptr + vic_ii.vbuf[i] * 8);
        c = vic_ii.cbuf[i];

        if (c & 0x8) {
            BYTE c3;

            c3 = c & 0x7;
            DRAW_MC_BYTE (p, b, c1, c2, c3);
            *(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE + i)
                = mcmsktable[0x100 + b];
        } else {
            BYTE c3;

            c3 = c;
            DRAW_STD_TEXT_BYTE(p, b, c3);
            *(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE + i) = b;
        }
    }
}

/* Multicolor Bitmap Mode.  */

static int get_mc_bitmap(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    int r;

    if (vic_ii.raster.background_color != cache->background_data[0]) {
        cache->background_data[0] = vic_ii.raster.background_color;
        rr = 1;
    }

    r = raster_cache_data_fill_nibbles(cache->color_data_1,
                                       cache->color_data_2,
                                       vic_ii.vbuf,
                                       VIC_II_SCREEN_TEXTCOLS,
                                       1,
                                       xs, xe,
                                       rr);
    r |= raster_cache_data_fill(cache->color_data_3,
                                vic_ii.cbuf,
                                VIC_II_SCREEN_TEXTCOLS,
                                1,
                                xs, xe,
                                rr);
    r |= raster_cache_data_fill(cache->foreground_data,
                                (vic_ii.bitmap_ptr + 8 * vic_ii.memptr
                                 + vic_ii.raster.ycounter),
                                VIC_II_SCREEN_TEXTCOLS,
                                8,
                                xs, xe,
                                rr);

    if (!r) {
        vic_ii.sprite_sprite_collisions
            |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
            |= cache->sprite_background_collisions;
    }
    return r;
}

inline static void _draw_mc_bitmap(BYTE *p, int xs, int xe, BYTE *gfx_msk_ptr)
{
    BYTE *colptr, *bmptr, *ptmp;
    BYTE c[4];
    unsigned int i, j;

    colptr = vic_ii.cbuf;
    bmptr = vic_ii.bitmap_ptr;

    c[0] = vic_ii.raster.background_color;

    ptmp = p + xs * 8;

    for (j = ((vic_ii.memptr << 3) + vic_ii.raster.ycounter + xs * 8) & 0x1fff,
        i = xs; i <= xe; i++, j = (j + 8) & 0x1fff) {

        unsigned int d;

        d = bmptr[j];

        *(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i) = mcmsktable[d | 0x100];

        c[1] = vic_ii.vbuf[i] >> 4;
        c[2] = vic_ii.vbuf[i] & 0xf;
        c[3] = colptr[i];

        ptmp[1] = ptmp[0] = c[mc_table[0x100 + d]];
        ptmp[3] = ptmp[2] = c[mc_table[0x300 + d]];
        ptmp[5] = ptmp[4] = c[mc_table[0x500 + d]];
        ptmp[7] = ptmp[6] = c[mc_table[0x700 + d]];
        ptmp += 8;
    }
}

static void draw_mc_bitmap(void)
{
    ALIGN_DRAW_FUNC(_draw_mc_bitmap,
                    0, VIC_II_SCREEN_TEXTCOLS - 1,
                    vic_ii.raster.gfx_msk);
}

static void draw_mc_bitmap_cached(raster_cache_t *cache, int xs, int xe)
{
    ALIGN_DRAW_FUNC(_draw_mc_bitmap, xs, xe, cache->gfx_msk);
}

static void draw_mc_bitmap_foreground(int start_char, int end_char)
{
    BYTE *p;
    BYTE *bmptr;
    unsigned int i, j;

    p = (vic_ii.raster.draw_buffer_ptr + vic_ii.screen_borderwidth
        + vic_ii.raster.xsmooth + 8 * start_char);
    bmptr = vic_ii.bitmap_ptr;

    for (j = ((vic_ii.memptr << 3)
        + vic_ii.raster.ycounter + 8 * start_char) & 0x1fff,
        i = start_char; i <= end_char; j = (j + 8) & 0x1fff, i++, p += 8) {

        BYTE c1, c2, c3;
        BYTE b;

        c1 = vic_ii.vbuf[i] >> 4;
        c2 = vic_ii.vbuf[i] & 0xf;
        c3 = vic_ii.cbuf[i];
        b = bmptr[j];

        *(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE + i)
            = mcmsktable[0x100 + b];
        DRAW_MC_BYTE(p, b, c1, c2, c3);
    }
}

/* Extended Text Mode.  */

static int get_ext_text(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    int r;

    if (vic_ii.raster.background_color != cache->color_data_2[0]
        || vic_ii.ext_background_color[0] != cache->color_data_2[1]
        || vic_ii.ext_background_color[1] != cache->color_data_2[2]
        || vic_ii.ext_background_color[2] != cache->color_data_2[3]) {
        cache->color_data_2[0] = vic_ii.raster.background_color;
        cache->color_data_2[1] = vic_ii.ext_background_color[0];
        cache->color_data_2[2] = vic_ii.ext_background_color[1];
        cache->color_data_2[3] = vic_ii.ext_background_color[2];
        rr = 1;
    }

    r = raster_cache_data_fill(cache->color_data_1,
                               vic_ii.cbuf,
                               VIC_II_SCREEN_TEXTCOLS,
                               1,
                               xs, xe,
                               rr);
    r |= raster_cache_data_fill(cache->color_data_2,
                                vic_ii.vbuf,
                                VIC_II_SCREEN_TEXTCOLS,
                                1,
                                xs, xe,
                                rr);
    r |= raster_cache_data_fill(cache->foreground_data,
                                vic_ii.vbuf,
                                VIC_II_SCREEN_TEXTCOLS,
                                1,
                                xs, xe,
                                rr);

    if (!r) {
        vic_ii.sprite_sprite_collisions
            |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
            |= cache->sprite_background_collisions;
    }

    return r;
}

inline static void _draw_ext_text(BYTE *p, int xs, int xe, BYTE *gfx_msk_ptr)
{
    BYTE *char_ptr;
    unsigned int i;

    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;

    for (i = xs; i <= xe; i++) {
        DWORD *ptr;
        int bg_idx;
        int d;

        ptr = hr_table + (vic_ii.cbuf[i] << 8);
        bg_idx = vic_ii.vbuf[i] >> 6;
        d = *(char_ptr + (vic_ii.vbuf[i] & 0x3f) * 8);

        if (bg_idx == 0)
            ptr += vic_ii.raster.background_color << 4;
        else
            ptr += vic_ii.ext_background_color[bg_idx - 1] << 4;

        *(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i) = d;
        *((DWORD *)p + 2 * i) = *(ptr + (d >> 4));
        *((DWORD *)p + 2 * i + 1) = *(ptr + (d & 0xf));
    }
}

static void draw_ext_text(void)
{
    ALIGN_DRAW_FUNC(_draw_ext_text,
                    0, VIC_II_SCREEN_TEXTCOLS - 1,
                    vic_ii.raster.gfx_msk);
}

static void draw_ext_text_cached(raster_cache_t *cache, int xs, int xe)
{
    ALIGN_DRAW_FUNC(_draw_ext_text, xs, xe, cache->gfx_msk);
}

/* FIXME: This is *slow* and might not be 100% correct.  */
static void draw_ext_text_foreground(int start_char, int end_char)
{
    unsigned int i;
    BYTE *char_ptr;
    BYTE *p;

    char_ptr = vic_ii.chargen_ptr + vic_ii.raster.ycounter;
    p = (vic_ii.raster.draw_buffer_ptr + vic_ii.screen_borderwidth
        + vic_ii.raster.xsmooth + 8 * start_char);

    for (i = start_char; i <= end_char; i++, p += 8) {
        BYTE b;
        BYTE f;
        int bg_idx;

        b = char_ptr[(vic_ii.vbuf[i] & 0x3f) * 8];
        f = vic_ii.cbuf[i];
        bg_idx = vic_ii.vbuf[i] >> 6;

        if (bg_idx > 0) {
            p[7] = p[6] = p[5] = p[4] = p[3] = p[2] = p[1] = p[0] =
                             vic_ii.ext_background_color[bg_idx - 1];
        }

        *(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE + i) = b;
        DRAW_STD_TEXT_BYTE(p, b, f);
    }
}

/* Illegal mode.  Everything is black.  */

static int get_black(raster_cache_t *cache, int *xs, int *xe, int r)
{
    /* Let's simplify here: if also the previous time we had the Black Mode,
       nothing has changed.  If we had not, the whole line has changed.  */

    if (r) {
        *xs = 0;
        *xe = VIC_II_SCREEN_TEXTCOLS - 1;
    } else {
        vic_ii.sprite_sprite_collisions
            |= cache->sprite_sprite_collisions;
        vic_ii.sprite_background_collisions
            |= cache->sprite_background_collisions;
    }

    return r;
}

static void draw_black(void)
{
    BYTE *p;

    p = (vic_ii.raster.draw_buffer_ptr
        + vic_ii.screen_borderwidth + vic_ii.raster.xsmooth);

    vid_memset(p, 0,
               VIC_II_SCREEN_TEXTCOLS * 8);

    /* FIXME: this is not exact! */
    memset(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE,
           0, VIC_II_SCREEN_TEXTCOLS);
}

static void draw_black_cached(raster_cache_t *cache, int xs, int xe)
{
    BYTE *p;

    p = (vic_ii.raster.draw_buffer_ptr
        + vic_ii.screen_borderwidth + vic_ii.raster.xsmooth);

    vid_memset(p, 0,
               VIC_II_SCREEN_TEXTCOLS * 8);

    memset(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE,
           0, VIC_II_SCREEN_TEXTCOLS);
}

static void draw_black_foreground(int start_char, int end_char)
{
    BYTE *p;

    p = (vic_ii.raster.draw_buffer_ptr
        + (vic_ii.screen_borderwidth + vic_ii.raster.xsmooth +
        8 * start_char));

    vid_memset(p, 0, (end_char - start_char + 1) * 8);

    memset(vic_ii.raster.gfx_msk + GFX_MSK_LEFTBORDER_SIZE,
           0, VIC_II_SCREEN_TEXTCOLS);
}


/* Idle state.  */

static int get_idle(raster_cache_t *cache, int *xs, int *xe, int rr)
{
    if (rr
        || vic_ii.raster.background_color != cache->color_data_1[0]
        || vic_ii.idle_data != cache->foreground_data[0]) {
        cache->color_data_1[0] = vic_ii.raster.background_color;
        cache->foreground_data[0] = (BYTE)vic_ii.idle_data;
        *xs = 0;
        *xe = VIC_II_SCREEN_TEXTCOLS - 1;
        return 1;
    } else
        return 0;
}

inline static void _draw_idle(int xs, int xe, BYTE *gfx_msk_ptr)
{
    BYTE *p;
    BYTE d;
    unsigned int i;

    d = (BYTE)vic_ii.idle_data;

#ifdef ALLOW_UNALIGNED_ACCESS
    p = (vic_ii.raster.draw_buffer_ptr
        + vic_ii.screen_borderwidth + vic_ii.raster.xsmooth);
#else
    p = aligned_line_buffer;
#endif

    if (VIC_II_IS_TEXT_MODE(vic_ii.raster.video_mode)) {
        /* The foreground color is always black (0).  */
        unsigned int offs;
        DWORD c1, c2;

        offs = vic_ii.raster.overscan_background_color << 4;
        c1 = *(hr_table + offs + (d >> 4));
        c2 = *(hr_table + offs + (d & 0xf));

        for (i = xs * 8; i <= xe * 8; i += 8) {
            *((DWORD *)(p + i)) = c1;
            *((DWORD *)(p + i + 4)) = c2;
        }
        memset(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE, d,
               VIC_II_SCREEN_TEXTCOLS);
    } else {
        if (vic_ii.raster.video_mode == VIC_II_MULTICOLOR_BITMAP_MODE) {
            /* FIXME: Could be optimized */
            BYTE *ptmp;
            BYTE c[4];

            c[0] = vic_ii.raster.background_color;
            c[1] = 0;
            c[2] = 0;
            c[3] = 0;

            ptmp = p + xs * 8;

            for (i = xs; i <= xe; i++) {
                *(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE + i) = mcmsktable[d
                                                               | 0x100];

                ptmp[1] = ptmp[0] = c[mc_table[0x100 + d]];
                ptmp[3] = ptmp[2] = c[mc_table[0x300 + d]];
                ptmp[5] = ptmp[4] = c[mc_table[0x500 + d]];
                ptmp[7] = ptmp[6] = c[mc_table[0x700 + d]];
                ptmp += 8;
            }
        } else {
            vid_memset(p, xs * 8, (xe + 1 - xs) * 8);
            memset(gfx_msk_ptr + GFX_MSK_LEFTBORDER_SIZE, d,
                   VIC_II_SCREEN_TEXTCOLS);
        }
    }

#ifndef ALLOW_UNALIGNED_ACCESS
    vid_memcpy(vic_ii.raster.draw_buffer_ptr + (vic_ii.screen_borderwidth
               + vic_ii.raster.xsmooth),
               aligned_line_buffer + xs * 8,
               (xe - xs + 1) * 8);
#endif
}

static void draw_idle(void)
{
    _draw_idle(0, VIC_II_SCREEN_TEXTCOLS - 1, vic_ii.raster.gfx_msk);
}

static void draw_idle_cached(raster_cache_t *cache, int xs, int xe)
{
    _draw_idle(xs, xe, cache->gfx_msk);
}

static void draw_idle_foreground(int start_char, int end_char)
{
    BYTE *p;
    BYTE c;
    BYTE d;
    unsigned int i;

    p = (vic_ii.raster.draw_buffer_ptr + vic_ii.screen_borderwidth
        + vic_ii.raster.xsmooth);
    c = 0;
    d = (BYTE)vic_ii.idle_data;

    for (i = start_char; i <= end_char; i++) {
        DRAW_STD_TEXT_BYTE(p + i * 8, d, c);
        vic_ii.raster.gfx_msk[GFX_MSK_LEFTBORDER_SIZE + i] = d;
    }
}

static void setup_single_size_modes(void)
{
    raster_modes_set(vic_ii.raster.modes, VIC_II_NORMAL_TEXT_MODE,
                     get_std_text,
                     draw_std_text_cached,
                     draw_std_text,
                     draw_std_background,
                     draw_std_text_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_MULTICOLOR_TEXT_MODE,
                     get_mc_text,
                     draw_mc_text_cached,
                     draw_mc_text,
                     draw_std_background,
                     draw_mc_text_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_HIRES_BITMAP_MODE,
                     get_hires_bitmap,
                     draw_hires_bitmap_cached,
                     draw_hires_bitmap,
                     draw_std_background,
                     draw_hires_bitmap_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_MULTICOLOR_BITMAP_MODE,
                     get_mc_bitmap,
                     draw_mc_bitmap_cached,
                     draw_mc_bitmap,
                     draw_std_background,
                     draw_mc_bitmap_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_EXTENDED_TEXT_MODE,
                     get_ext_text,
                     draw_ext_text_cached,
                     draw_ext_text,
                     draw_std_background,
                     draw_ext_text_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_IDLE_MODE,
                     get_idle,
                     draw_idle_cached,
                     draw_idle,
                     draw_idle_std_background,
                     draw_idle_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_ILLEGAL_TEXT_MODE,
                     get_black,
                     draw_black_cached,
                     draw_black,
                     draw_std_background,
                     draw_black_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_ILLEGAL_BITMAP_MODE_1,
                     get_black,
                     draw_black_cached,
                     draw_black,
                     draw_std_background,
                     draw_black_foreground);

    raster_modes_set(vic_ii.raster.modes, VIC_II_ILLEGAL_BITMAP_MODE_2,
                     get_black,
                     draw_black_cached,
                     draw_black,
                     draw_std_background,
                     draw_black_foreground);
}

/* Initialize the drawing tables.  */
static void init_drawing_tables(void)
{
    DWORD i;
    unsigned int f, b;
    char tmptable[4] = { 0, 4, 5, 3 };

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
        mc_table[i + 0x100] = i >> 6;
        mc_table[i + 0x300] = (i >> 4) & 0x3;
        mc_table[i + 0x500] = (i >> 2) & 0x3;
        mc_table[i + 0x700] = i & 0x3;
        mc_table[i] = tmptable[i >> 6];
        mc_table[i + 0x200] = tmptable[(i >> 4) & 0x3];
        mc_table[i + 0x400] = tmptable[(i >> 2) & 0x3];
        mc_table[i + 0x600] = tmptable[i & 0x3];
        mcmsktable[i + 0x100] = 0;
        mcmsktable[i + 0x100] |= ((i >> 6) & 0x2) ? 0xc0 : 0;
        mcmsktable[i + 0x100] |= ((i >> 4) & 0x2) ? 0x30 : 0;
        mcmsktable[i + 0x100] |= ((i >> 2) & 0x2) ? 0x0c : 0;
        mcmsktable[i + 0x100] |= (i & 0x2) ? 0x03 : 0;
        mcmsktable[i] = i;
    }
}

void vic_ii_draw_init(void)
{
    init_drawing_tables();

    raster_set_table_refresh_handler(&vic_ii.raster, init_drawing_tables);
}

void vic_ii_draw_set_double_size(int enabled)
{
    setup_single_size_modes();
}

