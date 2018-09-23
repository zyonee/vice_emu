/*
 * video_sdl2.c - SDL2 video
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
 *  Ettore Perazzoli
 *  Andre Fachat
 *  Oliver Schaertel
 *  Martin Pottendorfer
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

/* #define SDL_DEBUG */

#include "vice.h"

#include <stdio.h>
#include "vice_sdl.h"

#include "archdep.h"
#include "cmdline.h"
#include "fullscreen.h"
#include "fullscreenarch.h"
#include "icon.h"
#include "joy.h"
#include "joystick.h"
#include "lib.h"
#include "lightpendrv.h"
#include "log.h"
#include "machine.h"
#include "palette.h"
#include "raster.h"
#include "resources.h"
#include "uimenu.h"
#include "uistatusbar.h"
#include "util.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsidui_sdl.h"
#include "vsync.h"

#ifdef SDL_DEBUG
#define DBG(x)  log_debug x
#else
#define DBG(x)
#endif

static log_t sdlvideo_log = LOG_ERR;

static int sdl_bitdepth;

static int sdl_limit_mode;
static int sdl_ui_finalized;

/* Custom w/h, used for fullscreen and limiting*/
static int sdl_custom_width = 0;
static int sdl_custom_height = 0;

/* window size, used for free scaling */
static int sdl_window_width = 0;
static int sdl_window_height = 0;

int sdl_active_canvas_num = 0;
static int sdl_num_screens = 0;
static video_canvas_t *sdl_canvaslist[MAX_CANVAS_NUM];
video_canvas_t *sdl_active_canvas = NULL;

static int sdl_gl_aspect_mode;
static char *aspect_ratio_s = NULL;
static double aspect_ratio;

static int sdl_gl_flipx;
static int sdl_gl_flipy;

static int sdl_gl_filter_res;
static int sdl_gl_filter;

static char *sdl2_renderer_name = NULL;
SDL_RendererFlip flip;
SDL_Window *new_window = NULL;
SDL_Renderer *new_renderer = NULL;
SDL_Texture *new_texture = NULL;
SDL_Surface *new_screen = NULL;
static Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
static int texformat = 0;

uint8_t *draw_buffer_vsid = NULL;
/* ------------------------------------------------------------------------- */
/* Video-related resources.  */

static int set_sdl_bitdepth(int d, void *param)
{
    switch (d) {
        case 8:
            texformat = SDL_PIXELFORMAT_RGB332;
            rmask = 0x000000e0, gmask = 0x0000001c, bmask = 0x00000003, amask = 0x00000000;
            break;
        case 15:
            /* Fixme: add render support for that format */
            return -1;
        case 16:
            texformat = SDL_PIXELFORMAT_RGB565;
            rmask = 0x0000f800, gmask = 0x000007e0, bmask = 0x0000001f, amask = 0x00000000;
            break;
        case 24:
            texformat = SDL_PIXELFORMAT_RGB24;
            rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0x00000000;
            break;
        case 32:
            texformat = SDL_PIXELFORMAT_ARGB8888;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            rmask = 0x0000ff00, gmask = 0x00ff0000, bmask = 0xff000000, amask = 0x000000ff;
#else
            rmask = 0x00ff0000, gmask = 0x0000ff00, bmask = 0x000000ff, amask = 0xff000000;
#endif
            break;
        default:
            return -1;
    }

    if (sdl_bitdepth == d) {
        return 0;
    }
    sdl_bitdepth = d;
    /* update */
    return 0;
}

static int set_sdl_limit_mode(int v, void *param)
{
    switch (v) {
        case SDL_LIMIT_MODE_OFF:
        case SDL_LIMIT_MODE_MAX:
        case SDL_LIMIT_MODE_FIXED:
            break;
        default:
            return -1;
    }

    if (sdl_limit_mode != v) {
        sdl_limit_mode = v;
        video_viewport_resize(sdl_active_canvas, 1);
    }
    return 0;
}

static int set_sdl_custom_width(int w, void *param)
{
    if (w <= 0) {
        return -1;
    }

    if (sdl_custom_width != w) {
        sdl_custom_width = w;
        if (sdl_active_canvas && sdl_active_canvas->fullscreenconfig->enable
            && sdl_active_canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            video_viewport_resize(sdl_active_canvas, 1);
        }
    }
    return 0;
}

static int set_sdl_custom_height(int h, void *param)
{
    if (h <= 0) {
        return -1;
    }

    if (sdl_custom_height != h) {
        sdl_custom_height = h;
        if (sdl_active_canvas && sdl_active_canvas->fullscreenconfig->enable
            && sdl_active_canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            video_viewport_resize(sdl_active_canvas, 1);
        }
    }
    return 0;
}

static int set_sdl_window_width(int w, void *param)
{
    if (w < 0) {
        return -1;
    }

    sdl_window_width = w;
    return 0;
}

static int set_sdl_window_height(int h, void *param)
{
    if (h < 0) {
        return -1;
    }

    sdl_window_height = h;
    return 0;
}

static int set_sdl_gl_aspect_mode(int v, void *param)
{
    int old_v = sdl_gl_aspect_mode;

    switch (v) {
        case SDL_ASPECT_MODE_OFF:
        case SDL_ASPECT_MODE_CUSTOM:
        case SDL_ASPECT_MODE_TRUE:
            break;
        default:
            return -1;
    }

    sdl_gl_aspect_mode = v;

    if (old_v != v) {
        if (sdl_active_canvas && sdl_active_canvas->videoconfig->hwscale) {
            video_viewport_resize(sdl_active_canvas, 1);
            sdl_video_resize_event(sdl_active_canvas->actual_width, sdl_active_canvas->actual_height);
        }
    }

    return 0;
}

static int set_aspect_ratio(const char *val, void *param)
{
    double old_aspect = aspect_ratio;
    char buf[20];

    if (val) {
        char *endptr;

        util_string_set(&aspect_ratio_s, val);

        aspect_ratio = strtod(val, &endptr);
        if (val == endptr) {
            aspect_ratio = 1.0;
        } else if (aspect_ratio < 0.5) {
            aspect_ratio = 0.5;
        } else if (aspect_ratio > 2.0) {
            aspect_ratio = 2.0;
        }
    } else {
        aspect_ratio = 1.0;
    }

    sprintf(buf, "%f", aspect_ratio);
    util_string_set(&aspect_ratio_s, buf);

    if (old_aspect != aspect_ratio) {
        if (sdl_active_canvas && sdl_active_canvas->videoconfig->hwscale) {
            video_viewport_resize(sdl_active_canvas, 1);
            sdl_video_resize_event(sdl_active_canvas->actual_width, sdl_active_canvas->actual_height);
        }
    }

    return 0;
}

static int set_sdl_gl_flipx(int v, void *param)
{
    sdl_gl_flipx = v ? 1 : 0;

    if (sdl_gl_flipx) {
        flip |= SDL_FLIP_HORIZONTAL;
    } else {
        flip &= ~SDL_FLIP_HORIZONTAL;
    }

    return 0;
}

static int set_sdl_gl_flipy(int v, void *param)
{
    sdl_gl_flipy = v ? 1 : 0;

    if (sdl_gl_flipy) {
        flip |= SDL_FLIP_VERTICAL;
    } else {
        flip &= ~SDL_FLIP_VERTICAL;
    }

    return 0;
}

static int set_sdl_gl_filter(int v, void *param)
{
    switch (v) {
        case SDL_FILTER_NEAREST:
            sdl_gl_filter = GL_NEAREST;
            break;

        case SDL_FILTER_LINEAR:
            sdl_gl_filter = GL_LINEAR;
            break;

        default:
            return -1;
    }

    sdl_gl_filter_res = v;
    return 0;
}

static int set_sdl2_renderer_name(const char *val, void *param)
{
    if (!val || val[0] == '\0') {
        util_string_set(&sdl2_renderer_name, "");
    } else {
        util_string_set(&sdl2_renderer_name, val);
    }
    return 0;
}

static const resource_string_t resources_string[] = {
    { "AspectRatio", "1.0", RES_EVENT_NO, NULL,
      &aspect_ratio_s, set_aspect_ratio, NULL },
    { "SDL2Renderer", "", RES_EVENT_NO, NULL,
      &sdl2_renderer_name, set_sdl2_renderer_name, NULL },
    RESOURCE_STRING_LIST_END
};

#define VICE_DEFAULT_BITDEPTH 32

#ifdef ANDROID_COMPILE
#define SDLLIMITMODE_DEFAULT     SDL_LIMIT_MODE_MAX
#define SDLCUSTOMWIDTH_DEFAULT   320
#define SDLCUSTOMHEIGHT_DEFAULT  200
#else
#define SDLLIMITMODE_DEFAULT     SDL_LIMIT_MODE_OFF
#define SDLCUSTOMWIDTH_DEFAULT   800
#define SDLCUSTOMHEIGHT_DEFAULT  600
#endif

static const resource_int_t resources_int[] = {
    { "SDLBitdepth", VICE_DEFAULT_BITDEPTH, RES_EVENT_NO, NULL,
      &sdl_bitdepth, set_sdl_bitdepth, NULL },
    { "SDLLimitMode", SDLLIMITMODE_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_limit_mode, set_sdl_limit_mode, NULL },
    { "SDLCustomWidth", SDLCUSTOMWIDTH_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_custom_width, set_sdl_custom_width, NULL },
    { "SDLCustomHeight", SDLCUSTOMHEIGHT_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_custom_height, set_sdl_custom_height, NULL },
    { "SDLWindowWidth", 0, RES_EVENT_NO, NULL,
      &sdl_window_width, set_sdl_window_width, NULL },
    { "SDLWindowHeight", 0, RES_EVENT_NO, NULL,
      &sdl_window_height, set_sdl_window_height, NULL },
    { "SDLGLAspectMode", SDL_ASPECT_MODE_TRUE, RES_EVENT_NO, NULL,
      &sdl_gl_aspect_mode, set_sdl_gl_aspect_mode, NULL },
    { "SDLGLFlipX", 0, RES_EVENT_NO, NULL,
      &sdl_gl_flipx, set_sdl_gl_flipx, NULL },
    { "SDLGLFlipY", 0, RES_EVENT_NO, NULL,
      &sdl_gl_flipy, set_sdl_gl_flipy, NULL },
    { "SDLGLFilter", SDL_FILTER_LINEAR, RES_EVENT_NO, NULL,
      &sdl_gl_filter_res, set_sdl_gl_filter, NULL },
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        if (joy_arch_resources_init() < 0) {
            return -1;
        }
    }

    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        joy_arch_resources_shutdown();
    }

    lib_free(aspect_ratio_s);
    lib_free(sdl2_renderer_name);
}

/* ------------------------------------------------------------------------- */
/* Video-related command-line options.  */

static const cmdline_option_t cmdline_options[] =
{
    { "-sdlbitdepth", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLBitdepth", NULL,
      "<bpp>", "Set bitdepth (0 = current, 8, 15, 16, 24, 32)" },
    { "-sdllimitmode", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLLimitMode", NULL,
      "<mode>", "Set resolution limiting mode (0 = off, 1 = max, 2 = fixed)" },
    { "-sdlcustomw", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLCustomWidth", NULL,
      "<width>", "Set custom resolution width" },
    { "-sdlcustomh", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLCustomHeight", NULL,
      "<height>", "Set custom resolution height" },
    { "-sdlaspectmode", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLGLAspectMode", NULL,
      "<mode>", "Set aspect ratio mode (0 = off, 1 = custom, 2 = true)" },
    { "-aspect", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "AspectRatio", NULL,
      "<aspect ratio>", "Set custom aspect ratio (0.5 - 2.0)" },
    { "-sdlflipx", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "SDLGLFlipX", (resource_value_t)1,
      NULL, "Enable X flip" },
    { "+sdlflipx", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "SDLGLFlipX", (resource_value_t)0,
      NULL, "Disable X flip" },
    { "-sdlflipy", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "SDLGLFlipY", (resource_value_t)1,
      NULL, "Enable Y flip" },
    { "+sdlflipy", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "SDLGLFlipY", (resource_value_t)0,
      NULL, "Disable Y flip" },
    { "-sdlglfilter", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDLGLFilter", NULL,
      "<mode>", "Set OpenGL filtering mode (0 = nearest, 1 = linear)" },
    { "-sdl2renderer", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "SDL2Renderer", NULL,
      "<renderer name>", "Set the preferred SDL2 renderer" },
    CMDLINE_LIST_END
};

int video_arch_cmdline_options_init(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        if (joystick_cmdline_options_init() < 0) {
            return -1;
        }
    }

    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

int video_init(void)
{
    sdlvideo_log = log_open("SDLVideo");
    return 0;
}

void video_shutdown(void)
{
    DBG(("%s", __func__));

    if (draw_buffer_vsid) {
        lib_free(draw_buffer_vsid);
    }

    sdl_active_canvas = NULL;
}

/* ------------------------------------------------------------------------- */
/* static helper functions */

static int sdl_video_canvas_limit(unsigned int limit_w, unsigned int limit_h, unsigned int *w, unsigned int *h, int mode)
{
    DBG(("%s", __func__));
    switch (mode & 3) {
        case SDL_LIMIT_MODE_MAX:
            if ((*w > limit_w) || (*h > limit_h)) {
                *w = MIN(*w, limit_w);
                *h = MIN(*h, limit_h);
                return 1;
            }
            break;
        case SDL_LIMIT_MODE_FIXED:
            if ((*w != limit_w) || (*h != limit_h)) {
                *w = limit_w;
                *h = limit_h;
                return 1;
            }
            break;
        case SDL_LIMIT_MODE_OFF:
        default:
            break;
    }
    return 0;
}

static void sdl_gl_set_viewport(unsigned int src_w, unsigned int src_h, unsigned int dest_w, unsigned int dest_h)
{
    int dest_x = 0, dest_y = 0;

    if (sdl_gl_aspect_mode != SDL_ASPECT_MODE_OFF) {
        double aspect = aspect_ratio;

        /* Get "true" aspect ratio */
        if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE) {
            aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
        }

        /* Keep aspect ratio of src image. */
        if (dest_w * src_h < src_w * aspect * dest_h) {
            dest_y = dest_h;
            dest_h = (unsigned int)(dest_w * src_h / (src_w * aspect) + 0.5);
            dest_y = (dest_y - dest_h) / 2;
        } else {
            dest_x = dest_w;
            dest_w = (unsigned int)(dest_h * src_w * aspect / src_h + 0.5);
            dest_x = (dest_x - dest_w) / 2;
        }
    }

    /* Update lightpen adjustment parameters */
    sdl_lightpen_adjust.offset_x = dest_x;
    sdl_lightpen_adjust.offset_y = dest_y;

    sdl_lightpen_adjust.max_x = dest_w;
    sdl_lightpen_adjust.max_y = dest_h;

    sdl_lightpen_adjust.scale_x = (double)(src_w) / (double)(dest_w);
    sdl_lightpen_adjust.scale_y = (double)(src_h) / (double)(dest_h);

    SDL_RenderSetLogicalSize(sdl_active_canvas->renderer, dest_w, dest_h);
}

static video_canvas_t *sdl_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height)
{
    unsigned int new_width, new_height;
    unsigned int actual_width, actual_height;
    int flags;
    int fullscreen = 0;
    int limit = sdl_limit_mode;
    unsigned int limit_w = (unsigned int)sdl_custom_width;
    unsigned int limit_h = (unsigned int)sdl_custom_height;
    int lightpen_updated = 0;
    int it;
    int l;
    int drv_index = -1;
    double aspect = 1.0;
    char rendername[256] = { 0 };
    char **renderlist = NULL;
    char *gl_string;
    int renderamount = SDL_GetNumRenderDrivers();
    unsigned int window_h = 0;
    unsigned int window_w = 0;
    int temp_h = 0;
    int temp_w = 0;
    SDL_RendererInfo info;

    DBG(("%s: %i,%i (%i)", __func__, *width, *height, canvas->index));

    aspect = aspect_ratio;

    new_width = *width;
    new_height = *height;

    new_width *= canvas->videoconfig->scalex;
    new_height *= canvas->videoconfig->scaley;

    if ((canvas == sdl_active_canvas) && (canvas->fullscreenconfig->enable)) {
        fullscreen = 1;
    }

    if (fullscreen) {
        if (canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            flags = SDL_WINDOW_FULLSCREEN;
        } else {
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    } else {
        flags = SDL_WINDOW_RESIZABLE;
    }

    if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE && !fullscreen) {
        aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
    }

    if (sdl_video_canvas_limit(limit_w, limit_h, &new_width, &new_height, limit)) {
        canvas->draw_buffer->canvas_physical_width = new_width;
        canvas->draw_buffer->canvas_physical_height = new_height;
        video_viewport_resize(sdl_active_canvas, 0);
        if (sdl_ui_finalized) {
            return canvas; /* exit here as video_viewport_resize will recall */
        }
    }

    if (!sdl_ui_finalized) { /* remember first size */
        sdl_active_canvas->real_width = (unsigned int)((double)new_width * aspect + 0.5);
        sdl_active_canvas->real_height = new_height;
        DBG(("first: %d:%d\n", sdl_active_canvas->real_width, sdl_active_canvas->real_height));
    }

    actual_width = new_width;
    actual_height = new_height;

    if (!fullscreen) {
        /* if no window geometry given then create one. */
        if (!sdl_window_width || !sdl_window_height) {
            window_w = sdl_window_width = (unsigned int)((double)new_width * aspect + 0.5);
            window_h = sdl_window_height = new_height;
        } else { /* full window size remembering when aspect ratio is not important */
            window_w = (unsigned int)sdl_window_width;
            window_h = (unsigned int)sdl_window_height;
        }
    }

    if (new_window) {
        if (new_screen) {
            SDL_FreeSurface(new_screen);
            new_screen = NULL;
        }
        if (new_texture) {
            SDL_DestroyTexture(new_texture);
            new_texture = NULL;
        }
        if (new_renderer) {
            SDL_DestroyRenderer(new_renderer);
            new_renderer = NULL;
        }
        SDL_DestroyWindow(new_window);
        new_window = NULL;
    }

    /* Obtain the Window with the corresponding size and behavior based on the flags */
    new_window = SDL_CreateWindow(canvas->viewport->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, flags);
    if (new_window == NULL) {
        log_error(sdlvideo_log, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return NULL;
    }

    sdl_ui_set_window_icon(new_window);
    
    /* Allocate renderlist strings */
    renderlist = lib_malloc((renderamount + 1) * sizeof(char *));

    /* Fill in the renderlist and render info string */
    for (it = 0; it < renderamount; ++it) {
        SDL_GetRenderDriverInfo(it, &info);

        strcat(rendername, info.name);
        strcat(rendername, " ");
        renderlist[it] = lib_stralloc(info.name);
    }
    renderlist[it] = NULL;
        
    /* Check for resource preferred renderer */
    if (sdl2_renderer_name != NULL && *sdl2_renderer_name != '\0') {
        for (it = 0; it < renderamount; ++it) {
            if (!strcmp(sdl2_renderer_name, renderlist[it])) {
                drv_index = it;
            }
        }
        if (drv_index == -1) {
            log_warning(sdlvideo_log, "Resource preferred renderer %s not available, trying arch default renderer(s)", sdl2_renderer_name);
        }
    }

    /* Try arch default renderer(s) if the resource preferred renderer was not available */
    for (l = 0; drv_index == -1 && archdep_sdl2_default_renderers[l]; ++l) {
        for (it = 0; it < renderamount; ++it) {
            if (!strcmp(archdep_sdl2_default_renderers[l], renderlist[it])) {
                drv_index = it;
            }
        }
    }

    for (l = 0; l < renderamount; ++l) {
        lib_free(renderlist[l]);
    }
    lib_free(renderlist);
    renderlist = NULL;

    log_message(sdlvideo_log, "Available Renderers: %s", rendername);

    if (new_window) {
        new_renderer = SDL_CreateRenderer(new_window, drv_index, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (new_renderer) {
            SDL_SetRenderDrawColor(new_renderer, 0, 0, 0, 255);
            SDL_RenderClear(new_renderer);
            SDL_RenderPresent(new_renderer);
            new_screen = SDL_CreateRGBSurface(0, actual_width, actual_height, sdl_bitdepth, rmask, gmask, bmask, amask);
            if (fullscreen) {
                SDL_RenderSetLogicalSize(new_renderer, actual_width, actual_height);
            }
            if (new_screen) {
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
                new_texture = SDL_CreateTexture(new_renderer, texformat, SDL_TEXTUREACCESS_STREAMING, actual_width, actual_height);
                if (!new_texture) {
                    SDL_FreeSurface(new_screen);
                    new_screen = NULL;
                    SDL_DestroyRenderer(new_renderer);
                    new_renderer = NULL;
                    SDL_DestroyWindow(new_window);
                    new_window = NULL;
                }
            } else {
                SDL_DestroyRenderer(new_renderer);
                new_renderer = NULL;
                SDL_DestroyWindow(new_window);
                new_window = NULL;
            }
        } else {
            SDL_DestroyWindow(new_window);
            new_window = NULL;
            new_screen = NULL;
        }
    } else {
        log_error(sdlvideo_log, "SDL_CreateWindow failed!");
        return NULL;
    }

    /* some devices, OS do not provide a windowing system they have always a fixed width/height, 
       check if our desired window size is different from real size, needed by apect ratio */
    SDL_GetWindowSize(new_window, &temp_w, &temp_h);
    if (temp_w != window_w && temp_h != window_h && !fullscreen) {
        sdl_window_width = (unsigned int)temp_w;
        sdl_window_height = (unsigned int)temp_h;
    }

    sdl_bitdepth = new_screen->format->BitsPerPixel;
    actual_width = new_screen->w;
    actual_height = new_screen->h;

    canvas->depth = sdl_bitdepth;
    canvas->width = new_width;
    canvas->height = new_height;
    canvas->screen = new_screen;
    canvas->window = new_window;
    canvas->renderer = new_renderer;
    canvas->texture = new_texture;
    canvas->actual_width = actual_width;
    canvas->actual_height = actual_height;
    canvas->videoconfig->hwscale = 1;

    SDL_GetRenderDriverInfo(drv_index, &info);

    log_message(sdlvideo_log, "%s (%s) %ix%i %ibpp using %s%s", canvas->videoconfig->chip_name, (canvas == sdl_active_canvas) ? "active" : "inactive", actual_width, actual_height, sdl_bitdepth, info.name, (canvas->fullscreenconfig->enable) ? " (fullscreen)" : "");
#ifdef SDL_DEBUG
    log_message(sdlvideo_log, "Canvas %ix%i, real %ix%i", new_width, new_height, canvas->real_width, canvas->real_height);
#endif

    /* Update lightpen adjustment parameters */
    if (!lightpen_updated) {
        sdl_lightpen_adjust.max_x = actual_width;
        sdl_lightpen_adjust.max_y = actual_height;

        sdl_lightpen_adjust.scale_x = (double)*width / (double)actual_width;
        sdl_lightpen_adjust.scale_y = (double)*height / (double)actual_height;
    }

    video_canvas_set_palette(canvas, canvas->palette);

    if ((sdl_window_width || sdl_window_height) && !fullscreen) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_WINDOWEVENT;
        sdlevent.window.event = SDL_WINDOWEVENT_RESIZED;
        sdlevent.window.data1 = sdl_window_width;
        sdlevent.window.data2 = sdl_window_height;

        SDL_PushEvent(&sdlevent);
    }

    /* Enable file/text drag and drop support */
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    return canvas;
}

/* ------------------------------------------------------------------------- */
/* Main API */

/* called from raster/raster.c:realize_canvas */
video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    /* nothing to do here, the real work is done in sdl_ui_init_finalize */
    return canvas;
}

void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    uint8_t *backup;

    if ((canvas == NULL) || (canvas->screen == NULL) || (canvas != sdl_active_canvas)) {
        return;
    }

    if (sdl_vsid_state & SDL_VSID_ACTIVE) {
        sdl_vsid_draw();
    }

    if (sdl_vkbd_state & SDL_VKBD_ACTIVE) {
        sdl_vkbd_draw();
    }

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_draw();
    }

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

    w = MIN(w, canvas->width);
    h = MIN(h, canvas->height);

    /* FIXME attempt to draw outside canvas */
    if ((xi + w > canvas->width) || (yi + h > canvas->height)) {
        return;
    }

    if (machine_class == VICE_MACHINE_VSID) {
        canvas->draw_buffer_vsid->draw_buffer_width = canvas->draw_buffer->draw_buffer_width;
        canvas->draw_buffer_vsid->draw_buffer_height = canvas->draw_buffer->draw_buffer_height;
        canvas->draw_buffer_vsid->draw_buffer_pitch = canvas->draw_buffer->draw_buffer_pitch;
        canvas->draw_buffer_vsid->canvas_physical_width = canvas->draw_buffer->canvas_physical_width;
        canvas->draw_buffer_vsid->canvas_physical_height = canvas->draw_buffer->canvas_physical_height;
        canvas->draw_buffer_vsid->canvas_width = canvas->draw_buffer->canvas_width;
        canvas->draw_buffer_vsid->canvas_height = canvas->draw_buffer->canvas_height;
        canvas->draw_buffer_vsid->visible_width = canvas->draw_buffer->visible_width;
        canvas->draw_buffer_vsid->visible_height = canvas->draw_buffer->visible_height;

        backup = canvas->draw_buffer->draw_buffer;
        canvas->draw_buffer->draw_buffer = canvas->draw_buffer_vsid->draw_buffer;
        video_canvas_render(canvas, (uint8_t *)canvas->screen->pixels, w, h, xs, ys, xi, yi, canvas->screen->pitch, canvas->screen->format->BitsPerPixel);
        canvas->draw_buffer->draw_buffer = backup;
    } else {
        video_canvas_render(canvas, (uint8_t *)canvas->screen->pixels, w, h, xs, ys, xi, yi, canvas->screen->pitch, canvas->screen->format->BitsPerPixel);
    }

    SDL_UpdateTexture(canvas->texture, NULL, canvas->screen->pixels, canvas->screen->pitch);
    SDL_RenderClear(canvas->renderer);
    SDL_RenderCopyEx(canvas->renderer, canvas->texture, NULL, NULL, 0, NULL, flip);
    SDL_RenderPresent(canvas->renderer);
}

int video_canvas_set_palette(struct video_canvas_s *canvas, struct palette_s *palette)
{
    unsigned int i, col = 0;
    SDL_PixelFormat *fmt;

    DBG(("video_canvas_set_palette canvas: %p", canvas));

    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }

    canvas->palette = palette;

    fmt = canvas->screen->format;

    /* Fixme: needs further investigation how it can reach here without being fully initialized */
    if (canvas != sdl_active_canvas || canvas->width != canvas->screen->w) {
        DBG(("video_canvas_set_palette not active canvas or window not created, don't update hw palette"));
        return 0;
    }

    for (i = 0; i < palette->num_entries; i++) {
        if (canvas->depth % 8 == 0) {
            col = SDL_MapRGB(fmt, palette->entries[i].red, palette->entries[i].green, palette->entries[i].blue);
        }
        video_render_setphysicalcolor(canvas->videoconfig, i, col, canvas->depth);
    }

    if (canvas->depth % 8 == 0) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, SDL_MapRGB(fmt, (Uint8)i, 0, 0), SDL_MapRGB(fmt, 0, (Uint8)i, 0), SDL_MapRGB(fmt, 0, 0, (Uint8)i));
        }
        video_render_initraw(canvas->videoconfig);
    }

    return 0;
}

/* called from video_viewport_resize */
void video_canvas_resize(struct video_canvas_s *canvas, char resize_canvas)
{
    unsigned int width = canvas->draw_buffer->canvas_width;
    unsigned int height = canvas->draw_buffer->canvas_height;
    DBG(("%s: %ix%i (%i)", __func__, width, height, canvas->index));
    /* Check if canvas needs to be resized to real size first */
    if (sdl_ui_finalized) {
        /* NOTE: setting the resources to zero like this here would actually
                 not only force a recalculation of the resources, but also
                 result in the window size being recalculated from the default
                 dimensions instead of the (saved and supposed to be persistant)
                 values in the resources. what goes wrong when this is done can
                 be observed when x128 starts up.
            FIXME: remove this note and code below after some testing. hopefully
                   nothing else relies on the broken behavior...
         */
#if 0
        sdl_window_width = 0; /* force recalculate */
        sdl_window_height = 0;
#endif
        sdl_canvas_create(canvas, &width, &height); /* set the real canvas size */

        if (resize_canvas) {
            DBG(("%s: set and resize to real size (%ix%i)", __func__, width, height));
            canvas->real_width = canvas->actual_width;
            canvas->real_height = canvas->actual_height;
        }
    }
}

/* Resize window to w/h. */
static void sdl_video_resize(unsigned int w, unsigned int h)
{
    DBG(("%s: %ix%i", __func__, w, h));

    if ((w == 0) || (h == 0)) {
        DBG(("%s: ERROR, ignored!", __func__));
        return;
    }

    vsync_suspend_speed_eval();

    {
        sdl_gl_set_viewport(sdl_active_canvas->width, sdl_active_canvas->height, w, h);
        sdl_active_canvas->actual_width = w;
        sdl_active_canvas->actual_height = h;
    }
}

/* Resize window to stored real size */
void sdl_video_restore_size(void)
{
    unsigned int w, h;

    w = sdl_active_canvas->real_width;
    h = sdl_active_canvas->real_height;

    DBG(("%s: %ix%i->%ix%i", __func__, sdl_active_canvas->real_width, sdl_active_canvas->real_height, w, h));
    sdl_video_resize(w, h);
}

/* special case handling for the SDL window resize event */
void sdl_video_resize_event(unsigned int w, unsigned int h)
{
    DBG(("%s: %ix%i", __func__, w, h));
    if ((w == 0) || (h == 0)) {
        DBG(("%s: ERROR, ignored!", __func__));
        return;
    }
    sdl_video_resize(w, h);
    if (!sdl_active_canvas->fullscreenconfig->enable) {
        resources_set_int("SDLWindowWidth", sdl_active_canvas->actual_width);
        resources_set_int("SDLWindowHeight", sdl_active_canvas->actual_height);
    }
}

void sdl_video_canvas_switch(int index)
{
    struct video_canvas_s *canvas;

    DBG(("%s: %i->%i", __func__, sdl_active_canvas_num, index));

    if (sdl_active_canvas_num == index) {
        return;
    }

    if (index >= sdl_num_screens) {
        return;
    }

    sdl_active_canvas_num = index;

    canvas = sdl_canvaslist[sdl_active_canvas_num];
    sdl_active_canvas = canvas;

    video_viewport_resize(canvas, 1);
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    DBG(("%s: (%p, %i)", __func__, canvas, sdl_num_screens));

    if (sdl_num_screens == MAX_CANVAS_NUM) {
        log_error(sdlvideo_log, "Too many canvases!");
        exit(-1);
    }

    canvas->video_draw_buffer_callback = NULL;

    canvas->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));

    if (sdl_active_canvas_num == sdl_num_screens) {
        sdl_active_canvas = canvas;
    }

    canvas->index = sdl_num_screens;

    sdl_canvaslist[sdl_num_screens++] = canvas;

    canvas->screen = NULL;
    canvas->real_width = 0;
    canvas->real_height = 0;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
    int i;

    DBG(("%s: (%p, %i)", __func__, canvas, canvas->index));

    for (i = 0; i < sdl_num_screens; ++i) {
        if ((sdl_canvaslist[i] == canvas) && (canvas == sdl_active_canvas)) {
            SDL_FreeSurface(sdl_canvaslist[i]->screen);
            sdl_canvaslist[i]->screen = NULL;
            SDL_DestroyTexture(sdl_canvaslist[i]->texture);
            sdl_canvaslist[i]->texture = NULL;
            SDL_DestroyRenderer(sdl_canvaslist[i]->renderer);
            sdl_canvaslist[i]->renderer = NULL;
            SDL_DestroyWindow(sdl_canvaslist[i]->window);
            sdl_canvaslist[i]->window = NULL;
        }
    }

    lib_free(canvas->fullscreenconfig);
}

char video_canvas_can_resize(video_canvas_t *canvas)
{
    return 1;
}

void sdl_ui_init_finalize(void)
{
    unsigned int width = sdl_active_canvas->draw_buffer->canvas_width;
    unsigned int height = sdl_active_canvas->draw_buffer->canvas_height;

    sdl_canvas_create(sdl_active_canvas, &width, &height); /* set the real canvas size */
    sdl_ui_finalized = 1;
}
