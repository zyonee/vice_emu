/*
 * mon_disassemble.c - The VICE built-in monitor, disassembler module.
 *
 * Written by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "asm.h"
#include "log.h"
#include "mon_disassemble.h"
#include "mon_util.h"
#include "monitor.h"
#include "types.h"
#include "uimon.h"

const char *mon_disassemble_to_string_internal(MEMSPACE memspace, unsigned int addr,
                                               unsigned int x, unsigned int p1, unsigned int p2, unsigned int p3,
                                               int hex_mode, unsigned *opc_size_p, monitor_cpu_type_t *mon_cpu_type)
{
    static char buff[256];
    const char *string;
    char *buffp, *addr_name;
    int addr_mode;
    unsigned opc_size;
    WORD ival;
    WORD ival2;
    const asm_opcode_info_t *opinfo;

    ival = (WORD)(p1 & 0xff);

    buffp = buff;

    if (!mon_cpu_type) {
        mon_cpu_type=monitor_cpu_for_memspace[memspace];
    }
    opinfo = (mon_cpu_type->asm_opcode_info_get)(x, p1, p2);
    string = opinfo->mnemonic;
    addr_mode = opinfo->addr_mode;
    opc_size = (mon_cpu_type->asm_addr_mode_get_size)
               ((unsigned int)(addr_mode), x, p1);

    if (opc_size_p) {
        *opc_size_p = opc_size;
    }

    switch (opc_size) {
      case 1:
        sprintf(buff, "%02X         %s", x, string);
        break;
      case 2:
        sprintf(buff, "%02X %02X      %s", x, p1 & 0xff, string);
        break;
      case 3:
        sprintf(buff, "%02X %02X %02X   %s", x, p1 & 0xff, p2 & 0xff, string);
        break;
      case 4:
        sprintf(buff, "%02X %02X %02X %02X %s", x, p1 & 0xff, p2 & 0xff,
                p3 & 0xff, string);
        break;
      default:
        mon_out("Invalid opcode length: %d\n", opc_size);
        sprintf(buff, "            %s", string);
    }

    while (*++buffp);

    /* Print arguments of the machine instruction. */
    switch (addr_mode) {
      case ASM_ADDR_MODE_IMPLIED:
        break;

      case ASM_ADDR_MODE_ACCUMULATOR:
        sprintf(buffp, " A");
        break;

      case ASM_ADDR_MODE_IMMEDIATE:
        sprintf(buffp, (hex_mode ? " #$%02X" : " %3d"), ival);
        break;

      case ASM_ADDR_MODE_IMMEDIATE_24:
        sprintf(buffp, (hex_mode ? " #$%06X" : " %8d"), p1 | (p2 << 8) | (p3 << 16));
        break;

      case ASM_ADDR_MODE_ZERO_PAGE:
        sprintf(buffp, (hex_mode ? " $%02X" : " %3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT0:
        sprintf(buffp, (hex_mode ? " #0,$%02X" : " #0,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT1:
        sprintf(buffp, (hex_mode ? " #1,$%02X" : " #1,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT2:
        sprintf(buffp, (hex_mode ? " #2,$%02X" : " #2,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT3:
        sprintf(buffp, (hex_mode ? " #3,$%02X" : " #3,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT4:
        sprintf(buffp, (hex_mode ? " #4,$%02X" : " #4,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT5:
        sprintf(buffp, (hex_mode ? " #5,$%02X" : " #5,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT6:
        sprintf(buffp, (hex_mode ? " #6,$%02X" : " #6,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT7:
        sprintf(buffp, (hex_mode ? " #7,$%02X" : " #7,%3d"), ival);
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_X:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%02X,X" : " %3d,X"), ival);
        } else {
            sprintf(buffp, " %s,X", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_Y:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%02X,Y" : " %3d,Y"), ival);
        } else {
            sprintf(buffp, " %s,Y", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE:
        ival |= (WORD)((p2 & 0xff) << 8);
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
           sprintf(buffp, " %s", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " %s+1", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " $%04X" : " %5d"), ival);
            }
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_LONG:
        ival |= (WORD)((p2 & 0xff) << 8);
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
           sprintf(buffp, " %s", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " %s+1", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " $%06X" : " %7d"), ival | ((p3 & 0xff) << 16));
            }
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_LONG_X:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%06X,X" : " %7d,X"), ival | ((p3 & 0xff) << 16));
        } else {
            sprintf(buffp, " %s,X", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_X:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%04X,X" : " %5d,X"), ival);
        } else {
            sprintf(buffp, " %s,X", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_Y:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%04X,Y" : " %5d,Y"), ival);
        } else {
            sprintf(buffp, " %s,Y", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABS_INDIRECT:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%04X)" : " (%5d)"), ival);
        } else {
            sprintf(buffp, " (%s)", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABS_IND_LONG:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " [$%04X]" : " [%5d]"), ival);
        } else {
            sprintf(buffp, " [%s]", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABS_INDIRECT_X:
        ival |= (WORD)((p2 & 0xff) << 8);
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%04X,X)" : " (%5d,X)"), ival);
        } else {
            sprintf(buffp, " (%s,X)", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT_X:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%02X,X)" : " (%3d,X)"), ival);
        } else {
            sprintf(buffp, " (%s,X)", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT_Y:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%02X),Y" : " (%3d),Y"), ival);
        } else {
            sprintf(buffp, " (%s),Y", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT_Z:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%02X),Z" : " (%3d),Z"), ival);
        } else {
            sprintf(buffp, " (%s),Z", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " ($%02X)" : " (%3d)"), ival);
        } else {
            sprintf(buffp, " (%s)", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT_LONG:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " [$%02X]" : " [%3d]"), ival);
        } else {
            sprintf(buffp, " [%s]", addr_name);
        }
        break;

      case ASM_ADDR_MODE_INDIRECT_LONG_Y:
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " [$%02X],Y" : " [%3d],Y"), ival);
        } else {
            sprintf(buffp, " [%s],Y", addr_name);
        }
        break;

      case ASM_ADDR_MODE_STACK_RELATIVE:
        sprintf(buffp, (hex_mode ? " $%02X,S" : " %3d,S"), ival);
        break;

      case ASM_ADDR_MODE_STACK_RELATIVE_Y:
        sprintf(buffp, (hex_mode ? " ($%02X,S),Y" : " (%3d,S),Y"), ival);
        break;

      case ASM_ADDR_MODE_MOVE:
        sprintf(buffp, (hex_mode ? " $%02X,$%02X" : " %3d,%3d"), ival, p2 & 0xff);
        break;

      case ASM_ADDR_MODE_RELATIVE:
        if (0x80 & ival) {
            ival -= 256;
        }
        ival += addr;
        ival += 2;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%04X" : " %5d"), ival);
        } else {
            sprintf(buffp, " %s", addr_name);
        }
        break;

      case ASM_ADDR_MODE_RELATIVE_LONG:
        ival |= (p2 & 0xff) << 8;
        if (0x8000 & ival) {
            ival -= 65536;
        }
        ival += addr;
        ival += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, (hex_mode ? " $%04X" : " %5d"), ival);
        } else {
            sprintf(buffp, " %s", addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT0_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #0,$%02X,$%04X" : " #0,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #0,$%02X,%s" : " #0,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT1_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #1,$%02X,$%04X" : " #1,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #1,$%02X,%s" : " #1,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT2_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #2,$%02X,$%04X" : " #2,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #2,$%02X,%s" : " #2,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT3_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #3,$%02X,$%04X" : " #3,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #3,$%02X,%s" : " #3,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT4_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #4,$%02X,$%04X" : " #4,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #4,$%02X,%s" : " #4,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT5_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #5,$%02X,$%04X" : " #5,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #5,$%02X,%s" : " #5,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT6_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #6,$%02X,$%04X" : " #6,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #6,$%02X,%s" : " #6,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ZERO_PAGE_BIT7_RELATIVE:
        ival2 = (p2 & 0xff);
        if (0x80 & ival2) {
            ival2 -= 256;
        }
        ival2 += addr;
        ival2 += 3;
        if (!(addr_name = mon_symbol_table_lookup_name(e_comp_space, ival2))) {
            sprintf(buffp, (hex_mode ? " #7,$%02X,$%04X" : " #7,%3d,%5d"), ival, ival2);
        } else {
            sprintf(buffp, (hex_mode ? " #7,$%02X,%s" : " #7,%3d,%s"), ival, addr_name);
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_A:
        ival |= (WORD)((p2 & 0xff) << 8);
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
           sprintf(buffp, " (%s),A", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " (%s+1),A", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " ($%04X),A" : " (%5d),A"), ival);
            }
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_HL:
        ival |= (WORD)((p2 & 0xff) << 8);
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
           sprintf(buffp, " (%s),HL", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " (%s+1),HL", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " ($%04X),HL" : " (%5d),HL"), ival);
            }
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_IX:
        ival = (WORD)((p2 & 0xff) | ((p3 & 0xff) << 8));
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, " (%s),IX", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " (%s+1),IX", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " ($%04X),IX" : " (%5d),IX"), ival);
            }
        }
        break;

      case ASM_ADDR_MODE_ABSOLUTE_IY:
        ival = (WORD)((p2 & 0xff) | ((p3 & 0xff) << 8));
        if ((addr_name = mon_symbol_table_lookup_name(e_comp_space, ival))) {
            sprintf(buffp, " (%s),IY", addr_name);
        } else {
            if ((addr_name = mon_symbol_table_lookup_name(e_comp_space,
                (WORD)(ival - 1)))) {
                sprintf(buffp, " (%s+1),IY", addr_name);
            } else {
                sprintf(buffp, (hex_mode ? " ($%04X),IY" : " (%5d),IY"), ival);
            }
        }
        break;

      case ASM_ADDR_MODE_IMMEDIATE_16:
        ival |= (WORD)((p2 & 0xff) << 8);
        sprintf(buffp, (hex_mode ? " #$%04X" : " %5d"), ival);
        break;

      case ASM_ADDR_MODE_REG_B:
        sprintf(buffp, " B");
        break;

      case ASM_ADDR_MODE_REG_C:
        sprintf(buffp, " C");
        break;

      case ASM_ADDR_MODE_REG_D:
        sprintf(buffp, " D");
        break;

      case ASM_ADDR_MODE_REG_E:
        sprintf(buffp, " E");
        break;

      case ASM_ADDR_MODE_REG_H:
        sprintf(buffp, " H");
        break;

      case ASM_ADDR_MODE_REG_IXH:
        sprintf(buffp, " IXH");
        break;

      case ASM_ADDR_MODE_REG_IYH:
        sprintf(buffp, " IYH");
        break;

      case ASM_ADDR_MODE_REG_L:
        sprintf(buffp, " L");
        break;

      case ASM_ADDR_MODE_REG_IXL:
        sprintf(buffp, " IXL");
        break;

      case ASM_ADDR_MODE_REG_IYL:
        sprintf(buffp, " IYL");
        break;

      case ASM_ADDR_MODE_REG_AF:
        sprintf(buffp, " AF");
        break;

      case ASM_ADDR_MODE_REG_BC:
        sprintf(buffp, " BC");
        break;

      case ASM_ADDR_MODE_REG_DE:
        sprintf(buffp, " DE");
        break;

      case ASM_ADDR_MODE_REG_HL:
        sprintf(buffp, " HL");
        break;

      case ASM_ADDR_MODE_REG_IX:
        sprintf(buffp, " IX");
        break;

      case ASM_ADDR_MODE_REG_IY:
        sprintf(buffp, " IY");
        break;

      case ASM_ADDR_MODE_REG_SP:
        sprintf(buffp, " SP");
        break;

      case ASM_ADDR_MODE_REG_IND_BC:
        sprintf(buffp, " (BC)");
        break;

      case ASM_ADDR_MODE_REG_IND_DE:
        sprintf(buffp, " (DE)");
        break;

      case ASM_ADDR_MODE_REG_IND_HL:
        sprintf(buffp, " (HL)");
        break;

      case ASM_ADDR_MODE_REG_IND_IX:
        sprintf(buffp, " (IX)");
        break;

      case ASM_ADDR_MODE_REG_IND_IY:
        sprintf(buffp, " (IY)");
        break;

      case ASM_ADDR_MODE_REG_IND_SP:
        sprintf(buffp, " (SP)");
        break;
    }

    return buff;
}

const char *mon_disassemble_to_string(MEMSPACE memspace, unsigned int addr,
                                      unsigned int x, unsigned int p1, unsigned int p2, unsigned int p3,
                                      int hex_mode, const char *cpu_type)
{
    return mon_disassemble_to_string_internal(memspace, addr, x, p1, p2, p3, 
        hex_mode, NULL, monitor_find_cpu_type_from_string(cpu_type));
}

const char *mon_disassemble_to_string_ex(MEMSPACE memspace, unsigned int addr,
                                         unsigned int x, unsigned int p1, unsigned int p2, unsigned int p3,
                                         int hex_mode, unsigned *opc_size_p)
{
    return mon_disassemble_to_string_internal(memspace, addr, x, p1, p2, p3, 
        hex_mode, opc_size_p, monitor_cpu_for_memspace[memspace]);
}

unsigned mon_disassemble_instr(MON_ADDR addr)
{
    BYTE op, p1, p2, p3;
    MEMSPACE mem;
    WORD loc;
    int hex_mode = 1;
    char *label;
    const char *dis_inst;
    unsigned opc_size;

    mem = addr_memspace(addr);
    loc = addr_location(addr);

    op = mon_get_mem_val(mem, loc);
    p1 = mon_get_mem_val(mem, (WORD)(loc + 1));
    p2 = mon_get_mem_val(mem, (WORD)(loc + 2));
    p3 = mon_get_mem_val(mem, (WORD)(loc + 3));

    /* Print the label for this location - if we have one */
    label = mon_symbol_table_lookup_name(mem, loc);
    if (label) {
        mon_out(".%s:%04x   %s:\n", mon_memspace_string[mem], loc, label);
    }

    dis_inst = mon_disassemble_to_string_internal(mem, loc, op, p1, p2, p3, hex_mode,
                                                  &opc_size, monitor_cpu_for_memspace[mem]);

    /* Print the disassembled instruction */
    mon_out(".%s:%04x   %s\n", mon_memspace_string[mem], loc, dis_inst);

    return opc_size;
    /* asm_addr_mode_get_size(asm_opcode_info_get(op)->addr_mode); */
}

void mon_disassemble_lines(MON_ADDR start_addr, MON_ADDR end_addr)
{
    MEMSPACE mem;
    unsigned end_loc;
    long len, i, bytes;

    len = mon_evaluate_address_range(&start_addr, &end_addr, FALSE,
                                     DEFAULT_DISASSEMBLY_SIZE);

    if (len < 0) {
        log_error(LOG_ERR, "Invalid address range");
        return;
    }

    mem = addr_memspace(start_addr);
    dot_addr[mem] = start_addr;
    end_loc = addr_location(end_addr);

    i = 0;
    while (i <= len) {
        bytes = mon_disassemble_instr(dot_addr[mem]);
        i += bytes;
        mon_inc_addr_location(&(dot_addr[mem]), bytes);
        if (mon_stop_output != 0) {
            break;
        }
    }
}
