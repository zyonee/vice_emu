/*
 * par-flashrom.c - 29F040 Flash ROM Emulation
 *
 *   - very incomplete, just barely enough to make the MMCR Rescue flasher work
 *
 * Written by
 *  Groepaz/Hitmen <groepaz@gmx.net>
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

/* indent -gnu -bls -hnl -nut -sc -cli4 -npsl -i4 -bli0 -cbi0 -ci4 -di8 -l80 */

#include "par-flashrom.h"
#include "types.h"
#include "util.h"
#include "log.h"
#include "c64cart.h"

#define DEBUG

/* #define LOG_READ_STATUS */         /* log flashrom status reads */
/* #define LOG_CMD_BLOCK_ERASE */
/* #define LOG_CMD_PROGRAM */
/* #define LOG_CMD_RESET */
/* #define LOG_BOGUS_WRITE */

#ifdef DEBUG
#define LOG(_x_) log_debug _x_
#else
#define LOG(_x_)
#endif

/*
00h Read
10h Chip Erase Confirm
30h Block Erase Resume/Confirm
80h Set-up Erase
90h Read Electronic Signature/Block Protection Status
A0h Program
B0h Erase Suspend
F0h Read Array/Reset
*/

#define FLASH_CMD_00	0x00
#define FLASH_CMD_10	0x10
#define FLASH_CMD_30	0x30
#define FLASH_CMD_80	0x80
#define FLASH_CMD_90	0x90
#define FLASH_CMD_A0	0xA0
#define FLASH_CMD_B0	0xB0
#define FLASH_CMD_F0	0xF0

#define FLASH_CMD_CHIP_ERASE	0x10
#define FLASH_CMD_BLOCK_ERASE	0x30

#define FLASH_CMD_NONE	0xff

static unsigned int flashrom_status = 0;
static unsigned int flashrom_cycle_count = 0;
static unsigned int flashrom_read_cycle = 0;
static unsigned int flashrom_current_command = 0;

void flashrom_init (void)
{
    flashrom_status = 0;
    flashrom_cycle_count = 0;
    flashrom_read_cycle = 0;
    flashrom_current_command = FLASH_CMD_NONE;
}

BYTE flashrom_read (unsigned int addr)
{
    if (flashrom_current_command != FLASH_CMD_NONE)
    {
        switch (addr & 0xffff)
        {
            case 0x0000:
                switch (flashrom_read_cycle)
                {
                    case 0:
                        flashrom_status = 0x80;
                        flashrom_read_cycle++;
                        break;
                    case 1:
                        flashrom_status = 0x00;
                        flashrom_read_cycle++;
                        break;
                    case 2:
                        flashrom_status = 0xff;
                        flashrom_read_cycle++;
                        break;
                    case 3:
                        flashrom_status = 0x00;
                        flashrom_read_cycle++;
                        break;
                    case 4:
                        flashrom_status = 0xff;
                        break;
                }
#ifdef LOG_READ_STATUS
                LOG (("FLASHROM read status %02x", flashrom_status));
#endif
                return flashrom_status;
                break;
        }
    }
    return roml_banks[addr];
}


void flashrom_write (unsigned int addr, BYTE value)
{
    switch (flashrom_cycle_count)
    {
        case 0:
            if ((addr == 0x5555) & (value == 0xaa))
            {
                flashrom_cycle_count++;
            }

            else if ((value == 0xf0))
            {
#ifdef LOG_CMD_RESET
                LOG (("FLASHROM CMDF0: Read Array/Reset"));
#endif
                flashrom_read_cycle = 0;
                flashrom_status = 0;
                flashrom_cycle_count = 0;
                flashrom_current_command = FLASH_CMD_NONE;
            }
            else
            {
                flashrom_cycle_count = 0;
            }
            break;
        case 1:
            if ((addr == 0x2aaa) & (value == 0x55))
            {
                flashrom_cycle_count++;
            }
            else
            {
                flashrom_cycle_count = 0;
            }
            break;
        case 2:
            if ((addr == 0x5555) & (value == 0x80))
            {
                flashrom_cycle_count++;
            }
            else if ((addr == 0x5555) & (value == 0x90))
            {
                flashrom_cycle_count++;
            }
            else if ((addr == 0x5555) & (value == 0xa0))
            {
                flashrom_cycle_count++;
                flashrom_current_command = FLASH_CMD_A0;
            }
            else if ((addr == 0x5555) & (value == 0xf0))
            {
#ifdef LOG_CMD_RESET
                LOG (("FLASHROM CMDF0: Read Array/Reset"));
#endif
                flashrom_current_command = FLASH_CMD_NONE;
                flashrom_cycle_count = 0;
            }
            else
            {
                flashrom_cycle_count = 0;
            }
            break;
        case 3:
            if (flashrom_current_command == FLASH_CMD_A0)
            {
#ifdef LOG_CMD_PROGRAM
                LOG (("FLASHROM CMDA0: Program %06x:%02x", addr, value));
#endif
                roml_banks[addr] = value;
                flashrom_cycle_count = 0;
            }
            else
            {
                if ((addr == 0x5555) & (value == 0xaa))
                {
                    flashrom_cycle_count++;
                }
                else
                {
                    flashrom_cycle_count = 0;
                }
            }
            break;
        case 4:
            if ((addr == 0x2aaa) & (value == 0x55))
            {
                /* block erase or chip erase */
                flashrom_cycle_count++;
            }
            else
            {
                flashrom_cycle_count = 0;
            }
            break;
        case 5:
            if ((addr == 0x5555) & (value == 0x10))
            {
                /* chip erase */
                flashrom_cycle_count++;
                flashrom_current_command = FLASH_CMD_CHIP_ERASE;
            }
            else if ((value == 0x30))
            {
                /* block erase (addr=block addr) */
                flashrom_current_command = FLASH_CMD_BLOCK_ERASE;
#ifdef LOG_CMD_BLOCK_ERASE
                LOG (("FLASHROM CMD30 Block Erase Addr %06x", addr));
#endif
                flashrom_status = 0xff; /* not erased */
                flashrom_read_cycle = 0;
            }
            else
            {
                flashrom_cycle_count = 0;
            }
            break;
        case 6:
            /* FIXME: there could be more CMD30/cycle 5 blocks
             * following */
            break;
        default:
#ifdef LOG_BOGUS_WRITE
                LOG (("FLASHROM write %06x %02x", addr,value));
#endif
            break;
    }
}
