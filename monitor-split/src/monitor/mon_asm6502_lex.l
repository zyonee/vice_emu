/* -*- C -*-
 *
 * mon_lex.l - Lexer for the VICE built-in monitor.
 *
 * Written by
 *  Daniel Sladic <sladic@eecg.toronto.edu>
 *  Andreas Boose <viceteam@t-online.de>
 *  Thomas Giesel <skoe@directbox.com>
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
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * VICE DEVELOPERS, IMPORTANT NOTE: The automake environment doesn't
 * generate the flex/bison parsers automatically at the moment.
 *
 * We changed this because automake/ylwrap don't support some features
 * we need, e.g. more than one parser with different prefixes in one
 * executable.
 *
 * After you changed a *.l or *.y file, execute: cd src/monitor; ./genparser.sh
 * If everything went fine, commit these and the generated *.c and *.h files.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

%{

/* Lexer for x64 monitor */

#include "vice.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mon_parsers.h"

#include "lib.h"
#include "mon_command.h"
#include "mon_util.h"
#include "montypes.h"
#include "asm.h"
#include "mon_asm6502_parse.h"
#include "types.h"


#define YY_ALWAYS_INTERACTIVE 1
#define YY_NO_UNISTD_H

/* for our prefix, we don't want to change all of the rules below */
#define yylval mon_6502asm_yylval

static int yywrap(void);
static YY_BUFFER_STATE my_state;


%}

%option prefix="mon_6502asm_yy"
%option case-insensitive
%option nounput
%option noinput

%%


adc             { return OP_ADC; }
and             { return OP_AND; }
asl             { return OP_ASL; }
asl[ \t]*a      { return OP_ASL; }
bpl             { return OP_BPL; }
brk             { return OP_BRK; }
jam             { return OP_JAM; }
jsr             { return OP_JSR; }
nop             { return OP_NOP; }
ora             { return OP_ORA; }
slo             { return OP_SLO; }

x               { return REG_X; }
y               { return REG_Y; }


 /* number formats: No guessing, just the usual prefixes */
%[01]+          { yylval.i = strtol(yytext+1, NULL, 2); return B_NUMBER; }
&[0-7]+         { yylval.i = strtol(yytext+1, NULL, 8); return O_NUMBER; }
[0-9]+          { yylval.i = strtol(yytext, NULL, 10); return D_NUMBER; }
$[0-9a-f]+      { yylval.i = strtol(yytext+1, NULL, 16); return H_NUMBER; }


#               { return HASH; }
\(              { return L_BRACKET; }
\)              { return R_BRACKET; }
\,              { return COMMA; }

<*>[ \t]*

<*>\n           { return EOL; }

%%

void mon_asm6502_lex_set_buffer(char *str)
{
    my_state = yy_scan_buffer(str, strlen(str) + 2);
}

void mon_asm6502_lex_cleanup_buffer(void)
{
   yy_delete_buffer(my_state);
}

static int yywrap(void)
{
    return 1;
}

