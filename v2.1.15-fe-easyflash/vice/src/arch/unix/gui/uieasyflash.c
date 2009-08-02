/*
 * uieasyflash.h
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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
#include "types.h"

#include <stdio.h>

#include "easyflash.h"
#include "uilib.h"
#include "uimenu.h"
#include "uieasyflash.h"


UI_MENU_DEFINE_TOGGLE(EasyFlashJumper)
UI_MENU_DEFINE_TOGGLE(EasyFlashWriteCRT)

static UI_CALLBACK(easyflash_save_callback)
{
    easyflash_save_crt();
}

ui_menu_entry_t easyflash_submenu[] = {
    { N_("*Jumper"), (ui_callback_t)toggle_EasyFlashJumper,
      NULL, NULL },
    { N_("*Save CRT on detach"), (ui_callback_t)toggle_EasyFlashWriteCRT,
      NULL, NULL },
    { N_("Save CRT now"),
      (ui_callback_t)easyflash_save_callback, NULL, NULL },
    { NULL }
};

