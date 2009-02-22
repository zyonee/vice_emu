/*
 * mmc64.c - Cartridge handling, MMC64 cart.
 *
 * Written by
 *  Markus Stehr <bastetfurry@ircnet.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "c64_256k.h"
#include "c64cart.h"
#include "c64export.h"
#include "c64io.h"
#include "c64mem.h"
#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "mem.h"
#include "mmc64.h"
#include "plus256k.h"
#include "plus60k.h"
#include "resources.h"
#include "translate.h"
#include "types.h"
#include "util.h"
#include "vicii-phi1.h"
#include "cart/spi-sdcard.h"

#define DEBUG

/*
#define LOG_READ_DF10
#define LOG_READ_DF11
#define LOG_READ_DF12
#define LOG_READ_DF13

#define LOG_WRITE_DF10
#define LOG_WRITE_DF11
#define LOG_WRITE_DF12
#define LOG_WRITE_DF13
*/

#ifdef DEBUG
#define LOG(_x_) log_debug _x_
#else
#define LOG(_x_)
#endif

static const c64export_resource_t export_res = {
    "MMC64", 1, 0
};

/* MMC64 enable */
int mmc64_enabled;

/* MMC64 clockport enable */
int mmc64_clockport_enabled=1;

/* MMC64 clockport base address */
int mmc64_hw_clockport=0xde02;

/* MMC64 bios writable */
static int mmc64_bios_write;

/* Bios file name */
static char *mmc64_bios_filename = NULL;

/* Image file name */
static char *mmc64_image_filename = NULL;

/* Image file */
static FILE *mmc64_image_file;


/* $DF11 bit 7 unlock flag */
static int mmc64_bit7_unlocked=0;

/* Unlocking sequences buffer */
static BYTE mmc64_unlocking[2]={0,0};

/* BIOS changed flag */
static int mmc64_bios_changed=0;

/* flash jumper flag */
static int mmc64_hw_flashjumper;

/* write protect flag */
static int mmc64_hw_writeprotect;

/* Control Bits */
/* $DF11 (R/W): MMC64 control register */
#define MMC_ACTIVE  1	/* bit 7: 0 = MMC64 is active, 1 = MMC64 is completely disabled                   */
#define MMC_SPIMODE 1	/* bit 6: 0 = SPI write trigger mode, 1 = SPI read trigger mode                   */
#define MMC_EXTROM  1	/* bit 5: 0 = allow external rom when BIOS is disabled , 1 = disable external ROM */
#define MMC_FLASH   1	/* bit 4: 0 = normal Operation, 1 = flash mode                                    */
#define MMC_CPORT   1	/* bit 3: 0 = clock port @ $DE00, 1 = clock port @ $DF20                          */
#define MMC_SPEED   1	/* bit 2: 0 = 250khz transfer, 1 = 8mhz transfer                                  */
#define MMC_CARDSEL 1   /* bit 1: 0 = card selected, 1 = card not selected                                */
#define MMC_BIOSSEL 1 	/* bit 0: 0 = MMC64 BIOS active, 1 = external ROM active                          */


/* Variables of the various control bits */
static BYTE mmc64_active;
static BYTE mmc64_spi_mode;
static BYTE mmc64_extrom;
static BYTE mmc64_flashmode;
static BYTE mmc64_cport;
static BYTE mmc64_speed;
static BYTE mmc64_cardsel;
static BYTE mmc64_biossel;

/* Status Bits */
/* $DF12 (R): MMC64 status register */
#define MMC_FLASHJMP	0x20 /* bit 5: 0 = flash jumper not set, 1 = flash jumper set  */
#define MMC_WRITEPROT	0x10 /* bit 4: 0 = card write enabled, 1 = card write disabled */
#define MMC_CARDPRS	0x08 /* bit 3: 0 = card inserted, 1 = no card inserted         */
#define MMC_EXTEXROM	0x04 /* bit 2: external EXROM line                             */
#define MMC_EXTGAME	0x02 /* bit 1: external GAME line                              */
#define MMC_SPISTAT	0x01 /* bit 0: 0 = SPI ready, 1 = SPI busy                     
*/

/* Variables of the various status bits */
static BYTE mmc64_flashjumper;
static BYTE mmc64_extexrom;
static BYTE mmc64_extgame;

static int mmc64_revision;
static int mmc64_sd_type=0;
static BYTE mmc64_image_file_readonly=0;

static log_t mmc64_log = LOG_ERR;

static BYTE mmc64_bios[0x2002];
static int mmc64_bios_offset=0;

static int mmc64_activate(void);
static int mmc64_deactivate(void);

void mmc64_reset(void)
{
  mmc64_active=0;
  mmc64_spi_mode=0;
  mmc64_extrom=0;
  mmc64_flashmode=0;
  mmc64_cport=0;
  mmc64_speed=0;
  mmc64_cardsel=0;
  mmc64_biossel=0;

  mmc64_extexrom=1; //0x04;
  mmc64_extgame=1; //0x02;
  mmc64_clockport_enabled=1;

  if (mmc64_enabled)
  {
    export.exrom = 1;
    mem_pla_config_changed();
  }
}

static int set_mmc64_enabled(int val, void *param)
{
  if (!val)
  {
    if (mmc64_enabled)
    {
      if (mmc64_deactivate() < 0)
      {
        return -1;
      }
      machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    }
    c64export_remove(&export_res);
    mmc64_enabled = 0;
    export.exrom = 0;
    mem_pla_config_changed();
    return 0;
  }
  else
  {
    if (c64export_query(&export_res) >= 0)
    {
      if (!mmc64_enabled)
      {
        if (mmc64_activate() < 0)
        {
          return -1;
        }
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
      }

      if (c64export_add(&export_res) < 0)
        return -1;

      mmc64_enabled = 1;
      export.exrom = 1;
      mem_pla_config_changed();
      return 0;
    }
    else
    {
      return -1;
    }
  }
}

static int set_mmc64_readonly(int val, void *param)
{
  if (!mmc64_image_file_readonly)
  {
    mmc64_hw_writeprotect = val;
    	if(!(*mmc64_image_filename)==0)
	{
           return mmc_open_card_image(mmc64_image_filename, mmc64_hw_writeprotect^1);
	}
        return 0;
  }
  else
  {
    mmc64_hw_writeprotect=1;
  }

    	if(!(*mmc64_image_filename)==0)
	{
           return mmc_open_card_image(mmc64_image_filename, mmc64_hw_writeprotect^1);
	}
  
  return -1;
}

static int set_mmc64_flashjumper(int val, void *param)
{
  mmc64_hw_flashjumper = val;
  mmc64_flashjumper = val*MMC_FLASHJMP;
  return 0;
}

static int set_mmc64_revision(int val, void *param)
{
  mmc64_revision = val;
  return 0;
}

static int set_mmc64_sd_type(int val, void *param)
{
  mmc64_sd_type = val;
    mmc_set_card_type (val);
  return 0;
}

static int set_mmc64_bios_write(int val, void *param)
{
  mmc64_bios_write = val;
  return 0;
}

static int set_mmc64_bios_filename(const char *name, void *param)
{
  if (mmc64_bios_filename != NULL && name != NULL && strcmp(name, mmc64_bios_filename) == 0)
    return 0;

  if (name != NULL && *name != '\0') {
      if (util_check_filename_access(name) < 0)
          return -1;
  }

  if (mmc64_enabled)
  {
    mmc64_deactivate();
    util_string_set(&mmc64_bios_filename, name);
    mmc64_activate();
    machine_trigger_reset(MACHINE_RESET_MODE_HARD);
  }
  else
  {
    util_string_set(&mmc64_bios_filename, name);
  }

  return 0;
}

static int set_mmc64_image_filename(const char *name, void *param)
{
  if (mmc64_image_filename != NULL && name != NULL && strcmp(name, mmc64_image_filename) == 0)
    return 0;

  if (name != NULL && *name != '\0') {
      if (util_check_filename_access(name) < 0)
          return -1;
  }

  if (mmc64_enabled)
  {
    mmc64_deactivate();
    util_string_set(&mmc64_image_filename, name);
    mmc64_activate();
  }
  else
  {
    util_string_set(&mmc64_image_filename, name);
  }

  return 0;
}

void mmc64_init_card_config(void)
{
  mmc64_active=0;
  mmc64_spi_mode=0;
  mmc64_extrom=0;
  mmc64_flashmode=0;
  mmc64_cport=0;
  mmc64_speed=0;
  mmc64_cardsel=0;
  mmc64_biossel=0;

  /* for now external exrom and game are constantly   *
    * high until the pass-through port support is made */
  mmc64_extexrom=1; //0x04;
  mmc64_extgame=1; //0x02;

  if (mmc64_enabled)
  {
    export.exrom = 1;
    mem_pla_config_changed();
  }
}


void mmc64_clockport_enable_store(BYTE value)
{
  if (value&1)
  {
    mmc64_clockport_enabled=1;
  }
  else
  {
    mmc64_clockport_enabled=0;
  }
}

void REGPARM2 mmc64_io2_store(WORD addr, BYTE value)
{
  switch(addr)
  {
    case 0xdf10:    /* MMC64 SPI transfer register */
            /*
             * $DF10: MMC SPI transfer register
             *
             * byte written is sent to the card
             */
            if (mmc64_active==0)
            {
#ifdef LOG_WRITE_DF10
                LOG (("MMC64: IO2 ST %04x %02x", addr, value));
#endif
                spi_mmc_data_write (value);
                return;
            }
      
      break;



        case 0xdf11:
            /*
             * $DF11: MMC control register
             *        ------------------------
             *        bit 0:  0 = MMC BIOS enabled, 1 = MMC BIOS disabled                   (R/W)
             *        bit 1:  0 = card selected, 1 = card not selected                      (R/W)
             *        bit 2:  0 = 250khz transfer, 1 = 8mhz transfer                        (R/W)
             *        bit 3:  0 = clock port @ $DE00, 1 = clock port @ $DF20                (R/W)
             *        bit 4:  0 = normal Operation, 1 = flash mode                          (R/W)  (*)
             *        bit 5:  0 = allow external rom when BIOS is disabled , 1 = disable    (R/W)
             *        bit 6:  0 = SPI write trigger mode, 1 = SPI read trigger mode         (R/W)
             *        bit 7:  0 = MMC64 is active, 1 = MMC64 is completely disabled         (R/W)  (**)
             *
             * (*) bit can only be programmed when flash jumper is set
             * (**) bit can only be modified after unlocking
             */	
            if (mmc64_active==0)
            {
                mmc64_biossel = (value) & 1; /* bit 0 */
                mmc64_extrom = (value >> 5) & 1;      /* bit 5 */

#ifdef LOG_WRITE_DF11
                LOG (("MMC64: IO2 ST %04x %02x mmc64_biossel %x mmc64_extrom %x", addr, value, mmc64_biossel, mmc64_extrom));
#endif

                spi_mmc_card_selected_write (((value >> 1) ^ 1) & 1);   /* bit 1 */
                spi_mmc_enable_8mhz_write (((value >> 2)) & 1); /* bit 2 */
		mmc64_cport=(((value >> 3)) & 1); /* bit 3 */
		mmc64_flashmode=(((value >> 4)) & 1); /* bit 4 */
                spi_mmc_trigger_mode_write (((value >> 6)) & 1);        /* bit 6 */

		//mmc64_active=(((value >> 7)) & 1); /* bit 4 */
		
                /* bit 7 always 0 */
                if (mmc64_active)
                {
                    log_message(mmc64_log,"disabling MMC64");
                }

		if(mmc64_active)
		{
		    export.exrom = 0;
		    mem_pla_config_changed();
		}
		else
		{
			if (mmc64_biossel)   /* this controls the mapping of the MMC64 bios */
			{
			  export.exrom = 0;
			  mem_pla_config_changed();
			}
			else
			{
			  export.exrom = 1;
			  mem_pla_config_changed();
			}
		}

                if (mmc64_cport)
                {
                          mmc64_hw_clockport = 0xdf22;
                }
                else
                {
                       mmc64_hw_clockport = 0xde02;
                }
		
                return;
            }
            break;


#if 0

      
    case 0xdf11:    /* MMC64 control register */
      if (mmc64_active==0)    /* if the MMC64 has been disabled in software this input is ignored */
      {
        if (mmc64_bit7_unlocked==1 && mmc64_active==0)    /* this bit can only be changed after unlocking it */
        {
          if (value & MMC_ACTIVE)
          {
            log_message(mmc64_log,"disabling MMC64");
            mmc64_active = MMC_ACTIVE;
            export.exrom = 0;
            mem_pla_config_changed();
          }
        }
        mmc64_bit7_unlocked=0;
        if (value & MMC_SPIMODE)
        {
          mmc64_spi_mode = MMC_SPIMODE;
        }
        else
        {
          mmc64_spi_mode = 0;
        }
        if (value & MMC_EXTROM)
        {
          mmc64_extrom = MMC_EXTROM;
        }
        else
        {
          mmc64_extrom = 0;
        }
        if (mmc64_flashjumper)    /* this bit can only be changed if the flashjumper is on */
        {
          if (value & MMC_FLASH)
          {
            mmc64_flashmode = MMC_FLASH;
          }
          else
          {
            mmc64_flashmode = 0;
          }
        }
        if (value & MMC_CPORT)
        {
          mmc64_cport = MMC_CPORT;
          mmc64_hw_clockport = 0xdf22;
        }
        else
        {
          mmc64_cport = 0;
          mmc64_hw_clockport = 0xde02;
        }
        if (value & MMC_SPEED)
        {
          mmc64_speed = MMC_SPEED;
        }
        else
        {
          mmc64_speed = 0;
        }
        if (value & MMC_CARDSEL)
        {
          mmc64_cardsel = MMC_CARDSEL;
        }
        else
        {
          mmc64_cardsel = 0;
        }
        if (value & MMC_BIOSSEL)   /* this controls the mapping of the MMC64 bios */
        {
          mmc64_biossel = MMC_BIOSSEL;
          export.exrom = 0;
          mem_pla_config_changed();
        }
        else
        {
          mmc64_biossel = 0;
          export.exrom = 1;
          mem_pla_config_changed();
        }
      }
      break;
#endif
						  
    case 0xdf12:  /* MMC64 status register, read only */
      break;
    case 0xdf13:  /* MMC64 identification register, also used for unlocking sequences */
      mmc64_unlocking[0] = mmc64_unlocking[1];
      mmc64_unlocking[1] = value;
      if (mmc64_unlocking[0]==0x55 && mmc64_unlocking[1]==0xaa)
      {
        log_message(mmc64_log,"bit 7 unlocked");
        mmc64_bit7_unlocked=1;    /* unlock bit 7 of $DF11 */
      }
      if (mmc64_unlocking[0]==0x0a && mmc64_unlocking[1]==0x1c)
      {
        mmc64_active = 0;
        export.exrom = 1;
        mem_pla_config_changed();   /* re-enable the MMC64 */
      }
      break;
    default:      /* Not for us */
      return;
  }
}

BYTE REGPARM1 mmc64_io2_read(WORD addr)
{
  BYTE value;

  switch(addr)
  {

        case 0xdf10:
            /*
             * $DF10: MMC SPI transfer register
             *
             * response from the card is read here
             */
//            if (enable_mmc_regs)
            {
                io_source = IO_SOURCE_MMC64;
                value = spi_mmc_data_read ();
#ifdef LOG_READ_DF10
                LOG (("MMC64: IO2 RD %04x %02x", addr, value));
#endif
                return value;
            }
            break;
#if 0
    case 0xdf11:    /* MMC64 control register */
      io_source=IO_SOURCE_MMC64;
      return (mmc64_biossel + mmc64_cardsel + mmc64_speed + mmc64_cport + mmc64_flashmode + mmc64_extrom + mmc64_spi_mode + mmc64_active);
      break;
#endif
        case 0xdf11:
            /*
             * $DF11: MMC control register
             *        ------------------------
             *        bit 0:  0 = MMC BIOS enabled, 1 = MMC BIOS disabled                   (R/W)
             *        bit 1:  0 = card selected, 1 = card not selected                      (R/W)
             *        bit 2:  0 = 250khz transfer, 1 = 8mhz transfer                        (R/W)
             *        bit 3:  0 = clock port @ $DE00, 1 = clock port @ $DF20                (R/W)
             *        bit 4:  0 = normal Operation, 1 = flash mode                          (R/W)  (*)
             *        bit 5:  0 = allow external rom when BIOS is disabled , 1 = disable    (R/W)
             *        bit 6:  0 = SPI write trigger mode, 1 = SPI read trigger mode         (R/W)
             *        bit 7:  0 = MMC64 is active, 1 = MMC64 is completely disabled         (R/W)  (**)
             *
             * (*) bit can only be programmed when flash jumper is set
             * (**) bit can only be modified after unlocking
             */
//            if (enable_mmc_regs)
            {
                io_source = IO_SOURCE_MMC64;
                value = mmc64_biossel;       /* bit 0 */
                value |= (spi_mmc_card_selected_read () << 1);  /* bit 1 */
                value |= (spi_mmc_enable_8mhz_read () << 2);    /* bit 2 */
                /* bit 3,4 always 0 */
                value|=mmc64_cport<<3;
                value|=mmc64_flashmode<<4;
                value |= (mmc64_extrom << 5); /* bit 5 */
                value |= (spi_mmc_trigger_mode_read () << 6);   /* bit 6 */
		
		//value|=mmc64_active<<7;
                /* bit 7 always 0 */
#ifdef LOG_READ_DF11
                LOG (("MMC64: IO2 RD %04x %02x mmc64_biossel %x mmc64_extrom %x", addr, value, mmc64_biossel, mmc64_extrom));
#endif
                return value;
            }
            break;
#if 0
    case 0xdf12:    /* MMC64 status register */
      io_source=IO_SOURCE_MMC64;
      return (mmc64_flashjumper + mmc64_writeprotect + mmc64_cardpresent + mmc64_extexrom + mmc64_extgame + mmc64_spistatus);
      break;
#endif

        case 0xdf12:
            /*
             * $DF12: MMC status register
             *        -----------------------
             *        bit 0:  0 = SPI ready, 1 = SPI busy                       (R)
             *        bit 1:  feedback of $DE00 bit 0 (GAME)                    (R)
             *        bit 2:  feedback of $DE00 bit 1 (EXROM)                   (R)
             *        bit 3:  0 = card inserted, 1 = no card inserted           (R)
             *        bit 4:  0 = card write enabled, 1 = card write disabled   (R)
             *        bit 5:  0 = flash jumper not set, 1 = flash jumper set    (R)
             *        bit 6:  0
             *        bit 7:  0
             */
//            if (enable_mmc_regs)
            {
                BYTE    value = 0;
                io_source = IO_SOURCE_MMC64;
                value=mmc64_flashjumper<<5;    /* bit 5 */
                value |= (spi_mmc_busy ());     /* bit 0 */
                value |= (mmc64_extexrom << 1);    /* bit 1 */
                value |= (mmc64_extgame) << 2;       /* bit 2 */
                value |= (spi_mmc_card_inserted () ^ 1) << 3;   /* bit 3 */
                value |= (spi_mmc_card_write_enabled () ^ 1) << 4;      /* bit 4 */

                /* bit 6,7 not readable */
#ifdef LOG_READ_DF12
                LOG (("MMC64: IO2 RD %04x %02x mmc64_extgame %x mmc64_extexrom %x", addr, value, mmc64_extgame, (mmc64_extexrom ^ 1)));
#endif
                return value;
            }
            break;


      
            /*
             * $DF13 (R/W): MMC64 identification register
             *              -----------------------------
             * (R) #$64 when bit 1 of $DF11 is 0
             *     #$01 when bit 1 of $DF11 is 1 and REV A hardware is used
             *     #$02 when bit 1 of $DF11 is 1 and REV B hardware is used
             */
      
    case 0xdf13:    /* MMC64 identification register */
      io_source=IO_SOURCE_MMC64;
      if (!mmc64_cardsel)
      {
        return 0x64;
      }
      else
      {
        if (mmc64_revision)
        {
          return 2;
        }
        else
        {
          return 1;
        }
      }
      break;
  }
  return vicii_read_phi1();
}

BYTE REGPARM1 mmc64_roml_read(WORD addr)
{
  if (!mmc64_active && !mmc64_biossel)
  {
    return mmc64_bios[(addr&0x1fff)+mmc64_bios_offset];
  }

  if (plus60k_enabled)
    return plus60k_ram_read(addr);

  if (plus256k_enabled)
    return plus256k_ram_high_read(addr);

  if (c64_256k_enabled)
    return c64_256k_ram_segment2_read(addr);

  return mem_ram[addr];
}

void REGPARM2 mmc64_roml_store(WORD addr, BYTE byte)
{
  if (!mmc64_active && !mmc64_biossel && mmc64_flashjumper && mmc64_flashmode)
  {
    if (mmc64_bios[(addr&0x1fff)+mmc64_bios_offset]!=byte)
    {
      mmc64_bios[(addr&0x1fff)+mmc64_bios_offset]=byte;
      mmc64_bios_changed=1;
      return;
    }
  }

  if (plus60k_enabled)
  {
    plus60k_ram_store(addr, byte);
    return;
  }

  if (plus256k_enabled)
  {
    plus256k_ram_high_store(addr, byte);
    return;
  }

  if (c64_256k_enabled)
  {
    c64_256k_ram_segment2_store(addr, byte);
    return;
  }

  mem_ram[addr]=byte;
}

static const resource_string_t resources_string[] = {
  { "MMC64BIOSfilename", "", RES_EVENT_NO, NULL,
    &mmc64_bios_filename, set_mmc64_bios_filename, NULL },
  { "MMC64imagefilename", "", RES_EVENT_NO, NULL,
    &mmc64_image_filename, set_mmc64_image_filename, NULL },
  { NULL }
};

static const resource_int_t resources_int[] = {
  { "MMC64", 0, RES_EVENT_STRICT, (resource_value_t)0,
    &mmc64_enabled, set_mmc64_enabled, NULL },
  { "MMC64_RO", 0, RES_EVENT_NO, NULL,
    &mmc64_hw_writeprotect, set_mmc64_readonly, NULL },
  { "MMC64_flashjumper", 0, RES_EVENT_NO, NULL,
    &mmc64_hw_flashjumper, set_mmc64_flashjumper, NULL },
  { "MMC64_revision", 0, RES_EVENT_NO, NULL,
    &mmc64_revision, set_mmc64_revision, NULL },
  { "MMC64_bios_write", 0, RES_EVENT_NO, NULL,
    &mmc64_bios_write, set_mmc64_bios_write, NULL },
  { "MMC64_sd_type", 0, RES_EVENT_NO, NULL,
    &mmc64_sd_type, set_mmc64_sd_type, NULL },
  { NULL }
};

int mmc64_resources_init(void)
{
  if (resources_register_string(resources_string) < 0)
    return -1;

  return resources_register_int(resources_int);
}

void mmc64_resources_shutdown(void)
{
  lib_free(mmc64_bios_filename);
  lib_free(mmc64_image_filename);
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
  { "-mmc64", SET_RESOURCE, 0,
    NULL, NULL, "MMC64", (resource_value_t)1,
    USE_PARAM_STRING, USE_DESCRIPTION_ID,
    IDCLS_UNUSED, IDCLS_ENABLE_MMC64,
    NULL, NULL },
  { "+mmc64", SET_RESOURCE, 0,
    NULL, NULL, "MMC64", (resource_value_t)0,
    USE_PARAM_STRING, USE_DESCRIPTION_ID,
    IDCLS_UNUSED, IDCLS_DISABLE_MMC64,
    NULL, NULL },
  { "-mmc64bios", SET_RESOURCE, 1,
    NULL, NULL, "MMC64BIOSfilename", NULL,
    USE_PARAM_ID, USE_DESCRIPTION_ID,
    IDCLS_P_NAME, IDCLS_SPECIFY_MMC64_BIOS_NAME,
    NULL, NULL },
  { "-mmc64image", SET_RESOURCE, 1,
    NULL, NULL, "MMC64imagefilename", NULL,
    USE_PARAM_ID, USE_DESCRIPTION_ID,
    IDCLS_P_NAME, IDCLS_SPECIFY_MMC64_IMAGE_NAME,
    NULL, NULL },
  { "-mmc64readonly", SET_RESOURCE, 0,
    NULL, NULL, "MMC64_RO", (resource_value_t)1,
    USE_PARAM_STRING, USE_DESCRIPTION_ID,
    IDCLS_UNUSED, IDCLS_MMC64_READONLY,
    NULL, NULL },
  { "-mmc64readwrite", SET_RESOURCE, 0,
    NULL, NULL, "MMC64_RO", (resource_value_t)0,
    USE_PARAM_STRING, USE_DESCRIPTION_ID,
    IDCLS_UNUSED, IDCLS_MMC64_READWRITE,
    NULL, NULL },
  { "-mmc64bioswrite", SET_RESOURCE, 0,
    NULL, NULL, "MMC64_bios_write", (resource_value_t)1,
    USE_PARAM_STRING, USE_DESCRIPTION_ID,
    IDCLS_UNUSED, IDCLS_MMC64_BIOS_WRITE,
    NULL, NULL },
  { NULL }
};

int mmc64_cmdline_options_init(void)
{
  return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

void mmc64_init(void)
{
  mmc64_log = log_open("MMC64");
//  mmc64_log = stderr;
}

static int mmc64_activate(void)
{
  FILE *bios_file=NULL;
  int amount_read=0;

  if (mmc64_bios_filename==NULL)
    return -1;

  bios_file=fopen(mmc64_bios_filename,"rb");

  if (bios_file==NULL)
    return -1;

  amount_read=fread(&mmc64_bios,1,0x2002,bios_file);

  fclose(bios_file);

  if (amount_read!=0x2000 && amount_read!=0x2002)
    return -1;

  if (amount_read==0x2002)
    mmc64_bios_offset=2;
  else
    mmc64_bios_offset=0;

  mmc64_bios_changed=0;

  mmc_open_card_image(mmc64_image_filename, mmc64_hw_writeprotect^1);
  mmc64_reset();
  return 0;

}

static int mmc64_deactivate(void)
{
  FILE *bios_file=NULL;

  mmc_close_card_image();
 
  if (mmc64_bios_changed && mmc64_bios_write)
  {
    bios_file=fopen(mmc64_bios_filename,"wb");

    if (bios_file==NULL)
      return 0;

    fwrite(&mmc64_bios,1,0x2000+mmc64_bios_offset,bios_file);
    fclose(bios_file);
    mmc64_bios_changed=0;
  }
  return 0;
}

void mmc64_shutdown(void)
{
  mmc64_deactivate();
}
