
#ifndef PAR_FLASHROM_H
#define PAR_FLASHROM_H

#include "types.h"

extern void flashrom_init(void);
extern BYTE flashrom_read(unsigned int addr);
extern void flashrom_write(unsigned int addr,BYTE value);

#endif
