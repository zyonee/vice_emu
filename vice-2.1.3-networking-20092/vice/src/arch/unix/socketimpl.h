/*! \file socketimpl.h \n
 *  \author Spiro Trikaliotis\n
 *  \brief  Abstraction from network sockets.
 *
 * socketimpl.h - Abstraction from network sockets. Unix implementation.
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

#ifndef VICE_SOCKETIMPL_H
#define VICE_SOCKETIMPL_H

#ifdef HAVE_NETWORK
 
#ifdef MINIX_SUPPORT
# include <limits.h>
# define PF_INET AF_INET

# ifndef MINIX_HAS_RECV_SEND
extern ssize_t recv(int socket, void *buffer, size_t length, int flags);
extern ssize_t send(int socket, const void *buffer, size_t length, int flags);
# endif

#endif /* #ifdef MINIX_SUPPORT */

#ifdef AMIGA_SUPPORT
# ifndef AMIGA_OS4
#  ifdef AMIGA_M68K
#   include <utility/tagitem.h>
#   include <clib/exec_protos.h>
#  endif
#  ifdef AMIGA_AROS
#   include <proto/exec.h>
#  endif
#  include <proto/socket.h>
struct Library *SocketBase;
# else
#  define __USE_INLINE__
#  include <proto/bsdsocket.h>
# endif
# if !defined(AMIGA_AROS) && !defined(AMIGA_MORPHOS)
#  define select(nfds, read_fds, write_fds, except_fds, timeout) \
          WaitSelect(nfds, read_fds, write_fds, except_fds, timeout, NULL)
# endif
#endif

#ifdef __BEOS__
# include <socket.h>
# include <netdb.h>
# include <byteorder.h>
typedef unsigned int SOCKET;
typedef struct timeval TIMEVAL;
# define PF_INET AF_INET
# define INVALID_SOCKET (SOCKET)(~0)
# define HAVE_HTONS
# define HAVE_HTONL

#else /* #ifdef __BEOS__ */

# if !defined(HAVE_GETDTABLESIZE) && defined(HAVE_GETRLIMIT)
#  include <sys/resource.h>
# endif
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# ifndef __MSDOS__
#  include <sys/time.h>
#  if !defined(AMIGA_SUPPORT) && !defined(VMS)
#   include <sys/select.h>
#  endif
# endif
# if !defined(AMIGA_M68K) && !defined(AMIGA_AROS)
#  include <unistd.h>
# endif

# ifdef __minix
#  define recv(socket, buffer, length, flags) \
          recvfrom(socket, buffer, length, flags, NULL, NULL)
extern ssize_t send(int socket, const void *buffer, size_t length, int flags);
# endif

typedef unsigned int SOCKET;
typedef struct timeval TIMEVAL;

# ifdef AMIGA_SUPPORT
#  ifdef AMIGA_OS4
#   define closesocket close
#  else
#   define closesocket CloseSocket
#  endif
# else
#  define closesocket close
# endif

# ifndef INVALID_SOCKET
#  define INVALID_SOCKET (SOCKET)(~0)
# endif

#endif /* UNIX */

#define SOCKET_IS_INVALID(_x) ((_x) < 0)

#endif /* #ifdef HAVE_NETWORK */

#endif /* #ifndef VICE_SOCKETIMPL_H */
