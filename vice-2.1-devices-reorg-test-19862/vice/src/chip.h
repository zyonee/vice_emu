/*! \file chip.h \n
 *  \author Spiro Trikaliotis\n
 *  \brief  Chip access interface
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
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

#ifndef VICE_CHIP_H
#define VICE_CHIP_H

#include "snapshot.h"

typedef int           chip_init_cmdline_options_t(void);
typedef int           chip_init_resources_t(void);
typedef void          chip_init_t(void);
typedef void          chip_shutdown_t(void);
typedef void          chip_reset_t(void);
typedef int           chip_snapshot_read_module_t(snapshot_t *p);
typedef int           chip_snapshot_write_module_t(snapshot_t *p);
typedef BYTE          chip_peek_t(WORD addr);
typedef BYTE REGPARM1 chip_read_t(WORD addr);
typedef void REGPARM2 chip_store_t(WORD addr, BYTE byte);


typedef
struct {
    chip_init_cmdline_options_t  * init_cmdline_options;
    chip_init_resources_t        * init_resources;
    chip_init_t                  * init;
    chip_shutdown_t              * shutdown;
    chip_reset_t                 * reset;
    chip_snapshot_write_module_t * snapshot_write_module;
    chip_snapshot_read_module_t  * snapshot_read_module;
    chip_peek_t                  * peek;
    chip_read_t                  * read;
    chip_store_t                 * store;
} vice_chip_interface_t;

typedef
struct {
    vice_chip_interface_t        * chip_interface;
    const unsigned char          * name;
    void                         * context;
    WORD                           address_bitmask;
    WORD                           address_start;
    WORD                           address_end;
    WORD                           address_end_mirrored;
} vice_chip_instance_t;

#define DEFINE_CHIP_INTERFACE(_variable,     \
                      _init_cmdline_options, \
                      _init_resources,       \
                      _init,                 \
                      _shutdown,             \
                      _reset,                \
                      _snapshot_read_module, \
                      _snapshot_write_module,\
                      _peek,                 \
                      _read,                 \
                      _store)                \
    \
 vice_chip_interface_t _variable = { \
                      _init_cmdline_options, \
                      _init_resources,       \
                      _init,                 \
                      _shutdown,             \
                      _reset,                \
                      _snapshot_read_module, \
                      _snapshot_write_module,\
                      _peek,                 \
                      _read,                 \
                      _store                 \
                      };

#define DEFINE_CHIP_INSTANCE(                \
                      _interface,            \
                      _name,                 \
                      _context,              \
                      _address_bitmask,      \
                      _address_start,        \
                      _address_end,          \
                      _address_end_mirrored) \
    \
                      { \
                      _interface,            \
                      _name,                 \
                      _context,              \
                      _address_bitmask,      \
                      _address_start,        \
                      _address_end,          \
                      _address_end_mirrored  \
                      };

#endif

