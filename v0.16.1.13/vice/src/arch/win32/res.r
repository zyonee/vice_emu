// Common resources.
//
#include "res.h"
#include "config.h"

/////////////////////////////////////////////////////////////////////////////
//
// Icon
//

// Icon with lowest ID value placed first to ensure application icon
// remains consistent on all systems.
IDI_ICON1               ICON    DISCARDABLE     "c64.ico"



//#define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
//#include "afxres.h"

/////////////////////////////////////////////////////////////////////////////
//#undef APSTUDIO_READONLY_SYMBOLS

/////////////////////////////////////////////////////////////////////////////
// English (U.S.) resources

#define DS_MODALFRAME		0x80L
#define LANG_ENGLISH		0x09
#define SUBLANG_ENGLISH_US	0x01
#define WS_POPUP		0x80000000L
#define WS_CAPTION		0x00c00000L
#define WS_VSCROLL		0x00200000L
#define WS_SYSMENU		0x00080000L
#define WS_GROUP		0x00020000L
#define WS_TABSTOP		0x00010000L
#define IDOK			1
#define IDC_STATIC		(-1)
#define BS_AUTOCHECKBOX		0x00000003L
#define BS_AUTORADIOBUTTON	0x00000009L
#define CBS_DROPDOWN		0x0002L
#define CBS_DROPDOWNLIST	0x0003L
#define CBS_SORT		0x0100L

//#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)
//#ifdef _WIN32
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
#pragma code_page(1252)
//#endif //_WIN32

//#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

//1 TEXTINCLUDE DISCARDABLE
//BEGIN
//    "resource.h\0"
//END

//2 TEXTINCLUDE DISCARDABLE
//BEGIN
//    "#include ""afxres.h""\r\n"
//    "\0"
//END

//3 TEXTINCLUDE DISCARDABLE
//BEGIN
//    "\r\n"
//    "\0"
//END

//#endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// Menu
//

// The menus are machine specific.


/////////////////////////////////////////////////////////////////////////////
//
// Dialog
//

IDD_ABOUT DIALOG DISCARDABLE  0, 0, 195, 137
STYLE DS_MODALFRAME | WS_CAPTION | WS_SYSMENU
CAPTION "About VICE"
FONT 8, "MS Sans Serif"
BEGIN
    DEFPUSHBUTTON "OK",IDOK,132,14,50,14
    CTEXT    "VICE",IDC_STATIC,7,5,118,8
    CTEXT    "Versatile Commodore Emulator",IDC_STATIC,7,18,118,8
    CTEXT    "Preview " VERSION "- ALPHA version",IDC_STATIC,7,31,118,8
    CTEXT    "Copyright (c) 1996-1999 Ettore Perazzoli",IDC_STATIC,16,50,158,8
    CTEXT    "Copyright (c) 1996-1999 Andr� Fachat",IDC_STATIC,16,59,158,8
    CTEXT    "Copyright (c) 1993-1994, 1997-1999 Teemu Rantanen",IDC_STATIC,5,68,180,8
    CTEXT    "Copyright (c) 1997-1999 Daniel Sladic",IDC_STATIC,16,77,158,8
    CTEXT    "Copyright (c) 1998-1999 Andreas Boose",IDC_STATIC,36,86,118,8
    CTEXT    "Copyright (c) 1998-1999 Dag Lem",IDC_STATIC,36,95,118,8
    CTEXT    "Copyright (c) 1998-1999 Tibor Biczo",IDC_STATIC,16,104,158,8
    CTEXT    "Copyright (c) 1993-1996 Jouko Valta",IDC_STATIC,36,113,118,8
    CTEXT    "Copyright (c) 1993-1994 Jarkko Sonninen",IDC_STATIC,16,122,158,8
END

IDD_DISKDEVICE_DIALOG DIALOG DISCARDABLE  0, 0, 307, 190
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif"
BEGIN
    AUTORADIOBUTTON "Disk &image",IDC_SELECTDISK,15,20,51,10,BS_AUTORADIOBUTTON |
                    WS_GROUP | WS_TABSTOP
    AUTORADIOBUTTON "&Directory",IDC_SELECTDIR,15,75,44,10,BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "&None",IDC_SELECTNONE,15,160,33,10,BS_AUTORADIOBUTTON
    COMBOBOX        IDC_DISKIMAGE,81,19,210,12,CBS_DROPDOWN | CBS_SORT |
                    WS_VSCROLL | WS_GROUP | WS_TABSTOP
    PUSHBUTTON      "&Browse...",IDC_BROWSEDISK,81,37,50,14,WS_GROUP
    PUSHBUTTON      "&Autostart",IDC_AUTOSTART,241,37,50,14,WS_GROUP
    COMBOBOX        IDC_DIR,81,74,210,12,CBS_DROPDOWN | CBS_SORT |
                    WS_VSCROLL | WS_GROUP | WS_TABSTOP
    PUSHBUTTON      "&Browse...",IDC_BROWSEDIR,81,92,50,14,WS_GROUP
    GROUPBOX        "&Options",IDC_STATIC,195,90,95,63
    AUTOCHECKBOX    "&Read P00 files",IDC_TOGGLE_READP00,205,105,62,10,
                    BS_AUTOCHECKBOX | WS_GROUP | WS_TABSTOP
    AUTOCHECKBOX    "&Write P00 files",IDC_TOGGLE_WRITEP00,205,120,61,10,
                    BS_AUTOCHECKBOX | WS_TABSTOP
    AUTOCHECKBOX    "&Hide non-P00 files",IDC_TOGGLE_HIDENONP00,205,135,73,10,
                    BS_AUTOCHECKBOX | WS_TABSTOP
END

IDD_DRIVE_SETTINGS_DIALOG DIALOG DISCARDABLE  0, 0, 250, 120
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif"
BEGIN
    GROUPBOX        "&Drive type",IDC_STATIC,5,15,60,90
    AUTORADIOBUTTON "1541",IDC_SELECT_DRIVE_TYPE_1541,15,30,35,10,
                            BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP
    AUTORADIOBUTTON "1571",IDC_SELECT_DRIVE_TYPE_1571,15,45,35,10,
                            BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "1581",IDC_SELECT_DRIVE_TYPE_1581,15,60,35,10,
                            BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "2031",IDC_SELECT_DRIVE_TYPE_2031,15,75,35,10,
                            BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "None",IDC_SELECT_DRIVE_TYPE_NONE,15,90,35,10,
                            BS_AUTORADIOBUTTON
    GROUPBOX        "&40 track handling",IDC_STATIC,75,15,90,60
    AUTORADIOBUTTON "Never extend",IDC_SELECT_DRIVE_EXTEND_NEVER,85,30,65,10,
                            BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP
    AUTORADIOBUTTON "Ask on extend",IDC_SELECT_DRIVE_EXTEND_ASK,85,45,65,10,
                            BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "Extend on access",IDC_SELECT_DRIVE_EXTEND_ACCESS,
                            85,60,65,10, BS_AUTORADIOBUTTON
    GROUPBOX        "&Idle method",IDC_STATIC,175,15,70,60
    AUTORADIOBUTTON "None",IDC_SELECT_DRIVE_IDLE_NO_IDLE,185,30,51,10,
                            BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP
    AUTORADIOBUTTON "Trap idle",IDC_SELECT_DRIVE_IDLE_TRAP_IDLE,185,45,51,10,
                            BS_AUTORADIOBUTTON
    AUTORADIOBUTTON "Skip cycles",IDC_SELECT_DRIVE_IDLE_SKIP_CYCLES,
                            185,60,51,10,BS_AUTORADIOBUTTON
    AUTOCHECKBOX    "Enable &parallel cable",IDC_TOGGLE_DRIVE_PARALLEL_CABLE,
                    85,90,80,10, BS_AUTOCHECKBOX | WS_GROUP | WS_TABSTOP
END

IDD_SNAPSHOT_SAVE_DIALOG DIALOG DISCARDABLE  0, 0, 180, 108
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif"
BEGIN
    COMBOBOX        IDC_SNAPSHOT_SAVE_IMAGE,5,15,170,12,CBS_DROPDOWN |
                    CBS_SORT | WS_VSCROLL | WS_GROUP | WS_TABSTOP
    GROUPBOX        "&Options",IDC_STATIC,5,35,150,45
    AUTOCHECKBOX    "Save currently attached &disks images",
                    IDC_TOGGLE_SNAPSHOT_SAVE_DISKS,
                    15,50,135,10, BS_AUTOCHECKBOX | WS_GROUP | WS_TABSTOP
    AUTOCHECKBOX    "Save currently loaded &ROM images",
                    IDC_TOGGLE_SNAPSHOT_SAVE_ROMS,
                    15,65,135,10, BS_AUTOCHECKBOX | WS_TABSTOP
    DEFPUSHBUTTON   "OK", IDOK, 70,90,50,14, WS_TABSTOP
    PUSHBUTTON      "Cancel", IDC_CANCEL, 125,90,50,14, WS_TABSTOP

END

IDD_JOY_SETTINGS_DIALOG DIALOG DISCARDABLE 0, 0, 147, 87
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Joystick settings"
FONT 8, "MS Sans Serif"
BEGIN
    LTEXT           "Joystick in port #1 :",IDC_STATIC,7,5,118,8
    LTEXT           "Joystick in port #2 :",IDC_STATIC,7,18,118,8
    COMBOBOX        IDC_JOY_DEV1,70,2,70,68,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    COMBOBOX        IDC_JOY_DEV2,70,15,70,68,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    PUSHBUTTON      "Config Keyset A", IDC_JOY_CONFIG_A, 9,35,60,14, WS_TABSTOP
    PUSHBUTTON      "Config Keyset B", IDC_JOY_CONFIG_B, 77,35,60,14, WS_TABSTOP
    PUSHBUTTON      "Calibrate", IDC_JOY_CALIBRATE, 44,52,50,14, WS_TABSTOP
    DEFPUSHBUTTON   "OK", IDOK, 14,68,50,14, WS_TABSTOP
    PUSHBUTTON      "Cancel", IDC_CANCEL, 82,68,50,14, WS_TABSTOP
END

IDD_CONFIG_KEYSET_DIALOG DIALOG DISCARDABLE 0, 0, 130, 190
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif"
BEGIN
    PUSHBUTTON      "South West",IDC_KEYSET_SW,7,5,60,14, WS_TABSTOP
    PUSHBUTTON      "South",IDC_KEYSET_S,7,23,60,14, WS_TABSTOP
    PUSHBUTTON      "South East",IDC_KEYSET_SE,7,41,60,14, WS_TABSTOP
    PUSHBUTTON      "West",IDC_KEYSET_W,7,59,60,14, WS_TABSTOP
    PUSHBUTTON      "East",IDC_KEYSET_E,7,77,60,14, WS_TABSTOP
    PUSHBUTTON      "North West",IDC_KEYSET_NW,7,95,60,14, WS_TABSTOP
    PUSHBUTTON      "North",IDC_KEYSET_N,7,113,60,14, WS_TABSTOP
    PUSHBUTTON      "North East",IDC_KEYSET_NE,7,131,60,14, WS_TABSTOP
    PUSHBUTTON      "Fire button",IDC_KEYSET_FIRE,7,149,60,14,WS_TABSTOP
    LTEXT           "",IDC_KEY_SW,80,8,100,8
    LTEXT           "",IDC_KEY_S,80,26,100,8
    LTEXT           "",IDC_KEY_SE,80,44,100,8
    LTEXT           "",IDC_KEY_W,80,62,100,8
    LTEXT           "",IDC_KEY_E,80,80,100,8
    LTEXT           "",IDC_KEY_NW,80,98,100,8
    LTEXT           "",IDC_KEY_N,80,116,100,8
    LTEXT           "",IDC_KEY_NE,80,134,100,8
    LTEXT           "",IDC_KEY_FIRE,80,152,100,8
    DEFPUSHBUTTON   "OK", IDOK, 15,170,50,14, WS_TABSTOP
    PUSHBUTTON      "Cancel", IDC_CANCEL, 70,170,50,14, WS_TABSTOP
END

IDD_CONFIG_KEY_DIALOG DIALOG DISCARDABLE 0, 0, 120, 24
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif"
BEGIN
END

IDD_SOUND_SETTINGS_DIALOG DIALOG DISCARDABLE 0, 0, 180, 80
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Configure Sound Device Settings"
FONT 8, "MS Sans Serif"
BEGIN
    LTEXT           "Sample Frequency :",IDC_STATIC,7,5,118,8
    LTEXT           "Sound Buffer Size :",IDC_STATIC,7,18,118,8
    LTEXT           "Oversampling Factor :",IDC_STATIC,7,31,118,8
    LTEXT           "Synchronization Method :",IDC_STATIC,7,44,118,8
    COMBOBOX        IDC_SOUND_FREQ,100,2,70,68,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    COMBOBOX        IDC_SOUND_BUFFER,100,15,70,76,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    COMBOBOX        IDC_SOUND_OVERSAMPLE,100,28,70,68,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    COMBOBOX        IDC_SOUND_SYNCH,100,41,70,68,CBS_DROPDOWNLIST |
                    WS_GROUP | WS_TABSTOP
    DEFPUSHBUTTON   "OK", IDOK, 40,60,50,14, WS_TABSTOP
    PUSHBUTTON      "Cancel", IDC_CANCEL, 95,60,50,14, WS_TABSTOP
END

/////////////////////////////////////////////////////////////////////////////
//
// DESIGNINFO
//

//#ifdef APSTUDIO_INVOKED
//GUIDELINES DESIGNINFO DISCARDABLE
//BEGIN
//    IDD_ABOUT, DIALOG
//    BEGIN
//        LEFTMARGIN, 6
//        RIGHTMARGIN, 184
//        TOPMARGIN, 7
//        BOTTOMMARGIN, 81
//    END

//    IDD_DISKDEVICE_DIALOG, DIALOG
//    BEGIN
//        LEFTMARGIN, 7
//        RIGHTMARGIN, 300
//        TOPMARGIN, 7
//        BOTTOMMARGIN, 203
//    END
//END
//#endif    // APSTUDIO_INVOKED

//#endif    // English (U.S.) resources
/////////////////////////////////////////////////////////////////////////////



//#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
//#endif    // not APSTUDIO_INVOKED
