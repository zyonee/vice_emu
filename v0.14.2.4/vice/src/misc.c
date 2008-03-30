/*
 * misc.c - Miscellaneous functions for debugging.
 *
 * Written by
 *  Vesa-Matti Puro (vmp@lut.fi)
 *  Jarkko Sonninen (sonninen@lut.fi)
 *  Jouko Valta     (jopi@stekt.oulu.fi)
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

/*
 * This file contains misc funtions to help debugging.
 * Included are:
 *	o Show numeric conversions
 *	o Show CPU registers
 *	o Show Stack contents
 *	o Print binary number
 *	o Print instruction hexas from memory
 *	o Print instruction from memory
 *	o Decode instruction
 *	o Find effective address for operand
 *	o Create a copy of string
 *	o Move memory
 *
 * sprint_opcode returns mnemonic code of machine instruction.
 * sprint_binary returns binary form of given code (8bit)
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "maincpu.h"
#include "misc.h"
#include "macro.h"
#include "mshell.h"
#include "asm.h"
#include "resources.h"

#define HEX 1


/*
 * Base conversions.
 * (+) -%&#$
 */

void    show_bases(char *line, int mode)
{
    char buf[20];
    int temparg = sconv(line, 0, mode);

    strcpy(buf, sprint_binary(UPPER(temparg)) );
    printf("\n %17x x\n %17d d\n %17o o\n %s %s b\n\n",
	   temparg, temparg, temparg, buf,
	   sprint_binary(LOWER(temparg)));
}


/*
 * show prints PSW and contents of registers.
 */

void    show(void)
{
#if 0
    if (verflg)
	printf(hexflg ?
	    "PC=%4X AC=%2X XR=%2X YR=%2X PF=%s SP=%2X %3X %3d %s\n" :
	    "PC=%04d AC=%03d XR=%03d YR=%03d PF=%s SP=%03d %3d %2X %s\n",
	    (int) PC, (int) AC, (int) XR, (int) YR,
	    sprint_status(), (int) SP,
	    LOAD(PC), LOAD(PC), sprint_opcode(PC, hexflg));
    else
#endif
	printf(app_resources.hexFlag ? "%lx %4X %s\n" : "%ld %4d %s\n",
	       clk, PC, sprint_opcode(PC, app_resources.hexFlag));
}


void    print_stack(BYTE sp)
{
    int     i;

    printf("Stack: ");
    for (i = 0x101 + sp; i < 0x200; i += 2)
	printf("%02X%02X  ", ram[i + 1], ram[i]);
    printf("\n");
}


char   *sprint_binary(BYTE code)
{
    static char bin[9];
    int     i;

    bin[8] = 0;		/* Terminator. */

    for (i = 0; i < 8; i++) {
	bin[i] = (code & 128) ? '1' : '0';
	code <<= 1;
    }

    return bin;
}


/* ------------------------------------------------------------------------- */

char   *sprint_ophex (ADDRESS p)
{
    static char hexbuf [20];
    char *bp;
    int   j, len;

    len = clength[lookup[LOAD(p)].addr_mode];
    *hexbuf = '\0';
    for (j = 0, bp = hexbuf; j < 3 ; j++, bp += 3) {
	if (j < len) {
	    sprintf (bp, "%02X ", LOAD(p+j));
	} else {
	    strcat (bp, "   ");
	}
    }
    return hexbuf;
}


char   *sprint_opcode(ADDRESS counter, int base)
{
    BYTE    x = LOAD(counter);
    BYTE    p1 = LOAD(counter + 1);
    BYTE    p2 = LOAD(counter + 2);

    return sprint_disassembled(counter, x, p1, p2, base);
}


char   *sprint_disassembled(ADDRESS counter, BYTE x, BYTE p1, BYTE p2, int base)
{
    static char buff[20];
    const char *string;
    char *buffp;
    int addr_mode;
    int ival;

    ival = p1 & 0xFF;

    buffp = buff;
    string = lookup[x].mnemonic;
    addr_mode = lookup[x].addr_mode;

    sprintf(buff, "$%02X %s", x, string);	/* Print opcode and mnemonic. */
    while (*++buffp);

    switch (addr_mode) {
	/*
	 * Bits 0 and 1 are usual marks for X and Y indexed addresses, i.e.
	 * if  bit #0 is set addressing mode is X indexed something and if
	 * bit #1 is set addressing mode is Y indexed something. This is not
	 * from MOS6502, but convention used in this program. See
	 * "vmachine.h" for details.
	 */

	/* Print arguments of the machine instruction. */

      case IMPLIED:
	break;

      case ACCUMULATOR:
	sprintf(buffp, " A");
	break;

      case IMMEDIATE:
	sprintf(buffp, ((base & HEX) ? " #$%02X" : " %3d"), ival);
	break;

      case ZERO_PAGE:
	sprintf(buffp, ((base & HEX) ? " $%02X" : " %3d"), ival);
	break;

      case ZERO_PAGE_X:
	sprintf(buffp, ((base & HEX) ? " $%02X,X" : " %3d,X"), ival);
	break;

      case ZERO_PAGE_Y:
	sprintf(buffp, ((base & HEX) ? " $%02X,Y" : " %3d,Y"), ival);
	break;

      case ABSOLUTE:
	ival |= ((p2 & 0xFF) << 8);
	sprintf(buffp, ((base & HEX) ? " $%04X" : " %5d"), ival);
	break;

      case ABSOLUTE_X:
	ival |= ((p2 & 0xFF) << 8);
	sprintf(buffp, ((base & HEX) ? " $%04X,X" : " %5d,X"), ival);
	break;

      case ABSOLUTE_Y:
	ival |= ((p2 & 0xFF) << 8);
	sprintf(buffp, ((base & HEX) ? " $%04X,Y" : " %5d,Y"), ival);
	break;

      case INDIRECT_X:
	sprintf(buffp, ((base & HEX) ? " ($%02X,X)" : " (%3d,X)"), ival);
	break;

      case INDIRECT_Y:
	sprintf(buffp, ((base & HEX) ? " ($%02X),Y" : " (%3d),Y"), ival);
	break;

      case ABS_INDIRECT:
	ival |= ((p2 & 0xFF) << 8);
	sprintf(buffp, ((base & HEX) ? " ($%04X)" : " (%5d)"), ival);
	break;

      case RELATIVE:
	if (0x80 & ival)
	    ival -= 256;
	ival += counter;
	ival += 2;
	sprintf(buffp, ((base & HEX) ? " $%04X" : " %5d"), ival);
	break;
    }

    return buff;
}


int     eff_address(ADDRESS counter, int step)
{
    int     addr_mode, eff;
    BYTE    x = LOAD(counter);
    BYTE    p1 = 0;
    ADDRESS p2 = 0;


    addr_mode = lookup[x].addr_mode;

    switch (clength[addr_mode]) {
      case 2:
	p1 = LOAD(counter + 1);
	break;
      case 3:
	p2 = LOAD(counter + 1) | (LOAD(counter + 2) << 8);
	break;
    }

    switch (addr_mode) {

      case IMPLIED:
      case ACCUMULATOR:
	eff = -1;
	break;

      case IMMEDIATE:
	eff = -1;
	break;

      case ZERO_PAGE:
	eff = p1;
	break;

      case ZERO_PAGE_X:
	eff = (p1 + XR) & 0xff;
	break;

      case ZERO_PAGE_Y:
	eff = (p1 + YR) & 0xff;
	break;

      case ABSOLUTE:
	eff = p2;
	break;

      case ABSOLUTE_X:
	eff = p2 + XR;
	break;

      case ABSOLUTE_Y:
	eff = p2 + YR;
	break;

      case ABS_INDIRECT:  /* loads 2-byte address */
	eff = p2;
	break;

      case INDIRECT_X:
	eff = LOAD_ZERO_ADDR(p1 + XR);
	break;

      case INDIRECT_Y:
	eff = LOAD_ZERO_ADDR(p1) + YR;
	break;

      case RELATIVE:
	eff = REL_ADDR(counter +2, p1);
	break;

      default:
	eff = -1;
    }

    return eff;
}
