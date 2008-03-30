/*
 * video-resources.c - Resources for the video layer
 *
 * Written by
 *  John Selck <graham@cruise.de>
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

#include "raster.h" /* Temporary */
#include "resources.h"
#include "utils.h"
#include "video-resources.h"
#include "video-color.h"
#include "video.h"
#include "videoarch.h"
#include "ui.h"
#include "utils.h"


video_resources_t video_resources =
{
    1000, 1100, 1100, 880,
    0, 0,
    0, 0,
};

static int set_color_saturation(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    video_resources.color_saturation = val;
    return video_color_update_palette();
}

static int set_color_contrast(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    video_resources.color_contrast = val;
    return video_color_update_palette();
}

static int set_color_brightness(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 2000)
        val = 2000;
    video_resources.color_brightness = val;
    return video_color_update_palette();
}

static int set_color_gamma(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val=0;
    if (val > 2000)
        val=2000;
    video_resources.color_gamma = val;
    return video_color_update_palette();
}

static int set_ext_palette(resource_value_t v, void *param)
{
    video_resources.ext_palette = (int)v;
    return video_color_update_palette();
}

static int set_palette_file_name(resource_value_t v, void *param)
{
    util_string_set(&video_resources.palette_file_name, (char *)v);
    return video_color_update_palette();
}

#ifndef USE_GNOMEUI
/* remove this once all ports have implemented this ui function */
#define ui_update_pal_ctrls(a)
#endif

static int set_delayloop_emulation(resource_value_t v, void *param)
{
    int old = video_resources.delayloop_emulation;
    video_resources.delayloop_emulation = (int)v;

    if (video_color_update_palette() < 0) {
        video_resources.delayloop_emulation = old;
	ui_update_pal_ctrls(video_resources.delayloop_emulation);
        return -1;
    }
    ui_update_pal_ctrls(video_resources.delayloop_emulation);

    return 0;
}

static int set_pal_scanlineshade(resource_value_t v, void *param)
{
    int val;
    val = (int)v;
    if (val < 0)
        val = 0;
    if (val > 1000)
        val = 1000;
    video_resources.pal_scanlineshade = val;
    return video_color_update_palette();
}

static int set_pal_mode(resource_value_t v, void *param)
{
    video_resources.pal_mode = (int)v;
	return 0;
}

static resource_t resources[] =
{
    { "ExternalPalette", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&video_resources.ext_palette,
      set_ext_palette, NULL },
    { "PaletteFile", RES_STRING, (resource_value_t)"default",
      (resource_value_t *)&video_resources.palette_file_name,
      set_palette_file_name, NULL },
    { NULL }
};

static resource_t resources_pal[] =
{
    { "ColorSaturation", RES_INTEGER, (resource_value_t)1000,
      (resource_value_t *)&video_resources.color_saturation,
      set_color_saturation, NULL },
    { "ColorContrast", RES_INTEGER, (resource_value_t)1000,
      (resource_value_t *)&video_resources.color_contrast,
      set_color_contrast, NULL },
    { "ColorBrightness", RES_INTEGER, (resource_value_t)1000,
      (resource_value_t *)&video_resources.color_brightness,
      set_color_brightness, NULL },
    { "ColorGamma", RES_INTEGER, (resource_value_t)880,
      (resource_value_t *)&video_resources.color_gamma,
      set_color_gamma, NULL },
    { "PALEmulation", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&video_resources.delayloop_emulation,
      set_delayloop_emulation, NULL },
    { "PALScanLineShade", RES_INTEGER, (resource_value_t)667,
      (resource_value_t *)&video_resources.pal_scanlineshade,
      set_pal_scanlineshade, NULL },
    { "PALMode", RES_INTEGER, (resource_value_t)VIDEO_RESOURCE_PAL_MODE_BLUR,
      (resource_value_t *)&video_resources.pal_mode,
      set_pal_mode, NULL },
    { NULL }
};

int video_resources_init(int mode)
{
    int result = 0;

    video_resources.palette_file_name = NULL;

    switch (mode) {
      case VIDEO_RESOURCES_MONOCHROME:
        result = resources_register(resources);
        break;
      case VIDEO_RESOURCES_PAL:
      case VIDEO_RESOURCES_PAL_NOFAKE:
       result = resources_register(resources)
                | resources_register(resources_pal);
       break;
    }

    return result | video_arch_init_resources();
}

/*-----------------------------------------------------------------------*/
/* Per chip resources.  */

struct video_resource_chip_s {
    struct raster_s *raster;
    video_chip_cap_t *video_chip_cap;
    int double_scan_enabled;
    int double_size_enabled;
    int fullscreen_enabled;
    char *fullscreen_device;
    int fullscreen_double_size_enabled;
    int fullscreen_double_scan_enabled;
    int fullscreen_mode[FULLSCREEN_MAXDEV];
};
typedef struct video_resource_chip_s video_resource_chip_t;

struct video_resource_chip_mode_s {
    video_resource_chip_t *resource_chip;
    unsigned int device;
};
typedef struct video_resource_chip_mode_s video_resource_chip_mode_t;

int set_double_size_enabled(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;
    cap_render_t *cap_render;
    raster_t *raster;

    video_resource_chip = (video_resource_chip_t *)param;
    video_chip_cap = video_resource_chip->video_chip_cap;
    raster = video_resource_chip->raster;

    if ((int)v)
        cap_render = &video_chip_cap->double_mode;
    else
        cap_render = &video_chip_cap->single_mode;

    raster->videoconfig->rendermode = cap_render->rmode;

    if (cap_render->sizex > 1)
        raster->videoconfig->doublesizex = 1;
    else
        raster->videoconfig->doublesizex = 0;

    if (cap_render->sizey > 1)
        raster->videoconfig->doublesizey = 1;
    else
        raster->videoconfig->doublesizey = 0;

    if (video_resource_chip->double_size_enabled != (int)v
        && raster->canvas != NULL
        && raster->canvas->viewport->update_canvas > 0) {
        video_viewport_resize(raster->canvas);
    }

    video_resource_chip->double_size_enabled = (int)v;

    return 0;
}

static const char *vname_chip_size[] = { "DoubleSize", NULL };

static resource_t resources_chip_size[] =
{
    { NULL, RES_INTEGER, (resource_value_t)0, NULL,
      set_double_size_enabled, NULL },
    { NULL }
};

int set_double_scan_enabled(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;

    video_resource_chip = (video_resource_chip_t *)param;

    video_resource_chip->double_scan_enabled = (int)v;
    video_resource_chip->raster->videoconfig->doublescan = (int)v;

    if (video_resource_chip->raster->canvas != NULL)
        video_canvas_refresh_all(video_resource_chip->raster->canvas);

    return 0;
}

static const char *vname_chip_scan[] = { "DoubleScan", NULL };

static resource_t resources_chip_scan[] =
{
    { NULL, RES_INTEGER, (resource_value_t)1, NULL,
      set_double_scan_enabled, NULL },
    { NULL }
};

int set_fullscreen_enabled(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;

    video_resource_chip = (video_resource_chip_t *)param;
    video_chip_cap = video_resource_chip->video_chip_cap;

    video_resource_chip->fullscreen_enabled = (int)v;

    return (video_chip_cap->fullscreen.enable)
        (video_resource_chip->raster->canvas, (int)v);
}

int set_fullscreen_double_size_enabled(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;

    video_resource_chip = (video_resource_chip_t *)param;
    video_chip_cap = video_resource_chip->video_chip_cap;

    video_resource_chip->fullscreen_double_size_enabled = (int)v;

    return (video_chip_cap->fullscreen.double_size)
        (video_resource_chip->raster->canvas, (int)v);
}

int set_fullscreen_double_scan_enabled(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;

    video_resource_chip = (video_resource_chip_t *)param;
    video_chip_cap = video_resource_chip->video_chip_cap;

    video_resource_chip->fullscreen_double_scan_enabled = (int)v;

    return (video_chip_cap->fullscreen.double_scan)
        (video_resource_chip->raster->canvas, (int)v);
}

int set_fullscreen_device(resource_value_t v, void *param)
{
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;

    video_resource_chip = (video_resource_chip_t *)param;
    video_chip_cap = video_resource_chip->video_chip_cap;

    if (util_string_set(&video_resource_chip->fullscreen_device,
        (const char *)v))
        return 0;

    return (video_chip_cap->fullscreen.device)
        (video_resource_chip->raster->canvas, (const char *)v);
}

static const char *vname_chip_fullscreen[] = {
    "Fullscreen", "FullscreenDoubleSize", "FullscreenDoubleScan",
    "FullscreenDevice", NULL
};

static resource_t resources_chip_fullscreen[] =
{
    { NULL, RES_INTEGER, (resource_value_t)0, NULL,
      set_fullscreen_enabled, NULL },
    { NULL, RES_INTEGER, (resource_value_t)0, NULL,
      set_fullscreen_double_size_enabled, NULL },
    { NULL, RES_INTEGER, (resource_value_t)0, NULL,
      set_fullscreen_double_scan_enabled, NULL },
    { NULL, RES_STRING, (resource_value_t)NULL, NULL,
      set_fullscreen_device, NULL },
    { NULL }
};

int set_fullscreen_mode(resource_value_t v, void *param)
{
    video_resource_chip_mode_t *video_resource_chip_mode;
    video_resource_chip_t *video_resource_chip;
    video_chip_cap_t *video_chip_cap;

    unsigned device;

    video_resource_chip_mode = (video_resource_chip_mode_t *)param;
    video_resource_chip = video_resource_chip_mode->resource_chip;
    video_chip_cap = video_resource_chip->video_chip_cap;

    device = video_resource_chip_mode->device;

    video_resource_chip->fullscreen_mode[device] = (int)v;

    return (video_chip_cap->fullscreen.mode[device])
        (video_resource_chip->raster->canvas, (int)v);

}

static const char *vname_chip_fullscreen_mode[] = { "FullscreenMode", NULL };

static resource_t resources_chip_fullscreen_mode[] =
{
    { NULL, RES_INTEGER, (resource_value_t)0, NULL,
      set_fullscreen_mode, NULL },
    { NULL }
};

int video_resources_chip_init(const char *chipname, struct raster_s *raster,
                              video_chip_cap_t *video_chip_cap)
{
    unsigned int i;
    video_resource_chip_t *resource_chip;

    resource_chip
        = (video_resource_chip_t *)xcalloc(1, sizeof(video_resource_chip_t));

    raster->videoconfig
        = (video_render_config_t *)xcalloc(1, sizeof(video_render_config_t));
    /*raster->canvas = video_canvas_init(raster->videoconfig);*/

    video_render_initconfig(raster->videoconfig);

    /* Set single size render as default.  */
    raster->videoconfig->rendermode = video_chip_cap->single_mode.rmode;
    raster->videoconfig->doublesizex
        = video_chip_cap->single_mode.sizex > 1 ? 1 : 0;
    raster->videoconfig->doublesizey
        = video_chip_cap->single_mode.sizey > 1 ? 1 : 0;

    resource_chip->raster = raster;
    resource_chip->video_chip_cap = video_chip_cap;

    if (video_chip_cap->dscan_allowed != 0) {
        resources_chip_scan[0].name
            = concat(chipname, vname_chip_scan[0], NULL);
        resources_chip_scan[0].value_ptr
            = (resource_value_t *)&(resource_chip->double_scan_enabled);
        resources_chip_scan[0].param = (void *)resource_chip;
        if (resources_register(resources_chip_scan) < 0)
            return -1;
    }

    if (video_chip_cap->dsize_allowed != 0) {
        resources_chip_size[0].name
            = concat(chipname, vname_chip_size[0], NULL);
        resources_chip_size[0].value_ptr
            = (resource_value_t *)&(resource_chip->double_size_enabled);
        resources_chip_size[0].param = (void *)resource_chip;
        if (resources_register(resources_chip_size) < 0)
            return -1;
    }

    if (video_chip_cap->fullscreen.device_num > 0) {
        video_resource_chip_mode_t *resource_chip_mode;

        resources_chip_fullscreen[0].name
            = concat(chipname, vname_chip_fullscreen[0], NULL);
        resources_chip_fullscreen[0].value_ptr
            = (resource_value_t *)&(resource_chip->fullscreen_enabled);
        resources_chip_fullscreen[0].param = (void *)resource_chip;

        resources_chip_fullscreen[1].name
            = concat(chipname, vname_chip_fullscreen[1], NULL);
        resources_chip_fullscreen[1].value_ptr
            = (resource_value_t *)&(resource_chip->fullscreen_double_size_enabled);
        resources_chip_fullscreen[1].param = (void *)resource_chip;

        resources_chip_fullscreen[2].name
            = concat(chipname, vname_chip_fullscreen[2], NULL);
        resources_chip_fullscreen[2].value_ptr
            = (resource_value_t *)&(resource_chip->fullscreen_double_scan_enabled);
        resources_chip_fullscreen[2].param = (void *)resource_chip;

        resources_chip_fullscreen[3].name
            = concat(chipname, vname_chip_fullscreen[3], NULL);
        resources_chip_fullscreen[3].factory_value
            = (resource_value_t)(video_chip_cap->fullscreen.device_name[0]);
        resources_chip_fullscreen[3].value_ptr
            = (resource_value_t *)&(resource_chip->fullscreen_device);
        resources_chip_fullscreen[3].param = (void *)resource_chip;

        if (resources_register(resources_chip_fullscreen) < 0)
            return -1;

        for (i = 0; i < video_chip_cap->fullscreen.device_num; i++) {
            resource_chip_mode = (video_resource_chip_mode_t *)malloc(
                                 sizeof(video_resource_chip_mode_t));
            resource_chip_mode->resource_chip = resource_chip;
            resource_chip_mode->device = i;

            resources_chip_fullscreen_mode[i].name
                = concat(chipname, video_chip_cap->fullscreen.device_name[i],
                    vname_chip_fullscreen_mode[0], NULL);
            resources_chip_fullscreen_mode[i].value_ptr
                = (resource_value_t *)&(resource_chip->fullscreen_mode[i]);
            resources_chip_fullscreen_mode[i].param
                = (void *)resource_chip_mode;

            if (resources_register(resources_chip_fullscreen_mode) < 0)
                return -1;
        }
    }

    return 0;
}

