/*
 * print.c - Printer interface.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "cmdline.h"
#include "print.h"
#include "resources.h"
#include "types.h"
#include "utils.h"
#include "string.h"



static char *PrinterFile=NULL;

/* The handle to be returned by print_open() is architecture independent
   and a simple int. We save the real file descriptor here */
static FILE *fd[3]={NULL, NULL, NULL};

static int set_printer_file(resource_value_t v)
{
  const char *name = (const char*)v;

  if ((PrinterFile != NULL) && (name != NULL) && (strcmp(name, PrinterFile) == 0))
    return 0;

  string_set(&PrinterFile, name);
  return 0;
}

resource_t resources[] = {
  {"PrinterFile", RES_STRING, (resource_value_t)"viceprnt.out",
    (resource_value_t*)&PrinterFile, set_printer_file},
  {NULL}
};

int print_init_resources(void)
{
  return resources_register(resources);
}

static cmdline_option_t cmdline_options[] =
{
    { "-prfile", SET_RESOURCE, 1, NULL, NULL, "PrinterFile", NULL,
     "<name>", "Specify name of printer dump file (viceprnt.out)" },
    { NULL }
};

int print_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

void print_init(void)
{
}


void print_reset(void)
{
}


int print_open(int device)
{
  switch (device)
  {
    case 0:
      if (PrinterFile == NULL) return -1;
      if (fd[0] == NULL) fd[0] = fopen(PrinterFile, "ab+");
      return 0;
    case 1:
      if (fd[1] == NULL) fd[1] = fopen("LPT:", "ab+");
      return 1;
    default:
      return -1;
  }
}


void print_close(int fi)
{
  if (fd[fi] != NULL) fclose(fd[fi]);
  fd[fi] = NULL;
}


int print_putc(int fi, BYTE b)
{
  if (fd[fi] == NULL) return -1;
  fputc(b, fd[fi]);

  /* BeOS-compatible lineend?? FIXME */
//  if (b==13)
//      fputc(10, fd[fi]);

  return 0;
}


int print_getc(int fi, BYTE *b)
{
  if (fd[fi] == NULL) return -1;
  *b = fgetc(fd[fi]);
  return 0;
}


int print_flush(int fi)
{
  if (fd[fi] == NULL) return -1;
  fflush(fd[fi]);
  return 0;
}
