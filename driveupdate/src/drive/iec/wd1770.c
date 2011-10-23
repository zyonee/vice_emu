/*
 * wd1770.c - WD1770/1772 emulation for the 1571 and 1581 disk drives.
 *
 * Rewritten by
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

#include <stdio.h>
#include <string.h>

#include "clkguard.h"
#include "drive.h"
#include "drivecpu.h"
#include "drivetypes.h"
#include "log.h"
#include "types.h"
#include "wd1770.h"
#include "fdd.h"
#include "lib.h"
#include "crc16.h"

#ifdef WD1770_DEBUG
#define debug(...) log_message(wd1770_log, __VA_ARGS__)
#else
#define debug(...) {}
#endif

const static int wd1770_step_rate[2][4] = {
    {6000, 12000, 20000, 30000}, /* WD1770 */
    {6000, 12000, 2000, 3000},   /* WD1772 */
};

#define SETTLING 30000

/*-----------------------------------------------------------------------*/

/* WD1770/1772 registers.  */
typedef enum wd_reg_e {
    WD_STATUS  = 0,
    WD_COMMAND = 0,
    WD_TRACK   = 1,
    WD_SECTOR  = 2,
    WD_DATA    = 3
} wd_reg_t;

/* WD1770/1772 command bits  */
typedef enum wd_cflags_e {
    WD_A  = 0x01,
    WD_P  = 0x02,
    WD_R  = 0x03,
    WD_V  = 0x04,
    WD_E  = 0x04,
    WD_H  = 0x08,
    WD_U  = 0x10,
    WD_M  = 0x10,
    WD_I0 = 0x01,
    WD_I1 = 0x02,
    WD_I2 = 0x04,
    WD_I3 = 0x08,
} wd_cflags_t;

/* WD1770/1772 status bits  */
typedef enum wd_status_e {
    WD_MO  = 0x80,
    WD_WP  = 0x40,
    WD_SU  = 0x20,
    WD_RT  = 0x20,
    WD_SE  = 0x10,
    WD_RNF = 0x10,
    WD_CRC = 0x08,
    WD_T0  = 0x04,
    WD_LD  = 0x04,
    WD_IP  = 0x02,
    WD_DRQ = 0x02,
    WD_BSY = 0x01
} wd_status_t;

/* WD1770/1772 commands */
typedef enum wd_cmd_e {
    WD_RESTORE            = 0x00,
    WD_SEEK               = 0x10,
    WD_STEP               = 0x20,
    WD_STEP_IN            = 0x40,
    WD_STEP_OUT           = 0x60,
    WD_READ_SECTOR        = 0x80,
    WD_WRITE_SECTOR       = 0xa0,
    WD_READ_ADDRESS       = 0xc0,
    WD_FORCE_INTERRUPT    = 0xd0,
    WD_READ_TRACK         = 0xe0,
    WD_WRITE_TRACK        = 0xf0
} wd_cmd_t;

/* WD1770/1772 commands, masks, types */
const static struct {
    BYTE mask;
    wd_cmd_t command;
    BYTE type;
} wd_commands[11]={
    {0xf0, WD_RESTORE        , 1},
    {0xf0, WD_SEEK           , 1},
    {0xe0, WD_STEP           , 1},
    {0xe0, WD_STEP_IN        , 1},
    {0xe0, WD_STEP_OUT       , 1},
    {0xe0, WD_READ_SECTOR    , 2},
    {0xe0, WD_WRITE_SECTOR   , 2},
    {0xf0, WD_READ_ADDRESS   , 3},
    {0xf0, WD_READ_TRACK     , 3},
    {0xf0, WD_FORCE_INTERRUPT, 4},
    {0xf0, WD_WRITE_TRACK    , 3}
};

struct wd1770_s {
    /* WD1770/1772 registers.  */
    BYTE data, track, sector, status, cmd;
    WORD crc;

    /* Command and type  */
    wd_cmd_t command;
    int type;

    /* Floppy drive */
    fd_drive_t *fdd;
    int step;
    int byte_count;
    int tmp;
    int direction;
    int clock_frequency; /* MHz of main CPU*/

    CLOCK clk, *cpu_clk_ptr;

    int irq;
    int dden;
    int sync;
    int is1772;
};

static log_t wd1770_log = LOG_ERR;

/*-----------------------------------------------------------------------*/
/* WD1770 external interface.  */

/* Clock overflow handling.  */
static void clk_overflow_callback(CLOCK sub, void *data)
{
    wd1770_t *drv = (wd1770_t *)data;

    if (drv->clk > (CLOCK) 0)
        drv->clk -= sub;
}

/* Functions using drive context.  */
void wd1770_init(drive_context_t *drv)
{
    if (wd1770_log == LOG_ERR)
        wd1770_log = log_open("WD1770");

    drv->wd1770->is1772 = 0;
    drv->wd1770->clock_frequency = 2;

    clk_guard_add_callback(drv->cpu->clk_guard, clk_overflow_callback,
                           drv->wd1770);
}

void wd1770_setup_context(drive_context_t *drv)
{
    drv->wd1770 = lib_calloc(1, sizeof(wd1770_t));
    drv->wd1770->cpu_clk_ptr = drv->clk_ptr;
}

/* Execute microcode. Clock rate is 62.5 KHz for MFM, and 31.25 KHz for FM */
void wd1770_execute(wd1770_t *drv)
{
    int res;
    CLOCK clk;

    if ((drv->cmd & 0xf0) == WD_FORCE_INTERRUPT && drv->type > 0 && drv->type < 4) {
        drv->type = 4;
    }
    for (clk = (*(drv->cpu_clk_ptr) - drv->clk) / 2 / 16; clk > 0; clk--, drv->clk += 2 * 16) {
        fdd_set_clk(drv->fdd, drv->clk);
        switch (drv->type) {
        case -1:
            drv->status &= ~(WD_WP | WD_IP | WD_T0);
            drv->status |= fdd_index(drv->fdd) ? 0 : WD_IP;
            drv->status |= fdd_track0(drv->fdd) ? 0 : WD_T0;
            drv->status |= fdd_write_protect(drv->fdd) ? 0 : WD_WP;
        case 0: /* idle */
            if (drv->status & WD_BSY) {
                drv->status &= ~WD_BSY;
                drv->cmd = WD_FORCE_INTERRUPT;
                drv->irq = 1;
                drv->step = 0;
                fdd_index_count_reset(drv->fdd);
                continue;
            }
            switch (drv->step) {
            case 0:
                if (fdd_index_count(drv->fdd) >= 10) {
                    drv->status &= ~WD_MO;
                }
                if ((drv->cmd & 0xf0) != WD_FORCE_INTERRUPT) {
                    break;
                }
                if ((drv->cmd & WD_I2) && fdd_index_count(drv->fdd) != drv->tmp) {
                    drv->irq = 1;
                    drv->tmp = fdd_index_count(drv->fdd);
                }
                if (drv->cmd & WD_I3) {
                    drv->irq = 1;
                }
                continue;
            case 1:
                for (res = 0; res < sizeof(wd_commands) / sizeof(wd_commands[0]); res++) {
                    if (wd_commands[res].command == (wd_cmd_t)(wd_commands[res].mask & drv->cmd)) {
                        break;
                    }
                }
                drv->command = wd_commands[res].command;
                drv->type = wd_commands[res].type;
                drv->step = 0;
                drv->status |= WD_BSY;
                drv->irq = 0;
                switch (drv->command) {
                    case WD_RESTORE:
                        debug("RESTORE");
                        break;
                    case WD_SEEK:
                        debug("SEEK %d", drv->data);
                        break;
                    case WD_STEP:
                        debug("STEP %d", drv->direction ? -1 : 1);
                        break;
                    case WD_STEP_IN:
                        debug("STEP IN");
                        break;
                    case WD_STEP_OUT:
                        debug("STEP OUT");
                        break;
                    case WD_READ_SECTOR:
                        debug("READ SECTOR %d/%d", drv->track, drv->sector);
                        break;
                    case WD_WRITE_SECTOR:
                        debug("WRITE SECTOR %d/%d", drv->track, drv->sector);
                        break;
                    case WD_READ_ADDRESS:
                        debug("READ ADDRESS");
                        break;
                    case WD_READ_TRACK:
                        debug("READ TRACK");
                        break;
                    case WD_FORCE_INTERRUPT:
                        debug("FORCE INTERRUPT");
                        break;
                    case WD_WRITE_TRACK:
                        debug("WRITE TRACK");
                        break;
                }
                continue;
            }
            drv->step++;
            break;
        case 1: /* type 1 */
            switch (drv->step) {
            case 0: /* init */
                drv->status |= WD_BSY;
                drv->status &= ~(WD_CRC | WD_SE | WD_DRQ);
                break;
            case 1: /* spinup delay required? */
                if ((drv->cmd & WD_H) || (drv->status & WD_MO)) {
                    drv->status |= WD_MO;
                    drv->step += 2;
                    continue;
                }
                drv->status |= WD_MO;
                fdd_index_count_reset(drv->fdd);
                break;
            case 2: /* 6 revolution of spinup delay */
                if (fdd_index_count(drv->fdd) < 6) {
                    continue;
                }
                break;
            case 3: /* type of step */
                switch (drv->command) {
                case WD_STEP_IN:
                    drv->direction = 0;
                    break;
                case WD_STEP_OUT:
                    drv->direction = 1;
                case WD_STEP:
                    break;
                case WD_RESTORE:
                    drv->track = 0xff;
                    drv->data = 0x00;
                default:
                    drv->step++;
                    continue;
                }
                drv->step = (drv->cmd & WD_U) ? 5 : 6;
                continue;
            case 4: /* track reached? */
                if (drv->data == drv->track) {
                    drv->step = 8;
                    continue;
                }
                drv->direction = (drv->data < drv->track);
                break;
            case 5: /* update track register */
                drv->track += drv->direction ? -1 : 1;
                break;
            case 6: /* do step, unless on track 0 already */
                if (!fdd_track0(drv->fdd) && drv->direction) {
                    drv->track = 0;
                    drv->step = 8;
                    continue;
                }
                fdd_step_pulse(drv->fdd, drv->direction);
                drv->byte_count = wd1770_step_rate[drv->is1772][drv->cmd & WD_R];
                break;
            case 7: /* wait head movement */
                if (drv->byte_count > 0) {
                    drv->byte_count -= 16;
                    continue;
                }
                if (drv->cmd < WD_STEP) {
                    drv->step = 4;
                    continue;
                }
                break;
            case 8: /* verification required? */
                if (!(drv->cmd & WD_V)) {
                    drv->type = -1;
                    continue;
                }
                drv->byte_count = SETTLING;
                break;
            case 9: /* wait settling */
                if (drv->byte_count > 0) {
                    drv->byte_count -= 16;
                    continue;
                }
                fdd_index_count_reset(drv->fdd);
                drv->sync = 0;
                break;
            case 10: /* look for sync for 6 revolutions */
                if (fdd_index_count(drv->fdd) >= 6) {
                    drv->status |= WD_SE;
                    drv->type = -1;
                    continue;
                }
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (!drv->dden || res != 0x1fe) {
                    if (!drv->sync || res != 0xfe) {
                        drv->sync = (res == 0x1a1);
                        continue;
                    }
                }
                drv->sync = 0;
                drv->crc = 0xb230;
                drv->byte_count = 6;
                break;
            case 11: /* check id mark, store track */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (drv->byte_count == 6 && res != drv->track) {
                    drv->step--;
                    continue;
                }
                drv->crc = crc16(drv->crc, res);
                if (--drv->byte_count) {
                    continue;
                }
                if (drv->crc) {
                    drv->status |= WD_CRC;
                    drv->step--;
                    continue;
                }
                drv->status &= ~WD_CRC;
                drv->type = -1;
                continue;
            }
            drv->step++;
            break;
        case 2: /* type 2 */
            switch (drv->step) {
            case 0: /* init */
                drv->status |= WD_BSY;
                drv->status &= ~(WD_DRQ | WD_LD | WD_RNF | WD_RT | WD_WP);
                break;
            case 1: /* spinup delay required? */
                if ((drv->cmd & WD_H) || (drv->status & WD_MO)) {
                    drv->status |= WD_MO;
                    drv->step += 2;
                    continue;
                }
                drv->status |= WD_MO;
                fdd_index_count_reset(drv->fdd);
                break;
            case 2: /* 6 revolution of spinup delay */
                if (fdd_index_count(drv->fdd) < 6) {
                    continue;
                }
                break;
            case 3: /* settling required? */
                if (!(drv->cmd & WD_E)) {
                    drv->step += 2;
                    continue;
                }
                drv->byte_count = SETTLING;
                break;
            case 4: /* wait settling */
                if (drv->byte_count > 0) {
                    drv->byte_count -= 16;
                    continue;
                }
                break;
            case 5: /* writing on write protected media? */
                if (drv->command == WD_WRITE_SECTOR && !fdd_write_protect(drv->fdd)) {
                    drv->status |= WD_WP;
                    drv->type = 0;
                    continue;
                }
                fdd_index_count_reset(drv->fdd);
                drv->sync = 0;
                break;
            case 6: /* wait sync for 5 revolutions */
                if (fdd_index_count(drv->fdd) >= 5) {
                    drv->status |= WD_RNF;
                    drv->type = 0;
                    continue;
                }
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (!drv->dden || res != 0x1fe) {
                    if (!drv->sync || res != 0xfe) {
                        drv->sync = (res == 0x1a1);
                        continue;
                    }
                }
                drv->sync = 0;
                drv->crc = 0xb230;
                drv->byte_count = 6;
                break;
            case 7: /* check id mark */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (drv->byte_count == 6 && res != drv->track) {
                    drv->step--;
                    continue;
                }
                if (drv->byte_count == 4 && res != drv->sector) {
                    drv->step--;
                    continue;
                }
                if (drv->byte_count == 3) {
                    drv->tmp = res;
                }
                drv->crc = crc16(drv->crc, res);
                if (--drv->byte_count) {
                    continue;
                }
                if (drv->crc) {
                    drv->status |= WD_CRC;
                    drv->step--;
                    continue;
                }
                drv->status &= ~WD_CRC;
                drv->crc = 0xffff;
                if (drv->command == WD_WRITE_SECTOR) {
                    drv->byte_count = 0;
                    drv->step = 10;
                    continue;
                }
                drv->byte_count = 43;
                break;
            case 8: /* wait for data mark */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (!drv->byte_count--) {
                    drv->step -= 2;
                    continue;
                }
                if (!drv->dden || (res != 0x1fb && res != 0x1f8)) {
                    if (!drv->sync || (res != 0xfb && res != 0xf8)) {
                        if (!drv->sync) {
                            drv->crc = 0xffff;
                        }
                        drv->crc = crc16(drv->crc, res);
                        drv->sync = (res == 0x1a1);
                        continue;
                    }
                }
                drv->crc = crc16(drv->crc, res);
                drv->status |= ((res & 0xff) == 0xf8) ? WD_RT : 0;
                drv->byte_count = (128 << drv->tmp) + 2;
                break;
            case 9: /* read sector bytes */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (drv->byte_count > 2) {
                    drv->status |= (drv->status & WD_DRQ) ? WD_LD : WD_DRQ;
                    drv->data = res;
                }
                drv->crc = crc16(drv->crc, res);
                if (--drv->byte_count) {
                    continue;
                }
                if (drv->crc) {
                    drv->status |= WD_CRC;
                    drv->type = 0;
                    continue;
                }
                if (drv->cmd & WD_M) {
                    drv->sector++;
                    drv->step = 5;
                    continue;
                }
                drv->type = 0;
                continue;
            case 10: /* wait gap, write sync and header */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->byte_count++;
                drv->status |= (drv->byte_count == 2) ? WD_DRQ : 0;
                if (drv->byte_count == (2 + 9) && (drv->status & WD_DRQ)) {
                    drv->status ^= WD_DRQ | WD_LD;
                    drv->type = 0;
                    continue;
                }
                if (drv->byte_count < (drv->dden ? 0 : 11) + 2 + 9) {
                    continue;
                }
                if (drv->byte_count < (drv->dden ? 6 : (11 + 12)) + 2 + 9) {
                    fdd_set_write_gate(drv->fdd, 0);
                    fdd_mfm_write(drv->fdd, 0);
                    continue;
                }
                if (!drv->dden && drv->byte_count < (11 + 12 + 2 + 9 + 3)) {
                    fdd_mfm_write(drv->fdd, 0x1a1);
                    drv->crc = crc16(drv->crc, 0xa1);
                    continue;
                }
                res = ((drv->cmd & WD_A) ? 0xf8 : 0xfb) | (drv->dden ? 0x100 : 0);
                fdd_mfm_write(drv->fdd, res);
                drv->crc = crc16(drv->crc, res);
                drv->byte_count = (128 << drv->tmp) + 3;
                break;
            case 11: /* write bytes */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                switch (--drv->byte_count) {
                case 0:
                    fdd_mfm_write(drv->fdd, 0xff);
                    break;
                case 1:
                    fdd_mfm_write(drv->fdd, drv->crc & 0xff);
                    continue;
                case 2:
                    fdd_mfm_write(drv->fdd, drv->crc >> 8);
                    continue;
                default:
                    drv->status |= (drv->status & WD_DRQ) ? WD_LD : WD_DRQ;
                    drv->crc = crc16(drv->crc, drv->data);
                    fdd_mfm_write(drv->fdd, drv->data);
                    drv->data = 0;
                    continue;
                }
                fdd_set_write_gate(drv->fdd, 1);
                if (drv->cmd & WD_M) {
                    drv->sector++;
                    drv->step = 5;
                    continue;
                }
                drv->type = 0;
                continue;
            }
            drv->step++;
            break;
        case 3: /* type 3 */
            switch (drv->step) {
            case 0: /* init */
                drv->status |= WD_BSY;
                drv->status &= ~(WD_DRQ | WD_LD | WD_RNF | WD_CRC);
                break;
            case 1: /* spinup delay required? */
                if ((drv->cmd & WD_H) || (drv->status & WD_MO)) {
                    drv->status |= WD_MO;
                    drv->step += 2;
                    continue;
                }
                drv->status |= WD_MO;
                fdd_index_count_reset(drv->fdd);
                break;
            case 2: /* 6 revolution of spinup delay */
                if (fdd_index_count(drv->fdd) < 6) {
                    continue;
                }
                break;
            case 3: /* settling required? */
                if (!(drv->cmd & WD_E)) {
                    drv->step += 2;
                    continue;
                }
                drv->byte_count = SETTLING;
                break;
            case 4: /* wait settling */
                if (drv->byte_count > 0) {
                    drv->byte_count -= 16;
                    continue;
                }
                break;
            case 5: /* prepare */
                fdd_index_count_reset(drv->fdd);
                drv->sync = 0;
                if (drv->command == WD_WRITE_TRACK) {
                    if (!fdd_write_protect(drv->fdd)) {
                        drv->status |= WD_WP;
                        drv->type = 0;
                        continue;
                    }
                    drv->status |= WD_DRQ;
                    drv->byte_count = 3;
                    drv->step = 9;
                    continue;
                }
                if (drv->command != WD_READ_TRACK) {
                    drv->step += 2;
                    continue;
                }
                break;
            case 6: /* read track */
                if (fdd_index_count(drv->fdd) < 1) {
                    continue;
                }
                if (fdd_index_count(drv->fdd) > 1) {
                    drv->type = 0;
                    continue;
                }
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->data = fdd_mfm_read(drv->fdd);
                drv->status |= (drv->status & WD_DRQ) ? WD_LD : WD_DRQ;
                continue;
            case 7: /* read id, look for sync */
                if (fdd_index_count(drv->fdd) >= 6) {
                    drv->status |= WD_RNF;
                    drv->type = 0;
                    continue;
                }
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = fdd_mfm_read(drv->fdd);
                if (!drv->dden || res != 0x1fe) {
                    if (!drv->sync || res != 0xfe) {
                        drv->sync = (res == 0x1a1);
                        continue;
                    }
                }
                drv->crc = 0xb230;
                drv->byte_count = 6;
                break;
            case 8: /* read id */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                drv->status |= (drv->status & WD_DRQ) ? WD_LD : WD_DRQ;
                drv->data = fdd_mfm_read(drv->fdd);
                if (drv->byte_count == 6) {
                    drv->sector = drv->data;
                }
                drv->crc = crc16(drv->crc, drv->data);
                if (--drv->byte_count) {
                    continue;
                }
                drv->status |= drv->crc ? WD_CRC : 0;
                drv->type = 0;
                continue;
            case 9: /* write track prepare */
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                fdd_mfm_read(drv->fdd);
                if (--drv->byte_count) {
                    continue;
                }
                if (drv->status & WD_DRQ) {
                    drv->status ^= WD_DRQ | WD_LD;
                    drv->type = 0;
                    continue;
                }
                drv->byte_count = 0;
                drv->tmp = 0;
                break;
            case 10: /* write track */
                if (fdd_index_count(drv->fdd) < 1) {
                    continue;
                }
                drv->step++;
                fdd_set_write_gate(drv->fdd, 0);
            case 11: /* write track */
                if (fdd_index_count(drv->fdd) > 1) {
                    fdd_set_write_gate(drv->fdd, 1);
                    drv->status &= ~WD_DRQ;
                    drv->type = 0;
                    continue;
                }
                if (fdd_mfm_ready(drv->fdd)) {
                    continue;
                }
                res = drv->data;
                if (drv->byte_count) {
                    fdd_mfm_write(drv->fdd, drv->crc & 0xff);
                    drv->byte_count--;
                } else {
                    drv->status |= (drv->status & WD_DRQ) ? WD_LD : WD_DRQ;

                    if (drv->dden) { /* fm */
                        switch (res) {
                        case 0xf7:
                            drv->byte_count = 1;
                            res = drv->crc >> 8;
                            drv->tmp = 0;
                            break;
                        case 0xf8:
                        case 0xf9:
                        case 0xfa:
                        case 0xfb:
                        case 0xfe:
                            if (!drv->tmp) {
                                drv->crc = 0xffff;
                                drv->tmp = 1;
                            }
                        case 0xfc:
                            res |= 0x100;
                        }
                    } else { /* mfm */
                        switch (res) {
                        case 0xf5:
                            res = 0x1a1;
                            if (!drv->tmp) {
                                drv->crc = 0xffff;
                                drv->tmp = 1;
                            }
                            break;
                        case 0xf6:
                            res = 0x1c2;
                            break;
                        case 0xf7:
                            drv->byte_count = 1;
                            res = drv->crc >> 8;
                            drv->tmp = 0;
                            break;
                        }
                    }
                    if (drv->tmp) {
                        drv->crc = crc16(drv->crc, res);
                    }
                    fdd_mfm_write(drv->fdd, res);
                    drv->data = 0;
                }
                continue;
            }
            drv->step++;
            break;
        case 4: /* type 4 */
            switch (drv->step) {
            case 0:
                drv->status |= WD_BSY;
                drv->status &= ~WD_DRQ;
                fdd_set_write_gate(drv->fdd, 1);
                break;
            case 1:
                fdd_index_count_reset(drv->fdd);
                drv->tmp = fdd_index_count(drv->fdd);
                drv->type = -1;
                continue;
            }
            drv->step++;
            break;
        }
    }

}
/*-----------------------------------------------------------------------*/
/* WD1770 register read/write access.  */

static void wd1770_store(wd1770_t *drv, WORD addr, BYTE byte)
{
    wd1770_execute(drv);

    switch (addr) {
    case WD_COMMAND:
        drv->cmd = byte;
        break;
    case WD_TRACK:
        drv->track = byte;
        break;
    case WD_SECTOR:
        drv->sector = byte;
        break;
    case WD_DATA:
        drv->status &= ~WD_DRQ;
        drv->data = byte;
        break;
    }
}

static BYTE wd1770_read(wd1770_t *drv, WORD addr)
{
    wd1770_execute(drv);

    switch (addr) {
      case WD_STATUS:
          drv->irq = 0;
          return drv->status;
      case WD_TRACK:
          return drv->track;
      case WD_SECTOR:
          return drv->sector;
      case WD_DATA:
          drv->status &= ~WD_DRQ;
          return drv->data;
    }
    return 0;
}

void wd1770_reset(wd1770_t *drv)
{
    drv->type = 0;
    drv->status = 0;
    drv->track = 0;
    drv->sector = 0;
    drv->data = 0;
    drv->cmd = WD_FORCE_INTERRUPT;
    drv->step = -1;
    drv->clk = *drv->cpu_clk_ptr;
    fdd_set_write_gate(drv->fdd, 1);
    fdd_set_rate(drv->fdd, 2);
}

inline void wd1770_set_fdd(wd1770_t *drv, fd_drive_t *fdd)
{
    drv->fdd = fdd;
}

/*-----------------------------------------------------------------------*/

inline void wd1770d_store(drive_context_t *drv, WORD addr, BYTE byte)
{
    wd1770_store(drv->wd1770, (WORD)(addr & 3), byte);
}

inline BYTE wd1770d_read(drive_context_t *drv, WORD addr)
{
    return wd1770_read(drv->wd1770, (WORD)(addr & 3));
}

