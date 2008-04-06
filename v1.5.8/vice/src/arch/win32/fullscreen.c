/*
 * fullscreen.c - Fullscreen related support functions for Win32
 *
 * Written by
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

#include <stdlib.h>

#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include "res.h"
#include "ui.h"
#include "winmain.h"
#include "palette.h"
#include "resources.h"
#include "raster.h"
#include "utils.h"
#include "log.h"
#include "videoarch.h"

raster_t *video_find_raster_for_canvas(canvas_t *canvas);


// ----------------------------------------------

#ifndef HAVE_GUIDLIB
const GUID IID_IDirectDraw2={0xB3A6F3E0,0x2B43,0x11CF,
{0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56}};
#endif

typedef struct _DDL {
    struct _DDL *next;
    int         isNullGUID;
    GUID        guid;
    LPSTR       desc;
} DirectDrawDeviceList;

typedef struct _ML {
    struct _ML  *next;
    int         devicenumber;
    int         width;
    int         height;
    int         bitdepth;
    int         refreshrate;
} DirectDrawModeList;

static  DirectDrawDeviceList    *devices=NULL;
static  DirectDrawModeList      *modes=NULL;
static  LPDIRECTDRAW            DirectDrawObject;
static  LPDIRECTDRAW2           DirectDrawObject2;

#define CHECK_DDRESULT(ddresult)    {\
if (ddresult!=DD_OK) {\
ui_error("DirectDraw error: Code:%8x Error:%s",ddresult,dd_error(ddresult));\
}}\

BOOL WINAPI DDEnumCallbackFunction(
  GUID FAR *lpGUID,           
  LPSTR  lpDriverDescription, 
  LPSTR  lpDriverName,        
  LPVOID lpContext            
)
{
DirectDrawDeviceList    *new_device;
DirectDrawDeviceList    *search_device;

    new_device=malloc(sizeof(DirectDrawDeviceList));
    new_device->next=NULL;
    if (lpGUID!=NULL) {
        memcpy(&new_device->guid,lpGUID,sizeof(GUID));
        new_device->isNullGUID=0;
    } else {
        new_device->isNullGUID=1;
    }
    new_device->desc=stralloc(lpDriverDescription);
    if (devices==NULL) {
        devices=new_device;
    } else {
        search_device=devices;
        while (search_device->next!=NULL) {
            search_device=search_device->next;
        }
        search_device->next=new_device;
    }

/*
    log_debug("--------- DirectDraw Device ---");
    log_debug("GUID: %16x",lpGUID);
    log_debug("Desc: %s",lpDriverDescription);
    log_debug("Name: %s",lpDriverName);
*/
    return DDENUMRET_OK;
}
 
HRESULT WINAPI ModeCallBack(LPDDSURFACEDESC desc, LPVOID context)
{
DirectDrawModeList  *new_mode;
DirectDrawModeList  *search_mode;

    new_mode=malloc(sizeof(DirectDrawModeList));
    new_mode->next=NULL;
    new_mode->devicenumber=*(int*)context;
    new_mode->width=new_mode->height=new_mode->bitdepth=new_mode->refreshrate=0;
    if (desc->dwFlags&(DDSD_WIDTH)) {
//        log_debug("Width:       %d",desc->dwWidth);
        new_mode->width=desc->dwWidth;
    }
    if (desc->dwFlags&(DDSD_HEIGHT)) {
//        log_debug("Height:      %d",desc->dwHeight);
        new_mode->height=desc->dwHeight;
    }
    if (desc->dwFlags&(DDSD_PIXELFORMAT)) {
//        log_debug("Bitdepth:    %d",desc->ddpfPixelFormat.dwRGBBitCount);
//        log_debug("Red mask:    %04x",desc->ddpfPixelFormat.dwRBitMask);
//        log_debug("Blue mask:   %04x",desc->ddpfPixelFormat.dwBBitMask);
//        log_debug("Green mask:  %04x",desc->ddpfPixelFormat.dwGBitMask);
#ifdef HAVE_UNNAMED_UNIONS
        new_mode->bitdepth=desc->ddpfPixelFormat.dwRGBBitCount;
#else
        new_mode->bitdepth=desc->ddpfPixelFormat.u1.dwRGBBitCount;
#endif
    }
    if (desc->dwFlags&(DDSD_REFRESHRATE)) {
//        log_debug("Refreshrate: %d",desc->dwRefreshRate);
#ifdef HAVE_UNNAMED_UNIONS
        new_mode->refreshrate=desc->dwRefreshRate;
#else
        new_mode->refreshrate=desc->u2.dwRefreshRate;
#endif
    }
    if (modes==NULL) {
        modes=new_mode;
    } else {
        search_mode=modes;
        while (search_mode->next!=NULL) {
            search_mode=search_mode->next;
        }
        search_mode->next=new_mode;
    }
    return DDENUMRET_OK;
}

void fullscreen_getmodes(void)
{
HRESULT                 ddresult;
DirectDrawDeviceList    *search_device;
int                     i;

    /*  Enumerate DirectDraw Devices */
    ddresult = DirectDrawEnumerate(DDEnumCallbackFunction, NULL);

    /*  List all available modes for all available devices */
    search_device=devices;
    i=0;
    while (search_device!=NULL) {
//        log_debug("--- Video modes for device %s",search_device->desc);
//        log_debug("MODEPROBE_Create");
        if (search_device->isNullGUID) {
            ddresult = DirectDrawCreate(NULL, &DirectDrawObject, NULL);
        } else {
            ddresult = DirectDrawCreate(&search_device->guid, &DirectDrawObject, NULL);
        }
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_SetCooperativeLevel");
        ddresult = IDirectDraw_SetCooperativeLevel(DirectDrawObject, ui_get_main_hwnd(), DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_NOWINDOWCHANGES);
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_ObtainDirectDraw2");
        ddresult=IDirectDraw_QueryInterface(DirectDrawObject,&IID_IDirectDraw2,(LPVOID *)&DirectDrawObject2);
        CHECK_DDRESULT(ddresult);
//        log_debug("MODEPROBE_EnumDisplayModes");
        ddresult=IDirectDraw2_EnumDisplayModes(DirectDrawObject2,DDEDM_REFRESHRATES,NULL,&i,ModeCallBack);
        CHECK_DDRESULT(ddresult);
        IDirectDraw2_Release(DirectDrawObject2);
        DirectDrawObject2=NULL;
        IDirectDraw_Release(DirectDrawObject);
        DirectDrawObject=NULL;
        search_device=search_device->next;
        i++;
    }
}

GUID *GetGUIDForActualDevice()
{
int                     device;
DirectDrawDeviceList    *search_device;

    resources_get_value("FullscreenDevice",(resource_value_t *)&device);
    search_device=devices;
    while (search_device!=NULL) {
        if (device==0) {
            if (search_device->isNullGUID) {
                return NULL;
            } else {
                return (&search_device->guid);
            }
        }
        device--;
        search_device=search_device->next;
    }
    return NULL;
}

void validate_mode(int *device, int *width, int *height, int *bitdepth, int *rate)
{
DirectDrawModeList  *mode;

//  Validate devicenumber
    mode=modes;
    while (mode!=NULL) {
        if (mode->devicenumber==*device) break;
        mode=mode->next;
    }
    if (mode==NULL) {
        *device=modes->devicenumber;
    }

//  Validate bitdepth
    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==*device) && (mode->bitdepth==*bitdepth)) break;
        mode=mode->next;
    }
    if (mode==NULL) {
        mode=modes;
        while (mode!=NULL) {
            if (mode->devicenumber==*device) {
                *bitdepth=mode->bitdepth;
                break;
            }
            mode=mode->next;
        }
    }

//  Validate resolution
    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==*device) && (mode->bitdepth==*bitdepth) && (mode->width==*width) && (mode->height==*height)) break;
        mode=mode->next;
    }
    if (mode==NULL) {
        mode=modes;
        while (mode!=NULL) {
            if ((mode->devicenumber==*device) && (mode->bitdepth==*bitdepth)) {
                *width=mode->width;
                *height=mode->height;
                break;
            }
            mode=mode->next;
        }
    }

//  Validate refreshrate
    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==*device) && (mode->bitdepth==*bitdepth) && (mode->width==*width) && (mode->height==*height) && (mode->refreshrate==*rate)) break;
        mode=mode->next;
    }
    if (mode==NULL) {
        *rate=0;
    }
}

typedef struct _VL {
    struct _VL  *next;
    struct _VL  *prev;
    char        *text;
    int         value;
} ValueList;

ValueList   *bitdepthlist=NULL;
ValueList   *resolutionlist=NULL;
ValueList   *refresh_rates=NULL;

int         fullscreen_device=0;
int         fullscreen_bitdepth=0;
int         fullscreen_width=0;
int         fullscreen_height=0;
int         fullscreen_refreshrate=0;

int GetIndexFromList (ValueList *list, int value)
{
ValueList   *search;
int         pos;

    pos=0;
    search=list;
    while (search!=NULL) {
        if (search->value==value) return pos;
        search=search->next;
        pos++;
    }
    return -1;
}

int GetValueFromList (ValueList * list, int index)
{
ValueList   *search;
int         pos;

    search=list;
    pos=0;
    while (search!=NULL) {
        if (pos==index) return search->value;
        pos++;
        search=search->next;
    }
    return 0;
}

void InsertInto (ValueList **list, ValueList *value)
{
ValueList   *after;
ValueList   *before;

    after=*list;
    before=NULL;
    while (after!=NULL) {
        if (value->value<after->value) {
            break;
        }
        before=after;
        after=after->next;
    }
    value->prev=before;
    value->next=after;
    if (*list==NULL) {
        *list=value;
    } else if (after==NULL) {
        before->next=value;
    } else if (before==NULL) {
        after->prev=value;
        *list=value;
    } else {
        before->next=value;
        after->prev=value;
    }
}

void DestroyList (ValueList **list)
{
ValueList   *value;
ValueList   *value2;

    value=*list;
    while (value!=NULL) {
        free(value->text);
        value2=value->next;
        free(value);
        value=value2;
    }
    *list=NULL;
}

void get_refreshratelist(int device, int bitdepth, int width, int height)
{
DirectDrawModeList  *mode;
ValueList           *value;
char                buff[256];

    DestroyList(&refresh_rates);

    //  We always need 'Default' as when support for different
    //  Refreshrates exists, then it is not reported back
    value=malloc(sizeof(ValueList));
    value->value=0;
    value->text=stralloc("Default");
    InsertInto(&refresh_rates, value);

    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==device) && (mode->bitdepth==bitdepth) && (mode->width==width) && (mode->height==height)) {
            if (GetIndexFromList(refresh_rates,mode->refreshrate)==-1) {
                value=malloc(sizeof(ValueList));
                value->value=mode->refreshrate;
                itoa(mode->refreshrate,buff,10);
                value->text=stralloc(buff);
                InsertInto(&refresh_rates, value);
            }
        }
        mode=mode->next;
    }
}

void get_bitdepthlist(int device)
{
DirectDrawModeList  *mode;
ValueList           *value;
char                buff[256];

    DestroyList(&bitdepthlist);
    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==device)) {
            if (GetIndexFromList(bitdepthlist,mode->bitdepth)==-1) {
                value=malloc(sizeof(ValueList));
                value->value=mode->bitdepth;
                itoa(mode->bitdepth,buff,10);
                value->text=stralloc(buff);
                InsertInto(&bitdepthlist,value);
            }
        }
        mode=mode->next;
    }
}

void get_resolutionlist(int device, int bitdepth)
{
DirectDrawModeList  *mode;
ValueList           *value;
char                buff[256];

    DestroyList(&resolutionlist);
    mode=modes;
    while (mode!=NULL) {
        if ((mode->devicenumber==device) && (mode->bitdepth==bitdepth)) {
            if (GetIndexFromList(resolutionlist,((mode->width<<16)+mode->height))==-1) {
                value=malloc(sizeof(ValueList));
                value->value=(mode->width<<16)+mode->height;
                sprintf(buff,"%dx%d",mode->width,mode->height);
                value->text=stralloc(buff);
                InsertInto(&resolutionlist,value);
            }
        }
        mode=mode->next;
    }
}

static void init_fullscreen_dialog(HWND hwnd)
{
HWND                    setting_hwnd;
DirectDrawDeviceList    *dev;
ValueList               *value;

    validate_mode(&fullscreen_device,&fullscreen_width,&fullscreen_height,&fullscreen_bitdepth,&fullscreen_refreshrate);
    setting_hwnd=GetDlgItem(hwnd, IDC_FULLSCREEN_DEVICE);
    SendMessage(setting_hwnd,CB_RESETCONTENT,0,0);
    dev=devices;
    while (dev!=NULL) {
        SendMessage(setting_hwnd,CB_ADDSTRING,0,(LPARAM)dev->desc);
        dev=dev->next;
    }
    SendMessage(setting_hwnd,CB_SETCURSEL,(WPARAM)fullscreen_device,0);

    get_bitdepthlist(fullscreen_device);
    setting_hwnd=GetDlgItem(hwnd, IDC_FULLSCREEN_BITDEPTH);
    SendMessage(setting_hwnd,CB_RESETCONTENT,0,0);
    value=bitdepthlist;
    while (value!=NULL) {
        SendMessage(setting_hwnd,CB_ADDSTRING,0,(LPARAM)value->text);
        value=value->next;
    }
    SendMessage(setting_hwnd,CB_SETCURSEL,(WPARAM)GetIndexFromList(bitdepthlist,fullscreen_bitdepth),0);

    get_resolutionlist(fullscreen_device,fullscreen_bitdepth);
    setting_hwnd=GetDlgItem(hwnd, IDC_FULLSCREEN_RESOLUTION);
    SendMessage(setting_hwnd,CB_RESETCONTENT,0,0);
    value=resolutionlist;
    while (value!=NULL) {
        SendMessage(setting_hwnd,CB_ADDSTRING,0,(LPARAM)value->text);
        value=value->next;
    }
    SendMessage(setting_hwnd,CB_SETCURSEL,(WPARAM)GetIndexFromList(resolutionlist,(fullscreen_width<<16)+fullscreen_height),0);

    get_refreshratelist(fullscreen_device,fullscreen_bitdepth,fullscreen_width,fullscreen_height);
    setting_hwnd=GetDlgItem(hwnd, IDC_FULLSCREEN_REFRESHRATE);
    SendMessage(setting_hwnd,CB_RESETCONTENT,0,0);
    value=refresh_rates;
    while (value!=NULL) {
        SendMessage(setting_hwnd,CB_ADDSTRING,0,(LPARAM)value->text);
        value=value->next;
    }
    SendMessage(setting_hwnd,CB_SETCURSEL,(WPARAM)GetIndexFromList(refresh_rates,fullscreen_refreshrate),0);
}


static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
int notifycode;
int item;
int index;
int value;
int command;

    switch (msg) {
        case WM_COMMAND:
            notifycode=HIWORD(wparam);
            item=LOWORD(wparam);
            if (notifycode==CBN_SELENDOK) {
                if (item==IDC_FULLSCREEN_DEVICE) { 
                    fullscreen_device=SendMessage(GetDlgItem(hwnd,IDC_FULLSCREEN_DEVICE),CB_GETCURSEL,0,0);
                } else if (item==IDC_FULLSCREEN_BITDEPTH) {
                    index=SendMessage(GetDlgItem(hwnd,IDC_FULLSCREEN_BITDEPTH),CB_GETCURSEL,0,0);
                    fullscreen_bitdepth=GetValueFromList(bitdepthlist,index);
                } else if (item==IDC_FULLSCREEN_RESOLUTION) {
                    index=SendMessage(GetDlgItem(hwnd,IDC_FULLSCREEN_RESOLUTION),CB_GETCURSEL,0,0);
                    value=GetValueFromList(resolutionlist,index);
                    fullscreen_width=value>>16;
                    fullscreen_height=value&0xffff;
                } else if (item==IDC_FULLSCREEN_REFRESHRATE) {
                    index=SendMessage(GetDlgItem(hwnd,IDC_FULLSCREEN_REFRESHRATE),CB_GETCURSEL,0,0);
                    fullscreen_refreshrate=GetValueFromList(refresh_rates,index);
                }
                init_fullscreen_dialog(hwnd);
            } else {
                command=LOWORD(wparam);
                switch (command) {
                    case IDOK:
                        resources_set_value("FullScreenDevice",(resource_value_t)fullscreen_device);
                        resources_set_value("FullScreenBitdepth",(resource_value_t)fullscreen_bitdepth);
                        resources_set_value("FullScreenWidth",(resource_value_t)fullscreen_width);
                        resources_set_value("FullScreenHeight",(resource_value_t)fullscreen_height);
                        resources_set_value("FullScreenRefreshRate",(resource_value_t)fullscreen_refreshrate);
                    case IDCANCEL:
                        EndDialog(hwnd,0);
                        return TRUE;
                }
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd,0);
            return TRUE;
        case WM_INITDIALOG:
            resources_get_value("FullscreenDevice",(resource_value_t *)&fullscreen_device);
            resources_get_value("FullscreenBitdepth",(resource_value_t *)&fullscreen_bitdepth);
            resources_get_value("FullscreenWidth",(resource_value_t *)&fullscreen_width);
            resources_get_value("FullscreenHeight",(resource_value_t *)&fullscreen_height);
            resources_get_value("FullscreenRefreshRate",(resource_value_t *)&fullscreen_refreshrate);
            init_fullscreen_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_fullscreen_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance,(LPCTSTR)IDD_FULLSCREEN_SETTINGS_DIALOG,hwnd,dialog_proc);
}

void ui_fullscreen_init(void)
{
    fullscreen_getmodes();
}

int IsFullscreenEnabled(void)
{
int b;

    resources_get_value("FullscreenEnabled",(resource_value_t *)&b);
    return b;
}

void GetCurrentModeParameters(int *width, int *height, int *bitdepth, int *refreshrate)
{
    resources_get_value("FullscreenBitdepth",(resource_value_t *)bitdepth);
    resources_get_value("FullscreenWidth",(resource_value_t *)width);
    resources_get_value("FullscreenHeight",(resource_value_t *)height);
    resources_get_value("FullscreenRefreshRate",(resource_value_t *)refreshrate);
}


HMENU   old_menu;
RECT    old_rect;
DWORD   old_style;
int     old_width;
int     old_height;
int     old_bitdepth;
int     old_client_width;
int     old_client_height;
int     fullscreen_active;

void SwitchToFullscreenMode(HWND hwnd)
{
int             w,h,wnow,hnow;
int             fullscreen_width;
int             fullscreen_height;
int             bitdepth;
int             refreshrate;
canvas_t        *c;
HRESULT         ddresult;
DDSURFACEDESC   desc;
DDSURFACEDESC   desc2;
GUID            *device_guid;
int             i;
HDC             hdc;
raster_t        *r;

    //  Get fullscreen parameters
    GetCurrentModeParameters(&fullscreen_width,&fullscreen_height,&bitdepth,&refreshrate);
    //  Get the Canvas for this window
    c=canvas_find_canvas_for_hwnd(hwnd);


    memset(&desc2,0,sizeof(desc2));
    desc2.dwSize=sizeof(desc2);
    ddresult=IDirectDraw2_GetDisplayMode(c->dd_object2,&desc2);
    old_width=desc2.dwWidth;
    old_height=desc2.dwHeight;
#ifdef HAVE_UNNAMED_UNIONS
    old_bitdepth=desc2.ddpfPixelFormat.dwRGBBitCount;;
#else
    old_bitdepth=desc2.ddpfPixelFormat.u1.dwRGBBitCount;;
#endif

    IDirectDrawSurface_Release(c->temporary_surface);
    IDirectDrawSurface_Release(c->primary_surface);
    IDirectDraw_Release(c->dd_object2);
    IDirectDraw_Release(c->dd_object);

    //  Remove Window Styles
    old_style=GetWindowLong(hwnd,GWL_STYLE);
    GetWindowRect(hwnd,&old_rect);
    SetWindowLong(hwnd,GWL_STYLE,old_style&~WS_SYSMENU&~WS_CAPTION);
    //  Remove Menu
    old_menu=GetMenu(hwnd);
    SetMenu(hwnd,NULL);
    //  Cover screen with window
    wnow=GetSystemMetrics(SM_CXSCREEN);
    hnow=GetSystemMetrics(SM_CYSCREEN);
    w=(fullscreen_width>wnow) ? fullscreen_width : wnow;
    h=(fullscreen_height>hnow) ? fullscreen_height : hnow;
    SetWindowPos(hwnd,HWND_TOPMOST,0,0,w,h,SWP_NOCOPYBITS);
    ShowCursor(FALSE);

    device_guid=GetGUIDForActualDevice();
    ddresult=DirectDrawCreate(device_guid, &c->dd_object, NULL);
    ddresult=IDirectDraw_SetCooperativeLevel(c->dd_object, c->hwnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_NOWINDOWCHANGES);
    ddresult=IDirectDraw_QueryInterface(c->dd_object,&IID_IDirectDraw2,(LPVOID *)&c->dd_object2);

    //  Set cooperative level
    ddresult=IDirectDraw_SetCooperativeLevel(c->dd_object, c->hwnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_NOWINDOWCHANGES);
    //  Set Mode
    ddresult=IDirectDraw2_SetDisplayMode(c->dd_object2,fullscreen_width,fullscreen_height,bitdepth,refreshrate,0);
    //  Adjust window size
    old_client_width=c->client_width;
    old_client_height=c->client_height;
    c->client_width=fullscreen_width;
    c->client_height=fullscreen_height;

    /*  Create Primary surface */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc, &c->primary_surface, NULL);
    if (ddresult != DD_OK) {
    }
    ddresult = IDirectDraw2_CreateClipper(c->dd_object2, 0, &c->clipper, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create clipper for primary surface:\n%s",
                 dd_error(ddresult));
    }
    ddresult = IDirectDrawSurface_SetClipper(c->primary_surface, c->clipper);
    if (ddresult != DD_OK) {
        ui_error("Cannot set clipper for primary surface:\n%s",
                 dd_error(ddresult));
    }

    /* Create the temporary surface.  */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    /* FIXME: SYSTEMMEMORY?  */
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = fullscreen_width;
    desc.dwHeight = fullscreen_height;
    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc, &c->temporary_surface, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create temporary DirectDraw surface:\n%s",
                 dd_error(ddresult));
    }

    c->depth=bitdepth;
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
        }
    }

    set_palette(c);
    set_physical_colors(c);

    r=video_find_raster_for_canvas(c);
    video_frame_buffer_translate(c);
    raster_rebuild_tables(r);
    IDirectDrawSurface_GetDC(c->primary_surface,&hdc);
    canvas_update(c->hwnd,hdc,0,0,fullscreen_width,fullscreen_height);
    IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);
    fullscreen_active=1;

}

void SwitchToWindowedMode(HWND hwnd)
{
canvas_t        *c;
HRESULT         ddresult;
DDSURFACEDESC   desc;
DDSURFACEDESC   desc2;
int             i;
HDC             hdc;
raster_t        *r;

    //  Get the Canvas for this window
    c=canvas_find_canvas_for_hwnd(hwnd);

    IDirectDrawSurface_Release(c->temporary_surface);
    IDirectDrawSurface_Release(c->primary_surface);
    ddresult = IDirectDraw_SetCooperativeLevel(c->dd_object, NULL, DDSCL_NORMAL);
    IDirectDraw_RestoreDisplayMode(c->dd_object);
    IDirectDraw_Release(c->dd_object2);
    IDirectDraw_Release(c->dd_object);


    LockWindowUpdate(hwnd);
    SetWindowLong(hwnd,GWL_STYLE,old_style);
    //  Remove Menu
    SetMenu(hwnd,old_menu);
    SetWindowPos(hwnd,HWND_TOP,old_rect.left,old_rect.top,old_rect.right-old_rect.left,old_rect.bottom-old_rect.top,SWP_NOCOPYBITS);
    ShowCursor(TRUE);
    c->client_width=old_client_width;
    c->client_height=old_client_height;
    LockWindowUpdate(NULL);


    ddresult=DirectDrawCreate(NULL, &c->dd_object, NULL);
    ddresult=IDirectDraw_SetCooperativeLevel(c->dd_object, NULL, DDSCL_NORMAL);
    ddresult=IDirectDraw_QueryInterface(c->dd_object,&IID_IDirectDraw2,(LPVOID *)&c->dd_object2);

    /*  Create Primary surface */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc, &c->primary_surface, NULL);
    if (ddresult != DD_OK) {
    }
    ddresult = IDirectDraw2_CreateClipper(c->dd_object2, 0, &c->clipper, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create clipper for primary surface:\n%s",
                 dd_error(ddresult));
    }
    ddresult = IDirectDrawSurface_SetClipper(c->primary_surface, c->clipper);
    if (ddresult != DD_OK) {
        ui_error("Cannot set clipper for primary surface:\n%s",
                 dd_error(ddresult));
    }

    memset(&desc2,0,sizeof(desc2));
    desc2.dwSize=sizeof(desc2);
    ddresult=IDirectDraw2_GetDisplayMode(c->dd_object2,&desc2);

    /* Create the temporary surface.  */
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    /* FIXME: SYSTEMMEMORY?  */
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = desc2.dwWidth;
    desc.dwHeight = desc2.dwHeight;
    ddresult = IDirectDraw2_CreateSurface(c->dd_object2, &desc, &c->temporary_surface, NULL);
    if (ddresult != DD_OK) {
        ui_error("Cannot create temporary DirectDraw surface:\n%s",
                 dd_error(ddresult));
    }

    c->depth=old_bitdepth;


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
        }
    }

    set_palette(c);
    set_physical_colors(c);


    r=video_find_raster_for_canvas(c);
    video_frame_buffer_translate(c);
    raster_rebuild_tables(r);
    IDirectDrawSurface_GetDC(c->primary_surface,&hdc);
    canvas_update(c->hwnd,hdc,0,0,c->client_width,c->client_height);
    IDirectDrawSurface_ReleaseDC(c->primary_surface, hdc);
    fullscreen_active=0;
}


void StartFullscreenMode(HWND hwnd)
{
    SwitchToFullscreenMode(hwnd);
    resources_set_value("FullScreenEnabled",(resource_value_t)1);
}


void EndFullscreenMode(HWND hwnd)
{
    SwitchToWindowedMode(hwnd);
    resources_set_value("FullScreenEnabled",(resource_value_t)0);
}

void SwitchFullscreenMode(HWND hwnd)
{
    if (IsFullscreenEnabled()) {
        EndFullscreenMode(hwnd);
    } else {
        StartFullscreenMode(hwnd);
    }
}


int     fullscreen_nesting_level=0;

void SuspendFullscreenMode(HWND hwnd)
{
    if (IsFullscreenEnabled()) {
        if (fullscreen_nesting_level==0) {
            SwitchToWindowedMode(hwnd);
        }
        fullscreen_nesting_level++;
    }
}

void ResumeFullscreenMode(HWND hwnd)
{
    if (IsFullscreenEnabled()) {
        fullscreen_nesting_level--;
        if (fullscreen_nesting_level==0) {
            SwitchToFullscreenMode(hwnd);
        }
    }
}
