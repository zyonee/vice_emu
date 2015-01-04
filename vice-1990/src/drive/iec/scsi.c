/*
 * scsi.c - SCSI device emulation
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

#define SCSI_BUS_FREE 0
#define SCSI_DATA_OUT (SCSI_BSY | SCSI_REQ)
#define SCSI_DATA_IN (SCSI_BSY | SCSI_REQ | SCSI_I_O)
#define SCSI_COMMAND (SCSI_BSY | SCSI_REQ | SCSI_C_D)
#define SCSI_STATUS (SCSI_BSY | SCSI_REQ | SCSI_C_D | SCSI_I_O)
#define SCSI_MESSAGE_OUT (SCSI_BSY | SCSI_REQ | SCSI_MSG | SCSI_C_D)
#define SCSI_MESSAGE_IN (SCSI_BSY | SCSI_REQ | SCSI_MSG | SCSI_C_D | SCSI_I_O)
#define SCSI_MESSAGE_COMMAND_COMPLETE (SCSI_BSY | SCSI_REQ | SCSI_MSG | SCSI_C_D | SCSI_I_O | 0x00)
#define SCSI_MESSAGE_COMMAND_ABORT (SCSI_BSY | SCSI_REQ | SCSI_MSG | SCSI_C_D | SCSI_I_O | 0x06)
#define SCSI_STATUS_GOOD (SCSI_BSY | SCSI_REQ | SCSI_C_D | SCSI_I_O | 0x00)
#define SCSI_STATUS_CHECK_CONDITION (SCSI_BSY | SCSI_REQ | SCSI_C_D | SCSI_I_O | 0x02)
#define SCSI_COPYRIGHT "KAJTAR ZSOLT"
#define SCSI_VENDOR "VICE-EMU"
#define SCSI_REVISION &"$Revision::          $"[12]

/* required for off_t on some platforms */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

/* VAC++ has off_t in sys/stat.h */
#ifdef __IBMC__
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

#include "archdep.h"
#include "log.h"
#include "scsi.h"
#include "snapshot.h"
#include "types.h"
#include "util.h"
#include "lib.h"
#include "maincpu.h"
#include "monitor.h"

#ifdef SCSI_DEBUG
#define debug(_x_) log_message _x_
#else
#define debug(_x_)
#endif

struct scsi_drive_s {
    WORD bus;
    BYTE id;        /* own scsi ID */
    int sector_count;
    BYTE command[12];
    BYTE sense[18];
    int bufp, buflen;
    BYTE *buffer;
    FILE *file;
    char *filename;
    char *myname;
    int size;
    int readonly;
    int attention;
    int locked;
    int wcache;
    scsi_drive_type_t type;
    int pos;
    log_t log;
    int sector_size;
};

#define SCSI_NO_ADDITIONAL_SENSE_INFORMATION 0x00000
#define SCSI_LOGICAL_UNIT_NOT_SUPPORTED 0x52500
#define SCSI_UNRECOVERED_READ_ERROR 0x31100
#define SCSI_WRITE_ERROR 0x30c00
#define SCSI_LOGICAL_BLOCK_OUT_OF_RANGE 0xd2100
#define SCSI_MEDIUM_NOT_PRESENT 0x23a00
#define SCSI_NO_SEEK_COMPLETE 0x40200
#define SCSI_WRITE_PROTECTED 0x72700
#define SCSI_ILLEGAL_REQUEST 0x50500
#define SCSI_MEDIA_LOAD_OR_EJECT_FAILED 0x40500
#define SCSI_MEDIUM_REMOVAL_PREVENTED 0x55302

static WORD scsi_update_sense(scsi_drive_t *drv, DWORD asc) {
    memset(drv->sense, 0, sizeof(drv->sense));
    drv->sense[0] = 0x80 | 0x70;
    drv->sense[2] = asc >> 16;
    drv->sense[7] = sizeof(drv->sense) - 7;
    drv->sense[12] = asc >> 8;
    drv->sense[13] = asc;
    return drv->sense[2] ? SCSI_STATUS_CHECK_CONDITION : SCSI_STATUS_GOOD;
}

static WORD seek_sector(scsi_drive_t *drv, int lba)
{
    if (!drv->file) {
        return scsi_update_sense(drv, SCSI_MEDIUM_NOT_PRESENT);
    }
    if (lba >= drv->size || lba < 0) {
        return scsi_update_sense(drv, SCSI_LOGICAL_BLOCK_OUT_OF_RANGE);
    }
    drv->pos = lba;
    if (fseek(drv->file, (off_t)lba * drv->sector_size, SEEK_SET)) {
        return scsi_update_sense(drv, SCSI_NO_SEEK_COMPLETE);
    }
    return 0;
}

static int read_sector(scsi_drive_t *drv)
{
    drv->buflen = drv->sector_size;

    if (!drv->file) {
        return scsi_update_sense(drv, SCSI_MEDIUM_NOT_PRESENT);
    }

    clearerr(drv->file);
    if (fread(drv->buffer, drv->sector_size, 1, drv->file) != 1) {
        memset(drv->buffer, 0, drv->sector_size);
    }

    if (ferror(drv->file)) {
        return scsi_update_sense(drv, SCSI_UNRECOVERED_READ_ERROR);
    }
    drv->pos++;
    drv->bufp = 0;
    return 0;
}

static int write_sector(scsi_drive_t *drv)
{
    drv->buflen = drv->sector_size;

    if (!drv->file) {
        return scsi_update_sense(drv, SCSI_MEDIUM_NOT_PRESENT);
    }

    if (drv->readonly) {
        return scsi_update_sense(drv, SCSI_WRITE_PROTECTED);
    }

    if (fwrite(drv->buffer, 1, drv->sector_size, drv->file) != (size_t)drv->sector_size) {
        return scsi_update_sense(drv, SCSI_WRITE_ERROR);
    }
    drv->pos++;

    if (!drv->wcache) {
        if (fflush(drv->file)) {
            return scsi_update_sense(drv, SCSI_WRITE_ERROR);
        }
    }
    return 0;
}

void scsi_reset(scsi_drive_t *drv)
{
    memset(drv->sense, 0, sizeof(drv->sense));
    drv->bus = 0;
}

static void scsi_poweron(scsi_drive_t *drv, scsi_drive_type_t type)
{
    drv->wcache = 0;
    drv->attention = 1;
    drv->pos = 0;

    drv->type = type;
    switch (drv->type) {
        case SCSI_DRIVE_FDD:
            drv->locked = 0;
            drv->sector_size = 512;
            drv->readonly = 0;
            break;
        case SCSI_DRIVE_CD:
            drv->locked = 0;
            drv->sector_size = 2048;
            drv->readonly = 1;
            break;
        case SCSI_DRIVE_HDD:
            drv->locked = 1;
            drv->sector_size = 512;
            drv->readonly = 0;
            break;
        default:
            drv->locked = 0;
            drv->sector_size = 512;
            drv->readonly = 1;
            drv->type = SCSI_DRIVE_NONE;
            return;
    }

    scsi_reset(drv);
}

scsi_drive_t *scsi_init(int drive)
{
    scsi_drive_t *drv = lib_malloc(sizeof(scsi_drive_t));

    drv->myname = lib_msprintf("SCSI%d", drive);
    drv->log = log_open(drv->myname);
    drv->file = NULL;
    drv->filename = NULL;
    drv->buffer = lib_malloc(2048);
    drv->id = 1 << (drive & 7);
    scsi_poweron(drv, SCSI_DRIVE_NONE);
    return drv;
}

void scsi_shutdown(scsi_drive_t *drv)
{
    if (drv->filename) {
        lib_free(drv->filename);
        drv->filename = NULL;
    }
    log_close(drv->log);
    lib_free(drv->myname);
    lib_free(drv->buffer);
    lib_free(drv);
}

static WORD scsi_execute_command(scsi_drive_t *drv)
{
    int lba;
    BYTE *command = drv->command;
    WORD err;

    switch (command[0]) {
    case 0x00:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "TEST UNIT READY"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if (!drv->file) {
            return scsi_update_sense(drv, SCSI_MEDIUM_NOT_PRESENT);
        }
        return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
    case 0x03:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "REQUEST SENSE"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        memcpy(drv->buffer, drv->sense, sizeof(drv->sense));
        memset(drv->sense, 0, sizeof(drv->sense)); 
        drv->buflen = command[4] < sizeof(drv->sense) ? command[4] : sizeof(drv->sense);
        if (!drv->buflen) {
            return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
        }
        drv->bufp = 0;
        return SCSI_DATA_IN | drv->buffer[drv->bufp++];
    case 0x04:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "FORMAT UNIT"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if (!drv->file) {
            return scsi_update_sense(drv, SCSI_MEDIUM_NOT_PRESENT);
        }
        if (drv->readonly) {
            return scsi_update_sense(drv, SCSI_WRITE_PROTECTED);
        }
        break;
    case 0x08:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        lba = ((command[1] & 0x1f) << 16) | (command[2] << 8) | command[3];
        drv->sector_count = command[4];
        if (!drv->sector_count) {
            drv->sector_count = 256;
        }
        debug((drv->log, "READ 6 (%d)*%d", lba, drv->sector_count));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if ((err = seek_sector(drv, lba))) {
            return err;
        }
        if ((err = read_sector(drv))) {
            return err;
        }
        return SCSI_DATA_IN | drv->buffer[drv->bufp++];
    case 0x0a:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        lba = ((command[1] & 0x1f) << 16) | (command[2] << 8) | command[3];
        drv->sector_count = command[8];
        if (!drv->sector_count) {
            drv->sector_count = 256;
        }
        debug((drv->log, "WRITE 6 (%d)*%d", lba, drv->sector_count));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if ((err = seek_sector(drv, lba))) {
            return err;
        }
        if (drv->readonly) {
            return scsi_update_sense(drv, SCSI_WRITE_PROTECTED);
        }
        drv->bufp = 0;
        return SCSI_DATA_OUT;
    case 0x12:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "INQUIRY"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        memset(drv->buffer, 0, 36);
        drv->buffer[0] = (drv->type == SCSI_DRIVE_CD) ? 0x05 : 0x00; /* device type */
        drv->buffer[1] = (drv->type == SCSI_DRIVE_CD || drv->type == SCSI_DRIVE_FDD) ? 0x80 : 0x00; /* removable */
        drv->buffer[4] = 36 - 4;
        memcpy(drv->buffer + 8, SCSI_VENDOR, 8);
        switch (drv->type) {
        case SCSI_DRIVE_HDD:
            memcpy(drv->buffer + 16, "HDD " SCSI_COPYRIGHT, 16);
            break;
        case SCSI_DRIVE_FDD:
            memcpy(drv->buffer + 16, "FDD " SCSI_COPYRIGHT, 16);
            break;
        case SCSI_DRIVE_CD:
            memcpy(drv->buffer + 16, "DVD " SCSI_COPYRIGHT, 16);
            break;
        case SCSI_DRIVE_NONE:
            memset(drv->buffer + 16, ' ', 16);
            break;
        }
        memcpy(drv->buffer + 32, SCSI_REVISION, 4);
        drv->buflen = command[4] < 36 ? command[4] : 36;
        if (!drv->buflen) {
            return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
        }
        drv->bufp = 0;
        return SCSI_DATA_IN | drv->buffer[drv->bufp++];
    case 0x15:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "MODE SELECT 6"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        break;
    case 0x1a:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "MODE SENSE 6"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        break;
    case 0x1b:
        if (drv->bufp < 6) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "START/STOP UNIT"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        switch (command[4]) {
        case 0: break;
        case 1: break;
        case 2:
            if (drv->file) {
                if (drv->locked) {
                    return scsi_update_sense(drv, SCSI_MEDIUM_REMOVAL_PREVENTED);
                } else {
                    scsi_image_detach(drv);
                }
            }
            break;
        case 3:
            if (!drv->file) {
                scsi_image_attach(drv, drv->filename, drv->type);
                if (!drv->file) {
                    return scsi_update_sense(drv, SCSI_MEDIA_LOAD_OR_EJECT_FAILED);
                }
            }
            break;
        default:
            break;
        }
        return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
    case 0x25:
        if (drv->bufp < 10) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "READ CAPACITY"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        drv->buffer[0] = drv->size >> 24;
        drv->buffer[1] = drv->size >> 16;
        drv->buffer[2] = drv->size >> 8;
        drv->buffer[3] = drv->size;
        drv->buffer[4] = drv->sector_size >> 24;
        drv->buffer[5] = drv->sector_size >> 16;
        drv->buffer[6] = drv->sector_size >> 8;
        drv->buffer[7] = drv->sector_size;
        drv->buflen = 8;
        drv->bufp = 0;
        return SCSI_DATA_IN | drv->buffer[drv->bufp++];
    case 0x28:
        if (drv->bufp < 10) {
            return SCSI_COMMAND;
        }
        lba = (command[2] << 24) | (command[3] << 16) | (command[4] << 8) | command[5];
        drv->sector_count = (command[7] << 8) | command[8];
        debug((drv->log, "READ 10 (%d)*%d", lba, drv->sector_count));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if ((err = seek_sector(drv, lba))) {
            return err;
        }
        if ((err = read_sector(drv))) {
            return err;
        }
        if (!drv->sector_count) {
            return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
        }
        return SCSI_DATA_IN | drv->buffer[drv->bufp++];
    case 0x2a:
        if (drv->bufp < 10) {
            return SCSI_COMMAND;
        }
        lba = (command[2] << 24) | (command[3] << 16) | (command[4] << 8) | command[5];
        drv->sector_count = (command[7] << 8) | command[8];
        debug((drv->log, "WRITE 10 (%d)*%d", lba, drv->sector_count));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        if ((err = seek_sector(drv, lba))) {
            return err;
        }
        if (drv->readonly) {
            return scsi_update_sense(drv, SCSI_WRITE_PROTECTED);
        }
        if (!drv->sector_count) {
            return scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
        }
        drv->bufp = 0;
        return SCSI_DATA_OUT;
    case 0x55:
        if (drv->bufp < 10) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "MODE SELECT 10"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        break;
    case 0x5a:
        if (drv->bufp < 10) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "MODE SENSE 10"));
        if (command[1] & 0xe0) {
            return scsi_update_sense(drv, SCSI_LOGICAL_UNIT_NOT_SUPPORTED);
        }
        break;
    default:
        if (drv->bufp < 6 && command[0] < 0x20) {
            return SCSI_COMMAND;
        }
        if (drv->bufp < 10 && command[0] < 0x60 && command[0] >= 0x20) {
            return SCSI_COMMAND;
        }
        if (drv->bufp < 12 && command[0] < 0xc0 && command[0] >= 0xa0) {
            return SCSI_COMMAND;
        }
        debug((drv->log, "SCSI COMMAND %02x", command[0]));
    }
    return scsi_update_sense(drv, SCSI_ILLEGAL_REQUEST);
}

WORD scsi_read(scsi_drive_t *drv, WORD bus)
{
    if (drv->type == SCSI_DRIVE_NONE) {
        return bus;
    }
    bus |= drv->bus;
    if (bus & SCSI_ACK) {
        switch (drv->bus & ~0xff) {
        case SCSI_DATA_IN:
            switch (drv->command[0]) {
            case 0x03:
            case 0x12:
            case 0x25:
                if (drv->bufp >= drv->buflen) {
                    drv->bus = scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
                    break;
                }
                drv->bus = SCSI_DATA_IN | drv->buffer[drv->bufp++];
                break;
            case 0x08:
            case 0x28:
                if (drv->bufp >= drv->buflen) {
                    WORD err;
                    drv->sector_count--;
                    if (!drv->sector_count) {
                        drv->bus = scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
                        break;
                    }
                    if ((err = read_sector(drv))) {
                        drv->bus = err;
                        break;
                    }
                }
                drv->bus = SCSI_DATA_IN | drv->buffer[drv->bufp++];
                break;
            default:
                drv->bus = scsi_update_sense(drv, SCSI_ILLEGAL_REQUEST);
            }
            break;
        case SCSI_STATUS:
            if (drv->bus & 0xff) {
                drv->bus = SCSI_MESSAGE_COMMAND_COMPLETE;
            } else {
                drv->bus = SCSI_MESSAGE_COMMAND_ABORT;
            }
            break;
        case SCSI_MESSAGE_IN:
            drv->bus = SCSI_BUS_FREE;
            break;
        default: 
            break;
        }
    }
    return bus;
}

void scsi_store(scsi_drive_t *drv, WORD value)
{
    if (drv->type == SCSI_DRIVE_NONE) {
        return;
    }
    value |= drv->bus;

    switch (value & (SCSI_BSY | SCSI_SEL)) {
    case 0:
        return; /* bus free */
    case SCSI_SEL:
        if (value & SCSI_I_O) {
            return; /* reselection */
        }
        if (value & drv->id) {
            drv->bus = SCSI_MESSAGE_OUT;
        }
        return; /* selection */
    case SCSI_BSY:
        switch (drv->bus & ~0xff) {
        case SCSI_MESSAGE_OUT:
            if (value & SCSI_ATN) {
                return;
            }
            drv->bufp = 0;
            drv->bus = SCSI_COMMAND;
            /* fall through */
        case SCSI_COMMAND:
            if (value & SCSI_ACK) {
                drv->command[drv->bufp++] = value;
                drv->bus = scsi_execute_command(drv);
            }
            return;
        case SCSI_DATA_OUT:
            if (value & SCSI_ACK) {
                drv->buffer[drv->bufp++] = value & 0xff;
                if (drv->bufp >= drv->buflen) {
                    WORD err;
                    if ((err = write_sector(drv))) {
                        drv->bus = err;
                        return;
                    }
                    if (--drv->sector_count) {
                        drv->bufp = 0;
                        return;
                    }
                    if (!drv->file || fflush(drv->file)) {
                        drv->bus = scsi_update_sense(drv, SCSI_WRITE_ERROR);
                        return;
                    }
                    drv->bus = scsi_update_sense(drv, SCSI_NO_ADDITIONAL_SENSE_INFORMATION);
                }
            }
            return;
        default: 
            return;
        }
    default:
        return; /* arbitration */
    }
    return;
}

void scsi_image_attach(scsi_drive_t *drv, char *filename, scsi_drive_type_t type)
{
    if (drv->file != NULL) {
        fclose(drv->file);
        drv->file = NULL;
    }

    if (drv->filename != filename) {
        util_string_set(&drv->filename, filename);
    }
    drv->size = 0;

    if (type != SCSI_DRIVE_NONE) {
        off_t size = 0;
        if (drv->filename && drv->filename[0]) {
            if (type != SCSI_DRIVE_CD) {
                drv->file = fopen(drv->filename, MODE_READ_WRITE);
            }
            if (!drv->file) {
                drv->file = fopen(drv->filename, MODE_READ);
            }
        }
        if (drv->file && fseek(drv->file, 0, SEEK_END) == 0) {
            size = ftell(drv->file);
            if (size < 0) {
                size = 0;
            }
            size /= drv->sector_size;
        }

        drv->size = size;
        if (drv->size < 1) {
            drv->size = 16384;
            log_warning(drv->log, "Image size invalid, using default %d MiB.", drv->size / (1048576 / drv->sector_size));
        }
    }

    if (drv->type != type) {
        scsi_poweron(drv, type); /* update actual geometry */
    } else {
        drv->attention = 1; /* disk change only */
    }

    if (drv->file) {
        log_message(drv->log, "Attached `%s' %u sectors total.", drv->filename, drv->size);
    } else {
        if (drv->filename && drv->filename[0] && drv->type != SCSI_DRIVE_NONE) {
            log_warning(drv->log, "Cannot use image file `%s', drive disabled.", drv->filename);
        }
    }
    return;
}

void scsi_image_detach(scsi_drive_t *drv)
{
    if (drv->file != NULL) {
        fclose(drv->file);
        drv->file = NULL;
        log_message(drv->log, "Detached.");
    }
    return;
}

int scsi_image_change(scsi_drive_t *drv, char *filename, scsi_drive_type_t type)
{
    if (drv->type != type) {
        return 1;
    }
    scsi_image_attach(drv, filename, type);
    return 0;
}

/* ---------------------------------------------------------------------*/
/*    snapshot support functions                                             */

/* Please note that after loading a snapshot the the image is in readonly
 * mode to prevent any filesystem corruption. This could be solved later by
 * checksumming the image (this might be slow!) and comparing it to the stored
 * checksum to check if there was any modification meanwhile.
 */

#define SCSI_DUMP_VER_MAJOR   0
#define SCSI_DUMP_VER_MINOR   1

int scsi_snapshot_write_module(scsi_drive_t *drv, snapshot_t *s)
{
    snapshot_module_t *m;
    off_t pos = 0;

    m = snapshot_module_create(s, drv->myname,
                               SCSI_DUMP_VER_MAJOR, SCSI_DUMP_VER_MINOR);
    if (m == NULL) {
        return -1;
    }

    if (drv->file) {
        pos = ftell(drv->file);
        if (pos < 0) {
            pos = 0;
        }
    }

    SMW_STR(m, drv->filename);
    SMW_DW(m, drv->type);
    SMW_DW(m, drv->size);
    SMW_DW(m, drv->sector_count);
    SMW_BA(m, drv->command, sizeof(drv->command));
    SMW_W(m, drv->bus);
    SMW_W(m, (WORD)drv->bufp);
    SMW_BA(m, drv->buffer, drv->sector_size);
    SMW_DW(m, drv->pos);
    SMW_DW(m, pos / drv->sector_size);
    SMW_B(m, (BYTE)drv->wcache);

    return snapshot_module_close(m);
}

int scsi_snapshot_read_module(scsi_drive_t *drv, snapshot_t *s)
{
    BYTE vmajor, vminor;
    snapshot_module_t *m;
    char *filename = NULL;
    int pos, type;

    m = snapshot_module_open(s, drv->myname, &vmajor, &vminor);
    if (m == NULL) {
        return -1;
    }

    if ((vmajor != SCSI_DUMP_VER_MAJOR) || (vminor != SCSI_DUMP_VER_MINOR)) {
        snapshot_module_close(m);
        return -1;
    }

    SMR_STR(m, &filename);
    if (!drv->filename || strcmp(filename, drv->filename)) {
        lib_free(filename);
        snapshot_module_close(m);
        return -1;
    }
    lib_free(filename);
    SMR_DW_INT(m, &type);
    drv->type = type;
    if (drv->type != SCSI_DRIVE_HDD && drv->type != SCSI_DRIVE_FDD && drv->type != SCSI_DRIVE_CD) {
        drv->type = SCSI_DRIVE_NONE;
    }
    SMR_DW_INT(m, &drv->size);
    scsi_image_attach(drv, drv->filename, drv->type);
    SMR_DW_INT(m, &drv->sector_count);
    SMR_BA(m, drv->command, sizeof(drv->command));
    SMR_W(m, &drv->bus);
    SMR_W_INT(m, &drv->bufp);
    if (drv->bufp < 0 || drv->bufp > drv->sector_size) {
        drv->bufp = drv->sector_size;
    }
    SMR_BA(m, drv->buffer, drv->sector_size);
    SMR_DW_INT(m, &drv->pos);
    if (drv->pos < 0 || drv->pos > 268435455) {
        drv->pos = 0;
    }
    SMR_DW_INT(m, &pos);
    SMR_B_INT(m, &drv->wcache);
    if (drv->wcache) {
        drv->wcache = 1;
    }

    if (drv->file) {
        fseek(drv->file, (off_t)pos * drv->sector_size, SEEK_SET);
    }
    drv->readonly = 1; /* make sure that there's no filesystem corruption */

    return snapshot_module_close(m);
}
