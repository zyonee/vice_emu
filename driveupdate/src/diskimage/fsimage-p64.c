/*
 * fsimage-gcr.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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
#include <string.h>

#include "diskconstants.h"
#include "diskimage.h"
#include "fsimage-p64.h"
#include "fsimage.h"
#include "gcr.h"
#include "log.h"
#include "lib.h"
#include "types.h"
#include "util.h"
#include "p64.h"

static log_t fsimage_p64_log = LOG_ERR;

/*-----------------------------------------------------------------------*/
/* Intial P64 buffer setup.  */

int fsimage_read_p64_image(disk_image_t *image)
{
    TP64MemoryStream P64MemoryStreamInstance;
    TP64Image *P64Image = image->p64;
    int lSize, rc;
    void *buffer;

    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    fseek(fsimage->fd, 0, SEEK_END);
    lSize = ftell(fsimage->fd);
    fseek(fsimage->fd, 0, SEEK_SET);
    buffer = (char*)malloc(sizeof(char) * lSize);
    if (fread(buffer, 1, lSize, fsimage->fd) < 1)  {
        log_error(fsimage_p64_log, "Could not read P64 disk image.");
        return -1;
    }
    
    /*num_tracks = image->tracks;*/
    
    P64MemoryStreamCreate(&P64MemoryStreamInstance);
    P64MemoryStreamWrite(&P64MemoryStreamInstance,buffer,lSize);
    P64MemoryStreamSeek(&P64MemoryStreamInstance,0);
    if (P64ImageReadFromStream(P64Image,&P64MemoryStreamInstance)) {
	rc = 0;
    } else {
	rc = -1;
        log_error(fsimage_p64_log, "Could not read P64 disk image.");
    }
    P64MemoryStreamDestroy(&P64MemoryStreamInstance);
     
    return rc;
}

int fsimage_write_p64_image(disk_image_t *image)
{
    TP64MemoryStream P64MemoryStreamInstance;
    TP64Image *P64Image = image->p64;
    int rc;

    fsimage_t *fsimage;

    fsimage = image->media.fsimage;

    P64MemoryStreamCreate(&P64MemoryStreamInstance);
    P64MemoryStreamClear(&P64MemoryStreamInstance);
    if (P64ImageWriteToStream(P64Image,&P64MemoryStreamInstance)) {
	fseek(fsimage->fd, 0, SEEK_SET);
	if (fwrite(P64MemoryStreamInstance.Data, 1, P64MemoryStreamInstance.Size, fsimage->fd) < 1){
		rc = 0;
	} else {
		rc = -1;
	        log_error(fsimage_p64_log, "Could not write P64 disk image.");
	}
    } else {
	rc = -1;
        log_error(fsimage_p64_log, "Could not write P64 disk image.");
    }
    P64MemoryStreamDestroy(&P64MemoryStreamInstance);
     
    return rc;
}

/*-----------------------------------------------------------------------*/
/* Read an entire P64 track from the disk image.  */

int fsimage_p64_read_track(disk_image_t *image, unsigned int track,
                           BYTE *gcr_data, int *gcr_track_size)
{
    return -1;
}

/*-----------------------------------------------------------------------*/
/* Write an entire P64 track to the disk image.  */

int fsimage_p64_write_track(disk_image_t *image, unsigned int track,
                            int gcr_track_size, BYTE *gcr_speed_zone,
                            BYTE *gcr_track_start_ptr)
{
    return -1;
}

/*-----------------------------------------------------------------------*/
/* Read a sector from the P64 disk image.  */

int fsimage_p64_read_sector(disk_image_t *image, BYTE *buf,
                               unsigned int track, unsigned int sector)
{
    return -1;
}


/*-----------------------------------------------------------------------*/
/* Write a sector to the P64 disk image.  */

int fsimage_p64_write_sector(disk_image_t *image, BYTE *buf,
                                unsigned int track, unsigned int sector)
{
    return -1;
}

/*-----------------------------------------------------------------------*/

void fsimage_p64_init(void)
{
    fsimage_p64_log = log_open("Filesystem Image P64");
}

