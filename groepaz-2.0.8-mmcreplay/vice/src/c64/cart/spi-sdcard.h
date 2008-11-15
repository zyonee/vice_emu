
#ifndef SPI_SDCARD
#define SPI_SDCARD

#include "types.h"
#include <stdio.h>

void spi_mmc_trigger_mode_write(BYTE value);
BYTE spi_mmc_trigger_mode_read(void);
BYTE spi_mmc_card_inserted(void);
BYTE spi_mmc_busy(void);
BYTE spi_mmc_card_write_enabled(void);
BYTE spi_mmc_card_selected_read(void);
void spi_mmc_card_selected_write(BYTE value);
BYTE spi_mmc_enable_8mhz_read(void);
void spi_mmc_enable_8mhz_write(BYTE value);
BYTE spi_mmc_trigger_mode_read(void);
void spi_mmc_trigger_mode_write(BYTE value);
BYTE spi_mmc_data_read(void);
void spi_mmc_data_write(BYTE value);
FILE *mmc_open_card_image(char *name);
void mmc_close_card_image(void);

#endif
