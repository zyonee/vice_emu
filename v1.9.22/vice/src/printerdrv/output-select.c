/*
 * output-select.c - Select an output driver.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "cmdline.h"
#include "output-select.h"
#include "resources.h"
#include "types.h"
#include "utils.h"


struct output_select_list_s {
    output_select_t output_select;
    struct output_select_list_s *next;
};
typedef struct output_select_list_s output_select_list_t;


/* Names of currently used printer driver.  To be removed.  */
static char *output_device[] = { NULL, NULL, NULL };

/* Currently used output device.  */
static output_select_t output_select[3];

/* Pointer to registered printer driver.  */
static output_select_list_t *output_select_list = NULL;


static int set_output_device(resource_value_t v, void *param)
{
    const char *name = (const char *)v;
    output_select_list_t *list;

    list = output_select_list;

    if (list == NULL)
        return -1;

    do {
        if (!strcmp(list->output_select.output_name, name)) {
            util_string_set(&output_device[(int)param], name);
            memcpy(&output_select[(int)param], &(list->output_select),
                   sizeof(output_select_t));
            return 0;
        }
        list = list->next;
    } while (list != NULL);

    return -1;
}

static resource_t resources[] = {
    { "Printer4Output", RES_STRING, (resource_value_t)"text",
      (resource_value_t *)&output_device[0], set_output_device, (void *)0 },
    { "Printer5Output", RES_STRING, (resource_value_t)"text",
      (resource_value_t *)&output_device[1], set_output_device, (void *)1 },
    { "PrinterUserportOutput", RES_STRING, (resource_value_t)"text",
      (resource_value_t *)&output_device[2], set_output_device, (void *)2 },
    { NULL }
};

int output_select_init_resources(void)
{
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] =
{
    { "-pr4output", SET_RESOURCE, 1, NULL, NULL, "Printer4Output", NULL,
      "<name>", "Specify name of output device for device #4" },
    { "-pr5output", SET_RESOURCE, 1, NULL, NULL, "Printer5Output", NULL,
      "<name>", "Specify name of output device for device #5" },
    { "-pruseroutput", SET_RESOURCE, 1, NULL, NULL,
      "PrinterUserportOutput", NULL,
      "<name>", "Specify name of output device for the userport printer" },
    { NULL }
};

int output_select_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}


void output_select_init(void)
{

}

/* ------------------------------------------------------------------------- */

void output_select_register(output_select_t *output_select)
{
    output_select_list_t *list, *prev;

    prev = output_select_list;
    while (prev != NULL && prev->next != NULL)
        prev = prev->next;

    list = (output_select_list_t *)xmalloc(sizeof(output_select_list_t));
    memcpy(&(list->output_select), output_select, sizeof(output_select_t));
    list->next = NULL;

    if (output_select_list != NULL)
        prev->next = list;
    else
        output_select_list = list;
}

/* ------------------------------------------------------------------------- */

int output_select_open(unsigned int prnr,
                       struct output_parameter_s *output_parameter)
{
    return output_select[prnr].output_open(prnr, output_parameter);
}

void output_select_close(unsigned int prnr)
{
    output_select[prnr].output_close(prnr);
}

int output_select_putc(unsigned int prnr, BYTE b)
{
    return output_select[prnr].output_putc(prnr, b);
}

int output_select_getc(unsigned int prnr, BYTE *b)
{
    return output_select[prnr].output_getc(prnr, b);
}

int output_select_flush(unsigned int prnr)
{
    return output_select[prnr].output_flush(prnr);
}

