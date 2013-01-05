/*
 * video_mbuffer.h - multi buffer video routines.  
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#ifndef __video_mbuffer_h
#define __video_mbuffer_h

#define MAX_BUFFERS 8
void mbuffer_init(void *widget, int w, int h, int depth);
unsigned char *mbuffer_get_buffer(void);
struct s_mbufs
{
    long stamp; /* timestamp in usecs */
    int w;
    int h;
    unsigned char *buffer;
    struct s_mbufs *next;
    /*GLu */ unsigned int bindId;	/* XXX Fixme: try to avoid GL specifics */
};

void video_dthread_init(void);
void dthread_lock();
void dthread_unlock();

#endif /* __video_mbuffer_h */
