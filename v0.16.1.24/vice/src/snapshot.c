/*
 * snapshot.c - Implementation of machine snapshot files.
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

#include "vice.h"

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#ifdef __riscos
typedef int off_t;
#else
#include <unistd.h>
#endif
#endif

#include "snapshot.h"

#include "log.h"
#include "utils.h"
#include "zfile.h"

/* ------------------------------------------------------------------------- */

char snapshot_magic_string[] = "VICE Snapshot File\032";

#define SNAPSHOT_MAGIC_LEN              19

struct snapshot_module {
    /* File descriptor.  */
    FILE *file;

    /* Flag: are we writing it?  */
    int write_mode;

    /* Size of the module.  */
    DWORD size;

    /* Offset of the module in the file.  */
    long offset;

    /* Offset of the size field in the file.  */
    long size_offset;
};

struct snapshot {
    /* File descriptor.  */
    FILE *file;

    /* Offset of the first module.  */
    long first_module_offset;

    /* Flag: are we writing it?  */
    int write_mode;
};

/* ------------------------------------------------------------------------- */

int snapshot_write_byte(FILE *f, BYTE b)
{
    if (fputc(b, f) == EOF)
        return -1;

    return 0;
}

int snapshot_write_word(FILE *f, WORD w)
{
    if (snapshot_write_byte(f, w & 0xff) < 0
        || snapshot_write_byte(f, w >> 8) < 0)
        return -1;

    return 0;
}

int snapshot_write_dword(FILE *f, DWORD w)
{
    if (snapshot_write_word(f, w & 0xffff) < 0
        || snapshot_write_word(f, w >> 16) < 0)
        return -1;

    return 0;
}

int snapshot_write_padded_string(FILE *f, const char *s, BYTE pad_char,
                                 int len)
{
    int i, found_zero;
    BYTE c;

    for (i = found_zero = 0; i < len; i++) {
        if (!found_zero && s[i] == 0)
            found_zero = 1;
        c = found_zero ? (BYTE) pad_char : (BYTE) s[i];
        if (snapshot_write_byte(f, c) < 0)
            return -1;
    }

    return 0;
}

int snapshot_write_byte_array(FILE *f, BYTE *b, int len)
{
    int i;

    for (i = 0; i < len; i++)
        if (snapshot_write_byte(f, b[i]) < 0)
            return -1;

    return 0;
}

int snapshot_write_string(FILE *f, const char *s)
{
    int i, len;

    len = s ? (strlen(s) + 1) : 0;	/* length includes nullbyte */

    if (snapshot_write_word(f, len) < 0)
	return -1;

    for (i = 0; i < len; i++)
        if (snapshot_write_byte(f, s[i]) < 0)
            return -1;

    return len + sizeof(WORD);
}

int snapshot_read_byte(FILE *f, BYTE *b_return)
{
    int c;

    c = fgetc(f);
    if (c == EOF)
        return -1;
    *b_return = (BYTE) c;
    return 0;
}

int snapshot_read_word(FILE *f, WORD *w_return)
{
    BYTE lo, hi;

    if (snapshot_read_byte(f, &lo) < 0 || snapshot_read_byte(f, &hi) < 0)
        return -1;

    *w_return = lo | (hi << 8);
    return 0;
}

int snapshot_read_dword(FILE *f, DWORD *dw_return)
{
    WORD lo, hi;

    if (snapshot_read_word(f, &lo) < 0 || snapshot_read_word(f, &hi) < 0)
        return -1;

    *dw_return = lo | (hi << 16);
    return 0;
}

int snapshot_read_byte_array(FILE *f, BYTE *b_return, int size)
{
    int i;

    for (i = 0; i < size; i++)
        if (snapshot_read_byte(f, b_return + i) < 0)
            return -1;

    return 0;
}

int snapshot_read_string(FILE *f, char **s)
{
    int i, len;
    WORD w;
    char *p=NULL;

    /* first free the previous string */
    if (*s) {
	free(*s);
	*s = NULL;	/* don't leave a bogus pointer */
    }

    if (snapshot_read_word(f, &w) < 0)
	return -1;

    len = (int) w;

    if (len) {
        p = xmalloc(len);
        *s = p;

        for (i = 0; i < len; i++) {
            if (snapshot_read_byte(f, (BYTE*)(p+i)) < 0) {
		p[0] = 0;
                return -1;
    	    }
        }
	p[len-1] = 0;	/* just to be save */
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int snapshot_module_write_byte(snapshot_module_t *m, BYTE b)
{
    if (snapshot_write_byte(m->file, b) < 0)
        return -1;

    m->size++;
    return 0;
}

int snapshot_module_write_word(snapshot_module_t *m, WORD w)
{
    if (snapshot_write_word(m->file, w) < 0)
        return -1;

    m->size += 2;
    return 0;
}

int snapshot_module_write_dword(snapshot_module_t *m, DWORD dw)
{
    if (snapshot_write_dword(m->file, dw) < 0)
        return -1;

    m->size += 4;
    return 0;
}

int snapshot_module_write_padded_string(snapshot_module_t *m, const char *s,
                                        BYTE pad_char, int len)
{
    if (snapshot_write_padded_string(m->file, s, pad_char, len) < 0)
        return -1;

    m->size += len;
    return 0;
}

int snapshot_module_write_byte_array(snapshot_module_t *m, BYTE *b, int len)
{
    if (snapshot_write_byte_array(m->file, b, len) < 0)
        return -1;

    m->size += len;
    return 0;
}

int snapshot_module_write_string(snapshot_module_t *m, const char *s)
{
    int len;
    len = snapshot_write_string(m->file, s);
    if (len < 0)
        return -1;

    m->size += len;
    return 0;
}

int snapshot_module_read_byte(snapshot_module_t *m, BYTE *b_return)
{
    if (ftell(m->file) + sizeof(BYTE) > m->offset + m->size)
        return -1;

    return snapshot_read_byte(m->file, b_return);
}

int snapshot_module_read_word(snapshot_module_t *m, WORD *w_return)
{
    if (ftell(m->file) + sizeof(WORD) > m->offset + m->size)
        return -1;

    return snapshot_read_word(m->file, w_return);
}

int snapshot_module_read_dword(snapshot_module_t *m, DWORD *dw_return)
{
    if (ftell(m->file) + sizeof(DWORD) > m->offset + m->size)
        return -1;

    return snapshot_read_dword(m->file, dw_return);
}

int snapshot_module_read_byte_array(snapshot_module_t *m, BYTE *b_return,
                                    int size)
{
    if (ftell(m->file) + size > m->offset + m->size)
        return -1;

    return snapshot_read_byte_array(m->file, b_return, size);
}

int snapshot_module_read_string(snapshot_module_t *m, char **charp_return)
{
    if (ftell(m->file) + sizeof(WORD) > m->offset + m->size)
        return -1;

    return snapshot_read_string(m->file, charp_return);
}

snapshot_module_t *snapshot_module_create(snapshot_t *s,
                                          const char *name,
                                          BYTE major_version,
                                          BYTE minor_version)
{
    snapshot_module_t *m;

    m = xmalloc(sizeof(snapshot_module_t));
    m->file = s->file;
    m->offset = ftell(s->file);
    if (m->offset == -1) {
        free(m);
        return NULL;
    }
    m->write_mode = 1;

    if (snapshot_write_padded_string(s->file, name, 0,
                                     SNAPSHOT_MODULE_NAME_LEN) < 0
        || snapshot_write_byte(s->file, major_version) < 0
        || snapshot_write_byte(s->file, minor_version) < 0
        || snapshot_write_dword(s->file, 0) < 0)
        return NULL;

    m->size = ftell(s->file) - m->offset;
    m->size_offset = ftell(s->file) - sizeof(DWORD);

    return m;
}

snapshot_module_t *snapshot_module_open(snapshot_t *s,
                                        const char *name,
                                        BYTE *major_version_return,
                                        BYTE *minor_version_return)
{
    snapshot_module_t *m;
    char n[SNAPSHOT_MODULE_NAME_LEN];
    unsigned int name_len = strlen(name);

    if (fseek(s->file, s->first_module_offset, SEEK_SET) < 0)
        return NULL;

    m = xmalloc(sizeof(snapshot_module_t));
    m->file = s->file;
    m->write_mode = 0;

    m->offset = s->first_module_offset;

    /* Search for the module name.  This is quite inefficient, but I don't
       think we care.  */
    while (1) {
        if (snapshot_read_byte_array(s->file, (BYTE*)n, SNAPSHOT_MODULE_NAME_LEN) < 0
            || snapshot_read_byte(s->file, major_version_return) < 0
            || snapshot_read_byte(s->file, minor_version_return) < 0
            || snapshot_read_dword(s->file, &m->size))
            goto fail;

        /* Found?  */
        if (memcmp(n, name, name_len) == 0
            && (name_len == SNAPSHOT_MODULE_NAME_LEN || n[name_len] == 0))
            break;

        m->offset += m->size;
        if (fseek(s->file, m->offset, SEEK_SET) < 0)
            goto fail;
    }

    m->size_offset = ftell(s->file) - sizeof(DWORD);

    return m;

fail:
    fseek(s->file, s->first_module_offset, SEEK_SET);
    free(m);
    return NULL;
}

int snapshot_module_close(snapshot_module_t *m)
{
    /* Backpatch module size if writing.  */
    if (m->write_mode
        && (fseek(m->file, m->size_offset, SEEK_SET) < 0
            || snapshot_write_dword(m->file, m->size) < 0))
        return -1;

    /* Skip module.  */
    if (fseek(m->file, m->offset + m->size, SEEK_SET) < 0)
        return -1;

    free(m);
    return 0;
}

/* ------------------------------------------------------------------------- */

snapshot_t *snapshot_create(const char *filename,
                            BYTE major_version, BYTE minor_version,
                            const char *snapshot_machine_name)
{
    FILE *f;
    snapshot_t *s;

    f = fopen(filename, "wb");
    if (f == NULL)
        return NULL;

    /* Magic string.  */
    if (snapshot_write_padded_string(f, snapshot_magic_string,
                                     0, SNAPSHOT_MAGIC_LEN) < 0)
        goto fail;

    /* Version number.  */
    if (snapshot_write_byte(f, major_version) < 0
        || snapshot_write_byte(f, minor_version) < 0)
        goto fail;

    /* Machine.  */
    if (snapshot_write_padded_string(f, snapshot_machine_name, 0,
                                     SNAPSHOT_MACHINE_NAME_LEN) < 0)
        goto fail;

    s = xmalloc(sizeof(snapshot_t));
    s->file = f;
    s->first_module_offset = ftell(f);
    s->write_mode = 1;

    return s;

fail:
    fclose(f);
    unlink(filename);
    return NULL;
}

snapshot_t *snapshot_open(const char *filename,
                          BYTE *major_version_return,
                          BYTE *minor_version_return,
                          const char *snapshot_machine_name)
{
    FILE *f;
    char magic[SNAPSHOT_MAGIC_LEN];
    char read_name[SNAPSHOT_MACHINE_NAME_LEN];
    snapshot_t *s = NULL;
    int machine_name_len;

    f = zfopen(filename, "rb");
    if (f == NULL)
        return NULL;

    /* Magic string.  */
    if (snapshot_read_byte_array(f, (BYTE*)magic, SNAPSHOT_MAGIC_LEN) < 0
        || memcmp(magic, snapshot_magic_string, SNAPSHOT_MAGIC_LEN) != 0)
        goto fail;

    /* Version number.  */
    if (snapshot_read_byte(f, major_version_return) < 0
        || snapshot_read_byte(f, minor_version_return) < 0)
        goto fail;

    /* Machine.  */
    if (snapshot_read_byte_array(f, (BYTE*)read_name,
                                 SNAPSHOT_MACHINE_NAME_LEN) < 0)
        goto fail;

    /* Check machine name.  */
    machine_name_len = strlen(snapshot_machine_name);
    if (memcmp(read_name, snapshot_machine_name, machine_name_len) != 0
        || (machine_name_len != SNAPSHOT_MODULE_NAME_LEN
            && read_name[machine_name_len] != 0)) {
        log_error(LOG_DEFAULT, "SNAPSHOT: Wrong machine type.");
        goto fail;
    }

    s = xmalloc(sizeof(snapshot_t));
    s->file = f;
    s->first_module_offset = ftell(f);
    s->write_mode = 0;

    return s;

fail:
    fclose(f);
    return NULL;
}

int snapshot_close(snapshot_t *s)
{
    int retval;

    if (!s->write_mode) {
        if (zfclose(s->file) == EOF)
            retval = -1;
        else
            retval = 0;
    } else {
        if (fclose(s->file) == EOF)
            retval = -1;
        else
            retval = 0;
    }

    free(s);
    return retval;
}
