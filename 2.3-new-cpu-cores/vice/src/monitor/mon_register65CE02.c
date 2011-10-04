/*
 * mon_register65CE02.c - The VICE built-in monitor 65CE02 register functions.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "lib.h"
#include "log.h"
#include "mon_register.h"
#include "mon_util.h"
#include "montypes.h"
#include "mos65CE02.h"
#include "uimon.h"


#define TEST(x) ((x)!=0)

static unsigned int mon_register_get_val(int mem, int reg_id)
{
    mos65CE02_regs_t *reg_ptr;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return 0;
        }
    }

    reg_ptr = mon_interfaces[mem]->cpu_65CE02_regs;

    switch(reg_id) {
      case e_A:
        return MOS65CE02_REGS_GET_A(reg_ptr);
      case e_X:
        return MOS65CE02_REGS_GET_X(reg_ptr);
      case e_Y:
        return MOS65CE02_REGS_GET_Y(reg_ptr);
      case e_Z:
        return MOS65CE02_REGS_GET_Z(reg_ptr);
      case e_PC:
        return MOS65CE02_REGS_GET_PC(reg_ptr);
      case e_SP:
        return MOS65CE02_REGS_GET_SP(reg_ptr);
      case e_BP:
        return MOS65CE02_REGS_GET_BP(reg_ptr);
      case e_FLAGS:
          return MOS65CE02_REGS_GET_FLAGS(reg_ptr)
              | MOS65CE02_REGS_GET_SIGN(reg_ptr)
              | (MOS65CE02_REGS_GET_ZERO(reg_ptr) << 1);
      default:
        log_error(LOG_ERR, "Unknown register!");
    }
    return 0;
}

static void mon_register_set_val(int mem, int reg_id, WORD val)
{
    mos65CE02_regs_t *reg_ptr;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return;
        }
    }

    reg_ptr = mon_interfaces[mem]->cpu_65CE02_regs;

    switch(reg_id) {
      case e_A:
        MOS65CE02_REGS_SET_A(reg_ptr, (BYTE)val);
        break;
      case e_X:
        MOS65CE02_REGS_SET_X(reg_ptr, (BYTE)val);
        break;
      case e_Y:
        MOS65CE02_REGS_SET_Y(reg_ptr, (BYTE)val);
        break;
      case e_Z:
        MOS65CE02_REGS_SET_Z(reg_ptr, (BYTE)val);
        break;
      case e_PC:
        MOS65CE02_REGS_SET_PC(reg_ptr, val);
        if (monitor_diskspace_dnr(mem) >= 0) {
            mon_interfaces[mem]->set_bank_base(mon_interfaces[mem]->context);
        }
        break;
      case e_SP:
        MOS65CE02_REGS_SET_SP(reg_ptr, (WORD)val);
        break;
      case e_BP:
        MOS65CE02_REGS_SET_BP(reg_ptr, (BYTE)val);
        break;
      case e_FLAGS:
        MOS65CE02_REGS_SET_STATUS(reg_ptr, (BYTE)val);
        break;
      default:
        log_error(LOG_ERR, "Unknown register!");
        return;
    }
    force_array[mem] = 1;
}

static void mon_register_print(int mem)
{
    mos65CE02_regs_t *regs;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return;
        }
    } else if (mem != e_comp_space) {
        log_error(LOG_ERR, "Unknown memory space!");
        return;
    }

    regs = mon_interfaces[mem]->cpu_65CE02_regs;

    mon_out("  ADDR AC XR YR ZR STCK BP NVEBDIZC");

    if (mem == e_comp_space && mon_interfaces[mem]->get_line_cycle != NULL) {
        mon_out(" LIN CYC\n");
    } else {
        mon_out("\n");
    }

    mon_out(".;%04x %02x %02x %02X %02x %04x %02x %d%d%d%d%d%d%d%d",
              addr_location(mon_register_get_val(mem, e_PC)),
              mon_register_get_val(mem, e_A),
              mon_register_get_val(mem, e_X),
              mon_register_get_val(mem, e_Y),
              mon_register_get_val(mem, e_Z),
              mon_register_get_val(mem, e_SP),
              mon_register_get_val(mem, e_BP),
              TEST(MOS65CE02_REGS_GET_SIGN(regs)),
              TEST(MOS65CE02_REGS_GET_OVERFLOW(regs)),
              TEST(MOS65CE02_REGS_GET_STACK_EXTEND(regs)),
              TEST(MOS65CE02_REGS_GET_BREAK(regs)),
              TEST(MOS65CE02_REGS_GET_DECIMAL(regs)),
              TEST(MOS65CE02_REGS_GET_INTERRUPT(regs)),
              TEST(MOS65CE02_REGS_GET_ZERO(regs)),
              TEST(MOS65CE02_REGS_GET_CARRY(regs)));

    if (mem == e_comp_space && mon_interfaces[mem]->get_line_cycle != NULL) {
        unsigned int line, cycle;
        int half_cycle;

        mon_interfaces[mem]->get_line_cycle(&line, &cycle, &half_cycle);

        if (half_cycle == -1) {
            mon_out(" %03i %03i\n", line, cycle);
        } else {
            mon_out(" %03i %03i %i\n", line, cycle, half_cycle);
        }
    } else {
        mon_out("\n");
    }
}

static mon_reg_list_t *mon_register_list_get65CE02(int mem)
{
    mon_reg_list_t *mon_reg_list;

    mon_reg_list = lib_malloc(sizeof(mon_reg_list_t) * 9);

    mon_reg_list[0].name = "PC";
    mon_reg_list[0].val = (unsigned int)mon_register_get_val(mem, e_PC);
    mon_reg_list[0].size = 16;
    mon_reg_list[0].flags = 0;
    mon_reg_list[0].next = &mon_reg_list[1];

    mon_reg_list[1].name = "AC";
    mon_reg_list[1].val = (unsigned int)mon_register_get_val(mem, e_A);
    mon_reg_list[1].size = 8;
    mon_reg_list[1].flags = 0;
    mon_reg_list[1].next = &mon_reg_list[2];

    mon_reg_list[2].name = "XR";
    mon_reg_list[2].val = (unsigned int)mon_register_get_val(mem, e_X);
    mon_reg_list[2].size = 8;
    mon_reg_list[2].flags = 0;
    mon_reg_list[2].next = &mon_reg_list[3];

    mon_reg_list[3].name = "YR";
    mon_reg_list[3].val = (unsigned int)mon_register_get_val(mem, e_Y);
    mon_reg_list[3].size = 8;
    mon_reg_list[3].flags = 0;
    mon_reg_list[3].next = &mon_reg_list[4];

    mon_reg_list[4].name = "ZR";
    mon_reg_list[4].val = (unsigned int)mon_register_get_val(mem, e_Z);
    mon_reg_list[4].size = 8;
    mon_reg_list[4].flags = 0;
    mon_reg_list[4].next = &mon_reg_list[5];

    mon_reg_list[5].name = "SP";
    mon_reg_list[5].val = (unsigned int)mon_register_get_val(mem, e_SP);
    mon_reg_list[5].size = 16;
    mon_reg_list[5].flags = 0;
    mon_reg_list[5].next = &mon_reg_list[6];

    mon_reg_list[6].name = "BP";
    mon_reg_list[6].val = (unsigned int)mon_register_get_val(mem, e_BP);
    mon_reg_list[6].size = 8;
    mon_reg_list[6].flags = 0;
    mon_reg_list[6].next = &mon_reg_list[7];

    mon_reg_list[7].name = "FL";
    mon_reg_list[7].val = (unsigned int)mon_register_get_val(mem, e_FLAGS);
    mon_reg_list[7].size = 8;
    mon_reg_list[7].flags = 0;
    mon_reg_list[7].next = &mon_reg_list[8];

    mon_reg_list[8].name = "NVEBDIZC";
    mon_reg_list[8].val = (unsigned int)mon_register_get_val(mem, e_FLAGS);
    mon_reg_list[8].size = 8;
    mon_reg_list[8].flags = 1;
    mon_reg_list[8].next = NULL;

    return mon_reg_list;
}

static void mon_register_list_set65CE02(mon_reg_list_t *reg_list, int mem)
{
    do {
        if (!strcmp(reg_list->name, "PC")) {
            mon_register_set_val(mem, e_PC, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "AC")) {
            mon_register_set_val(mem, e_A, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "XR")) {
            mon_register_set_val(mem, e_X, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "YR")) {
            mon_register_set_val(mem, e_Y, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "ZR")) {
            mon_register_set_val(mem, e_Z, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "SP")) {
            mon_register_set_val(mem, e_SP, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "BP")) {
            mon_register_set_val(mem, e_BP, (WORD)(reg_list->val));
        }
        if (!strcmp(reg_list->name, "NVEBDIZC")) {
            mon_register_set_val(mem, e_FLAGS, (WORD)(reg_list->val));
        }

        reg_list = reg_list->next;
    } while (reg_list != NULL);
}

void mon_register65CE02_init(monitor_cpu_type_t *monitor_cpu_type)
{
    monitor_cpu_type->mon_register_get_val = mon_register_get_val;
    monitor_cpu_type->mon_register_set_val = mon_register_set_val;
    monitor_cpu_type->mon_register_print = mon_register_print;
    monitor_cpu_type->mon_register_list_get = mon_register_list_get65CE02;
    monitor_cpu_type->mon_register_list_set = mon_register_list_set65CE02;
}
