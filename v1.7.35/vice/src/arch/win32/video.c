/*
 * video.c - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@matavnet.hu>
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

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>

#include "cmdline.h"
#include "log.h"
#include "fullscreen.h"
#include "palette.h"
#include "raster.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "utils.h"
#include "video.h"
#include "videoarch.h"
#include "statusbar.h"

void video_resize(void);
raster_t *video_find_raster_for_canvas(video_canvas_t *canvas);

#ifndef HAVE_GUIDLIB
extern const GUID IID_IDirectDraw2;
#endif

#define EXIT_REASON(reason) {log_debug("Error %08x",reason);return -1;}

/* ------------------------------------------------------------------------ */

//#define DEBUG_VIDEO

/* Debugging stuff.  */
#ifdef DEBUG_VIDEO
static void video_debug(const char *format, ...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    log_debug(tmp);
}
#define DEBUG(x) video_debug x
#else
#define DEBUG(x)
#endif

/* ------------------------------------------------------------------------ */

/* Video-related resources.  */

/* Flag: are we in fullscreen mode?  */
int fullscreen_enabled;

static resource_t resources[] = {
    { NULL }
};

int video_arch_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------ */

/* Video-related command-line options.  */

static cmdline_option_t cmdline_options[] = {
    { "-fullscreen", SET_RESOURCE, 0, NULL, NULL,
      "FullScreenEnabled", (resource_value_t) 1,
      NULL, "Start VICE in fullscreen mode" },
    { NULL }
};


int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------ */

/* DirectDraw errors.  This is shamelessly copied from `win32.c' in UAE,
   althouigh some values were missing there and I wrote my own.  */
const char *dd_error(HRESULT ddrval)
{
    switch (ddrval) {
      case DDERR_NODIRECTDRAWHW:
        return "No DirectDraw hardware available";
      case DDERR_NOEMULATION:
        return "No emulation layer available";
      case DDERR_NOEXCLUSIVEMODE:
        return "No exclusive mode";
      case DDERR_NOFLIPHW:
        return "No flipping hardware";
      case DDERR_NOZBUFFERHW:
        return "No ZBuffer hardware";
      case DDERR_NOCOOPERATIVELEVELSET:
        return "No cooperative level set";
      case DDERR_ALREADYINITIALIZED:
        return "This object is already initialized.";
      case DDERR_BLTFASTCANTCLIP:
        return "Cannot use BLTFAST with Clipper attached to surface.";
      case DDERR_CANNOTATTACHSURFACE:
        return "Cannot attach surface.";
      case DDERR_CANNOTDETACHSURFACE:
        return "Cannot detach surface.";
      case DDERR_CANTCREATEDC:
        return "Cannot create DC Device Context.";
      case DDERR_CANTDUPLICATE:
        return "Cannot duplicate.";
      case DDERR_CANTLOCKSURFACE:
        return "Access to surface refused (no DCI provider).";
      case DDERR_CANTPAGELOCK:
        return "PageLock failure.";
      case DDERR_CANTPAGEUNLOCK:
        return "PageUnlock failure.";
      case DDERR_CLIPPERISUSINGHWND:
        return "Can't set a clip-list for a Clipper which is attached to an HWND.";
      case DDERR_COLORKEYNOTSET:
        return "No source colour-key provided.";
      case DDERR_CURRENTLYNOTAVAIL:
        return "Support unavailable.";
      case DDERR_DCALREADYCREATED:
        return "Surface already has a Device Context.";
      case DDERR_DIRECTDRAWALREADYCREATED:
        return "DirectDraw already bound to this process.";
      case DDERR_EXCEPTION:
        return "Unexpected exception.";
      case DDERR_EXCLUSIVEMODEALREADYSET:
        return "Already in exclusive mode.";
      case DDERR_GENERIC:
        return "Undefined";
      case DDERR_HEIGHTALIGN:
        return "Height needs to be aligned.";
      case DDERR_HWNDALREADYSET:
        return "HWND already set for cooperative level.";
      case DDERR_HWNDSUBCLASSED:
        return "HWND has been subclassed.";
      case DDERR_IMPLICITLYCREATED:
        return "Can't restore an implicitly created surface.";
      case DDERR_INCOMPATIBLEPRIMARY:
        return "New params doesn't match existing primary surface.";
      case DDERR_INVALIDCAPS:
        return "Device doesn't have requested capabilities.";
      case DDERR_INVALIDCLIPLIST:
        return "Provided clip-list not supported.";
      case DDERR_INVALIDDIRECTDRAWGUID:
        return "Invalid GUID.";
      case DDERR_INVALIDMODE:
        return "Mode not supported.";
      case DDERR_INVALIDOBJECT:
        return "Invalid object.";
      case DDERR_INVALIDPARAMS:
        return "Invalid params.";
      case DDERR_INVALIDPIXELFORMAT:
        return "Device doesn't support requested pixel format.";
      case DDERR_INVALIDPOSITION:
        return "Overlay position illegal.";
      case DDERR_INVALIDRECT:
        return "Invalid RECT.";
      case DDERR_INVALIDSURFACETYPE:
        return "Wrong type of surface.";
      case DDERR_LOCKEDSURFACES:
        return "Surface locked.";
      case DDERR_NO3D:
        return "No 3d capabilities.";
      case DDERR_NOALPHAHW:
        return "No alpha h/w.";
      case DDERR_NOBLTHW:
        return "No blit h/w.";
      case DDERR_NOCLIPLIST:
        return "No clip-list.";
      case DDERR_NOCLIPPERATTACHED:
        return "No Clipper attached.";
      case DDERR_NOCOLORCONVHW:
        return "No colour-conversion h/w.";
      case DDERR_NOCOLORKEY:
        return "No colour-key.";
      case DDERR_NOTLOCKED:
        return "Not locked.";
      case DDERR_NOTPAGELOCKED:
        return "Not page-locked.";
      case DDERR_NOTPALETTIZED:
        return "Not palette-based.";
      case DDERR_OUTOFCAPS:
        return "out of caps";
      case DDERR_OUTOFMEMORY:
        return "Out of memory.";
      case DDERR_OUTOFVIDEOMEMORY:
        return "out of video memory.";
      case DDERR_PALETTEBUSY:
        return "Palette busy.";
      case DDERR_PRIMARYSURFACEALREADYEXISTS:
        return "Already a primary surface.";
      case DDERR_SURFACEBUSY:
        return "Surface busy.";
        /*case DDERR_SURFACEOBSCURED:     return "Surface is obscured."; */
      case DDERR_SURFACELOST:
        return "Surface lost.";
      case DDERR_UNSUPPORTED:
        return "Unsupported.";
      case DDERR_UNSUPPORTEDMODE:
        return "Unsupported mode.";
      case DDERR_UNSUPPORTEDFORMAT:
        return "Unsupported format.";
      case DDERR_WASSTILLDRAWING:
        return "Was still drawing.";
    }
    return "???";
}

/* ------------------------------------------------------------------------ */

/* Initialization.  */
int video_init(void)
{

    return 0;
}


/* ------------------------------------------------------------------------ */

/* Frame buffer functions.  */

int video_frame_buffer_alloc(video_frame_buffer_t **f,
                       unsigned int width,
                       unsigned int height)
{
    DEBUG(("frame_buffer_alloc()"));

    *f = (video_frame_buffer_t *)xmalloc(sizeof(struct video_frame_buffer_s));

    (*f)->width = width;
    (*f)->height = height;
    (*f)->buffer = xmalloc(width * height * sizeof(PIXEL));

    DEBUG(("Allocated frame buffer, %d x %d pixels.", width, height));

    return 0;
}


void video_frame_buffer_free(video_frame_buffer_t *f)
{
    if (!f)
        return;

    free(f->buffer);
    free(f);
}

void video_frame_buffer_clear(video_frame_buffer_t *f, PIXEL value)
{
    memset(f->buffer, value, f->height * f->width);
}

void video_frame_buffer_translate(video_canvas_t *c)
{
    int i;
    raster_t *r;

    return;
    r=video_find_raster_for_canvas(c);

    if (r) {
        for (i = 0; i < r->frame_buffer->width * r->frame_buffer->height; i++) {
            r->frame_buffer->buffer[i] = c->pixel_translate[r->frame_buffer->buffer[i]];
        }
    }
}

/* ------------------------------------------------------------------------ */

/* Canvas functions.  */

/* Set the palettes for canvas `c'.  */
int set_palette(video_canvas_t *c)
{
    if (c->depth == 8) {
        HRESULT result;

        /* FIXME: Surface lost errors?  */
        result = IDirectDrawSurface_SetPalette(c->primary_surface,
                                               c->dd_palette);
        if (result == DDERR_SURFACELOST) {
            IDirectDrawSurface_Restore(c->primary_surface);
            result = IDirectDrawSurface_SetPalette(c->primary_surface,
                                                   c->dd_palette);
        }
        if (result != DD_OK) {
            ui_error("Cannot set palette on primary DirectDraw surface:\n%s",
                     dd_error(result));
            return -1;
        }
    }
    return 0;
}

int set_physical_colors(video_canvas_t *c)
{
    HDC hdc;
    DDSURFACEDESC ddsd;
    int i;
    HRESULT result;
    COLORREF    oldcolor;
	DWORD pcol;
    DDPIXELFORMAT   format;
    DWORD   mask=0;
    int     rshift=0;
    int     rbits=0;
    int     gshift=0;
    int     gbits=0;
    int     bshift=0;
    int     bbits=0;
    DWORD   rmask=0;
    DWORD   gmask=0;
    DWORD   bmask=0;

    format.dwSize=sizeof(DDPIXELFORMAT);    
    IDirectDrawSurface_GetPixelFormat(c->primary_surface,&format);
    if (format.dwFlags&DDPF_RGB) {
        log_debug("RGB surface...");
#ifdef HAVE_UNNAMED_UNIONS
        log_debug("dwRGBBitCount: %d",format.dwRGBBitCount);
        log_debug("dwRBitMask: %08x",format.dwRBitMask);
        log_debug("dwGBitMask: %08x",format.dwGBitMask);
        log_debug("dwBBitMask: %08x",format.dwBBitMask);
#else
        log_debug("dwRGBBitCount: %d",format.u1.dwRGBBitCount);
        log_debug("dwRBitMask: %08x",format.u2.dwRBitMask);
        log_debug("dwGBitMask: %08x",format.u3.dwGBitMask);
        log_debug("dwBBitMask: %08x",format.u4.dwBBitMask);
#endif
    if (c->depth!=8){

#ifdef HAVE_UNNAMED_UNIONS
        mask=format.dwRBitMask;
#else
        mask=format.u2.dwRBitMask;
#endif
        rshift=0;
        while (!(mask&1)) {
            rshift++;
            mask>>=1;
        }
        rmask=mask;
        rbits=8;
        while(mask&1) {
            rbits--;
            mask>>=1;
        }
        log_debug("rshift: %d",rshift);
        log_debug("rmask: %02x",rmask);
        log_debug("rbits: %d",rbits);

#ifdef HAVE_UNNAMED_UNIONS
        mask=format.dwGBitMask;
#else
        mask=format.u3.dwGBitMask;
#endif
        gshift=0;
        while (!(mask&1)) {
            gshift++;
            mask>>=1;
        }
        gmask=mask;
        gbits=8;
        while(mask&1) {
            gbits--;
            mask>>=1;
        }
        log_debug("gshift: %d",gshift);
        log_debug("gmask: %02x",gmask);
        log_debug("gbits: %d",gbits);

#ifdef HAVE_UNNAMED_UNIONS
        mask=format.dwBBitMask;
#else
        mask=format.u4.dwBBitMask;
#endif
        bshift=0;
        while (!(mask&1)) {
            bshift++;
            mask>>=1;
        }
        bmask=mask;
        bbits=8;
        while(mask&1) {
            bbits--;
            mask>>=1;
        }
        log_debug("bshift: %d",bshift);
        log_debug("bmask: %02x",bmask);
        log_debug("bbits: %d",bbits);
    }
    } else {
        log_debug("Non RGB surface...");
    }


    for (i = 0; i < c->palette->num_entries; i++) {
        DWORD p;

        DEBUG(("Allocating color \"%s\"",
               c->palette->entries[i].name));

        if (c->depth==8) {
//            log_debug("depth==8");
            result = IDirectDrawSurface_GetDC(c->primary_surface,
                                              &hdc);
            if (result == DDERR_SURFACELOST) {
                IDirectDrawSurface_Restore(c->primary_surface);
                result = IDirectDrawSurface_GetDC(c->primary_surface,
                                                  &hdc);
            }
            if (result != DD_OK) {
                ui_error("Cannot get DC on DirectDraw surface while allocating colors:\n%s",
                         dd_error(result));
                return -1;
            }
            oldcolor = GetPixel(hdc,0,0);
            SetPixel(hdc, 0, 0, PALETTERGB(c->palette->entries[i].red,
                                           c->palette->entries[i].green,
                                           c->palette->entries[i].blue));
            IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);

            ddsd.dwSize = sizeof(ddsd);
            while ((result
                    = IDirectDrawSurface_Lock(c->primary_surface,
                                              NULL, &ddsd, 0,
                                              NULL))
                   == DDERR_WASSTILLDRAWING)
                ;
            if (result == DDERR_SURFACELOST) {
                IDirectDrawSurface_Restore(c->primary_surface);
                result = IDirectDrawSurface_Lock(c->primary_surface,
                                                 NULL, &ddsd, 0,
                                                 NULL);
            }
            if (result != DD_OK) {
                ui_error("Cannot lock temporary surface:\n%s",
                         dd_error(result));
                return -1;
            }

            p = *(DWORD *)ddsd.lpSurface;
        } else {
//            log_debug("depth!=8");
            p = (((c->palette->entries[i].red&(rmask<<rbits))>>rbits)<<rshift) +
                (((c->palette->entries[i].green&(gmask<<gbits))>>gbits)<<gshift) +
                (((c->palette->entries[i].blue&(bmask<<bbits))>>bbits)<<bshift);
        }
        c->pixel_translate[c->pixels[i]]=i;
        c->pixels[i] = i;
        pcol = p;
        if (c->depth==24) {
			pcol &= 0x00FFFFFF;
        }
        if (c->depth==16) {
			pcol &= 0x0000FFFF;
			pcol = (pcol << 16) | pcol;
        }
        if (c->depth==15) {
			pcol &= 0x00007FFF;
			pcol = (pcol << 16) | pcol;
        }
        if (c->depth== 8) {
			pcol &= 0x000000FF;
			pcol = (pcol << 8) | pcol;
/*
            c->pixel_translate[c->pixels[i]]=(BYTE) p & 0xff;
            c->pixels[i] = (BYTE) p & 0xff;
*/
        }
        c->physical_colors[i] = pcol;

        DEBUG(("Physical color for %d is 0x%04X",i,c->physical_colors[i]));
        DEBUG(("Pixel return %d 0x%02X", i, c->pixels[i]));
        if (c->depth==8) {
            if (IDirectDrawSurface_Unlock(c->primary_surface, NULL)
                == DDERR_SURFACELOST) {
                IDirectDrawSurface_Restore(c->primary_surface);
                IDirectDrawSurface_Unlock(c->primary_surface, NULL);
            }
            IDirectDrawSurface_GetDC(c->primary_surface,&hdc);
            SetPixel(hdc, 0, 0, oldcolor);
            IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);
        }
    }
    return 0;
}

int video_number_of_canvases;
video_canvas_t *video_canvases[2];
extern int fullscreen_active;
extern int fullscreen_transition;

/* Create a `video_canvas_t' with tile `win_name', of widht `*width' x `*height'
   pixels, exposure handler callback `exposure_handler' and palette
   `palette'.  If specified width/height is not possible, return an
   alternative in `*width' and `*height'; return the pixel values for the
   requested palette in `pixel_return[]'.  */
#define CANVAS_ERROR ((video_canvas_t *) -1)
video_canvas_t *video_canvas_create(const char *title, unsigned int *width,
                              unsigned int *height, int mapped,
                              void_t exposure_handler,
                              const palette_t *palette, PIXEL *pixel_return,
                              struct video_frame_buffer_s *fb)
{
    HRESULT result;
    HRESULT ddresult;
    DDSURFACEDESC desc;
    DDSURFACEDESC desc2;
    video_canvas_t *c;
    int i;
    GUID *device_guid;

    fullscreen_transition=1;

    c = xmalloc(sizeof(struct video_canvas_s));
    memset(c, 0, sizeof(struct video_canvas_s));

    /* "Normal" window stuff.  */
    c->title = stralloc(title);
    c->width = *width;
    c->height = *height;
    c->exposure_handler = (canvas_redraw_t)exposure_handler;
    c->palette = palette;
    c->hwnd = ui_open_canvas_window(title, c->width, c->height,
                                    exposure_handler,
                                    IsFullscreenEnabled());

    /*  Create the DirectDraw object */
//    device_guid=GetGUIDForActualDevice();
    device_guid=NULL;
    ddresult = DirectDrawCreate(device_guid, &c->dd_object, NULL);

    if (ddresult != DD_OK)
        return NULL;

    {
        ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, NULL,
                                                   DDSCL_NORMAL);
        if (ddresult != DD_OK) {
            ui_error("Cannot set DirectDraw cooperative level:\n%s",
                     dd_error(ddresult));
            return NULL;
        }
    }

    ddresult = IDirectDraw_QueryInterface(c->dd_object,
                                          (GUID *)&IID_IDirectDraw2,
                                          (LPVOID *)&c->dd_object2);
    if (ddresult != DD_OK) {
        log_debug("Can't get DirectDraw2 interface");
    }

    {
        c->client_width =* width;
        c->client_height =* height;
        fullscreen_active = 0;
    }

    /*  Create Primary surface */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc,
                                          &c->primary_surface, NULL);
    if (ddresult != DD_OK) {
        DEBUG(("Cannot create primary surface: %s", dd_error(ddresult)));
        return NULL;
    }

    ddresult = IDirectDraw2_CreateClipper(c->dd_object2, 0, &c->clipper, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create clipper for primary surface:\n%s",
                 dd_error(ddresult));
        return NULL;
    }
    ddresult = IDirectDrawSurface_SetClipper(c->primary_surface, c->clipper);
    if (ddresult != DD_OK) {
        ui_error("Cannot set clipper for primary surface:\n%s",
                 dd_error(ddresult));
        return NULL;
    }

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    /* For now, the back surface is always NULL because we have not
       implemented the full-screen mode yet.  */
    c->back_surface = NULL;


    memset(&desc2, 0, sizeof(desc2));
    desc2.dwSize = sizeof(desc2);
    ddresult = IDirectDraw2_GetDisplayMode(c->dd_object2, &desc2);

    /* Create the temporary surface.  */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    /* FIXME: SYSTEMMEMORY?  */
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = desc2.dwWidth;
    desc.dwHeight = desc2.dwHeight;
    result = IDirectDraw2_CreateSurface(c->dd_object2, &desc,
                                        &c->temporary_surface, NULL);
    if (result != DD_OK) {
        ui_error("Cannot create temporary DirectDraw surface:\n%s",
                 dd_error(result));
        goto error;
    }

    IDirectDrawSurface_GetCaps(c->temporary_surface, &desc.ddsCaps);
    DEBUG(("Allocated working surface in %s memory successfully.",
           (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY
            ? "system"
            : (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY
               ? "video"
               : "unknown"))));

    /* Find the color depth.  */
#ifdef HAVE_UNNAMED_UNIONS
    c->depth = desc2.ddpfPixelFormat.dwRGBBitCount;
#else
    c->depth = desc2.ddpfPixelFormat.u1.dwRGBBitCount;
#endif


    /* Create palette.  */
    if (c->depth == 8) {
        PALETTEENTRY ape[256];
        HRESULT result;

        /* Default to a 332 palette.  */
        for (i = 0; i < 256; i++) {
            ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
            ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
            ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
            ape[i].peFlags = (BYTE)0;
        }

        /* Overwrite first colors with the palette ones.  */
        for (i = 0; i < c->palette->num_entries; i++) {
            ape[i].peRed = c->palette->entries[i].red;
            ape[i].peGreen = c->palette->entries[i].green;
            ape[i].peBlue = c->palette->entries[i].blue;
            ape[i].peFlags = 0;
        }

        result = IDirectDraw2_CreatePalette(c->dd_object2, DDPCAPS_8BIT,
                                            ape, &c->dd_palette, NULL);
        if (result != DD_OK) {
            DEBUG(("Cannot create palette: %s", dd_error(result)));
            goto error;
        }
    }

    c->pixel_translate=malloc(sizeof(PIXEL)*256);
    if (set_palette(c) < 0)
        goto error;
    c->pixels = pixel_return;
    for (i = 0; i < c->palette->num_entries; i++) {
        c->pixels[i] = i;
    }

    if (set_physical_colors(c) < 0)
        goto error;

    video_canvases[video_number_of_canvases++]=c;

    if (IsFullscreenEnabled()) {
        SwitchToFullscreenMode(c->hwnd);
    }
    fullscreen_transition = 0;

    return c;

error:
    video_canvas_destroy(c);
    return NULL;

#if 0
    c = xmalloc(sizeof(struct video_canvas_s));
    memset(c, 0, sizeof(struct video_canvas_s));

    /* "Normal" window stuff.  */
    c->title = stralloc(title);
    c->width = *width;
    c->height = *height;
    if (IsFullscreenEnabled()) {
        GetCurrentModeParameters(&fullscreen_width, &fullscreen_height,
                                 &bitdepth, &refreshrate);
    }
    c->exposure_handler = (canvas_redraw_t)exposure_handler;
    c->palette = palette;
    c->hwnd = ui_open_canvas_window(title, c->width, c->height,
                                    exposure_handler,
                                    IsFullscreenEnabled());

    /*  Create the DirectDraw object */
    device_guid = GetGUIDForActualDevice();
    ddresult = DirectDrawCreate(device_guid, &c->dd_object, NULL);

    if (ddresult != DD_OK)
        return CANVAS_ERROR;

    if (IsFullscreenEnabled()) {
        /*  Set cooperative level */
        ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, c->hwnd,
                                                   DDSCL_EXCLUSIVE
                                                   | DDSCL_FULLSCREEN
                                                   | DDSCL_NOWINDOWCHANGES);
        if (ddresult != DD_OK) {
            ui_error("Cannot set DirectDraw cooperative level:\n%s",
                     dd_error(ddresult));
            return CANVAS_ERROR;
        }
    } else {
        ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, NULL,
                                                   DDSCL_NORMAL);
        if (ddresult != DD_OK) {
            ui_error("Cannot set DirectDraw cooperative level:\n%s",
                     dd_error(ddresult));
            return CANVAS_ERROR;
        }
    }

    ddresult=IDirectDraw_QueryInterface(c->dd_object, &IID_IDirectDraw2,
                                        (LPVOID *)&c->dd_object2);
    if (ddresult!=DD_OK) {
        log_debug("Can't get DirectDraw2 interface");
    }

    if (IsFullscreenEnabled()) {
        int w,h;
        int wnow, hnow;

        GetCurrentModeParameters(&fullscreen_width, &fullscreen_height,
                                 &bitdepth, &refreshrate);
        //  Cover up the larger area, so no messages sent to underlying windows
        //  This should prevent rearrangement of icons on the desktop.
        wnow = GetSystemMetrics(SM_CXSCREEN);
        hnow = GetSystemMetrics(SM_CYSCREEN);
        w = (fullscreen_width>wnow) ? fullscreen_width : wnow;
        h = (fullscreen_height>hnow) ? fullscreen_height : hnow;
        SetWindowPos(c->hwnd, HWND_TOPMOST, 0, 0, w, h, SWP_NOCOPYBITS);
        ShowCursor(FALSE);
        //  Set desired fullscreen mode
        ddresult = IDirectDraw2_SetDisplayMode(c->dd_object2, fullscreen_width,
                                               fullscreen_height, bitdepth,
                                               refreshrate, 0);
        c->client_width = fullscreen_width;
        c->client_height = fullscreen_height;
        fullscreen_active = 1;
    } else {
        c->client_width = *width;
        c->client_height = *height;
        fullscreen_active=0;
    }

    /*  Create Primary surface */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc,
                                          &c->primary_surface, NULL);
    if (ddresult != DD_OK) {
        DEBUG(("Cannot create primary surface: %s", dd_error(ddresult)));
        return CANVAS_ERROR;
    }

    ddresult = IDirectDraw2_CreateClipper(c->dd_object2, 0, &c->clipper, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create clipper for primary surface:\n%s",
                 dd_error(ddresult));
        return CANVAS_ERROR;
    }
    ddresult = IDirectDrawSurface_SetClipper(c->primary_surface, c->clipper);
    if (ddresult != DD_OK) {
        ui_error("Cannot set clipper for primary surface:\n%s",
                 dd_error(ddresult));
        return CANVAS_ERROR;
    }

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    /* For now, the back surface is always NULL because we have not
       implemented the full-screen mode yet.  */
    c->back_surface = NULL;


    memset(&desc2, 0, sizeof(desc2));
    desc2.dwSize = sizeof(desc2);
    ddresult = IDirectDraw2_GetDisplayMode(c->dd_object2, &desc2);

    /* Create the temporary surface.  */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    /* FIXME: SYSTEMMEMORY?  */
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = desc2.dwWidth;
    desc.dwHeight = desc2.dwHeight;
    result = IDirectDraw2_CreateSurface(c->dd_object2, &desc,
                                        &c->temporary_surface, NULL);
    if (result != DD_OK) {
        ui_error("Cannot create temporary DirectDraw surface:\n%s",
                 dd_error(result));
        goto error;
    }

    IDirectDrawSurface_GetCaps(c->temporary_surface, &desc.ddsCaps);
    DEBUG(("Allocated working surface in %s memory successfully.",
           (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY
            ? "system"
            : (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY
               ? "video"
               : "unknown"))));

    /* Find the color depth.  */
#ifdef HAVE_UNNAMED_UNIONS
    c->depth = desc2.ddpfPixelFormat.dwRGBBitCount;
#else
    c->depth = desc2.ddpfPixelFormat.u1.dwRGBBitCount;
#endif


    /* Create palette.  */
    if (c->depth == 8) {
        PALETTEENTRY ape[256];
        HRESULT result;

        /* Default to a 332 palette.  */
        for (i = 0; i < 256; i++) {
            ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
            ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
            ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
            ape[i].peFlags = (BYTE)0;
        }

        /* Overwrite first colors with the palette ones.  */
        for (i = 0; i < c->palette->num_entries; i++) {
            ape[i].peRed = c->palette->entries[i].red;
            ape[i].peGreen = c->palette->entries[i].green;
            ape[i].peBlue = c->palette->entries[i].blue;
            ape[i].peFlags = 0;
        }

        result = IDirectDraw2_CreatePalette(c->dd_object2, DDPCAPS_8BIT,
                                           ape, &c->dd_palette, NULL);
        if (result != DD_OK) {
            DEBUG(("Cannot create palette: %s", dd_error(result)));
            goto error;
        }
    }

    c->pixel_translate=malloc(sizeof(PIXEL) * 256);
    if (set_palette(c) < 0)
        goto error;
    c->pixels = pixel_return;
    for (i = 0; i < c->palette->num_entries; i++) {
        c->pixels[i] = i;
    }

    if (set_physical_colors(c) < 0)
        goto error;

    video_canvases[video_number_of_canvases++] = c;

    return c;

error:
    video_canvas_destroy(c);
    return NULL;
#endif
}

/* Destroy `s'.  */
void video_canvas_destroy(video_canvas_t *c)
{
    if (c != NULL) {
        if (c->hwnd !=0) {
            DestroyWindow(c->hwnd);
        }
        if (c->title != NULL) {
            free(c->title);
        }
        free(c);
    }
}

/* Make `s' visible.  */
void video_canvas_map(video_canvas_t *c)
{
}

/* Make `s' unvisible.  */
void video_canvas_unmap(video_canvas_t *c)
{
}

/* Change the size of `s' to `width' * `height' pixels.  */
void video_canvas_resize(video_canvas_t *c, unsigned int width,
                         unsigned int height)
{
    int fullscreen_width;
    int fullscreen_height;
    int bitdepth;
    int refreshrate;

    c->width = width;
    c->height = height;
    if (IsFullscreenEnabled()) {
        GetCurrentModeParameters(&fullscreen_width, &fullscreen_height,
                                 &bitdepth, &refreshrate);
    } else {
        c->client_width = width;
        c->client_height = height;
        ui_resize_canvas_window(c->hwnd, width, height);
    }
}

/* Set the palette of `c' to `p', and return the pixel values in
   `pixel_return[].  */
int video_canvas_set_palette(struct video_canvas_s *c, const palette_t *p,
                             PIXEL *pixel_return)
{
    int i;

    /* Always OK.  */
    c->palette = p;
    /* Create palette.  */
    if (c->depth == 8) {
        PALETTEENTRY ape[256];
        HRESULT result;

        /* Default to a 332 palette.  */
        for (i = 0; i < 256; i++) {
            ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
            ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
            ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
            ape[i].peFlags = (BYTE)0;
        }

        /* Overwrite first colors with the palette ones.  */
        for (i = 0; i < c->palette->num_entries; i++) {
            ape[i].peRed = c->palette->entries[i].red;
            ape[i].peGreen = c->palette->entries[i].green;
            ape[i].peBlue = c->palette->entries[i].blue;
            ape[i].peFlags = 0;
        }

        result = IDirectDraw2_CreatePalette(c->dd_object2, DDPCAPS_8BIT,
                                            ape, &c->dd_palette, NULL);
    }
    set_palette(c);
    set_physical_colors(c);
    return 0;
}

/* ------------------------------------------------------------------------ */

/*
   Here is where the whole thing gets complicate.  We basically have three
   ways to update the window:

   a) manual/direct framebuffer -> primary_surface copy;

   b) framebuffer -> tmp_surface conversion, and consequent tmp_surface ->
      primary_surface blit;

   c) manual/direct framebuffer -> back_surface copy, and consequent
      back_surface <-> primary_surface flip.

   As the frame buffer is 8bpp, method (a) is only possible when the screen
   is in 8bpp mode.  Unluckily, this cannot be done with some Windows NT
   DirectDraw drivers either, because some of them (e.g. the ET4000 one I am
   using now) do not provide direct access to windows in non-exclusive mode:
   in this case, the only way is to use method (b) (see below).  It would be
   great to just use a DirectDraw surface as the framebuffer, but unluckily
   this is not possible because we might need a framebuffer that is larger
   than the physical screen (to clip sprites), and this is not allowed by
   DirectX 3.  (Maybe future versions?  Does anybody have any info on this?)

   Method (b) is necessary when we work in non-exclusive mode and the depth
   is not 8 bpp: in this case, we convert the framebuffer to 16, 24 or 32 bpp
   into a temporary surface and then Blit the temporary surface into the
   window.

   Method (c) is only possible in full-screen mode: we copy the bitmap into a
   backing surface (without any conversion, because we assume full-screen
   mode is always 8bpp) and then order a flip which will happen at vsync.
   This is the best method because it's faster than the other two and makes
   animations as smooth as possible.

*/

char Region[2048];

static raster_t *raster_cache[2];
static int number_of_rasters;

void video_register_raster(raster_t *raster)
{
    int i;
    //  First check if already registered
    for (i = 0; i < number_of_rasters; i++) {
        if (raster_cache[i] == raster) return;
    }
    if (number_of_rasters == 2) {
        log_debug("PANIC: too much rasters...");
        exit(0);
    }
    raster_cache[number_of_rasters++] = raster;
}

raster_t *video_find_raster_for_canvas(video_canvas_t *canvas)
{
    int i;

    for (i = 0; i < number_of_rasters; i++) {
        if (raster_cache[i]->viewport.canvas == canvas) {
            return raster_cache[i];
        }
    }
    return 0;
}

video_canvas_t *canvas_find_canvas_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->hwnd == hwnd) {
            return video_canvases[i];
        }
    }
    return 0;
}

static void clear(HDC hdc, int x1, int y1, int x2, int y2)
{
    static HBRUSH back_color;
    RECT clear_rect;

    if (fullscreen_transition) return;

    if (back_color == NULL)
        back_color = CreateSolidBrush(0);
    clear_rect.left = x1;
    clear_rect.top = y1;
    clear_rect.right = x2;
    clear_rect.bottom = y2;
    FillRect(hdc, &clear_rect, back_color);
}

static void real_refresh(video_canvas_t *c, video_frame_buffer_t *f,
                         unsigned int xs, unsigned int ys,
                         unsigned int xi, unsigned int yi,
                         unsigned int w, unsigned int h);

extern HWND window_handles[2];
extern int number_of_windows;
extern int window_canvas_xsize[2];
extern int window_canvas_ysize[2];
//extern int status_height;

void canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h)
{
    video_canvas_t *c;
    raster_t *r;
    int xs;   //  upperleft x in framebuffer
    int ys;   //  upperleft y in framebuffer
    int xi;   //  upperleft x in client space
    int yi;   //  upperleft y in client space
    int window_index;
    RECT rect;
    int safex;
    int safey;
    int safey2;
    int cut_bottomline;
    int cut_rightline;

    c = canvas_find_canvas_for_hwnd(hwnd);
    if (c) {
        r = video_find_raster_for_canvas(c);
        if (r) {
        for (window_index = 0; window_index < number_of_windows;
            window_index++) {
            if (window_handles[window_index] == hwnd)
                break;
        }

        GetClientRect(hwnd, &rect);
        if (fullscreen_active) {
            rect.right = c->client_width;
            rect.bottom = c->client_height;
        }
        //  Calculate upperleft point's framebuffer coords
#ifndef VIDEO_REMOVE_2X
        xs = xclient - ((rect.right - window_canvas_xsize[window_index]) / 2)
             - r->viewport.x_offset
             + r->viewport.first_x
             * r->viewport.pixel_size.width
             + r->geometry.extra_offscreen_border;
#else /* VIDEO_REMOVE_2X */
        xs = xclient - ((rect.right - window_canvas_xsize[window_index]) / 2)
             - r->viewport.x_offset
             + (r->viewport.first_x + r->geometry.extra_offscreen_border)
             * r->viewport.pixel_size.width;
#endif /* VIDEO_REMOVE_2X */
        ys = yclient - ((rect.bottom - statusbar_get_status_height()
             - window_canvas_ysize[window_index]) / 2)
             - r->viewport.y_offset+r->viewport.first_line *
             r->viewport.pixel_size.height;
        //  Cut off areas outside of framebuffer and clear them
        xi = xclient;
        yi = yclient;

#ifndef VIDEO_REMOVE_2X
        safex = r->viewport.first_x * r->viewport.pixel_size.width
                + r->geometry.extra_offscreen_border - r->viewport.x_offset;
#else /* VIDEO_REMOVE_2X */
        safex = (r->viewport.first_x + r->geometry.extra_offscreen_border)
                * r->viewport.pixel_size.width - r->viewport.x_offset;
#endif /* VIDEO_REMOVE_2X */
        safey = r->viewport.first_line
                * r->viewport.pixel_size.height-r->viewport.y_offset;
        safey2 = r->viewport.last_line
                 * r->viewport.pixel_size.height-r->viewport.y_offset;

        if (r->frame_buffer) {

            cut_rightline=safex+r->viewport.width;
            cut_bottomline=safey+r->viewport.height;
            if (cut_rightline>r->frame_buffer->width) {
                cut_rightline=r->frame_buffer->width;
            }
            if (cut_bottomline>r->frame_buffer->height) {
                cut_bottomline=r->frame_buffer->height;
            }

            //  Check if it's out
            if ((xs + w <= safex) || (xs >= cut_rightline) ||
                (ys + h <= safey) || (ys >= cut_bottomline)) {
                clear(hdc, xi, yi, xi + w, yi + h);
                return;
            }

            //  Cut top
            if (ys < safey) {
                clear(hdc, xi, yi, xi + w, yi - ys + safey);
                yi -= ys - safey;
                h += ys - safey;
                ys = safey;
            }
            //  Cut left
            if (xs < safex) {
                clear(hdc, xi, yi, xi - xs + safex, yi + h);
                xi -= xs - safex;
                w += xs - safex;
                xs = safex;
            }
            //  Cut bottom
            if (ys + h > safey2) {
                clear(hdc, xi, yi + safey2 - ys, xi + w, yi + h);
                h = safey2 - ys;
            }
            //  Cut right
            if (xs + w > cut_rightline) {
                clear(hdc, xi + cut_rightline - xs, yi, xi + w,
                      yi + h);
                w = cut_rightline - xs;
            }

            //  Update remaining area from framebuffer....

            if ((w > 0) && (h > 0)) {
                real_refresh(c, r->frame_buffer, xs, ys, xi, yi, w, h);
            }
        }
        }
    }
}

void video_canvas_refresh(video_canvas_t *c, video_frame_buffer_t *f,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    int window_index;
    unsigned int frame_buffer_x;
    unsigned int frame_buffer_y;
    unsigned int client_x;
    unsigned int client_y;
    RECT rect;

    for (window_index = 0; window_index < number_of_windows; window_index++) {
        if (window_handles[window_index] == c->hwnd)
            break;
    }
    if (window_index == number_of_windows) {
        DEBUG(("PANIC: can't find window"));
        return;
    }
    frame_buffer_x = xs;
    frame_buffer_y = ys;
    client_x = xi;
    client_y = yi;

    GetClientRect(c->hwnd, &rect);
    if (fullscreen_active) {
        rect.right = c->client_width;
        rect.bottom = c->client_height;
    }
    client_x += (rect.right - window_canvas_xsize[window_index]) / 2;
    client_y += (rect.bottom - statusbar_get_status_height()
                - window_canvas_ysize[window_index]) / 2;

    real_refresh(c, f, frame_buffer_x, frame_buffer_y, client_x, client_y, w, h);
}

static void real_refresh(video_canvas_t *c, video_frame_buffer_t *f,
                         unsigned int xs, unsigned int ys,
                         unsigned int xi, unsigned int yi,
                         unsigned int w, unsigned int h)
{
    HRESULT result;
    DDSURFACEDESC desc;
    LPDIRECTDRAWSURFACE surface = NULL;
    RECT rect;
    RECT trect;
    int depth, pitch;

    DWORD starttime;
    DWORD difftime;
    int bytesmoved;
    DWORD *ct;
    DWORD clipsize;
    int regioncount,j;

    int px,py,ph,pw;

    DEBUG(("Entering canvas_render : xs=%d ys=%d xi=%d yi=%d w=%d h=%d",
          xs, ys, xi, yi, w, h));

    if (IsIconic(c->hwnd))
        return;

    if (fullscreen_transition)
        return;

    starttime = timeGetTime();
    bytesmoved = 0;

    {
        extern int syscolorchanged, displaychanged, querynewpalette, palettechanged;

        if (syscolorchanged) {
            ui_error("System colors changed!\n(not implemented yet)");
            syscolorchanged = 0;
        }
        if (displaychanged) {
            displaychanged = 0;
        }
        if (querynewpalette) {
            querynewpalette = 0;
        }
        if (palettechanged) {
            palettechanged = 0;
        }
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = c->client_width;
    rect.bottom = c->client_height;
    rect.left += xi;
    rect.top += yi;
    ClientToScreen(c->hwnd, (LPPOINT) &rect);
    rect.right = rect.left + w;
    rect.bottom = rect.top + h;

    if (c->back_surface != NULL) {
        desc.dwSize = sizeof(desc);
        do {
            result = IDirectDrawSurface_Lock(c->back_surface, NULL, &desc,
                                             DDLOCK_WAIT, NULL);
            if (result == DDERR_SURFACELOST)
                if (IDirectDrawSurface_Restore(c->back_surface) != DD_OK)
                    break;
        } while (result == DDERR_SURFACELOST);
        if (result == DD_OK)
            surface = c->back_surface;
    }

    if (surface == NULL) {
        desc.dwSize = sizeof(desc);
        do {
            result = IDirectDrawSurface_Lock(c->primary_surface, NULL, &desc,
                                             0, NULL);
            if (result == DDERR_SURFACELOST)
                if (IDirectDrawSurface_Restore(c->primary_surface) != DD_OK)
                    break;
        } while (result == DDERR_SURFACELOST);
        if (result == DD_OK)
            surface = c->primary_surface;
                else {
                        static int no_primary_lock_reported;

                        if (!no_primary_lock_reported
                            && result == DDERR_CANTLOCKSURFACE) {
                                ui_error("Your DirectDraw driver does not let me lock\n"
                                         "the primary DirectDraw surface.\n\n"
                                         "Performance will be poor!");
                                no_primary_lock_reported = 1;
                        }
                }
    }
    if (surface == NULL) {
        /* Try to lock the temporary surface (last resort).  */
        desc.dwSize = sizeof(desc);
        do {
            result = IDirectDrawSurface_Lock(c->temporary_surface, NULL, &desc,
                                             0, NULL);
            if (result == DDERR_SURFACELOST)
                if (IDirectDrawSurface_Restore(c->temporary_surface) != DD_OK)
                    break;
        } while (result == DDERR_WASSTILLDRAWING);
        if (result == DD_OK)
            surface = c->temporary_surface;
    }

    if (surface == NULL) {
        DEBUG(("Can't lock surface"));
        return;
    }

#ifdef HAVE_UNNAMED_UNIONS
    depth = desc.ddpfPixelFormat.dwRGBBitCount;
    pitch = desc.lPitch;
#else
    depth = desc.ddpfPixelFormat.u1.dwRGBBitCount;
    pitch = desc.u1.lPitch;
#endif

    DEBUG(("Original rect: %d %d %d %d", rect.left, rect.top, rect.right,
          rect.bottom));

    result = IDirectDrawClipper_SetHWnd(c->clipper, 0, c->hwnd);
    if (result != DD_OK) {
        ui_error("Cannot set HWND for primary surface clipper:\n%s",
                 dd_error(result));
    }

    clipsize = 2048;
    IDirectDrawClipper_GetClipList(c->clipper, &rect, (LPRGNDATA)&Region,
                                   &clipsize);
    regioncount = ((RGNDATA*)Region)->rdh.nCount;
    DEBUG(("REGION count: %d",regioncount));

    for (j = 0; j < regioncount; j++) {
        trect.top = ((RECT*)((RGNDATA*)Region)->Buffer)[j].top;
        trect.bottom = ((RECT*)((RGNDATA*)Region)->Buffer)[j].bottom;
        trect.left = ((RECT*)((RGNDATA*)Region)->Buffer)[j].left;
        trect.right = ((RECT*)((RGNDATA*)Region)->Buffer)[j].right;
        DEBUG(("RECT: %d %d %d %d", trect.left, trect.top, trect.right,
              trect.bottom));
        px = xs + trect.left - rect.left;
        py = ys + trect.top - rect.top;
        pw = trect.right - trect.left;
        ph = trect.bottom - trect.top;

        video_render_main(ct=c->physical_colors,
                          (BYTE *)(VIDEO_FRAME_BUFFER_START(f)),
                          (BYTE *)(desc.lpSurface),
                          pw, ph,
                          px, py,
                          trect.left, trect.top,
                          VIDEO_FRAME_BUFFER_LINE_SIZE(f), pitch,
                          depth,c->videorendermode);
    }

    if (IDirectDrawSurface_Unlock(surface, NULL) == DDERR_SURFACELOST) {
        IDirectDrawSurface_Restore(surface);
        IDirectDrawSurface_Unlock(surface, NULL);
    }
    if (surface == c->back_surface) {
        /* Back surface: we can flip.  */
    } else if (surface == c->primary_surface) {
        /* Nothing to do...  the window has already been updated.  */
    } else{
        /* Temporary surface: we have to blit.  */
        for (j = 0; j < regioncount; j++) {
            trect.top = ((RECT*)((RGNDATA*)Region)->Buffer)[j].top;
            trect.bottom = ((RECT*)((RGNDATA*)Region)->Buffer)[j].bottom;
            trect.left = ((RECT*)((RGNDATA*)Region)->Buffer)[j].left;
            trect.right = ((RECT*)((RGNDATA*)Region)->Buffer)[j].right;
            result = IDirectDrawSurface_Blt(c->primary_surface,
                                            &trect,
                                            surface,
                                            &trect,
                                            DDBLT_WAIT, NULL);
            if (result == DD_OK) {
            }else if (result == DDERR_INVALIDRECT) {
                DEBUG(("INVALID rect %d, %d, %d, %d",
                       rect.left, rect.top,
                       rect.right, rect.bottom));
            } else if (result == DDERR_SURFACELOST) {
                result = IDirectDrawSurface_Restore(c->primary_surface);
                if (result != DD_OK) {
                }
            } else if (result==DDERR_SURFACEBUSY) {
            } else {
                    ui_error("Cannot update emulation window:\n%s",
                             dd_error(result));
            }
        }
    }
    difftime = timeGetTime() - starttime;
    DEBUG(("screen update took %d msec, moved %d bytes, width %d, height %d",
          difftime, bytesmoved, w, h));
}

