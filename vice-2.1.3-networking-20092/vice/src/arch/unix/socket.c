/*! \file socket.c \n
 *  \author Spiro Trikaliotis\n
 *  \brief  Abstraction from network sockets.
 *
 * socket.c - Abstraction from network sockets. Windows implementation.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
 *
 * based on code from network.c written by
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "socket.h"
#include "socketimpl.h"

#define arraysize(_x) ( sizeof _x / sizeof _x[0] )

union socket_addresses_u {
    struct sockaddr     generic;
    struct sockaddr_un  local;
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
};

struct vice_network_socket_address_s
{
    unsigned int used;
    int domain;
    int protocol;
    size_t len;
    union socket_addresses_u address;
};

static vice_network_socket_address_t address_pool[16] = { { 0 } };

vice_network_socket_t vice_network_server(const vice_network_socket_address_t * server_address)
{
    int sockfd = INVALID_SOCKET;
    int error = 1;

    do {
        sockfd = socket(server_address->domain, SOCK_STREAM, server_address->protocol);

        if (sockfd < 0) {
            sockfd = INVALID_SOCKET;
            break;
        }

        if (bind(sockfd, & server_address->address.generic, server_address->len) < 0) {
            break;
        }
        if (listen(sockfd, 2) < 0) {
            break;
        }
        error = 0;
    } while(0);

    if (error) {
        if (sockfd >= 0) {
            closesocket(sockfd);
        }
        sockfd = INVALID_SOCKET;
    }

    return (vice_network_socket_t) (sockfd ^ INVALID_SOCKET);
}

vice_network_socket_t vice_network_client(const vice_network_socket_address_t * server_address)
{
    int sockfd = INVALID_SOCKET;
    int error = 1;

    do {
        sockfd = socket(server_address->domain, SOCK_STREAM, server_address->protocol);

        if (sockfd < 0) {
            sockfd = INVALID_SOCKET;
            break;
        }

        if (connect(sockfd, & server_address->address.generic, server_address->len) < 0) {
            break;
        }
        error = 0;
    } while(0);

    if (error) {
        if (sockfd >= 0) {
            closesocket(sockfd);
        }
        sockfd = INVALID_SOCKET;
    }

    return (vice_network_socket_t) (sockfd ^ INVALID_SOCKET);
}

static vice_network_socket_address_t * vice_network_alloc_new_socket_address(void)
{
    vice_network_socket_address_t * return_address = NULL;
    int i;

    for (i = 0; i < arraysize(address_pool); i++)
    {
        if (address_pool[i].used == 0) {
            return_address = & address_pool[i];
            memset(return_address, 0, sizeof * return_address);
            return_address->used = 1;
            return_address->len = sizeof return_address->address;
            break;
        }
    }

    return return_address;
}

vice_network_socket_address_t * vice_network_address_generate(const char * address_string, unsigned short port)
{
    vice_network_socket_address_t * socket_address = NULL;

    const char * address_part = address_string;

    do {
        socket_address = vice_network_alloc_new_socket_address();
        if (socket_address == NULL) {
            break;
        }

        /* preset the socket address with port and INADDR_ANY */

        socket_address->domain = PF_INET;
        socket_address->protocol = IPPROTO_TCP;
        socket_address->len = sizeof socket_address->address.ipv4;
        socket_address->address.ipv4.sin_family = PF_INET;
        socket_address->address.ipv4.sin_port = htons(port);
        socket_address->address.ipv4.sin_addr.s_addr = INADDR_ANY;

        if (address_string) {
            /* an address string was specified, try to use it */
            struct hostent * host_entry;

            char * port_part = NULL;

            /* try to find out if a port has been specified */
            port_part = strchr(address_string, ':');

            if (port_part) {
                char * p;
                unsigned long new_port;

                /* yes, there is a port: Copy the part before, so we can modify it */
                p = lib_stralloc(address_string);

                p[port_part - address_string] = 0;
                address_part = p;

                ++port_part;

                new_port = strtoul(port_part, &p, 10);

                if (*p == 0) {

                    socket_address->address.ipv4.sin_port = htons((unsigned short) new_port);
                }
            }
 
            host_entry = gethostbyname(address_part);

            if (host_entry != NULL && host_entry->h_addrtype == AF_INET) {
                if ( host_entry->h_length != sizeof socket_address->address.ipv4.sin_addr.s_addr ) {
                    /* something weird happened... SHOULD NOT HAPPEN! */
                    fprintf(stderr, 
                              "gethostbyname() returned an IPv4 address, "
                              "but the length is wrong: %u", host_entry->h_length );
                    break;
                }

                memcpy(&socket_address->address.ipv4.sin_addr.s_addr, host_entry->h_addr_list[0], host_entry->h_length);
            }
            else {
                /* no host name: Assume it is an IP address */
                socket_address->address.ipv4.sin_addr.s_addr = inet_addr(address_part);
            }
        }
    } while (0);

    /* if we allocated memory for the address part
     * because a port was specified,
     * free that memory now.
     */
    if (address_part != address_string) {
        lib_free(address_part);
    }

    return socket_address;
}


void vice_network_address_close(vice_network_socket_address_t * address)
{
    if (address)
    {
        address->used = 0;
    }
}

vice_network_socket_t vice_network_accept(vice_network_socket_t sockfd, vice_network_socket_address_t ** client_address)
{
    vice_network_socket_t newsocket = -1;
    vice_network_socket_address_t * socket_address;

    do {
        socket_address = vice_network_alloc_new_socket_address();
        if (socket_address == NULL) {
            break;
        }

        newsocket = accept(sockfd ^ INVALID_SOCKET, & socket_address->address.generic, & socket_address->len);

    } while (0);

    if (newsocket >= 0 && client_address) {
        * client_address = socket_address;
    }
    else {
        vice_network_address_close(socket_address);
    }

    return newsocket ^ INVALID_SOCKET;
}

int vice_network_socket_close(vice_network_socket_t sockfd)
{
    return closesocket(sockfd ^ INVALID_SOCKET);
}

int vice_network_send(vice_network_socket_t sockfd, const void * buffer, size_t buffer_length, int flags)
{
    return send(sockfd ^ INVALID_SOCKET, buffer, buffer_length, flags);
}

int vice_network_receive(vice_network_socket_t sockfd, void * buffer, size_t buffer_length, int flags)
{
    return recv(sockfd ^ INVALID_SOCKET, buffer, buffer_length, flags);
}

int vice_network_select_poll_one(vice_network_socket_t readsockfd)
{
    TIMEVAL timeout = { 0 };

    fd_set fdsockset;

    FD_ZERO(&fdsockset);
    FD_SET(readsockfd ^ INVALID_SOCKET, &fdsockset);

    return select( (readsockfd ^ INVALID_SOCKET) + 1, &fdsockset, NULL, NULL, &timeout);
}

int vice_network_get_errorcode(void)
{
    return errno;
}
