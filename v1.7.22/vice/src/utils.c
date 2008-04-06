/*
 * utils.c - Miscellaneous utility functions.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

/* FIXME: This file should have all the system-specific #ifdefs removed
   someday.  */

#include "vice.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __IBMC__
#include <direct.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archdep.h"
#include "log.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */

/* Like malloc, but abort if not enough memory is available.  */
void *xmalloc(size_t size)
{
    void *p = malloc(size);

    if (p == NULL && size > 0) {
        log_error(LOG_DEFAULT,
                  "xmalloc - virtual memory exhausted: "
                  "cannot allocate %lu bytes.", (unsigned long)size);
        exit(-1);
    }

    return p;
}

/* Like calloc, but abort if not enough memory is available.  */
void *xcalloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);

    if (p == NULL && (size * nmemb) > 0) {
        log_error(LOG_DEFAULT,
                  "xcalloc - virtual memory exhausted: cannot allocate %lux%lu bytes.",
                  (unsigned long)nmemb,(unsigned long)size);
        exit(-1);
    }

    return p;
}

/* Like realloc, but abort if not enough memory is available.  */
void *xrealloc(void *p, size_t size)
{
    void *new_p = realloc(p, size);

    if (new_p == NULL) {
        log_error(LOG_DEFAULT,
                  "xrealloc - virtual memory exhausted: cannot allocate %lu bytes.",
                  (unsigned long)size);
        exit(-1);
    }

    return new_p;
}

/* Malloc enough space for `str', copy `str' into it and return its
   address.  */
char *stralloc(const char *str)
{
    size_t length;
    char *p;

    if (str == NULL) {
        log_error(LOG_DEFAULT, "stralloc() called with NULL pointer.");
        exit(-1);
    }

    length = strlen(str);
    p = (char *)xmalloc(length + 1);

    memcpy(p, str, length + 1);
    return p;
}

/* Malloc a new string whose contents concatenate the arguments until the
   first NULL pointer (max `_CONCAT_MAX_ARGS' arguments).  */
char *concat(const char *s, ...)
{
#define _CONCAT_MAX_ARGS 128
    const char *arg;
    char *newp, *ptr;
    int num_args;
    size_t arg_len[_CONCAT_MAX_ARGS], tot_len;
    int i;
    va_list ap;

    arg_len[0] = tot_len = strlen(s);

    va_start(ap, s);
    for (i = 1;
        i < _CONCAT_MAX_ARGS && (arg = va_arg(ap, const char *)) != NULL;
        i++) {
        arg_len[i] = strlen(arg);
        tot_len += arg_len[i];
    }
    num_args = i;

    newp = (char *)xmalloc(tot_len + 1);

    memcpy(newp, s, arg_len[0]);
    ptr = newp + arg_len[0];

    va_start(ap, s);
    for (i = 1; i < num_args; i++) {
        memcpy(ptr, va_arg(ap, const char *), arg_len[i]);
        ptr += arg_len[i];
    }
    *ptr = '\0';

    va_end(ap);
    return newp;
}

/* Add the first `src_size' bytes of `src' to the end of `buf', which is a
   malloc'ed block of `max_buf_size' bytes of which only the first `buf_size'
   ones are used.  If the `buf' is not large enough, realloc it.  Return a
   pointer to the new block.  */
char *util_bufcat(char *buf, int *buf_size, size_t *max_buf_size,
                  const char *src, int src_size)
{
#define BUFCAT_GRANULARITY 0x1000
    if (*buf_size + src_size > (int)(*max_buf_size)) {
        char *new_buf;

        *max_buf_size = (((*buf_size + src_size) / BUFCAT_GRANULARITY + 1)
                          * BUFCAT_GRANULARITY);
        new_buf = (char *)xrealloc(buf, *max_buf_size);
        buf = new_buf;
    }
    memcpy(buf + *buf_size, src, src_size);
    *buf_size += src_size;
    return buf;
}

/* Remove spaces from start and end of string `s'.  The string is not
   reallocated even if it becomes smaller.  */
void util_remove_spaces(char *s)
{
    char *p;
    size_t l = strlen(s);

    for (p = s; *p == ' '; p++)
        ;

    l -= (p - s);
    memmove(s, p, l + 1);

    if (l > 0) {
        for (p = s + l - 1; l > 0 && *p == ' '; l--, p--)
            ;
        *(p + 1) = '\0';
    }
}

/* Set a new value to the dynamically allocated string *str.
   Returns `-1' if nothing has to be done.  */
int util_string_set(char **str, const char *new_value)
{
    if (*str == NULL) {
        if (new_value != NULL)
            *str = stralloc(new_value);
    } else {
        if (new_value == NULL) {
            free(*str);
            *str = NULL;
        } else {
            /* Skip copy if src and dest are already the same.  */
            if (strcmp(*str, new_value) == 0)
                return -1;

            *str = (char*)xrealloc(*str, strlen(new_value) + 1);
            strcpy(*str, new_value);
        }
    }
    return 0;
}

int util_check_null_string(const char *string)
{
    if (string != NULL && *string != '\0')
        return 0;

    return -1;
}

/* ------------------------------------------------------------------------- */

int util_string_to_long(const char *str, const char **endptr, int base,
                        long *result)
{
    const char *sp, *ep;
    long weight, value;
    long sign;
    char last_letter = 0;       /* Initialize to make compiler happy.  */
    char c;

    if (base > 10)
        last_letter = 'A' + base - 11;

    c = toupper((int) *str);

    if (!isspace((int)c)
        && !isdigit((int)c)
        && (base <= 10 || c > last_letter || c < 'A')
        && c != '+' && c != '-')
        return -1;

    if (*str == '+') {
        sign = +1;
        str++;
    } else if (*str == '-') {
        str++;
        sign = -1;
    } else
        sign = +1;

    for (sp = str; isspace((int)*sp); sp++)
        ;

    for (ep = sp;
         (isdigit((int)*ep)
          || (base > 10
              && toupper((int)*ep) <= last_letter
              && toupper((int)*ep) >= 'A')); ep++)
        ;

    if (ep == sp)
        return -1;

    if (endptr != NULL)
        *endptr = (char *)ep;

    ep--;

    for (value = 0, weight = 1; ep >= sp; weight *= base, ep--) {
        if (base > 10 && toupper((int) *ep) >= 'A')
            value += weight * (toupper((int)*ep) - 'A' + 10);
        else
            value += weight * (int)(*ep - '0');
    }

    *result = value * sign;
    return 0;
}

/* Replace every occurrence of `string' in `s' with `replacement' and return
   the result as a malloc'ed string.  */
char *util_subst(const char *s, const char *string, const char *replacement)
{
    int num_occurrences;
    int total_size;
    size_t s_len = strlen(s);
    size_t string_len = strlen(string);
    size_t replacement_len = strlen(replacement);
    const char *sp;
    char *dp;
    char *result;

    /* First, count the occurrences so that we avoid re-allocating every
       time.  */
    for (num_occurrences = 0, sp = s;
         (sp = strstr(sp, string)) != NULL;
         num_occurrences++, sp += string_len)
        ;

    total_size = s_len - (string_len - replacement_len) * num_occurrences + 1;

    result = (char *) xmalloc(total_size);

    sp = s;
    dp = result;
    do {
        char *f = strstr(sp, string);

        if (f == NULL)
            break;

        memcpy(dp, sp, f - sp);
        memcpy(dp + (f - sp), replacement, replacement_len);
        dp += (f - sp) + replacement_len;
        s_len -= (f - sp) + string_len;
        sp = f + string_len;
        num_occurrences--;
    } while (num_occurrences != 0);

    memcpy(dp, sp, s_len + 1);

    return result;
}

/* ------------------------------------------------------------------------- */

/* Get the current working directory as a malloc'ed string.  */
char *util_get_current_dir(void)
{
#ifdef __riscos
    return GetCurrentDirectory();
#else
    static size_t len = 128;
    char *p = (char *)xmalloc(len);

    while (getcwd(p, len) == NULL) {
        if (errno == ERANGE) {
            len *= 2;
            p = (char *)xrealloc(p, len);
        } else
            return NULL;
    }

    return p;
#endif
}

/* ------------------------------------------------------------------------- */

/* Return the length of an open file in bytes.  */
size_t util_file_length(FILE *fd)
{
    size_t off, filesize;

    off = ftell(fd);
    fseek(fd, 0, SEEK_END);
    filesize = ftell(fd);
    fseek(fd, off, SEEK_SET);
    return filesize;
}

/* Load the first `size' bytes of file named `name' into `dest'.  Return 0 on
   success, -1 on failure.  */
int util_file_load(const char *name, BYTE *dest, size_t size,
                   unsigned int load_flag)
{
    FILE *fd;
    size_t r, length;
    BYTE tmpbuf[2];

    if (util_check_null_string(name)) {
        log_error(LOG_ERR, "No file name given for load_file().");
        return -1;
    }

    fd = fopen(name, MODE_READ);

    if (fd == NULL)
        return -1;

    if (load_flag & UTIL_FILE_LOAD_SKIP_ADDRESS) {
        length = util_file_length(fd);
        if (length & 2) {
            r = fread((void *)tmpbuf, 2, 1, fd);
            if (r < 1) {
                fclose(fd);
                return -1;
            }
        }
    }

    r = fread((void *)dest, size, 1, fd);

    fclose(fd);

    if (r < 1)
        return -1;

    return 0;
}

/* Write the first `size' bytes of `src' into a newly created file `name'.
   If `name' already exists, it is replaced by the new one.  Returns 0 on
   success, -1 on failure.  */
int util_file_save(const char *name, BYTE *src, int size)
{
    FILE *fd;
    size_t r;

    if (util_check_null_string(name)) {
        log_error(LOG_ERR, "No file name given for save_file().");
        return -1;
    }

    fd = fopen(name, MODE_WRITE);

    if (fd == NULL)
        return -1;

    r = fwrite((char *)src, size, 1, fd);

    fclose(fd);

    if (r < 1)
        return -1;

    return 0;
}

int util_file_remove(const char *name)
{
    return unlink(name);
}

/* Input one line from the file descriptor `f'.  FIXME: we need something
   better, like GNU `getline()'.  */
int util_get_line(char *buf, int bufsize, FILE *f)
{
    char *r;
    size_t len;

    r = fgets(buf, bufsize, f);
    if (r == NULL)
        return -1;

    len = strlen(buf);

    if (len > 0) {
        char *p;

        /* Remove trailing newline characters.  */
        /* Remove both 0x0a and 0x0d characters, this solution makes it */
        /* work on all target platforms: Unixes, Win32, DOS, and even for MAC */
        while ((len > 0) && ((*(buf+len-1)==0x0d) || (*(buf+len-1)==0x0a)))
            len--;

        /* Remove useless spaces.  */
        while ((len > 0) && (*(buf + len - 1) == ' '))
            len--;
        for (p = buf; *p == ' '; p++, len--);
        memmove(buf, p, len + 1);
        *(buf + len) = '\0';
    }

    return len;
}

/* Split `path' into a file name and a directory component.  Unlike
   the MS-DOS `fnsplit', the directory does not have a trailing '/'.  */
void util_fname_split(const char *path, char **directory_return,
                      char **name_return)
{
    const char *p;

    if (path == NULL) {
        *directory_return = *name_return = NULL;
        return;
    }

    p = strrchr(path, FSDEV_DIR_SEP_CHR);

#if defined __MSDOS__ || defined WIN32 || defined __OS2__
    /* Both `/' and `\' are valid.  */
    {
        const char *p1;

        p1 = strrchr(path, '\\');
        if (p == NULL || p < p1)
            p = p1;
    }
#endif

    if (p == NULL) {
        if (directory_return != NULL)
            *directory_return = NULL;
        if (name_return != NULL)
            *name_return = stralloc(path);
        return;
    }

    if (directory_return != NULL) {
        *directory_return = (char*)xmalloc((size_t)(p - path + 1));
        memcpy(*directory_return, path, p - path);
        (*directory_return)[p - path] = '\0';
    }

    if (name_return != NULL)
        *name_return = stralloc(p + 1);

    return;
}

/* ------------------------------------------------------------------------- */

int read_dword(FILE *fd, DWORD *buf, size_t num)
{
    int i;
    BYTE *tmpbuf;

    tmpbuf = xmalloc(num);

    if (fread((char *)tmpbuf, num, 1, fd) < 1) {
        free(tmpbuf);
        return -1;
    }

    for (i = 0; i < ((int)(num) / 4); i++)
        buf[i] = (tmpbuf[i * 4] + (tmpbuf[i * 4 + 1] << 8)
            + (tmpbuf[i * 4 + 2] << 16) + (tmpbuf[i * 4 + 3] << 24));

    free(tmpbuf);
    return 0;
}

int write_dword(FILE *fd, DWORD *buf, size_t num)
{
    int i;
    BYTE *tmpbuf;

    tmpbuf = xmalloc(num);

    for (i = 0; i < ((int)(num) / 4); i++) {
        tmpbuf[i * 4] = (BYTE)(buf[i] & 0xff);
        tmpbuf[i * 4 + 1] = (BYTE)((buf[i] >> 8) & 0xff);
        tmpbuf[i * 4 + 2] = (BYTE)((buf[i] >> 16) & 0xff);
        tmpbuf[i * 4 + 3] = (BYTE)((buf[i] >> 24) & 0xff);
    }

    if (fwrite((char *)tmpbuf, num, 1, fd) < 1) {
        free(tmpbuf);
        return -1;
    }

    free(tmpbuf);
    return 0;
}

/* ------------------------------------------------------------------------- */

void util_dword_to_le_buf(BYTE *buf, DWORD data)
{
    buf[0] = (BYTE) (data & 0xff);
    buf[1] = (BYTE) ((data >> 8) & 0xff);
    buf[2] = (BYTE) ((data >> 16) & 0xff);
    buf[3] = (BYTE) ((data >> 24) & 0xff);
}

/* ------------------------------------------------------------------------- */

/* Check for existance of file named `name'.  */
int util_file_exists_p(const char *name)
{
    FILE *f;

    f = fopen(name, MODE_READ);
    if (f != NULL) {
        fclose(f);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

char *util_find_next_line(const char *pos)
{
    char *p = strchr(pos, '\n');

    return (char *) (p == NULL ? pos : p + 1);
}

char *util_find_prev_line(const char *text, const char *pos)
{
    const char *p;

    if (pos - text <= 2)
        return (char *) text;

    for (p = pos - 2; p != text; p--)
        if (*p == '\n')
            break;

    if (*p == '\n')
        p++;

    return (char *) p;
}

/* ------------------------------------------------------------------------- */

/* This code is grabbed from GNU make.  It returns the maximum path length by
   using `pathconf'.  */
#ifdef NEED_GET_PATH_MAX
unsigned int get_path_max(void)
{
    static unsigned int value;

    if (value == 0) {
        long int x = pathconf("/", _PC_PATH_MAX);

        if (x > 0)
            value = x;
        else
            return MAXPATHLEN;
    }

    return value;
}
#endif

/* The following are replacements for libc functions that could be missing.  */

#if !defined HAVE_MEMMOVE

void *memmove(void *target, const void *source, unsigned int length)
{
    char *tptr = (char *) target;
    const char *sptr = (const char *) source;

    if (tptr > sptr) {
        tptr += length;
        sptr += length;
        while (length--)
            *(--tptr) = *(--sptr);
    } else if (tptr < sptr) {
        while (length--)
            *(tptr++) = *(sptr++);
    }

    return target;
}

#endif /* !defined HAVE_MEMMOVE */

#if !defined HAVE_ATEXIT

static void atexit_support_func(int status, void *arg)
{
    void (*f)(void) = (void (*)(void))arg;

    f();
}

int atexit(void (*function)(void))
{
    return on_exit(atexit_support_func, (void *)function);
}

#endif /* !defined HAVE_ATEXIT */

#if !defined HAVE_STRERROR

char *strerror(int errnum)
{
    static char buffer[100];

    sprintf(buffer, "Error %d", errnum);
    return buffer;
}

#endif /* !defined HAVE_STRERROR */

/* The following `strcasecmp()' and `strncasecmp()' implementations are
   taken from:
   GLIB - Library of useful routines for C programming
   Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh
   MacDonald.
   The source is available from http://www.gtk.org/.  */

#if !defined HAVE_STRCASECMP

int strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;

    if (s1 == NULL || s2 == NULL)
        return 0;

    while (*s1 && *s2) {
        /* According to A. Cox, some platforms have islower's that don't work
           right on non-uppercase.  */
        c1 = isupper ((unsigned int)*s1) ? tolower ((unsigned int)*s1) : *s1;
        c2 = isupper ((unsigned int)*s2) ? tolower ((unsigned int)*s2) : *s2;
        if (c1 != c2)
            return (c1 - c2);
        s1++; s2++;
    }

    return (((int)(unsigned char)*s1) - ((int)(unsigned char)*s2));
}

#endif

#if !defined HAVE_STRNCASECMP

int strncasecmp(const char *s1, const char *s2, unsigned int n)
{
    int c1, c2;

    if (s1 == NULL || s2 == NULL)
        return 0;

    while (n-- && *s1 && *s2) {
        /* According to A. Cox, some platforms have islower's that don't work
           right on non-uppercase.  */
        c1 = isupper ((unsigned int)*s1) ? tolower ((unsigned int)*s1) : *s1;
        c2 = isupper ((unsigned int)*s2) ? tolower ((unsigned int)*s2) : *s2;
        if (c1 != c2)
            return (c1 - c2);
        s1++; s2++;
    }

    if (n)
        return (((int)(unsigned char) *s1) - ((int)(unsigned char) *s2));
    else
        return 0;
}

#endif

/* ------------------------------------------------------------------------- */

/* util_add_extension() add the extension if not already there.
   If the extension is added `name' is realloced. */

void util_add_extension(char **name, const char *extension)
{
    size_t name_len, ext_len;

    name_len = strlen(*name);
    ext_len = strlen(extension);

    if (ext_len == 0)
        return;

    if ((name_len > ext_len + 1)
        && (strcasecmp(&((*name)[name_len - ext_len]), extension) == 0))
        return;

    *name = xrealloc(*name, name_len + ext_len + 2);
    (*name)[name_len] = FSDEV_EXT_SEP_CHR;
    memcpy(&((*name)[name_len + 1]), extension, ext_len + 1);
}

/* Like util_add_extension() but a const filename is passed.  */
char *util_add_extension_const(const char *filename, const char *extension)
{
    char *ext_filename;

    ext_filename = stralloc(filename);

    util_add_extension(&ext_filename, extension);

    return ext_filename;
}

/* ------------------------------------------------------------------------- */

/* xmsprintf() is like sprintf() but xmalloc's the buffer by itself.  */

#define xmvsprintf_is_digit(c) ((c) >= '0' && (c) <= '9')

static int xmvsprintf_skip_atoi(const char **s)
{
    int i = 0;

    while (xmvsprintf_is_digit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */

inline int xmvsprintf_do_div(long *n, unsigned int base)
{
    int __res;
    __res = ((unsigned long) *n) % (unsigned) base;
    *n = ((unsigned long) *n) / (unsigned) base;
    return __res;
}

static size_t xmvsprintf_strnlen(const char * s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

static void xmvsprintf_add(char **buf, unsigned int *bufsize,
                           unsigned int *position, char write)
{
    if (*position == *bufsize) {
        *bufsize *= 2;
        *buf = xrealloc(*buf, *bufsize);
    }
    (*buf)[*position] = write;
    *position += 1;
}

static void xmvsprintf_number(char **buf, unsigned int *bufsize,
                              unsigned int *position, long num, int base,
                              int size, int precision, int type)
{
    char c, sign, tmp[66];
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }
    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else while (num != 0)
        tmp[i++] = digits[xmvsprintf_do_div(&num, base)];
    if (i > precision)
        precision = i;
        size -= precision;
    if (!(type & (ZEROPAD + LEFT)))
        while(size-->0)
            xmvsprintf_add(buf, bufsize, position, ' ');
    if (sign)
        xmvsprintf_add(buf, bufsize, position, sign);
    if (type & SPECIAL) {
        if (base == 8)
            xmvsprintf_add(buf, bufsize, position, '0');
        else if (base == 16) {
            xmvsprintf_add(buf, bufsize, position, '0');
            xmvsprintf_add(buf, bufsize, position, digits[33]);
        }
    }
    if (!(type & LEFT))
        while (size-- > 0)
            xmvsprintf_add(buf, bufsize, position, c);
    while (i < precision--)
        xmvsprintf_add(buf, bufsize, position, '0');
    while (i-- > 0)
        xmvsprintf_add(buf, bufsize, position, tmp[i]);
    while (size-- > 0)
        xmvsprintf_add(buf, bufsize, position, ' ');
}

char *xmvsprintf(const char *fmt, va_list args)
{
    char *buf;
    unsigned int position, bufsize;

    int len, i, base;
    unsigned long num;
    const char *s;

    int flags;        /* flags to number() */
    int field_width;  /* width of output field */
    int precision;    /* min. # of digits for integers; max
                         number of chars for from string */
    int qualifier;    /* 'h', 'l', or 'L' for integer fields */

    /* Setup the initial buffer.  */
    buf = xmalloc(10);
    position = 0;
    bufsize = 10;

    for ( ; *fmt ; ++fmt) {
        if (*fmt != '%') {
            xmvsprintf_add(&buf, &bufsize, &position, *fmt);
            continue;
        }

        /* process flags */
        flags = 0;
repeat:
        ++fmt;  /* this also skips first '%' */
        switch (*fmt) {
          case '-':
            flags |= LEFT;
            goto repeat;
          case '+':
            flags |= PLUS;
            goto repeat;
          case ' ':
            flags |= SPACE;
            goto repeat;
          case '#':
            flags |= SPECIAL;
            goto repeat;
          case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (xmvsprintf_is_digit(*fmt))
            field_width = xmvsprintf_skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (xmvsprintf_is_digit(*fmt))
                precision = xmvsprintf_skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                 /* it's the next argument */
                 precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            ++fmt;
        }

        /* default base */
        base = 10;

        switch (*fmt) {
          case 'c':
            if (!(flags & LEFT))
                while (--field_width > 0)
                    xmvsprintf_add(&buf, &bufsize, &position, ' ');
            xmvsprintf_add(&buf, &bufsize, &position,
                           (unsigned char) va_arg(args, int));
            while (--field_width > 0)
                xmvsprintf_add(&buf, &bufsize, &position, ' ');
            continue;

          case 's':
            s = va_arg(args, char *);
            if (!s)
                s = "<NULL>";

            len = xmvsprintf_strnlen(s, precision);

            if (!(flags & LEFT))
                while (len < field_width--)
                    xmvsprintf_add(&buf, &bufsize, &position, ' ');
            for (i = 0; i < len; ++i)
                xmvsprintf_add(&buf, &bufsize, &position, *s++);
            while (len < field_width--)
                xmvsprintf_add(&buf, &bufsize, &position, ' ');
            continue;

          case 'p':
            if (field_width == -1) {
                field_width = 2*sizeof(void *);
                flags |= ZEROPAD;
            }
            xmvsprintf_number(&buf, &bufsize, &position,
                              (unsigned long) va_arg(args, void *), 16,
                              field_width, precision, flags);
            continue;

          case '%':
            xmvsprintf_add(&buf, &bufsize, &position, '%');
            continue;

          /* integer number formats - set up the flags and "break" */
          case 'o':
            base = 8;
            break;
          case 'X':
            flags |= LARGE;
          case 'x':
            base = 16;
            break;
          case 'd':
          case 'i':
            flags |= SIGN;
          case 'u':
            break;

          default:
            xmvsprintf_add(&buf, &bufsize, &position, '%');
            if (*fmt)
                xmvsprintf_add(&buf, &bufsize, &position, *fmt);
            else
                --fmt;
            continue;
        }
        if (qualifier == 'l')
            num = va_arg(args, unsigned long);
        else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
            num = (short) num;
        } else if (flags & SIGN)
            num = va_arg(args, int);
        else
            num = va_arg(args, unsigned int);
        xmvsprintf_number(&buf, &bufsize, &position, num, base, field_width,
                          precision, flags);
    }
    xmvsprintf_add(&buf, &bufsize, &position, '\0');

    /* Trim buffer to final size.  */
    buf = xrealloc(buf, strlen(buf) + 1);

    return buf;
}

char *xmsprintf(const char *fmt, ...)
{
    va_list args;
    char *buf;

    va_start(args, fmt);
    buf = xmvsprintf(fmt, args);
    va_end(args);

    return buf;
}

