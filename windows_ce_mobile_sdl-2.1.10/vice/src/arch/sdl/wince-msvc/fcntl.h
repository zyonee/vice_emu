/*
 * fcntl.h - fcntl.h replacement for Windows CE.
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

#ifndef VICE_FCNTL_H
#define VICE_FCNTL_H

#define F_SETFL         1
#define F_SETFD         2
#define F_GETFL         3

#define _O_RDONLY       0x0000
#define _O_WRONLY       0x0001
#define _O_RDWR         0x0002

#define _O_NONBLOCK     0x0004

#define _O_APPEND       0x0008
#define _O_CREAT        0x0100
#define _O_TRUNC        0x0200
#define _O_EXCL         0x0400
#define _O_TEXT         0x4000
#define _O_BINARY       0x8000
#define _O_ACCMODE      0x10000

#define _O_NOINHERIT    0

#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR

#define O_NONBLOCK      _O_NONBLOCK

#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_ACCMODE       _O_ACCMODE

extern int fcntl(int fd, int mode, int flag);

#endif
