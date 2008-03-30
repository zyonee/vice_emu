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
#include <tchar.h>
#ifdef HAVE_COMMCTRL_H
#include <commctrl.h>
#endif

#include "attach.h"
#include "autostart.h"
#include "archdep.h"
#include "datasette.h"
#include "debug.h"
#include "drive.h"
#include "drivecpu.h"
#include "event.h"
#include "fliplist.h"
#include "fullscrn.h"
#include "imagecontents.h"
#include "info.h"
#include "interrupt.h"
#include "ioutil.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "monitor.h"
#include "mousedrv.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "tape.h"
#include "types.h"
#include "ui.h"
#include "uiperipheral.h"
#include "uicmdline.h"
#include "uidatasette.h"
#include "uifliplist.h"
#include "uijoystick.h"
#include "uilib.h"
#include "uimediafile.h"
#include "uiram.h"
#include "uisnapshot.h"
#include "uisound.h"
#include "uispeed.h"
#include "util.h"
#include "version.h"
#include "videoarch.h"
#include "vsync.h"
#include "winmain.h"
#include "statusbar.h"
#include "printer.h"



static TCHAR *hwnd_titles[2];

/* Exposure handler.  */
HWND window_handles[2];
int number_of_windows;
int window_canvas_xsize[2];
int window_canvas_ysize[2];

static HACCEL ui_accelerator;

/* Forward prototypes.  */
static long CALLBACK dummywindowproc(HWND window, UINT msg,
                                     WPARAM wparam, LPARAM lparam);
static long CALLBACK window_proc(HWND window, UINT msg,
                                 WPARAM wparam, LPARAM lparam);
static int ui_emulation_is_paused(void);


/* List of resources that can be grayed out from the menus.  */
static const ui_menu_toggle_t grayed_list[] = {
#ifdef HAVE_TFE
/*    { "ETHERNET_DISABLED", IDM_TFE_SETTINGS }, */
#endif /* #ifdef HAVE_TFE */
    { NULL, 0 }
};

/* List of resources that can be switched on and off from the menus.  */
static const ui_menu_toggle_t toggle_list[] = {
    { "Sound", IDM_TOGGLE_SOUND },
    { "DriveTrueEmulation", IDM_TOGGLE_DRIVE_TRUE_EMULATION },
    { "EmuID", IDM_TOGGLE_EMUID },
    { "WarpMode", IDM_TOGGLE_WARP_MODE },
    { "WarpMode", IDM_TOGGLE_WARP_MODE|0x00010000 },
    { "VirtualDevices", IDM_TOGGLE_VIRTUAL_DEVICES },
    { "SaveResourcesOnExit", IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", IDM_TOGGLE_CONFIRM_ON_EXIT },
    { "FullScreenEnabled", IDM_TOGGLE_FULLSCREEN },
#ifdef DEBUG
    { "MainCPU_TRACE", IDM_TOGGLE_MAINCPU_TRACE },
    { "MainCPU_TRACE", IDM_TOGGLE_MAINCPU_TRACE|0x00010000 },
    { "Drive0CPU_TRACE", IDM_TOGGLE_DRIVE0CPU_TRACE },
    { "Drive0CPU_TRACE", IDM_TOGGLE_DRIVE0CPU_TRACE|0x00010000 },
    { "Drive1CPU_TRACE", IDM_TOGGLE_DRIVE1CPU_TRACE },
    { "Drive1CPU_TRACE", IDM_TOGGLE_DRIVE1CPU_TRACE|0x00010000 },
#endif
    { NULL, 0 }
};

/*  List of resources which can have multiple mutual exclusive menu entries. */

static const ui_res_possible_values_t RefreshRateValues[] = {
    { 0, IDM_REFRESH_RATE_AUTO },
    { 1, IDM_REFRESH_RATE_1 },
    { 2, IDM_REFRESH_RATE_2 },
    { 3, IDM_REFRESH_RATE_3 },
    { 4, IDM_REFRESH_RATE_4 },
    { 5, IDM_REFRESH_RATE_5 },
    { 6, IDM_REFRESH_RATE_6 },
    { 7, IDM_REFRESH_RATE_7 },
    { 8, IDM_REFRESH_RATE_8 },
    { 9, IDM_REFRESH_RATE_9 },
    { 10, IDM_REFRESH_RATE_10 },
    { -1, 0 }
};

static ui_res_possible_values_t SpeedValues[] = {
    { 0, IDM_MAXIMUM_SPEED_NO_LIMIT },
    { 10, IDM_MAXIMUM_SPEED_10 },
    { 20, IDM_MAXIMUM_SPEED_20 },
    { 50, IDM_MAXIMUM_SPEED_50 },
    { 100, IDM_MAXIMUM_SPEED_100 },
    { 200, IDM_MAXIMUM_SPEED_200 },
    { -1, 0 }
};

static ui_res_possible_values_t RecordingOptions[] = {
    { EVENT_START_MODE_FILE_SAVE, IDM_EVENT_RECORDMODE_SAVE },
    { EVENT_START_MODE_FILE_LOAD, IDM_EVENT_RECORDMODE_LOAD },
    { EVENT_START_MODE_RESET, IDM_EVENT_RECORDMODE_RESET },
    { -1, 0 }
};

static const ui_res_possible_values_t SyncFactor[] = {
    { MACHINE_SYNC_PAL, IDM_SYNC_FACTOR_PAL },
    { MACHINE_SYNC_NTSC, IDM_SYNC_FACTOR_NTSC },
    { MACHINE_SYNC_NTSCOLD, IDM_SYNC_FACTOR_NTSCOLD },
    { -1, 0 }
};

static const ui_res_value_list_t value_list[] = {
    { "RefreshRate", RefreshRateValues, 0 },
    { "Speed", SpeedValues, IDM_MAXIMUM_SPEED_CUSTOM },
    { "MachineVideoStandard", SyncFactor, 0 },
    { "EventStartMode", RecordingOptions, 0 },
    { NULL, NULL, 0 }
};

/* ------------------------------------------------------------------------ */
#ifdef DEBUG
#define NUM_OF_DEBUG_HOTKEYS 3
#define UI_DEBUG_HOTKEYS                                                \
    { FVIRTKEY | FALT | FNOINVERT, VK_F10, IDM_TOGGLE_MAINCPU_TRACE },     \
    { FVIRTKEY | FALT | FNOINVERT, VK_F11, IDM_TOGGLE_DRIVE0CPU_TRACE },   \
    { FVIRTKEY | FALT | FNOINVERT, VK_F12, IDM_TOGGLE_DRIVE1CPU_TRACE },
#else
#define NUM_OF_DEBUG_HOTKEYS 0
#define UI_DEBUG_HOTKEYS
#endif /* DEBUG*/

#define NUM_OF_COMMON_HOTKEYS 26
#define UI_COMMON_HOTKEYS                                               \
    { FVIRTKEY | FCONTROL | FALT | FNOINVERT, 'R', IDM_RESET_HARD },    \
    { FVIRTKEY | FALT | FNOINVERT, 'R', IDM_RESET_SOFT },               \
    { FVIRTKEY | FALT | FNOINVERT, '4', IDM_FORMFEED_PRINTERIEC4 },     \
    { FVIRTKEY | FALT | FNOINVERT, '5', IDM_FORMFEED_PRINTERIEC5 },     \
    { FVIRTKEY | FALT | FNOINVERT, '8', IDM_ATTACH_8 },                 \
    { FVIRTKEY | FALT | FNOINVERT, '9', IDM_ATTACH_9 },                 \
    { FVIRTKEY | FALT | FNOINVERT, '0', IDM_ATTACH_10 },                \
    { FVIRTKEY | FALT | FNOINVERT, '1', IDM_ATTACH_11 },                \
    { FVIRTKEY | FALT | FNOINVERT, 'A', IDM_DETACH_ALL },               \
    { FVIRTKEY | FALT | FNOINVERT, 'T', IDM_ATTACH_TAPE },              \
    { FVIRTKEY | FALT | FNOINVERT, 'L', IDM_LOADQUICK },                \
    { FVIRTKEY | FALT | FNOINVERT, 'S', IDM_SAVEQUICK },                \
    { FVIRTKEY | FALT | FNOINVERT, 'M', IDM_MONITOR },                  \
    { FVIRTKEY | FALT | FNOINVERT, 'X', IDM_EXIT },                     \
    { FVIRTKEY | FALT | FNOINVERT, 'W', IDM_TOGGLE_WARP_MODE },         \
    { FVIRTKEY | FALT | FNOINVERT, 'I', IDM_FLIP_ADD },                 \
    { FVIRTKEY | FALT | FNOINVERT, 'K', IDM_FLIP_REMOVE },              \
    { FVIRTKEY | FALT | FNOINVERT, 'N', IDM_FLIP_NEXT },                \
    { FVIRTKEY | FALT | FNOINVERT, 'B', IDM_FLIP_PREVIOUS },            \
    { FVIRTKEY | FALT | FNOINVERT, 'J', IDM_SWAP_JOYSTICK },            \
    { FVIRTKEY | FALT | FNOINVERT, 'C', IDM_MEDIAFILE },                \
    { FVIRTKEY | FALT | FNOINVERT, 'G', IDM_EVENT_SETMILESTONE },       \
    { FVIRTKEY | FALT | FNOINVERT, 'H', IDM_EVENT_RESETMILESTONE },     \
    { FVIRTKEY | FALT | FNOINVERT, 'D', IDM_TOGGLE_FULLSCREEN },        \
    { FVIRTKEY | FALT | FNOINVERT, VK_RETURN, IDM_TOGGLE_FULLSCREEN },  \
    { FVIRTKEY | FALT | FNOINVERT, VK_PAUSE, IDM_PAUSE }

static ACCEL c64_accel[] = {
    { FVIRTKEY | FALT | FNOINVERT, 'Z', IDM_CART_FREEZE },
    { FVIRTKEY | FALT | FNOINVERT, 'Q', IDM_MOUSE },
    UI_DEBUG_HOTKEYS
    UI_COMMON_HOTKEYS
};

static ACCEL c128_accel[] = {
    { FVIRTKEY | FALT | FNOINVERT, 'Q', IDM_MOUSE },
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

static ACCEL plus4_accel[] = {
    UI_COMMON_HOTKEYS
};

/*static HBRUSH led_red;
static HBRUSH led_green;
static HBRUSH led_black;
static HBRUSH tape_motor_on_brush;
static HBRUSH tape_motor_off_brush;
*/
HWND main_hwnd;

/* Initialize the UI before setting all the resource values.  */
int ui_init(int *argc, char **argv)
{
    WNDCLASS window_class;
    WORD menu;

    switch (machine_class) {
      case VICE_MACHINE_C64:
        menu = IDR_MENUC64;
        ui_accelerator = CreateAcceleratorTable(c64_accel, 
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS + 2);
        break;
      case VICE_MACHINE_C128:
        menu = IDR_MENUC128;
        ui_accelerator = CreateAcceleratorTable(c128_accel, 
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS + 1);
        break;
      case VICE_MACHINE_VIC20:
        menu = IDR_MENUVIC;
        ui_accelerator = CreateAcceleratorTable(vic_accel,
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS);
        break;
      case VICE_MACHINE_PET:
        menu = IDR_MENUPET;
        ui_accelerator = CreateAcceleratorTable(pet_accel,
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS);
        break;
      case VICE_MACHINE_PLUS4:
        menu = IDR_MENUPLUS4;
        ui_accelerator = CreateAcceleratorTable(plus4_accel,
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS);
        break;
      case VICE_MACHINE_CBM2:
        menu = IDR_MENUCBM2;
        ui_accelerator = CreateAcceleratorTable(cbm2_accel,
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS);
        break;
      default:
        log_debug("UI: No menu entries for this machine defined!");
        log_debug("UI: Using C64 type UI menues.");
        menu = IDR_MENUC64;
        ui_accelerator = CreateAcceleratorTable(c64_accel,
            NUM_OF_COMMON_HOTKEYS + NUM_OF_DEBUG_HOTKEYS);
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


    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = dummywindowproc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = winmain_instance;
    window_class.hIcon = LoadIcon(winmain_instance,
                                  MAKEINTRESOURCE(IDI_ICON1));
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0) + 1);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = APPLICATION_CLASS_MAIN;
    RegisterClass(&window_class);

    /* Create the main window.  Notice that we are not going to
       `ShowWindow()' it yet; this will be done as soon as the video module
       requires us to do so.  This is needed both because the video module
       needs an application window to be created to initialize itself, and
       because this might allow us to support more than one emulation window
       in the future.  */
#if 1
    main_hwnd = CreateWindow(APPLICATION_CLASS_MAIN,
                             TEXT("No title"), /* (for now) */
                             WS_OVERLAPPED | WS_CLIPCHILDREN | WS_BORDER
                             | WS_DLGFRAME| WS_SYSMENU | WS_MINIMIZEBOX
                             | WS_MAXIMIZEBOX,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             NULL,
                             NULL,
                             winmain_instance,
                             NULL);
#endif
    InitCommonControls();

    number_of_windows=0;

    statusbar_create_brushes();

    return 0;
}

void ui_shutdown(void)
{

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
                           unsigned int height, int fullscreen)
{
    HWND hwnd;
    int xpos, ypos;

    resources_get_sprintf("Window%dXpos", &xpos, number_of_windows);
    resources_get_sprintf("Window%dYpos", &ypos, number_of_windows);

    hwnd_titles[number_of_windows] = system_mbstowcs_alloc(title);
    hwnd = CreateWindow(APPLICATION_CLASS,
                            hwnd_titles[number_of_windows],
                            WS_OVERLAPPED | WS_CLIPCHILDREN | WS_BORDER
                            | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX
                            | WS_MAXIMIZEBOX,
                            xpos,
                            ypos,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL,
                            NULL,
                            winmain_instance,
                            NULL);

    if (hwnd == NULL)
        log_debug("Window creation failed");

    window_handles[number_of_windows] = hwnd;
    window_canvas_xsize[number_of_windows] = width;
    window_canvas_ysize[number_of_windows] = height;
    number_of_windows++;

    if (!fullscreen) {
        statusbar_create(hwnd);
    }

    ui_resize_canvas_window(hwnd, width, height);

    ShowWindow(hwnd, winmain_cmd_show);

    return hwnd;

}

/* Resize `w' so that the client rectangle is of the requested size.  */
void ui_resize_canvas_window(HWND w, unsigned int width, unsigned int height)
{
    RECT wrect;
    int window_index;
    WINDOWPLACEMENT place;

/*  TODO:
    We should store the windowplacement when the window is
    maximized and we switch to fullscreen, and resume it when
    we are switching back to windowed mode... If the canvas
    size should be changed while in fullscreen (mode changes whatever)
    then this cached data should be updated....
*/
//    if (IsFullscreenEnabled()) return;

    for (window_index = 0; window_index < number_of_windows; window_index++) {
        if (window_handles[window_index] == w)
            break;
    }
    place.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(w, &place);

    window_canvas_xsize[window_index] = width;
    window_canvas_ysize[window_index] = height;

    GetClientRect(w, &wrect);
    ClientToScreen(w, (LPPOINT)&wrect);
    ClientToScreen(w, ((LPPOINT)&wrect) + 1);
    wrect.right = wrect.left + width;
    wrect.bottom = wrect.top + height + statusbar_get_status_height();
    //status_height;
    AdjustWindowRect(&wrect, WS_OVERLAPPED|WS_BORDER|WS_DLGFRAME, TRUE);
    if (place.showCmd == SW_SHOWNORMAL) {
        MoveWindow(w,
                   wrect.left,
                   wrect.top,
                   wrect.right - wrect.left,
                   wrect.bottom - wrect.top,
                   TRUE);
    } else {
        place.rcNormalPosition.right = place.rcNormalPosition.left
                                       + wrect.right - wrect.left;
        place.rcNormalPosition.bottom = place.rcNormalPosition.top
                                        + wrect.bottom - wrect.top;
        SetWindowPlacement(w, &place);
        InvalidateRect(w, NULL, FALSE);
    }
}

/* Update all the menus according to the current settings.  */
void ui_update_menus(void)
{
}

static const ui_menu_toggle_t *machine_specific_toggles = NULL;
static const ui_res_value_list_t *machine_specific_values = NULL;

void ui_register_menu_toggles(const ui_menu_toggle_t *toggles)
{
    machine_specific_toggles = toggles;
}

void ui_register_res_values(const ui_res_value_list_t *valuelist)
{
    machine_specific_values = valuelist;
}

static void update_menus(HWND hwnd)
{
    HMENU menu = GetMenu(hwnd);
    int i, j;
    int value;
    int result;

    for (i = 0; grayed_list[i].name != NULL; i++) {
        resources_get_value(grayed_list[i].name, (void *)&value);
        EnableMenuItem(menu, grayed_list[i].item_id,
                      value ? MF_GRAYED : MF_ENABLED);
    }
    
    for (i = 0; toggle_list[i].name != NULL; i++) {
        resources_get_value(toggle_list[i].name, (void *)&value);
        CheckMenuItem(menu, toggle_list[i].item_id,
                      value ? MF_CHECKED : MF_UNCHECKED);
    }
    
    if (machine_specific_toggles) {
        for (i = 0; machine_specific_toggles[i].name != NULL; i++) {
            resources_get_value(machine_specific_toggles[i].name,
                                (void *)&value);
            CheckMenuItem(menu, machine_specific_toggles[i].item_id,
                          value ? MF_CHECKED : MF_UNCHECKED);
        }
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        result = resources_get_value(value_list[i].name, (void *)&value);
        if (result == 0) {
            unsigned int checked = 0;

            CheckMenuItem(menu, value_list[i].default_item_id, MF_UNCHECKED);
            for (j = 0; value_list[i].vals[j].item_id != 0; j++) {
                if (value == value_list[i].vals[j].value && !checked) {
                    CheckMenuItem(menu, value_list[i].vals[j].item_id,
                                  MF_CHECKED);
                    checked = 1;
                } else {
                    CheckMenuItem(menu, value_list[i].vals[j].item_id,
                                  MF_UNCHECKED);
                }
            }
            if (checked == 0 && value_list[i].default_item_id > 0)
                CheckMenuItem(menu, value_list[i].default_item_id, MF_CHECKED);
        }
    }

    if (machine_specific_values){
        for (i = 0; machine_specific_values[i].name != NULL; i++) {
            result = resources_get_value(machine_specific_values[i].name,
                                         (void *)&value);
            if (result == 0) {
                for (j = 0; machine_specific_values[i].vals[j].item_id != 0;
                    j++) {
                    if (value == machine_specific_values[i].vals[j].value) {
                        CheckMenuItem(menu,
                                      machine_specific_values[i].vals[j].item_id,
                                      MF_CHECKED);
                    } else {
                        CheckMenuItem(menu,
                                      machine_specific_values[i].vals[j].item_id,
                                      MF_UNCHECKED);
                    }
                }
            }
        }
    }
    CheckMenuItem(menu, IDM_PAUSE,
                  ui_emulation_is_paused() ? MF_CHECKED : MF_UNCHECKED);

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
    char *tmp;
    TCHAR *st;
    va_list args;

    va_start(args, format);
    tmp = lib_mvsprintf(format, args);
    va_end(args);

    log_debug(tmp);
    st = system_mbstowcs_alloc(tmp);
    ui_messagebox(st, TEXT("VICE Error!"), MB_OK | MB_ICONSTOP);
    system_mbstowcs_free(st);
    vsync_suspend_speed_eval();
    lib_free(tmp);
}

/* Report an error to the user (one string).  */
void ui_error_string(const char *text)
{
    TCHAR *st;

    log_debug(text);
    st = system_mbstowcs_alloc(text);
    ui_messagebox(st, TEXT("VICE Error!"), MB_OK | MB_ICONSTOP);
    system_mbstowcs_free(st);
}

/* Report a message to the user (`printf()' style).  */
void ui_message(const char *format, ...)
{
    char *tmp;
    TCHAR *st;
    va_list args;

    va_start(args, format);
    tmp = lib_mvsprintf(format, args);
    va_end(args);

    st = system_mbstowcs_alloc(tmp);
    ui_messagebox(st, TEXT("VICE Information"), MB_OK | MB_ICONASTERISK);
    system_mbstowcs_free(st);
    vsync_suspend_speed_eval();
    lib_free(tmp);
}

/* Handle the "CPU JAM" case.  */
ui_jam_action_t ui_jam_dialog(const char *format,...)
{
    char *txt, *txt2;
    TCHAR *st;
    int ret;

    va_list ap;
    va_start(ap, format);
    txt = lib_mvsprintf(format, ap);
    txt2 = lib_msprintf("%s\n\nStart monitor?", txt);
    st = system_mbstowcs_alloc(txt2);
    ret = ui_messagebox(st, TEXT("VICE CPU JAM"), MB_YESNO);
    system_mbstowcs_free(st);
    lib_free(txt2);
    lib_free(txt);
    return (ret == IDYES) ? UI_JAM_MONITOR : UI_JAM_HARD_RESET;
//    UI_JAM_RESET, UI_JAM_HARD_RESET, UI_JAM_MONITOR
}

/* Handle the "Do you want to extend the disk image to 40-track format"?
   dialog.  */
int ui_extend_image_dialog(void)
{
    int ret;

    ret = ui_messagebox(TEXT("Extend image to 40-track format?"),
                        TEXT("VICE question"), MB_YESNO | MB_ICONQUESTION);
    return ret == IDYES;
}

/* ------------------------------------------------------------------------- */
static int is_paused = 0;

static void pause_trap(WORD addr, void *data)
{
    ui_display_paused(1);
    vsync_suspend_speed_eval();
    while (is_paused)
    {
        Sleep(10);
        ui_dispatch_next_event();
    }
}

void ui_pause_emulation(void)
{
    is_paused = is_paused ? 0 : 1;
    if (is_paused) {
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        ui_display_paused(0);
    }
}

static int ui_emulation_is_paused(void)
{
    return is_paused;
}

/* ------------------------------------------------------------------------- */
/* Dispay the current emulation speed.  */
static int statustext_display_time = 0;

void ui_display_speed(float percent, float framerate, int warp_flag)
{
    char *buf, *title;
    TCHAR *st_buf;
    int index;

    for (index = 0; index < number_of_windows; index++) {
        title = system_wcstombs_alloc(hwnd_titles[index]);

        buf = lib_msprintf("%s at %d%% speed, %d fps%s",
                           title, (int)(percent + .5),
                           (int)(framerate + .5),
                           warp_flag ? " (warp)" : "");
        system_wcstombs_free(title);
        st_buf = system_mbstowcs_alloc(buf);
        SetWindowText(window_handles[index], st_buf);
        system_mbstowcs_free(st_buf);

        lib_free(buf);
    }
    
    if (statustext_display_time > 0) {
        statustext_display_time--;
        if (statustext_display_time == 0)
            ui_display_statustext("");
    }
}


void ui_display_statustext(const char *text)
{
    statusbar_setstatustext(text);
    statustext_display_time = 5;
}


/* ------------------------------------------------------------------------- */
/* Dispay the drive status.  */
void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    statusbar_enable_drive_status(enable,drive_led_color);
}

/* Toggle displaying of the drive track.  */
/* drive_base is either 8 or 0 depending on unit or drive display.
   Dual drives display drive 0: and 1: instead of unit 8: and 9: */
void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base,
                            unsigned int half_track_number)
{
    double track_number = (double)half_track_number / 2.0;

    statusbar_display_drive_track(drive_number, drive_base, track_number);
}

/* Toggle displaying of the drive LED.  */
void ui_display_drive_led(int drivenum, int status)
{
    statusbar_display_drive_led(drivenum, status);
}

/* display current image */
void ui_display_drive_current_image(unsigned int drivenum, const char *image)
{
    char *directory_name, *image_name, *text;
    char device_str[4];

    if (image == NULL || image[0] == 0) {
        text = util_concat("Detached device ", 
                           itoa(drivenum + 8, device_str, 10), NULL);
    } else {
       	util_fname_split(image, &directory_name, &image_name);
        text = util_concat("Attached ", image_name, " to device ", 
            itoa(drivenum+8, device_str, 10), NULL);
        lib_free(image_name);
        lib_free(directory_name);
    }

    ui_display_statustext(text);
    lib_free(text);
}


/* ------------------------------------------------------------------------- */
/* Dispay the tape status.  */
void ui_set_tape_status(int tape_status)
{
    statusbar_set_tape_status(tape_status);
}

void ui_display_tape_motor_status(int motor)
{   
    statusbar_display_tape_motor_status(motor);
}

void ui_display_tape_control_status(int control)
{
    statusbar_display_tape_control_status(control);
}

void ui_display_tape_counter(int counter)
{
    statusbar_display_tape_counter(counter);
}

/* display the attched tape image */
void ui_display_tape_current_image(const char *image)
{
    char *directory_name, *image_name, *text;

    if (image == NULL || image[0] == 0) {
        text = lib_stralloc("Detached tape");
    } else {
       	util_fname_split(image, &directory_name, &image_name);
        text = util_concat("Attached tape ", image_name, NULL);
        lib_free(image_name);
        lib_free(directory_name);
    }

    ui_display_statustext(text);
    lib_free(text);
}

/* ------------------------------------------------------------------------- */
/* Dispay the recording/playback status.  */
void ui_display_recording(int recording_status)
{
    if (recording_status)
        ui_display_statustext("Recording started...");
    else
        ui_display_statustext("Recording stopped!");
}

void ui_display_playback(int playback_status)
{
    if (playback_status)
        ui_display_statustext("Playback started...");
    else
        ui_display_statustext("Playback stopped!");
}


/* ------------------------------------------------------------------------- */
/* Dispay the joystick status.  */
static BYTE ui_joyport[3] = { 0, 0, 0 };

void ui_display_joyport(BYTE *joyport)
{
	if (ui_joyport[1] != joyport[1] || ui_joyport[2] != joyport[2])
    {
        ui_joyport[1] = joyport[1];
        ui_joyport[2] = joyport[2];

        statusbar_display_joyport(ui_joyport);
    }
}


/* ------------------------------------------------------------------------- */
/* Toggle displaying of paused state.  */
void ui_display_paused(int flag)
{
    int index;
    char *buf, *title;
    TCHAR *st_buf;

    for (index = 0; index < number_of_windows; index++) {
        title = system_wcstombs_alloc(hwnd_titles[index]);
        buf = lib_msprintf("%s (%s)", title, flag ? "paused" : "resumed");
        system_wcstombs_free(title);
        st_buf = system_mbstowcs_alloc(buf);
        SetWindowText(window_handles[index], st_buf);
        system_mbstowcs_free(st_buf);
        lib_free(buf);
    }
}

ui_button_t ui_ask_confirmation(const char *title, const char *text)
{
    return UI_BUTTON_NONE;
}

static void save_snapshot_trap(WORD unused_addr, void *hwnd)
{
    SuspendFullscreenModeKeep(hwnd);
    ui_snapshot_save_dialog(hwnd);
    ResumeFullscreenModeKeep(hwnd);
}

static void load_snapshot_trap(WORD unused_addr, void *hwnd)
{
    SuspendFullscreenModeKeep(hwnd);
    ui_snapshot_load_dialog(hwnd);
    ResumeFullscreenModeKeep(hwnd);
}

typedef struct {
    char name[MAX_PATH];
    int valid;
} snapfiles;

static snapfiles files[10];
static int lastindex;
static int snapcounter;

static void save_quicksnapshot_trap(WORD unused_addr, void *unused_data)
{
    int i,j;
    char *fullname, *fullname2;
    TCHAR *st_fullname, *st_fullname2;

    if (lastindex == -1) {
        lastindex = 0;
        strcpy(files[lastindex].name, "quicksnap0.vsf");
    } else {
        if (lastindex == 9) {
            if (snapcounter == 10) {
                fullname = util_concat(archdep_boot_path(), "\\", machine_name,
                                       "\\", files[0].name, NULL);
                st_fullname = system_mbstowcs_alloc(fullname);
                DeleteFile(st_fullname);
                system_mbstowcs_free(st_fullname);
                lib_free(fullname);
                for (i = 1; i < 10; i++) {
                    fullname = util_concat(archdep_boot_path(), "\\",
                                           machine_name,
                                           "\\", files[i].name, NULL);
                    fullname2 = util_concat(archdep_boot_path(), "\\",
                                            machine_name,
                                            "\\", files[i-1].name, NULL);
                    st_fullname = system_mbstowcs_alloc(fullname);
                    st_fullname2 = system_mbstowcs_alloc(fullname2);
                    MoveFile(st_fullname, st_fullname2);
                    system_mbstowcs_free(st_fullname);
                    system_mbstowcs_free(st_fullname2);
                    lib_free(fullname);
                    lib_free(fullname2);
                }
            } else {
                for (i = 0; i < 10; i++) {
                    if (files[i].valid == 0) break;
                }
                for (j = i + 1; j < 10; j++) {
                    if (files[j].valid) {
                        strcpy(files[i].name,files[j].name);
                        files[i].name[strlen(files[i].name) - 5] = '0' + i;
                        fullname = util_concat(archdep_boot_path(), "\\",
                                               machine_name, "\\",
                                               files[j].name, NULL);
                        fullname2 = util_concat(archdep_boot_path(), "\\",
                                                machine_name, "\\",
                                                files[i].name, NULL);
                        st_fullname = system_mbstowcs_alloc(fullname);
                        st_fullname2 = system_mbstowcs_alloc(fullname2);
                        MoveFile(st_fullname, st_fullname2);
                        system_mbstowcs_free(st_fullname);
                        system_mbstowcs_free(st_fullname2);
                        lib_free(fullname);
                        lib_free(fullname2);
                        i++;
                    }
                }
                strcpy(files[i].name,files[0].name);
                files[i].name[strlen(files[i].name) - 5]= '0' + i;
                lastindex = i;
            }
        } else {
            strcpy(files[lastindex + 1].name,files[lastindex].name);
            lastindex++;
            files[lastindex].name[strlen(files[lastindex].name) - 5]
                = '0' + lastindex;
        }
    }

    fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\",
                      files[lastindex].name, NULL);
    if (machine_write_snapshot(fullname, 0, 0, 0) < 0) {
        ui_error("Can't write snapshot file.");
    }
    lib_free(fullname);
}

static void load_quicksnapshot_trap(WORD unused_addr, void *unused_data)
{
    char *fullname;

    fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\",
                           files[lastindex].name, NULL);
    if (machine_read_snapshot(fullname, 0) < 0) {
        ui_error("Cannot read snapshot image");
    }
    lib_free(fullname);
}

/* ------------------------------------------------------------------------ */

/* Return the main window handler.  */
HWND ui_get_main_hwnd(void)
{
    if (window_handles[0] == NULL)
        return main_hwnd;

    return window_handles[0];
}

/* Dispatch the next pending event, if any.  Otherwise, just return.  */
void ui_dispatch_next_event(void)
{
    MSG msg;

    if (!GetMessage(&msg, NULL, 0, 0))
        exit(msg.wParam);
    if (ui_accelerator) {
        if (!TranslateAccelerator(msg.hwnd, ui_accelerator, &msg)) {
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
    char *version;
    TCHAR *st_version;

    switch (msg) {
      case WM_INITDIALOG:
#ifdef UNSTABLE
        version = lib_msprintf("Version %s *UNSTABLE*", VERSION);
#else /* #ifdef UNSTABLE */
        version = lib_msprintf("Version %s", VERSION);
#endif /* #ifdef UNSTABLE */
        st_version = system_mbstowcs_alloc(version);
        SetDlgItemText(dialog, IDC_ABOUT_VERSION, st_version);
        system_mbstowcs_free(st_version);
        lib_free(version);
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

static void disk_attach_dialog_proc(WPARAM wparam, HWND hwnd)
{
    TCHAR *st_name;
    int unit = 8;
    int autostart_index = -1;

    SuspendFullscreenModeKeep(hwnd);
    switch (wparam & 0xffff) {
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
    if ((st_name = uilib_select_file_autostart(hwnd, TEXT("Attach disk image"),
        UILIB_FILTER_DISK | UILIB_FILTER_ZIP | UILIB_FILTER_ALL,
        UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DISK,
        &autostart_index)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index,
                AUTOSTART_MODE_RUN) < 0)
                ui_error("Cannot autostart specified file.");
        } else {
            if (file_system_attach_disk(unit, name) < 0)
                ui_error("Cannot attach specified file");
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
    ResumeFullscreenModeKeep(hwnd);
}

static void tape_attach_dialog_proc(HWND hwnd)
{
    TCHAR *st_name;
    int autostart_index = -1;

    SuspendFullscreenModeKeep(hwnd);
    if ((st_name = uilib_select_file_autostart(hwnd, TEXT("Attach tape image"),
        UILIB_FILTER_TAPE | UILIB_FILTER_ZIP | UILIB_FILTER_ALL,
        UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_TAPE,
        &autostart_index)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index,
                AUTOSTART_MODE_RUN) < 0)
                ui_error("Cannot autostart specified file.");
        } else {
            if (tape_image_attach(1, name) < 0)
                ui_error("Cannot attach specified file");
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
    ResumeFullscreenModeKeep(hwnd);
}

static void autostart_attach_dialog_proc(HWND hwnd)
{
    TCHAR *st_name;
    int autostart_index = 0;

    if ((st_name = uilib_select_file_autostart(hwnd,
        TEXT("Autostart disk/tape image"),
        UILIB_FILTER_DISK | UILIB_FILTER_TAPE | UILIB_FILTER_ZIP
        | UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD,
        UILIB_SELECTOR_STYLE_DISK_AND_TAPE, &autostart_index)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (autostart_autodetect(name, NULL, autostart_index,
            AUTOSTART_MODE_RUN) < 0)
            ui_error("Cannot autostart specified file.");
        system_wcstombs_free(name);
        lib_free(st_name);
    }
}

static void reset_dialog_proc(WPARAM wparam)
{
    vsync_suspend_speed_eval();
/*
    if (ui_messagebox("Do you really want to reset the emulated machine?",
                      ((wparam & 0xffff) == IDM_RESET_HARD ? "Hard reset"
                      : "Soft reset"),
                      MB_YESNO | MB_ICONQUESTION) == IDYES)
*/
    {
        if ((wparam & 0xffff) == IDM_RESET_HARD) {
            machine_trigger_reset(MACHINE_RESET_MODE_HARD);
        } else {
            machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
        }
    }
}

static void scan_files(void)
{
    WIN32_FIND_DATA file_info;
    HANDLE search_handle;
    int i;
    char *dirname;
    TCHAR *st_dirname;

    dirname = util_concat(archdep_boot_path(), "\\", machine_name,
                          "\\quicksnap?.vsf", NULL);
    st_dirname = system_mbstowcs_alloc(dirname);
    search_handle = FindFirstFile(st_dirname, &file_info);
    system_mbstowcs_free(st_dirname);
    snapcounter = 0;
    lastindex = -1;
    for (i = 0; i < 10; i++) {
        files[i].valid = 0;
    }
    if (search_handle != INVALID_HANDLE_VALUE) {
        do {
            TCHAR c;
            c = file_info.cFileName[_tcslen(file_info.cFileName) - 5];
            if ((c >= '0') && (c <= '9')) {
                strcpy(files[c - '0'].name, file_info.cFileName);
                files[c - '0'].valid = 1;
                if ((c - '0') > lastindex) {
                    lastindex = c - '0';
                }
                snapcounter++;
            }
        } while (FindNextFile(search_handle, &file_info));
        FindClose(search_handle);
    }
    lib_free(dirname);
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
        ui_peripheral_dialog(hwnd);
        break;
      case IDM_EXIT | 0x00010000:
      case IDM_EXIT:
        PostMessage(hwnd, WM_CLOSE, wparam, lparam);
        break;
      case IDM_ABOUT:
        DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_ABOUT), hwnd,
                  (DLGPROC)about_dialog_proc);
        break;
      case IDM_HELP:
        fname = util_concat(archdep_boot_path(), "\\DOC\\vice_toc.html", NULL);
        dname = util_concat(archdep_boot_path(), "\\DOC", NULL);
        ShellExecute(hwnd, "open", fname, NULL, dname, 0);
        lib_free(fname);
        lib_free(dname);
        break;
      case IDM_CONTRIBUTORS:
        ui_show_text(hwnd, "VICE contributors", "Who made what?",
                     info_contrib_text);
        break;
      case IDM_LICENSE:
        ui_show_text(hwnd, "License",
                     "VICE license (GNU General Public License)",
                     info_license_text);
        break;
      case IDM_WARRANTY:
        ui_show_text(hwnd, "No warranty!",
                     "VICE is distributed WITHOUT ANY WARRANTY!",
                     info_warranty_text);
        break;
      case IDM_CMDLINE:
        uilib_show_options(hwnd);
        break;
      case IDM_ATTACH_8 | 0x00010000:
      case IDM_ATTACH_9 | 0x00010000:
      case IDM_ATTACH_10 | 0x00010000:
      case IDM_ATTACH_11 | 0x00010000:
      case IDM_ATTACH_8:
      case IDM_ATTACH_9:
      case IDM_ATTACH_10:
      case IDM_ATTACH_11:
        disk_attach_dialog_proc(wparam, hwnd);
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
      case IDM_DETACH_ALL | 0x00010000:
      case IDM_DETACH_ALL:
        file_system_detach_disk(8);
        file_system_detach_disk(9);
        file_system_detach_disk(10);
        file_system_detach_disk(11);
        break;
      case IDM_FLIP_ADD | 0x00010000:
      case IDM_FLIP_ADD:
        flip_add_image(8);
        break;
      case IDM_FLIP_REMOVE | 0x00010000:
      case IDM_FLIP_REMOVE:
        flip_remove(8, NULL);
        break;
      case IDM_FLIP_NEXT | 0x00010000:
      case IDM_FLIP_NEXT:
        flip_attach_head(8, 1);
        break;
      case IDM_FLIP_PREVIOUS | 0x00010000:
      case IDM_FLIP_PREVIOUS:
        flip_attach_head(8, 0);
        break;
      case IDM_FLIP_LOAD:
        uifliplist_load_dialog(hwnd);
        break;
      case IDM_FLIP_SAVE:
        uifliplist_save_dialog(hwnd);
        break;
      case IDM_ATTACH_TAPE | 0x00010000:
      case IDM_ATTACH_TAPE:
        tape_attach_dialog_proc(hwnd);
        break;
      case IDM_DETACH_TAPE:
        tape_image_detach(1);
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
        autostart_attach_dialog_proc(hwnd);
        break;
      case IDM_SNAPSHOT_LOAD | 0x00010000:
      case IDM_SNAPSHOT_LOAD:
        if (!ui_emulation_is_paused())
            interrupt_maincpu_trigger_trap(load_snapshot_trap, hwnd);
        else
            load_snapshot_trap(0, 0);
        /* ui_snapshot_load_dialog(main_hwnd);*/
        break;
      case IDM_SNAPSHOT_SAVE | 0x00010000:
      case IDM_SNAPSHOT_SAVE:
        interrupt_maincpu_trigger_trap(save_snapshot_trap, hwnd);
        break;
      case IDM_SAVEQUICK | 0x00010000:
      case IDM_SAVEQUICK:
        scan_files();
        interrupt_maincpu_trigger_trap(save_quicksnapshot_trap, (void *)0);
        break;
      case IDM_LOADQUICK | 0x00010000:
      case IDM_LOADQUICK:
        scan_files();
        if (snapcounter > 0) {
            interrupt_maincpu_trigger_trap(load_quicksnapshot_trap, (void *)0);
        }
        break;
      case IDM_MEDIAFILE | 0x00010000:
      case IDM_MEDIAFILE:
        SuspendFullscreenModeKeep(hwnd);
        ui_mediafile_save_dialog(hwnd);
        ResumeFullscreenModeKeep(hwnd);
        break;
      case IDM_PAUSE:
      case IDM_PAUSE | 0x00010000:
        ui_pause_emulation();
        break;
      case IDM_MONITOR | 0x00010000:
      case IDM_MONITOR:
        if (!ui_emulation_is_paused())
            monitor_startup_trap();
/*
        else
            monitor_startup();
*/
        break;
      case IDM_RESET_HARD + 0x00010000:
      case IDM_RESET_SOFT + 0x00010000:
      case IDM_RESET_HARD:
      case IDM_RESET_SOFT:
        reset_dialog_proc(wparam);
        break;
      case IDM_RESET_DRIVE8:
        vsync_suspend_speed_eval();
        drivecpu_trigger_reset(0);
        break;
      case IDM_RESET_DRIVE9:
        vsync_suspend_speed_eval();
        drivecpu_trigger_reset(1);
        break;
      case IDM_RESET_DRIVE10:
        vsync_suspend_speed_eval();
        drivecpu_trigger_reset(2);
        break;
      case IDM_RESET_DRIVE11:
        vsync_suspend_speed_eval();
        drivecpu_trigger_reset(3);
        break;
      case IDM_MAXIMUM_SPEED_CUSTOM:
        ui_speed_settings_dialog(hwnd);
        break;
      case IDM_DATASETTE_SETTINGS:
        ui_datasette_settings_dialog(hwnd);
        break;
      case IDM_JOY_SETTINGS:
        ui_joystick_settings_dialog(hwnd);
        break;
      case IDM_SWAP_JOYSTICK | 0x00010000:
      case IDM_SWAP_JOYSTICK:
        ui_joystick_swap_joystick();
        break;
      case IDM_SOUND_SETTINGS:
        ui_sound_settings_dialog(hwnd);
        break;
      case IDM_RAM_SETTINGS:
        ui_ram_settings_dialog(hwnd);
        break;
      case IDM_TOGGLE_FULLSCREEN | 0x00010000:
      case IDM_TOGGLE_FULLSCREEN:
        SwitchFullscreenMode(hwnd);
        break;
      case IDM_SETTINGS_SAVE:
        if (resources_save(NULL) < 0)
            ui_error("Cannot save settings.");
        else
            ui_message("Settings saved successfully.");
	{
	    char *fname = archdep_default_fliplist_file_name();
	    flip_save_list((unsigned int) -1, fname);
	    lib_free(fname);
	}
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
      case IDM_EVENT_DIRECTORY:
        uilib_select_file(hwnd, TEXT("Select directory for event history"), 
                          UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT,
                          UILIB_SELECTOR_TYPE_DIR_EXIST,
                          UILIB_SELECTOR_STYLE_EVENT);
        break;
      case IDM_EVENT_TOGGLE_RECORD:
        {
            int recording_new = (event_record_active() ? 0 : 1);

            if (recording_new)
                event_record_start();
            else
                event_record_stop();
        }
        break;
      case IDM_EVENT_TOGGLE_PLAYBACK:
        {
            int playback_new = (event_playback_active() ? 0 : 1);

            if (playback_new)
                event_playback_start();
            else
                event_playback_stop();
        }
        break;
      case IDM_EVENT_SETMILESTONE:
      case IDM_EVENT_SETMILESTONE | 0x00010000:
        event_record_set_milestone();
        break;
      case IDM_EVENT_RESETMILESTONE:
      case IDM_EVENT_RESETMILESTONE | 0x00010000:
        event_record_reset_milestone();
        break;
      case IDM_FORMFEED_PRINTERIEC4 | 0x00010000:
        printer_formfeed(0);
        break;
      case IDM_FORMFEED_PRINTERIEC5 | 0x00010000:
        printer_formfeed(1);
        break;
      default:
        {
            int i, j, command_found = 0;

            for (i = 0; toggle_list[i].name != NULL && !command_found; i++) {
                if (toggle_list[i].item_id == wparam) {
                    resources_toggle(toggle_list[i].name, NULL);
                     command_found = 1;
                }
            }

            if (machine_specific_toggles) {
                for (i = 0; machine_specific_toggles[i].name != NULL 
                            && !command_found; i++) {
                    if (machine_specific_toggles[i].item_id == wparam) {
                        resources_toggle(machine_specific_toggles[i].name,
                                         NULL);
                     command_found = 1;
                    }
                }
            }


            for (i = 0; value_list[i].name != NULL && !command_found; i++) {
                for (j = 0; value_list[i].vals[j].item_id != 0 
                            && !command_found; j++) {
                    if (value_list[i].vals[j].item_id == wparam) {
                        resources_set_value(value_list[i].name, 
                            (resource_value_t) value_list[i].vals[j].value);
                        command_found = 1;
                    }
                }
            }

            if (machine_specific_values) {
                for (i = 0; machine_specific_values[i].name != NULL 
                            && !command_found; i++) {
                    for (j = 0; machine_specific_values[i].vals[j].item_id != 0
                                && !command_found; j++) {
                        if (machine_specific_values[i].vals[j].item_id 
                                == wparam) {
                            resources_set_value(
                                machine_specific_values[i].name, 
                                (resource_value_t) 
                                    machine_specific_values[i].vals[j].value);
                            command_found = 1;
                        }
                    }
                }
            }
        }
    }
}


int ui_active = FALSE;
HWND ui_active_window;

/* Window procedure.  All messages are handled here.  */
static long CALLBACK dummywindowproc(HWND window, UINT msg,
                                     WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
      case WM_ENABLE:
//      log_debug("DUMMY WM_ENABLE %s", wparam == TRUE ? "TRUE" : "FALSE");
        break;
      case WM_ACTIVATEAPP:
//      log_debug("DUMMY WM_ACTIVATEAPP %s",
//                wparam == TRUE ? "ACTIVATE" : "DEACTIVATE");
        break;
      case WM_ACTIVATE:
//      log_debug("DUMMY WM_ACTIVATE %s", wparam == WA_ACTIVE ? "ACTIVATE"
//                : wparam == WA_INACTIVE ?"DEACTIVATE" : "MOUSECLICK");
        break;
      case WM_KILLFOCUS:
//      log_debug("DUMMY WM_KILLFOCUS");
        break;
      case WM_SETFOCUS:
//      log_debug("DUMMY WM_SETFOCUS");
        break;
      case WM_SETREDRAW:
//      log_debug("DUMMY WM_SETREDRAW %s",wparam==TRUE? "TRUE":"FALSE");
        break;
    }
    return DefWindowProc(window, msg, wparam, lparam);
}

extern int fullscreen_transition;

/* Window procedure.  All messages are handled here.  */
static long CALLBACK window_proc(HWND window, UINT msg,
                                 WPARAM wparam, LPARAM lparam)
{
    int window_index;

    for (window_index = 0; window_index < number_of_windows; window_index++) {
        if (window_handles[window_index] == window)
            break;
    }

    switch (msg) {
      case WM_CREATE:
        DragAcceptFiles(window, TRUE);
        return 0;
      case WM_SETREDRAW:
//      log_debug("WM_SETREDRAW %s", wparam == TRUE? "TRUE" : "FALSE");
        break;
      case WM_KILLFOCUS:
//      log_debug("WM_KILLFOCUS");
        break;
      case WM_SETFOCUS:
//      log_debug("WM_SETFOCUS");
        break;
      case WM_ENABLE:
//      log_debug("WM_ENABLE %s %d %08x",
//                wparam == TRUE ? "TRUE" : "FALSE", window_index, window);
        break;
      case WM_ACTIVATEAPP:
        if (wparam == TRUE) {
//          log_debug("WM_ACTIVATEAPP activate %d %08x", window_index,window);
        } else {
//          log_debug("WM_ACTIVATEAPP deactivate %d %08x", window_index,window);
        }
//      return 0;
        break;
      case WM_ACTIVATE:
        if (wparam == WA_INACTIVE) {
//          log_debug("WM_ACTIVATE inactive %d %08x", window_index, window);
            ui_active = FALSE;
//          if (!fullscreen_transition)
//              SuspendFullscreenMode(window);
        } else {
//          log_debug("WM_ACTIVATE active %d %08x", window_index, window);
            ui_active = TRUE;
            ui_active_window = window;
//          if (!fullscreen_transition)
//              ResumeFullscreenMode(window);
        }
        mouse_update_mouse_acquire();
        break;
      case WM_SIZE:
        if (window_index<number_of_windows) {
            statusbar_handle_WMSIZE(msg, wparam, lparam, window_index);
        }
        return 0;
      case WM_DRAWITEM:
        statusbar_handle_WMDRAWITEM(wparam,lparam);
        return 0;
      case WM_COMMAND:
        handle_wm_command(wparam, lparam, window);
        return 0;
      case WM_ENTERMENULOOP:
        update_menus(window);
      case WM_ENTERSIZEMOVE:
        vsync_suspend_speed_eval();
        ui_active = FALSE;
        mouse_update_mouse_acquire();
        break;
      case WM_EXITMENULOOP:
      case WM_EXITSIZEMOVE:
        if (GetActiveWindow() == window || !IsIconic(window)) {
            ui_active = TRUE;
        } else {
            ui_active = FALSE;
        }
        mouse_update_mouse_acquire();
        break;
      case WM_MOVE:
          if (window_index<number_of_windows) {
              WINDOWPLACEMENT place;
              RECT  rect;

              place.length = sizeof(WINDOWPLACEMENT);
              GetWindowPlacement(window, &place);
              GetWindowRect(window, &rect);
              if (place.showCmd == SW_SHOWNORMAL) {
                  resources_set_sprintf("Window%dXpos", 
                                        (resource_value_t)rect.left, window_index);
                  resources_set_sprintf("Window%dYpos", 
                                        (resource_value_t)rect.top, window_index);
              }
          }
          break;
      case WM_SYSKEYDOWN:
        if (wparam == VK_F10) {
            kbd_handle_keydown(wparam, lparam);
            return 0;
        }
        break;
      case WM_KEYDOWN:
        kbd_handle_keydown(wparam, lparam);
        return 0;
      case WM_SYSKEYUP:
        kbd_handle_keyup(wparam, lparam);
        if (wparam == VK_F10)
              return 0;
        break;
      case WM_KEYUP:
        kbd_handle_keyup(wparam, lparam);
        return 0;
      case WM_SYSCOLORCHANGE:
        syscolorchanged = 1;
        break;
      case WM_DISPLAYCHANGE:
//      log_debug("Display changed %d %d %d",
//                LOWORD(lparam), HIWORD(lparam), wparam);
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
            int confirm_on_exit, save_on_exit;

            resources_get_value("ConfirmOnExit", &confirm_on_exit);
            resources_get_value("SaveResourcesOnExit", &save_on_exit);

            SuspendFullscreenModeKeep(window);
            vsync_suspend_speed_eval();
            if (confirm_on_exit)
            {
//              log_debug("Asking exit confirmation");
                if (MessageBox(window,
                    TEXT("Do you really want to exit?\n\n"
                    "All the data present in the emulated RAM will be lost."),
                    TEXT("VICE"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
                    | MB_TASKMODAL) == IDYES) {
                    quit = 1;
                } else {
                    quit = 0;
                }
            }

            if (quit) {
	            SuspendFullscreenMode(window);
               if (save_on_exit) {
                   if (resources_save(NULL)<0) {
                       ui_error("Cannot save settings.");
                   }
               }
               DestroyWindow(window);
            } else {
                ResumeFullscreenModeKeep(window);
            }
        }
        return 0;
      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
      case WM_ERASEBKGND:
        return 1;
      case WM_DROPFILES:
        {
            char szFile[256];
            HDROP hDrop;

            hDrop = (HDROP)wparam;
            DragQueryFile(hDrop, 0, (char *)&szFile, 256);
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                if (file_system_attach_disk(8, szFile) < 0)
                    ui_error("Cannot attach specified file");
            } else {
                if (autostart_autodetect(szFile, NULL, 0,
                    AUTOSTART_MODE_RUN) < 0)
                    ui_error("Cannot autostart specified file.");
            }
            DragFinish (hDrop);
            return 0;
        }
      case WM_PAINT:
        {
            RECT update_rect;

            if (GetUpdateRect(window, &update_rect, FALSE)) {
                PAINTSTRUCT ps;
                HDC hdc;
                int frame_coord[6];

                hdc = BeginPaint(window, &ps);

                frame_coord[0] = update_rect.left;
                frame_coord[1] = update_rect.top;
                frame_coord[2] = update_rect.right;
                frame_coord[3] = update_rect.bottom;

                canvas_update(window, hdc,update_rect.left, update_rect.top,
                              update_rect.right - update_rect.left,
                              update_rect.bottom - update_rect.top);

                EndPaint(window, &ps);
                return 0;
            } else
                break;
        }
    }

    return DefWindowProc(window, msg, wparam, lparam);
}

/*
 The following shows a messagebox on the screen.
 It first searches the current active window of VICE, and then 
 suspends fullscreen mode, shows the messagebox, and then resumes
 fullscreen mode.
 If it can't find the current active window of VICE (e.g., no one
 is currently active), it does not suspend and resume fullscreen
 mode. This should be correct because when fullscreen, that window
 has to be active.
*/
int ui_messagebox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    int ret;
    HWND hWnd = NULL;
    
    if (number_of_windows == 1) {
        /* we only have one window, so use that one */
        hWnd = window_handles[0];
    } else {
        int window_index;

        HWND hWndActive = GetActiveWindow();

        for (window_index = 0; window_index < number_of_windows;
            window_index++) {
            if (window_handles[window_index] == hWndActive) {
                hWnd = hWndActive;
                break;
            }
        }
    }

    if (hWnd != NULL) {
        SuspendFullscreenModeKeep(hWnd);
    }

    ret = MessageBox(hWnd, lpText, lpCaption, uType);

    if (hWnd != NULL) {
        ResumeFullscreenModeKeep(hWnd);
    }

    return ret;
}

