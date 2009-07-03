/*
 * vic20cartmem.c -- VIC20 Cartridge memory handling.
 *
 * Written by
 *  Daniel Kahlin <daniel@kahlin.net>
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
#include <stdlib.h>
#include <string.h>

#include "cartridge.h"
#include "megacart.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"

#ifdef WATCOM_COMPILE
#include "../mem.h"
#else
#include "mem.h"
#endif

#include "ram.h"
#include "resources.h"
#include "types.h"
#include "uiapi.h"
#include "vic20-resources.h"
#include "vic20mem.h"
#include "vic20memrom.h"

/* ------------------------------------------------------------------------- */

int mem_cartridge_type = CARTRIDGE_NONE;

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 cartridge_read_io2(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_io2_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_io2(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_io2_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_io3(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_io3_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_io3(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_io3_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_ram123(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_mem_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_ram123(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_mem_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_blk1(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_mem_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_blk1(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_mem_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_blk2(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_mem_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_blk2(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_mem_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_blk3(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_mem_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_blk3(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_mem_store(addr, value);
        break;
    }
}

BYTE REGPARM1 cartridge_read_blk5(WORD addr)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        return megacart_mem_read(addr);
        break;
    }
    return 0xff;
}

void REGPARM2 cartridge_store_blk5(WORD addr, BYTE value)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_mem_store(addr, value);
        break;
    }
}

/* ------------------------------------------------------------------------- */

void cartridge_init(void)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_init();
        break;
    }
}

void cartridge_reset(void)
{
    switch (mem_cartridge_type) {
    case CARTRIDGE_MEGACART:
        megacart_reset();
        break;
    }
}

void cartridge_attach(int type, BYTE *rawcart)
{

    mem_cartridge_type = type;
    switch (type) {
    case CARTRIDGE_MEGACART:
        megacart_config_setup(rawcart);
        break;
    default:
        mem_cartridge_type = CARTRIDGE_NONE;
    }
}

void cartridge_detach(int type)
{
    switch (type) {
    case CARTRIDGE_MEGACART:
        //        megacart_detach();
        break;
    }
    mem_cartridge_type = CARTRIDGE_NONE;
}

/* ------------------------------------------------------------------------- */
