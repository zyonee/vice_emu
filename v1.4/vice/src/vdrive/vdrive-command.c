/*
 * vdrive-command.c - Virtual disk-drive implementation. Command interpreter.
 *
 * Written by
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

#include "vice.h"

/* #define DEBUG_DRIVE */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diskimage.h"
#include "log.h"
#include "serial.h"
#include "types.h"
#include "utils.h"
#include "vdrive-bam.h"
#include "vdrive-command.h"
#include "vdrive-dir.h"
#include "vdrive-iec.h"
#include "vdrive-rel.h"
#include "vdrive.h"

/* RAM/ROM.  */
extern BYTE drive_rom1541[];
extern BYTE drive_rom1541ii[];
extern BYTE drive_rom1571[];
extern BYTE drive_rom1581[];
extern BYTE drive_rom2031[];
extern BYTE drive_rom1001[];

/* If nonzero, the ROM image has been loaded.  */
extern int rom1541_loaded;
extern int rom1541ii_loaded;
extern int rom1571_loaded;
extern int rom1581_loaded;
extern int rom2031_loaded;
extern int rom1001_loaded;

#define IP_MAX_COMMAND_LEN 128 /* real 58 */

errortext_t floppy_error_messages[] =
{
    { 0, " OK"},
    { 1, "FILES SCRATCHED"},
    { 2, "SELECTED PARTITION"},           /* 1581 */
    { 3, "UNIMPLEMENTED"},
    {20, "READ ERROR"},
    {21, "READ ERROR"},
    {22, "READ ERROR"},
    {23, "READ ERROR"},
    {24, "READ ERROR"},
    {25, "WRITE ERROR"},
    {26, "WRITE PROTECT ON"},
    {27, "READ ERROR"},
    {28, "WRITE ERROR"},
    {29, "DISK ID MISMATCH"},
    {30, "SYNTAX ERROR"},
    {31, "SYNTAX ERROR"},
    {32, "SYNTAX ERROR"},
    {33, "SYNTAX ERROR"},
    {34, "SYNTAX ERROR"},
    {39, "SYNTAX ERROR"},
    {50, "RECORD NOT RESENT"},
    {60, "WRITE FILE OPEN"},
    {61, "FILE NOT OPEN"},
    {62, "FILE NOT FOUND"},
    {63, "FILE EXISTS"},
    {64, "FILE TYPE MISMATCH"},
    {65, "NO BLOCK"},
    {66, "ILLEGAL TRACK OR SECTOR"},
    {67, "ILLEGAL SYSTEM T OR S"},
    {70, "NO CHANNEL"},
    {72, "DISK FULL"},
    {73, "VIRTUAL DRIVE EMULATION V2.2"}, /* The program version */
    {74, "DRIVE NOT READY"},
    {77, "SELECTED PARTITION ILLEGAL"},   /* 1581 */
    {80, "DIRECTORY NOT EMPTY"},
    {81, "PERMISSION DENIED"},
    {-1, 0}

};

static log_t vdrive_command_log = LOG_ERR;

static int vdrive_command_block(vdrive_t *vdrive, char command, char *buffer);
static int vdrive_command_memory(vdrive_t *vdrive, BYTE *buffer,
                                 unsigned int length);
static int vdrive_command_initialize(vdrive_t *vdrive);
static int vdrive_command_copy(vdrive_t *vdrive, char *dest, int length);
static int vdrive_command_rename(vdrive_t *vdrive, char *dest, int length);
static int vdrive_command_position(vdrive_t *vdrive, BYTE *buf,
                                   unsigned int length);

void vdrive_command_init(void)
{
    vdrive_command_log = log_open("VDriveCommand");
}

int vdrive_command_execute(vdrive_t *vdrive, BYTE *buf, unsigned int length)
{
    int status = IPE_OK;
    BYTE *p, *p2;
    char *name;
    BYTE *minus;

    if (!length)
        return IPE_OK;
    if (length > IP_MAX_COMMAND_LEN) {
        vdrive_command_set_error(vdrive, IPE_LONG_LINE, 0, 0);
        return IPE_LONG_LINE;
    }

    p = (BYTE *)xmalloc(length + 1);
    memcpy(p, buf, length);

    if (p[length - 1] == 0x0d)
        --length; /* chop CR character */
    p[length] = 0;

    name = (char *)memchr(p, ':', length);
    minus = (BYTE *)memchr(p, '-', length);

    if (name) /* Fix name length */
        for (p2 = p; *p2 && *p2 != ':' && length > 0; p2++, length--);

#ifdef DEBUG_DRIVE
    log_debug("Command %c.", *p);
#endif

    switch (*p) {
      case 'C': /* Copy command.  */
        status = vdrive_command_copy(vdrive, (char *)name, length);
        break;

      case 'D':		/* Backup unused */
        status = IPE_INVAL;
        break;

      case 'R':		/* Rename */
        status = vdrive_command_rename(vdrive, (char *)name, length);
        break;

      case 'S':		/* Scratch */
        {
            BYTE *slot;
            char *realname = name;
            unsigned int reallength = 0, filetype = 0, readmode = 0;

            /* XXX
             * Wrong name parser - s0:file1,file2 means scratch
             * those 2 files.
             */

            if (vdrive_parse_name(name, length, realname, &reallength,
                                  &readmode, &filetype, NULL) != SERIAL_OK) {
                status = IPE_NO_NAME;
            } else if (vdrive->image->read_only) {
                status = IPE_WRITE_PROTECT_ON;
            } else {
#ifdef DEBUG_DRIVE
                log_debug("remove name= '%s' len=%d (%d) type= %d.",
                          realname, reallength, length, filetype);
#endif
                vdrive->deleted_files = 0;

                /* Since vdrive_dir_remove_slot() uses
                 * vdrive_dir_find_first_slot() too, we cannot find the
                 * matching files by simply repeating
                 * vdrive_dir find_next_slot() calls alone; we have to re-call
                 * vdrive_dir_find_first_slot() each time... EP 1996/04/07
                 */

                vdrive_dir_find_first_slot(vdrive, realname, reallength, 0);
                while ((slot = vdrive_dir_find_next_slot(vdrive))) {
                    vdrive_dir_remove_slot(vdrive, slot);
                    vdrive->deleted_files++;
                    vdrive_dir_find_first_slot(vdrive, realname, reallength, 0);
                }
                if (vdrive->deleted_files)
                    status = IPE_DELETED;
                else
                    status = IPE_NOT_FOUND;
                vdrive_command_set_error(vdrive, status, 1, 0);
            } /* else */
        }
        break;

      case 'I':
	status = vdrive_command_initialize(vdrive);
	break;

      case 'N':
        /* Skip ":" at the start of the name.  */
	status = vdrive_command_format(vdrive, name + 1);
	break;

      case 'V':
	status = vdrive_command_validate(vdrive);
	break;

      case 'B': /* Block, Buffer */
        if (!name)	/* B-x does not require a : */
            name = (char *)(p + 2);
        if (!minus)
            status = IPE_INVAL;
	else
	    status = vdrive_command_block(vdrive, minus[1], name + 1);
	break;

      case 'M': /* Memory */
        if (!minus)     /* M-x does not allow a : */
            status = IPE_INVAL;
        else
            status = vdrive_command_memory(vdrive, minus + 1, length);
	break;

      case 'P': /* Position */
	status = vdrive_command_position(vdrive, p + 1, length);
	break;

      case 'U': /* User */
        if (!name)
            name = (char *)(p + 1);
	if (p[1] == '0') {
	    status = IPE_OK;
	} else {
 	    switch ((p[1] - 1) & 0x0f) {
 	      case 0: /* UA */
 	        /* XXX incorrect: U1 is not exactly the same as B-R */
 	        /*      -- should store the buffer pointer */
 	        if (name)
		    status = vdrive_command_block(vdrive, 'R', name + 1);
 	        break;

 	      case 1: /* UB */
 	        /* XXX incorrect: U2 is not exactly the same as B-W */
 	        /*      -- should store the buffer pointer */
 	        if (name)
 		    status = vdrive_command_block(vdrive, 'W', name + 1);
 	        break;

 	      case 2: /* Jumps */
 	      case 3:
 	      case 4:
 	      case 5:
 	      case 6:
 	      case 7:
 	        status = IPE_NOT_READY;
 	        break;

 	      case 8: /* UI */
 	        if (p[2] == '-' || p[2] == '+') {
 		    status = IPE_OK;	/* Set IEC bus speed */
 	        } else {
 		    vdrive_close_all_channels(vdrive); /* Warm reset */
 		    status = IPE_DOS_VERSION;
 	        }
 	        break;

 	      case 9: /* UJ */
 	        vdrive_close_all_channels(vdrive); /* Cold reset */
 	        status = IPE_DOS_VERSION;
 	        break;

 	      case 10: /* UK..UP */
 	      case 11:
 	      case 12:
 	      case 13:
 	      case 14:
 	      case 15:
 	        status = IPE_NOT_READY;
 	        break;
 	    }
	} /* Un */
	break;

      default:
	status = IPE_INVAL;
	break;
    } /* commands */

    if (status == IPE_INVAL)
        log_error(vdrive_command_log, "Wrong command `%s'.", p);

    vdrive_command_set_error(vdrive, status, 0, 0);

    free((char *)p);
    return status;
}

static int vdrive_get_block_parameters(char *buf, int *p1, int *p2, int *p3,
                                       int *p4)
{
    int ip;
    char *bp, endsign;
    int *p[4];	/* This is a kludge */
    p[0] = p1;
    p[1] = p2;
    p[2] = p3;
    p[3] = p4;

    bp = buf;

    for (ip = 0; ip < 4; ip++) {
	while (*bp == ' ' || *bp == ')' || *bp == ',' || *bp == '#')
	    bp++;
	if (*bp == 0)
	    break;
	/* Convert and skip over decimal number.  */
	*p[ip] = strtol(bp, &bp, 10);
    }
    endsign = *bp;
    if (isalnum((int)endsign) && (ip == 4))
	return IPE_SYNTAX;
    return -ip;			/* negative of # arguments found */
}

static int vdrive_command_block(vdrive_t *vdrive, char command, char *buffer)
{
    int channel = 0, drive = 0, track = 0, sector = 0, position = 0;
    int l, rc;

#ifdef DEBUG_DRIVE
    log_debug("vdrive_command_block command:%c.", command);
#endif

    switch (command) {
      case 'R':
      case 'W':
        l = vdrive_get_block_parameters(buffer, &channel, &drive, &track,
                                        &sector);

        if (l < 0) {
#ifdef DEBUG_DRIVE
            log_debug("B-R/W parsed ok. (l=%d) channel %d mode %d, "
                      "drive=%d, track=%d sector=%d.", l, channel,
                      vdrive->buffers[channel].mode, drive, track, sector);
#endif

            if (vdrive->buffers[channel].mode != BUFFER_MEMORY_BUFFER)
                return IPE_NO_CHANNEL;

            if (command == 'W') {
                if (vdrive->image->read_only)
                    return IPE_WRITE_PROTECT_ON;
                if (disk_image_write_sector(vdrive->image,
                                            vdrive->buffers[channel].buffer,
                                            track, sector) < 0)
                    return IPE_NOT_READY;
            } else {
                rc = disk_image_read_sector(vdrive->image,
                                            vdrive->buffers[channel].buffer,
                                            track, sector);
                if (rc > 0)
                    return rc;
                if (rc < 0)
                    return IPE_NOT_READY;
            }
            vdrive->buffers[channel].bufptr = 0;
        } else {
            log_error(vdrive_command_log, "B-R/W invalid parameter "
                      "C:%i D:%i T:%i S:%i.", channel, drive, track, sector);
        }
        break;
      case 'A':
      case 'F':
        l = vdrive_get_block_parameters(buffer, &drive, &track, &sector,
                                        &channel);
        if (l > 0) /* just 3 args used */
            return l;
        if (command == 'A') {
            if (!vdrive_bam_allocate_sector(vdrive->image_format, vdrive->bam,
                track, sector)) {
                /*
                 * Desired sector not free. Suggest another. XXX The 1541
                 * uses an inferior search function that only looks on
                 * higher tracks and can return sectors in the directory
                 * track.
                 */
                if (vdrive_bam_alloc_next_free_sector(vdrive, vdrive->bam,
                    (unsigned int*)&track, (unsigned int *)&sector) >= 0) {
                    /* Deallocate it and merely suggest it */
                    vdrive_bam_free_sector(vdrive->image_format, vdrive->bam,
                                           track, sector);
                } else {
                    /* Found none */
                    track = 0;
                    sector = 0;
                }
                vdrive_command_set_error(vdrive, IPE_NO_BLOCK, track, sector);
                return IPE_NO_BLOCK;
            }
        } else {
            vdrive_bam_free_sector(vdrive->image_format, vdrive->bam,
                                   track, sector);
        }
        break;
      case 'P':
        l = vdrive_get_block_parameters(buffer, &channel, &position, &track,
                                        &sector);
        if (l > 0) /* just 2 args used */
            return l;
        if (vdrive->buffers[channel].mode != BUFFER_MEMORY_BUFFER)
            return IPE_NO_CHANNEL;
        vdrive->buffers[channel].bufptr = position;
        break;
      default:
        return IPE_INVAL;
    }
    return IPE_OK;
}


static int vdrive_command_memory(vdrive_t *vdrive, BYTE *buffer,
                                 unsigned int length)
{
    ADDRESS addr = 0;

    if (length < 3)
        return IPE_SYNTAX;

    addr = buffer[1] | (buffer[2] << 8);

    switch (*buffer) {
#if 0
      case 'W':
        if (length < 5)
            return IPE_SYNTAX;
        count = buffer[3];
        /* data= buffer[4 ... 4+34]; */

        if (vdrive->buffers[addrlo].mode != BUFFER_MEMORY_BUFFER) {
            return IPE_SYNTAX;
        memcpy ( ... , buffer + 4, buffer[3]);
        }
        break;
#endif
      case 'R':
        return vdrive_command_memory_read(vdrive, addr, length);
#if 0
      case 'E':
        break;
#endif
      default:
        return IPE_SYNTAX;
    }
    return IPE_OK;
}

static int vdrive_command_copy(vdrive_t *vdrive, char *dest, int length)
{
    char *name, *files, *p, c;

    /* Split command line */
    if (!dest || !(files = (char*)memchr(dest, '=', length)) )
        return (IPE_SYNTAX);

    *files++ = 0;

    if (strchr (dest, ':'))
        dest = strchr (dest, ':') +1;

#ifdef DEBUG_DRIVE
    log_debug("COPY: dest= '%s', orig= '%s'.", dest, files);
#endif

    if (vdrive_iec_open(vdrive, dest, strlen(dest), 1))
        return (IPE_FILE_EXISTS);

    p = name = files;

    while (*name) { /* Loop for given files.  */
        for (; *p && *p != ','; p++);
        *p++ = 0;

        if (strchr (name, ':'))
            name = strchr (name, ':') +1;

#ifdef DEBUG_DRIVE
        log_debug("searching for file '%s'.", name);
#endif
        if (vdrive_iec_open(vdrive, name, strlen(name), 0)) {
            vdrive_iec_close(vdrive, 1);
            return (IPE_NOT_FOUND);
        }

        while (!vdrive_iec_read(vdrive, (BYTE *)&c, 0)) {
            if (vdrive_iec_write(vdrive, c, 1)) {
                vdrive_iec_close(vdrive, 0); /* No space on disk.  */
                vdrive_iec_close(vdrive, 1);
                return (IPE_DISK_FULL);
            }
        }

        vdrive_iec_close(vdrive, 0);
        name = p; /* Next file.  */
    }
    vdrive_iec_close(vdrive, 1);
    return(IPE_OK);
}


/*
 * Rename disk entry
 */

static int vdrive_command_rename(vdrive_t *vdrive, char *dest, int length)
{
    char *src;
    char dest_name[256], src_name[256];
    unsigned int dest_reallength, dest_readmode = FAM_READ;
    unsigned int dest_filetype, dest_rl;
    unsigned int src_reallength, src_readmode = FAM_READ;
    unsigned int src_filetype, src_rl;
    BYTE *slot;

    if (!dest || !(src = (char*)memchr(dest, '=', length)) )
        return (IPE_SYNTAX);

    *src++ = 0;

    if (strchr (dest, ':'))
        dest = strchr (dest, ':') +1;

#ifdef DEBUG_DRIVE
    log_debug("RENAME: dest= '%s', orig= '%s'.", dest, src);
#endif

    if (!vdrive_parse_name(dest, strlen(dest), dest_name, &dest_reallength,
        &dest_readmode, &dest_filetype, &dest_rl) == FLOPPY_ERROR)
        return IPE_SYNTAX;
    if (!vdrive_parse_name(src, strlen(src), src_name, &src_reallength,
        &src_readmode, &src_filetype, &src_rl) == FLOPPY_ERROR)
        return IPE_SYNTAX;

    if (vdrive->image->read_only)
        return IPE_WRITE_PROTECT_ON;

    /* Check if the destination name is already in use.  */

    vdrive_dir_find_first_slot(vdrive, dest_name, dest_reallength,
                               dest_filetype);
    slot = vdrive_dir_find_next_slot(vdrive);

    if (slot)
        return (IPE_FILE_EXISTS);

    /* Find the file to rename. */

    vdrive_dir_find_first_slot(vdrive, src_name, src_reallength, src_filetype);
    slot = vdrive_dir_find_next_slot(vdrive);

    if (!slot)
        return (IPE_NOT_FOUND);

    /* Now we can replace the old file name...  */
    /* We write directly to the Dir_buffer.  */

    slot = &vdrive->Dir_buffer[vdrive->SlotNumber * 32];
    memset(slot + SLOT_NAME_OFFSET, 0xa0, 16);
    memcpy(slot + SLOT_NAME_OFFSET, dest_name, dest_reallength);
    if (dest_filetype)
        slot[SLOT_TYPE_OFFSET] = dest_filetype; /* FIXME: is this right? */

    /* Update the directory.  */
    if (disk_image_write_sector(vdrive->image, vdrive->Dir_buffer,
        vdrive->Curr_track, vdrive->Curr_sector) < 0)
        return IPE_WRITE_ERROR;

    return IPE_OK;
}

static int vdrive_command_initialize(vdrive_t *vdrive)
{
    vdrive_close_all_channels(vdrive);

    /* Update BAM in memory.  */
    if (vdrive->image != NULL)
        vdrive_bam_read_bam(vdrive);

    return IPE_OK;
}

int vdrive_command_validate(vdrive_t *vdrive)
{
    int t, s, status;
    /* FIXME: size of BAM define */
    BYTE *b, oldbam[5 * 256];

    status = vdrive_command_initialize(vdrive);
    if (status != IPE_OK)
        return status;
    if (vdrive->image->read_only)
        return IPE_WRITE_PROTECT_ON;
    /* FIXME: size of BAM define */
    memcpy(oldbam, vdrive->bam, 5 * 256);

    vdrive_bam_clear_all(vdrive->image_format, vdrive->bam);
    for (t = 1; t <= vdrive->num_tracks; t++) {
        int max_sector;
        max_sector = vdrive_get_max_sectors(vdrive->image_format, t);
        for (s = 0; s < max_sector; s++)
            vdrive_bam_free_sector(vdrive->image_format, vdrive->bam, t, s);
    }

    /* First map out the BAM and directory itself.  */
    status = vdrive_bam_allocate_chain(vdrive, vdrive->Bam_Track,
                                       vdrive->Bam_Sector);
    if (status != IPE_OK) {
	/* FIXME: size of BAM define */
        memcpy(vdrive->bam, oldbam, 5 * 256);
        return status;
    }

    if (vdrive->image_format == VDRIVE_IMAGE_FORMAT_1571) {
        int max_sector;
        max_sector = vdrive_get_max_sectors(vdrive->image_format, 53);
        for (s = 0; s < max_sector; s++)
            vdrive_bam_allocate_sector(vdrive->image_format, vdrive->bam, 53,
                                       s);
    }

    if (vdrive->image_format == VDRIVE_IMAGE_FORMAT_1581) {
        vdrive_bam_allocate_sector(vdrive->image_format, vdrive->bam,
                                   vdrive->Bam_Track, vdrive->Bam_Sector + 1);
        vdrive_bam_allocate_sector(vdrive->image_format, vdrive->bam,
                                   vdrive->Bam_Track, vdrive->Bam_Sector + 2);
    }

    vdrive_dir_find_first_slot(vdrive, "*", 1, 0);

    while ((b = vdrive_dir_find_next_slot(vdrive))) {
        char *filetype = (char *)
        &vdrive->Dir_buffer[vdrive->SlotNumber * 32 + SLOT_TYPE_OFFSET];

        if (*filetype & FT_CLOSED) {
            status = vdrive_bam_allocate_chain(vdrive, b[SLOT_FIRST_TRACK],
                                    b[SLOT_FIRST_SECTOR]);
            if (status != IPE_OK) {
                memcpy(vdrive->bam, oldbam, 256);
                return status;
            }
            /* The real drive always validates side sectors even if the file
               type is not REL.  */
            status = vdrive_bam_allocate_chain(vdrive, b[SLOT_SIDE_TRACK],
                                               b[SLOT_SIDE_SECTOR]);
            if (status != IPE_OK) {
                memcpy(vdrive->bam, oldbam, 256);
                return status;
            }
        } else {
            *filetype = FT_DEL;
            if (disk_image_write_sector(vdrive->image, vdrive->Dir_buffer,
                vdrive->Curr_track, vdrive->Curr_sector) < 0)
                return IPE_WRITE_ERROR;
        }
    }

    /* Write back BAM only if validate was successful.  */
    vdrive_bam_write_bam(vdrive);
    return status;
}

int vdrive_command_format(vdrive_t *vdrive, const char *disk_name)
{
    BYTE tmp[256];
    int status;
    char *name, *comma;
    BYTE id[2];

    if (!disk_name)
        return IPE_SYNTAX;

    if (vdrive->image->read_only)
        return IPE_WRITE_PROTECT_ON;

    if (vdrive->image->fd == NULL)
        return IPE_NOT_READY;

    comma = memchr(disk_name, ',', strlen(disk_name));

    if (comma != NULL) {
        if (comma != disk_name) {
            name = xmalloc(comma - disk_name + 1);
            memcpy(name, disk_name, comma - disk_name);
            name[comma - disk_name] = '\0';
        } else {
            name = stralloc(" ");
        }
        if (comma[1] != '\0') {
            if (comma[2] != '\0') {
                id[0] = comma[1];
                id[1] = comma[2];
            } else {
                id[0] = comma[1];
                id[1] = ' ';
            }
        } else {
            id[0] = id[1] = ' ';
        }
    } else {
        name = stralloc(disk_name);
        id[0] = id[1] = ' ';
    }

    /* Make the first dir-entry.  */
    memset(tmp, 0, 256);
    tmp[1] = 255;

    if (disk_image_write_sector(vdrive->image, tmp, vdrive->Dir_Track,
        vdrive->Dir_Sector) < 0) {
        free(name);
        return IPE_WRITE_ERROR;
    }

    vdrive_bam_create_empty_bam(vdrive, name, id);
    vdrive_bam_write_bam(vdrive);

    /* Validate is called to clear the BAM.  */
    status = vdrive_command_validate(vdrive);

    free(name);

    return status;
}

static int vdrive_command_position(vdrive_t *vdrive, BYTE *buf,
                                   unsigned int length)
{
    unsigned int channel, rec_lo, rec_hi, position;

    if (length < 5)
        return IPE_NO_RECORD;

    channel = buf[0];
    rec_lo = buf[1];
    rec_hi = buf[2];
    position = buf[3];

    if (vdrive->buffers[channel].mode != BUFFER_RELATIVE)
        return IPE_NO_CHANNEL;

    vdrive_rel_position(vdrive, channel, rec_lo, rec_hi, position);

    vdrive->buffers[channel].bufptr = 0;

    return IPE_OK;
}


/* ------------------------------------------------------------------------- */

void vdrive_command_set_error(vdrive_t *vdrive, int code, unsigned int track,
                              unsigned int sector)
{
    const char *message = "";
    errortext_t *e;
    static int last_code;
    bufferinfo_t *p = &vdrive->buffers[15];

#ifdef DEBUG_DRIVE
    log_debug("Set error channel: code =%d, last_code =%d, track =%d, "
              "sector =%d.", code, last_code, track, sector);
#endif

    /* Only set an error once per command */
    if (code != IPE_OK && last_code != IPE_OK)
        return;

    last_code = code;

    if (code != IPE_MEMORY_READ) {
        e = &(floppy_error_messages[0]);
        while (e->nr >= 0 && e->nr != code)
            e++;

        if (e->nr >= 0)
            message = e->text;
        else
            message = "UNKNOWN ERROR NUMBER";

        sprintf((char *)p->buffer, "%02d,%s,%02d,%02d\015",
                code == IPE_DELETED ? vdrive->deleted_files : code,
                message, track, sector);

        /* Length points to the last byte, and doesn't give the length.  */
        p->length = strlen((char *)p->buffer) - 1;
    } else {
        memcpy((char *)p->buffer, vdrive->mem_buf, vdrive->mem_length);
        p->length = vdrive->mem_length - 1;
        message = "MEMORY READ";
    }
    p->bufptr = 0;

    if (code && code != IPE_DOS_VERSION && code != IPE_MEMORY_READ)
        log_message(vdrive_command_log, "ERR = %02d, %s, %02d, %02d",
                    code == IPE_DELETED ? vdrive->deleted_files : code,
                    message, track, sector);

    p->readmode = FAM_READ;
}

int vdrive_command_memory_read(vdrive_t *vdrive, ADDRESS addr,
                               unsigned int length)
{
    unsigned int i;

    if (length == 0 || length > IP_MAX_COMMAND_LEN)
        length = IP_MAX_COMMAND_LEN;

    for (i = 0; i < length; i++) {
        BYTE val = 0;

        if (addr >= 0x8000) {
            switch (vdrive->image_format) {
              case VDRIVE_IMAGE_FORMAT_1541:
                if (rom1541_loaded)
                    val = drive_rom1541[addr & 0x3fff];
                else
                    val = 0x55;
                break;
              case VDRIVE_IMAGE_FORMAT_1571:
                if (rom1571_loaded)
                    val = drive_rom1571[addr & 0x7fff];
                else
                    val = 0x55;
                break;
              case VDRIVE_IMAGE_FORMAT_1581:
                if (rom1581_loaded)
                    val = drive_rom1581[addr & 0x7fff];
                else
                    val = 0x55;
                break;
              case VDRIVE_IMAGE_FORMAT_8050:
              case VDRIVE_IMAGE_FORMAT_8250:
                if (rom1001_loaded)
                    val = drive_rom1001[addr & 0x3fff];
                else
                    val = 0x55;
                break;
            }
        }
        vdrive->mem_buf[i] = val;
        addr++;
    }

    vdrive->mem_length = length - 1;
    return IPE_MEMORY_READ;
}

