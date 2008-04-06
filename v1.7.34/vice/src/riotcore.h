/*
 * riotcore.h - Core functions for RIOT emulation.
 *
 * Written by
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
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

#include "log.h"
#include "riot.h"
#include "types.h"

#if defined(NO_INLINE)
#define _RIOT_FUNC       static
#else
#define _RIOT_FUNC       static inline
#endif

/*
 * Local variables
 */

#ifdef RIOT_SHARED_CODE
#define RIOT_CONTEXT_PARAM      RIOTCONTEXT *ctxptr,
#define RIOT_CONTEXT_PARVOID    RIOTCONTEXT *ctxptr
#define RIOT_CONTEXT_CALL       ctxptr,
#define RIOT_CONTEXT_CALLVOID   ctxptr
#define RIOTRPARM1              REGPARM2
#define RIOTRPARM2              REGPARM3
#else
#define RIOT_CONTEXT_PARAM
#define RIOT_CONTEXT_PARVOID    void
#define RIOT_CONTEXT_CALL
#define RIOT_CONTEXT_CALLVOID
#define RIOTRPARM1              REGPARM1
#define RIOTRPARM2              REGPARM2

static BYTE riotio[4];          /* I/O register data */

static BYTE oldpa;              /* the actual output on PA (input = high) */
static BYTE oldpb;              /* the actual output on PB (input = high) */

static log_t riot_log = LOG_ERR;

#endif

