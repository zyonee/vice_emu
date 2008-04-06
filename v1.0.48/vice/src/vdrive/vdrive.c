/*
 * vdrive.c - Virtual disk-drive implementation.
 *
 * Written by
 *  Andreas Boose       <boose@linux.rz.fh-hannover.de>
 *
 * Based on old code by
 *  Teemu Rantanen      <tvr@cs.hut.fi>
 *  Jarkko Sonninen     <sonninen@lut.fi>
 *  Jouko Valta         <jopi@stekt.oulu.fi>
 *  Olaf Seibert        <rhialto@mbfys.kun.nl>
 *  Andr� Fachat        <a.fachat@physik.tu-chemnitz.de>
 *  Ettore Perazzoli    <ettore@comm2000.it>
 *  Martin Pottendorfer <Martin.Pottendorfer@aut.alcatel.at>
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

#include <string.h>

#ifdef __riscos
#include "ROlib.h"
#include "ui.h"
#endif

#include "archdep.h"
#include "charsets.h"
#include "drive.h"
#include "fsdevice.h"
#include "log.h"
#include "serial.h"
#include "utils.h"
#include "vdrive-bam.h"
#include "vdrive-dir.h"
#include "vdrive-iec.h"
#include "vdrive.h"

/* ------------------------------------------------------------------------- */

log_t vdrive_log = LOG_ERR;

int speed_map_1541[42] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                           3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1,
                           1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0 };

int speed_map_1571[70] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                           3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1,
                           1, 1, 1, 1, 0, 0, 0, 0, 0,
                           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                           3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1,
                           1, 1, 1, 1, 0, 0, 0, 0, 0 };

/* Keeps the number of entries deleted with the `S' command */
static int deleted_files;

/* PC64 files need this too */
char const *slot_type[] = {
    "DEL", "SEQ", "PRG", "USR", "REL", "CBM", "DJJ", "FAB"
};

static int compare_filename (char *name, char *pattern);
static int vdrive_command_block(vdrive_t *vdrive, char command, char *buffer);
static int vdrive_command_memory(vdrive_t *vdrive, BYTE *buffer, int length);
static int vdrive_command_initialize(vdrive_t *vdrive);
static int vdrive_command_format(vdrive_t *vdrive, const char *name, BYTE *id,
                                 BYTE *minus);
static int vdrive_command_copy(vdrive_t *vdrive, char *dest, int length);
static int vdrive_command_rename(vdrive_t *vdrive, char *dest, int length);
static void vdrive_set_disk_geometry(vdrive_t *vdrive, int type);

/* ------------------------------------------------------------------------- */

/*
 * Error messages
 */

errortext_t floppy_error_messages[] =
{
    { 0, "OK"},
    { 1, "FILES SCRATCHED"},
    { 2, "SELECTED PARTITION"},           /* 1581 */
    { 3, "UNIMPLEMENTED"},
    {26, "WRITE PROTECT ON"},
    {30, "SYNTAX ERROR"},
    {31, "SNERR INVALID COMMAND"},
    {32, "SNERR LINE TOO LONG"},
    {33, "SNERR INVAL FILE NAME"},
    {34, "SNERR NO FILE NAME"},
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

/* ------------------------------------------------------------------------- */

int vdrive_setup_device(vdrive_t *vdrive, int unit)
{
    int i;

    if (vdrive_log == LOG_ERR)
        vdrive_log = log_open("VDrive");

    vdrive->unit = unit;

    for (i = 0; i < 15; i++)
        vdrive->buffers[i].mode = BUFFER_NOT_IN_USE;

    vdrive->buffers[15].mode = BUFFER_COMMAND_CHANNEL;
    vdrive->buffers[15].buffer = (BYTE *)xmalloc(256);

    vdrive_command_set_error(&vdrive->buffers[15], IPE_DOS_VERSION, 0, 0);
    return 0;
}

/* ------------------------------------------------------------------------- */

/*
 * Should set values to somewhere so that they could be read from
 * command channel
 */
void vdrive_command_set_error(bufferinfo_t *p, int code, int track, int sector)
{
    const char   *message;
    errortext_t *e;
    static int last_code;

#ifdef DEBUG_DRIVE
    log_debug("Set error channel: code =%d, last_code =%d, track =%d, "
              "sector =%d.", code, last_code, track, sector);
#endif

    /* Only set an error once per command */
    if (code != IPE_OK && last_code != IPE_OK)
        return;

    last_code = code;

    e = &(floppy_error_messages[0]);
    while (e->nr >= 0 && e->nr != code)
        e++;

    if (e->nr >= 0)
        message = e->text;
    else
        message = "UNKNOWN ERROR NUMBER";

    sprintf((char *)p->buffer, "%02d,%s,%02d,%02d\015",
            code == IPE_DELETED ? deleted_files : code,
            message, track, sector);

    /* length points to the last byte, and doesn't give the length... */
    p->length = strlen((char *)p->buffer)-1;
    p->bufptr = 0;

    if (code && code != IPE_DOS_VERSION)
        log_message(vdrive_log, "ERR = %02d, %s, %02d, %02d",
                    code == IPE_DELETED ? deleted_files : code,
                    message, track, sector);

    p->readmode = FAM_READ;
}


/* ------------------------------------------------------------------------- */

/*
 * Input Processor Simulator. IP interprets incoming commands and generates
 * the error messages according to return values.
 */

int vdrive_command_execute(vdrive_t *vdrive, BYTE *buf, int length)
{
    int status = IPE_OK;
    BYTE *p, *p2;
    char *name;
    BYTE *minus, *id;

    if (!length)
        return IPE_OK;
    if (length > IP_MAX_COMMAND_LEN) {
        vdrive_command_set_error(&vdrive->buffers[15], IPE_LONG_LINE, 0, 0);
        return IPE_LONG_LINE;
    }

    p = (BYTE *)xmalloc(length + 1);
    memcpy(p, buf, length);

    if (p[length - 1] == 0x0d)
        --length; /* chop CR character */
    p[length] = 0;

    name = (char *)memchr(p, ':', length);
    id   = (BYTE *)memchr(p, ',', length);
    minus= (BYTE *)memchr(p, '-', length);

    if (name) /* Fix name length */
        for (p2 = p; *p2 && *p2 != ':' && length > 0; p2++, length--);

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
            int reallength = 0, filetype = 0, readmode = 0;

            /* XXX
             * Wrong name parser - s0:file1,file2 means scratch
             * those 2 files.
             */

            if (vdrive_parse_name(name, length, realname, &reallength,
                                  &readmode, &filetype, NULL) != SERIAL_OK) {
                status = IPE_NO_NAME;
            } else if (vdrive->read_only) {
                status = IPE_WRITE_PROTECT_ON;
            } else {
#ifdef DEBUG_DRIVE
                log_debug("remove name= '%s' len=%d (%d) type= %d.",
                          realname, reallength, length, filetype);
#endif
                deleted_files = 0;

                /* Since vdrive_dir_remove_slot() uses 
                 * vdrive_dir_find_first_slot() too, we cannot find the
                 * matching files by simply repeating 
                 * vdrive_dir find_next_slot() calls alone; we have to re-call
                 * vdrive_dir_find_first_slot() each time... EP 1996/04/07 
                 */

                vdrive_dir_find_first_slot(vdrive, realname, reallength, 0);
                while ((slot = vdrive_dir_find_next_slot(vdrive))) {
                    vdrive_dir_remove_slot(vdrive, slot);
                    deleted_files++;
                    vdrive_dir_find_first_slot(vdrive, realname, reallength, 0);
                }
                if (deleted_files)
                    status = IPE_DELETED;
                else
                    status = IPE_NOT_FOUND;
                vdrive_command_set_error(&vdrive->buffers[15], status, 1, 0);
            } /* else */
        }
        break;

      case 'I':
	status = vdrive_command_initialize(vdrive);
	break;

      case 'N':
	status = vdrive_command_format(vdrive, name, id, minus);
	break;

      case 'V':
	status = vdrive_command_validate(vdrive);
	break;

      case 'B':		/* Block, Buffer */
        if (!name)	/* B-x does not require a : */
            name = (char *)(p + 2);
        if (!minus)
            status = IPE_INVAL;
	else
	    status = vdrive_command_block(vdrive, minus[1], name + 1);
	break;

      case 'M':		/* Memory */
        if (!minus)     /* M-x does not allow a : */
            status = IPE_INVAL;
        else
            status = vdrive_command_memory(vdrive, minus +1, length);
	break;

      case 'P':		/* Position */
	/* 4 byte parameters: channel, rec_lo, rec_hi, pos */
	break;

      case 'U':		/* User */
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
        log_error(vdrive_log, "Wrong command `%s'.", p);

    vdrive_command_set_error(&vdrive->buffers[15], status, 0, 0);

    free((char *)p);
    return status;
}

/*
 * This function is modeled after BLKPAR, in the 1541 ROM at CC6F.
 */

static int  get_block_parameters(char *buf, int *p1, int *p2, int *p3, int *p4)
{
    int ip;
    char *bp;
    int *p[4];	/* This is a kludge */
    p[0] = p1;
    p[1] = p2;
    p[2] = p3;
    p[3] = p4;

    bp = buf;

    for (ip = 0; ip < 4; ip++) {
	while (*bp == ' ' || *bp == ')' || *bp == ',')
	    bp++;
	if (*bp == 0)
	    break;
	/* convert and skip over decimal number */
	*p[ip] = strtol(bp, &bp, 10);
    }

    if ((*bp != 0) && (ip == 4))
	return IPE_SYNTAX;
    return -ip;			/* negative of # arguments found */
}

static int vdrive_command_block(vdrive_t *vdrive, char command, char *buffer)
{
    int channel = 0, drive = 0, track = 0, sector = 0, position = 0;
    int l;

    switch (command) {
      case 'R':
      case 'W':
        l = get_block_parameters(buffer, &channel, &drive, &track, &sector);

        if (l < 0) {
#ifdef DEBUG_DRIVE
            log_debug("B-R/W parsed ok. (l=%d) channel %d mode %d, "
                      "drive=%d, track=%d sector=%d.", l, channel,
                      vdrive->buffers[channel].mode, drive, track, sector);
#endif

            if (vdrive->buffers[channel].mode != BUFFER_MEMORY_BUFFER)
                return IPE_NO_CHANNEL;

            if (command == 'W') {
                if (vdrive->read_only)
                    return IPE_WRITE_PROTECT_ON;
                if (disk_image_write_sector(vdrive->image,
                                            vdrive->buffers[channel].buffer,
                                            track, sector) < 0)
                    return IPE_NOT_READY;
            } else {
                if (disk_image_read_sector(vdrive->image,
                                      vdrive->buffers[channel].buffer,
                                      track, sector) < 0)
                    return IPE_NOT_READY;
            }
            vdrive->buffers[channel].bufptr = 0;
        }
        break;
      case 'A':
      case 'F':
        l = get_block_parameters(buffer, &drive, &track, &sector, &channel);
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
                    &track, &sector) >= 0) {
                    /* Deallocate it and merely suggest it */
                    vdrive_bam_free_sector(vdrive->image_format, vdrive->bam,
                                           track, sector);
                } else {
                    /* Found none */
                    track = 0;
                    sector = 0;
                }
                vdrive_command_set_error(&vdrive->buffers[15], IPE_NO_BLOCK,
                                         track, sector);
                return IPE_NO_BLOCK;
            }
        } else {
            vdrive_bam_free_sector(vdrive->image_format, vdrive->bam,
                                   track, sector);
        }
        break;
      case 'P':
        l = get_block_parameters(buffer, &channel, &position, &track, &sector);
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


static int vdrive_command_memory(vdrive_t *vdrive, BYTE *buffer, int length)
{
#if 0
    int addr = 0;

    if (length < 3)		/* All commands at least 3 bytes */
        return IPE_SYNTAX;

    addr = (buffer[1] & 0xff) | ((buffer[2] & 0xff) << 8);

    switch (*buffer) {
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
      case 'R':
        vdrive->buffers[channel].bufptr = addr;
        break;
      case 'E':
        vdrive->buffers[channel].bufptr = addr;
        break;
      default:
        return IPE_SYNTAX;
    }
    return IPE_OK;
#else
    return IPE_UNIMPL;
#endif
}

/* ------------------------------------------------------------------------- */

/*
 * Parse name realname, type and read/write mode. '@' on write must
 * be checked elsewhere
 */

int vdrive_parse_name(const char *name, int length, char *ptr,
                      int *reallength, int *readmode, int *filetype,
                      int *rl)
{
    const char *p;
    char *c;
    int t;

    if (!name || !*name)
        return FLOPPY_ERROR;

    p = (char *)memchr(name, ':', length);
    if (p)
        p++;
    else {	/* no colon found */
        if (*name != '$')
            p = name;
        else
            p = name + strlen(name);	/* set to null byte */
    }
#ifdef DEBUG_DRIVE
    log_debug("Name (%d): '%s'.", length, p);
#endif

#if 0
    if (*name == '@' && p == name)
        p++;
#endif

    t = strlen(p);
    *reallength = 0;

    while (*p != ',' && t-- > 0) {
	(*reallength)++;
	*(ptr++) = *(p++);	/* realname pointer */
#ifdef DEBUG_DRIVE
	log_debug("parsing... [%d] %02x  t=%d.", *reallength, *(ptr-1), t);
#endif
    }  /* while */


    *filetype = 0;		/* EP */

    /*
     * Change modes ?
     */
    while (t > 0) {
	t--;
	p++;

	if (t == 0) {
#ifdef DEBUG_DRIVE
	    log_debug("done. [%d] %02x  t=%d.", *reallength, *p, t);
	    log_debug("No type.");
#endif
	    return FLOPPY_ERROR;
	}

	switch (*p) {
	  case 'S':
	    *filetype = FT_SEQ;
	    break;
	  case 'P':
	    *filetype = FT_PRG;
	    break;
	  case 'U':
	    *filetype = FT_USR;
	    break;
	  case 'L':			/* L,(#record length)  max 254 */
	    if (rl && p[1] == ',') {
		*rl = p[2];		/* Changing RL causes error */

		if (*rl > 254)
		    return FLOPPY_ERROR;
	    }
	    *filetype = FT_REL;
	    break;


	    /* 1581 types */

	  case 'C':
	    *filetype = FT_CBM;		/* 1581 partition */
	    break;
	  case 'J':
	    *filetype = FT_DJJ;
	    break;
	  case 'F':
	    *filetype = FT_FAB;		/* Fred's format */
	    break;


	    /* Access Control Methods */

	  case 'R':
	    *readmode = FAM_READ;
	    break;
	  case 'W':
	    *readmode = FAM_WRITE;
	    break;
	  case 'A':
	    if (*filetype != FT_SEQ)
		return FLOPPY_ERROR;
	    *readmode = FAM_APPEND;		/* Append on a SEQ file */
	    break;

	  default:
#ifdef DEBUG_DRIVE
	    log_debug("No way. p='%s'.", p);
#endif
	    return FLOPPY_ERROR;
	}

	c = (char *)memchr(p, ',', t);

	if (c) {
	    t -= (c - p);
	    p = c;
	} else
	    t = 0;
    }  /* while (t) */

#ifdef DEBUG_DRIVE
    log_debug("Type = %s  %s.", slot_type[*filetype],
	    (*readmode == FAM_READ ?  "read" : "write"));
#endif

    return FLOPPY_COMMAND_OK;
}

/* ------------------------------------------------------------------------- */


static int vdrive_command_copy(vdrive_t *vdrive, char *dest, int length)
{
    char *name, *files, *p, c;

    /* Split command line */
    if (!dest || !(files = memchr(dest, '=', length)) )
        return (IPE_SYNTAX);

    *files++ = 0;

    if (strchr (dest, ':'))
        dest = strchr (dest, ':') +1;

#ifdef DEBUG_DRIVE
    log_debug("COPY: dest= '%s', orig= '%s'.", dest, files);
#endif

    if (vdrive_open(vdrive, dest, strlen(dest), 1))
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
        if (vdrive_open(vdrive, name, strlen(name), 0)) {
            vdrive_close(vdrive, 1);
            return (IPE_NOT_FOUND);
        }

        while (!vdrive_read(vdrive, (BYTE *)&c, 0)) {
            if (vdrive_write(vdrive, c, 1)) {
                vdrive_close(vdrive, 0); /* No space on disk.  */
                vdrive_close(vdrive, 1);
                return (IPE_DISK_FULL);
            }
        }

        vdrive_close(vdrive, 0);
        name = p; /* Next file.  */
    }
    vdrive_close(vdrive, 1);
    return(IPE_OK);
}


/*
 * Rename disk entry
 */

static int vdrive_command_rename(vdrive_t *vdrive, char *dest, int length)
{
    char *src;
    char dest_name[256], src_name[256];
    int dest_reallength, dest_readmode, dest_filetype, dest_rl;
    int	src_reallength, src_readmode, src_filetype, src_rl;
    BYTE *slot;

    if (!dest || !(src = memchr(dest, '=', length)) )
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

    if (vdrive->read_only)
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
    disk_image_write_sector(vdrive->image, vdrive->Dir_buffer,
                       vdrive->Curr_track, vdrive->Curr_sector);
    return(IPE_OK);
}

/* ------------------------------------------------------------------------- */

/*
 * Close all channels. This happens on 'I' -command and on command-
 * channel close.
 */

void vdrive_close_all_channels(vdrive_t *vdrive)
{
    int i;
    bufferinfo_t *p;

    for (i = 0; i <= 15; i++) {
        p = &(vdrive->buffers[i]);
        if (p->mode != BUFFER_NOT_IN_USE && p->mode != BUFFER_COMMAND_CHANNEL)
            vdrive_close(vdrive, i);
    }
}

static int vdrive_command_initialize(vdrive_t *vdrive)
{
    vdrive_close_all_channels(vdrive);

    /* Update BAM in memory.  */
    if (vdrive->image != NULL)
        vdrive_bam_read_bam(vdrive);
#if 0
    if (vdrive->ErrFlg)
        set_error_data(vdrive, 3); /* Clear or read error data.  */
#endif
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
    if (vdrive->read_only)
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
            disk_image_write_sector(vdrive->image, vdrive->Dir_buffer,
                                    vdrive->Curr_track, vdrive->Curr_sector);
        }
    }

    /* Write back BAM only if validate was successful.  */
    vdrive_bam_write_bam(vdrive);
    return status;
}

static int vdrive_command_format(vdrive_t *vdrive, const char *name, BYTE *id,
                                 BYTE *minus)
{
    BYTE tmp[256];
    int status;
    BYTE null = 0;

    if (!name)
        return IPE_SYNTAX;

    if (vdrive->read_only)
        return IPE_WRITE_PROTECT_ON;

    if (vdrive->image->fd == NULL)
        return IPE_NOT_READY;

    /*
     * If id, skip comma
     */

    if (id)
        *id++ = 0;
    else
        id = &null;

#if 0
    if (vdrive->ErrFlg)
        set_error_data(vdrive, 5);	/* clear and write error data */
#endif

    /*
     * Make the first dir-entry
     */
    memset(tmp, 0, 256);
    tmp[1] = 255;
    disk_image_write_sector(vdrive->image, tmp, vdrive->Dir_Track,
                            vdrive->Dir_Sector);
    vdrive_bam_create_empty_bam(vdrive, name, id);
    vdrive_bam_write_bam(vdrive);

    /* Validate is called to clear the BAM.  */
    status = vdrive_command_validate(vdrive);
    return status;
}

/* ------------------------------------------------------------------------- */

#if 0
/*
 * Sync (update) the Error Block image in memory
 */

static int set_error_data(vdrive_t *vdrive, int flags)
{
    int    size;
    off_t  offset;

    if (!(vdrive->ErrData)) {
	vdrive->ErrData = (char *)xmalloc((size_t)MAX_BLOCKS_ANY);
	memset(vdrive->ErrData, 0x01, MAX_BLOCKS_ANY);
    }
    else if (flags & 1)		/* clear error data */
	memset(vdrive->ErrData, 0x01, MAX_BLOCKS_ANY);


    size = vdrive_calc_num_blocks(vdrive->image_format, vdrive->num_tracks);
    offset = ((off_t)size << 8) + (off_t)(vdrive->D64_Header?0:HEADER_LENGTH);
    fseek(vdrive->image->fd, offset, SEEK_SET);

    if (flags & 2) {		/* read from file */
        if (fread((char *)vdrive->ErrData, size, 1, vdrive->image->fd) < 1) {
            log_error(vdrive_log, "Floppy image read error.");
            return(-2);
        }
    }

    if (flags & 4) {		/* write to file */
        if (fwrite((char *)vdrive->ErrData, size, 1, vdrive->image->fd) < 1) {
            log_error(vdrive_log, "Floppy image write error.");
            return(-2);
        }
    }

    return (0);
}
#endif

/* ------------------------------------------------------------------------- */

int vdrive_calculate_disk_half(int type)
{
    switch (type) {
      case VDRIVE_IMAGE_FORMAT_1541:
        return 17;
      case VDRIVE_IMAGE_FORMAT_1571:
        return 17;
      case VDRIVE_IMAGE_FORMAT_1581:
        return 40;
      case VDRIVE_IMAGE_FORMAT_8050:
      case VDRIVE_IMAGE_FORMAT_8250:
        return 39;
      default:
        log_error(vdrive_log,
                  "Unknown disk type %i.  Cannot calculate disk half.", type);
    }
    return -1;
}

int vdrive_get_max_sectors(int type, int track)
{
    switch (type) {
      case VDRIVE_IMAGE_FORMAT_1541:
        return disk_image_sector_per_track(DISK_IMAGE_TYPE_D64, track);
      case VDRIVE_IMAGE_FORMAT_1571:
        return disk_image_sector_per_track(DISK_IMAGE_TYPE_D71, track);
      case VDRIVE_IMAGE_FORMAT_8050:
        return disk_image_sector_per_track(DISK_IMAGE_TYPE_D80, track);
      case VDRIVE_IMAGE_FORMAT_1581:
        return 40;
      case VDRIVE_IMAGE_FORMAT_8250:
        if (track <= NUM_TRACKS_8250 / 2) {
            return disk_image_sector_per_track(DISK_IMAGE_TYPE_D80, track);
        } else {
            return disk_image_sector_per_track(DISK_IMAGE_TYPE_D80, track
                                               - (NUM_TRACKS_8250 / 2));
        }
      default:
        log_message(vdrive_log,
                    "Unknown disk type %i.  Cannot calculate max sectors",
                    type);

    }
    return -1;
}

/* ------------------------------------------------------------------------- */

/*
 * Functions to attach the disk image files.
 */

void vdrive_detach_image(disk_image_t *image, int unit, vdrive_t *vdrive)
{
    switch(image->type) {
      case DISK_IMAGE_TYPE_D64:
        log_message(vdrive_log, "Unit %d: D64 disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D71:
        log_message(vdrive_log, "Unit %d: D71 disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D81:
        log_message(vdrive_log, "Unit %d: D81 disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D80:
        log_message(vdrive_log, "Unit %d: D80 disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_D82:
        log_message(vdrive_log, "Unit %d: D82 disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_GCR:
        log_message(vdrive_log, "Unit %d: GCR disk image detached: %s.",
                    unit, image->name);
        break;
      case DISK_IMAGE_TYPE_X64:
        log_message(vdrive_log, "Unit %d: X64 disk image detached: %s.",
                    unit, image->name);
        break;
      default:
        return;
    }
    vdrive_close_all_channels(vdrive);
    vdrive->image = NULL;
}

int vdrive_attach_image(disk_image_t *image, int unit, vdrive_t *vdrive)
{
    /* Compatibily cruft (soon to be removed).  */
    vdrive->unit = unit;

    switch(image->type) {
      case DISK_IMAGE_TYPE_D64:
        log_message(vdrive_log, "Unit %d: D64 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_1541;
        vdrive->num_tracks  = image->tracks;
        break;
      case DISK_IMAGE_TYPE_D71:
        log_message(vdrive_log, "Unit %d: D71 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_1571;
        vdrive->num_tracks  = image->tracks;
        break;
      case DISK_IMAGE_TYPE_D81:
        log_message(vdrive_log, "Unit %d: D81 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_1581;
        vdrive->num_tracks  = image->tracks;
        break;
      case DISK_IMAGE_TYPE_D80:
        log_message(vdrive_log, "Unit %d: D80 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_8050;
        vdrive->num_tracks  = image->tracks;
        break;
      case DISK_IMAGE_TYPE_D82:
        log_message(vdrive_log, "Unit %d: D82 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_8250;
        vdrive->num_tracks = image->tracks;
        break;
      case DISK_IMAGE_TYPE_GCR:
        log_message(vdrive_log, "Unit %d: GCR disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_1541;
        vdrive->num_tracks = 35;
        break;
      case DISK_IMAGE_TYPE_X64:
        log_message(vdrive_log, "Unit %d: X64 disk image attached: %s.",
                    vdrive->unit, image->name);
        vdrive->image_format = VDRIVE_IMAGE_FORMAT_1541;
        vdrive->num_tracks = image->tracks;
        break;
      default:
        return -1;
    }

    /* Initialise format constants */
    vdrive_set_disk_geometry(vdrive, vdrive->image_format);

    vdrive->image = image;

    if (vdrive_bam_read_bam(vdrive)) {
        log_error(vdrive_log, "Cannot access BAM.");
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

/*
 * Calculate and return the total number of blocks available on a disk.
 */

int vdrive_calc_num_blocks(int format, int tracks)
{
    int blocks = -1;

    switch (format) {
      case VDRIVE_IMAGE_FORMAT_1541:
        if (tracks > MAX_TRACKS_1541)
            tracks = MAX_TRACKS_1541;
        blocks = NUM_BLOCKS_1541 + (tracks - 35) * 17;
        break;
      case VDRIVE_IMAGE_FORMAT_1571:
        if (tracks > MAX_TRACKS_1571)
            tracks = MAX_TRACKS_1571;
        blocks = NUM_BLOCKS_1571 + (tracks - 70) * 17;
        break;
      case VDRIVE_IMAGE_FORMAT_1581:
        blocks = NUM_BLOCKS_1581;
        break;
      case VDRIVE_IMAGE_FORMAT_8050:
        blocks = NUM_BLOCKS_8050;
        break;
      case VDRIVE_IMAGE_FORMAT_8250:
        blocks = NUM_BLOCKS_8250;
        break;
      default:
        log_error(vdrive_log,
                  "Unknown disk type %i.  Cannot calculate number of blocks.",
                  format);
    }
    return blocks;
}

/* ------------------------------------------------------------------------- */

/*
 * Initialise format constants
 */

static void vdrive_set_disk_geometry(vdrive_t *vdrive, int type)
{
    switch (type) {
      case VDRIVE_IMAGE_FORMAT_1541:
        vdrive->Bam_Track  = BAM_TRACK_1541;
        vdrive->Bam_Sector = BAM_SECTOR_1541;
        vdrive->bam_name   = BAM_NAME_1541;
        vdrive->bam_id     = BAM_ID_1541;
        vdrive->Dir_Track  = DIR_TRACK_1541;
        vdrive->Dir_Sector = DIR_SECTOR_1541;
        break;
      case VDRIVE_IMAGE_FORMAT_1571:
        vdrive->Bam_Track  = BAM_TRACK_1571;
        vdrive->Bam_Sector = BAM_SECTOR_1571;
        vdrive->bam_name   = BAM_NAME_1571;
        vdrive->bam_id     = BAM_ID_1571;
        vdrive->Dir_Track  = DIR_TRACK_1571;
        vdrive->Dir_Sector = DIR_SECTOR_1571;
        break;
      case VDRIVE_IMAGE_FORMAT_1581:
        vdrive->Bam_Track  = BAM_TRACK_1581;
        vdrive->Bam_Sector = BAM_SECTOR_1581;
        vdrive->bam_name   = BAM_NAME_1581;
        vdrive->bam_id     = BAM_ID_1581;
        vdrive->Dir_Track  = DIR_TRACK_1581;
        vdrive->Dir_Sector = DIR_SECTOR_1581;
        break;
      case VDRIVE_IMAGE_FORMAT_8050:
        vdrive->Bam_Track  = BAM_TRACK_8050;
        vdrive->Bam_Sector = BAM_SECTOR_8050;
        vdrive->bam_name   = BAM_NAME_8050;
        vdrive->bam_id     = BAM_ID_8050;
        vdrive->Dir_Track  = DIR_TRACK_8050;
        vdrive->Dir_Sector = DIR_SECTOR_8050;
        break;
      case VDRIVE_IMAGE_FORMAT_8250:
        vdrive->Bam_Track  = BAM_TRACK_8250;
        vdrive->Bam_Sector = BAM_SECTOR_8250;
        vdrive->bam_name   = BAM_NAME_8250;
        vdrive->bam_id     = BAM_ID_8250;
        vdrive->Dir_Track  = DIR_TRACK_8250;
        vdrive->Dir_Sector = DIR_SECTOR_8250;
        break;
      default:
        log_error(vdrive_log,
                  "Unknown disk type %i.  Cannot set disk geometry.", type);
    }
}

/* ------------------------------------------------------------------------- */

/* Wild-card match routine
 *
 * This routine takes two string parameters, the path to be macthed and
 * a pattern to match it with. There are two wild-cards - the ? and *.
 * A question mark will match exactly one character. An asterisk will
 * match any number (or zero) characters. Each '*' introduces one level
 * of recursion. A backslash forces the next character to be taken verbatim.
 * However, a null character cannot be protected.
 */

static int compare_filename (char *name, char *pattern)
{
    char *p, *q;
    int   literal = 0;


    p = pattern;
    q = name;

    while (*p && *q) {

	if (!literal) {
	    if (*p == '?') {
		++p;
		++q;
		continue;
	    }
	    if (*p == '*') {
		while (*++p == '*');
		if (!*p)
		    return (1);		/* End of pattern -> matches */

		while (!compare_filename (q, p) && *++q);
		return (*q);		/* if *q > 0 it must have matched */
	    }
	    if (*p == '\\') {
		++p;
		++literal;
		continue;
	    }
	}  /* literal */
	else {
	    literal = 0;
	}

	if (*p == *q) {
	    ++p;
	    ++q;
	}
	else
	    return (0);			/* No match */

    }  /* while */

    return (!*p && !*q);		/* Match */
}

