/*
 * flash040core.c - (AM)29F040(B) Flash emulation.
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

#include <stdio.h>

#include "lib.h"
#include "flash040.h"
#include "types.h"

/* -------------------------------------------------------------------------- */

struct flash_types_s {
    BYTE manufacturer_ID;
    BYTE device_ID;
    unsigned int magic_1_addr;
    unsigned int magic_2_addr;
    unsigned int magic_1_mask;
    unsigned int magic_2_mask;
};
typedef struct flash_types_s flash_types_t;

static flash_types_t flash_types[FLASH040_TYPE_NUM] = {
    { 0x01, 0xa4, 0x5555, 0x2aaa, 0x7fff, 0x7fff },
    { 0x01, 0xa4, 0x555,  0x2aa,  0x7ff,  0x7ff  }
};

/* -------------------------------------------------------------------------- */

static void REGPARM3 flash040core_store_internal(flash040_context_t *flash040_context,
                                                 unsigned int addr, BYTE byte)
{
    switch (flash040_context->flash_state) {
        case FLASH040_STATE_READ:
            if (((addr & flash_types[flash040_context->flash_type].magic_1_mask) == flash_types[flash040_context->flash_type].magic_1_addr) && (byte == 0xaa)) {
                flash040_context->flash_state = FLASH040_STATE_MAGIC_1;
            }
            break;

        case FLASH040_STATE_MAGIC_1:
            if (((addr & flash_types[flash040_context->flash_type].magic_2_mask) == flash_types[flash040_context->flash_type].magic_2_addr) && (byte == 0x55)) {
                flash040_context->flash_state = FLASH040_STATE_MAGIC_2;
            } else {
                flash040_context->flash_state = FLASH040_STATE_READ;
            }
            break;

        case FLASH040_STATE_MAGIC_2:
            if ((addr & flash_types[flash040_context->flash_type].magic_1_mask) == flash_types[flash040_context->flash_type].magic_1_addr) {
                switch (byte) {
                    case 0x90:
                        flash040_context->flash_state = FLASH040_STATE_AUTOSELECT;
                        break;
                    case 0xa0:
                        flash040_context->flash_state = FLASH040_STATE_BYTE_PROGRAM;
                        break;
                    case 0x80:
                        flash040_context->flash_state = FLASH040_STATE_ERASE_MAGIC_1;
                        break;
                    default:
                        flash040_context->flash_state = FLASH040_STATE_READ;
                        break;
                }
            } else {
                flash040_context->flash_state = FLASH040_STATE_READ;
            }
            break;

        case FLASH040_STATE_BYTE_PROGRAM:
            flash040_context->flash_data[addr] &= byte;
            flash040_context->flash_state = FLASH040_STATE_BYTE_PROGRAM_HAPPENING;
            break;

        case FLASH040_STATE_ERASE_MAGIC_1:
            if (((addr & flash_types[flash040_context->flash_type].magic_1_mask) == flash_types[flash040_context->flash_type].magic_1_addr) && (byte == 0xaa)) {
                flash040_context->flash_state = FLASH040_STATE_ERASE_MAGIC_2;
            } else {
                flash040_context->flash_state = FLASH040_STATE_READ;
            }
            break;

        case FLASH040_STATE_ERASE_MAGIC_2:
            if (((addr & flash_types[flash040_context->flash_type].magic_2_mask) == flash_types[flash040_context->flash_type].magic_2_addr) && (byte == 0x55)) {
                flash040_context->flash_state = FLASH040_STATE_ERASE_SELECT;
            } else {
                flash040_context->flash_state = FLASH040_STATE_READ;
            }
            break;

        case FLASH040_STATE_ERASE_SELECT:
            if (((addr & flash_types[flash040_context->flash_type].magic_1_mask) == flash_types[flash040_context->flash_type].magic_1_addr) && (byte == 0x10)) {
                /* erase chip here */
                flash040_context->flash_state = FLASH040_STATE_CHIP_ERASE;
            } else if (byte == 0x30) {
                /* erase sector here */
                flash040_context->flash_state = FLASH040_STATE_SECTOR_ERASE;
            }
            break;

        case FLASH040_STATE_AUTOSELECT:
            if (byte == 0xf0) {
                flash040_context->flash_state = FLASH040_STATE_READ;
            }
            break;

        default:
            break;
    }
}

/* -------------------------------------------------------------------------- */

void REGPARM3 flash040core_store(flash040_context_t *flash040_context, unsigned int addr, BYTE byte)
{
    if (*(flash040_context->rmw_flag)) {
        (*(flash040_context->clk_ptr))--;
        flash040core_store_internal(flash040_context, addr, flash040_context->last_read);
        (*(flash040_context->clk_ptr))++;
    }

    flash040core_store_internal(flash040_context, addr, byte);
}


BYTE REGPARM2 flash040core_read(flash040_context_t *flash040_context, unsigned int addr)
{
    BYTE value;

    switch (flash040_context->flash_state) {
        case FLASH040_STATE_AUTOSELECT:
            switch (addr & 0xff) {
                case 0x00:
                    value = flash_types[flash040_context->flash_type].manufacturer_ID;
                    break;
                case 0x01:
                    value = flash_types[flash040_context->flash_type].device_ID;
                    break;
                case 0x02:
                    value = 0;
                    break;
                default:
                    value = flash040_context->flash_data[addr];
                    break;
            }
            break;

        default:
            flash040_context->flash_state = FLASH040_STATE_READ;
            /* fall through */
        case FLASH040_STATE_READ:
            value = flash040_context->flash_data[addr];
            break;
    }

    flash040_context->last_read = value;
    return value;
}

BYTE REGPARM2 flash040core_peek(flash040_context_t *flash040_context, unsigned int addr)
{
    return flash040_context->flash_data[addr];
}

void flash040core_reset(flash040_context_t *flash040_context)
{
    flash040_context->flash_state = FLASH040_STATE_READ;
}


void flash040core_init(struct flash040_context_s *flash040_context,
                       flash040_type_t type, unsigned int size)
{
    flash040_context->flash_data = lib_malloc(size);
    flash040_context->flash_state = FLASH040_STATE_READ;
}

void flash040core_shutdown(flash040_context_t *flash040_context)
{
    lib_free(flash040_context->flash_data);
    lib_free(flash040_context);
}

