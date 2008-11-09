/*! \file aciacore.c \n
 *  \author André Fachat, Spiro Trikaliotis\n
 *  \brief  Template file for ACIA 6551 emulation.
 *
 * Written by
 *  André Fachat <fachat@physik.tu-chemnitz.de>
 *  Spiro Trikaliotis
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

#include "acia.h"
#include "alarm.h"
#include "clkguard.h"
#include "cmdline.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "rs232drv.h"
#include "snapshot.h"
#ifdef HAS_TRANSLATION
#include "translate.h"
#endif
#include "types.h"


#undef  DEBUG   /*!< define if you want "normal" debugging output */
#undef  DEBUG_VERBOSE /*!< define if you want very verbose debugging output. */
/* #define DEBUG */
/* #define DEBUG_VERBOSE */

/*! \brief Helper macro for outputting debugging messages */
#ifdef DEBUG
# define DEBUG_LOG_MESSAGE(_x) log_message _x
#else
# define DEBUG_LOG_MESSAGE(_x)
#endif

/*! \brief Helper macro for outputting verbose debugging messages */
#ifdef DEBUG_VERBOSE
# define DEBUG_VERBOSE_LOG_MESSAGE(_x) log_message _x
#else
# define DEBUG_VERBOSE_LOG_MESSAGE(_x)
#endif

/*! \brief specify the transmit state the ACIA is currently in
*/
enum acia_tx_state {
    ACIA_TX_STATE_NO_TRANSMIT = 0, /*!< currently, no transmit */
    ACIA_TX_STATE_TX_STARTED = 1,  /*!< the transmit has already begun */
    ACIA_TX_STATE_DR_WRITTEN = 2   /*!< the data register has been written, but the byte written is not yet in transmit */
};

static alarm_t *acia_alarm_tx = NULL; /*!< handling of the transmit (TX) alarm */
static alarm_t *acia_alarm_rx = NULL; /*!< handling of the receive (RX) alarm */
static unsigned int acia_int_num;     /*!< the (internal) number for the ACIA interrupt. */

static int acia_ticks = 21111;  /*!< number of clock ticks per char */
static int fd = -1;             /*!< file descriptor used to access the RS232 physical device on the host machine */
static enum acia_tx_state in_tx = ACIA_TX_STATE_NO_TRANSMIT;   /*!< indicates that a transmit is currently ongoing */
static int irq = 0;
static BYTE cmd;        /*!< value of the 6551 command register */
static BYTE ctrl;       /*!< value of the 6551 control register */
static BYTE rxdata;     /*!< data that has been received last */
static BYTE txdata;     /*!< data prepared to be send */
static BYTE status;     /*!< value of the 6551 status register */
static BYTE ectrl;      /*!< value of the extended control register of the turbo232 card */
static int alarm_active_tx = 0;    /*!< 1 if TX alarm is set; else 0 */
static int alarm_active_rx = 0;    /*!< 1 if RX alarm is set; else 0 */

static log_t acia_log = LOG_ERR; /*!< the log where to write debugging messages */

static void int_acia_tx(CLOCK offset, void *data);
static void int_acia_rx(CLOCK offset, void *data);

static BYTE acia_last_read = 0;  /*!< the byte read the last time (for RMW) */

/******************************************************************/

/*! \brief the clock value the TX alarm has last been set to fire at

 \note
  If alarm_active_tx is set to 1, to alarm is
  actually set. If alarm_active_tx is 0, then
  the alarm either has already fired, or it
  already has been cancelled.
*/
static CLOCK acia_alarm_clk_tx = 0;

/*! \brief the clock value the RX alarm has last been set to fire at

 \note
  If alarm_active_rx is set to 1, to alarm is
  actually set. If alarm_active_rx is 0, then
  the alarm either has already fired, or it
  already has been cancelled.
*/
static CLOCK acia_alarm_clk_rx = 0;

/*! \brief the arch-dependant RS232 device to use for this acia implementation */
static int acia_device;

/*! \brief the type of interrupt implemented by the ACIA
 
 The ACIA either implements an IRQ (IK_IRQ), an NMI (IK_NMI),
 or no interrupt at all (IK_NONE).

 \note
   As some cartridges can be switched between these modes,
   it is necessary to remember this value.
*/
static enum cpu_int acia_irq = IK_NONE;

/*! \brief the type of interrupt implemented by the ACIA,
  as defined in the resources

  Essentially, this is the same info as acia_irq. As the
  resource stored in the VICE system is different from
  the actual value in acia_irq, the resources value is
  stored here.
*/
static int acia_irq_res;

/*! \brief the acia variant implemented.
  Specifies if this acia implements a "raw" 6551 device
  (ACIA_MODE_NORMAL), a swiftlink device (ACIA_MODE_SWIFTLINK)
  or a turbo232 device (ACIA_MODE_TURBO232).
*/
static int acia_mode = ACIA_MODE_NORMAL; 

/******************************************************************/

/*! \brief the bps rates available in the order of the control register

  This array is used to set the bps rate of the 6551.
  For this, the values are set in the same order as
  they are defined in the CONTROL register.

 \remark
   the first value is bogus. It should be 16*external clock.

 \remark
   swiftlink and turbo232 modes use the same table
   except they double the values.
*/
static const double acia_bps_table[16] = {
    10, 50, 75, 109.92, 134.58, 150, 300, 600, 1200, 1800,
    2400, 3600, 4800, 7200, 9600, 19200
};

/*! \brief the extra bps rates of the turbo232 card

   This lists the extra bps rates available in the
   turbo232 card. In the turbo232 card, if the CTRL register
   is set to the bps rate of 10 bps, the extended ctrl
   register determines the bps rates. The extended ctrl
   register is used as an index in this table to get the
   bps rate.

 \remark
   the last value is a bogus value and in the real module
   that value is reserved for future use.
*/
static const double t232_bps_table[4] = {
    230400, 115200, 57600, 28800
};

/******************************************************************/

static int acia_set_device(int val, void *param)
{

    if (fd >= 0) {
        log_error(acia_log,
                  "Device open, change effective only after close!");
    }

    acia_device = val;
    return 0;
}

static void acia_set_int(int aciairq, unsigned int int_num, int value)
{
    if (aciairq == IK_IRQ)
        mycpu_set_irq(int_num, value);
    if (aciairq == IK_NMI)
        mycpu_set_nmi(int_num, value);
}

static int acia_set_irq(int new_irq_res, void *param)
{
    int new_irq;
    static const int irq_tab[] = { IK_NONE, IK_IRQ, IK_NMI };

    if (new_irq_res < 0 || new_irq_res > 2)
        return -1;

    new_irq = irq_tab[new_irq_res];

    if (acia_irq != new_irq) {
        acia_set_int(acia_irq, acia_int_num, IK_NONE);
        if (irq) {
            acia_set_int(new_irq, acia_int_num, new_irq);
        }
    }
    acia_irq = new_irq;
    acia_irq_res = new_irq_res;

    return 0;
}

/*! \internal \brief get the bps rate ("baud rate") of the ACIA

 \return
   the bps rate the acia is currently programme to.
*/
static double get_acia_bps(void)
{
    switch(acia_mode) {
      case ACIA_MODE_NORMAL:
        return acia_bps_table[ctrl & ACIA_CTRL_BITS_BPS_MASK];

      case ACIA_MODE_SWIFTLINK:
        return acia_bps_table[ctrl & ACIA_CTRL_BITS_BPS_MASK] * 2;

      case ACIA_MODE_TURBO232:
        if ((ctrl & ACIA_CTRL_BITS_BPS_MASK) == ACIA_CTRL_BITS_BPS_16X_EXT_CLK)
            return t232_bps_table[ectrl & T232_ECTRL_BITS_EXT_BPS_MASK];
        else
            return acia_bps_table[ctrl & ACIA_CTRL_BITS_BPS_MASK] * 2;

      default:
        log_message(acia_log, "Invalid acia_mode = %u in get_acia_bps()", acia_mode);
        return acia_bps_table[0]; /* return dummy value */
    }
}

/*! \internal \brief set the ticks between characters of the ACIA according to the bps rate

 \return
   set the ticks that will pass for one character to be transferred
   according to the current ACIA settings.

 \todo
   Currently, 10 bits (for example, 8 data bits, 1 start and 1 stop bit)
   are hard-coded.
*/
static void set_acia_ticks(void)
{
    /* calculate time in ticks for 8 data bits, 1 start and 1 stop bit = 10 bit */
    acia_ticks = (int) (machine_get_cycles_per_second() / get_acia_bps() * 15);

    /* adjust the alarm rate for reception */
    if (alarm_active_rx) {
        acia_alarm_clk_rx = myclk + acia_ticks;
        alarm_set(acia_alarm_rx, acia_alarm_clk_rx);
        alarm_active_rx = 1;
    }
}

static int acia_set_mode(int new_mode, void *param)
{
    if (new_mode < ACIA_MODE_LOWEST || new_mode > ACIA_MODE_HIGHEST)
        return -1;

    acia_mode = new_mode;
    set_acia_ticks();

    return 0;
}

/*! \brief integer resources used by the ACIA module */
static const resource_int_t resources_int[] = {
    { MYACIA "Dev", MyDevice, RES_EVENT_NO, NULL,
      &acia_device, acia_set_device, NULL },
    { MYACIA "Irq", MyIrq, RES_EVENT_NO, NULL,
      &acia_irq_res, acia_set_irq, NULL },
    { MYACIA "Mode", ACIA_MODE_NORMAL, RES_EVENT_NO, NULL,
      &acia_mode, acia_set_mode, NULL },
    { NULL }
};

/*! \brief initialize the ACIA resources
 \return
   0 on success, else -1.

 \remark
   Registers the integer resources
*/
int myacia_init_resources(void)
{
    return resources_register_int(resources_int);
}

#ifdef HAS_TRANSLATION
static const cmdline_option_t cmdline_options[] = {
    { "-myaciadev", SET_RESOURCE, 1, NULL, NULL, MYACIA "Dev", NULL,
      IDCLS_P_0_3, IDCLS_SPECIFY_ACIA_RS232_DEVICE },
    { NULL }
};
#else
static const cmdline_option_t cmdline_options[] = {
    { "-myaciadev", SET_RESOURCE, 1, NULL, NULL, MYACIA "Dev", NULL,
      "<0-3>", N_("Specify RS232 device this ACIA should work on") },
    { NULL }
};
#endif

/*! \brief initialize the command-line options'
 \return
   0 on success, else -1.

 \remark
   Registers the command-line options
*/
int myacia_init_cmdline_options(void) {
    return cmdline_register_options(cmdline_options);
}

/******************************************************************/

/*! \internal \brief Prevent clock overflow by adjusting clock value

 \remark
   In order to prevent a clock overflow, the system is able
   to subtract a given amount from the clock values. When this
   happens, this function is called in order for the module to
   adjust its own values.
*/
static void clk_overflow_callback(CLOCK sub, void *var)
{
    acia_alarm_clk_tx -= sub;
    acia_alarm_clk_rx -= sub;
}

/*! \brief initialize the ACIA */
void myacia_init(void)
{
    acia_int_num = interrupt_cpu_status_int_new(maincpu_int_status, MYACIA);

    acia_alarm_tx = alarm_new(mycpu_alarm_context, MYACIA, int_acia_tx, NULL);
    acia_alarm_rx = alarm_new(mycpu_alarm_context, MYACIA, int_acia_rx, NULL);

    clk_guard_add_callback(mycpu_clk_guard, clk_overflow_callback, NULL);

    if (acia_log == LOG_ERR)
        acia_log = log_open(MYACIA);
}

/*! \brief reset the ACIA */
void myacia_reset(void)
{
    DEBUG_LOG_MESSAGE((acia_log, "reset_myacia"));

    cmd = ACIA_CMD_DEFAULT_AFTER_HW_RESET;
    ctrl = ACIA_CTRL_DEFAULT_AFTER_HW_RESET;
    ectrl = T232_ECTRL_DEFAULT_AFTER_HW_RESET;

    set_acia_ticks();

    status = ACIA_SR_DEFAULT_AFTER_HW_RESET;
    in_tx = ACIA_TX_STATE_NO_TRANSMIT;

    if (fd >= 0)
        rs232drv_close(fd);
    fd = -1;

    alarm_unset(acia_alarm_tx);
    alarm_unset(acia_alarm_rx);
    alarm_active_tx = 0;
    alarm_active_rx = 0;

    acia_set_int(acia_irq, acia_int_num, 0);
    irq = 0;
}

/* -------------------------------------------------------------------------- */
/* The dump format has a module header and the data generated by the
 * chip...
 *
 * The version of this dump description is 0/0
 */

#define ACIA_DUMP_VER_MAJOR      1
#define ACIA_DUMP_VER_MINOR      0

/*
 * The dump data:
 *
 * UBYTE        TDR     Transmit Data Register
 * UBYTE        RDR     Receiver Data Register
 * UBYTE        SR      Status Register (includes state of IRQ line)
 * UBYTE        CMD     Command Register
 * UBYTE        CTRL    Control Register
 *
 * UBYTE        INTX    0 = no data to tx; 2 = TDR valid; 1 = in transmit
 *
 * DWORD        TICKS   ticks till the next TDR empty interrupt
 *
 * DWORD        TICKS2  ticks till the next RDF empty interrupt
 *                      TICKS2 has been added with 2.0.9; if it does not
 *                      exist on read, it is assumed that it has the same
 *                      value as TICKS to emulate the old behaviour.
 */

static const char module_name[] = MYACIA;

/* FIXME!!!  Error check.  */
/* FIXME!!!  If no connection, emulate carrier lost or so */
int myacia_snapshot_write_module(snapshot_t *p)
{
    snapshot_module_t *m;

    m = snapshot_module_create(p, module_name, (BYTE)ACIA_DUMP_VER_MAJOR,
                               (BYTE)ACIA_DUMP_VER_MINOR);
    if (m == NULL)
        return -1;

    SMW_B(m, txdata);
    SMW_B(m, rxdata);
    SMW_B(m, (BYTE)(status | (irq ? ACIA_SR_BITS_IRQ : 0)));
    SMW_B(m, cmd);
    SMW_B(m, ctrl);
    SMW_B(m, (BYTE)(in_tx));

    if (alarm_active_tx) {
        SMW_DW(m, (acia_alarm_clk_tx - myclk));
    } else {
        SMW_DW(m, 0);
    }

    if (alarm_active_rx) {
        SMW_DW(m, (acia_alarm_clk_rx - myclk));
    } else {
        SMW_DW(m, 0);
    }

    snapshot_module_close(m);

    return 0;
}

int myacia_snapshot_read_module(snapshot_t *p)
{
    BYTE vmajor, vminor;
    BYTE byte;
    DWORD dword;
    snapshot_module_t *m;

    alarm_unset(acia_alarm_tx);   /* just in case we don't find module */
    alarm_unset(acia_alarm_rx);   /* just in case we don't find module */
    alarm_active_tx = 0;
    alarm_active_rx = 0;

    mycpu_set_int_noclk(acia_int_num, 0);

    m = snapshot_module_open(p, module_name, &vmajor, &vminor);
    if (m == NULL)
        return -1;

    if (vmajor != ACIA_DUMP_VER_MAJOR) {
        snapshot_module_close(m);
        return -1;
    }

    SMR_B(m, &txdata);
    SMR_B(m, &rxdata);

    irq = 0;
    SMR_B(m, &status);
    if (status & ACIA_SR_BITS_IRQ) {
        status &= ~ACIA_SR_BITS_IRQ;
        irq = 1;
        mycpu_set_int_noclk(acia_int_num, acia_irq);
    } else {
        mycpu_set_int_noclk(acia_int_num, 0);
    }

    SMR_B(m, &cmd);
    if ((cmd & ACIA_CMD_BITS_DTR_ENABLE_RECV_AND_IRQ) && (fd < 0)) {
        fd = rs232drv_open(acia_device);
    } else
        if ((fd >= 0) && !(cmd & ACIA_CMD_BITS_DTR_ENABLE_RECV_AND_IRQ)) {
        rs232drv_close(fd);
        fd = -1;
    }

    SMR_B(m, &ctrl);
    set_acia_ticks();

    SMR_B(m, &byte);
    in_tx = byte;

    SMR_DW(m, &dword);
    if (dword) {
        acia_alarm_clk_tx = myclk + dword;
        alarm_set(acia_alarm_tx, acia_alarm_clk_tx);
        alarm_active_tx = 1;

        /*
         * for compatibility reasons of old snapshots with new ones,
         * set the RX alarm to the same value.
         * if we have a new snapshot (2.0.8 and up), this will be
         * overwritten directly afterwards.
         */
        acia_alarm_clk_rx = myclk + dword;
        alarm_set(acia_alarm_rx, acia_alarm_clk_rx);
        alarm_active_rx = 1;

    }

    /*
     * this is new with VICE 2.0.8; thus, only use the settings
     * if it does exist.
     */
    if ( SMR_DW(m, &dword) >= 0 ) {
        if (dword) {
            acia_alarm_clk_rx = myclk + dword;
            alarm_set(acia_alarm_rx, acia_alarm_clk_rx);
            alarm_active_rx = 1;
        } else {
            alarm_unset(acia_alarm_rx);
            alarm_active_rx = 0;
        }
    }


    if (snapshot_module_close(m) < 0)
        return -1;

    return 0;
}


/*! \brief write the ACIA register values
  This function is used to write the ACIA values from the computer.

  \param addr
    The address of the ACIA register to write

  \param byte
    The value to set the register to
*/
void REGPARM2 myacia_store(WORD a, BYTE b)
{
    int acia_register_size;

    DEBUG_LOG_MESSAGE((acia_log, "store_myacia(%04x,%02x)", a, b));

    if (mycpu_rmw_flag) {
        myclk --;
        mycpu_rmw_flag = 0;
        myacia_store(a, acia_last_read);
        myclk ++;
    }

    if (acia_mode==ACIA_MODE_TURBO232)
      acia_register_size=7;
    else
      acia_register_size=3;

    switch(a & acia_register_size) {
      case ACIA_DR:
        txdata = b;
        if (cmd & ACIA_CMD_BITS_DTR_ENABLE_RECV_AND_IRQ) {
            if (in_tx != ACIA_TX_STATE_NO_TRANSMIT) {
                acia_alarm_clk_tx = myclk + 1;
                alarm_set(acia_alarm_tx, acia_alarm_clk_tx);
                alarm_active_tx = 1;
                in_tx = ACIA_TX_STATE_DR_WRITTEN;
            } else {
                if (in_tx == ACIA_TX_STATE_DR_WRITTEN) {
                    log_message(acia_log, "ACIA: data register written "
                        "although data has not been sent yet.");
                }
                in_tx = ACIA_TX_STATE_DR_WRITTEN;
            }
            status &= ~ ACIA_SR_BITS_TRANSMIT_DR_EMPTY; /* clr TDRE */
        }
        break;
      case ACIA_SR:
        if (fd >= 0)
            rs232drv_close(fd);
        fd = -1;
        status &= ~ ACIA_SR_BITS_OVERRUN_ERROR;
        cmd &= ACIA_CMD_BITS_PARITY_TYPE_MASK | ACIA_CMD_BITS_PARITY_ENABLED;
        in_tx = ACIA_TX_STATE_NO_TRANSMIT;
        acia_set_int(acia_irq, acia_int_num, 0);
        irq = 0;
        alarm_unset(acia_alarm_tx);
        alarm_active_tx = 0;
        break;
      case ACIA_CTRL:
        ctrl = b;
        set_acia_ticks();
        break;
      case ACIA_CMD:
        cmd = b;
        if ((cmd & ACIA_CMD_BITS_DTR_ENABLE_RECV_AND_IRQ) && (fd < 0)) {
            fd = rs232drv_open(acia_device);
            acia_alarm_clk_tx = myclk + acia_ticks;
            alarm_set(acia_alarm_tx, acia_alarm_clk_tx);
            alarm_active_tx = 1;
            /* enable RX alarm */
            alarm_active_rx = 1;
            set_acia_ticks();
        } else
            if ((fd >= 0) && !(cmd & ACIA_CMD_BITS_DTR_ENABLE_RECV_AND_IRQ)) {
                rs232drv_close(fd);
                alarm_unset(acia_alarm_tx);
                alarm_active_tx = 0;
                fd = -1;
        }
        break;
      case T232_ECTRL:
        if ((ctrl & ACIA_CTRL_BITS_BPS_MASK) == ACIA_CTRL_BITS_BPS_16X_EXT_CLK) {
          ectrl=b;
          set_acia_ticks();
        }
    }
}

/*! \brief read the ACIA register values
  This function is used to read the ACIA values from the computer.
  All side-effects are executed.

  \param addr
    The address of the ACIA register to read

  \return
    The value the register has
*/
BYTE REGPARM1 myacia_read(WORD a)
{
#if 0 /* def DEBUG */
    static BYTE myacia_read_(WORD);
    BYTE b = myacia_read_(a);
    static WORD lasta = 0;
    static BYTE lastb = 0;

    if ((a != lasta) || (b != lastb)) {
        DEBUG_LOG_MESSAGE((acia_log, "read_myacia(%04x) -> %02x", a, b));
    }
    lasta = a; lastb = b;
    return b;
}
static BYTE myacia_read_(WORD a)
{
#endif
    int acia_register_size;

    if (acia_mode==ACIA_MODE_TURBO232)
      acia_register_size=7;
    else
      acia_register_size=3;

    switch(a & acia_register_size) {
      case ACIA_DR:
        status &= ~ ACIA_SR_BITS_RECEIVE_DR_FULL;
        acia_last_read = rxdata;
        return rxdata;
      case ACIA_SR:
        {
            BYTE c = status | (irq ? ACIA_SR_BITS_IRQ : 0);
            acia_set_int(acia_irq, acia_int_num, 0);
            irq = 0;
            acia_last_read = c;
            return c;
        }
      case ACIA_CTRL:
        acia_last_read = ctrl;
        return ctrl;
      case ACIA_CMD:
        acia_last_read = cmd;
        return cmd;
      case T232_NDEF1:
      case T232_NDEF2:
      case T232_NDEF3:
        return 0xff;
      case T232_ECTRL:
        return ectrl 
                     + ((ctrl & ACIA_CTRL_BITS_BPS_MASK) == ACIA_CTRL_BITS_BPS_16X_EXT_CLK) 
                       ? T232_ECTRL_BITS_EXT_ACTIVE 
                       : 0;
    }
    /* should never happen */
    return 0;
}

/*! \brief read the ACIA register values without side effects
  This function reads the ACIA values, so they can be accessed like
  an array of bytes. No side-effects that would be performed if a real
  read access would occur are executed.

  \param addr
    The address of the ACIA register to read

  \return
    The value the register has

  \todo
    Currently unused
*/
BYTE myacia_peek(WORD a)
{
    switch(a & 3) {
      case ACIA_DR:
        return rxdata;
      case ACIA_SR:
        {
            BYTE c = status | (irq ? ACIA_SR_BITS_IRQ : 0);
            return c;
        }
      case ACIA_CTRL:
        return ctrl;
      case ACIA_CMD:
        return cmd;
    }
    return 0;
}

static void int_acia_tx(CLOCK offset, void *data)
{
    DEBUG_VERBOSE_LOG_MESSAGE((acia_log, "int_acia_tx(offset=%ld, myclk=%d", offset, myclk));

    if ((in_tx == ACIA_TX_STATE_DR_WRITTEN) && (fd >= 0)) {
        rs232drv_putc(fd,txdata);
    }

    if (in_tx != ACIA_TX_STATE_NO_TRANSMIT) {
        in_tx--;
    }

    acia_alarm_clk_tx = myclk + acia_ticks;
    alarm_set(acia_alarm_tx, acia_alarm_clk_tx);
    alarm_active_tx = 1;

    status |= ACIA_SR_BITS_TRANSMIT_DR_EMPTY;

    if ( (cmd & ACIA_CMD_BITS_TRANSMITTER_MASK) == ACIA_CMD_BITS_TRANSMITTER_TX_WITH_IRQ ) {
        acia_set_int(acia_irq, acia_int_num, acia_irq);
        irq = 1;
    }
}

static int int_acia_read_data(void)
{
    char received_byte;

    int read_data = 0;

    DEBUG_VERBOSE_LOG_MESSAGE((acia_log, "int_acia_read_data()"));

    do {
        if (fd < 0) {
            break;
        }

        if ( ! rs232drv_getc(fd, &received_byte) ) {
            break;
        }

        DEBUG_LOG_MESSAGE((acia_log, "received byte: %u = '%c'.", 
            (unsigned) received_byte, received_byte));

        /*! \todo: What happens on the real 6551? Is the old value overwritten in
         * case of an overrun, or is it not?
         */
        rxdata = received_byte;

        read_data = 1;

        if ( status & ACIA_SR_BITS_RECEIVE_DR_FULL) {
            status |= ACIA_SR_BITS_OVERRUN_ERROR;
            break;
        }

        status |= ACIA_SR_BITS_RECEIVE_DR_FULL;

    } while (0);

    return read_data;
}

static void int_acia_rx(CLOCK offset, void *data)
{
    CLOCK next_alarm = myclk + acia_ticks;

    DEBUG_VERBOSE_LOG_MESSAGE((acia_log, "int_acia_rx(offset=%ld, myclk=%d", offset, myclk));

    if (int_acia_read_data()) {
        if ( ! (cmd & ACIA_CMD_BITS_IRQ_DISABLED) ) {
            acia_set_int(acia_irq, acia_int_num, acia_irq);
            irq = 1;
        }
    }

    acia_alarm_clk_rx = next_alarm;
    alarm_set(acia_alarm_rx, acia_alarm_clk_rx);
    alarm_active_rx = 1;
}
