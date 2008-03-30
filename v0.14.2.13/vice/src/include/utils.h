/*
 * utils.h - Miscellaneous utility functions.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>

extern char *stralloc(const char *str);
extern void *xmalloc(size_t s);
extern void *xrealloc(void *p, size_t s);
extern char *concat(const char *s1, ...);
extern char *bufcat(char *buf, int *buf_size, int *max_buf_size,
		    const char *src, int src_size);
extern void remove_spaces(char *s);
extern char *make_backup_filename(const char *fname);
extern int make_backup_file(const char *fname);
extern char *get_current_dir(void);
extern unsigned long file_length(int fd);
extern int spawn(const char *name, char **argv, const char *stdout_redir,
		 const char *stderr_redir);
extern int load_file(const char *name, void *dest, int size);
extern int save_file(const char *name, const void *src, int size);

int string_to_long(const char *str, char **endptr, int base,
		   long *result);
    
#if !defined HAVE_MEMMOVE
extern void *memmove(void *target, const void *source, unsigned int length);
#endif

#if !defined HAVE_ATEXIT
extern int atexit(void (*function)(void));
#endif

#endif /* UTILS_H */
