/*
 * vicii-resources.c - Resources for the MOS 6569 (VIC-II) emulation.
 *
 * Written by
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

#include "resources.h"
#include "utils.h"
#include "vicii-resources.h"
#include "vicii.h"
#include "viciitypes.h"
#ifdef USE_XF86_EXTENSIONS
#include "fullscreen.h"
#endif


#ifdef __MSDOS__
#define DEFAULT_VideoCache_VALUE 0
#else
#define DEFAULT_VideoCache_VALUE 1
#endif


vic_ii_resources_t vic_ii_resources;

int vic_ii_activate_palette(void)
{
    if (vic_ii_resources.ext_palette) {
        /* external palette file */
        return vic_ii_load_palette (vic_ii_resources.palette_file_name);
    } else {
        /* calculated palette */
        vic_ii_calc_palette(vic_ii_resources.color_saturation,
                            vic_ii_resources.color_contrast,
                            vic_ii_resources.color_brightness,
                            vic_ii_resources.color_gamma,
                            vic_ii_resources.new_luminances,
                            vic_ii_resources.fast_delayloop_emulation);
    }
    return 0;
}

static int set_color_saturation(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    vic_ii_resources.color_saturation = val;
    return vic_ii_activate_palette();
}

static int set_color_contrast(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    vic_ii_resources.color_contrast = val;
    return vic_ii_activate_palette();
}

static int set_color_brightness(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    vic_ii_resources.color_brightness = val;
    return vic_ii_activate_palette();
}

static int set_color_gamma(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val=0;
    if (val > 2000)
        val=2000;
    vic_ii_resources.color_gamma = val;
    return vic_ii_activate_palette();
}

static int set_new_luminances(resource_value_t v, void *param)
{
    vic_ii_resources.new_luminances = (int)v;
    return vic_ii_activate_palette();
}

static int set_ext_palette(resource_value_t v, void *param)
{
    vic_ii_resources.ext_palette = (int)v;
    return vic_ii_activate_palette();
}

static int set_fast_delayloop_emulation(resource_value_t v, void *param)
{
    vic_ii_resources.fast_delayloop_emulation = (int)v;
    return vic_ii_activate_palette();
}

static int set_pal_emulation(resource_value_t v, void *param)
{
    vic_ii_resources.pal_emulation = (int)v;
    return vic_ii_activate_palette();
}

static int set_sprite_sprite_collisions_enabled(resource_value_t v, void *param)
{
    vic_ii_resources.sprite_sprite_collisions_enabled = (int)v;
    return 0;
}

static int set_sprite_background_collisions_enabled(resource_value_t v,
                                                    void *param)
{
    vic_ii_resources.sprite_background_collisions_enabled = (int)v;
    return 0;
}

static int set_video_cache_enabled (resource_value_t v, void *param)
{
    vic_ii_resources.video_cache_enabled = (int)v;
    if (vic_ii.initialized)
        raster_enable_cache(&vic_ii.raster,
                            vic_ii_resources.video_cache_enabled);

    return 0;
}

static int set_palette_file_name(resource_value_t v, void *param)
{
    util_string_set(&vic_ii_resources.palette_file_name, (char *)v);
    return vic_ii_activate_palette();
}

static resource_t resources[] =
{
    { "ColorSaturation", RES_INTEGER, (resource_value_t)1000,
      (resource_value_t *)&vic_ii_resources.color_saturation,
      set_color_saturation, NULL },
    { "ColorContrast", RES_INTEGER, (resource_value_t)1100,
      (resource_value_t *)&vic_ii_resources.color_contrast,
      set_color_contrast, NULL },
    { "ColorBrightness", RES_INTEGER, (resource_value_t)1100,
      (resource_value_t *)&vic_ii_resources.color_brightness,
      set_color_brightness, NULL },
    { "ColorGamma", RES_INTEGER, (resource_value_t)900,
      (resource_value_t *)&vic_ii_resources.color_gamma,
      set_color_gamma, NULL },
    { "NewLuminances", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&vic_ii_resources.new_luminances,
      set_new_luminances, NULL },
    { "ExternalPalette", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.ext_palette,
      set_ext_palette, NULL },
    { "DelayLoopEmulation", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.fast_delayloop_emulation,
      set_fast_delayloop_emulation, NULL },
    { "PALEmulation", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.pal_emulation,
      set_pal_emulation, NULL },
    { "CheckSsColl", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&vic_ii_resources.sprite_sprite_collisions_enabled,
      set_sprite_sprite_collisions_enabled, NULL },
    { "CheckSbColl", RES_INTEGER, (resource_value_t)1,
      (resource_value_t *)&vic_ii_resources.sprite_background_collisions_enabled,
      set_sprite_background_collisions_enabled, NULL },
    { "PaletteFile", RES_STRING, (resource_value_t)"default",
      (resource_value_t *)&vic_ii_resources.palette_file_name,
      set_palette_file_name, NULL },
    { "VideoCache", RES_INTEGER, (resource_value_t)DEFAULT_VideoCache_VALUE,
      (resource_value_t *)&vic_ii_resources.video_cache_enabled,
      set_video_cache_enabled, NULL },
    { NULL }
};


#ifdef VIC_II_NEED_2X

static int set_double_size_enabled(resource_value_t v, void *param)
{
    vic_ii_resources.double_size_enabled = (int)v;
#ifdef USE_XF86_EXTENSIONS
    if (!fullscreen_is_enabled)
#endif
        vic_ii_resize ();

    return 0;
}

static int set_double_scan_enabled (resource_value_t v, void *param)
{
    vic_ii_resources.double_scan_enabled = (int)v;
#ifdef USE_XF86_EXTENSIONS
    if (vic_ii.initialized && !fullscreen_is_enabled)
#else
    if (vic_ii.initialized)
#endif
        raster_enable_double_scan(&vic_ii.raster,
                                  vic_ii_resources.double_scan_enabled);

    return 0;
}

#ifdef USE_XF86_EXTENSIONS
static int set_fullscreen_double_size_enabled(resource_value_t v, void *param)
{
    vic_ii_resources.fullscreen_double_size_enabled = (int)v;
    if (fullscreen_is_enabled)
        vic_ii_resize();
    return 0;
}

static int
set_fullscreen_double_scan_enabled(resource_value_t v, void *param)
{
    vic_ii_resources.fullscreen_double_scan_enabled = (int)v;
    if (fullscreen_is_enabled && vic_ii.initialized)
        raster_enable_double_scan(&vic_ii.raster,
                                  vic_ii_resources.fullscreen_double_scan_enabled);
    return 0;
}
#endif

static resource_t resources_2x[] =
{
    { "DoubleSize", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.double_size_enabled,
      set_double_size_enabled, NULL },
    { "DoubleScan", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.double_scan_enabled,
      set_double_scan_enabled, NULL },
#ifdef USE_XF86_EXTENSIONS
    { "FullscreenDoubleSize", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.fullscreen_double_size_enabled,
      set_fullscreen_double_size_enabled, NULL },
    { "FullscreenDoubleScan", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&vic_ii_resources.fullscreen_double_scan_enabled,
      set_fullscreen_double_scan_enabled, NULL },
#endif
    { NULL }
};

#endif /* VIC_II_NEED_2X */


int vic_ii_resources_init(void)
{
#ifdef VIC_II_NEED_2X
    if (resources_register(resources_2x) < 0)
        return -1;
#endif

    return resources_register(resources);
}

