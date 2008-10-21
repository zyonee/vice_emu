/*
 * rs232dev.c - RS232 emulation.
 *
 * Written by
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

/*
 * The RS232 emulation captures the bytes sent to the RS232 interfaces
 * available (currently ACIA 6551, std C64 and Daniel Dallmanns fast RS232
 * with 9600 Baud).
 *
 * I/O is done to a socket.  If the socket isnt connected, no data
 * is read and written data is discarded.
 */

#undef        DEBUG
/* #define DEBUG */

#include "vice.h"

#include <errno.h>
#include <string.h>
#include <winsock.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include "log.h"
#include "rs232.h"
#include "types.h"
#include "util.h"

#ifdef DEBUG
# define DEBUG_LOG_MESSAGE(_xxx) log_message _xxx
#else
# define DEBUG_LOG_MESSAGE(_xxx)
#endif

/* ------------------------------------------------------------------------- */

int rs232dev_resources_init(void)
{
    return 0;
}

void rs232dev_resources_shutdown(void)
{
}

int rs232dev_cmdline_options_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

typedef struct rs232 {
    int inuse;
    HANDLE fd;
    char *file;
} rs232dev_t;

static rs232dev_t fds[RS232_NUM_DEVICES];

static log_t rs232dev_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

void rs232dev_close(int fd);

/* initializes all RS232 stuff */
void rs232dev_init(void)
{
    rs232dev_log = log_open("RS232");
}

/* reset RS232 stuff */
void rs232dev_reset(void)
{
    int i;

    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        if (!fds[i].inuse) {
            rs232dev_close(i);
        }
    }
}

/* opens a rs232 window, returns handle to give to functions below. */
int rs232dev_open(int device)
{
    HANDLE serial_port;
    int ret = -1;
    int i;

    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        if (!fds[i].inuse)
            break;
    }
    if (i >= RS232_NUM_DEVICES) {
        log_error(rs232dev_log, "No more devices available.");
        return -1;
    }

    DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev_open(device=%d).", device));

    do {
        DCB dcb;

        serial_port = CreateFile(rs232_devfile[device], GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, OPEN_EXISTING, 0, NULL);


        if (serial_port == INVALID_HANDLE_VALUE) 
        {
            DEBUG_LOG_MESSAGE(("CreateFile '%s' failed: %d.\n",
                rs232_devfile[device], GetLastError()));
            break;
        }

        memset(&dcb, 0, sizeof dcb);
        dcb.DCBlength = sizeof dcb;

        if ( ! GetCommState(serial_port, &dcb) ) {
            DEBUG_LOG_MESSAGE(("GetCommState '%s' failed: %d.\n",
                rs232_devfile[device], GetLastError()));
            break;
        }

        /*! \todo this is only for testing purposes! Use BuildCommDCB() instead! */

        dcb.BaudRate = CBR_57600;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;

        /*! \todo get st out of rs232_devfile, and activate this code.

        { char st[] = "";
        if ( BuildCommDCB(st, &dcb) == 0) {
            DEBUG_LOG_MESSAGE(("BuildCommDCB '%s' for device '%s' failed: %d.\n",
                st, rs232_devfile[device], GetLastError()));
            break;
        }
        }
        */

        if ( ! SetCommState(serial_port, &dcb) ) {
            DEBUG_LOG_MESSAGE(("SetCommState '%s' failed: %d.\n",
                rs232_devfile[device], GetLastError()));
            break;
        }

        fds[i].inuse = 1;
        fds[i].fd = serial_port;
        fds[i].file = rs232_devfile[device];
        ret = i;
        serial_port = INVALID_HANDLE_VALUE;

    } while (0);

    if (serial_port != INVALID_HANDLE_VALUE) {
        CloseHandle(serial_port);
    }

    return ret;
}

/* closes the rs232 window again */
void rs232dev_close(int fd)
{
    DEBUG_LOG_MESSAGE((rs232dev_log, "close(fd=%d).", fd));

    if (fd < 0 || fd >= RS232_NUM_DEVICES) {
        log_error(rs232dev_log, "Attempt to close invalid fd %d.", fd);
        return;
    }
    if (!fds[fd].inuse) {
        log_error(rs232dev_log, "Attempt to close non-open fd %d.", fd);
        return;
    }

    CloseHandle(fds[fd].fd);
    fds[fd].inuse = 0;
}

/* sends a byte to the RS232 line */
int rs232dev_putc(int fd, BYTE b)
{
    DWORD number_of_bytes = 1;

    DEBUG_LOG_MESSAGE((rs232dev_log, "Output `%c'.", b));

    if ( WriteFile(fds[fd].fd, &b, number_of_bytes, &number_of_bytes, NULL) == 0)
        return -1;

    if (number_of_bytes != 1)
        return -1;

    return 0;
}

/* gets a byte to the RS232 line, returns !=0 if byte received, byte in *b. */
int rs232dev_getc(int fd, BYTE * b)
{
    DWORD number_of_bytes = 1;

    if ( ReadFile(fds[fd].fd, &b, number_of_bytes, &number_of_bytes, NULL) == 0)
        return -1;

    if (number_of_bytes != 1)
        return -1;

    return 0;
}
