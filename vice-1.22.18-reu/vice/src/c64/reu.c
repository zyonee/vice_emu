/*
 * reu.c - REU emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
 * 
 * Based on old code by
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *  Richard Hable <K3027E7@edvz.uni-linz.ac.at>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "c64io.h"
#include "cartridge.h"
#include "cmdline.h"
#include "interrupt.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "resources.h"
#include "reu.h"
#include "snapshot.h"
#ifdef HAS_TRANSLATION
#include "translate.h"
#endif
#include "types.h"
#include "util.h"


/**
#define REU_DEBUG
**/

/*
 * Status and Command Registers
 * bit  7       6       5       4       3       2       1       0
 * 00   Int     EOB     Fault   RamSize ________ Version ________
 * 01   Exec    0       Load    Delayed 0       0          Mode
 */

/*
 Offsets of the different REU registers
*/
enum {
    REU_REG_R_STATUS         = 0x00,
    REU_REG_RW_COMMAND       = 0x01,
    REU_REG_RW_BASEADDR_LOW  = 0x02,
    REU_REG_RW_BASEADDR_HIGH = 0x03,
    REU_REG_RW_RAMADDR_LOW   = 0x04,
    REU_REG_RW_RAMADDR_HIGH  = 0x05,
    REU_REG_RW_BANK          = 0x06,
    REU_REG_RW_BLOCKLEN_LOW  = 0x07,
    REU_REG_RW_BLOCKLEN_HIGH = 0x08,
    REU_REG_RW_INTERRUPT     = 0x09,
    REU_REG_RW_ADDR_CONTROL  = 0x0A,
    REU_REG_LAST_REG         = 0x1F
};

enum {
    REU_REG_R_STATUS_CHIPVERSION_MASK  = 0x0F,
    REU_REG_R_STATUS_64K_CHIPS         = 0x10,
    REU_REG_R_STATUS_VERIFY_ERROR      = 0x20,
    REU_REG_R_STATUS_END_OF_BLOCK      = 0x40,
    REU_REG_R_STATUS_INTERRUPT_PENDING = 0x80
};

enum {
    REU_REG_RW_COMMAND_TRANSFER_TYPE_MASK    = 0x03,
    REU_REG_RW_COMMAND_TRANSFER_TYPE_TO_REU     = 0x00,
    REU_REG_RW_COMMAND_TRANSFER_TYPE_FROM_REU   = 0x01,
    REU_REG_RW_COMMAND_TRANSFER_TYPE_SWAP       = 0x02,
    REU_REG_RW_COMMAND_TRANSFER_TYPE_VERIFY     = 0x03,
    REU_REG_RW_COMMAND_RESERVED_MASK         = 0x4C,
    REU_REG_RW_COMMAND_FF00_TRIGGER_DISABLED = 0x10,
    REU_REG_RW_COMMAND_AUTOLOAD              = 0x20,
    REU_REG_RW_COMMAND_EXECUTE               = 0x80
};

enum {
    REU_REG_RW_INTERRUPT_UNUSED_MASK          = 0x1F,
    REU_REG_RW_INTERRUPT_VERIFY_ENABLED       = 0x20,
    REU_REG_RW_INTERRUPT_END_OF_BLOCK_ENABLED = 0x40,
    REU_REG_RW_INTERRUPT_INTERRUPTS_ENABLED   = 0x80
};

enum {
    REU_REG_RW_ADDR_CONTROL_UNUSED_MASK       = 0x3f,
    REU_REG_RW_ADDR_CONTROL_FIX_REC           = 0x40,
    REU_REG_RW_ADDR_CONTROL_FIX_C64           = 0x80
};

/* REU registers */

typedef
struct rec_s {
    BYTE status;
    BYTE command;

    WORD base_computer;
    WORD base_reu;
    BYTE bank_reu;
    WORD transfer_length;

    BYTE int_mask_reg;
    BYTE address_control_reg;

    /* shadow registers for implementing the "Half-Autoload-Bug" */
    WORD base_computer_shadow;
    WORD base_reu_shadow;
    BYTE bank_reu_shadow;
    WORD transfer_length_shadow;
} rec_t;

static rec_t rec;


/* REU image.  */
static BYTE *reu_ram = NULL;
static int old_reu_ram_size = 0;

static log_t reu_log = LOG_ERR;

static int reu_activate(void);
static int reu_deactivate(void);

static unsigned int reu_int_num;

/* ------------------------------------------------------------------------- */

/* Flag: Do we enable the external REU?  */
int reu_enabled;

/* Size of the REU.  */
static int reu_size = 0;

/* Size of the REU in KB.  */
static int reu_size_kb = 0;

/* Filename of the REU image.  */
static char *reu_filename = NULL;


static int set_reu_enabled(int val, void *param)
{
    if (!val) {
        if (reu_enabled) {
            if (reu_deactivate() < 0) {
                return -1;
            }
        }
        reu_enabled = 0;
        return 0;
    } else { 
        if (!reu_enabled) {
            if (reu_activate() < 0) {
                return -1;
            }
        }

        reu_enabled = 1;
        return 0;
    }
}

static int set_reu_size(int val, void *param)
{
    if (val == reu_size_kb)
        return 0;

    switch (val) {
      case 128:
      case 256:
      case 512:
      case 1024:
      case 2048:
      case 4096:
      case 8192:
      case 16384:
        break;
      default:
        log_message(reu_log, "Unknown REU size %d.", val);
        return -1;
    }

    if (reu_enabled) {
        reu_deactivate();
        reu_size_kb = val;
        reu_size = reu_size_kb << 10;
        reu_activate();
    } else {
        reu_size_kb = val;
        reu_size = reu_size_kb << 10;
    }

    return 0;
}

static int set_reu_filename(const char *name, void *param)
{
    if (reu_filename != NULL && name != NULL
        && strcmp(name, reu_filename) == 0)
        return 0;

    if (name != NULL && *name != '\0') {
        if (util_check_filename_access(name) < 0)
            return -1;
    }

    if (reu_enabled) {
        reu_deactivate();
        util_string_set(&reu_filename, name);
        reu_activate();
    } else {
        util_string_set(&reu_filename, name);
    }

    return 0;
}

static const resource_string_t resources_string[] = {
    { "REUfilename", "", RES_EVENT_NO, NULL,
      &reu_filename, set_reu_filename, NULL },
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "REU", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &reu_enabled, set_reu_enabled, NULL },
    { "REUsize", 512, RES_EVENT_NO, NULL,
      &reu_size_kb, set_reu_size, NULL },
    { NULL }
};

int reu_resources_init(void)
{
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}

void reu_resources_shutdown(void)
{
    lib_free(reu_filename);
}

/* ------------------------------------------------------------------------- */

#ifdef HAS_TRANSLATION
static const cmdline_option_t cmdline_options[] =
{
    { "-reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t)1,
      0, IDCLS_ENABLE_REU },
    { "+reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t)0,
      0, IDCLS_DISABLE_REU },
    { "-reuimage", SET_RESOURCE, 1, NULL, NULL, "REUfilename", NULL,
      IDCLS_P_NAME, IDCLS_SPECIFY_REU_NAME },
    { "-reusize", SET_RESOURCE, 1, NULL, NULL, "REUsize", NULL,
      IDCLS_P_SIZE_IN_KB, IDCLS_REU_SIZE },
    { NULL }
};
#else
static const cmdline_option_t cmdline_options[] =
{
    { "-reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t)1,
      NULL, N_("Enable the RAM expansion unit") },
    { "+reu", SET_RESOURCE, 0, NULL, NULL, "REU", (resource_value_t)0,
      NULL, N_("Disable the RAM expansion unit") },
    { "-reuimage", SET_RESOURCE, 1, NULL, NULL, "REUfilename", NULL,
      N_("<name>"), N_("Specify name of REU image") },
    { "-reusize", SET_RESOURCE, 1, NULL, NULL, "REUsize", NULL,
      N_("<size in KB>"), N_("Size of the RAM expansion unit") },
    { NULL }
};
#endif

int reu_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

void reu_init(void)
{
    reu_log = log_open("REU");

    reu_int_num = interrupt_cpu_status_int_new(maincpu_int_status, "REU");
}

void reu_reset(void)
{
    memset(&rec, 0, sizeof rec);

    if (reu_size >= (256 << 10)) {
        rec.status |= REU_REG_R_STATUS_64K_CHIPS;
    }

    rec.command = REU_REG_RW_COMMAND_FF00_TRIGGER_DISABLED;

    rec.transfer_length =
    rec.transfer_length_shadow = 0xffff;

    rec.bank_reu =
    rec.bank_reu_shadow = 0xf8;

    rec.int_mask_reg = REU_REG_RW_INTERRUPT_UNUSED_MASK;

    rec.address_control_reg = REU_REG_RW_ADDR_CONTROL_UNUSED_MASK;
}

static int reu_activate(void)
{
    if (!reu_size)
        return 0;

    reu_ram = (BYTE *)lib_realloc((void *)reu_ram, (size_t)reu_size);

    /* Clear newly allocated RAM.  */
    if (reu_size > old_reu_ram_size)
        memset(reu_ram, 0, (size_t)(reu_size - old_reu_ram_size));

    old_reu_ram_size = reu_size;

    log_message(reu_log, "%dKB unit installed.", reu_size >> 10);

    if (!util_check_null_string(reu_filename)) {
        if (util_file_load(reu_filename, reu_ram, (size_t)reu_size,
                           UTIL_FILE_LOAD_RAW) < 0) {
            log_message(reu_log,
                        "Reading REU image %s failed.", reu_filename);
            if (util_file_save(reu_filename, reu_ram, reu_size) < 0) {
                log_message(reu_log,
                            "Creating REU image %s failed.", reu_filename);
                return -1;
            }
            log_message(reu_log, "Creating REU image %s.", reu_filename);
            return 0;
        }
        log_message(reu_log, "Reading REU image %s.", reu_filename);
    }

    reu_reset();
    return 0;
}

static int reu_deactivate(void)
{
    if (reu_ram == NULL)
        return 0;

    if (!util_check_null_string(reu_filename)) {
        if (util_file_save(reu_filename, reu_ram, reu_size) < 0) {
            log_message(reu_log,
                        "Writing REU image %s failed.", reu_filename);
            return -1;
        }
        log_message(reu_log, "Writing REU image %s.", reu_filename);
    }

    lib_free(reu_ram);
    reu_ram = NULL;
    old_reu_ram_size = 0;

    return 0;
}

void reu_shutdown(void)
{
    reu_deactivate();
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 reu_read(WORD addr)
{
    BYTE retval = 0xff;

    if (addr < 0x0b) { /*! \TODO remove magic number! */
        io_source = IO_SOURCE_REU;
    }

    switch (addr & REU_REG_LAST_REG) {
      case REU_REG_R_STATUS:
        retval = rec.status;

        /* Bits 7-5 are cleared when register is read, and pending IRQs are
           removed. */
        rec.status &= 
            ~(REU_REG_R_STATUS_VERIFY_ERROR 
              | REU_REG_R_STATUS_END_OF_BLOCK 
              | REU_REG_R_STATUS_INTERRUPT_PENDING
             );

        maincpu_set_irq(reu_int_num, 0);
        break;

      case REU_REG_RW_COMMAND:
        retval = rec.command;
        break;

      case REU_REG_RW_BASEADDR_LOW:
        retval = rec.base_computer & 0xff;
        break;

      case REU_REG_RW_BASEADDR_HIGH:
        retval = (rec.base_computer >> 8) & 0xff;
        break;

      case REU_REG_RW_RAMADDR_LOW:
        retval = rec.base_reu & 0xff;
        break;

      case REU_REG_RW_RAMADDR_HIGH:
        retval = (rec.bank_reu >> 8) & 0xff;
        break;

      case REU_REG_RW_BANK:
        retval = rec.bank_reu | 0xf8;
        break;

      case REU_REG_RW_BLOCKLEN_LOW:
        retval = rec.transfer_length & 0xff;
        break;

      case REU_REG_RW_BLOCKLEN_HIGH:
        retval = (rec.transfer_length >> 8) & 0xff;
        break;

      case REU_REG_RW_INTERRUPT:
        assert((rec.int_mask_reg & REU_REG_RW_INTERRUPT_UNUSED_MASK) == REU_REG_RW_INTERRUPT_UNUSED_MASK);
        retval = rec.int_mask_reg;
        break;

      case REU_REG_RW_ADDR_CONTROL:
        assert((rec.address_control_reg & REU_REG_RW_ADDR_CONTROL_UNUSED_MASK) == REU_REG_RW_ADDR_CONTROL_UNUSED_MASK);
        retval = rec.address_control_reg;
        break;
    }

#ifdef REU_DEBUG
    log_message(reu_log, "read [$%02X] => $%02X.", addr, retval);
#endif
    return retval;
}


void REGPARM2 reu_store(WORD addr, BYTE byte)
{
    switch (addr & REU_REG_LAST_REG)
    {
    case REU_REG_R_STATUS:
        /* REC status register is Read Only */
        break;

    case REU_REG_RW_COMMAND:
        rec.command = byte;
        break;

    case REU_REG_RW_BASEADDR_LOW:
        rec.base_computer = 
        rec.bank_reu_shadow = (rec.base_computer_shadow & 0xff00) | byte;
        break;

    case REU_REG_RW_BASEADDR_HIGH:
        rec.base_computer = 
        rec.bank_reu_shadow = (rec.base_computer_shadow & 0xff) | (byte << 8);
        break;

    case REU_REG_RW_RAMADDR_LOW:
        rec.base_reu =
        rec.base_reu_shadow = (rec.base_computer_shadow & 0xff00) | byte;
        break;

    case REU_REG_RW_RAMADDR_HIGH:
        rec.base_reu =
        rec.base_reu_shadow = (rec.base_computer_shadow & 0xff) | (byte << 8);
        break;

    case REU_REG_RW_BANK:
        //! \TODO
        break;

    case REU_REG_RW_BLOCKLEN_LOW:
        rec.transfer_length =
        rec.transfer_length_shadow = (rec.transfer_length_shadow & 0xff00) | byte;
        break;

    case REU_REG_RW_BLOCKLEN_HIGH:
        rec.transfer_length =
        rec.transfer_length_shadow = (rec.transfer_length_shadow & 0xff) | (byte << 8);
        break;

    case REU_REG_RW_INTERRUPT:
        rec.int_mask_reg = byte | REU_REG_RW_INTERRUPT_UNUSED_MASK;
        break;

    case REU_REG_RW_ADDR_CONTROL:
        rec.address_control_reg = byte | REU_REG_RW_ADDR_CONTROL_UNUSED_MASK;
        break;

    default:
        break;
    }

#ifdef REU_DEBUG
    log_message(reu_log, "store [$%02X] <= $%02X.", addr, (int)byte);
#endif

    /* write REC command register
     * DMA only if execution bit (7) set  - RH */
    /*! \BUG What if FF00 option is enabled? */
    if ((addr == REU_REG_RW_COMMAND) && (rec.command &  REU_REG_RW_COMMAND_EXECUTE)) {
        reu_dma(rec.command & REU_REG_RW_COMMAND_FF00_TRIGGER_DISABLED);
    }
}

/* ------------------------------------------------------------------------- */

static void reu_dma_update_regs(WORD host_addr, unsigned int reu_addr,
                                int len)
{
    if (!(rec.command & REU_REG_RW_COMMAND_AUTOLOAD)) {
        /* not autoload
         * incr. of addr. disabled, as already pointing to correct addr.
         * address changes only if not fixed, correct reu base registers  -RH
         */
#ifdef REU_DEBUG
        log_message(reu_log, "No autoload.");
#endif
        if ( (rec.address_control_reg & REU_REG_RW_ADDR_CONTROL_FIX_C64) == 0) {
            rec.base_computer = host_addr;
        }

        if ( (rec.address_control_reg & REU_REG_RW_ADDR_CONTROL_FIX_REC) == 0) {
            rec.base_reu = reu_addr & 0xffff;
            rec.bank_reu = (reu_addr >> 8) & 0xff;
        }

        rec.transfer_length = len;
    }
}

static void reu_dma_host_to_reu(WORD host_addr, unsigned int reu_addr, 
                                int host_step, int reu_step, int len)
{
    BYTE value;
#ifdef REU_DEBUG
    log_message(reu_log,
                "copy ext $%05X %s<= main $%04X%s, $%04X (%d) bytes.",
                reu_addr, reu_step ? "" : "(fixed) ", host_addr,
                host_step ? "" : " (fixed)", len, len);
#endif

    for (; len--; reu_addr += reu_step) {
        maincpu_clk++;
        machine_handle_pending_alarms(0);
        value = mem_read(host_addr);

#ifdef REU_DEBUG
        log_message(reu_log,
                    "Transferring byte: %x from main $%04X to ext $%05X.",
                    value, host_addr, reu_addr);
#endif

        reu_ram[reu_addr % reu_size] = value;
        host_addr = (host_addr + host_step) & 0xffff;
    }
    len = 0x1;
    rec.status |= REU_REG_R_STATUS_END_OF_BLOCK;
    reu_dma_update_regs(host_addr, reu_addr, len);
}

static void reu_dma_reu_to_host(WORD host_addr, unsigned int reu_addr,
                                int host_step, int reu_step, int len)
{
#ifdef REU_DEBUG
    log_message(reu_log,
                "copy ext $%05X %s=> main $%04X%s, $%04X (%d) bytes.",
                reu_addr, reu_step ? "" : "(fixed) ", host_addr,
                host_step ? "" : " (fixed)", len, len);
#endif

    for (; len--; reu_addr += reu_step) {
#ifdef REU_DEBUG
        log_message(reu_log,
                    "Transferring byte: %x from ext $%05X to main $%04X.",
                    reu_ram[reu_addr % reu_size], reu_addr, host_addr);
#endif
        maincpu_clk++;
        mem_store(host_addr, reu_ram[reu_addr % reu_size]);
        machine_handle_pending_alarms(0);
        host_addr = (host_addr + host_step) & 0xffff;
    }
    len = 1;
    rec.status |= REU_REG_R_STATUS_END_OF_BLOCK;
    reu_dma_update_regs(host_addr, reu_addr, len);
}

static void reu_dma_swap(WORD host_addr, unsigned int reu_addr,
                         int host_step, int reu_step, int len)
{
    BYTE c;
#ifdef REU_DEBUG
    log_message(reu_log,
                "swap ext $%05X %s<=> main $%04X%s, $%04X (%d) bytes.",
                reu_addr, reu_step ? "" : "(fixed) ", host_addr,
                host_step ? "" : " (fixed)", len, len);
#endif

    for (; len--; reu_addr += reu_step ) {
        c = reu_ram[reu_addr % reu_size];
        maincpu_clk++;
        machine_handle_pending_alarms(0);
        reu_ram[reu_addr % reu_size] = mem_read(host_addr);
        mem_store(host_addr, c);
        maincpu_clk++;
        machine_handle_pending_alarms(0);
        host_addr = (host_addr + host_step) & 0xffff;
    }
    len = 1;
    rec.status |= REU_REG_R_STATUS_END_OF_BLOCK;
    reu_dma_update_regs(host_addr, reu_addr, len);
}

static void reu_dma_compare(WORD host_addr, unsigned int reu_addr,
                            int host_step, int reu_step, int len)
{
#ifdef REU_DEBUG
    log_message(reu_log,
                "compare ext $%05X %s<=> main $%04X%s, $%04X (%d) bytes.",
                reu_addr, reu_step ? "" : "(fixed) ", host_addr,
                host_step ? "" : " (fixed)", len, len);
#endif

    rec.status &= ~ (REU_REG_R_STATUS_VERIFY_ERROR | REU_REG_R_STATUS_END_OF_BLOCK);

    while (len--) {
        maincpu_clk++;
        machine_handle_pending_alarms(0);
        if (reu_ram[reu_addr % reu_size] != mem_read(host_addr)) {
            host_addr = (host_addr + host_step) & 0xffff;
            reu_addr += reu_step;

            rec.status |= REU_REG_R_STATUS_VERIFY_ERROR;

            if (rec.int_mask_reg & (REU_REG_RW_INTERRUPT_END_OF_BLOCK_ENABLED | REU_REG_RW_INTERRUPT_INTERRUPTS_ENABLED)) {
                rec.status |= REU_REG_R_STATUS_INTERRUPT_PENDING;
                maincpu_set_irq(reu_int_num, 1);
            }
            break;
        }
        host_addr = (host_addr + host_step) & 0xffff;;
        reu_addr += reu_step;
    }

    if (len < 0) {
        /* all bytes are equal, mark End Of Block */
        rec.status |= REU_REG_R_STATUS_END_OF_BLOCK;
        len = 1;
    }

    reu_dma_update_regs(host_addr, reu_addr, len);
}

/* ------------------------------------------------------------------------- */

/* This function is called when write to REC command register or memory
 * location FF00 is detected.
 *
 * If host address exceeds ffff transfer contiues at 0000.
 * If reu address exceeds 7ffff transfer continues at 00000.
 * If address is fixed the same value is used during the whole transfer.
 */
/* Added correct handling of fixed addresses with transfer length 1  - RH */
/* Added fixed address support - [EP] */

void reu_dma(int immed)
{
    static int delay = 0;
    int len;
    int reu_step, host_step;
    WORD host_addr;
    unsigned int reu_addr, reu6_mask;

    if (!reu_enabled)
        return;

    if (!immed) {
        delay = 1;
        return;
    } else {
        if (!delay && immed < 0)
            return;
        delay = 0;
    }

    reu6_mask = (reu_size >> 16) - 1;

    /* wrong address of bank register & calculations corrected  - RH */
    host_addr = rec.base_computer;
    reu_addr = rec.base_reu | (rec.bank_reu << 16);
    len = rec.transfer_length ? rec.transfer_length : 0x10000;

    /* Fixed addresses implemented -- [EP] 04-16-97. */
    host_step = rec.address_control_reg & REU_REG_RW_ADDR_CONTROL_FIX_C64 ? 0 : 1;
    reu_step  = rec.address_control_reg & REU_REG_RW_ADDR_CONTROL_FIX_REC ? 0 : 1;

    switch (rec.command & REU_REG_RW_COMMAND_TRANSFER_TYPE_MASK) {
      case REU_REG_RW_COMMAND_TRANSFER_TYPE_TO_REU:
        reu_dma_host_to_reu(host_addr, reu_addr, host_step, reu_step, len);
        break;
      case REU_REG_RW_COMMAND_TRANSFER_TYPE_FROM_REU:
        reu_dma_reu_to_host(host_addr, reu_addr, host_step, reu_step, len);
        break;
      case REU_REG_RW_COMMAND_TRANSFER_TYPE_SWAP:
        reu_dma_swap(host_addr, reu_addr, host_step, reu_step, len);
        break;
      case REU_REG_RW_COMMAND_TRANSFER_TYPE_VERIFY:
        reu_dma_compare(host_addr, reu_addr, host_step, reu_step, len);
        break;
    }

    rec.command &= ~ REU_REG_RW_COMMAND_EXECUTE;

    /* Bit 7: interrupt enable.  */
    /* Bit 6: interrupt on end of block */
    if ((rec.int_mask_reg 
            & (REU_REG_RW_INTERRUPT_END_OF_BLOCK_ENABLED | REU_REG_RW_INTERRUPT_INTERRUPTS_ENABLED)) 
           == (REU_REG_RW_INTERRUPT_END_OF_BLOCK_ENABLED | REU_REG_RW_INTERRUPT_INTERRUPTS_ENABLED))
    {
        rec.status |= REU_REG_R_STATUS_INTERRUPT_PENDING;
        maincpu_set_irq(reu_int_num, 1);
    }
}

/* ------------------------------------------------------------------------- */

static char snap_module_name[] = "REU1764";
#define SNAP_MAJOR 0
#define SNAP_MINOR 0

int reu_write_snapshot_module(snapshot_t *s)
{
    snapshot_module_t *m;
    BYTE reu[16];

    memset(reu, 0xff, sizeof reu);

    m = snapshot_module_create(s, snap_module_name, SNAP_MAJOR, SNAP_MINOR);
    if (m == NULL)
        return -1;

    if (SMW_DW(m, (reu_size >> 10)) < 0
        || SMW_BA(m, reu, sizeof(reu)) < 0
        || SMW_BA(m, reu_ram, reu_size) < 0) {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_close(m);
    return 0;
}

int reu_read_snapshot_module(snapshot_t *s)
{
    BYTE major_version, minor_version;
    snapshot_module_t *m;
    DWORD size;
    BYTE reu[16];

    memset(reu, 0xff, sizeof reu);

    m = snapshot_module_open(s, snap_module_name,
                             &major_version, &minor_version);
    if (m == NULL)
        return -1;

    if (major_version != SNAP_MAJOR) {
        log_error(reu_log, "Major version %d not valid; should be %d.",
                major_version, SNAP_MAJOR);
        goto fail;
    }

    /* Read RAM size.  */
    if (SMR_DW(m, &size) < 0)
        goto fail;

    if (size > 16384) {
        log_error(reu_log, "Size %d in snapshot not supported.", (int)size);
        goto fail;
    }

    set_reu_size((int)size, NULL);

    if (!reu_enabled)
        set_reu_enabled(1, NULL);

    if (SMR_BA(m, reu, sizeof(reu)) < 0 || SMR_BA(m, reu_ram, reu_size) < 0)
        goto fail;

    if (reu[REU_REG_R_STATUS] & 0x80)
        interrupt_restore_irq(maincpu_int_status, reu_int_num, 1);
    else
        interrupt_restore_irq(maincpu_int_status, reu_int_num, 0);

    /*! \TODO restore the reu registers */

    snapshot_module_close(m);
    return 0;

fail:
    snapshot_module_close(m);
    return -1;
}

