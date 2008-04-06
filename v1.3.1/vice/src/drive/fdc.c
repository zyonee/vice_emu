
/*
 * fdc.c - 1001/8x50 FDC emulation
 *
 * Written by
 *  Andre' Fachat <fachat@physik.tu-chemnitz.de>
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

#include <string.h>

#include "alarm.h"
#include "diskimage.h"
#include "drive.h"
#include "drivecpu.h"
#include "drivetypes.h"
#include "fdc.h"
#include "log.h"
#include "types.h"
#include "vdrive-bam.h"
#include "vdrive.h"

#undef FDC_DEBUG

#define	DOS_IS_80(type)  (type == DRIVE_TYPE_8050  	\
	    || type == DRIVE_TYPE_8250			\
	    || type == DRIVE_TYPE_1001)
#define	DOS_IS_40(type)  (type == DRIVE_TYPE_4040)
#define	DOS_IS_30(type)  (type == DRIVE_TYPE_3040)
#define	DOS_IS_20(type)  (type == DRIVE_TYPE_2040)


static void clk_overflow_callback(int fnum, CLOCK sub, void *data);
static int int_fdc(int fnum, long offset);

static void clk_overflow_callback0(CLOCK sub, void *data)
{
    clk_overflow_callback(0, sub, data);
}

static void clk_overflow_callback1(CLOCK sub, void *data)
{
    clk_overflow_callback(1, sub, data);
}

static int int_fdc0(CLOCK offset)
{
    return int_fdc(0, offset);
}

static int int_fdc1(CLOCK offset)
{
    return int_fdc(1, offset);
}

/************************************************************************/

#define	NUM_FDC	2

static log_t fdc_log = LOG_ERR;

typedef struct fdc_t {
    int	fdc_state;
    alarm_t	fdc_alarm;
    CLOCK	alarm_clk;
    BYTE 	*buffer;
    BYTE 	*iprom;
    int		drive_type;
    int		last_track;
    int		last_sector;
    int         wps_change;	/* if not zero, toggle wps and decrement */
    disk_image_t *image;
    disk_image_t *realimage;
} fdc_t;

static fdc_t fdc[NUM_FDC];

void fdc_reset(int fnum, int drive_type)
{
#ifdef FDC_DEBUG
    log_message(fdc_log, "fdc_reset: drive %d type=%d\n",fnum, drive_type);
#endif


    /* detach disk images */
    if (fdc[fnum].image) {
        fdc[fnum].wps_change = 0;
	fdc_detach_image(fdc[fnum].image, fnum + 8);
    }
    if (fnum == 0
	&& DRIVE_IS_DUAL(fdc[fnum].drive_type)
	&& fdc[1].image) {
        fdc[1].wps_change = 0;
	fdc_detach_image(fdc[1].image, fnum + 8);
    }

    if (DRIVE_IS_OLDTYPE(drive_type)) {
	fdc[fnum].drive_type = drive_type;
	fdc[fnum].fdc_state = FDC_RESET0;
	alarm_set(&fdc[fnum].fdc_alarm, drive_clk[fnum] + 20);
    } else {
	fdc[fnum].drive_type = DRIVE_TYPE_NONE;
	alarm_unset(&fdc[fnum].fdc_alarm);
	fdc[fnum].fdc_state = FDC_UNUSED;
    }

    /* re-attach disk images */
    if (fdc[fnum].realimage) {
	fdc_attach_image(fdc[fnum].realimage, fnum + 8);
    }
    if (fnum == 0
	&& DRIVE_IS_DUAL(drive_type)
	&& fdc[1].realimage) {
	fdc_attach_image(fdc[1].realimage, fnum + 8);
    }
}

static BYTE fdc_do_job(int fnum, int buf,
				int drv, BYTE job, BYTE *header)
{
#ifdef FDC_DEBUG
static BYTE fdc_do_job_(int fnum, int buf,
				int drv, BYTE job, BYTE *header);
    BYTE retval = fdc_do_job_(fnum, buf, drv, job, header);
    log_message(fdc_log, "  fdc_do_job (%02x) -> %02x\n",
	job, retval);
    return retval;
}
static BYTE fdc_do_job_(int fnum, int buf,
				int drv, BYTE job, BYTE *header)
{
#endif
    int rc, dnr;
    int i;
    unsigned int track, sector;
    BYTE *base;
    BYTE sector_data[256];
    BYTE disk_id[2];

    track = header[2];
    sector = header[3];

    if (DRIVE_IS_DUAL(fdc[fnum].drive_type)) {
	/* dual disk drive */
        dnr = drv;
    } else {
	/* single disk drive */
        if (drv) {
	    return FDC_ERR_SYNC;
	}
        dnr = fnum;
    }

    rc = 0;
    base = &(fdc[fnum].buffer[(buf + 1) << 8]);

#ifdef FDC_DEBUG
    log_message(fdc_log, "do job %02x, buffer %d ($%04x): d%d t%d s%d, "
		"image=%p, type=%04d",
		job, buf, (buf + 1) << 8, dnr, track, sector,
		fdc[dnr].image,
		fdc[dnr].image ? fdc[dnr].image->type : 0);
#endif

    if (fdc[dnr].image == NULL) {
#ifdef FDC_DEBUG
	log_message(fdc_log, "dnr=%d, image=NULL -> no disk!", dnr);
#endif
	return FDC_ERR_SYNC;
    }

    if(DOS_IS_80(fdc[fnum].drive_type)
		&& (fdc[dnr].image->type != DISK_IMAGE_TYPE_D80)
		&& (fdc[dnr].image->type != DISK_IMAGE_TYPE_D82)
	) {
#ifdef FDC_DEBUG
	log_message(fdc_log, "dos(%d) %d, disk image(%d) %d -> no disk!", 
		fnum, fdc[fnum].drive_type, dnr, fdc[dnr].image->type);
#endif
	return FDC_ERR_SYNC;
    }
    if((!DOS_IS_80(fdc[fnum].drive_type))
		&& (fdc[dnr].image->type != DISK_IMAGE_TYPE_D64)
		&& (fdc[dnr].image->type != DISK_IMAGE_TYPE_G64)
		&& (fdc[dnr].image->type != DISK_IMAGE_TYPE_X64)
	) {
#ifdef FDC_DEBUG
	log_message(fdc_log, "dos(%d) %d, disk image(%d) %d -> no disk!", 
		fnum, fdc[fnum].drive_type, dnr, fdc[dnr].image->type);
#endif
	return FDC_ERR_SYNC;
    }

    vdrive_bam_get_disk_id(dnr + 8, disk_id);

    switch (job) {
    case 0x80:		/* read */
	if (header[0] != disk_id[0] || header[1] != disk_id[1]) {
	    rc = FDC_ERR_ID;
	    break;
	}
	rc = disk_image_read_sector(fdc[dnr].image, sector_data, track, sector);
        if (rc < 0) {
            log_error(drive[dnr].log,
                  "Cannot read T:%d S:%d from disk image.",
                  track, sector);
            rc = FDC_ERR_DRIVE;
        } else {
	    memcpy(base, sector_data, 256);
	    rc = FDC_ERR_OK;
	}
	break;
    case 0x90:		/* write */
	if (header[0] != disk_id[0] || header[1] != disk_id[1]) {
	    rc = FDC_ERR_ID;
	    break;
	}
	if (fdc[dnr].image->read_only) {
	    rc = FDC_ERR_WPROT;
	    break;
	}
	memcpy(sector_data, base, 256);
        rc = disk_image_write_sector(fdc[dnr].image, sector_data, track,
                                     sector);
        if (rc < 0) {
            log_error(drive[dnr].log,
                  "Could not update T:%d S:%d on disk image.",
                  track, sector);
            rc = FDC_ERR_DRIVE;
        } else {
            rc = FDC_ERR_OK;
        }
        break;
    case 0xA0:		/* verify */
	if (header[0] != disk_id[0] || header[1] != disk_id[1]) {
	    rc = FDC_ERR_ID;
	    break;
	}
        rc = disk_image_read_sector(fdc[dnr].image, sector_data, track,
                                    sector);
        if (rc < 0) {
            log_error(drive[dnr].log,
                  "Cannot read T:%d S:%d from disk image.",
                  track, sector);
            rc = FDC_ERR_DRIVE;
        } else {
            rc = FDC_ERR_OK;
            for (i = 0; i < 256; i++) {
                if (fnum) {
                    if (sector_data[i] != base[i]) {
			rc = FDC_ERR_VERIFY;
		    }
                } else {
                    if (sector_data[i] != base[i]) {
			rc = FDC_ERR_VERIFY;
		    }
                }
	    }
        }
        break;
    case 0xB0:		/* seek */
	header[0] = disk_id[0];
	header[1] = disk_id[1];
	header[2] = fdc[dnr].last_track;
	header[3] = 1;
	rc = FDC_ERR_OK;
	break;
    case 0xC0:		/* bump (to track 0) */
	rc = FDC_ERR_OK;
	break;
    case 0xD0:		/* jump to buffer :-( */
	rc = FDC_ERR_DRIVE;
	break;
    case 0xE0:		/* execute when drive/head ready */
	/* we have to check for standard format code that is copied
	   to buffers 0-3 */
	if(DOS_IS_80(fdc[fnum].drive_type)) {
	    if (!memcmp(fdc[fnum].iprom, &fdc[fnum].buffer[0x100], 0x300)) {
	        int ntracks, nsectors = 0;
	        /* detected format code */
#ifdef FDC_DEBUG
	        log_message(fdc_log, "format code: ");
	        log_message(fdc_log, "   track for zones side 0: %d %d %d %d",
		    fdc[fnum].buffer[0xb0], fdc[fnum].buffer[0xb1],
		    fdc[fnum].buffer[0xb2], fdc[fnum].buffer[0xb3]);
	        log_message(fdc_log, "   track for zones side 1: %d %d %d %d",
		    fdc[fnum].buffer[0xb4], fdc[fnum].buffer[0xb5],
		    fdc[fnum].buffer[0xb6], fdc[fnum].buffer[0xb7]);
	        log_message(fdc_log, "   secs per track: %d %d %d %d",
		    fdc[fnum].buffer[0x99], fdc[fnum].buffer[0x9a],
		    fdc[fnum].buffer[0x9b], fdc[fnum].buffer[0x9c]);
	        log_message(fdc_log, "   vars: 870=%d 873=%d 875=%d",
		    fdc[fnum].buffer[0x470], fdc[fnum].buffer[0x473],
		    fdc[fnum].buffer[0x475]);
	        log_message(fdc_log, "   track=%d, sector=%d",
		    track, sector);
	        log_message(fdc_log, "   id=%02x,%02x (%c%c)",
		    header[0],header[1], header[0],header[1]);
	        log_message(fdc_log, "   sides=%d",
		    fdc[fnum].buffer[0xac]);
#endif
	        if (fdc[dnr].image->read_only) {
	            rc = FDC_ERR_WPROT;
	            break;
	        }
	        ntracks = (fdc[fnum].buffer[0xac] > 1) ? 154 : 77;

	        memset(sector_data, 0, 256);

	        for (rc = 0, track = 1; rc == 0 && track <= ntracks; track ++) {
		    if (track < 78) {
		        for (i=3; i >= 0; i--) {
			    if (track < fdc[fnum].buffer[0xb0 + i]) {
			        nsectors = fdc[fnum].buffer[0x99 + i];
			        break;
			    }
		        }
		    } else {
		        for (i=3; i >= 0; i--) {
			    if (track < fdc[fnum].buffer[0xb4 + i]) {
			        nsectors = fdc[fnum].buffer[0x99 + i];
			        break;
			    }
		        }
		    }
		    for (sector = 0; sector < nsectors; sector ++) {
                        rc = disk_image_write_sector(fdc[dnr].image, sector_data,
                                                 track, sector);
                        if (rc < 0) {
                            log_error(drive[dnr].log,
                  		"Could not update T:%d S:%d on disk image.",
                  		track, sector);
            		    rc = FDC_ERR_DCHECK;
			    break;
		        }
		    }
	        }

                vdrive_bam_set_disk_id(dnr + 8, header);
	    } 
	    if (!rc) {
	        rc = FDC_ERR_OK;
	    }
	} else 
	if(DOS_IS_40(fdc[fnum].drive_type)
	    || DOS_IS_30(fdc[fnum].drive_type)) {
	    if (!memcmp(fdc[fnum].iprom + 0x1000, &fdc[fnum].buffer[0x100], 0x200)) {
                static unsigned int sectorchangeat[4] = { 0, 17, 24, 30 };
	        int ntracks, nsectors = 0;

#ifdef FDC_DEBUG
	        log_message(fdc_log, "format code: ");
	        log_message(fdc_log, "   secs per track: %d %d %d %d",
		    fdc[fnum].buffer[0x99], fdc[fnum].buffer[0x9a],
		    fdc[fnum].buffer[0x9b], fdc[fnum].buffer[0x9c]);
	        log_message(fdc_log, "   track=%d, sector=%d",
		    track, sector);
	        log_message(fdc_log, "   id=%02x,%02x (%c%c)",
		    header[0],header[1], header[0],header[1]);
#endif
	        if (fdc[dnr].image->read_only) {
	            rc = FDC_ERR_WPROT;
	            break;
	        }
	        ntracks = 35;

	        memset(sector_data, 0, 256);

	        for (rc = 0, track = 1; rc == 0 && track <= ntracks; track ++) {
		    for (i=3; i >= 0; i--) {
			if (track > sectorchangeat[i]) {
		   	    nsectors = fdc[fnum].buffer[0x99 + 3 - i];
		    	    break;
		    	}
		    }
#ifdef FDC_DEBUG
	            log_message(fdc_log, "   track %d, -> %d sectors",
			track, nsectors);
#endif
		    for (sector = 0; sector < nsectors; sector ++) {
                        rc = disk_image_write_sector(fdc[dnr].image, 
				sector_data, track, sector);
                        if (rc < 0) {
                            log_error(drive[dnr].log,
                  		"Could not update T:%d S:%d on disk image.",
                  		track, sector);
            		    rc = FDC_ERR_DCHECK;
			    break;
		        }
		    }
	        }

                vdrive_bam_set_disk_id(dnr + 8, header);
	    } 
	    if (!rc) {
	        rc = FDC_ERR_OK;
	    }
	} else
	if(DOS_IS_20(fdc[fnum].drive_type)) {
	    if (!memcmp(fdc[fnum].iprom + 0x2000, &fdc[fnum].buffer[0x100], 0x200)) {
                /*
                static unsigned int sectorchangeat[4] = { 0, 17, 24, 30 };
	        int ntracks, nsectors = 0;
                */
#ifdef FDC_DEBUG
	        log_message(fdc_log, "format code: ");
	        log_message(fdc_log, "   secs per track: %d %d %d %d",
		    fdc[fnum].buffer[0x99], fdc[fnum].buffer[0x9a],
		    fdc[fnum].buffer[0x9b], fdc[fnum].buffer[0x9c]);
	        log_message(fdc_log, "   track=%d, sector=%d",
		    track, sector);
	        log_message(fdc_log, "   id=%02x,%02x (%c%c)",
		    header[0],header[1], header[0],header[1]);
#endif
#if 0
	        if (fdc[dnr].image->read_only) {
	            rc = FDC_ERR_WPROT;
	            break;
	        }
	        ntracks = 35;

	        memset(sector_data, 0, 256);

	        for (rc = 0, track = 1; rc == 0 && track <= ntracks; track ++) {
		    for (i=3; i >= 0; i--) {
			if (track > sectorchangeat[i]) {
		   	    nsectors = fdc[fnum].buffer[0x99 + 3 - i];
		    	    break;
		    	}
		    }
#ifdef FDC_DEBUG
	            log_message(fdc_log, "   track %d, -> %d sectors",
			track, nsectors);
#endif
		    for (sector = 0; sector < nsectors; sector ++) {
                        rc = disk_image_write_sector(fdc[dnr].image, 
				sector_data, track, sector);
                        if (rc < 0) {
                            log_error(drive[dnr].log,
                  		"Could not update T:%d S:%d on disk image.",
                  		track, sector);
            		    rc = FDC_ERR_DCHECK;
			    break;
		        }
		    }
	        }

                vdrive_bam_set_disk_id(dnr + 8, header);
#endif
	    } 
	    if (!rc) {
	        rc = FDC_ERR_OK;
	    }
	} else {
	    rc = FDC_ERR_DRIVE;
	}
	break;
    case 0xF0:
	if (header[0] != disk_id[0] || header[1] != disk_id[1]) {
	    rc = FDC_ERR_ID;
	    break;
	}
	/* try to read block header from disk */
	rc = FDC_ERR_OK;
	break;
    }

    drive[dnr].current_half_track = 2 * track;
    fdc[dnr].last_track = track;
    fdc[dnr].last_sector = sector;

    return (BYTE) rc;
}


static int int_fdc(int fnum, long offset)
{
    CLOCK rclk = drive_clk[fnum] - offset;
    int i, j;

#ifdef FDC_DEBUG
    if (fdc[fnum].fdc_state < FDC_RUN) {
	static int old_state[NUM_FDC] = { -1, -1 };
	if (fdc[fnum].fdc_state != old_state[fnum])
	    log_message(fdc_log, "int_fdc%d %d: state=%d\n",
					fnum, rclk, fdc[fnum].fdc_state);
	old_state[fnum] = fdc[fnum].fdc_state;
    }
#endif

    switch(fdc[fnum].fdc_state) {
    case FDC_RESET0:
	if (DOS_IS_80(fdc[fnum].drive_type)) {
            drive[fnum].current_half_track = 2 * 38;
	} else {
            drive[fnum].current_half_track = 2 * 18;
	}
	if (DOS_IS_80(fdc[fnum].drive_type)) {
	    fdc[fnum].buffer[0] = 2;
	} else {
	    fdc[fnum].buffer[0] = 0x3f;
	}
	fdc[fnum].fdc_state++;
	fdc[fnum].alarm_clk = rclk + 2000;
	alarm_set(&fdc[fnum].fdc_alarm, fdc[fnum].alarm_clk);
	break;
    case FDC_RESET1:
	if (DOS_IS_80(fdc[fnum].drive_type)) {
	    if (fdc[fnum].buffer[0] == 0) {
	        fdc[fnum].buffer[0] = 1;
	        fdc[fnum].fdc_state++;
	    }
	} else {
	    if (fdc[fnum].buffer[3] == 0xd0) {
	        fdc[fnum].buffer[3] = 0;
	        fdc[fnum].fdc_state++;
	    }
	}
	fdc[fnum].alarm_clk = rclk + 2000;
	alarm_set(&fdc[fnum].fdc_alarm, fdc[fnum].alarm_clk);
	break;
    case FDC_RESET2:
	if (DOS_IS_80(fdc[fnum].drive_type)) {
	
	    if (fdc[fnum].buffer[0] == 0) {
	        /* emulate routine written to buffer RAM */
	        fdc[fnum].buffer[1] = 0x0e;
	        fdc[fnum].buffer[2] = 0x2d;
	        /* number of sides on disk drive */
	        fdc[fnum].buffer[0xac] =
		    (fdc[fnum].drive_type == DRIVE_TYPE_8050) ? 1 : 2;
	        /* 0 = 4040 (2A), 1 = 8x80 (2C) drive type */
	        fdc[fnum].buffer[0xea] = 1;
	        fdc[fnum].buffer[0xee] = 5;	/* 3 for 4040, 5 for 8x50 */
	        fdc[fnum].buffer[0] = 3;	/* 5 for 4040, 3 for 8x50 */

	        fdc[fnum].fdc_state = FDC_RUN;
	        fdc[fnum].alarm_clk = rclk + 10000;
	    } else {
	        fdc[fnum].alarm_clk = rclk + 2000;
	    }
	} else 
	if (DOS_IS_40(fdc[fnum].drive_type)
	    || DOS_IS_30(fdc[fnum].drive_type)) {
	    if(fdc[fnum].buffer[0] == 0) {
	        fdc[fnum].buffer[0] = 0x0f;
	        fdc[fnum].fdc_state = FDC_RUN;
	        fdc[fnum].alarm_clk = rclk + 10000;
	    } else {
	        fdc[fnum].alarm_clk = rclk + 2000;
	    }
	}
	alarm_set(&fdc[fnum].fdc_alarm, fdc[fnum].alarm_clk);
	break;
    case FDC_RUN:
	/* check write protect switch */
	if (fdc[fnum].wps_change) {
	    fdc[fnum].buffer[0xA6] = 1;
	    fdc[fnum].wps_change--;
#ifdef FDC_DEBUG
	    log_message(fdc_log, "Detect Unit %d Drive %d wps change",
		fnum + 8, fnum);
#endif
	}
	if (fnum == 0 && DRIVE_IS_DUAL(fdc[0].drive_type)) {
	    if (fdc[1].wps_change) {
	        fdc[0].buffer[0xA6 + 1] = 1;
		fdc[1].wps_change--;
#ifdef FDC_DEBUG
	        log_message(fdc_log, "Detect Unit %d Drive 1 wps change",
		    fnum + 8);
#endif
	    }
	}

	/* check buffers */
	for (i=14; i >= 0; i--) {
	    /* job there? */
	    if (fdc[fnum].buffer[i + 3] > 127) {
		/* pointer to buffer/block header:
		    +0 = ID1
		    +1 = ID2
		    +2 = Track
		    +3 = Sector
		*/
		j = (i << 3) + 0x21;
#ifdef FDC_DEBUG
		log_message(fdc_log, "D/Buf %d/%x: Job code %02x t:%02d s:%02d",
			fnum, i, fdc[fnum].buffer[i+3],
			fdc[fnum].buffer[j+2],fdc[fnum].buffer[j+3]);
#endif
		fdc[fnum].buffer[i + 3] =
			fdc_do_job(fnum, 			/* FDC# */
				i,				/* buffer# */
				fdc[fnum].buffer[i+3] & 1,	/* drive */
				(int)(fdc[fnum].buffer[i+3] & 0xfe),
                                                                /* job code */
				&(fdc[fnum].buffer[j]) 		/* header */
			);
	    }
	}
	/* check "move head", by half tracks I guess... */
	for (i = 0; i < 2; i++) {
	    if (fdc[fnum].buffer[i + 0xa1]) {
#ifdef FDC_DEBUG
		log_message(fdc_log, "D %d: move head %d",
		    fnum, fdc[fnum].buffer[i + 0xa1]);
#endif
		fdc[fnum].buffer[i + 0xa1] = 0;
	    }
	}
	fdc[fnum].alarm_clk = rclk + 30000;
	alarm_set(&fdc[fnum].fdc_alarm, fdc[fnum].alarm_clk);
	/* job loop */
	break;
    }

    return 0;
}

static void clk_overflow_callback(int fnum, CLOCK sub, void *data)
{
    if (fdc[fnum].fdc_state != FDC_UNUSED) {
	if (fdc[fnum].alarm_clk > sub) {
	    fdc[fnum].alarm_clk -= sub;
	} else {
	    fdc[fnum].alarm_clk = 0;
	}
    }
}

void fdc_init(int fnum, BYTE *buffermem, BYTE *ipromp)
{
    fdc[fnum].buffer = buffermem;
    fdc[fnum].iprom = ipromp;

    if (fdc_log == LOG_ERR)
        fdc_log = log_open("fdc");

#ifdef FDC_DEBUG
    log_message(fdc_log, "fdc_init(drive %d)", fnum);
#endif


    if (fnum == 0) {
        alarm_init(&fdc[fnum].fdc_alarm, &drive0_context.cpu.alarm_context,
               "fdc0", int_fdc0);
        clk_guard_add_callback(&drive0_context.cpu.clk_guard, clk_overflow_callback0, NULL);
    } else
    if (fnum == 1) {
        alarm_init(&fdc[fnum].fdc_alarm, &drive1_context.cpu.alarm_context,
               "fdc1", int_fdc1);
        clk_guard_add_callback(&drive1_context.cpu.clk_guard, clk_overflow_callback1, NULL);
    }
}

/************************************************************************/

int fdc_attach_image(disk_image_t *image, unsigned int unit)
{
    int drive_no;
    int imgno;

#ifdef FDC_DEBUG
    log_message(fdc_log, "fdc_attach_image(image=%p, unit=%d)",
	image, unit);
#endif

    if (unit != 8 && unit != 9)
        return -1;

    if (DRIVE_IS_DUAL(fdc[0].drive_type)) {
	drive_no = 0;
    } else {
	drive_no = unit - 8;
    }
    imgno = unit - 8;

    if (fdc[drive_no].drive_type == DRIVE_TYPE_NONE)
	return 0;

    /* FIXME: hack - we need to save the image to be able to re-attach
       when the disk drive type changes */
    fdc[imgno].realimage = image;

    if (fdc[drive_no].drive_type == DRIVE_TYPE_8050
	|| fdc[drive_no].drive_type == DRIVE_TYPE_8250
	|| fdc[drive_no].drive_type == DRIVE_TYPE_1001) {
        switch(image->type) {
          case DISK_IMAGE_TYPE_D80:
            log_message(fdc_log, "Unit %d (%d): D80 disk image attached: %s",
                    unit, fdc[drive_no].drive_type, image->name);
            break;
          case DISK_IMAGE_TYPE_D82:
            log_message(fdc_log, "Unit %d (%d): D82 disk image attached: %s",
                    unit, fdc[drive_no].drive_type, image->name);
            break;
          default:
#ifdef FDC_DEBUG
	    log_message(fdc_log, "Could not attach image type %d to disk %d",
		image->type, fdc[drive_no].drive_type);
#endif
            return -1;
	}
    } else {
        switch(image->type) {
          case DISK_IMAGE_TYPE_D64:
            log_message(fdc_log, "Unit %d (%d): D64 disk image attached: %s",
                    unit, fdc[drive_no].drive_type, image->name);
            break;
          case DISK_IMAGE_TYPE_G64:
            log_message(fdc_log, "Unit %d (%d): G64 disk image attached: %s",
                    unit, fdc[drive_no].drive_type, image->name);
            break;
          case DISK_IMAGE_TYPE_X64:
            log_message(fdc_log, "Unit %d (%d): X64 disk image attached: %s",
                    unit, fdc[drive_no].drive_type, image->name);
            break;
          default:
#ifdef FDC_DEBUG
	    log_message(fdc_log, "Could not attach image type %d to disk %d",
		image->type, fdc[drive_no].drive_type);
#endif
            return -1;
	}
    }

    fdc[imgno].wps_change += 2;
    fdc[imgno].image = image;
    return 0;
}

int fdc_detach_image(disk_image_t *image, unsigned int unit)
{
    int drive_no;
    int imgno;

#ifdef FDC_DEBUG
    log_message(fdc_log, "fdc_detach_image(image=%p, unit=%d)",
	image, unit);
#endif

    if (unit != 8 && unit != 9)
        return -1;

    if (DRIVE_IS_DUAL(fdc[0].drive_type)) {
	drive_no = 0;
    } else {
	drive_no = unit - 8;
    }
    imgno = unit - 8;

    fdc[imgno].realimage = NULL;

    if (fdc[drive_no].drive_type == DRIVE_TYPE_8050
	|| fdc[drive_no].drive_type == DRIVE_TYPE_8250
	|| fdc[drive_no].drive_type == DRIVE_TYPE_1001) {
        switch(image->type) {
          case DISK_IMAGE_TYPE_D80:
            log_message(fdc_log, "Unit %d: D80 disk image attached: %s",
                    unit, image->name);
            break;
          case DISK_IMAGE_TYPE_D82:
            log_message(fdc_log, "Unit %d: D82 disk image attached: %s",
                    unit, image->name);
            break;
          default:
            return -1;
	}
    } else {
        switch(image->type) {
          case DISK_IMAGE_TYPE_D64:
            log_message(fdc_log, "Unit %d: D64 disk image attached: %s",
                    unit, image->name);
            break;
          case DISK_IMAGE_TYPE_G64:
            log_message(fdc_log, "Unit %d: G64 disk image attached: %s",
                    unit, image->name);
            break;
          case DISK_IMAGE_TYPE_X64:
            log_message(fdc_log, "Unit %d: X64 disk image attached: %s",
                    unit, image->name);
            break;
          default:
            return -1;
	}
    }

    fdc[imgno].wps_change += 2;
    fdc[imgno].image = NULL;
    return 0;
}

/************************************************************************/

/* The dump format has a module header and the data generated by the
 * chip...
 *
 * The version of this dump description is 0/0
 */

#define FDC_DUMP_VER_MAJOR      0
#define FDC_DUMP_VER_MINOR      0

/*
 * The dump data:
 *
 * UBYTE        STATE		FDC state
 * DWORD        CLK		clk ticks till next fdc invocation
 * UBYTE	NDRV		number of drives (1 or 2)
 * UBYTE        LTRACK0		last track
 * UBYTE        LSECTOR0	last sector
 * UBYTE        LTRACK1		last track (if ndrv == 2)
 * UBYTE        LSECTOR1	last sector (if ndrv == 2)
 *
 */


int fdc_write_snapshot_module(snapshot_t *p, int fnum)
{
    snapshot_module_t *m;
    char name[100];

    if (fdc[fnum].fdc_state == FDC_UNUSED) {
	return 0;
    }

    sprintf(name, "FDC%i", fnum);

    m = snapshot_module_create(p, name,
                              FDC_DUMP_VER_MAJOR, FDC_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    snapshot_module_write_byte(m, fdc[fnum].fdc_state);

    /* clk till next invocation */
    snapshot_module_write_dword(m,
                                (DWORD)(fdc[fnum].alarm_clk - drive_clk[fnum]));

    /* number of drives - so far 1 only */
    snapshot_module_write_byte(m, 1);

    /* last accessed track/sector */
    snapshot_module_write_byte(m, ((BYTE)(fdc[fnum].last_track)));
    snapshot_module_write_byte(m, ((BYTE)(fdc[fnum].last_sector)));

    snapshot_module_close(m);

    return 0;
}

int fdc_read_snapshot_module(snapshot_t *p, int fnum)
{
    BYTE vmajor, vminor;
    BYTE byte, ndrv;
    DWORD dword;
    snapshot_module_t *m;
    char name[100];

    sprintf(name, "FDC%d", fnum);

    m = snapshot_module_open(p, name, &vmajor, &vminor);
    if (m == NULL) {
	log_message(fdc_log, "Could not find snapshot module %s", name);
        return -1;
    }

    if (vmajor != FDC_DUMP_VER_MAJOR) {
        log_error(fdc_log,
                  "Snapshot module version (%d.%d) newer than %d.%d.",
                  vmajor, vminor, FDC_DUMP_VER_MAJOR, FDC_DUMP_VER_MINOR);
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_read_byte(m, &byte);
    if (byte > FDC_LAST_STATE) {
        snapshot_module_close(m);
	return -1;
    }
    fdc[fnum].fdc_state = byte;

    /* clk till next invocation */
    snapshot_module_read_dword(m, &dword);
    fdc[fnum].alarm_clk = drive_clk[fnum] + dword;
    alarm_set(&fdc[fnum].fdc_alarm, fdc[fnum].alarm_clk);

    /* number of drives - so far 1 only */
    snapshot_module_read_byte(m, &ndrv);

    /* last accessed track/sector */
    snapshot_module_read_byte(m, &byte);
    fdc[fnum].last_track = byte;
    snapshot_module_read_byte(m, &byte);
    fdc[fnum].last_sector = byte;

    if (ndrv > 1) {
	/* ignore drv 0 values */
        snapshot_module_read_byte(m, &byte);
        snapshot_module_read_byte(m, &byte);
    }

    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}

