/*
 * gpio1990.c - gpio emulation for the 1990 disk drive.
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

#include "drivetypes.h"
#include "gpio1990.h"
#include "snapshot.h"

struct gpio1990_s {
    char *myname;
    struct drive_context_s *mycontext;
    unsigned char reg;
};


BYTE gpio1990_read(drive_context_t *drv, WORD address)
{
    return drv->gpio1990->reg;
}

void gpio1990_store(drive_context_t *drv, WORD address,
                                         BYTE value)
{
    drv->drive->led_status = (~value & 0x03);
    drv->gpio1990->reg = value;
}

void gpio1990_reset(gpio1990_t *gpio)
{
    gpio->mycontext->drive->led_status = 0x03;
    gpio->reg = 0;
}

void gpio1990_init(drive_context_t *drv)
{
}

void gpio1990_setup_context(drive_context_t *drv)
{
    drv->gpio1990 = lib_calloc(1, sizeof(gpio1990_t));
    drv->gpio1990->myname = lib_msprintf("GPIO1770%d", drv->mynumber);
    drv->gpio1990->mycontext = drv;
}

void gpio1990_shutdown(gpio1990_t *gpio)
{
    lib_free(gpio->myname);
    lib_free(gpio);
}

#define GPIO1990_SNAP_MAJOR 1
#define GPIO1990_SNAP_MINOR 0

int gpio1990_snapshot_write_module(gpio1990_t *drv, struct snapshot_s *s)
{
    snapshot_module_t *m;
    int res = -1;

    m = snapshot_module_create(s, drv->myname,
                               GPIO1990_SNAP_MAJOR, GPIO1990_SNAP_MINOR);
    if (m) {
        res = SMW_B(m, drv->reg);
        if (snapshot_module_close(m) < 0) {
            res = -1;
        }
    }

    return res;
}

int gpio1990_snapshot_read_module(gpio1990_t *drv, struct snapshot_s *s)
{
    BYTE vmajor, vminor;
    snapshot_module_t *m;
    int res = -1;

    m = snapshot_module_open(s, drv->myname, &vmajor, &vminor);

    if (m) {
        if (vmajor == GPIO1990_SNAP_MAJOR && vminor == GPIO1990_SNAP_MINOR) {
            res = SMR_B(m, &drv->reg);
            gpio1990_store(drv->mycontext, 0, drv->reg);
        }
        if (snapshot_module_close(m) < 0) {
            res = -1;
        }
    }

    return res;
}
