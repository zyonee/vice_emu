/*
 * pc8477.c - dp8473/pc8477 emulation for the 4000 disk drive.
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

#include <string.h>

#include "clkguard.h"
#include "drive.h"
#include "drivecpu.h"
#include "drivetypes.h"
#include "log.h"
#include "lib.h"
#include "types.h"
#include "pc8477.h"
#include "alarm.h"
#include "drivesync.h"
#include "fdd.h"
#include "crc16.h"

#ifdef PC8477_DEBUG
#define debug(...) log_message(pc8477_log, __VA_ARGS__)
#else
#define debug(...) {}
#endif

const int fdd_data_rates[4] = {500, 300, 250, 1000}; /* kbit/s */

#define STEP_RATE ((16 - drv->step_rate) * drv->mycontext->drive->clock_frequency * 500000 / drv->rate)
#define BYTE_RATE (drv->mycontext->drive->clock_frequency * 8000 / drv->rate)

typedef enum pc8477_state_e {
    PC8477_WAIT, PC8477_COMMAND, PC8477_READ, PC8477_WRITE, PC8477_EXEC, PC8477_RESULT
} pc8477_state_t;

typedef enum pc8477_cmd_e {
    PC8477_CMD_INVALID            = 0x00,
    PC8477_CMD_MODE               = 0x01,
    PC8477_CMD_READ_A_TRACK       = 0x02,
    PC8477_CMD_SPECIFY            = 0x03,
    PC8477_CMD_SENSE_DRIVE_STATUS = 0x04,
    PC8477_CMD_WRITE_DATA         = 0x05,
    PC8477_CMD_READ_DATA          = 0x06,
    PC8477_CMD_RECALIBRATE        = 0x07,
    PC8477_CMD_SENSE_INTERRUPT    = 0x08,
    PC8477_CMD_WRITE_DELETED_DATA = 0x09,
    PC8477_CMD_READ_ID            = 0x0a,
    PC8477_CMD_READ_DELETED_DATA  = 0x0c,
    PC8477_CMD_FORMAT_A_TRACK     = 0x0d,
    PC8477_CMD_DUMPREG            = 0x0e,
    PC8477_CMD_SEEK               = 0x0f,
    PC8477_CMD_VERSION            = 0x10,
    PC8477_CMD_SCAN_EQUAL         = 0x11,
    PC8477_CMD_PERPENDICULAR_MODE = 0x12,
    PC8477_CMD_CONFIGURE          = 0x13,
    PC8477_CMD_LOCK               = 0x14,
    PC8477_CMD_VERIFY             = 0x16,
    PC8477_CMD_NSC                = 0x18,
    PC8477_CMD_SCAN_LOW_OR_EQUAL  = 0x19,
    PC8477_CMD_SCAN_HIGH_OR_EQUAL = 0x1d,
    PC8477_CMD_SET_TRACK          = 0x21,
    PC8477_CMD_RELATIVE_SEEK      = 0x8f
} pc8477_cmd_t;

enum pc8477_st0_e {
    PC8477_ST0_EC  = 0x10, /* Equipment check */
    PC8477_ST0_SE  = 0x20, /* Seek end */
};

enum pc8477_st1_e {
    PC8477_ST1_MA  = 0x01, /* Missing address mark */
    PC8477_ST1_NW  = 0x02, /* Not writable */
    PC8477_ST1_ND  = 0x04, /* No data */
    PC8477_ST1_OR  = 0x10, /* Overrun */
    PC8477_ST1_CE  = 0x20, /* CRC error */
    PC8477_ST1_EOT = 0x80, /* End of track */
};

enum pc8477_st2_e {
    PC8477_ST2_MD  = 0x01, /* Missing address mark in data field */
    PC8477_ST2_BT  = 0x02, /* Bad track */
    PC8477_ST2_SNS = 0x04, /* Scan not statisfied */
    PC8477_ST2_SEH = 0x08, /* Scan equal hit */
    PC8477_ST2_WT  = 0x10, /* Wrong track */
    PC8477_ST2_CM  = 0x20, /* CRC error in data field */
};

enum pc8477_st3_e {
    PC8477_ST3_TK0 = 0x10, /* Track 0 */
    PC8477_ST3_WP  = 0x40, /* Write protect */
};

enum pc8477_flags_e {
    PC8477_FLAGS_DS  = 0x01,
    PC8477_FLAGS_HDS = 0x02,
    PC8477_FLAGS_MOT = 0x04,
};

const static struct {
    BYTE mask; 
    pc8477_cmd_t command;
    BYTE len, rlen, flags;
} pc8477_commands[17]={
    {0x1f, PC8477_CMD_READ_DATA,          9, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0x1f, PC8477_CMD_READ_DELETED_DATA,  9, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0xbf, PC8477_CMD_READ_ID,            2, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0xbf, PC8477_CMD_FORMAT_A_TRACK,     6, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0x3f, PC8477_CMD_WRITE_DATA,         9, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0x3f, PC8477_CMD_WRITE_DELETED_DATA, 9, 7,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0xff, PC8477_CMD_SENSE_DRIVE_STATUS, 2, 1,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS},
    {0xff, PC8477_CMD_SPECIFY,            3, 0,  0},
    {0xff, PC8477_CMD_SEEK,               3, 0,  PC8477_FLAGS_DS | PC8477_FLAGS_HDS | PC8477_FLAGS_MOT},
    {0xff, PC8477_CMD_RECALIBRATE,        2, 0,  PC8477_FLAGS_DS},
    {0xbf, PC8477_CMD_SET_TRACK,          3, 0,  PC8477_FLAGS_DS},
    {0xff, PC8477_CMD_SENSE_INTERRUPT,    1, 2,  0},
    {0xff, PC8477_CMD_VERSION,            1, 1,  0},
    {0xff, PC8477_CMD_NSC,                1, 1,  0},
    {0xff, PC8477_CMD_DUMPREG,            1, 10, 0},
    {0xff, PC8477_CMD_PERPENDICULAR_MODE, 2, 0,  0},
    {0x00, PC8477_CMD_INVALID,            1, 1,  0}
};

struct pc8477_s {
    char *myname;
    pc8477_cmd_t command;
    pc8477_state_t state;
    int int_step, sub_step;
    struct drive_context_s *mycontext;

    /* Floppy drives */
    struct {
        fd_drive_t *fdd;
        int seeking; /* seeking status */
        int recalibrating; 
        int track; /* actual track register */
        int perpendicular; /* mode */
        int seek_pulses;
        int num;
        pc8477_motor_on_callback_t motor_on;
        void *motor_on_data;
        int motor_on_out;
    } fdds[4], *current;
    int seeking_active;
    fd_drive_t *fdd;
    int head_sel;
    int cmd_flags;
    int irq;
    WORD crc;

    CLOCK clk, motor_clk;

    /* Registers */
    BYTE st[4];
    BYTE dor, tdr;
    int step_rate, motor_off_time, motor_on_time, nodma;
    int rate;

    int sector; /* sector register */
    int is8477; /* dp8473 or pc 8477 */

    alarm_t *seek_alarm;
    int byte_count;
    int fifop, fifop2, fifo_size, fifo_fill;
    BYTE fifo[16];
    int cmdp, cmd_size;
    BYTE cmd[9];
    int resp, res_size;
    BYTE res[10];
};

static log_t pc8477_log = LOG_ERR;
void pc8477_set_motor(drive_t *drive, int on);

/*-----------------------------------------------------------------------*/

static void seek_alarm_handler(CLOCK offset, void *data)
{
    pc8477_t *drv = (pc8477_t *)data;
    int i;

    for (i = 0; i < 4; i++) {
        fdd_set_clk(drv->fdds[i].fdd, *drv->mycontext->clk_ptr);
        if (drv->fdds[i].seek_pulses < 0) {
            if (!fdd_track0(drv->fdds[i].fdd)) {
                continue;
            }
            fdd_step_pulse(drv->fdds[i].fdd, 1);
            drv->fdds[i].seek_pulses++;
            drv->fdds[i].seeking = 1;
            if (drv->fdds[i].recalibrating && drv->fdds[i].seek_pulses == 0
                    && fdd_track0(drv->fdds[i].fdd)) {
                drv->st[0] |= PC8477_ST0_EC;
            }
            break;
        }
        if (drv->fdds[i].seek_pulses > 0) {
            fdd_step_pulse(drv->fdds[i].fdd, 0);
            drv->fdds[i].seek_pulses--;
            drv->fdds[i].seeking = 1;
            break;
        }
    }

    if (i == 4) {
        alarm_unset(drv->seek_alarm);
        drv->seeking_active = 0;
        drv->st[0] |= PC8477_ST0_SE;
        drv->irq = 1;
    } else {
        alarm_set(drv->seek_alarm, *drv->mycontext->clk_ptr + STEP_RATE);
    }
}

/* Clock overflow handling.  */
static void clk_overflow_callback(CLOCK sub, void *data)
{
    pc8477_t *drv = (pc8477_t *)data;

    if (drv->clk > (CLOCK) 0)
        drv->clk -= sub;
    if (drv->motor_clk > (CLOCK) 0)
        drv->motor_clk -= sub;
}

/* Functions using drive context.  */
void pc8477_init(drive_context_t *drv)
{
    char *name;

    if (pc8477_log == LOG_ERR)
        pc8477_log = log_open("PC8477");

    clk_guard_add_callback(drv->cpu->clk_guard, clk_overflow_callback, drv->pc8477);

    name = lib_msprintf("%sEXEC", drv->pc8477->myname);
    drv->pc8477->seek_alarm = alarm_new(drv->cpu->alarm_context, name, seek_alarm_handler, drv->pc8477);
    lib_free(name);
}

void pc8477_setup_context(drive_context_t *drv)
{
    int i;
    drv->pc8477 = lib_calloc(1, sizeof(pc8477_t));
    drv->pc8477->myname = lib_msprintf("PC8477_%d", drv->mynumber);
    for (i = 0; i < 4; i++) {
        drv->pc8477->fdds[i].num = i;
        drv->pc8477->fdds[i].fdd = NULL;
        drv->pc8477->fdds[i].motor_on = NULL;
        drv->pc8477->fdds[i].motor_on_data = NULL;
    }
    drv->pc8477->fdds[0].motor_on = (pc8477_motor_on_callback_t)drivesync_set_4000;
    drv->pc8477->fdds[0].motor_on_data = (void *)drv;
    drv->pc8477->fdds[1].motor_on = (pc8477_motor_on_callback_t)pc8477_set_motor;
    drv->pc8477->fdds[1].motor_on_data = (void *)drv->drive;
    drv->pc8477->mycontext = drv;
}

/*-----------------------------------------------------------------------*/
/* WD1770 register read/write access.  */

static void pc8477_software_reset(pc8477_t *drv)
{
    drv->st[0] = 0xc0;
    drv->st[1] = 0x00;
    drv->st[2] = 0x00;
    drv->st[3] = 0x00;
    drv->state = PC8477_WAIT;
    drv->irq = 1;
    fdd_set_write_gate(drv->fdd, 1);
}

static void pc8477_result(pc8477_t *drv)
{
    switch (drv->command) {
    case PC8477_CMD_SPECIFY:
        return;
    case PC8477_CMD_SENSE_INTERRUPT:
        drv->res[0] = drv->st[0];
        drv->res[1] = drv->current->track;
        return;
    case PC8477_CMD_VERSION:
        drv->res[0] = 0x90;
        return;
    case PC8477_CMD_NSC:
        drv->res[0] = 0x72;
        return;
    case PC8477_CMD_SENSE_DRIVE_STATUS:
        fdd_set_clk(drv->fdd, drv->clk);
        drv->res[0] = drv->st[3] | 0x20 | (drv->is8477 ? 0x08 : 0)
                    | (fdd_track0(drv->fdd) ? 0 : PC8477_ST3_TK0)
                    | (fdd_write_protect(drv->fdd) ? 0 : PC8477_ST3_WP);
        return;
    case PC8477_CMD_READ_ID:
        memcpy(drv->res, drv->st, 3);
        return;
    case PC8477_CMD_RECALIBRATE:
        return;
    case PC8477_CMD_SEEK:
        return;
    case PC8477_CMD_DUMPREG:
        drv->res[0] = drv->fdds[0].track;
        drv->res[1] = drv->fdds[1].track;
        drv->res[2] = drv->fdds[2].track;
        drv->res[3] = drv->fdds[3].track;
        drv->res[4] = (drv->step_rate << 4) | drv->motor_off_time;
        drv->res[5] = (drv->motor_on_time << 1) | drv->nodma;
        drv->res[6] = drv->sector;
        drv->res[7] = (drv->fdds[0].perpendicular ? 0x02 : 0);
        drv->res[7] |= (drv->fdds[1].perpendicular ? 0x04 : 0);
        drv->res[7] |= (drv->fdds[2].perpendicular ? 0x08 : 0);
        drv->res[7] |= (drv->fdds[3].perpendicular ? 0x10 : 0);
        /* TODO */ 
        return;
    case PC8477_CMD_SET_TRACK:
        drv->res[0] = drv->current->track >> ((drv->cmd[1] & 4) ? 8 : 0);
        return;
    case PC8477_CMD_READ_DATA:
    case PC8477_CMD_READ_DELETED_DATA:
    case PC8477_CMD_WRITE_DATA:
    case PC8477_CMD_WRITE_DELETED_DATA:
    case PC8477_CMD_FORMAT_A_TRACK:
        memcpy(drv->res, drv->st, 3);
        memcpy(drv->res + 3, drv->cmd + 2, 4);
        return;
    default:
        drv->res[0] = drv->st[0];
    }
}

static int pc8477_micro_find_sync(pc8477_t *drv) {
    WORD w;

    for (; *drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE; drv->clk += BYTE_RATE) {
        fdd_set_clk(drv->fdd, drv->clk);
        if (fdd_index_count(drv->fdd) > 1) {
            return -1;
        }
        if (fdd_mfm_ready(drv->fdd)) {
            continue;
        }
        w = fdd_mfm_read(drv->fdd);
        switch (drv->sub_step) {
        case 0: /* zeros start */
            if (w != 0) {
                break;
            }
            drv->sub_step++;
            break;
        case 1: /* zeros end */
            if (w == 0) {
                break;
            }
            if (w != 0x1a1) {
                drv->sub_step = 0;
                break;
            }
            drv->sub_step++;
            break;
        case 2: /* sync end */
            if (w == 0x1a1) {
                break;
            }
            drv->sub_step = 0;
            return w;
        }
    }
    return 0x200;
}

static int pc8477_micro_readid(pc8477_t *drv) {
    BYTE b;

    for (; *drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE; drv->clk += BYTE_RATE) {
        fdd_set_clk(drv->fdd, drv->clk);
        if (fdd_mfm_ready(drv->fdd)) {
            continue;
        }
        b = (BYTE)fdd_mfm_read(drv->fdd);
        switch (drv->sub_step) {
        case 0: /* track */
            drv->res[3] = b; 
            break;
        case 1: /* head */
            drv->res[4] = b;
            break;
        case 2: /* sector */
            drv->res[5] = b;
            break;
        case 3: /* size */
            drv->res[6] = b;
            break;
        case 4: /* crc */
            break;
        case 5: /* crc */
            drv->st[1] &= ~PC8477_ST1_MA;
            return 0;
        }
        drv->sub_step++;
        if (fdd_index_count(drv->fdd) > 1) {
            return -1;
        }
    }
    return 1;
}

static pc8477_state_t pc8477_execute(pc8477_t *drv)
{
    int res = -1;

    switch (drv->command) {
    case PC8477_CMD_SPECIFY:
        debug("SPECIFY %d, %d, %d, %d", drv->cmd[1] >> 4, drv->cmd[1] & 0xf, drv->cmd[2] >> 1, drv->cmd[2] & 1);
        drv->step_rate = drv->cmd[1] >> 4;
        drv->motor_off_time = drv->cmd[1] & 0xf;
        drv->motor_on_time = drv->cmd[2] >> 1;
        drv->nodma = drv->cmd[2] & 1;
        return PC8477_WAIT;
    case PC8477_CMD_SENSE_INTERRUPT:
        if (!drv->irq) {
            break;
        }
        debug("SENSE INTERRUPT");
        drv->irq = 0;
        drv->current->seeking = 0; /* TODO: Too early */
        return PC8477_RESULT;
    case PC8477_CMD_VERSION:
        if (!drv->is8477) break;
        debug("VERSION");
        return PC8477_RESULT;
    case PC8477_CMD_NSC:
        if (!drv->is8477) break;
        debug("NSC");
        return PC8477_RESULT;
    case PC8477_CMD_SENSE_DRIVE_STATUS:
        debug("SENSE DRIVE STATUS #%d", drv->current->num);
        return PC8477_RESULT;
    case PC8477_CMD_READ_ID:
        switch (drv->int_step) {
        case 0:
            debug("READ ID #%d", drv->current->num);
            drv->st[1] |= PC8477_ST1_MA;
            drv->sub_step = 0;
            drv->int_step++;
        case 1:
            while (*drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE) {
                res = pc8477_micro_find_sync(drv);
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (res == 0xfe) {
                    break;
                }
            }
            if (res != 0xfe) {
                break;
            }
            drv->sub_step = 0;
            drv->int_step++;
        case 2:
            res = pc8477_micro_readid(drv);
            if (res > 0) {
                break;
            }
            if (res < 0) {
                drv->st[0] |= 0x40;
            }
            return PC8477_RESULT;
        }
        return PC8477_EXEC;
    case PC8477_CMD_RECALIBRATE:
        debug("RECALIBRATE #%d", drv->current->num);
        drv->current->seek_pulses = drv->is8477 ? -77 : -85;
        drv->current->track = 0;
        drv->current->recalibrating = 1;
        if (!drv->seeking_active) {
            alarm_set(drv->seek_alarm, *drv->mycontext->clk_ptr + STEP_RATE);
            drv->seeking_active = 1;
        }
        return PC8477_WAIT;
    case PC8477_CMD_SEEK:
        debug("SEEK #%d %d", drv->current->num, drv->cmd[2]);
        drv->current->seek_pulses = drv->cmd[2] - drv->current->track;
        drv->current->track = drv->cmd[2];
        drv->current->recalibrating = 0;
        if (!drv->seeking_active) {
            alarm_set(drv->seek_alarm, *drv->mycontext->clk_ptr + STEP_RATE);
            drv->seeking_active = 1;
        }
        return PC8477_WAIT;
    case PC8477_CMD_DUMPREG:
        if (!drv->is8477) break;
        debug("DUMPREG");
        return PC8477_RESULT;
    case PC8477_CMD_PERPENDICULAR_MODE:
        if (!drv->is8477) break;
        debug("PERPENDICULAR MODE %02x", drv->cmd[1]);
        if (drv->cmd[1] & 0x80) {
            for (res = 0; res < 4; res++) {
                drv->fdds[res].perpendicular = (drv->cmd[1] >> (2 + res)) & 1;
            }
        }
        return PC8477_WAIT;
    case PC8477_CMD_SET_TRACK:
        if ((drv->cmd[1] & 0xf8) != 0x30) break;
        debug("SET TRACK #%d %d", drv->current->num, drv->cmd[2]);
        if (drv->cmd[0] & 0x40) {
            if (drv->cmd[1] & 4) {
                drv->current->track = (drv->current->track & 0xff) | (drv->cmd[2] << 8);
            } else {
                drv->current->track = (drv->current->track & 0xff00) | drv->cmd[2];
            }
        }
        return PC8477_RESULT;
    case PC8477_CMD_READ_DATA:
    case PC8477_CMD_READ_DELETED_DATA:
        for (; *drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE; drv->clk += BYTE_RATE) {
            switch (drv->int_step) {
            case 0:
                if (drv->command == PC8477_CMD_READ_DATA) { 
                    debug("READ DATA #%d (%d/%d/%d)-%d %d", drv->current->num, drv->cmd[2], drv->cmd[3], drv->cmd[4], drv->cmd[6], 128 << drv->cmd[5]);
                } else {
                    debug("READ DELETED DATA #%d (%d/%d/%d)-%d %d", drv->current->num, drv->cmd[2], drv->cmd[3], drv->cmd[4], drv->cmd[6], 128 << drv->cmd[5]);
                }
                drv->sector = drv->cmd[4];
                drv->st[1] |= PC8477_ST1_MA;
                drv->sub_step = 0;
                drv->int_step++;
            case 1:
                res = pc8477_micro_find_sync(drv);
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (res != 0xfe) {
                    if (res == 0x200) {
                        return PC8477_READ;
                    }
                    break;
                }
                drv->st[1] &= ~PC8477_ST1_MA;
                drv->st[1] |= PC8477_ST1_ND;
                drv->sub_step = 0;
                drv->int_step++;
            case 2:
                res = pc8477_micro_readid(drv);
                if (res > 0) {
                    return PC8477_READ;
                }
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (0xff == drv->res[3]) {
                    drv->st[2] = PC8477_ST2_BT;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (drv->cmd[2] != drv->res[3]) {
                    drv->st[2] = PC8477_ST2_WT;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (drv->cmd[3] != drv->res[4] || drv->sector != drv->res[5]
                        || drv->cmd[5] != drv->res[6]) {
                    drv->sub_step = 0;
                    drv->int_step = 1;
                    break;
                }
                drv->byte_count = 128 << drv->res[6];
                drv->sub_step = 0;
                drv->int_step++;
            case 3:
                res = pc8477_micro_find_sync(drv);
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (res == 0x200) {
                    return PC8477_READ;
                }
                if (res == ((drv->command == PC8477_CMD_READ_DATA) ? 0xf8 : 0xfb)) {
                    drv->st[2] |= PC8477_ST2_CM;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (res != ((drv->command == PC8477_CMD_READ_DATA) ? 0xfb : 0xf8)) {
                    drv->st[2] |= PC8477_ST2_MD;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                drv->st[1] &= ~PC8477_ST1_ND;
                drv->int_step++;
            case 4:
                fdd_set_clk(drv->fdd, drv->clk);
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->fifo[drv->fifop2] = fdd_mfm_read(drv->fdd);
                drv->fifop2++;
                if (drv->fifop2 >= drv->fifo_size) {
                    drv->fifop2 = 0;
                }
                if (drv->fifo_fill >= drv->fifo_size) {
                    debug("Overrun");
                    drv->st[1] |= PC8477_ST1_OR;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                } else {
                    drv->fifo_fill++;
                }
                drv->byte_count--;
                if (drv->byte_count) {
                    break;
                }
                drv->int_step++;
            case 5:
                if (drv->fifo_fill) {
                    return PC8477_READ;
                }
                if (drv->cmd[6] != drv->sector) {
                    drv->sector++;
                    fdd_set_clk(drv->fdd, drv->clk);
                    fdd_index_count_reset(drv->fdd);
                    drv->sub_step = 0;
                    drv->int_step = 1;
                    break;
                }
                drv->st[1] |= PC8477_ST1_EOT;
                drv->st[0] |= 0x40;
                return PC8477_RESULT;
            }
        }
        return PC8477_READ;
    case PC8477_CMD_WRITE_DATA:
    case PC8477_CMD_WRITE_DELETED_DATA:
        for (; *drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE; drv->clk += BYTE_RATE) {
            switch (drv->int_step) {
            case 0:
                if (drv->command == PC8477_CMD_WRITE_DATA) { 
                    debug("WRITE DATA #%d (%d/%d/%d)-%d %d", drv->current->num, drv->cmd[2], drv->cmd[3], drv->cmd[4], drv->cmd[6], 128 << drv->cmd[5]);
                } else {
                    debug("WRITE DELETED DATA #%d (%d/%d/%d)-%d %d", drv->current->num, drv->cmd[2], drv->cmd[3], drv->cmd[4], drv->cmd[6], 128 << drv->cmd[5]);
                }
                drv->st[1] |= PC8477_ST1_MA;
                drv->sector = drv->cmd[4];
                drv->sub_step = 0;
                drv->int_step++;
            case 1:
                res = pc8477_micro_find_sync(drv);
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (res != 0xfe) {
                    if (res == 0x200) {
                        return PC8477_WRITE;
                    }
                    break;
                }
                drv->st[1] &= ~PC8477_ST1_MA;
                drv->st[1] |= PC8477_ST1_ND;
                drv->sub_step = 0;
                drv->int_step++;
            case 2:
                res = pc8477_micro_readid(drv);
                if (res > 0) {
                    return PC8477_WRITE;
                }
                if (res < 0) {
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (0xff == drv->res[3]) {
                    drv->st[2] = PC8477_ST2_BT;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (drv->cmd[2] != drv->res[3]) {
                    drv->st[2] = PC8477_ST2_WT;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (!fdd_write_protect(drv->fdd)) {
                    drv->st[1] |= PC8477_ST1_NW;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                if (drv->cmd[3] != drv->res[4] || drv->sector != drv->res[5]
                        || drv->cmd[5] != drv->res[6]) {
                    drv->sub_step = 0;
                    drv->int_step = 1;
                    break;
                }
                drv->st[1] &= ~PC8477_ST1_ND;
                drv->byte_count = 0;
                drv->crc = 0xffff;
                drv->int_step++;
            case 3:
                fdd_set_clk(drv->fdd, drv->clk);
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                res = (drv->rate == 1000 && drv->current->perpendicular) ? 41: 22;
                if (drv->byte_count < res) {
                    continue;
                }
                if (drv->byte_count < res + 12) {
                    fdd_set_write_gate(drv->fdd, 0);
                    fdd_mfm_write(drv->fdd, 0);
                    continue;
                }
                if (drv->byte_count < res + 12 + 3) {
                    fdd_mfm_write(drv->fdd, 0x1a1);
                    drv->crc = crc16(drv->crc, 0xa1);
                    continue;
                }
                res = (drv->command == PC8477_CMD_WRITE_DATA) ? 0xfb : 0xf8;
                fdd_mfm_write(drv->fdd, res);
                drv->crc = crc16(drv->crc, res);
                drv->int_step++;
                drv->byte_count = 128 << drv->res[6];
            case 4:
                fdd_set_clk(drv->fdd, drv->clk);
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                if (drv->fifo_fill == 0) {
                    fdd_set_write_gate(drv->fdd, 1);
                    debug("Underrun");
                    drv->st[1] |= PC8477_ST1_OR;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                res = drv->fifo[drv->fifop2];
                fdd_mfm_write(drv->fdd, res);
                drv->crc = crc16(drv->crc, res);
                drv->fifop2++;
                if (drv->fifop2 >= drv->fifo_size) {
                    drv->fifop2 = 0;
                }
                drv->fifo_fill--;
                drv->byte_count--;
                if (drv->byte_count) {
                    break;
                }
                drv->int_step++;
            case 5:
                fdd_set_clk(drv->fdd, drv->clk);
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                switch (drv->byte_count++) {
                case 0:
                    fdd_mfm_write(drv->fdd, drv->crc >> 8);
                    continue;
                case 1:
                    fdd_mfm_write(drv->fdd, drv->crc & 0xff);
                    continue;
                default:
                    fdd_set_write_gate(drv->fdd, 1);
                }
                if (drv->cmd[6] != drv->sector) {
                    drv->sector++;
                    fdd_index_count_reset(drv->fdd);
                    drv->sub_step = 0;
                    drv->int_step = 1;
                    break;
                }
                drv->st[1] |= 0x80; /* end of track */
                drv->st[0] |= 0x40;
                return PC8477_RESULT;
            }
        }
        return PC8477_WRITE;
    case PC8477_CMD_FORMAT_A_TRACK:
        for (; *drv->mycontext->clk_ptr >= drv->clk + BYTE_RATE; drv->clk += BYTE_RATE) {
            fdd_set_clk(drv->fdd, drv->clk);
            switch (drv->int_step) {
            case 0:
                debug("FORMAT TRACK #%d %d %d*%d %d %02x", drv->current->num, (drv->cmd[1] >> 2) & 1, 128 << drv->cmd[2], drv->cmd[3], drv->cmd[4], drv->cmd[5]);
                drv->sector = 0;
                drv->sub_step = 0;
                drv->int_step++;
            case 1:
                if (!fdd_index_count(drv->fdd)) {
                    break;
                }
                if (!fdd_write_protect(drv->fdd)) {
                    drv->st[1] |= PC8477_ST1_NW;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                fdd_set_write_gate(drv->fdd, 0);
                drv->int_step++;
                drv->byte_count = 80;
            case 2:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0x4e);
                drv->byte_count--;
                if (drv->byte_count) {
                    break;
                }
                drv->int_step++;
            case 3:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                if (drv->byte_count <= 12) {
                    fdd_mfm_write(drv->fdd, 0x00);
                    continue;
                }
                if (drv->byte_count <= 12 + 3) {
                    fdd_mfm_write(drv->fdd, 0x1a1);
                    continue;
                }
                if (drv->byte_count <= 12 + 3 + 1) {
                    fdd_mfm_write(drv->fdd, 0xfc);
                    continue;
                }
                if (drv->byte_count < 12 + 3 + 1 + 50) {
                    fdd_mfm_write(drv->fdd, 0x4e);
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0x4e);
                drv->byte_count = 0;
                drv->int_step++;
            case 4:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                if (drv->byte_count <= 12) {
                    fdd_mfm_write(drv->fdd, 0x00);
                    continue;
                }
                if (drv->byte_count <= 12 + 3) {
                    fdd_mfm_write(drv->fdd, 0x1a1);
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0xfe);
                drv->byte_count = 4;
                drv->crc = 0xb230;
                drv->int_step++;
            case 5:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                if (drv->fifo_fill == 0) {
                    fdd_set_write_gate(drv->fdd, 1);
                    debug("Underrun");
                    drv->st[1] |= PC8477_ST1_OR;
                    drv->st[0] |= 0x40;
                    return PC8477_RESULT;
                }
                res = drv->fifo[drv->fifop2];
                fdd_mfm_write(drv->fdd, res);
                drv->crc = crc16(drv->crc, res);
                drv->fifop2++;
                if (drv->fifop2 >= drv->fifo_size) {
                    drv->fifop2 = 0;
                }
                drv->fifo_fill--;
                drv->byte_count--;
                if (drv->byte_count) {
                    break;
                }
                drv->int_step++;
            case 6:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                if (drv->byte_count <= 1) {
                    fdd_mfm_write(drv->fdd, drv->crc >> 8);
                    continue;
                }
                if (drv->byte_count <= 2) {
                    fdd_mfm_write(drv->fdd, drv->crc & 0xff);
                    continue;
                }
                if (drv->byte_count < 2 + ((drv->rate == 1000 && drv->current->perpendicular) ? 41: 22)) {
                    fdd_mfm_write(drv->fdd, 0x4e);
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0x4e);
                drv->crc = 0xffff;
                drv->byte_count = 0;
                drv->int_step++;
            case 7:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                if (drv->byte_count <= 12) {
                    fdd_mfm_write(drv->fdd, 0x00);
                    continue;
                }
                if (drv->byte_count <= 12 + 3) {
                    fdd_mfm_write(drv->fdd, 0x1a1);
                    drv->crc = crc16(drv->crc, 0xa1);
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0xfb);
                drv->crc = crc16(drv->crc, 0xfb);
                drv->byte_count = 128 << drv->cmd[2];
                drv->int_step++;
            case 8:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                fdd_mfm_write(drv->fdd, drv->cmd[5]);
                drv->crc = crc16(drv->crc, drv->cmd[5]);
                drv->byte_count--;
                if (drv->byte_count) {
                    break;
                }
                drv->int_step++;
            case 9:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                if (drv->byte_count <= 1) {
                    fdd_mfm_write(drv->fdd, drv->crc >> 8);
                    continue;
                }
                if (drv->byte_count <= 2) {
                    fdd_mfm_write(drv->fdd, drv->crc & 0xff);
                    continue;
                }
                if (drv->byte_count < 2 + drv->cmd[4]) {
                    fdd_mfm_write(drv->fdd, 0x4e);
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0x4e);
                drv->sector++;
                if (drv->sector < drv->cmd[3]) {
                    drv->byte_count = 0;
                    drv->int_step = 4;
                    break;
                }
                drv->int_step++;
            case 10:
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                fdd_mfm_write(drv->fdd, 0x4e);
                break;
            }
            if (fdd_index_count(drv->fdd) > 1) {
                fdd_set_write_gate(drv->fdd, 1);
                drv->cmd[3] = drv->sector;
                drv->st[0] |= 0x40;
                return PC8477_RESULT;
            }
        }
        return PC8477_WRITE;
    default:
        break;
    }
    debug("invalid command %02x", drv->cmd[0]);
    drv->command = PC8477_CMD_INVALID;
    drv->st[0] = drv->st[3] | 0x80; /* invalid command */
    drv->res_size = 1;
    return PC8477_RESULT;
}

static void pc8477_store(pc8477_t *drv, WORD addr, BYTE byte)
{
    int i;

    if (drv->state == PC8477_EXEC || drv->state == PC8477_READ || drv->state == PC8477_WRITE) {
        drv->state = pc8477_execute(drv);
    } else {
        drv->clk = *drv->mycontext->clk_ptr;
    }

    switch (addr) {
    case 2: /* DCR */
        if (byte & 0x04) {
            if (!(drv->dor & 0x04)) {
                debug("RESET");
            }
            pc8477_software_reset(drv);
        }
        drv->dor = byte;
        for (i = 0; i < 4; i++) {
            if ((byte & (0x10 << i)) != drv->fdds[i].motor_on_out && drv->fdds[i].motor_on) {
                (drv->fdds[i].motor_on)(drv->fdds[i].motor_on_data, !drv->fdds[i].motor_on_out);
            }
            drv->fdds[i].motor_on_out = byte & (0x10 << i);
        }
        drv->current = &drv->fdds[byte & 3];
        drv->fdd = drv->current->fdd;
        break;
    case 3: /* TDR */
        drv->tdr = byte;
        break;
    case 5: /* DATA */
        switch (drv->state) {
        case PC8477_WAIT:
            drv->cmdp = 0;
            drv->resp = 0;
            for (i = 0; i < sizeof(pc8477_commands) / sizeof(pc8477_commands[0]); i++) {
                if (pc8477_commands[i].command == (pc8477_cmd_t)(pc8477_commands[i].mask & byte)) {
                    break;
                }
            }
            drv->command = pc8477_commands[i].command;
            drv->cmd_size = pc8477_commands[i].len;
            drv->res_size = pc8477_commands[i].rlen;
            drv->state = PC8477_COMMAND;
            drv->cmd_flags = pc8477_commands[i].flags;
            /* fall through */
        case PC8477_COMMAND:
            if (drv->cmdp < drv->cmd_size) {
                drv->cmd[drv->cmdp++] = byte;
            }
            if (drv->cmdp < drv->cmd_size) {
                return;
            }
            if (drv->command != PC8477_CMD_SENSE_INTERRUPT) {
                drv->st[1] = 0;
                drv->st[2] = 0;
                if (drv->cmd_flags & PC8477_FLAGS_DS) {
                    drv->current = &drv->fdds[drv->cmd[1] & 3];
                    drv->fdd = drv->current->fdd;
                    drv->st[3] = drv->cmd[1] & 3;
                } 
                if (drv->cmd_flags & PC8477_FLAGS_HDS) {
                    drv->head_sel = (drv->cmd[1] >> 2) & 1;
                    fdd_set_clk(drv->fdd, drv->clk);
                    fdd_set_side(drv->fdd, drv->head_sel);
                } 
                drv->st[3] = drv->current->num | (drv->head_sel << 2);
                drv->st[0] = drv->st[3];
                drv->irq = 0;
            }
            memset(drv->res, 0, sizeof(drv->res));
            drv->int_step = 0;
            drv->fifo_fill = 0;
            drv->fifop2 = drv->fifop;
            fdd_set_clk(drv->fdd, drv->clk);
            fdd_index_count_reset(drv->fdd);
            drv->state = pc8477_execute(drv);
            break;
        case PC8477_WRITE:
            if (drv->fifo_fill < drv->fifo_size) {
                drv->fifo[drv->fifop] = byte;
                drv->fifo_fill++;
                drv->fifop++;
                if (drv->fifop >= drv->fifo_size) {
                    drv->fifop = 0;
                }
            }
            return;
        case PC8477_EXEC:
            drv->st[0] |= 0x40;
            drv->state = PC8477_RESULT;
            return;
        case PC8477_RESULT:
        case PC8477_READ:
            return;
        }
        break;
    case 7: /* DRR */
        if (drv->rate != fdd_data_rates[byte & 3]) {
            for (i = 0; i < 4; i++) {
                fdd_set_clk(drv->fdds[i].fdd, drv->clk);
                fdd_set_rate(drv->fdds[i].fdd, byte);
            }
            drv->rate = fdd_data_rates[byte & 3];
        }
        break;
    }
}

static BYTE pc8477_read(pc8477_t *drv, WORD addr)
{
    BYTE result = 0;

    if (drv->state == PC8477_EXEC || drv->state == PC8477_READ || drv->state == PC8477_WRITE) {
        drv->state = pc8477_execute(drv);
    } else {
        drv->clk = *drv->mycontext->clk_ptr;
    }

    switch (addr) {
    case 2:
        if (drv->is8477) {
            return drv->dor;
        }
        break;
    case 3: /* TDR */
        if (drv->is8477) {
            result = (addr >> 8) & 0xfc;
            result |= drv->tdr & 0x03;
            return result;
        }
        break;
    case 4: /* MSR */
        result |= drv->fdds[0].seeking ? 0x01 : 0x00;
        result |= drv->fdds[1].seeking ? 0x02 : 0x00;
        result |= drv->fdds[2].seeking ? 0x04 : 0x00;
        result |= drv->fdds[3].seeking ? 0x08 : 0x00;

        if (drv->state != PC8477_WAIT) {
            result |= 0x10;
        }
        if (drv->nodma && (drv->state == PC8477_READ || drv->state == PC8477_WRITE)) {
            result |= 0x20;
        }
        if (drv->state == PC8477_READ || drv->state == PC8477_RESULT) {
            result |= 0x40;
        }
        if (drv->state != PC8477_EXEC) {
            result |= 0x80;
            if (drv->state == PC8477_READ && !drv->fifo_fill) {
                result &= ~0x80;
            }
            if (drv->state == PC8477_WRITE && drv->fifo_fill >= drv->fifo_size) {
                result &= ~0x80;
            }
        }
        return result;
    case 5: /* DATA */
        switch (drv->state) {
        case PC8477_WAIT:
        case PC8477_COMMAND:
        case PC8477_WRITE:
        case PC8477_EXEC:
            break;
        case PC8477_READ:
            result = drv->fifo[drv->fifop];
            if (drv->fifo_fill) {
                drv->fifo_fill--;
                drv->fifop++;
                if (drv->fifop >= drv->fifo_size) {
                    drv->fifop = 0;
                }
            }
            return result;
        case PC8477_RESULT:
            if (!drv->resp) {
                pc8477_result(drv);
                drv->irq = 0;
            }
            result = drv->res[drv->resp++];
            if (drv->resp >= drv->res_size) {
                drv->state = PC8477_WAIT;
            }
            return result;
        }
        break;
    case 7: /* DKR */
        fdd_set_clk(drv->fdd, drv->clk);
        result = (addr >> 8) & 0x7f;
        result |= fdd_disk_change(drv->fdd) ? 0x80 : 0;
        return result;
    }
    return addr >> 8; /* tri-state */
}

void pc8477_reset(pc8477_t *drv, int is8477)
{
    int i;
    drv->is8477 = is8477;
    for (i = 0; i < 4; i++) {
        drv->fdds[i].track = 0;
        drv->fdds[i].seeking = 0;
        if (drv->fdds[i].motor_on) {
            (drv->fdds[i].motor_on)(drv->fdds[i].motor_on_data, 0);
        }
        drv->fdds[i].motor_on_out = 0;
        drv->fdds[i].perpendicular = 0;
    }
    drv->current = &drv->fdds[0];
    drv->fdd = drv->current->fdd;
    drv->dor = 0;
    drv->tdr = 0;
    drv->rate = 250;
    memset(drv->fifo, 0, sizeof(drv->fifo));
    drv->fifo_size = 1;
    drv->clk = *drv->mycontext->clk_ptr;
    pc8477_software_reset(drv);
}

inline int pc8477_irq(pc8477_t *drv)
{
    return drv->irq;
}

inline void pc8477_set_fdd(pc8477_t *drv, fd_drive_t *fdd)
{
    drv->fdds[1].fdd = fdd;
}

void pc8477_set_motor(drive_t *drive, int on)
{
    fdd_set_clk(drive->fdds[0], *drive->clk);
    fdd_set_motor(drive->fdds[0], on);
}

/*-----------------------------------------------------------------------*/

void pc8477d_store(drive_context_t *drv, WORD addr, BYTE byte)
{
    pc8477_store(drv->pc8477, (WORD)(addr & 7), byte);
}

BYTE pc8477d_read(drive_context_t *drv, WORD addr)
{
    return pc8477_read(drv->pc8477, (WORD)(addr & 7));
}

