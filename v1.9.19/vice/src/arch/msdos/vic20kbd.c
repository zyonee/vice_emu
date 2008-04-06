/*
 * vic20kbd.c -- VIC20 keyboard handling for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef COMMON_KBD

#include "kbd.h"

static keyconv vic20_keyboard[256] = {
    { -1, -1, 0 },		/*           (no key)           */
    { -1, -1, 0 },		/*          ESC -> (no key)     */
    { 0, 0, 0 },		/*            1 -> 1            */
    { 0, 7, 0 },		/*            2 -> 2            */
    { 1, 0, 0 },		/*            3 -> 3            */
    { 1, 7, 0 },		/*            4 -> 4            */
    { 2, 0, 0 },		/*            5 -> 5            */
    { 2, 7, 0 },		/*            6 -> 6            */
    { 3, 0, 0 },		/*            7 -> 7            */
    { 3, 7, 0 },		/*            8 -> 8            */
    { 4, 0, 0 },		/*            9 -> 9            */
    { 4, 7, 0 },		/*            0 -> 0            */
    { 5, 0, 0 },		/*        Minus -> Plus         */
    { 5, 7, 0 },		/*        Equal -> Minus        */
    { 7, 0, 0 },		/*    Backspace -> Del          */
    { 0, 2, 0 },		/*          TAB -> Ctrl         */
    { 0, 6, 0 },		/*            Q -> Q            */
    { 1, 1, 0 },		/*            W -> W            */
    { 1, 6, 0 },		/*            E -> E            */
    { 2, 1, 0 },		/*            R -> R            */
    { 2, 6, 0 },		/*            T -> T            */
    { 3, 1, 0 },		/*            Y -> Y            */
    { 3, 6, 0 },		/*            U -> U            */
    { 4, 1, 0 },		/*            I -> I            */
    { 4, 6, 0 },		/*            O -> O            */
    { 5, 1, 0 },		/*            p -> P            */
    { 5, 6, 0 },		/*            [ -> @            */
    { 6, 1, 0 },		/*            ] -> *            */
    { 7, 1, 0 },		/*       Return -> Return       */
    { 0, 5, 0 },		/*    Left Ctrl -> CBM          */
    { 1, 2, 0 },		/*            A -> A            */
    { 1, 5, 0 },		/*            S -> S            */
    { 2, 2, 0 },		/*            D -> D            */
    { 2, 5, 0 },		/*            F -> F            */
    { 3, 2, 0 },		/*            G -> G            */
    { 3, 5, 0 },		/*            H -> H            */
    { 4, 2, 0 },		/*            J -> J            */
    { 4, 5, 0 },		/*            K -> K            */
    { 5, 2, 0 },		/*            L -> L            */
    { 5, 5, 0 },		/*            ; -> :            */
    { 6, 2, 0 },		/*            ' -> ;            */
    { 0, 1, 0 },		/*            ` -> Left Arrow   */
    { 1, 3, 1 },		/*   Left Shift -> Left Shift   */
    { 6, 5, 0 },		/*            \ -> =	        */
    { 1, 4, 0 },		/*            Z -> Z            */
    { 2, 3, 0 },		/*            X -> X            */
    { 2, 4, 0 },		/*            C -> C            */
    { 3, 3, 0 },		/*            V -> V            */
    { 3, 4, 0 },		/*            B -> B            */
    { 4, 3, 0 },		/*            N -> N            */
    { 4, 4, 0 },		/*            M -> M            */
    { 5, 3, 0 },		/*            , -> ,            */
    { 5, 4, 0 },		/*            . -> .            */
    { 6, 3, 0 },		/*            / -> /            */
    { 6, 4, 0 },		/*  Right Shift -> Right Shift  */
    { 6, 1, 0 },		/*       Grey * -> *            */
    { -1, -1, 0 },		/*     Left Alt -> (no key)     */
    { 0, 4, 0 },		/*        Space -> Space        */
    { 0, 3, 0 },		/*    Caps Lock -> Run/Stop     */
    { 7, 4, 0 },		/*           F1 -> F1           */
    { 7, 4, 1 },		/*           F2 -> F2           */
    { 7, 5, 0 },		/*           F3 -> F3           */
    { 7, 5, 1 },		/*           F4 -> F4           */
    { 7, 6, 0 },		/*           F5 -> F5           */
    { 7, 6, 1 },		/*           F6 -> F6           */
    { 7, 7, 0 },		/*           F7 -> F7           */
    { 7, 7, 1 },		/*           F8 -> F8           */
    { -1, -1, 0 },		/*           F9 -> (no key)     */
    { -1, -1, 0 },		/*          F10 -> (no key)	*/
    { -1, -1, 0 },		/*     Num Lock -> (no key)	*/
    { -1, -1, 0 },		/*  Scroll Lock -> (no key)	*/
    { -1, -1, 0 },		/*     Numpad 7 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 8 -> (no key)	*/
    { -1, -1, 0 },		/*     Numpad 9 -> (no key)	*/
    { -1, -1, 0 },		/*     Numpad - -> (no key)	*/
    { -1, -1, 0 },		/*     Numpad 4 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 5 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 6 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad + -> (no key)	*/
    { -1, -1, 0 },		/*     Numpad 1 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 2 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 3 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad 0 -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad . -> (no key) 	*/
    { -1, -1, 0 },		/*       SysReq -> (no key) 	*/
    { -1, -1, 0 },		/*           85 -> (no key) 	*/
    { -1, -1, 0 },		/*           86 -> (no key) 	*/
    { -1, -1, 0 },		/*          F11 -> (no key) 	*/
    { -1, -1, 0 },		/*          F12 -> (no key) 	*/
    { 6, 7, 0 },		/*         Home -> CLR/HOME 	*/
    { 7, 3, 1 },		/*           Up -> CRSR UP 	*/
    { -1, -1, 0 },		/*         PgUp -> (no key) 	*/
    { 7, 2, 1 },		/*         Left -> CRSR LEFT 	*/
    { 7, 2, 0 },		/*        Right -> CRSR RIGHT 	*/
    { -1, -1, 0 },		/*          End -> (no key) 	*/
    { 7, 3, 0 },		/*         Down -> CRSR DOWN 	*/
    { -1, -1, 0 },		/*       PgDown -> (no key) 	*/
    { 6, 0, 0 },		/*          Ins -> Pound 	*/
    { 6, 6, 0 },		/*          Del -> Up Arrow 	*/
    { -1, -1, 0 },		/* Numpad Enter -> (no key) 	*/
    { -1, -1, 0 },		/*   Right Ctrl -> (no key) 	*/
    { -1, -1, 0 },		/*        Pause -> (no key) 	*/
    { -1, -1, 0 },		/*       PrtScr -> (no key) 	*/
    { -1, -1, 0 },		/*     Numpad / -> (no key) 	*/
    { -1, -1, 0 },		/*    Right Alt -> (no key) 	*/
    { -1, -1, 0 },		/*        Break -> (no key) 	*/
    { -1, -1, 0 },		/*   Left Win95 -> (no key) 	*/
    { -1, -1, 0 }		/*  Right Win95 -> (no key) 	*/
};

int vic20_kbd_init(void)
{
    return kbd_init(1,
                    1, 3, vic20_keyboard, sizeof(vic20_keyboard));
}
#endif

