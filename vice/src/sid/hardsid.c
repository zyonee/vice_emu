/*
 * hardsid.c - Generic hardsid abstraction layer.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  HardSID Support <support@hardsid.com>
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

#ifdef HAVE_HARDSID

#include "hardsid.h"
#include "sid-snapshot.h"
#include "types.h"

static int hardsid_found = 0;
static int hardsid_is_open = 0;

/* buffer containing current register state of SIDs */
static BYTE sidbuf[0x20 * HS_MAXSID];

void hardsid_reset(void)
{
    if (hardsid_found) {
        hardsid_drv_reset();
    }
}

int hardsid_open(void)
{
    if (!hardsid_is_open) {
        hardsid_is_open = !hardsid_drv_open();
        hardsid_found = hardsid_is_open;
        memset(sidbuf, 0, sizeof(sidbuf));
    }
    return hardsid_is_open;
}

int hardsid_close(void)
{
    int retval = 0;

    if (hardsid_is_open) {
        retval = hardsid_drv_close();
        hardsid_is_open = 0;
    }
    return retval;
}

int hardsid_read(WORD addr, int chipno)
{
    if (hardsid_is_open) {
        /* use sidbuf[] for write-only registers */
        if (addr <= 0x18) {
            return sidbuf[chipno * 0x20 + addr];
        }
        return hardsid_drv_read(addr, chipno);
    }

    return 0;
}

void hardsid_store(WORD addr, BYTE val, int chipno)
{
    if (hardsid_is_open) {
        /* write to sidbuf[] for write-only registers */
        if (addr <= 0x18) {
            sidbuf[chipno * 0x20 + addr] = val;
        }
        hardsid_drv_store(addr, val, chipno);
    }
}

void hardsid_set_machine_parameter(long cycles_per_sec)
{
}

int hardsid_available(void)
{
    if (!hardsid_found) {
        hardsid_found = hardsid_drv_available();
    }
    return hardsid_found;
}

void hardsid_set_device(unsigned int chipno, unsigned int device)
{
    if (hardsid_found) {
        hardsid_drv_set_device(chipno, device);
    }
}

/* ---------------------------------------------------------------------*/

void hardsid_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    int i;

    for (i = 0; i < 32; ++i) {
        sid_state->regs[i] = sidbuf[i + (chipno * 0x20)];
    }
    hardsid_drv_state_read(chipno, sid_state);
}

void hardsid_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    int i;

    for (i = 0; i < 32; ++i) {
        sidbuf[i + (chipno * 0x20)] = sid_state->regs[i];
    }
    hardsid_drv_state_write(chipno, sid_state);
}
#endif
