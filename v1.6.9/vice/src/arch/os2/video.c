/*
 * video.c - Video implementation for Vice/2, using DIVE.
 *
 * Written by
 *  Thomas Bretz <tbretz@gsi.de>
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

#define INCL_GPI             // RGNRECT
#define INCL_WINSYS          // SV_CYTITLEBAR
#define INCL_WINHELP         // WinCreateHelpInstance
#define INCL_WININPUT        // WM_CHAR
#define INCL_WINMENUS        // PU_*
#define INCL_WINSTDDRAG      // PDRAGINFO in dragndrop.h
#define INCL_DOSPROCESS      // DosSleep
#define INCL_WINSTATICS      // SS_TEXT
#define INCL_WINFRAMEMGR     // WM_TRANSLATEACCEL
#define INCL_WINWINDOWMGR    // QWL_USER
#define INCL_DOSSEMAPHORES   // HMTX
#include <os2.h>
#define INCL_MMIO
#include <os2me.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __EMX__
#include <graph.h>
#endif

#include "video.h"
//#include "dlg-emulator.h" // ID_SPEEDDISP

#include "log.h"
#include "proc.h"
#include "ui.h"
#include "ui_status.h"
#include "utils.h"
#include "dialogs.h"
#include "menubar.h"
#include "dragndrop.h"
#include "machine.h"      // machine_canvas_screenshot
#include "cmdline.h"
#include "resources.h"
#include "screenshot.h"  // screenshot_t
#include "snippets\pmwin2.h"

#include <dive.h>
#ifdef __IBMC__
#include <fourcc.h>
#endif

#ifdef HAVE_MOUSE
#include "mouse.h"
#endif

#include "version.h"

#define DEBUG(txt)
//#define DEBUG(txt) log_debug(txt)

extern void archdep_create_mutex_sem(HMTX *hmtx, const char *pszName, int fState);

static HMTX  hmtx;
static CHAR  szClientClass [] = "VICE/2 Grafic Area";
static CHAR  szTitleBarText[] = "VICE/2 " VERSION;
static ULONG flFrameFlags =
    FCF_ICON | FCF_MENU | FCF_TITLEBAR | FCF_ACCELTABLE |
    FCF_SHELLPOSITION | FCF_SYSMENU | FCF_TASKLIST |
    FCF_AUTOICON | FCF_MINBUTTON | FCF_CLOSEBUTTON;

/* ------------------------------------------------------------------------ */
/* Video-related resources.  */

int stretch;            // Strech factor for window (1,2,3,...)
static int border;      // PM Border Type
static int menu;        // flag if menu should be enabled
static int status=0;    // flag if status should be enabled
static HWND *hwndlist = NULL;

extern void wmVrnDisabled(HWND hwnd);
extern void wmVrnEnabled(HWND hwnd);

void AddToWindowList(HWND hwnd)
{
    int i=0;

    HWND *newlist;

    if (!hwndlist)
    {
        //
        // If no list exists yet, creat an empty one
        //
        hwndlist = malloc(sizeof(HWND));
        hwndlist[0] = NULLHANDLE;
    }

    //
    // Count valid entries in list
    //
    while (hwndlist[i])
        i++;

    //
    // create new list
    //
    newlist = malloc(sizeof(HWND)*(i+2));

    //
    // copy old list to new list an delete old list
    //
    memcpy(newlist, hwndlist, sizeof(HWND)*i);
    free (hwndlist);

    hwndlist = newlist;

    //
    // add new entry to list
    //
    hwndlist[i]   = hwnd;
    hwndlist[i+1] = NULLHANDLE;
}

static int set_stretch_factor(resource_value_t v, void *param)
{
    int i=0;

    if (!hwndlist || stretch==(int)v)
    {
        stretch=(int)v;
        return 0;
    }


    //
    // disable visible region (stop blitting to display)
    //
    i = 0;
    while (hwndlist[i])
        wmVrnDisabled(hwndlist[i++]);

    //
    // set new stretch factor
    //
    stretch=(int)v;

    i = 0;
    while (hwndlist[i])
    {
        canvas_t *c = (canvas_t *)WinQueryWindowPtr(hwndlist[i], QWL_USER);
        //
        // resize canvas
        //
        canvas_resize(c, c->width, c->height);

        //
        // set visible region (start blitting again)
        //
        wmVrnEnabled(hwndlist[i]);
        i++;
    }

    return 0;
}

/* ------------------------------------------------------------------------ */
static UINT border_size_x(void)
{
    switch (border)
    {
    case 1:
        return WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
    case 2:
        return WinQuerySysValue(HWND_DESKTOP, SV_CXDLGFRAME);
    }
    return 0;
}

static UINT border_size_y(void)
{
    switch (border)
    {
    case 1:
        return WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);
    case 2:
        return WinQuerySysValue(HWND_DESKTOP, SV_CYDLGFRAME);
    }
    return 0;
}

static UINT statusbar_height(void)
{
    const UINT fntht  = 11;
    const UINT bdry   = WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);
    const UINT offset = border==2 ? 0 : 2;

    return fntht + 2*bdry + 2 + offset + 3;
}
/*
static HWND AddStatusFrame(HWND hwnd, UINT x, UINT width, int frame)
{
    const UINT fntht  = 11;
    const UINT bdry   = WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);
    const UINT offset = border==2 ? 0 : 2;

    return WinCreateWindow(hwnd, WC_FRAME, NULL,
                           WS_VISIBLE|(frame?FS_BORDER:0),
                           x+offset, offset,
                           width, fntht + 2*bdry + 2,
                           NULLHANDLE, HWND_TOP, -1, NULL, NULL);
}

static HWND AddStatusTxt(HWND hwnd, UINT x, UINT width, int id, char *txt)
{
    const UINT fntht  = 11;
    const UINT bdrx   = WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
    const UINT bdry   = WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

    char *font = "11.System VIO";

    HWND thwnd=WinCreateWindow(hwnd, WC_STATIC, txt,
                               WS_VISIBLE|SS_TEXT|DT_VCENTER|DT_CENTER,
                               x+2*bdrx, bdry,
                               width-4*bdrx, fntht,
                               NULLHANDLE, HWND_TOP,
                               id, //FID_STATUS,
                               NULL, NULL);

    WinSetFont(thwnd, font);

    return thwnd;
}
*/
static UINT canvas_fullheight(UINT height)
{
    height *= stretch;
    height += WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
    height += 2*border_size_y();
    if (menu)   height += WinQuerySysValue(HWND_DESKTOP, SV_CYMENU)+1;  // FIXME: +1 ?
    if (status) height += statusbar_height();
    return height;
}

static UINT canvas_fullwidth(UINT width)
{
    width *= stretch;
#if defined __XVIC__
    width *= 2;
#endif
    width += 2*border_size_x();
    return width;
}

/* ------------------------------------------------------------------------ */
/*
static void status_resize(canvas_t *c)
{
    SWP swp;
    WinQueryWindowPos(WinWindowFromID(c->hwndFrame, FID_STATUS), &swp);

    log_debug("Size 1: %3d %3d", swp.x, swp.y);

    // FIXME
    WinSetWindowPos(WinWindowFromID(c->hwndFrame, FID_STATUS), 0,
                    border_size_x(), border_size_y(),     // Position (x,y)
                    c->width*stretch, statusbar_height(), // Size (width,height)
                    SWP_SIZE|SWP_MOVE);

    WinQueryWindowPos(WinWindowFromID(c->hwndFrame, FID_STATUS), &swp);
    log_debug("Size 2: %3d %3d", swp.x, swp.y);
}

static int set_status(resource_value_t v, void *hwnd)
{
    SWP swp;
    canvas_t *c = (canvas_t *)WinQueryWindowPtr((HWND)hwnd, QWL_USER);

    status = (int)v;

    if (!hwnd || !c)
        return 0;

    //
    // correct window size and position
    //
    WinQueryWindowPos(c->hwndFrame, &swp);
    WinSetWindowPos(c->hwndFrame, 0,
                    swp.x, swp.y+(status?-statusbar_height():statusbar_height()),
                    canvas_fullwidth (c->width),
                    canvas_fullheight(c->height),
                    SWP_SIZE|SWP_MOVE|SWP_MINIMIZE);

    //
    // show or hide the menu bar
    //
    WinShowWindow(WinWindowFromID(c->hwndFrame, FID_STATUS), status);

    //
    // update frame (show or hide menubar physically)
    //
    // REMARK: instead of updating the frame I minimaze the
    // window and maximize it again. This makes sure that
    // the menubar is shown and not only activated. I don't know
    // why, but it seems to work well. (I don't use HIDE and
    // SHOW because this two functions are animated)
    //
    // WinSendMsg(c->hwndFrame, WM_UPDATEFRAME, (void*)FCF_MENU, NULL);

    WinSetWindowPos(c->hwndFrame, 0, 0, 0, 0, 0, SWP_RESTORE);

    return 0;
}
*/
static int set_menu(resource_value_t v, void *param)
{
    int i=0;

    menu = (int)v;

    if (!hwndlist)
        return 0;

    while (hwndlist[i])
    {
        canvas_t *c = (canvas_t *)WinQueryWindowPtr((HWND)hwndlist[i++], QWL_USER);
        //
        // correct window size
        //
        WinSetWindowPos(c->hwndFrame, 0, 0, 0,
                        canvas_fullwidth (c->width),
                        canvas_fullheight(c->height),
                        SWP_SIZE|SWP_MINIMIZE);

        //
        // show or hide the menu bar
        //
        WinSetParent(c->hwndMenu, menu?c->hwndFrame:HWND_OBJECT, FALSE);

        //
        // update frame (show or hide menubar physically)
        //
        // REMARK: instead of updating the frame I minimaze the
        // window and maximize it again. This makes sure that
        // the menubar is shown and not only activated. I don't know
        // why, but it seems to work well. (I don't use HIDE and
        // SHOW because this two functions are animated)
        //
        // WinSendMsg(c->hwndFrame, WM_UPDATEFRAME, (void*)FCF_MENU, NULL);

        WinSetWindowPos(c->hwndFrame, 0, 0, 0, 0, 0, SWP_RESTORE);
    }
    return 0;
}

static int set_border_type(resource_value_t v, void *param)
{
    switch ((int)v)
    {
    case 1:
        flFrameFlags &= ~FCF_DLGBORDER;
        flFrameFlags |= FCF_BORDER;
        border = 1;
        break;
    case 2:
        flFrameFlags &= ~FCF_BORDER;
        flFrameFlags |= FCF_DLGBORDER;
        border = 2;
        break;
    default:
        flFrameFlags &= ~FCF_BORDER;
        flFrameFlags &= ~FCF_DLGBORDER;
        border = 0;
    }
    return 0;
}

static resource_t resources[] = {
    { "WindowStretchFactor", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &stretch, set_stretch_factor, NULL },
    { "PMBorderType", RES_INTEGER, (resource_value_t) 2,
      (resource_value_t *) &border, set_border_type, NULL },
    { "Menubar", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &menu, set_menu, NULL },
/*
    { "Statusbar", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &status, set_status, NULL },
*/
      { NULL }
};

int video_init_resources(void)
{
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-stretch", SET_RESOURCE, 1, NULL, NULL, "WindowStretchFactor", NULL,
      "<number>", "Specify stretch factor for PM Windows (1,2,3,...)" },
    { "-border",  SET_RESOURCE, 1, NULL, NULL, "PMBorderType", NULL,
      "<number>", "Specify window border type (1=small, 2=dialog, else=no border)" },
    { "-menu", SET_RESOURCE, 0, NULL, NULL, "Menubar", (resource_value_t) 1,
      NULL, "Enable Main Menu Bar" },
    { "+menu", SET_RESOURCE, 0, NULL, NULL, "Menubar", (resource_value_t) 0,
      NULL, "Disable Main Menu Bar" },
/*
    { "-status", SET_RESOURCE, 0, NULL, NULL, "Statusbar", (resource_value_t) 1,
      NULL, "Enable Status Bar" },
    { "+status", SET_RESOURCE, 0, NULL, NULL, "Statusbar", (resource_value_t) 0,
      NULL, "Disable Status Bar" },
*/
    { NULL }
};

int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------ */
const int DIVE_RECTLS  = 50;
static HDIVE hDiveInst =  0;  // DIVE instance
static int initialized = FALSE;
static SETUP_BLITTER divesetup;

extern HMTX hmtxKey;

int video_init(void) // initialize Dive
{
    APIRET rc;

    if (rc=DiveOpen(&hDiveInst, FALSE, NULL))
    {
        log_message(LOG_DEFAULT,"video.c: Could not open DIVE (rc=0x%x).",rc);
        return -1;
    }
    // FIXME??? Do we need one sem for every canvas
    // This semaphore serializes the Dive and Frame Buffer access
    //
    archdep_create_mutex_sem(&hmtx, "Video", FALSE);

    divesetup.ulStructLen       = sizeof(SETUP_BLITTER);
    divesetup.fInvert           = FALSE;
    divesetup.ulDitherType      = 0;
    divesetup.fccSrcColorFormat = FOURCC_LUT8;
    divesetup.fccDstColorFormat = FOURCC_SCRN;
    divesetup.pVisDstRects      = xcalloc(DIVE_RECTLS, sizeof(RECTL));

    //
    // this is a dummy setup. It is needed for some graphic
    // drivers that they get valid values for the first time we
    // setup the dive blitter regions
    //
    divesetup.ulSrcWidth  = 1;
    divesetup.ulSrcHeight = 1;
    divesetup.ulSrcPosX   = 0;
    divesetup.ulSrcPosY   = 0;
    divesetup.ulDstWidth  = 1;
    divesetup.ulDstHeight = 1;
    divesetup.lDstPosX    = 0;
    divesetup.lDstPosY    = 0;

    // FIXME
    // Is this the right place to initialize the keyboard semaphore?
    //
    rc=DosCreateMutexSem("\\SEM32\\Vice2\\Keyboard", &hmtxKey, 0, FALSE);
    if (rc==258 /*ERROR_DUPLICATE_NAME*/)
    {
        //
        // we are in a different process (eg. second instance of x64)
        //
        rc=DosOpenMutexSem("\\SEM32\\Vice2\\Keyboard", &hmtxKey);
        if (rc)
            log_debug("video.c: DosOpenMutexSem (rc=%li)", rc);
    }
    else
        if (rc)
            log_debug("video.c: DosCreateMutexSem (rc=%li)", rc);

    initialized = TRUE;

    return 0;
}

void video_close(void)
{
    //
    // video_close is called from the main thread, that means, that
    // vice is not blitting at the same moment.
    //
    APIRET rc;

    DEBUG("VIDEO CLOSE 0");

    if (rc=DiveClose(hDiveInst))
        log_message(LOG_DEFAULT, "video.c: Dive closed (rc=0x%x).", rc);
    else
        log_message(LOG_DEFAULT, "video.c: Dive closed.");
    hDiveInst = 0;

    free(divesetup.pVisDstRects);

    DEBUG("VIDEO CLOSE 1");
}

/* ------------------------------------------------------------------------ */
/* Frame buffer functions.  */
int video_frame_buffer_alloc(video_frame_buffer_t **f, UINT width, UINT height)
{
    APIRET rc;

    if (width<sizeof(ULONG))
        width=sizeof(ULONG); // Sizeline Boundary, Workaround

    (*f) = (video_frame_buffer_t*) xcalloc(1, sizeof(video_frame_buffer_t));
    (*f)->bitmap = (char*) xcalloc(width*height, sizeof(BYTE));
    (*f)->width  = width;
    (*f)->height = height;

    //
    // allocate image buffer, I might be faster to specify 0 for
    // the scanlinesize, but then we need a correction for
    // every time we have a line size which is not
    // devidable by 4
    //
    rc=DiveAllocImageBuffer(hDiveInst, &((*f)->ulBuffer), FOURCC_LUT8,
                            width, height, width, (*f)->bitmap);
    //
    // check for errors
    //
    if (rc)
        log_message(LOG_DEFAULT,"video.c: Error DiveAllocImageBuffer (rc=0x%x).", rc);
    else
        log_message(LOG_DEFAULT,"video.c: Frame buffer #%d allocated (%lix%li)", (*f)->ulBuffer, width, height);

    return 0;
}

void video_frame_buffer_clear(video_frame_buffer_t *f, PIXEL value)
{   // raster_force_repaint, we needn't this
    //    memset((*f)->bitmap, value, ((*f)->width)*((*f)->height)*sizeof(BYTE));
}

void video_frame_buffer_free(video_frame_buffer_t *f)
{
    ULONG rc;
    //
    // This is if video_close calls video_free before a frame_buffer is allocated
    //
    if (!f)
        return;

    DEBUG("FRAME BUFFER FREE 0");

    if (rc=DiveFreeImageBuffer(hDiveInst, f->ulBuffer))
        log_message(LOG_DEFAULT,"video.c: Error DiveFreeImageBuffer (rc=0x%x).", rc);
    else
        log_message(LOG_DEFAULT,"video.c: Frame buffer #%d freed.", f->ulBuffer);


    //
    // if f is valid also f->bitmap must be valid, see video_frame_buffer_alloc
    //
    free(f->bitmap);
    free(f);

    DEBUG("FRAME BUFFER FREE 1");
}

/* ------------------------------------------------------------------------ */
/* PM Window mainloop */

int PM_winActive;

typedef struct _kstate
{
    //    BYTE numlock;
    BYTE scrllock;
    BYTE capslock;
} kstate;

static kstate vk_vice;
static kstate vk_desktop;

extern int use_leds;

void wmCreate(void)
{
    BYTE keyState[256];
    DosRequestMutexSem(hmtxKey, SEM_INDEFINITE_WAIT);
    WinSetKeyboardStateTable(HWND_DESKTOP, keyState, FALSE);
    //
    // store desktop LED status
    //
    vk_desktop.scrllock = keyState[VK_SCRLLOCK];
    vk_desktop.capslock = keyState[VK_CAPSLOCK];
    //
    // if allowed by user: switch LEDs off
    //
    if (use_leds)
    {
        keyState[VK_SCRLLOCK] = 0;   // switch off
        keyState[VK_CAPSLOCK] = 0;   // switch off
        WinSetKeyboardStateTable(HWND_DESKTOP, keyState, TRUE);
    }
    PM_winActive = TRUE;
    DosReleaseMutexSem(hmtxKey);
}

void wmDestroy(void)
{
    BYTE keyState[256];
    DosRequestMutexSem(hmtxKey, SEM_INDEFINITE_WAIT);
    WinSetKeyboardStateTable(HWND_DESKTOP, keyState, FALSE);
    //
    // restore desktop LED status
    //
    keyState[VK_SCRLLOCK] = vk_desktop.scrllock;
    keyState[VK_CAPSLOCK] = vk_desktop.capslock;
    WinSetKeyboardStateTable(HWND_DESKTOP, keyState, TRUE);
    PM_winActive = FALSE;
    DosReleaseMutexSem(hmtxKey);
}

void wmSetSelection(MPARAM mp1)
{
    BYTE keyState[256];
    DosRequestMutexSem(hmtxKey, SEM_INDEFINITE_WAIT);
    WinSetKeyboardStateTable(HWND_DESKTOP, keyState, FALSE);
    //
    // restore vice keyboard LED status (user switched to vice window)
    //
    if (mp1 && !PM_winActive)
    {
        //
        // store system LED status for later usage
        //
        vk_desktop.scrllock = keyState[VK_SCRLLOCK];
        vk_desktop.capslock = keyState[VK_CAPSLOCK];
        //
        // set new status for LEDs if allowed by user
        //
        if (use_leds)
        {
            keyState[VK_SCRLLOCK] = vk_vice.scrllock;  // restore warp led status
            keyState[VK_CAPSLOCK] = 0;                 // drive led off
            WinSetKeyboardStateTable(HWND_DESKTOP, keyState, TRUE);
        }
        PM_winActive=TRUE;
    }
    //
    // restore system keyboard LED status (user switched away from vice window)
    //
    if (!mp1 && PM_winActive)
    {
        //
        // store SCRLLOCK status for later use
        //
        vk_vice.scrllock = keyState[VK_SCRLLOCK];
        //
        // restore system LED status
        //
        keyState[VK_SCRLLOCK] = vk_desktop.scrllock;
        keyState[VK_CAPSLOCK] = vk_desktop.capslock;
        WinSetKeyboardStateTable(HWND_DESKTOP, keyState, TRUE);
        PM_winActive=FALSE;
    }
    DosReleaseMutexSem(hmtxKey);
}

void wmVrn(HWND hwnd)
{
    HPS  hps  = WinGetPS(hwnd);
    HRGN hrgn = GpiCreateRegion(hps, 0L, NULL);

    DEBUG("WM VRN 0");

    if (hrgn) {  // this should be controlled again (clr/home, stretch 3)
        RGNRECT rgnCtl;
        WinQueryVisibleRegion(hwnd, hrgn);
        rgnCtl.ircStart    = 1;
        rgnCtl.crc         = DIVE_RECTLS;
        rgnCtl.ulDirection = RECTDIR_LFRT_BOTTOP;
        if (GpiQueryRegionRects(hps, hrgn, NULL, &rgnCtl, divesetup.pVisDstRects))
        {
            SWP     swp;
            POINTL  pointl;
            WinQueryWindowPos(hwnd, &swp);
            pointl.x = swp.x;
            pointl.y = swp.y;
            WinMapWindowPoints(WinQueryWindow(hwnd, QW_PARENT),
                               HWND_DESKTOP, (POINTL*)&pointl, 1);
            divesetup.lScreenPosX   = pointl.x;
            divesetup.lScreenPosY   = pointl.y;
            divesetup.ulNumDstRects = rgnCtl.crcReturned;
            DiveSetupBlitter(hDiveInst, &divesetup);
        }
        GpiDestroyRegion(hps, hrgn);
    }
    WinReleasePS(hps);

    DEBUG("WM VRN 1");
}

void wmVrnEnabled(HWND hwnd)
{
    canvas_t *c = (canvas_t *)WinQueryWindowPtr(hwnd, QWL_USER);

    DEBUG("WM VRN ENABLED 0");

    DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);
    if (!hDiveInst || !initialized)
    {
        DosReleaseMutexSem(hmtx);
        return;
    }
    wmVrn(hwnd);
    c->vrenabled=TRUE;
    DosReleaseMutexSem(hmtx);
    //
    // blit the whole visible area to the screen at next blit time
    //
    c->exposure_handler(c->width, c->height);

    DEBUG("WM VRN ENABLED 1");
}

void wmVrnDisabled(HWND hwnd)
{
    canvas_t *c = (canvas_t *)WinQueryWindowPtr(hwnd,QWL_USER);

    DEBUG("WM VRN DISABLED 0");

    DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);
    if (!hDiveInst)
    {
        DosReleaseMutexSem(hmtx);
        return;
    }
    //
    // FIXME: This is system wide...
    //
    DiveSetupBlitter(hDiveInst, NULL);
    //
    // ...and this is only for one canvas.
    //
    c->vrenabled=FALSE;
    DosReleaseMutexSem(hmtx);

    DEBUG("WM VRN DISABLED 1");
}

void wmPaint(HWND hwnd)
{
    screenshot_t geom;

    //
    // get pointer to actual canvas from user data area
    //
    canvas_t *c = (canvas_t *)WinQueryWindowPtr(hwnd, QWL_USER);

    DEBUG("WM_PAINT 0");

    //
    // if the canvas isn't setup yet return
    //
    if (!c)
        return;

    DEBUG("WM_PAINT 1");

    //
    // get the frame_buffer and geometry from the machine
    //
    if (machine_canvas_screenshot(&geom, c) < 0)
        return;

    {
        const int x  = geom.x_offset;
        const int y  = geom.first_displayed_line;
        const int cx = geom.max_width & ~3;
        const int cy = geom.last_displayed_line - geom.first_displayed_line;

        DEBUG(("WM_PAINT 2: x=%d y=%d cx=%d cy=%d", x, y, cx, cy));

        //
        // enable the visible region - it is disabled
        //
        wmVrnEnabled(hwnd);

        //
        // blit to canvas (canvas_refresh should be thread safe by itself)
        //
        canvas_refresh(c, geom.frame_buffer, x, y, 0, 0, cx, cy);
    }
}

static int wmTranslateAccel(HWND hwnd, MPARAM mp1)
{
    const QMSG *qmsg = (QMSG*)mp1;

    //
    // if key is pressed together with alt let the acceltable
    // process the pressed key
    //
    if (SHORT1FROMMP(qmsg->mp1)&KC_ALT)
        return TRUE;

    //
    // if the focus of the actual canvas (eg. F1 pressed in
    // open menu) isn't the actual focus let the acceltable
    // process the pressed key
    //
    if (hwnd != qmsg->hwnd)
        return TRUE;

    //
    // don't let the acceltable translate the key (eg. to get
    // F1 passed to WM_CHAR)
    //
    return FALSE;
}

static void DisplayPopupMenu(HWND hwnd, POINTS *pts)
{
    const canvas_t *c = (canvas_t *)WinQueryWindowPtr(hwnd, QWL_USER);

    WinPopupMenu(hwnd, hwnd, c->hwndPopupMenu, pts->x, pts->y, 0,
                 PU_MOUSEBUTTON2DOWN|PU_HCONSTRAIN|PU_VCONSTRAIN);
}

MRESULT EXPENTRY PM_winProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
    case WM_CREATE:       wmCreate();                break;
    case WM_PAINT:        wmPaint(hwnd);             break;
    case WM_CHAR:         kbd_proc(hwnd, mp1, mp2);  break;
    case WM_CLOSE:
    case WM_DESTROY:      wmDestroy();               break;
    case WM_SETSELECTION: wmSetSelection(mp1);       break;
    case WM_VRNDISABLED:  wmVrnDisabled(hwnd);       break;
    case WM_VRNENABLED:   wmVrnEnabled(hwnd);        break;

    case DM_DRAGOVER:
    case DM_DROP:
        return DragDrop(hwnd, msg, mp1);

    case WM_TRANSLATEACCEL:
        if (wmTranslateAccel(hwnd, mp1))
            break;
        return FALSE;

    case WM_COMMAND:
        menu_action(hwnd, SHORT1FROMMP(mp1), mp2);
        break;

    case WM_MENUSELECT:
        menu_select(HWNDFROMMP(mp2), SHORT1FROMMP(mp1));
        break;

#ifdef HAVE_MOUSE
    case WM_BUTTON2DOWN:
        if (!menu)
        {
            DisplayPopupMenu(hwnd, (POINTS*)&mp1);
            return FALSE;
        }
    case WM_MOUSEMOVE:
    case WM_BUTTON1DOWN:
    case WM_BUTTON1UP:
    case WM_BUTTON2UP:
        mouse_button(hwnd, msg, mp1);
        break;
#endif
        /*
         case WM_SETFOCUS: vidlog("WM_SETFOCUS",mp1); break;
         case WM_ACTIVATE: vidlog("WM_ACTIVATE",mp1); break;
         case WM_FOCUSCHANGE: vidlog("WM_FOCUSCHANGE",mp1); break;
         case WM_MOVE:
         WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &pos);
         break;
         */
    }
    return WinDefWindowProc (hwnd, msg, mp1, mp2);
}

static void InitHelp(HWND hwndFrame, int id, PSZ title, PSZ libname)
{
    HWND     hwndHelp;
    HELPINIT hini;
    HAB      hab = WinQueryAnchorBlock(hwndFrame);

    //
    // initialize help init structure
    //
    memset(&hini, 0, sizeof(hini));
    hini.cb                 = sizeof(HELPINIT);
    hini.phtHelpTable       = (PHELPTABLE)MAKELONG(id, 0xFFFF);
    hini.pszHelpWindowTitle = (PSZ)title;
    hini.pszHelpLibraryName = (PSZ)libname;
#ifdef HLPDEBUG
    hini.fShowPanelId       = CMIC_SHOW_PANEL_ID;
#else
    hini.fShowPanelId       = CMIC_HIDE_PANEL_ID;
#endif

    //
    // creating help instance
    //
    hwndHelp = WinCreateHelpInstance(hab, &hini);

    if (!hwndHelp || hini.ulReturnCode)
    {
        log_debug("video.c: WinCreateHelpInstance failed (rc=%d)",
                  hini.ulReturnCode);
        return; // error
    }

    //
    // associate help instance with main frame
    //
    if (!WinAssociateHelpInstance(hwndHelp, hwndFrame))
    {
        log_debug("video.c: WinAssociateInstance failed.");
        return; // error
    }
}

extern int trigger_shutdown;

static void InitMenuBar(canvas_t *c)
{
    //
    // Load popup menu from resource file (this is attached to the frame)
    //
    c->hwndPopupMenu = WinLoadMenu(c->hwndFrame, NULLHANDLE, IDM_VICE2);

    //
    // detach it from the frame window
    //
    WinSetParent(c->hwndPopupMenu, HWND_OBJECT, FALSE);

    //
    // initialize handle to menubar
    //
    c->hwndMenu = WinWindowFromID(c->hwndFrame, FID_MENU);

    //
    // set visibility state of menubar to actual state
    // WM_UPDATEFRAME also hides the loaded popup menu
    // the size is also corrected
    //
    set_menu((void*)menu, (void*)c->hwndClient);
}
/*
void InitStatusBar(canvas_t *c)
{
    HWND hwnd=WinCreateWindow(c->hwndFrame,       // Parent window
                              WC_FRAME,           // Class name
                              NULL,               // Window text
                              0,                  // Frame control
                              border_size_x(), border_size_y(), // Position
                              c->width,
                              statusbar_height(), // Size (width,height)
                              NULLHANDLE,         // Owner window
                              HWND_TOP,           // Sibling window
                              FID_STATUS,         // Window id
                              NULL,               // Control data
                              NULL);              // Pres parameters

    HWND fhwnd = AddStatusFrame(hwnd, 0, 55, TRUE);
    HWND stat  = AddStatusFrame(hwnd, c->width-110, 110, FALSE);

    AddStatusTxt(fhwnd, 0, 55, -1, "Status Bar");

    AddStatusTxt(stat,  0, 55, ID_SPEEDDISP,   "99999%");
    AddStatusTxt(stat, 55, 55, ID_REFRATEDISP, "999fps");

    log_debug("speed3: %x", WinWindowFromID(stat, ID_SPEEDDISP));

    //WinSetDlgText(c->hwndFrame, FID_STATUS, "Test");

    set_status((void*)status, (void*)c->hwndClient);
}
*/
void PM_mainloop(VOID *arg)
{
    APIRET rc;
    SWP    swp;
    HAB    hab;  // Anchor Block to PM
    HMQ    hmq;  // Handle to Msg Queue
    QMSG   qmsg; // Msg Queue Event

    canvas_t *c=(canvas_t *)arg;

    // archdep_setup_signals(0); // signals are not shared between threads!

    hab = WinInitialize(0);            // Initialize PM
    hmq = WinCreateMsgQueue(hab, 0);   // Create Msg Queue

    //
    // 16 Byte Memory (Used eg for the Anchor Blocks)
    //  CS_MOVENOTIFY, CS_SIZEREDRAW skipped
    //  CS_SYNCPAINT: send WM_PAINT messages
    //
    WinRegisterClass(hab, szClientClass, PM_winProc, CS_SYNCPAINT, 0x10);

    //
    // create window on desktop, FIXME: WS_ANIMATE looks sometimes strange
    //
    c->hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_ANIMATE|WS_VISIBLE,
                                      &flFrameFlags, szClientClass,
                                      c->title, 0L, 0, IDM_VICE2,
                                      &(c->hwndClient));

    AddToWindowList(c->hwndClient);

    //
    // get actual window size and position
    //
    WinQueryWindowPos(c->hwndFrame, &swp);

    //
    // set user data pointer
    //
    WinSetWindowPtr(c->hwndClient, QWL_USER, (VOID*)c);

    //
    // initialize menu bar
    //
    InitMenuBar(c);

    //
    // initialize status bar
    //
    // InitStatusBar(c);

    //
    // initialize help system
    //
    InitHelp(c->hwndFrame, IDM_VICE2, "Vice/2 Help",
             "f:\\c64\\src\\vice\\src\\arch\\os2\\doc\\vice2.hlp");

    //
    // bring window to top, set size and position, set focus
    // correct for window height
    //
    WinSetWindowPos(c->hwndFrame, HWND_TOP,
                    swp.x, swp.y+swp.cy-canvas_fullheight(c->height),
                    0, 0,
                    SWP_SHOW|SWP_ZORDER|SWP_ACTIVATE|SWP_MOVE);

    //
    // set visible region notify on, enable visible region
    //
    WinSetVisibleRegionNotify(c->hwndClient, TRUE);
    WinSendMsg(c->hwndClient, WM_VRNENABLED, 0, 0);

    //
    // this makes reactions faster when shutdown of main thread is triggered
    //
    if (rc=DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, +1, 0))
        log_debug("video.c: Error DosSetPriority (rc=%li)", rc);

    //
    // MAINLOOP
    //
    while (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg (hab, &qmsg);

    //
    // make sure, that the state of the VRN doesn't change anymore, don't blit anymore
    //

    // FIXME: The disabeling can be done in WM_CLOSE???
    WinSetVisibleRegionNotify(c->hwndClient, FALSE);
    initialized = FALSE;
    wmVrnDisabled(c->hwndClient);

    log_debug("Window '%s': Quit!", c->title);
    free(c->title);

    /*
     ----------------------------------------------------------
     Is this right, that I couldn't call this for xpet & xcbm2?
     ----------------------------------------------------------
     if (WinDestroyWindow ((*ptr)->hwndFrame))
     */

    //
    // FIXME: For x128 this must be triggered before the Msg Queue is destroyed
    // because sometimes WinDestroyMsgQueue seems to hang. For the second
    // window the msg queue is never destroyed. Is it really necessary
    // to destroy the msg queue?
    //
    log_debug("Triggering Shutdown of Emulation Thread.");
    trigger_shutdown = 1;

    log_debug("Destroying Msg Queue.");
    if (!WinDestroyMsgQueue(hmq))
        log_message(LOG_DEFAULT,"video.c: Error! Destroying Msg Queue.");
    log_debug("Releasing PM Anchor.");
    if (!WinTerminate (hab))
        log_message(LOG_DEFAULT,"video.c: Error! Releasing PM anchor.");

    DosSleep(5000); // wait 5 seconds

    //
    // FIXME, this happens for x128 eg at my laptop when video_close is called
    //
    log_debug("Brutal Exit!");
    //
    // if the triggered shutdown hangs make sure that the program ends
    //
    exit(0);        // end VICE in all cases
}

/* ------------------------------------------------------------------------ */
/* Canvas functions.  */
/* Create a `canvas_t' with tile `win_name', of widht `*width' x `*height'
   pixels, exposure handler callback `exposure_handler' and palette
   `palette'.  If specified width/height is not possible, return an
   alternative in `*width' and `*height'; return the pixel values for the
   requested palette in `pixel_return[]'.  */
canvas_t *canvas_create(const char *title, UINT *width,
                        UINT *height, int mapped, canvas_redraw_t exposure_handler,
                        const palette_t *palette, PIXEL *pixel_return)
{
    canvas_t *canvas_new;

    if (palette->num_entries > 0x10)
    {
        log_error(LOG_DEFAULT, "video.c: Error! More than 16 colors requested for '%s'", title);
        return (canvas_t *) NULL;
    }

    canvas_new = (canvas_t *)xcalloc(1, sizeof(canvas_t));

    canvas_new->title            =  concat(szTitleBarText, " - ", title+6, NULL);
    canvas_new->width            = *width;
    canvas_new->height           = *height;
    canvas_new->stretch          =  stretch;
    canvas_new->palette          =  0;
    canvas_new->exposure_handler =  exposure_handler;
    canvas_new->vrenabled        =  FALSE;

    if (vsid_mode)
        return canvas_new;

    _beginthread(PM_mainloop, NULL, 0x4000, canvas_new);

    while (!canvas_new->vrenabled) // wait until canvas initialized
        DosSleep(1);

    log_debug("video.c: Canvas '%s' (%ix%i) created, hwnd=%x.",
              title, *width, *height, canvas_new->hwndClient);

    canvas_set_palette(canvas_new, palette, pixel_return);

    return canvas_new;
}

void canvas_destroy(canvas_t *c)
{
	/* FIXME: Just a dummy so far */
}


void canvas_map(canvas_t *c)
{
    WinShowWindow(c->hwndFrame, TRUE);
}

void canvas_unmap(canvas_t *c)
{
    WinShowWindow(c->hwndFrame, FALSE);
}

void canvas_resize(canvas_t *c, UINT width, UINT height)
{
    //
    // if this function is called from outside the main thread
    // make sure that the visible region notify is turned off
    // before and back on after.
    //
    SWP swp;

    //
    // if nothing has changed do nothing
    //
    if (c->width==width && c->height==height && c->stretch==stretch)
        return;

    //
    // make anchor left, top corner
    //
    WinQueryWindowPos(c->hwndFrame, &swp);
    if (!WinSetWindowPos(c->hwndFrame, 0,
                         swp.x, swp.y+swp.cy-canvas_fullheight(height),
                         canvas_fullwidth (width),
                         canvas_fullheight(height),
                         SWP_SIZE|SWP_MOVE))
    {
        log_debug("video.c: Error resizing canvas (%ix%i).", width, height);
        return;
    }
    log_debug("video.c: canvas resized (%ix%i --> %ix%i)", c->width, c->height, width, height);

    c->width   = width;
    c->height  = height;
    c->stretch = stretch;

    c->exposure_handler(width, height); // update whole window next time!
}

/* Set the palette of `c' to `p', and return the pixel values in
   `pixel_return[].  */
int canvas_set_palette(canvas_t *c, const palette_t *p, PIXEL *pixel_return)
{
    //
    // FIXME, this sets the palette for both x128 windows...
    //
    int i;
    ULONG rc;
    int offset;

    //
    // number of already used blocks of color data
    //
    static int blocks = 0;

    RGB2 *palette = xcalloc(p->num_entries, sizeof(RGB2));

    //
    // if this is the first or a new block of color data
    //
    if (!blocks || !c->palette)
    {
        if (blocks==0x10)
        {
            log_message(LOG_DEFAULT, "video.c: All 256 palette entries are in use.");
            free(palette);
            return -1;
        }
        blocks++;
        c->palette = blocks;
    }

    //
    // calculate offset for actual block of color data
    //
    offset = (c->palette-1) * 0x10;

    for (i=0; i<p->num_entries; i++)
    {
        palette[i].bRed   = p->entries[i].red;
        palette[i].bGreen = p->entries[i].green;
        palette[i].bBlue  = p->entries[i].blue;

        pixel_return[i] = i + offset;
    }

    rc=DiveSetSourcePalette(hDiveInst, offset, p->num_entries, (BYTE*)palette);
    if (rc)
        log_message(LOG_DEFAULT,"video.c: Error DiveSetSourcePalette (rc=0x%x).",rc);
    else
        log_message(LOG_DEFAULT,"video.c: Palette realized.",rc);

    free(palette);

    return 0;
}

/* ------------------------------------------------------------------------ */
void canvas_refresh(canvas_t *c, video_frame_buffer_t *f,
                    unsigned int xs, unsigned int ys,
                    unsigned int xi, unsigned int yi,
                    unsigned int w,  unsigned int h)
{
    if (DosRequestMutexSem(hmtx, SEM_IMMEDIATE_RETURN) || !hDiveInst || !initialized)
        return;

    DEBUG("CANVAS REFRESH 0");

    if (c->vrenabled)
    {
        //
        // calculate source and destinations
        //
        divesetup.ulSrcWidth  = w;
        divesetup.ulSrcHeight = h;
        divesetup.ulSrcPosX   = xs;
        divesetup.ulSrcPosY   = ys;
        divesetup.ulDstWidth  = w *stretch
#if defined __XVIC__
            *2
#endif
            ;
        divesetup.ulDstHeight = h*stretch;
        divesetup.lDstPosX    = xi*stretch
#if defined __XVIC__
            *2
#endif
            ;
        divesetup.lDstPosY    = (c->height-(yi+h))*stretch;

//        if (status)
//            divesetup.lDstPosY += statusbar_height();

        //
        // now setup the draw areas
        // (all other values are set already by WM_VRNENABLED)
        //
        // Because x128 has two canvases we have
        // to setup the absolute drawing area
        //
        // FIXME: maybe this should only be done when
        //        video cache is enabled
#ifdef __X128__
        wmVrn(c->hwndClient);
#else
        DiveSetupBlitter(hDiveInst, &divesetup);
#endif

        //
        // and blit the image to the screen
        //
        DiveBlitImage(hDiveInst, f->ulBuffer, DIVE_BUFFER_SCREEN);
    }

    DosReleaseMutexSem(hmtx);

    DEBUG("CANVAS REFRESH 1");
};

