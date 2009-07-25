/* traceserver.c - Tracer interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "monitor.h"
#include "resources.h"
#include "socket.h"
#include "tracer.h"
#include "traceserver.h"
#include "translate.h"
#include "uiapi.h"
#include "util.h"

#ifdef USE_TRACER
#ifdef HAVE_NETWORK

static vice_network_socket_t * listen_socket = NULL;
static vice_network_socket_t * connected_socket = NULL;

/* resources */
static char * server_address_string = NULL;
static int trace_server_enabled = 0;
static int trace_server_buffer_size = 0;
static int trace_server_send_flush = 0;

/* internal buffer state */
static BYTE * buffer = NULL;
static int    buffer_free = 0;
static int    buffer_pos = 0;

static int    flush_tick = 0;

/* forward declare */
static void transmit_buffer(void);
static int trace_server_transmit(const char * buffer, size_t buffer_length);
static int trace_server_data_available(void);

extern CLOCK maincpu_clk;

/* ----- Buffer Handling ------------------------------------------------- */

/* 
    Packet Layout (total 8 bytes):
        +0:     'VICE'
        +4:     <DWORD> size
        +8:     ... data ... (size bytes)
        
    Message Layout (total 12 bytes):
        +0:     <DWORD> message id
        +4:     <DWORD> size
        +8:     <DWORD> clock tick
        +12:    ... data ... (size bytes)
*/

#define PACKET_HEADER_SIZE  8
#define MESSAGE_HEADER_SIZE 12
#define DEFAULT_BUFFER_SIZE (65 * 1024)
#define DEFAULT_SEND_FLUSH  50

static void free_buffer(void)
{
    /* send last buffer first */
    if(buffer_pos > 0) {
        transmit_buffer();
    }
    
    lib_free(buffer);
    buffer = NULL;
}

static void alloc_buffer(void)
{
    /* free old buffer first */
    if(buffer != NULL) {
        free_buffer();
    }
    
    /* create new buffer */
    buffer = (BYTE *)lib_malloc(trace_server_buffer_size);
    buffer_free = trace_server_buffer_size - PACKET_HEADER_SIZE;
    buffer_pos = PACKET_HEADER_SIZE;
    
    /* prepare packet header */
    buffer[0] = 'V';
    buffer[1] = 'I';
    buffer[2] = 'C';
    buffer[3] = 'E';
    
    log_message(LOG_DEFAULT, "trace server: buffer has %d bytes", trace_server_buffer_size);
}

static void transmit_buffer(void)
{
    /* store size in packet at offset +4 */
    int size = buffer_pos;
    DWORD *ptr = (DWORD *)(buffer + 4);
    *ptr = htonl(size);
    
    /* send packet */
    trace_server_transmit((const char *)buffer, size);
    
    /* reset buffer */
    buffer_pos = PACKET_HEADER_SIZE;
    buffer_free = trace_server_buffer_size - PACKET_HEADER_SIZE;
}

BYTE *trace_server_request_buffer(DWORD message_id, DWORD size)
{
    DWORD total_size = size + MESSAGE_HEADER_SIZE;
    total_size = (total_size + 3) & ~3; /* align to 32 bit */
    
    /* message is too large for packet? */
    if(total_size > (trace_server_buffer_size - PACKET_HEADER_SIZE)) {
        return NULL;
    }
    
    /* does not fit into current buffer? */
    if(total_size > buffer_free) {
#ifdef DEBUG_TRACER
        log_message(LOG_DEFAULT, "tx full buffer (size %d, free %d)", buffer_pos, buffer_free);
#endif
        transmit_buffer();
    }
    
    /* allocate buffer */
    BYTE * buf = buffer + buffer_pos;
    buffer_pos += total_size;
    buffer_free -= total_size;
    
    /* write message header */
    DWORD * ptr = (DWORD *)buf;
    ptr[0] = htonl(message_id);
    ptr[1] = htonl(size);
    ptr[2] = htonl(maincpu_clk);
    
    /* reset flush tick */
    flush_tick = 0;
    
    /* return begin of data buffer */
    return buf + MESSAGE_HEADER_SIZE;
}

void trace_server_worker(void)
{
    /* handle connects and input data from trace client */
    if(trace_server_data_available()>0) {
        // TODO: handle client input
    }
    
    /* send unfinished packet after the flush delay */
    flush_tick++;
    if(flush_tick == trace_server_send_flush) {
        flush_tick = 0;
        if(buffer_pos > PACKET_HEADER_SIZE) {
#ifdef DEBUG_TRACER
            log_message(LOG_DEFAULT, "tx flush buffer (size %d, free %d)", buffer_pos, buffer_free);
#endif
            transmit_buffer();
        }
    }
}

/* ----- Tracer I/O ------------------------------------------------------ */

static int trace_server_open(void)
{
    vice_network_socket_address_t * server_addr = NULL;
    int error = 1;
   
    do {
        if ( ! server_address_string ) {
            break;
        }

        server_addr = vice_network_address_generate(server_address_string, 0);
        if ( ! server_addr ) {
            break;
        }

        listen_socket = vice_network_server(server_addr);
        if ( ! listen_socket ) {
            break;
        }

        log_message(LOG_DEFAULT, "trace server: started and listening at '%s'", server_address_string);
        error = 0;

    } while (0);

   if (server_addr) {
       vice_network_address_close(server_addr);
   }

   return error;
}

static int trace_server_close(void)
{
    if (listen_socket) {
        vice_network_socket_close(listen_socket);
        listen_socket = NULL;
        log_message(LOG_DEFAULT, "trace server: closed");
    }

    return 0;
}

static void trace_client_disconnect(void)
{
    if(connected_socket != NULL) {
        vice_network_socket_close(connected_socket);
        connected_socket = NULL;
        log_message(LOG_DEFAULT, "tracer: client disconnected");
    }
}

static int trace_server_transmit(const char * buffer, size_t buffer_length)
{
    int error = 0;
    if (connected_socket) {
        size_t len = vice_network_send(connected_socket, buffer, buffer_length, 0);
        if (len != buffer_length) {
            error = 1;
        }
    }
    return error;
}

static int trace_server_receive(char * buffer, size_t buffer_length)
{
    int count = 0;
    if (connected_socket ) {
        count = vice_network_receive(connected_socket, buffer, buffer_length, 0);
        if (count < 0) {
            log_message(LOG_DEFAULT, "tracer: receiption error, breaking connection");
            trace_client_disconnect();
        }
    }
    return count;
}

static int trace_server_data_available(void)
{
    int available = 0;

    if (connected_socket != NULL) {
        available = vice_network_select_poll_one(connected_socket);
        if(available < 0) {
            trace_client_disconnect();
        }
    }
    else if (listen_socket!= NULL) {
        /* we have no connection yet, allow for connection */
        if (vice_network_select_poll_one(listen_socket)) {
            connected_socket = vice_network_accept(listen_socket);
            if(connected_socket != NULL) {
                log_message(LOG_DEFAULT, "tracer: client connected");
            }
        }
    }


    return available;
}

/* ----- Resources ------------------------------------------------------- */

static int set_trace_server_enabled(int val, void *param)
{
    if (!val) {
        if (trace_server_enabled) {
            trace_server_close();
        }
        trace_server_enabled = 0;
        return 0;
    } else { 
        if (!trace_server_enabled) {
            if (trace_server_open() < 0) {
                return -1;
            }
            alloc_buffer();
        }
        trace_server_enabled = 1;
        return 0;
    }
}

static int set_trace_server_buffer_size(int val, void *param)
{
    if(val < 256) {
        return -1;
    }
    
    trace_server_buffer_size = val;
    alloc_buffer();
    return 0;
}

static int set_trace_server_send_flush(int val, void *param)
{
    if(val < 1) {
        return -1;
    }
    
    trace_server_send_flush = val;
    return 0;
}

static int set_server_address(const char *name, void *param)
{
    if (server_address_string != NULL && name != NULL
        && strcmp(name, server_address_string) == 0)
        return 0;

    if (trace_server_enabled) {
        trace_server_close();
    }
    util_string_set(&server_address_string, name);

    if (trace_server_enabled) {
        trace_server_open();
    }

    return 0;
}

static const resource_string_t resources_string[] = {
    { "TraceServerAddress", "ip4://127.0.0.1:6581", RES_EVENT_NO, NULL,
      &server_address_string, set_server_address, NULL },
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "TraceServer", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &trace_server_enabled, set_trace_server_enabled, NULL },
    { "TraceServerBufferSize", DEFAULT_BUFFER_SIZE, RES_EVENT_STRICT, (resource_value_t)0,
      &trace_server_buffer_size, set_trace_server_buffer_size, NULL },
    { "TraceServerSendFlush", DEFAULT_SEND_FLUSH, RES_EVENT_STRICT, (resource_value_t)0,
      &trace_server_send_flush, set_trace_server_send_flush, NULL },
    { NULL }
};

int trace_server_resources_init(void)
{
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}

void trace_server_resources_shutdown(void)
{
    if(trace_server_enabled) {
        trace_server_close();
    }

    /* free address string resource */
    if(server_address_string != NULL) {
        lib_free(server_address_string);
    }
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
    { "-traceserver", SET_RESOURCE, 0,
      NULL, NULL, "TraceServer", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable Trace Server") },
    { "+traceserver", SET_RESOURCE, 0,
      NULL, NULL, "TraceServer", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable Trace Server") },
    { "-traceserverbuffersize", SET_RESOURCE, 0,
      NULL, NULL, "TraceServerBufferSize", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<size>", N_("Trace Server Buffer Size") },
    { "-traceserversendflush", SET_RESOURCE, 0,
      NULL, NULL, "TraceServerSendFlush", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<time>", N_("Trace Server Time to Flush Buffer") },
    { "-traceserveraddress", SET_RESOURCE, 1,
      NULL, NULL, "TraceServerAddress", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_STRING,
      IDCLS_P_NAME, IDCLS_UNUSED,
      NULL, N_("Set Trace Server Address") },
    { NULL }
};

int trace_server_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

#endif
#endif
