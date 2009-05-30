SDL port of VICE
================


    Contents
    --------

    1. General info
    2. Usage
    3. Building


1. General info
===============

1.1 Goal

The SDL port is meant to be an easily portable version of VICE
that is fully usable with just a joystick (with at least 2 buttons)
or a keyboard. OS/arch-specific features (such as RS-232 support)
may be omitted for portability. Everything is configurable via the UI.


1.2 Features

Some new features that are missing from (some of) the native versions:
 - Free scaling using OpenGL (from the GTK port) with optional fixed aspect ratio
 - Virtual keyboard (adapted from the GP2X port)
 - Hotkey mapping to any menu item
 - (Host) joystick event mapping to (machine) joystick, keyboard or menu item
 - No mouse or keyboard required, but both are supported

Some missing features that are available in (some of) the native versions:
 - RS-232 support
 - vsid support


1.3 Ports

The SDL port has been tested to work on:
 - AIX / rs6000
 - Amiga OS 4.x / ppc
 - AROS / amd64/x86_64, ppc, x86
 - BeOS / ppc, x86
 - BSDi / x86
 - DragonflyBSD / x86
 - FreeBSD / alpha, amd64/x86_64, ia64, ppc, sparc64, x86
 - GP2X / arm (joystick unusable)
 - Hurd / x86
 - Linux / alpha, amd64/x86_64, arm, armeb, avr32, hppa, ia64, m68k, mips,
           mipsel, mips64, mips64el, ppc, ppc64, s390, s390x, sh3, sh4, sparc,
           sparc64, x86
 - MorphOS / ppc
 - NetBSD / alpha, amd64/x86_64, arm, hppa, m68010 (libaa only), m68k, mipseb,
            mipsel, ns32k, ppc, sh3eb, sh3le, sparc, sparc64, vax, x86
 - OpenBSD / alpha, amd64/x86_64, arm, hppa, ppc, sh4, sparc64, x86
 - OpenServer / x86
 - QNX 6.x / armle, mipsle, ppcbe, shle, x86
 - SkyOS / x86
 - Solaris / amd64/x86_64, sparc, sparc64, x86
 - SunOS / m68k, sparc
 - Syllable / x86
 - Tru64 4.x / alpha
 - uClinux / m68000
 - Ultrix / mipsel, vax
 - UnixWare / x86
 - Win32 / x86
 - Win64 / ia64, x64
 - Zaurus (qt) / arm
 - ...


2. Usage
========

2.1 The menu

The menu is used with the following commands:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Activate | F12   | btn2 | Activate the menu
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Cancel/Page up
Right    | right |  r   | Enter menu/Page down
Select   | enter | fire | Select the item
Cancel   | <-    | btn2 | Return to previous menu
Exit     | ESC   | N/A  | Exit the menu
Map      | m     | btn3 | Map a hotkey (see 2.2)
--------------------------------------------------

The keys and joystick events can be configured via the menu.

Left/Right work as Page up/down on the file selector, otherwise
left does Cancel and right enters the selected submenu.

The joystick command Activate behaves as Cancel while in the menu.


2.2 Hotkeys

By default, the SDL port doesn't have any hotkeys defined.
Mapping a hotkey is simple:

1. Navigate the menu to an item
2. Issue the Map command (default: 'm', button 3)
3. Press the desired key(-combo) or joystick direction/button

The keycombo can use multiple modifiers, for example Ctrl+q and
Ctrl+Shift+q can be mapped to different menu entries. Note that
the "left" and "right" versions of a modifier are regarded as the
same key in the context of hotkeys.

Hotkeys can be unmapped by mapping the hotkey to an empty menu item.

Hotkeys do not work while using the menu or virtual keyboard.


2.3 Virtual keyboard

The menu commands are also used in the virtual keyboard:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Move cursor left
Right    | right |  r   | Move cursor right
Select   | enter | fire | Press/release the key
Cancel   | <-    | btn2 | Press/release with shift
Exit     | ESC   | N/A  | Close the virtual kbd
Map      | m     | btn3 | Map a key/button
--------------------------------------------------

Note that pressing a key and releasing the key generate separate events.
This means that pressing Select on a key, moving to an another key and
releasing Select releases the latter key; with this, multiple keys
can be pressed down at once.

The joystick command Activate behaves as Cancel while using the virtual
keyboard.

The virtual keyboard can be moved by pressing (and holding) Select on an
empty space and moving the cursor.

The virtual keyboard can be closed by pressing the 'X' in the top left corner,
with the command Exit or with the command Cancel when the cursor is at an
empty spot.

Keys and joystick events can be mapped to the keyboard via the virtual keyboard.


2.4 Text input dialog virtual keyboard

The text input dialog also has a virtual keyboard, which can be activated
with the key F10 or joystick commands Cancel or Map.

When the virtual keyboard is active, the following commands are active:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Move cursor left
Right    | right |  r   | Move cursor right
Select   | enter | fire | Press the key
Cancel   | <-    | btn2 | Press with shift
Exit     | ESC   | N/A  | Close the virtual kbd
Map      | m     | btn3 | Close the virtual kbd
--------------------------------------------------

The joystick command Activate behaves as Cancel while using the virtual
keyboard.

Simultaneous keypresses are not possible. The virtual keyboard cannot be moved.

The virtual keyboard can be closed by pressing the 'X' in the top left corner,
with the commands Exit and Map or with the command Cancel when the cursor is at an
empty spot.

To exit/cancel the dialog itself, press "esc" on the virtual keyboard.

Note that normal text input via keyboard is not possible while the virtual
keyboard is active.


2.5 Settings

The settings are saved separately into 4 files:
 - main settings (sdl-vicerc, sdl-vice.ini, "Load/Save settings")
 - fliplist (...)
 - hotkey (sdl-hotkey-MACHINE.vkm, "Load/Save hotkeys")
 - joymap (sdl-joymap-MACHINE.vjm, "Load/Save joystick map")

Remember to save the relevant settings file.


3. Building
===========

3.1 Building in *nix compile enviroments

./configure --enable-sdlui
make
make install

You'll need the SDL libs and headers. For free scaling, the OpenGL is
also needed (libGL, opengl32.dll, ...)


3.2 Building in Visual Studio

For MSVC building instructions see src/arch/sdl/win32-msvc/Readme.txt
