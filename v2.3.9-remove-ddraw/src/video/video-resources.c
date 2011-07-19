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

/* #define DEBUG_VIDEO */

#ifdef DEBUG_VIDEO
#define DBG(_x_)        log_debug _x_
#else
#define DBG(_x_)
#endif

#include "vice.h"

#include <stdio.h>

#include "lib.h"
#include "resources.h"
#include "video-resources.h"
#include "video-color.h"
#include "video.h"
#include "raster.h"
#include "viewport.h"
#include "util.h"
#include "log.h"
#include "machine.h"

video_resources_t video_resources =
{
    1000, /* color_saturation */
    1100, /* color_contrast */
    1100, /* color_brightness */ 
    2200, /* color_gamma */
    1000, /* color_tint */
    0,    /* delayloop_emulation */
    667,  /* pal_scanlineshade */
    500,  /* pal_blur */
    1250, /* pal_oddlines_phase */
    750   /* pal_oddlines_offset */
};

int video_resources_init(void)
{
    return video_arch_resources_init();
}

void video_resources_shutdown(void)
{
    video_arch_resources_shutdown();
}

/*-----------------------------------------------------------------------*/
/* Per chip resources.  */

static void video_resources_update_ui(raster_t *raster);

struct video_resource_chip_mode_s {
    raster_t *resource_chip;
    unsigned int device;
};
typedef struct video_resource_chip_mode_s video_resource_chip_mode_t;

static int set_double_size_enabled(int val, void *param)
{
    cap_render_t *cap_render;
    raster_t *raster = (raster_t *)param;
    int old_doublesizex, old_doublesizey;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    if (val) {
        cap_render = &video_chip_cap->double_mode;
    } else {
        cap_render = &video_chip_cap->single_mode;
    }

    raster->videoconfig->rendermode = cap_render->rmode;

    old_doublesizex = raster->videoconfig->doublesizex;
    old_doublesizey = raster->videoconfig->doublesizey;

    if (cap_render->sizex > 1
        && (video_chip_cap->dsize_limit_width == 0
        || (raster->draw_buffer->canvas_width > 0
        && raster->draw_buffer->canvas_width
        <= video_chip_cap->dsize_limit_width))) {
        raster->videoconfig->doublesizex = (cap_render->sizex - 1);
    } else {
        raster->videoconfig->doublesizex = 0;
    }

    if (cap_render->sizey > 1
        && (video_chip_cap->dsize_limit_height == 0
        || (raster->draw_buffer->canvas_height > 0
        && raster->draw_buffer->canvas_height
        <= video_chip_cap->dsize_limit_height))) {
        raster->videoconfig->doublesizey = (cap_render->sizey - 1);
    } else {
        raster->videoconfig->doublesizey = 0;
    }

    DBG(("set_double_size_enabled sizex:%d sizey:%d doublesizex:%d doublesizey:%d", cap_render->sizex, cap_render->sizey, canvas->videoconfig->doublesizex, canvas->videoconfig->doublesizey));

    if ((raster->videoconfig->double_size_enabled != val
        || old_doublesizex != raster->videoconfig->doublesizex
        || old_doublesizey != raster->videoconfig->doublesizey)
        && raster->initialized
        && raster->viewport->update_canvas > 0) {
        video_viewport_resize(raster);
    }

    raster->videoconfig->double_size_enabled = val;

    video_resources_update_ui(raster);

    return 0;
}

static const char *vname_chip_size[] = { "DoubleSize", NULL };

static resource_int_t resources_chip_size[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_double_size_enabled, NULL },
    RESOURCE_INT_LIST_END
};

static int set_double_scan_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;

    raster->videoconfig->doublescan = val;

    if (raster->initialized && !video_disabled_mode) {
        video_canvas_refresh(raster,
        raster->viewport->first_x
        + raster->geometry->extra_offscreen_border_left,
        raster->viewport->first_line,
        raster->viewport->x_offset,
        raster->viewport->y_offset,
        MIN(raster->draw_buffer->canvas_width,
        raster->geometry->screen_size.width - raster->viewport->first_x),
        MIN(raster->draw_buffer->canvas_height,
        raster->viewport->last_line - raster->viewport->first_line + 1));
    }

    video_resources_update_ui(raster);

    return 0;
}

static const char *vname_chip_scan[] = { "DoubleScan", NULL };

static resource_int_t resources_chip_scan[] =
{
    { NULL, 1, RES_EVENT_NO, NULL,
      NULL, set_double_scan_enabled, NULL },
    RESOURCE_INT_LIST_END
};

static int hwscale_possible;

static int set_hwscale_possible(int val, void *param)
{
    hwscale_possible = val;

    return 0;
}

static int set_hwscale_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;

    if (val
        && !raster->videoconfig->hwscale
        && !hwscale_possible) {
        log_message(LOG_DEFAULT, "HW scale not available, forcing to disabled");
        return 0;
    }

    raster->videoconfig->hwscale = val;

    if (raster->initialized) {
        video_viewport_resize(raster);
        video_color_update_palette(raster);
    }

    video_resources_update_ui(raster);

    return 0;
}

static const char *vname_chip_hwscale[] = { "HwScale", NULL };

static resource_int_t resources_chip_hwscale[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_hwscale_enabled, NULL },
    RESOURCE_INT_LIST_END
};

static resource_int_t resources_chip_hwscale_possible[] =
{
    { "HwScalePossible",
#ifdef HAVE_HWSCALE
      1,
#else
      0,
#endif
      RES_EVENT_NO, NULL,
      &hwscale_possible, set_hwscale_possible, NULL },
    RESOURCE_INT_LIST_END
};

static int set_scale2x_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;

    raster->videoconfig->scale2x = val;

    if (raster->initialized && !video_disabled_mode) {
        video_canvas_refresh(raster,
          raster->viewport->first_x
          + raster->geometry->extra_offscreen_border_left,
          raster->viewport->first_line,
          raster->viewport->x_offset,
          raster->viewport->y_offset,
          MIN(raster->draw_buffer->canvas_width,
          raster->geometry->screen_size.width - raster->viewport->first_x),
          MIN(raster->draw_buffer->canvas_height,
          raster->viewport->last_line - raster->viewport->first_line + 1));
    }

    video_resources_update_ui(raster);

    return 0;
}

static const char *vname_chip_scale2x[] = { "Scale2x", NULL };

static resource_int_t resources_chip_scale2x[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_scale2x_enabled, NULL },
    RESOURCE_INT_LIST_END
};

static int set_fullscreen_enabled(int val, void *param)
{
    int r = 0;
    raster_t *raster = (raster_t *)param;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    raster->videoconfig->fullscreen_enabled = val;

#ifndef USE_SDLUI
    if (raster->initialized)
#endif
    {
        if (val) {
            r = (video_chip_cap->fullscreen.enable)(raster->canvas, val);
            (void) (video_chip_cap->fullscreen.statusbar)
            (raster->canvas, raster->videoconfig->fullscreen_statusbar_enabled); 
        } else {
            /* always show statusbar when coming back to window mode */
            (void) (video_chip_cap->fullscreen.statusbar) (raster->canvas, 1); 
            r = (video_chip_cap->fullscreen.enable)(raster->canvas, val);
        }
    }
    return r;
}

static int set_fullscreen_statusbar(int val, void *param)
{
    raster_t *raster = (raster_t *)param;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    raster->videoconfig->fullscreen_statusbar_enabled = val;

    return (video_chip_cap->fullscreen.statusbar)(raster->canvas, val);
}

static int set_fullscreen_double_size_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    raster->videoconfig->fullscreen_double_size_enabled = val;

    return (video_chip_cap->fullscreen.double_size)(raster->canvas, val);
}

static int set_fullscreen_double_scan_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    raster->videoconfig->fullscreen_double_scan_enabled = val;

    return (video_chip_cap->fullscreen.double_scan)(raster->canvas, val);
}

static int set_fullscreen_device(const char *val, void *param)
{
    raster_t *raster = (raster_t *)param;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    if (raster->videoconfig->fullscreen_enabled) {
        log_message(LOG_DEFAULT,
            _("Fullscreen (%s) already active - disable first."),
            raster->videoconfig->fullscreen_device);
        return 0;
    }

    if (util_string_set(&raster->videoconfig->fullscreen_device, val)) {
        return 0;
    }

    return (video_chip_cap->fullscreen.device)(raster->canvas, val);
}

static const char *vname_chip_fullscreen[] = {
    "Fullscreen", "FullscreenStatusbar", "FullscreenDevice", NULL
};

static resource_string_t resources_chip_fullscreen_string[] =
{
    { NULL, NULL, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_device, NULL },
    RESOURCE_STRING_LIST_END
};

static resource_int_t resources_chip_fullscreen_int[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_enabled, NULL },
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_statusbar, NULL },
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_double_size_enabled, NULL },
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_double_scan_enabled, NULL },
    RESOURCE_INT_LIST_END
};

static int set_fullscreen_mode(int val, void *param)
{
    video_resource_chip_mode_t *video_resource_chip_mode = (video_resource_chip_mode_t *)param;
    raster_t *raster = video_resource_chip_mode->resource_chip;
    video_chip_cap_t *video_chip_cap = raster->videoconfig->cap;

    unsigned device = video_resource_chip_mode->device;


    raster->videoconfig->fullscreen_mode[device] = val;

    return (video_chip_cap->fullscreen.mode[device])(raster->canvas, val);
}

static const char *vname_chip_fullscreen_mode[] = { "FullscreenMode", NULL };

static resource_int_t resources_chip_fullscreen_mode[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_fullscreen_mode, NULL },
    RESOURCE_INT_LIST_END
};

static int set_ext_palette(int val, void *param)
{
    raster_t *raster = (raster_t *)param;

    raster->videoconfig->external_palette = (unsigned int)val;

    return video_color_update_palette(raster);
}

static int set_palette_file_name(const char *val, void *param)
{
    raster_t *raster = (raster_t *)param;

    util_string_set(&(raster->videoconfig->external_palette_name), val);

    return video_color_update_palette(raster);
}

static const char *vname_chip_palette[] = { "PaletteFile", "ExternalPalette",
                                            NULL };

static resource_string_t resources_chip_palette_string[] =
{
    { NULL, NULL, RES_EVENT_NO, NULL,
      NULL, set_palette_file_name, NULL },
    RESOURCE_STRING_LIST_END
};

static resource_int_t resources_chip_palette_int[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_ext_palette, NULL },
    RESOURCE_INT_LIST_END
};

static int set_double_buffer_enabled(int val, void *param)
{
    raster_t *raster = (raster_t *)param;

    raster->videoconfig->double_buffer = val;

    return 0;
}

static const char *vname_chip_double_buffer[] = { "DoubleBuffer", NULL };

static resource_int_t resources_chip_double_buffer[] =
{
    { NULL, 0, RES_EVENT_NO, NULL,
      NULL, set_double_buffer_enabled, NULL },
    RESOURCE_INT_LIST_END
};

int video_resources_chip_init(const char *chipname,
                              raster_t *raster,
                              video_chip_cap_t *video_chip_cap)
{
    unsigned int i;

    video_render_initconfig(raster->videoconfig);
    raster->videoconfig->cap = video_chip_cap;

    /* Set single size render as default.  */
    raster->videoconfig->rendermode = video_chip_cap->single_mode.rmode;
    raster->videoconfig->doublesizex
        = video_chip_cap->single_mode.sizex > 1 ? 1 : 0;
    raster->videoconfig->doublesizey
        = video_chip_cap->single_mode.sizey > 1 ? 1 : 0;

    if (video_chip_cap->dscan_allowed != 0) {
        resources_chip_scan[0].name
            = util_concat(chipname, vname_chip_scan[0], NULL);
        resources_chip_scan[0].value_ptr
            = &(raster->videoconfig->doublescan);
        resources_chip_scan[0].param = (void *)raster;
        if (resources_register_int(resources_chip_scan) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_scan[0].name));
    }

    if (video_chip_cap->hwscale_allowed != 0) {
        resources_chip_hwscale[0].name
            = util_concat(chipname, vname_chip_hwscale[0], NULL);
        resources_chip_hwscale[0].value_ptr
            = &(raster->videoconfig->hwscale);
        resources_chip_hwscale[0].param = (void *)raster;
        if (resources_register_int(resources_chip_hwscale) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_hwscale[0].name));
    }

    if (resources_register_int(resources_chip_hwscale_possible) < 0) {
        return -1;
    }

    if (video_chip_cap->scale2x_allowed != 0) {
        resources_chip_scale2x[0].name
            = util_concat(chipname, vname_chip_scale2x[0], NULL);
        resources_chip_scale2x[0].value_ptr
            = &(raster->videoconfig->scale2x);
        resources_chip_scale2x[0].param = (void *)raster;
        if (resources_register_int(resources_chip_scale2x) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_scale2x[0].name));
    }

    if (video_chip_cap->dsize_allowed != 0) {
        resources_chip_size[0].name
            = util_concat(chipname, vname_chip_size[0], NULL);
        resources_chip_size[0].factory_value
            = video_chip_cap->dsize_default;
        resources_chip_size[0].value_ptr
            = &(raster->videoconfig->double_size_enabled);
        resources_chip_size[0].param = (void *)raster;
        if (resources_register_int(resources_chip_size) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_size[0].name));
    }

    if (video_chip_cap->fullscreen.device_num > 0) {
        video_resource_chip_mode_t *resource_chip_mode;

        resources_chip_fullscreen_int[0].name
            = util_concat(chipname, vname_chip_fullscreen[0], NULL);
        resources_chip_fullscreen_int[0].value_ptr
            = &(raster->videoconfig->fullscreen_enabled);
        resources_chip_fullscreen_int[0].param = (void *)raster;

        resources_chip_fullscreen_int[1].name
            = util_concat(chipname, vname_chip_fullscreen[1], NULL);
        resources_chip_fullscreen_int[1].value_ptr
            = &(raster->videoconfig->fullscreen_statusbar_enabled);
        resources_chip_fullscreen_int[1].param = (void *)raster;

        resources_chip_fullscreen_string[0].name
            = util_concat(chipname, vname_chip_fullscreen[2], NULL);
        resources_chip_fullscreen_string[0].factory_value
            = video_chip_cap->fullscreen.device_name[0];
        resources_chip_fullscreen_string[0].value_ptr
            = &(raster->videoconfig->fullscreen_device);
        resources_chip_fullscreen_string[0].param = (void *)raster;

        if (resources_register_string(resources_chip_fullscreen_string) < 0) {
            return -1;
        }

        if (resources_register_int(resources_chip_fullscreen_int) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_fullscreen_int[0].name));
        lib_free((char *)(resources_chip_fullscreen_int[1].name));
        lib_free((char *)(resources_chip_fullscreen_string[0].name));

        for (i = 0; i < video_chip_cap->fullscreen.device_num; i++) {
            resource_chip_mode = lib_malloc(sizeof(video_resource_chip_mode_t));
            resource_chip_mode->resource_chip = raster;
            resource_chip_mode->device = i;

            resources_chip_fullscreen_mode[0].name
                = util_concat(chipname,
                    video_chip_cap->fullscreen.device_name[i],
                    vname_chip_fullscreen_mode[0], NULL);
            resources_chip_fullscreen_mode[0].value_ptr
                = &(raster->videoconfig->fullscreen_mode[i]);
            resources_chip_fullscreen_mode[0].param
                = (void *)resource_chip_mode;

            if (resources_register_int(resources_chip_fullscreen_mode) < 0) {
                return -1;
            }

            lib_free((char *)(resources_chip_fullscreen_mode[0].name));
        }
    }

    resources_chip_palette_string[0].name
        = util_concat(chipname, vname_chip_palette[0], NULL);
    resources_chip_palette_string[0].factory_value
        = video_chip_cap->external_palette_name;
    resources_chip_palette_string[0].value_ptr
        = &(raster->videoconfig->external_palette_name);
    resources_chip_palette_string[0].param = (void *)raster;

    if (video_chip_cap->internal_palette_allowed != 0) {
        resources_chip_palette_int[0].name
            = util_concat(chipname, vname_chip_palette[1], NULL);
        resources_chip_palette_int[0].value_ptr
            = &(raster->videoconfig->external_palette);
        resources_chip_palette_int[0].param = (void *)raster;
    } else {
        resources_chip_palette_int[0].name = NULL;
        raster->videoconfig->external_palette = 1;
    }

    if (resources_register_string(resources_chip_palette_string) < 0) {
        return -1;
    }

    if (resources_register_int(resources_chip_palette_int) < 0) {
        return -1;
    }

    lib_free((char *)(resources_chip_palette_string[0].name));
    if (video_chip_cap->internal_palette_allowed != 0) {
        lib_free((char *)(resources_chip_palette_int[0].name));
    }

    if (video_chip_cap->double_buffering_allowed != 0) {
        resources_chip_double_buffer[0].name
            = util_concat(chipname, vname_chip_double_buffer[0], NULL);
        resources_chip_double_buffer[0].value_ptr
            = &(raster->videoconfig->double_buffer);
        resources_chip_double_buffer[0].param = (void *)raster;
        if (resources_register_int(resources_chip_double_buffer) < 0) {
            return -1;
        }

        lib_free((char *)(resources_chip_double_buffer[0].name));
    }

    return 0;
}

void video_resources_chip_shutdown(raster_t *raster)
{
    lib_free(raster->videoconfig->external_palette_name);

    if (raster->videoconfig->cap->fullscreen.device_num > 0) {
        lib_free(raster->videoconfig->fullscreen_device);
    }
}

static void video_resources_update_ui(raster_t *raster)
{
    int pal_enabled = 0;
    int ui_doublescan_enabled, ui_scale2x_enabled;

    if (raster->videoconfig->cap->palemulation_allowed)
        resources_get_int("PALEmulation", &pal_enabled);

    if (raster->videoconfig->double_size_enabled != 0) {
        if (pal_enabled) {
            ui_doublescan_enabled = 1;
            ui_scale2x_enabled = 0;
        } else if (raster->videoconfig->scale2x != 0) {
            ui_doublescan_enabled = 0;
            ui_scale2x_enabled = 1;
        } else {
            ui_doublescan_enabled = 1;
            ui_scale2x_enabled = 1;
        }
    } else {
        ui_doublescan_enabled = 0;
        ui_scale2x_enabled = 0;
    }
/*
    ui_enable_chip resources(ui_doublescan_enabled, ui_scale2x_enabled);
*/
}
