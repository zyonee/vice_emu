/*
 * ppi1990.c - ppi emulation for the 1990 disk drive.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#include <string.h>

#include "drivetypes.h"
#include "ppi1990.h"

#define PPI_ACTIVE   0x80
#define PPI_MODE_A   0x60
#define PPI_MODE_A0  0x00
#define PPI_MODE_A1  0x40
#define PPI_INPUT_A  0x10
#define PPI_INPUT_HC 0x08
#define PPI_MODE_B   0x04
#define PPI_MODE_B0  0x00
#define PPI_MODE_B1  0x04
#define PPI_INPUT_B  0x02
#define PPI_INPUT_LC 0x01
#define PPI_SET      0x01

/*
port A:
parallel port

port B:
bit 1 - drive 8 button (low active)
bit 2 - drive 9 button (low active)
bit 3 - write protect button (low active)
*/

struct ppi1990_s {
    struct drive_context_s *mycontext;
    unsigned char port_a;
    unsigned char port_b;
    unsigned char port_lc;
    unsigned char port_hc;
    unsigned char control;
};

BYTE ppi1990_read(drive_context_t *drv, WORD address)
{
    ppi1990_t *ppi = drv->ppi1990;

    switch (address & 3) {
    case 0: 
        return (ppi->control & PPI_INPUT_A) ? 0xff : ppi->port_a;
    case 1:
        if (ppi->control & PPI_INPUT_B) {
            BYTE byte = drv->drive->swap8_button ? 0x00 : 0x02;
            byte |= drv->drive->swap9_button ? 0x00 : 0x04;
            byte |= drv->drive->write_protect_button ? 0x00 : 0x08;
            return byte;
        }
        return ppi->port_b;
    case 2: 
        return ((ppi->control & PPI_INPUT_LC) ? 0x0f : ppi->port_lc) | ((ppi->control & PPI_INPUT_HC) ? 0xf0 : ppi->port_hc);
    default: 
        return 0xff; /* no read of control */
    }
}

void ppi1990_store(drive_context_t *drv, WORD address,
                                         BYTE value)
{
    ppi1990_t *ppi = drv->ppi1990;

    switch (address & 3) {
    case 0: 
        ppi->port_a = value;
        break;
    case 1:
        ppi->port_b = value;
        break;
    case 2:
        ppi->port_lc = value & 0x0f;
        ppi->port_hc = value & 0xf0;
        break;
    default:
        if (value & PPI_ACTIVE) { /* simple mode set */
            ppi->control = value;
            break;
        }
        if (value & PPI_SET) { /* bit set command */
            if (value & 0x08) {
                ppi->port_hc |= 1 << ((value >> 1) & 7);
            } else {
                ppi->port_lc |= 1 << ((value >> 1) & 7);
            }
        } else { /* bit reset command */
            if (value & 0x08) {
                ppi->port_hc &= ~(1 << ((value >> 1) & 7));
            } else {
                ppi->port_lc &= ~(1 << ((value >> 1) & 7));
            }
        }
        break;
    }
}

void ppi1990_reset(ppi1990_t *ppi)
{
    ppi->port_a = 0;
    ppi->port_b = 0;
    ppi->port_lc = 0;
    ppi->port_hc = 0;
    ppi->control = 0x9b;
}

void ppi1990_init(drive_context_t *drv)
{
}

void ppi1990_setup_context(drive_context_t *drv)
{
    drv->ppi1990 = lib_calloc(1, sizeof(ppi1990_t));
    drv->ppi1990->mycontext = drv;
}

void ppi1990_shutdown(ppi1990_t *ppi)
{
    lib_free(ppi);
}

int ppi1990_is_rom(drive_context_t *drv)
{
    ppi1990_t *ppi = drv->ppi1990;

    return ((ppi->control & PPI_INPUT_LC) ? 0x0f : ppi->port_lc) & 1;
}

int ppi1990_is_shift(drive_context_t *drv)
{
    ppi1990_t *ppi = drv->ppi1990;

    return !(((ppi->control & PPI_INPUT_LC) ? 0x0f : ppi->port_lc) & 2);
}
