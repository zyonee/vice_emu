/*! \file socket.c \n
 *  \author Spiro Trikaliotis\n
 *  \brief  Abstraction from network sockets.
 *
 * socket.c - Abstraction from network sockets.
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
#include "log.h"
#include "socket.h"
#include "socketimpl.h"

#define arraysize(_x) ( sizeof _x / sizeof _x[0] )

union socket_addresses_u {
    struct sockaddr     generic;

#ifdef HAVE_UNIX_DOMAIN_SOCKETS
    struct sockaddr_un  local;
#endif

    struct sockaddr_in  ipv4;

#ifdef HAVE_IPV6
    struct sockaddr_in6 ipv6;
#endif
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

        if (SOCKET_IS_INVALID(sockfd)) {
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
        if ( ! SOCKET_IS_INVALID(sockfd) ) {
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

        if (SOCKET_IS_INVALID(sockfd)) {
            sockfd = INVALID_SOCKET;
            break;
        }

        if (connect(sockfd, & server_address->address.generic, server_address->len) < 0) {
            break;
        }
        error = 0;
    } while(0);

    if (error) {
        if ( ! SOCKET_IS_INVALID(sockfd) ) {
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

static int vice_network_address_generate_ipv4(vice_network_socket_address_t * socket_address, const char * address_string, unsigned short port)
{
    const char * address_part = address_string;
    int error = 1;

    do {
        /* preset the socket address with port and INADDR_ANY */

        memset(&socket_address->address, 0, sizeof socket_address->address);
        socket_address->domain = PF_INET;
        socket_address->protocol = IPPROTO_TCP;
        socket_address->len = sizeof socket_address->address.ipv4;
        socket_address->address.ipv4.sin_family = AF_INET;
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
                    log_message(LOG_DEFAULT, 
                              "gethostbyname() returned an IPv4 address, "
                              "but the length is wrong: %u", host_entry->h_length );
                    break;
                }

                memcpy(&socket_address->address.ipv4.sin_addr.s_addr, host_entry->h_addr_list[0], host_entry->h_length);
            }
            else {
                /* Assume it is an IP address */

                if (address_part[0] != 0) {
#ifdef HAVE_INET_ATON
                    /*
                     * this implementation is preferable, as inet_addr() cannot
                     * return the broadcast address, as it has the same encoding
                     * as INADDR_NONE.
                     *
                     * Unfortunately, not all ports have inet_aton(), so, we must
                     * provide both implementations.
                     */
                    if (inet_aton(address_part, &socket_address->address.ipv4.sin_addr.s_addr) == 0) {
                        /* no valid IP address */
                        break;
                    }
#else
                    in_addr_t ip = inet_addr(address_part);

                    if (ip == INADDR_NONE) {
                        /* no valid IP address */
                        break;
                    }
                    socket_address->address.ipv4.sin_addr.s_addr = ip;
#endif
                }
            }

            error = 0;
        }
    } while (0);

    /* if we allocated memory for the address part
     * because a port was specified,
     * free that memory now.
     */
    if (address_part != address_string) {
        lib_free(address_part);
    }

    return error;
}

static int vice_network_address_generate_ipv6(vice_network_socket_address_t * socket_address, const char * address_string, unsigned short port)
{
    int error = 1;

#ifdef HAVE_IPV6
    do {
        struct hostent * host_entry = NULL;
        int err6;

        /* preset the socket address */

        memset(&socket_address->address, 0, sizeof socket_address->address);
        socket_address->domain = PF_INET6;
        socket_address->protocol = IPPROTO_TCP;
        socket_address->len = sizeof socket_address->address.ipv6;
        socket_address->address.ipv6.sin6_family = AF_INET6;
        socket_address->address.ipv6.sin6_port = htons(port);
        socket_address->address.ipv6.sin6_addr = in6addr_any;

#ifdef HAVE_GETHOSTBYNAME2
        host_entry = gethostbyname2(address_string, AF_INET6);
#else
        host_entry = getipnodebyname(address_string, AF_INET6, AI_DEFAULT, &err6);
#endif
        if (host_entry == NULL) {
            break;
        }

        memcpy(&socket_address->address.ipv6.sin6_addr, host_entry->h_addr, host_entry->h_length);

#ifdef HAVE_GETHOSTBYNAME2
#else
        freehostent(host_entry);
#endif
        error = 0;

    } while (0);
#else /* #ifdef HAVE_IPV6 */
    log_message(LOG_DEFAULT, "IPv6 is not supported in this installation of VICE!\n");
#endif /* #ifdef HAVE_IPV6 */

    return error;
}

static int vice_network_address_generate_local(vice_network_socket_address_t * socket_address, const char * address_string, unsigned short port)
{
    int error = 1;

#ifdef HAVE_UNIX_DOMAIN_SOCKETS
    do {
        if (address_string[0] == 0) {
            break;
        }

        /* preset the socket address with port and INADDR_ANY */

        memset(&socket_address->address, 0, sizeof socket_address->address);
        socket_address->domain = PF_UNIX;
        socket_address->protocol = 0;
        socket_address->len = sizeof socket_address->address.local;
        socket_address->address.local.sun_family = AF_UNIX;

        if ( strlen(address_string) >= sizeof socket_address->address.local.sun_path ) {
            log_message(LOG_DEFAULT,
                        "Unix domain socket name of '%s' is too long; only %u chars are allowed.",
                        address_string, sizeof socket_address->address.local.sun_path);
            break;
        }
        strcpy(socket_address->address.local.sun_path, address_string);

        error = 0;
    } while (0);

#else /* #ifdef HAVE_UNIX_DOMAIN_SOCKETS */
    log_message(LOG_DEFAULT, "Unix domain sockets are not supported in this installation of VICE!\n");
#endif /* #ifdef HAVE_UNIX_DOMAIN_SOCKETS */

    return error;
}

vice_network_socket_address_t * vice_network_address_generate(const char * address_string, unsigned short port)
{
    vice_network_socket_address_t * socket_address = NULL;
    int error = 1;

    do {
        socket_address = vice_network_alloc_new_socket_address();
        if (socket_address == NULL) {
            break;
        }

        if (address_string && address_string[0] == '|') {
            if (vice_network_address_generate_local(socket_address, &address_string[1], port)) {
                break;
            }
        }
        else if (address_string && strncmp("ip6://", address_string, sizeof "ip6://" - 1) == 0) {
            if (vice_network_address_generate_ipv6(socket_address, &address_string[sizeof "ip6://" - 1], port)) {
                break;
            }
        }
        else if (address_string && strncmp("ip4://", address_string, sizeof "ip4://" - 1) == 0) {
            if (vice_network_address_generate_ipv4(socket_address, &address_string[sizeof "ip4://" - 1], port)) {
                break;
            }
        }
        else {
            /* the user did not specify the type of the address, try to guess it by trying IPv6, then IPv4 */
#ifdef HAVE_IPV6
            if ( vice_network_address_generate_ipv6(socket_address, address_string, port))
#endif /* #ifdef HAVE_IPV6 */
            {
                if ( vice_network_address_generate_ipv4(socket_address, address_string, port)) {
                    break;
                }
            }

        }

        error = 0;

    } while (0);

    if (error && socket_address) {
        vice_network_address_close(socket_address);
        socket_address = NULL;
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

    if ( ! SOCKET_IS_INVALID(newsocket) && client_address) {
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
