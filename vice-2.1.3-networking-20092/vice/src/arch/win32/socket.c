/*! \file socket.c \n
 *  \author Spiro Trikaliotis\n
 *  \brief  Abstraction from network sockets.
 *
 * socket.c - Abstraction from network sockets.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
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

#include "lib.h"
#include "socket.h"

#include <winsock.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE 64 /* just in case mingw or msvc doesn't define it */
#endif

struct vice_network_socket_address_s
{
    unsigned int len;
    struct sockaddr * address;

};

vice_network_socket_t vice_network_socket_tcp(void)
{
    vice_network_socket_t sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    return sockfd;
}

vice_network_socket_t vice_network_socket_udp(void)
{
    vice_network_socket_t sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return sockfd;
}

static vice_network_socket_address_t * vice_network_alloc_new_socket_address(struct sockaddr * input_address, int address_length)
{
    vice_network_socket_address_t * return_address = NULL;

    do {
        return_address = lib_malloc(sizeof(* return_address));
        if (return_address == NULL)
            break;

        return_address->len = address_length;
        return_address->address = lib_malloc(return_address->len);

        if ( ! return_address->address ) {
            lib_free(return_address);
            return_address=NULL;
            break;
        }

        memcpy(return_address->address, input_address, return_address->len);

    } while (0);

    return return_address;
}

vice_network_socket_address_t * vice_network_address_generate(const char * address_string, unsigned short port)
{
    struct sockaddr_in socket_addr = { 0 };

    const char * address_part = address_string;

    do {
        /* preset the socket address with port and INADDR_ANY */

        socket_addr.sin_family = PF_INET;
        socket_addr.sin_port = htons(port);
        socket_addr.sin_addr.s_addr = INADDR_ANY;

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

                    socket_addr.sin_port = htons((unsigned short) new_port);
                }
            }
 
            host_entry = gethostbyname(address_part);

            if (host_entry != NULL && host_entry->h_addrtype == AF_INET) {
                if ( host_entry->h_length != sizeof socket_addr.sin_addr.s_addr ) {
                    /* something weird happened... SHOULD NOT HAPPEN! */
                    fprintf(stderr, 
                              "gethostbyname() returned an IPv4 address, "
                              "but the length is wrong: %u", host_entry->h_length );
                    break;
                }

                memcpy(&socket_addr.sin_addr.s_addr, host_entry->h_addr_list[0], host_entry->h_length);
            }
            else {
                /* no host name: Assume it is an IP address */
                socket_addr.sin_addr.s_addr = inet_addr(address_part);
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

    return vice_network_alloc_new_socket_address((struct sockaddr *) &socket_addr, sizeof socket_addr);
}


void vice_network_address_close(vice_network_socket_address_t * address)
{
    if (address)
    {
        free(address->address);
        free(address);
    }
}

int vice_network_bind(vice_network_socket_t sockfd, const vice_network_socket_address_t * addr)
{
    return bind(sockfd, addr->address, addr->len);
}

int vice_network_listen(vice_network_socket_t sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

int vice_network_connect(vice_network_socket_t sockfd, const vice_network_socket_address_t * server_address)
{
    return connect(sockfd, server_address->address, server_address->len);
}

vice_network_socket_t vice_network_accept(vice_network_socket_t sockfd, vice_network_socket_address_t ** client_address)
{
    struct sockaddr * addr = { 0 };
    int addr_length = sizeof(*addr);

    vice_network_socket_t newsocket;

    newsocket = accept(sockfd, addr, &addr_length);

    if (newsocket != INVALID_SOCKET) {
        * client_address = vice_network_alloc_new_socket_address(addr, addr_length);
    }
    return newsocket;
}

int vice_network_socket_close(vice_network_socket_t sockfd)
{
    return closesocket(sockfd);
}

int vice_network_send(vice_network_socket_t sockfd, const void * buffer, size_t buffer_length, int flags)
{
    return send(sockfd, buffer, buffer_length, flags);
}

int vice_network_receive(vice_network_socket_t sockfd, void * buffer, size_t buffer_length, int flags)
{
    return recv(sockfd, buffer, buffer_length, flags);
}

int vice_network_select_poll_one(vice_network_socket_t readsockfd)
{
    TIMEVAL timeout = { 0 };

    fd_set fdsockset;

    FD_ZERO(&fdsockset);
    FD_SET(readsockfd, &fdsockset);

    return select(0, &fdsockset, NULL, NULL, &timeout);
}

int vice_network_get_errorcode(void)
{
    return WSAGetLastError();
}
