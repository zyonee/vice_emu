/*
 * c610kbd.c - C610 keyboard implementation.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "kbd.h"

static keyconv c610_keyboard[256] = {
    { -1, -1, 0 },              /*           (no key)           */
    { 7, 0, 0 },                /*          ESC -> Run/Stop     */
    { 8, 0, 0 },                /*           F1 -> F1           */
    { 9, 0, 0 },                /*           F2 -> F2           */
    { 10, 0, 0 },               /*           F3 -> F3           */
    { 11, 0, 0 },               /*           F4 -> F4           */
    { 12, 0, 0 },               /*           F5 -> F5           */
    { 13, 0, 0 },               /*           F6 -> F6           */
    { 14, 0, 0 },               /*           F7 -> F7           */
    { 15, 0, 0 },               /*           F8 -> F8           */
    { 0, 0, 0 },                /*           F9 -> F9           */
    { 1, 0, 0 },                /*          F10 -> F10          */
    { -1, -1, 0 },              /*          F11 -> (no key)     */
    { -1, -1, 0 },              /*          F12 -> (no key)     */
    { -1, -1, 0 },              /*       PrtScr -> (no key)     */
    { -1, -1, 0 },              /*  Scroll Lock -> (no key)     */
    { -1, -1, 0 },              /*           16 -> (no key)     */
    { 8, 1, 0 },                /*            ` -> ESC          */
    { 9, 1, 0 },                /*            1 -> 1            */
    { 10, 1, 0 },               /*            2 -> 2            */
    { 11, 1, 0 },               /*            3 -> 3            */
    { 12, 1, 0 },               /*            4 -> 4            */
    { 13, 1, 0 },               /*            5 -> 5            */
    { 13, 2, 0 },               /*            6 -> 6            */
    { 14, 1, 0 },               /*            7 -> 7            */
    { 15, 1, 0 },               /*            8 -> 8            */
    { 0, 1, 0 },                /*            9 -> 9            */
    { 1, 1, 0 },                /*            0 -> 0            */
    { 1, 2, 0 },                /*        Minus -> Minus        */
    { 2, 1, 0 },                /*        Equal -> Equal        */
    { 3, 3, 0 },                /*    Backspace -> Del          */
    { 2, 2, 0 },                /*          Ins -> Pound        */
    { 4, 0, 0 },                /*         Home -> CLR/HOME     */
    { 5, 0, 0 },                /*         PgUp -> Rev/Off      */
    { -1, -1, 0 },              /*     Num Lock -> (no key)     */
    { 7, 1, 0 },                /*     Numpad / -> Numpad /     */
    { 6, 1, 0 },                /*     Numpad * -> *            */
    { 7, 2, 0 },                /*     Numpad - -> Numpad -     */
    { 8, 2, 0 },                /*          TAB -> TAB          */
    { 9, 2, 0 },                /*            Q -> Q            */
    { 10, 2, 0 },               /*            W -> W            */
    { 11, 2, 0 },               /*            E -> E            */
    { 12, 2, 0 },               /*            R -> R            */
    { 12, 3, 0 },               /*            T -> T            */
    { 13, 3, 0 },               /*            Y -> Y            */
    { 14, 2, 0 },               /*            U -> U            */
    { 15, 2, 0 },               /*            I -> I            */
    { 0, 2, 0 },                /*            O -> O            */
    { 1, 3, 0 },                /*            P -> P            */
    { 1, 4, 0 },                /*            [ -> [            */
    { 2, 3, 0 },                /*            ] -> ]            */
    { -1, -1, 0 },              /*            \ -> (no key)     */
    { 4, 1, 0 },                /*          Del -> ?            */
    { 5, 1, 0 },                /*          End -> CE           */
    { 6, 0, 0 },                /*       PgDown -> Norm/Graph   */
    { 4, 2, 0 },                /*     Numpad 7 -> Numpad 7     */
    { 5, 2, 0 },                /*     Numpad 8 -> Numpad 8     */
    { 6, 2, 0 },                /*     Numpad 9 -> Numpad 9     */
    { 7, 3, 0 },                /*     Numpad + -> Numpad +     */
    { 7, 0, 0 },                /*    Caps Lock -> Run/Stop     */
    { 9, 3, 0 },                /*            A -> A            */
    { 10, 3, 0 },               /*            S -> S            */
    { 11, 3, 0 },               /*            D -> D            */
    { 11, 4, 0 },               /*            F -> F            */
    { 12, 4, 0 },               /*            G -> G            */
    { 13, 4, 0 },               /*            H -> H            */
    { 14, 3, 0 },               /*            J -> J            */
    { 15, 3, 0 },               /*            K -> K            */
    { 0, 3, 0 },                /*            L -> L            */
    { 0, 4, 0 },                /*            ; -> ;            */
    { 1, 5, 0 },                /*            ' -> '            */
    { 2, 4, 0 },                /*       Return -> Return       */
    { 4, 3, 0 },                /*     Numpad 4 -> Numpad 4     */
    { 5, 3, 0 },                /*     Numpad 5 -> Numpad 5     */
    { 6, 3, 0 },                /*     Numpad 6 -> Numpad 6     */
    { 8, 4, 1 },                /*   Left Shift -> Left Shift   */
    { 9, 4, 0 },                /*            Z -> Z            */
    { 10, 4, 0 },               /*            X -> X            */
    { 10, 5, 0 },               /*            C -> C            */
    { 11, 5, 0 },               /*            V -> V            */
    { 12, 5, 0 },               /*            B -> B            */
    { 13, 5, 0 },               /*            N -> N            */
    { 14, 4, 0 },               /*            M -> M            */
    { 15, 4, 0 },               /*            , -> ,            */
    { 15, 5, 0 },               /*            . -> .            */
    { 0, 5, 0 },                /*            / -> /            */
    { 8, 3, 0 },                /*  Right Shift -> Right Shift  */
    { 3, 0, 0 },                /*           Up -> CRSR UP      */
    { 4, 4, 0 },                /*     Numpad 1 -> Numpad 1     */
    { 5, 4, 0 },                /*     Numpad 2 -> Numpad 2     */
    { 6, 4, 0 },                /*     Numpad 3 -> Numpad 3     */
    { 7, 4, 0 },                /* Numpad Enter -> Numpad Enter */
    { 8, 5, 0 },                /*    Left Ctrl -> CTRL         */
    { -1, -1, 0 },              /*     Left Alt -> (no key)     */
    { 14, 5, 0 },               /*        Space -> Space        */
    { -1, -1, 0 },              /*    Right Alt -> (no key)     */
    { 3, 4, 0 },                /*   Right Ctrl -> C=           */
    { 3, 1, 0 },                /*         Left -> CRSR LEFT    */
    { 2, 0, 0 },                /*         Down -> CRSR DOWN    */
    { 3, 2, 0 },                /*        Right -> CRSR RIGHT   */
    { 4, 5, 0 },                /*     Numpad 0 -> Numpad 0     */
    { 5, 5, 0 },                /*     Numpad . -> Numpad .     */
    { 2, 5, 0 },                /*   Left Win95 -> Pi           */
    { -1, -1, 0 },              /*  Right Win95 -> (no key)     */
    { -1, -1, 0 },              /*       SysReq -> (no key)     */
};

int c610_kbd_init(void)
{
    return kbd_init(1, 8, 4, c610_keyboard, sizeof(c610_keyboard));
}
