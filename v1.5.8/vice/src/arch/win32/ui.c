/*
 * ui.c - Windows user interface.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@mail.matav.hu>
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
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#ifdef HAVE_COMMCTRL_H
#include <commctrl.h>
#endif

#include "attach.h"
#include "autostart.h"
#include "archdep.h"
#include "cmdline.h"
#include "datasette.h"
#include "drive.h"
#include "fliplist.h"
#include "fullscreen.h"
#include "imagecontents.h"
#include "info.h"
#include "interrupt.h"
#include "kbd.h"
#include "keyboard.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "mon.h"
#include "mos6510.h"
#include "mouse.h"
#include "res.h"
#include "resources.h"
#include "tape.h"
#include "types.h"
#include "ui.h"
#include "uiattach.h"
#include "uicmdline.h"
#include "uidrive.h"
#include "uidatasette.h"
#include "uijoystick.h"
#include "uilib.h"
#include "uisnapshot.h"
#include "uisound.h"
#include "utils.h"
#include "version.h"
#include "vsync.h"
#include "winmain.h"

static HWND status_hwnd[2];

int status_height;

static char *hwnd_titles[2];

/* Exposure handler.  */
HWND            window_handles[2];
canvas_redraw_t exposure_handler[2];
int             number_of_windows;
int             window_canvas_xsize[2];
int             window_canvas_ysize[2];

static HACCEL   ui_accelerator;

/* Forward prototypes.  */
static long CALLBACK window_proc(HWND window, UINT msg,
                                 WPARAM wparam, LPARAM lparam);

/* List of resources that can be switched on and off from the menus.  */
ui_menu_toggle  toggle_list[] = {
    { "Sound", IDM_TOGGLE_SOUND },
    { "DriveTrueEmulation", IDM_TOGGLE_DRIVE_TRUE_EMULATION },
    { "EmuID", IDM_TOGGLE_EMUID },
    { "WarpMode", IDM_TOGGLE_WARP_MODE },
    { "WarpMode", IDM_TOGGLE_WARP_MODE|0x00010000 },
    { "VirtualDevices", IDM_TOGGLE_VIRTUAL_DEVICES },
    { "SaveResourcesOnExit", IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", IDM_TOGGLE_CONFIRM_ON_EXIT },
    { "FullScreenEnabled", IDM_TOGGLE_FULLSCREEN },
    { NULL, 0 }
};

/*  List of resources which can have multiple mutual exclusive menu entries. */

ui_res_possible_values RefreshRateValues[] = {
        {0, IDM_REFRESH_RATE_AUTO},
        {1, IDM_REFRESH_RATE_1},
        {2, IDM_REFRESH_RATE_2},
        {3, IDM_REFRESH_RATE_3},
        {4, IDM_REFRESH_RATE_4},
        {5, IDM_REFRESH_RATE_5},
        {6, IDM_REFRESH_RATE_6},
        {7, IDM_REFRESH_RATE_7},
        {8, IDM_REFRESH_RATE_8},
        {9, IDM_REFRESH_RATE_9},
        {10, IDM_REFRESH_RATE_10},
        {-1, 0}
};

ui_res_possible_values SpeedValues[] = {
        {0, IDM_MAXIMUM_SPEED_NO_LIMIT},
        {10, IDM_MAXIMUM_SPEED_10},
        {20, IDM_MAXIMUM_SPEED_20},
        {50, IDM_MAXIMUM_SPEED_50},
        {100, IDM_MAXIMUM_SPEED_100},
        {200, IDM_MAXIMUM_SPEED_200},
        {-1, 0}
};

ui_res_possible_values SyncFactor[] = {
        {DRIVE_SYNC_PAL, IDM_SYNC_FACTOR_PAL},
        {DRIVE_SYNC_NTSC, IDM_SYNC_FACTOR_NTSC},
        {DRIVE_SYNC_NTSCOLD, IDM_SYNC_FACTOR_NTSCOLD},
        {-1, 0}
};

ui_res_value_list value_list[] = {
    {"RefreshRate", RefreshRateValues},
    {"Speed", SpeedValues},
    {"VideoStandard", SyncFactor},
    {NULL,NULL}
};

/* ------------------------------------------------------------------------ */

/* UI-related resources.  */

struct {
    int fullscreendevice;
    int fullscreenbitdepth;
    int fullscreenwidth;
    int fullscreenheight;
    int fullscreenrefreshrate;
    int fullscreenenabled;
    int save_resources_on_exit;
    int confirm_on_exit;
} ui_resources;

static int set_fullscreen_device(resource_value_t v, void *param)
{
    ui_resources.fullscreendevice=(int) v;
    return 0;
}

static int set_fullscreen_bitdepth(resource_value_t v, void *param)
{
    ui_resources.fullscreenbitdepth=(int) v;
    return 0;
}

static int set_fullscreen_width(resource_value_t v, void *param)
{
    ui_resources.fullscreenwidth=(int) v;
    return 0;
}

static int set_fullscreen_height(resource_value_t v, void *param)
{
    ui_resources.fullscreenheight=(int) v;
    return 0;
}

static int set_fullscreen_refreshrate(resource_value_t v, void *param)
{
    ui_resources.fullscreenrefreshrate=(int) v;
    return 0;
}

static int set_fullscreen_enabled(resource_value_t v, void *param)
{
    ui_resources.fullscreenenabled=(int) v;
    return 0;
}

static int set_save_resources_on_exit(resource_value_t v, void *param)
{
    ui_resources.save_resources_on_exit=(int) v;
    return 0;
}

static int set_confirm_on_exit(resource_value_t v, void *param)
{
    ui_resources.confirm_on_exit=(int) v;
    return 0;
}


static resource_t resources[] = {
    {"FullscreenDevice",RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&ui_resources.fullscreendevice,
     set_fullscreen_device, NULL },
    {"FullscreenBitdepth",RES_INTEGER, (resource_value_t)8,
     (resource_value_t *)&ui_resources.fullscreenbitdepth,
     set_fullscreen_bitdepth, NULL },
    {"FullscreenWidth",RES_INTEGER, (resource_value_t)640,
     (resource_value_t *)&ui_resources.fullscreenwidth,
     set_fullscreen_width, NULL },
    {"FullscreenHeight",RES_INTEGER, (resource_value_t)480,
     (resource_value_t *)&ui_resources.fullscreenheight,
     set_fullscreen_height, NULL },
    {"FullscreenRefreshRate",RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&ui_resources.fullscreenrefreshrate,
     set_fullscreen_refreshrate, NULL },
    {"FullscreenEnabled",RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&ui_resources.fullscreenenabled,
     set_fullscreen_enabled, NULL },
    {"SaveResourcesOnExit",RES_INTEGER, (resource_value_t)0,
     (resource_value_t *)&ui_resources.save_resources_on_exit,
     set_save_resources_on_exit, NULL },
    {"ConfirmOnExit",RES_INTEGER, (resource_value_t)1,
     (resource_value_t *)&ui_resources.confirm_on_exit,
     set_confirm_on_exit, NULL },
    { NULL }
};

int ui_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------ */

/* UI-related command-line options.  */

static cmdline_option_t cmdline_options[] = {
    { "-saveres", SET_RESOURCE, 0, NULL, NULL,
      "SaveResourcesOnExit", (resource_value_t) 1,
      NULL, "Save settings (resources) on exit" },
    { "+saveres", SET_RESOURCE, 0, NULL, NULL,
      "SaveResourcesOnExit", (resource_value_t) 0,
      NULL, "Never save settings (resources) on exit" },
    { "-confirmexit", SET_RESOURCE, 0, NULL, NULL,
      "ConfirmOnExit", (resource_value_t) 0,
      NULL, "Confirm quiting VICE" },
    { "+confirmexit", SET_RESOURCE, 0, NULL, NULL,
      "ConfirmOnExit", (resource_value_t) 1,
      NULL, "Never confirm quiting VICE" },
    { NULL }
};


int ui_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------ */

#define UI_COMMON_HOTKEYS \
    {FVIRTKEY|FCONTROL|FALT|FNOINVERT,'R',IDM_HARD_RESET},  \
    {FVIRTKEY|FALT|FNOINVERT,'R',IDM_SOFT_RESET},           \
    {FVIRTKEY|FALT|FNOINVERT,'8',IDM_ATTACH_8},             \
    {FVIRTKEY|FALT|FNOINVERT,'9',IDM_ATTACH_9},             \
    {FVIRTKEY|FALT|FNOINVERT,'0',IDM_ATTACH_10},            \
    {FVIRTKEY|FALT|FNOINVERT,'1',IDM_ATTACH_11},            \
    {FVIRTKEY|FALT|FNOINVERT,'T',IDM_ATTACH_TAPE},          \
    {FVIRTKEY|FCONTROL|FALT|FNOINVERT,'L',IDM_LOADQUICK},   \
    {FVIRTKEY|FCONTROL|FALT|FNOINVERT,'S',IDM_SAVEQUICK},   \
    {FVIRTKEY|FALT|FNOINVERT,'L',IDM_SNAPSHOT_LOAD},        \
    {FVIRTKEY|FALT|FNOINVERT,'S',IDM_SNAPSHOT_SAVE},        \
    {FVIRTKEY|FALT|FNOINVERT,'M',IDM_MONITOR},              \
    {FVIRTKEY|FALT|FNOINVERT,'X',IDM_EXIT},                 \
    {FVIRTKEY|FALT|FNOINVERT,'W',IDM_TOGGLE_WARP_MODE},     \
    {FVIRTKEY|FALT|FNOINVERT,'I',IDM_FLIP_ADD},             \
    {FVIRTKEY|FALT|FNOINVERT,'K',IDM_FLIP_REMOVE},          \
    {FVIRTKEY|FALT|FNOINVERT,'N',IDM_FLIP_NEXT},            \
    {FVIRTKEY|FCONTROL|FALT|FNOINVERT,'N',IDM_FLIP_PREVIOUS},\
    {FVIRTKEY|FALT|FNOINVERT,'J',IDM_SWAP_JOYSTICK},\
    {FVIRTKEY|FALT|FNOINVERT,'C',IDM_SCREENSHOT},\
    {FVIRTKEY|FALT|FNOINVERT,'U',IDM_SOUNDSHOT},\
    {FVIRTKEY|FALT|FNOINVERT,'D',IDM_TOGGLE_FULLSCREEN},\
    {FVIRTKEY|FALT|FNOINVERT,0x0d,IDM_TOGGLE_FULLSCREEN}

static ACCEL c64_accel[] = {
    {FVIRTKEY|FALT|FNOINVERT,'Z',IDM_CART_FREEZE},
    UI_COMMON_HOTKEYS
};

static ACCEL c128_accel[] = {
    UI_COMMON_HOTKEYS
};

static ACCEL cbm2_accel[] = {
    UI_COMMON_HOTKEYS
};

static ACCEL vic_accel[] = {
    UI_COMMON_HOTKEYS
};

static ACCEL pet_accel[] = {
    UI_COMMON_HOTKEYS
};

static HBRUSH   led_red;
static HBRUSH   led_green;
static HBRUSH   led_black;
static HBRUSH   tape_motor_on_brush;
static HBRUSH   tape_motor_off_brush;

HWND    main_hwnd;

/* Initialize the UI before setting all the resource values.  */
int ui_init(int *argc, char **argv)
{
WNDCLASS    window_class;
WORD        menu;

    switch (machine_class) {
        case VICE_MACHINE_C64:
            menu = IDR_MENUC64;
            ui_accelerator=CreateAcceleratorTable(c64_accel,24);
            break;
        case VICE_MACHINE_C128:
            menu = IDR_MENUC128;
            ui_accelerator=CreateAcceleratorTable(c128_accel,23);
            break;
        case VICE_MACHINE_VIC20:
            menu = IDR_MENUVIC;
            ui_accelerator=CreateAcceleratorTable(vic_accel,23);
            break;
        case VICE_MACHINE_PET:
            menu = IDR_MENUPET;
            ui_accelerator=CreateAcceleratorTable(pet_accel,23);
            break;
        case VICE_MACHINE_CBM2:
            menu = IDR_MENUCBM2;
            ui_accelerator=CreateAcceleratorTable(cbm2_accel,23);
            break;
        default:
            log_debug("UI: No menu entries for this machine defined!");
            log_debug("UI: Using C64 type UI menues.");
            menu = IDR_MENUC64;
            ui_accelerator=CreateAcceleratorTable(c64_accel,24);
    }

    /* Register the window class.  */
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = window_proc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = winmain_instance;
    window_class.hIcon = LoadIcon(winmain_instance,
                                  MAKEINTRESOURCE(IDI_ICON1));
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0) + 1);
    window_class.lpszMenuName = MAKEINTRESOURCE(menu);
    window_class.lpszClassName = APPLICATION_CLASS;
    RegisterClass(&window_class);

    /* Create the main window.  Notice that we are not going to
       `ShowWindow()' it yet; this will be done as soon as the video module
       requires us to do so.  This is needed both because the video module
       needs an application window to be created to initialize itself, and
       because this might allow us to support more than one emulation window
       in the future.  */
    main_hwnd = CreateWindow(APPLICATION_CLASS,
                             "No title", /* (for now) */
                             WS_OVERLAPPED|WS_CLIPCHILDREN|WS_BORDER|WS_DLGFRAME|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             NULL,
                             NULL,
                             winmain_instance,
                             NULL);

    InitCommonControls();

    number_of_windows=0;

    led_green=CreateSolidBrush(0xff00);
    led_red=CreateSolidBrush(0xff);
    led_black=CreateSolidBrush(0x00);
    tape_motor_on_brush=CreateSolidBrush(0xffff);
    tape_motor_off_brush=CreateSolidBrush(0x808080);

    return 0;
}

/* Initialize the UI after setting all the resource values.  */
int ui_init_finish(void)
{
    ui_fullscreen_init();
    atexit(ui_exit);
    return 0;
}

int ui_init_finalize(void)
{
    return 0;
}

/* Exit.  */
void ui_exit(void)
{
    if (ui_accelerator) {
        DestroyAcceleratorTable(ui_accelerator);
    }
}

/*  Create a Window for the emulation.  */
HWND ui_open_canvas_window(const char *title, unsigned int width,
                           unsigned int height, canvas_redraw_t exp_handler,
                           int fullscreen)
{
HWND    hwnd;
RECT    rect;

    hwnd_titles[number_of_windows] = stralloc(title);
    if (fullscreen) {
        hwnd = CreateWindow(APPLICATION_CLASS,
                            hwnd_titles[number_of_windows],
                            WS_VISIBLE|WS_POPUP,
                            0,
                            0,
                            width,
                            height,
                            NULL,
                            NULL,
                            winmain_instance,
                            NULL);
        SetWindowPos(hwnd,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),SWP_NOCOPYBITS);
    } else {
        hwnd = CreateWindow(APPLICATION_CLASS,
                            hwnd_titles[number_of_windows],
                            WS_OVERLAPPED|WS_CLIPCHILDREN|WS_BORDER|WS_DLGFRAME|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL,
                            NULL,
                            winmain_instance,
                            NULL);
    }
    if (hwnd==NULL) log_debug("Window creation failed");

    if (!fullscreen) {
        status_hwnd[number_of_windows]=CreateStatusWindow(WS_CHILD|WS_VISIBLE,"",hwnd,IDM_STATUS_WINDOW);
        GetClientRect(status_hwnd[number_of_windows],&rect);
        status_height=rect.bottom-rect.top;
    }

    ui_resize_canvas_window(hwnd, width, height);

    ShowWindow(hwnd, winmain_cmd_show);

    window_handles[number_of_windows]=hwnd;
    exposure_handler[number_of_windows] = exp_handler;
    window_canvas_xsize[number_of_windows]=width;
    window_canvas_ysize[number_of_windows]=height;
    number_of_windows++;

    return hwnd;

}

/* Resize `w' so that the client rectangle is of the requested size.  */
void ui_resize_canvas_window(HWND w, unsigned int width, unsigned int height)
{
RECT            wrect;
int             window_index;
WINDOWPLACEMENT place;

    if (IsFullscreenEnabled()) return;

    for (window_index=0; window_index<number_of_windows; window_index++) {
        if (window_handles[window_index]==w) break;
    }
    place.length=sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(w,&place);

    window_canvas_xsize[window_index]=width;
    window_canvas_ysize[window_index]=height;

    GetClientRect(w, &wrect);
    ClientToScreen(w, (LPPOINT) &wrect);
    ClientToScreen(w, ((LPPOINT) &wrect) + 1);
    wrect.right = wrect.left + width;
    wrect.bottom = wrect.top + height + status_height;
    AdjustWindowRect(&wrect, WS_OVERLAPPED|WS_BORDER|WS_DLGFRAME, TRUE);
    if (place.showCmd==SW_SHOWNORMAL) {
        MoveWindow(w,
                   wrect.left,
                   wrect.top,
                   wrect.right - wrect.left,
                   wrect.bottom - wrect.top,
                   TRUE);
    } else {
        place.rcNormalPosition.right=place.rcNormalPosition.left+wrect.right-wrect.left;
        place.rcNormalPosition.bottom=place.rcNormalPosition.top+wrect.bottom-wrect.top;
        SetWindowPlacement(w,&place);
        InvalidateRect(w,NULL,FALSE);
    }
}

/* Update all the menus according to the current settings.  */
void ui_update_menus(void)
{
}

static  ui_menu_toggle      *machine_specific_toggles=NULL;
static  ui_res_value_list   *machine_specific_values=NULL;

void ui_register_menu_toggles(ui_menu_toggle *toggles)
{
    machine_specific_toggles=toggles;
}

void ui_register_res_values(ui_res_value_list *valuelist)
{
    machine_specific_values=valuelist;
}

static void update_menus(HWND hwnd)
{
    HMENU menu = GetMenu(hwnd);
    int i,j;
    int value;
    int result;

    for (i = 0; toggle_list[i].name != NULL; i++) {
        resources_get_value(toggle_list[i].name, (resource_value_t *) &value);
        CheckMenuItem(menu, toggle_list[i].item_id,
                      value ? MF_CHECKED : MF_UNCHECKED);
    }
    
    if (machine_specific_toggles) {
        for (i = 0; machine_specific_toggles[i].name != NULL; i++) {
            resources_get_value(machine_specific_toggles[i].name, (resource_value_t *) &value);
            CheckMenuItem(menu, machine_specific_toggles[i].item_id,
                          value ? MF_CHECKED : MF_UNCHECKED);
        }
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        result=resources_get_value(value_list[i].name,
                                   (resource_value_t *) &value);
        if (result==0) {
            for (j = 0; value_list[i].vals[j].item_id != 0; j++) {
                if (value == value_list[i].vals[j].value) {
                    CheckMenuItem(menu,value_list[i].vals[j].item_id,
                                  MF_CHECKED);
                } else {
                    CheckMenuItem(menu,value_list[i].vals[j].item_id,
                                  MF_UNCHECKED);
                }
            }
        }
    }

    if (machine_specific_values){
        for (i = 0; machine_specific_values[i].name != NULL; i++) {
            result=resources_get_value(machine_specific_values[i].name,
                                       (resource_value_t *) &value);
            if (result==0) {
                for (j = 0; machine_specific_values[i].vals[j].item_id != 0; j++) {
                    if (value == machine_specific_values[i].vals[j].value) {
                        CheckMenuItem(menu,machine_specific_values[i].vals[j].item_id,
                                      MF_CHECKED);
                    } else {
                        CheckMenuItem(menu,machine_specific_values[i].vals[j].item_id,
                                      MF_UNCHECKED);
                    }
                }
            }
        }
    }

}

/* ------------------------------------------------------------------------- */

static ui_machine_specific_t ui_machine_specific = NULL;

void ui_register_machine_specific(ui_machine_specific_t func)
{
    ui_machine_specific = func;
}

/* ------------------------------------------------------------------------- */

/* Report an error to the user (`printf()' style).  */
void ui_error(const char *format, ...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    log_debug(tmp);
    ui_messagebox(window_handles[0], tmp, "VICE Error!", MB_OK | MB_ICONSTOP);
}

/* Report an error to the user (one string).  */
void ui_error_string(const char *text)
{
    log_debug(text);
    ui_messagebox(window_handles[0], text, "VICE Error!", MB_OK | MB_ICONSTOP);
}

/* Report a message to the user (`printf()' style).  */
void ui_message(const char *format,...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    ui_messagebox(window_handles[0], tmp, "VICE Information",
               MB_OK | MB_ICONASTERISK);
}

/* Handle the "CPU JAM" case.  */
ui_jam_action_t ui_jam_dialog(const char *format,...)
{
    char *txt,*txt2;
    int ret;

    va_list ap;
    va_start(ap, format);
    txt = xmvsprintf(format, ap);
    txt2 = xmsprintf("%s\n\nStart monitor?", txt );
    ret = ui_messagebox(window_handles[0], txt2,
                  "VICE CPU JAM", MB_YESNO);
    free(txt2);
    free(txt);
    return (ret==IDYES) ? UI_JAM_MONITOR : UI_JAM_HARD_RESET;
//    UI_JAM_RESET, UI_JAM_HARD_RESET, UI_JAM_MONITOR, UI_JAM_DEBUG
}

/* Handle the "Do you want to extend the disk image to 40-track format"?
   dialog.  */
int ui_extend_image_dialog(void)
{
    int ret;

    ret = ui_messagebox(window_handles[0], "Extend image to 40-track format?",
                     "VICE question", MB_YESNO | MB_ICONQUESTION);
    return ret == IDYES;
}

/* ------------------------------------------------------------------------- */

/* Dispay the current emulation speed.  */
void ui_display_speed(float percent, float framerate, int warp_flag)
{
    char buf[256];
    int     index;

    for (index=0; index<number_of_windows; index++) {
        sprintf(buf, "%s at %d%% speed, %d fps%s",
                hwnd_titles[index], (int)(percent + .5), (int)(framerate + .5),
                warp_flag ? " (warp)" : "");
        SetWindowText(window_handles[index], buf);
    }

}

static ui_drive_enable_t    status_enabled;
static int                  status_led[2];
static int                  status_map[2];          //  Translate from window index -> drive index
static int                  status_partindex[2];    //  Translate from drive index -> window index
static double               status_track[2];
static int                 *drive_active_led;

static int                  tape_enabled = 0;
static int                  tape_motor;
static int                  tape_counter;
static int                  tape_control;

static char                 emu_status_text[1024];

static void SetStatusWindowParts(HWND hwnd)
{
int     number_of_parts;
int     enabled_drives;
RECT    rect;
int     posx[4];
int     width;
int     i;

    number_of_parts=0;
    enabled_drives=0;

    if (tape_enabled)
        number_of_parts++;

    if (status_enabled&UI_DRIVE_ENABLE_0) {
        status_map[enabled_drives++]=0;
        status_partindex[0]=number_of_parts++;
    }
    if (status_enabled&UI_DRIVE_ENABLE_1) {
        status_map[enabled_drives++]=1;
        status_partindex[1]=number_of_parts++;
    }
    GetWindowRect(hwnd,&rect);
    width=rect.right-rect.left;
    for (i=number_of_parts; i>=0; i--) {
        posx[i]=width;
        width-=110;
    }
    SendMessage(hwnd,SB_SETPARTS,number_of_parts+1,(LPARAM)posx);
    if (number_of_parts==3) {
        SendMessage(hwnd,SB_SETTEXT,3|SBT_OWNERDRAW,0);
    }
    if (number_of_parts==2) {
        SendMessage(hwnd,SB_SETTEXT,2|SBT_OWNERDRAW,0);
    }
    if (number_of_parts==1) {
        SendMessage(hwnd,SB_SETTEXT,1|SBT_OWNERDRAW,0);
    }
}

void ui_display_statustext(const char *text)
{
int i;
    strcpy(emu_status_text,text);
    for (i=0; i<number_of_windows; i++) {
        SendMessage(status_hwnd[i],SB_SETTEXT,0|SBT_OWNERDRAW,0);
    }
}


void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
int i;

    status_enabled = enable;
    drive_active_led = drive_led_color;
    for (i=0; i<number_of_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

/* Toggle displaying of the drive track.  */
/* drive_base is either 8 or 0 depending on unit or drive display.
   Dual drives display drive 0: and 1: instead of unit 8: and 9: */
void ui_display_drive_track(int drivenum, int drive_base, double track_number)
{
int i;

    status_track[drivenum]=track_number;
    for (i=0; i<number_of_windows; i++) {
        SendMessage(status_hwnd[i],SB_SETTEXT,(status_partindex[drivenum]+1)|SBT_OWNERDRAW,0);
    }
}

/* Toggle displaying of the drive LED.  */
void ui_display_drive_led(int drivenum, int status)
{
int i;

    status_led[drivenum]=status;
    for (i=0; i<number_of_windows; i++) {
        SendMessage(status_hwnd[i],SB_SETTEXT,(status_partindex[drivenum]+1)|SBT_OWNERDRAW,0);
    }
}

/* display current image */
void ui_display_drive_current_image(unsigned int drivenum, const char *image)
{
    /* just a dummy so far */
}

/* tape-status on*/
void ui_set_tape_status(int tape_status)
{
int i;

    tape_enabled = tape_status;
    for (i=0; i<number_of_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

void ui_display_tape_motor_status(int motor)
{   
int i;

    tape_motor = motor;
    for (i=0; i<number_of_windows; i++) {
        SendMessage(status_hwnd[i],SB_SETTEXT,1|SBT_OWNERDRAW,0);
    }
}

void ui_display_tape_control_status(int control)
{
int i;

    tape_control = control;
    for (i=0; i<number_of_windows; i++) {
        SendMessage(status_hwnd[i],SB_SETTEXT,1|SBT_OWNERDRAW,0);
    }
}

extern void ui_display_tape_counter(int counter)
{
int i;

    if (counter!=tape_counter) {
        tape_counter = counter;
        for (i=0; i<number_of_windows; i++) {
            SendMessage(status_hwnd[i],SB_SETTEXT,1|SBT_OWNERDRAW,0);
        }
    }
}

/* display the attched tape image */
void ui_display_tape_current_image(const char *image)
{
}

/* Toggle displaying of paused state.  */
void ui_display_paused(int flag)
{
}

ui_button_t ui_ask_confirmation(const char *title, const char *text)
{
    return UI_BUTTON_NONE;
}

static void mon_trap(ADDRESS addr, void *unused_data)
{
    mon(addr);
}

static void save_snapshot_trap(ADDRESS unused_addr, void *hwnd)
{
    SuspendFullscreenMode(hwnd);
    ui_snapshot_save_dialog(hwnd);
    ResumeFullscreenMode(hwnd);
}

static void load_snapshot_trap(ADDRESS unused_addr, void *hwnd)
{
    SuspendFullscreenMode(hwnd);
    ui_snapshot_load_dialog(hwnd);
    ResumeFullscreenMode(hwnd);
}

typedef struct {
    char    name[MAX_PATH];
    int     valid;
} snapfiles;

static snapfiles    files[10];
static int          lastindex;
static int          snapcounter;

static void save_quicksnapshot_trap(ADDRESS unused_addr, void *unused_data)
{
int     i,j;
char    *fullname;
char    *fullname2;

    if (lastindex==-1) {
        lastindex=0;
        strcpy(files[lastindex].name,"quicksnap0.vsf");
    } else {
        if (lastindex==9) {
            if (snapcounter==10) {
                fullname=concat(archdep_boot_path(),"\\",machine_name,"\\",files[0].name,NULL);
                DeleteFile(fullname);
                free(fullname);
                for (i=1; i<10; i++) {
                    fullname=concat(archdep_boot_path(),"\\",machine_name,"\\",files[i].name,NULL);
                    fullname2=concat(archdep_boot_path(),"\\",machine_name,"\\",files[i-1].name,NULL);
                    MoveFile(fullname,fullname2);
                    free(fullname);
                    free(fullname2);
                }
            } else {
                for (i=0; i<10; i++) {
                    if (files[i].valid==0) break;
                }
                for (j=i+1; j<10; j++) {
                    if (files[j].valid) {
                        strcpy(files[i].name,files[j].name);
                        files[i].name[strlen(files[i].name)-5]='0'+i;
                        fullname=concat(archdep_boot_path(),"\\",machine_name,"\\",files[j].name,NULL);
                        fullname2=concat(archdep_boot_path(),"\\",machine_name,"\\",files[i].name,NULL);
                        MoveFile(fullname,fullname2);
                        free(fullname);
                        free(fullname2);
                        i++;
                    }
                }
                strcpy(files[i].name,files[0].name);
                files[i].name[strlen(files[i].name)-5]='0'+i;
                lastindex=i;
            }
        } else {
            strcpy(files[lastindex+1].name,files[lastindex].name);
            lastindex++;
            files[lastindex].name[strlen(files[lastindex].name)-5]='0'+lastindex;
        }
    }

    fullname=concat(archdep_boot_path(),"\\",machine_name,"\\",files[lastindex].name,NULL);
    if (machine_write_snapshot(fullname,0,0)<0) {
        ui_error("Can't write snapshot file.");
    }
    free(fullname);
}

static void load_quicksnapshot_trap(ADDRESS unused_addr, void *unused_data)
{
    char *fullname;

    fullname=concat(archdep_boot_path(),"\\",machine_name,"\\",files[lastindex].name,NULL);
    if (machine_read_snapshot(fullname)<0) {
        ui_error("Cannot read snapshot image");
    }
    free(fullname);
}

/* ------------------------------------------------------------------------ */

/* Return the main window handler.  */
HWND ui_get_main_hwnd(void)
{
    if (window_handles[0]==NULL) return main_hwnd;
    return window_handles[0];
}

/* Dispatch the next pending event, if any.  Otherwise, just return.  */
void ui_dispatch_next_event(void)
{
    MSG msg;

    if (!GetMessage(&msg, NULL, 0, 0))
        exit(msg.wParam);
    if (ui_accelerator) {
        if (!TranslateAccelerator(msg.hwnd,ui_accelerator,&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/* Dispatch all the current pending events; return as soon as no more events
   are pending.  */
void ui_dispatch_events(void)
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        ui_dispatch_next_event();
}

/* ------------------------------------------------------------------------ */

int CALLBACK about_dialog_proc(HWND dialog, UINT msg,
                               UINT wparam, LONG lparam)
{
    char version[256];

    switch (msg) {
        case WM_INITDIALOG:
            sprintf(version, "Version %s", VERSION);
            SetDlgItemText(dialog, IDC_ABOUT_VERSION, version);
            return TRUE;
        case WM_CLOSE:
            EndDialog(dialog,0);
            return TRUE;
        case WM_COMMAND:
            if ((wparam == IDOK) || (wparam == IDCANCEL)) {
                EndDialog(dialog, 0);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static void scan_files(void)
{
WIN32_FIND_DATA     file_info;
HANDLE              search_handle;
int                 i;
char                *dirname;

    dirname=concat(archdep_boot_path(),"\\",machine_name,"\\quicksnap?.vsf",NULL);
    search_handle=FindFirstFile(dirname,&file_info);
    snapcounter=0;
    lastindex=-1;
    for (i=0; i<10; i++) {
        files[i].valid=0;
    }
    if (search_handle!=INVALID_HANDLE_VALUE) {
        do {
            char c;
            c=file_info.cFileName[strlen(file_info.cFileName)-5];
            if ((c>='0') && (c<='9')) {
                strcpy(files[c-'0'].name,file_info.cFileName);
                files[c-'0'].valid=1;
                if ((c-'0')>lastindex) {
                    lastindex=c-'0';
                }
                snapcounter++;
            }
        } while (FindNextFile(search_handle,&file_info));
        FindClose(search_handle);
    }
    free(dirname);
}

/* ------------------------------------------------------------------------ */

/* FIXME: tmp hack.  */
int syscolorchanged, displaychanged, querynewpalette, palettechanged;

static void handle_wm_command(WPARAM wparam, LPARAM lparam, HWND hwnd)
{
char *fname;
char *dname;

    /* Handle machine specific commands first.  */
    if (ui_machine_specific)
        ui_machine_specific(wparam, hwnd);

    switch (wparam) {
      case IDM_DEVICEMANAGER:
        ui_attach_dialog(hwnd);
        break;
      case IDM_EXIT|0x00010000:
      case IDM_EXIT:
        PostMessage(hwnd, WM_CLOSE, wparam, lparam);
        break;
      case IDM_ABOUT:
        DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_ABOUT), hwnd,
                  (DLGPROC) about_dialog_proc);
        break;
      case IDM_HELP:
        fname=concat(archdep_boot_path(),"\\DOC\\vice_toc.html",NULL);
        dname=concat(archdep_boot_path(),"\\DOC",NULL);
        ShellExecute(hwnd,"open",fname,NULL,dname,0);
        free(fname);
        free(dname);
        break;
      case IDM_CONTRIBUTORS:
        ui_show_text(hwnd, "VICE contributors", "Who made what?", contrib_text);
        break;
      case IDM_LICENSE:
        ui_show_text(hwnd, "License",
                     "VICE license (GNU General Public License)", license_text);
        break;
      case IDM_WARRANTY:
        ui_show_text(hwnd, "No warranty!",
                     "VICE is distributed WITHOUT ANY WARRANTY!",
                     warranty_text);
        break;
      case IDM_CMDLINE:
        ui_cmdline_show_options(hwnd);
        break;
      case IDM_ATTACH_8|0x00010000:
      case IDM_ATTACH_9|0x00010000:
      case IDM_ATTACH_10|0x00010000:
      case IDM_ATTACH_11|0x00010000:
      case IDM_ATTACH_8:
      case IDM_ATTACH_9:
      case IDM_ATTACH_10:
      case IDM_ATTACH_11:
        {
            char    *s;
            int     unit = 8;
            char    *autostart_filename=NULL;

            SuspendFullscreenMode(hwnd);
            switch (wparam&0xffff) {
              case IDM_ATTACH_8:
                unit = 8;
                break;
              case IDM_ATTACH_9:
                unit = 9;
                break;
              case IDM_ATTACH_10:
                unit = 10;
                break;
              case IDM_ATTACH_11:
                unit = 11;
                break;
            }
            if ((s = ui_select_file(hwnd,"Attach disk image",
                "Disk image files (*.d64;*.d71;*.d81;*.g64;*.g41;*.x64;*.d80;*.d82)\0*.d64;*.d71;*.d81;*.g64;*.g41;*.x64;*.d80;*.d82\0"
                "Zipped files (*.zip;*.gz;*.bz2;*.d6z;*.d7z;*.d8z;*.g6z;*.g4z;*.x6z)\0*.zip;*.bz2;*.gz;*.d6z;*.d7z;*.d8z;*.g6z;*.g4z;*.x6z\0"
                "All files (*.*)\0*.*\0", FILE_SELECTOR_DISK_STYLE,
                &autostart_filename)) != NULL) {
                if (autostart_filename!=NULL) {
/* FIXME: Set 2nd arg to NULL, use arg 3rd for program number */
                    if (autostart_autodetect(s, autostart_filename, 0) < 0)
                        ui_error("Cannot autostart specified file.");
                        free(autostart_filename);
                } else {
                    if (file_system_attach_disk(unit, s) < 0)
                        ui_error("Cannot attach specified file");
                }
                free(s);
            }
            ResumeFullscreenMode(hwnd);
        }
        break;
      case IDM_DETACH_8:
        file_system_detach_disk(8);
        break;
      case IDM_DETACH_9:
        file_system_detach_disk(9);
        break;
      case IDM_DETACH_10:
        file_system_detach_disk(10);
        break;
      case IDM_DETACH_11:
        file_system_detach_disk(11);
        break;
      case IDM_DETACH_ALL:
        file_system_detach_disk(8);
        file_system_detach_disk(9);
        file_system_detach_disk(10);
        file_system_detach_disk(11);
        break;
      case IDM_FLIP_ADD|0x00010000:
      case IDM_FLIP_ADD:
        flip_add_image(8);
        break;
      case IDM_FLIP_REMOVE|0x00010000:
      case IDM_FLIP_REMOVE:
        flip_remove(8, NULL);
        break;
      case IDM_FLIP_NEXT|0x00010000:
      case IDM_FLIP_NEXT:
        flip_attach_head(8, 1);
        break;
      case IDM_FLIP_PREVIOUS|0x00010000:
      case IDM_FLIP_PREVIOUS:
        flip_attach_head(8, 0);
        break;
      case IDM_ATTACH_TAPE|0x00010000:
      case IDM_ATTACH_TAPE:
        {
            char    *s;
            char    *autostart_filename=NULL;

            SuspendFullscreenMode(hwnd);
            if ((s = ui_select_file(hwnd,"Attach tape image",
                "Tape image files (*.t64;*.p00;*.tap;*.prg)\0*.t64;*.p00;*.tap;*.prg\0"
                "Zipped files (*.zip;*.gz;*.bz2;*.t6z;*.p0z;*.taz;*.prz)\0*.zip;*.bz2;*.gz;*.t6z;*.p0z;*.taz;*.prz\0"
                "All files (*.*)\0*.*\0", FILE_SELECTOR_TAPE_STYLE,
                &autostart_filename)) != NULL) {
                if (autostart_filename!=NULL) {
/* FIXME: Set 2nd arg to NULL, use arg 3rd for program number */
                    if (autostart_autodetect(s, autostart_filename, 0) < 0)
                        ui_error("Cannot autostart specified file.");
                        free(autostart_filename);
                } else {
                    if (tape_attach_image(s) < 0)
                        ui_error("Cannot attach specified file");
                }
                free(s);
            }
            ResumeFullscreenMode(hwnd);
        }
        break;
      case IDM_DETACH_TAPE:
        tape_detach_image();
        break;
      case IDM_DATASETTE_CONTROL_STOP:
        datasette_control(DATASETTE_CONTROL_STOP);
        break;
      case IDM_DATASETTE_CONTROL_START:
        datasette_control(DATASETTE_CONTROL_START);
        break;
      case IDM_DATASETTE_CONTROL_FORWARD:
        datasette_control(DATASETTE_CONTROL_FORWARD);
        break;
      case IDM_DATASETTE_CONTROL_REWIND:
        datasette_control(DATASETTE_CONTROL_REWIND);
        break;
      case IDM_DATASETTE_CONTROL_RECORD:
        datasette_control(DATASETTE_CONTROL_RECORD);
        break;
      case IDM_DATASETTE_CONTROL_RESET:
        datasette_control(DATASETTE_CONTROL_RESET);
        break;
      case IDM_DATASETTE_RESET_COUNTER:
        datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
        break;
      case IDM_AUTOSTART:
        {
            char    *s;
            char    *autostart_filename=NULL;

            if ((s = ui_select_file(hwnd,"Autostart disk/tape image",
                "Disk image files (*.d64;*.d71;*.d81;*.g64;*.g41;*.x64;*.d80;*.d82)\0*.d64;*.d71;*.d81;*.g64;*.g41;*.x64;*.d80;*.d82\0"
                "Tape image files (*.t64;*.p00;*.tap;*.prg)\0*.t64;*.p00;*.tap;*.prg\0"
                "Zipped files (*.zip;*.gz;*.bz2;*.d6z;*.d7z;*.d8z;*.g6z;*.g4z;*.x6z;*.t6z;*.p0z;*.taz;*.prz)\0*.zip;*.gz;*.bz2;*.d6z;*.d7z;*.d8z;*.g6z;*.g4z;*.x6z;*.t6z;*.p0z;*.taz;*.prz\0"
                "All files (*.*)\0*.*\0",
                FILE_SELECTOR_DISK_AND_TAPE_STYLE,
                &autostart_filename)) != NULL) {
/* FIXME: Set 2nd arg to NULL, use arg 3rd for program number */
                if (autostart_autodetect(s, autostart_filename, 0) < 0)
                    ui_error("Cannot autostart specified file.");
                if (autostart_filename!=NULL) free(autostart_filename);
                free(s);
            }
        }
        break;
      case IDM_SNAPSHOT_LOAD|0x00010000:
      case IDM_SNAPSHOT_LOAD:
        if (1 /* !ui_emulation_is_paused()*/ )
            maincpu_trigger_trap(load_snapshot_trap, hwnd);
        else
            load_snapshot_trap(0, 0);
        /* ui_snapshot_load_dialog(main_hwnd);*/
        break;
      case IDM_SNAPSHOT_SAVE|0x00010000:
      case IDM_SNAPSHOT_SAVE:
        maincpu_trigger_trap(save_snapshot_trap, hwnd);
        break;
      case IDM_SAVEQUICK|0x00010000:
      case IDM_SAVEQUICK:
        scan_files();
        maincpu_trigger_trap(save_quicksnapshot_trap, (void*) 0);
        break;
      case IDM_LOADQUICK|0x00010000:
      case IDM_LOADQUICK:
        scan_files();
        if (snapcounter>0) {
            maincpu_trigger_trap(load_quicksnapshot_trap, (void *) 0);
        }
        break;
      case IDM_SCREENSHOT|0x00010000:
      case IDM_SCREENSHOT:
          SuspendFullscreenMode(hwnd);
        ui_screenshot_save_dialog(hwnd);
        ResumeFullscreenMode(hwnd);
        break;
      case IDM_SOUNDSHOT|0x00010000:
      case IDM_SOUNDSHOT:
          SuspendFullscreenMode(hwnd);
        ui_soundshot_save_dialog(hwnd);
        ResumeFullscreenMode(hwnd);
        break;
      case IDM_MONITOR|0x00010000:
      case IDM_MONITOR:
        if (1 /* !ui_emulation_is_paused()*/ )
            maincpu_trigger_trap(mon_trap, (void *) 0);
        else
            mon_trap(MOS6510_REGS_GET_PC(&maincpu_regs), 0);
        break;
      case IDM_HARD_RESET+0x00010000:
      case IDM_SOFT_RESET+0x00010000:
      case IDM_HARD_RESET:
      case IDM_SOFT_RESET:
        keyboard_clear_keymatrix();
// @@@        SuspendFullscreenMode(hwnd);
        if (ui_messagebox(hwnd,
                       "Do you really want to reset the emulated machine?",
                       ((wparam&0xffff) == IDM_HARD_RESET ? "Hard reset"
                        : "Soft reset"),
                       MB_YESNO | MB_ICONQUESTION) == IDYES) {
            if ((wparam&0xffff) == IDM_HARD_RESET) {
                machine_powerup();
            } else {
                maincpu_trigger_reset();
            }
        }
// @@@        ResumeFullscreenMode(hwnd);
        break;
      case IDM_REFRESH_RATE_AUTO:
        resources_set_value("RefreshRate", (resource_value_t) 0);
        break;
      case IDM_REFRESH_RATE_1:
        resources_set_value("RefreshRate", (resource_value_t) 1);
        break;
      case IDM_REFRESH_RATE_2:
        resources_set_value("RefreshRate", (resource_value_t) 2);
        break;
      case IDM_REFRESH_RATE_3:
        resources_set_value("RefreshRate", (resource_value_t) 3);
        break;
      case IDM_REFRESH_RATE_4:
        resources_set_value("RefreshRate", (resource_value_t) 4);
        break;
      case IDM_REFRESH_RATE_5:
        resources_set_value("RefreshRate", (resource_value_t) 5);
        break;
      case IDM_REFRESH_RATE_6:
        resources_set_value("RefreshRate", (resource_value_t) 6);
        break;
      case IDM_REFRESH_RATE_7:
        resources_set_value("RefreshRate", (resource_value_t) 7);
        break;
      case IDM_REFRESH_RATE_8:
        resources_set_value("RefreshRate", (resource_value_t) 8);
        break;
      case IDM_REFRESH_RATE_9:
        resources_set_value("RefreshRate", (resource_value_t) 9);
        break;
      case IDM_REFRESH_RATE_10:
        resources_set_value("RefreshRate", (resource_value_t) 10);
        break;
      case IDM_MAXIMUM_SPEED_200:
        resources_set_value("Speed", (resource_value_t) 200);
        break;
      case IDM_MAXIMUM_SPEED_100:
        resources_set_value("Speed", (resource_value_t) 100);
        break;
      case IDM_MAXIMUM_SPEED_50:
        resources_set_value("Speed", (resource_value_t) 50);
        break;
      case IDM_MAXIMUM_SPEED_20:
        resources_set_value("Speed", (resource_value_t) 20);
        break;
      case IDM_MAXIMUM_SPEED_10:
        resources_set_value("Speed", (resource_value_t) 10);
        break;
      case IDM_MAXIMUM_SPEED_NO_LIMIT:
        resources_set_value("Speed", (resource_value_t) 0);
        break;
      case IDM_SIDTYPE_6581:
        resources_set_value("SidModel", (resource_value_t) 0);
        break;
      case IDM_SIDTYPE_8580:
        resources_set_value("SidModel", (resource_value_t) 1);
        break;
      case IDM_DRIVE_SETTINGS:
        ui_drive_settings_dialog(hwnd);
        break;
      case IDM_DATASETTE_SETTINGS:
        ui_datasette_settings_dialog(hwnd);
        break;
      case IDM_JOY_SETTINGS:
        ui_joystick_settings_dialog(hwnd);
        break;
      case IDM_SWAP_JOYSTICK|0x00010000:
      case IDM_SWAP_JOYSTICK:
        ui_joystick_swap_joystick();
        break;
      case IDM_SOUND_SETTINGS:
        ui_sound_settings_dialog(hwnd);
        break;
      case IDM_FULLSCREEN_SETTINGS:
        ui_fullscreen_settings_dialog(hwnd);
        break;
      case IDM_TOGGLE_FULLSCREEN|0x00010000:
      case IDM_TOGGLE_FULLSCREEN:
        SwitchFullscreenMode(hwnd);
        break;
      case IDM_SYNC_FACTOR_PAL:
        resources_set_value("VideoStandard",
                            (resource_value_t) DRIVE_SYNC_PAL);
        break;
      case IDM_SYNC_FACTOR_NTSC:
        resources_set_value("VideoStandard",
                            (resource_value_t) DRIVE_SYNC_NTSC);
        break;
      case IDM_SYNC_FACTOR_NTSCOLD:
        resources_set_value("VideoStandard",
                            (resource_value_t) DRIVE_SYNC_NTSCOLD);
        break;
      case IDM_SETTINGS_SAVE:
        if (resources_save(NULL) < 0)
            ui_error("Cannot save settings.");
        else
            ui_message("Settings saved successfully.");
        break;
      case IDM_SETTINGS_LOAD:
        if (resources_load(NULL) < 0) {
            ui_error("Cannot load settings.");
        } else {
            ui_message("Settings loaded successfully.");
        }
        break;
      case IDM_SETTINGS_DEFAULT:
        resources_set_defaults();
        ui_message("Default settings restored.");
        break;
      default:
        {
            int i;

            for (i = 0; toggle_list[i].name != NULL; i++) {
                if (toggle_list[i].item_id == wparam) {
                    resources_toggle(toggle_list[i].name, NULL);
                    break;
                }
            }

            if (machine_specific_toggles) {
                for (i = 0; machine_specific_toggles[i].name != NULL; i++) {
                    if (machine_specific_toggles[i].item_id == wparam) {
                        resources_toggle(machine_specific_toggles[i].name, NULL);
                        break;
                    }
                }
            }
            break;
        }
    }
}

static void clear(HDC hdc, int x1, int y1, int x2, int y2)
{
static HBRUSH   back_color;
RECT            clear_rect;

    if (back_color==NULL) back_color=CreateSolidBrush(0);
    clear_rect.left=x1;
    clear_rect.top=y1;
    clear_rect.right=x2;
    clear_rect.bottom=y2;
    FillRect(hdc,&clear_rect,back_color);
}


int     ui_active=FALSE;
HWND    ui_active_window;

/* Window procedure.  All messages are handled here.  */
static long CALLBACK window_proc(HWND window, UINT msg,
                                 WPARAM wparam, LPARAM lparam)
{
RECT    led;
char    text[256];
RECT    client_rect;
int     window_index;

    for (window_index=0; window_index<number_of_windows; window_index++) {
        if (window_handles[window_index]==window) break;
    }

    switch (msg) {
        case WM_ACTIVATEAPP:
            if (wparam==WA_INACTIVE) {
//                log_debug("WM_ACTIVATEAPP inactive %d %08x",window_index,window);
            } else {
//                log_debug("WM_ACTIVATEAPP active %d %08x",window_index,window);
            }
            break;
        case WM_ACTIVATE:
            if (wparam==WA_INACTIVE) {
//                log_debug("WM_ACTIVATE inactive %d %08x",window_index,window);
                ui_active=FALSE;
                SuspendFullscreenMode(window);
            } else {
//                log_debug("WM_ACTIVATE active %d %08x",window_index,window);
                ui_active=TRUE;
                ui_active_window=window;
                ResumeFullscreenMode(window);
            }
            mouse_update_mouse_acquire();
            break;
        case WM_SIZE:
            SendMessage(status_hwnd[window_index],msg,wparam,lparam);
            SetStatusWindowParts(status_hwnd[window_index]);
            GetClientRect(window, &client_rect);
            return 0;
        case WM_DRAWITEM:
            if (wparam==IDM_STATUS_WINDOW) {
                if (((DRAWITEMSTRUCT*)lparam)->itemID==0) {
                    /* it's the status info */
                    led=((DRAWITEMSTRUCT*)lparam)->rcItem;
                    SetBkColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENU));
                    SetTextColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENUTEXT));
                    DrawText(((DRAWITEMSTRUCT*)lparam)->hDC,emu_status_text,-1,&led,0);
               }
                if ((((DRAWITEMSTRUCT*)lparam)->itemID==1) && tape_enabled) {
                    /* it's the tape status */
                    POINT tape_control_sign[3];

                    /* the leading "Tape:" */
                    led.top=((DRAWITEMSTRUCT*)lparam)->rcItem.top+2;
                    led.bottom=((DRAWITEMSTRUCT*)lparam)->rcItem.top+18;
                    led.left=((DRAWITEMSTRUCT*)lparam)->rcItem.left+2;
                    led.right=((DRAWITEMSTRUCT*)lparam)->rcItem.left+34;
                    SetBkColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENU));
                    SetTextColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENUTEXT));
                    DrawText(((DRAWITEMSTRUCT*)lparam)->hDC,"Tape:",-1,&led,0);

                    /* the tape-motor */
                    led.top=((DRAWITEMSTRUCT*)lparam)->rcItem.top+1;
                    led.bottom=((DRAWITEMSTRUCT*)lparam)->rcItem.top+15;
                    led.left=((DRAWITEMSTRUCT*)lparam)->rcItem.left+36;
                    led.right=((DRAWITEMSTRUCT*)lparam)->rcItem.left+50;
                    FillRect(((DRAWITEMSTRUCT*)lparam)->hDC,&led,tape_motor?tape_motor_on_brush:tape_motor_off_brush);

                    /* the tape-control */
                    led.top+=3;
                    led.bottom-=3;
                    led.left+=3;
                    led.right-=3;
                    tape_control_sign[0].x = led.left;
                    tape_control_sign[1].x = led.left+4;
                    tape_control_sign[2].x = led.left;
                    tape_control_sign[0].y = led.top;
                    tape_control_sign[1].y = led.top+4;
                    tape_control_sign[2].y = led.top+8;
                    switch (tape_control) {
                        case DATASETTE_CONTROL_STOP:
                            FillRect(((DRAWITEMSTRUCT*)lparam)->hDC,&led,led_black);
                            break;
                        case DATASETTE_CONTROL_START:
                        case DATASETTE_CONTROL_RECORD:
                            SelectObject(((DRAWITEMSTRUCT*)lparam)->hDC,led_black);
                            Polygon(((DRAWITEMSTRUCT*)lparam)->hDC,tape_control_sign,3);
                            if (tape_control==DATASETTE_CONTROL_RECORD) {
                                SelectObject(((DRAWITEMSTRUCT*)lparam)->hDC,led_red);
                                Ellipse(((DRAWITEMSTRUCT*)lparam)->hDC,
                                    led.left+17,
                                    led.top+1,
                                    led.left+24,
                                    led.top+8);
                            }
                            break;
                        case DATASETTE_CONTROL_REWIND:
                            tape_control_sign[0].x += 4;
                            tape_control_sign[1].x -= 4;
                            tape_control_sign[2].x += 4;
                        case DATASETTE_CONTROL_FORWARD:
                            Polyline(((DRAWITEMSTRUCT*)lparam)->hDC,tape_control_sign,3);
                            tape_control_sign[0].x += 4;
                            tape_control_sign[1].x += 4;
                            tape_control_sign[2].x += 4;
                            Polyline(((DRAWITEMSTRUCT*)lparam)->hDC,tape_control_sign,3);
                    }

                    /* the tape-counter */
                    led.top=((DRAWITEMSTRUCT*)lparam)->rcItem.top+2;
                    led.bottom=((DRAWITEMSTRUCT*)lparam)->rcItem.top+18;
                    led.left=((DRAWITEMSTRUCT*)lparam)->rcItem.left+75;
                    led.right=((DRAWITEMSTRUCT*)lparam)->rcItem.left+110;
                    sprintf(text,"%03i",tape_counter);
                    DrawText(((DRAWITEMSTRUCT*)lparam)->hDC,text,-1,&led,0);

                }
                if (((DRAWITEMSTRUCT*)lparam)->itemID>tape_enabled?1:0) {
                    int index=((DRAWITEMSTRUCT*)lparam)->itemID-(tape_enabled?2:1);
                    /* it's a disk */
                    led.top=((DRAWITEMSTRUCT*)lparam)->rcItem.top+2;
                    led.bottom=((DRAWITEMSTRUCT*)lparam)->rcItem.top+18;
                    led.left=((DRAWITEMSTRUCT*)lparam)->rcItem.left+2;
                    led.right=((DRAWITEMSTRUCT*)lparam)->rcItem.left+84;
                    sprintf(text,"%d: Track: %.1f",status_map[index]+8,status_track[status_map[index]]);
                    SetBkColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENU));
                    SetTextColor(((DRAWITEMSTRUCT*)lparam)->hDC,(COLORREF)GetSysColor(COLOR_MENUTEXT));
                    DrawText(((DRAWITEMSTRUCT*)lparam)->hDC,text,-1,&led,0);

                    led.top=((DRAWITEMSTRUCT*)lparam)->rcItem.top+2;
                    led.bottom=((DRAWITEMSTRUCT*)lparam)->rcItem.top+2+12;
                    led.left=((DRAWITEMSTRUCT*)lparam)->rcItem.left+86;
                    led.right=((DRAWITEMSTRUCT*)lparam)->rcItem.left+86+16;
                    FillRect(((DRAWITEMSTRUCT*)lparam)->hDC,&led,status_led[status_map[index]] ? (drive_active_led[status_map[index]] ? led_green : led_red ) : led_black);
                }
            }
            return 0;
        case WM_COMMAND:
            handle_wm_command(wparam, lparam, window);
            return 0;
        case WM_ENTERMENULOOP:
            update_menus(window);
        case WM_ENTERSIZEMOVE:
            suspend_speed_eval();
            ui_active=FALSE;
            mouse_update_mouse_acquire();
            break;
        case WM_EXITMENULOOP:
        case WM_EXITSIZEMOVE:
            if (GetActiveWindow()==window || !IsIconic(window)) {
                ui_active=TRUE;
            } else {
                ui_active=FALSE;
            }
            mouse_update_mouse_acquire();
            break;
        case WM_SYSKEYDOWN:
            if (wparam==VK_F10) {
                kbd_handle_keydown(wparam, lparam);
                return 0;
            }
            break;
        case WM_KEYDOWN:
            kbd_handle_keydown(wparam, lparam);
            return 0;
        case WM_SYSKEYUP:
            if (wparam==VK_F10) {
                kbd_handle_keyup(wparam, lparam);
                return 0;
            }
            break;
        case WM_KEYUP:
            kbd_handle_keyup(wparam, lparam);
            return 0;
        case WM_SYSCOLORCHANGE:
            syscolorchanged = 1;
            break;
        case WM_DISPLAYCHANGE:
            displaychanged = 1;
            break;
        case WM_QUERYNEWPALETTE:
            querynewpalette = 1;
            break;
        case WM_PALETTECHANGED:
            if ((HWND) wparam != window)
                palettechanged = 1;
            break;
        case WM_CLOSE:
            {
            int quit = 1;

            SuspendFullscreenMode(window);
            if (ui_resources.confirm_on_exit)
            {
                if (MessageBox(window,
                       "Do you really want to exit?\n\n"
                       "All the data present in the emulated RAM will be lost.",
                       "VICE",
                       MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TASKMODAL)
                       == IDYES)
                {
                    quit = 1;
                }
                else
                {
                    quit = 0;
                }
            }

            if (quit)
            {
               if (ui_resources.save_resources_on_exit)
               {
                   if (resources_save(NULL)<0) {
                       ui_error("Cannot save settings.");
                   }
               }
               DestroyWindow(window);
            } else {
                ResumeFullscreenMode(window);
            }
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
        {
            RECT update_rect;

            if (GetUpdateRect(window, &update_rect, FALSE)) {
                PAINTSTRUCT ps;
                HDC hdc;
                int frame_coord[6];

                hdc = BeginPaint(window, &ps);

                frame_coord[0]=update_rect.left;
                frame_coord[1]=update_rect.top;
                frame_coord[2]=update_rect.right;
                frame_coord[3]=update_rect.bottom;

                canvas_update(window, hdc,update_rect.left,update_rect.top,update_rect.right-update_rect.left, update_rect.bottom-update_rect.top);

                EndPaint(window, &ps);
                return 0;
            } else
                break;
        }
    }

    return DefWindowProc(window, msg, wparam, lparam);
}

