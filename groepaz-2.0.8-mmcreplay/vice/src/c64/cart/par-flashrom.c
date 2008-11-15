
#include "par-flashrom.h"
#include "types.h"
#include "util.h"
#include "c64cart.h"

#define DEBUG

#ifdef DEBUG
#define LOG(_x_) log_debug _x_
#else
#define LOG(_x_)
#endif

static unsigned int flashrom_unlocked = 0;
static unsigned int flashrom_status = 0;

void flashrom_init (void)
{
    flashrom_unlocked = 0;
    flashrom_status = 0;
}

BYTE flashrom_read (unsigned int addr)
{
    if (flashrom_unlocked)
    {
        switch (addr & 0xffff)
        {
            case 0x0000:
                LOG (("FLASHROM read status %02x", flashrom_status));
                return flashrom_status;
                break;
        }
    }
    return roml_banks[addr];
}

static int flash_unlock_count = 0;

void flashrom_write (unsigned int addr, BYTE value)
{
    if (flashrom_unlocked)
    {
        LOG (("FLASHROM write %04x:%02x", addr, value));
        switch (value)
        {
            case 0x30:
                LOG (("FLASHROM CMD30 erase bank %d", addr >> 16));
                flashrom_status = 0xff;
                break;
        }
    }
    else
    {
/* LOG(("FLASHROM (locked) write %04x:%02x",addr,value)); */
        switch (flash_unlock_count)
        {
            case 0:
                if ((addr == 0x1555) & (value == 0xaa))
                {
                    flash_unlock_count++;
                }
                else
                {
                    flash_unlock_count = 0;
                }
                break;
            case 1:
                if ((addr == 0x0aaa) & (value == 0x55))
                {
                    flash_unlock_count++;
                }
                else
                {
                    flash_unlock_count = 0;
                }
                break;
            case 2:
                if ((addr == 0x1555) & (value == 0x80))
                {
                    flash_unlock_count++;
                }
                else
                {
                    flash_unlock_count = 0;
                }
                break;
            case 3:
                if ((addr == 0x1555) & (value == 0xaa))
                {
                    flash_unlock_count++;
                }
                else
                {
                    flash_unlock_count = 0;
                }
                break;
            case 4:
                if ((addr == 0x0aaa) & (value == 0x55))
                {
                    flash_unlock_count++;
                }
                else
                {
                    flash_unlock_count = 0;
                }
                break;
        }
        if (flash_unlock_count == 5)
        {
            LOG (("FLASHROM unlocked"));
            flashrom_unlocked = 1;
        }

    }
}
