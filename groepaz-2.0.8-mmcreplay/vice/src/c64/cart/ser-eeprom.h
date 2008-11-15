
#ifndef SER_EEPROM
#define SER_EEPROM

#include "types.h"
#include <stdio.h>

void eeprom_data_readadvance(void);
BYTE eeprom_data_readbyte(void);
BYTE eeprom_data_readbit(void);
BYTE eeprom_data_read(void);
void eeprom_cmd_reset(void);
void eeprom_cmd_write(BYTE value);
void eeprom_seq_reset(void);
void eeprom_seq_write(BYTE value);
int  eeprom_execute_command(int eeprom_mode);
void eeprom_port_write(int clk,int data,int ddr,int status);
FILE *eeprom_open_image(char *name);
void eeprom_close_image(void);

#endif
