%{
/* -*- C -*-
 *
 * mon_parse.y - Parser for the VICE built-in monitor.
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

#include "vice.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mon_parsers.h"

#include "asm.h"
#include "console.h"
#include "lib.h"
#include "machine.h"
#include "montypes.h"
#include "mon_util.h"
#include "types.h"
#include "uimon.h"


static int yyerror(char *s);

/* Defined in the lexer */
void mon_asm6502_lex_delete_buffer(void);
void mon_asm6502_lex_set_buffer(char *str);
int yylex(void);

extern int cur_len, last_len;

#define YYDEBUG 1

static void emit_imm(int op, int arg);
static void emit_abs(int op, int addr);
static void emit_abs_zp(int op_abs, int op_zp, int addr);

%}

%name-prefix "mon_6502asm_yy"

%union {
    MON_ADDR a;
    int i;
    REG_ID reg;
    ACTION action;
    char *str;
    asm_mode_addr_info_t mode;
}

%token<i>   B_NUMBER O_NUMBER D_NUMBER H_NUMBER
%token      EOL HASH L_BRACKET R_BRACKET COMMA
%token      OP_ADC
%token      OP_AND
%token      OP_ASL
%token      OP_BRK
%token      OP_BPL
%token      OP_JAM
%token      OP_JSR
%token      OP_NOP
%token      OP_ORA
%token      OP_SLO
%token      REG_X
%token      REG_Y
%type<i>    mode_imm mode_abs mode_abs_x mode_abs_y
%type<i>    mode_ind_x mode_ind_y
%type<i>    address imm_value number

 /* --------------------------------------------------------------------- */
%%

instruction_list:
      instruction EOL
    | EOL               {   mon_exit_assemble_mode(); }
    ;

 /* --------------------------------------------------------------------- */
 /*                           operation codes                             */
 /* --------------------------------------------------------------------- */
instruction:
      OP_BRK            { mon_emit_code(1, 0x00); }

    | OP_ADC mode_imm   { emit_imm(0x69, $2); }
    | OP_ADC mode_abs   { emit_abs_zp(0x6d, 0x65, $2); }
    | OP_ADC mode_abs_x { emit_abs_zp(0x7d, 0x75, $2); }
    | OP_ADC mode_abs_y { emit_abs(0x79, $2); }
    | OP_ADC mode_ind_x { emit_imm(0x61, $2); }
    | OP_ADC mode_ind_y { emit_imm(0x71, $2); }

    | OP_AND mode_imm   { emit_imm(0x29, $2); }
    | OP_AND mode_abs   { emit_abs_zp(0x2d, 0x25, $2); }
    | OP_AND mode_abs_x { emit_abs_zp(0x3d, 0x35, $2); }
    | OP_AND mode_abs_y { emit_abs(0x39, $2); }
    | OP_AND mode_ind_x { emit_imm(0x21, $2); }
    | OP_AND mode_ind_y { emit_imm(0x31, $2); }

    | OP_ORA mode_ind_x { emit_imm(0x01, $2); }

    | OP_JAM            { mon_emit_code(1, 0x02); }
    ;

 /* --------------------------------------------------------------------- */
 /*                           addressing modes                            */
 /* --------------------------------------------------------------------- */

mode_imm:
    HASH imm_value
                        {   $$ = $2; }
    ;

mode_abs:
    address
                        {   $$ = $1; }
    ;

mode_abs_x:
    address COMMA REG_X
                        {   $$ = $1; }
    ;

mode_abs_y:
    address COMMA REG_Y
                        {   $$ = $1; }
    ;

mode_ind_x:
    L_BRACKET address COMMA REG_X R_BRACKET
                        {   $$ = $2; }
    ;

mode_ind_y:
    L_BRACKET address R_BRACKET COMMA REG_Y
                        {   $$ = $2; }
    ;

 /* --------------------------------------------------------------------- */
 /*                           labels and numbers                          */
 /* --------------------------------------------------------------------- */
address:
      number            {   $$ = $1; }

imm_value:
      number            {   $$ = $1; }

number:
      B_NUMBER          {   $$ = $1; }
    | O_NUMBER          {   $$ = $1; }
    | D_NUMBER          {   $$ = $1; }
    | H_NUMBER          {   $$ = $1; }
    ;

 /* --------------------------------------------------------------------- */
%%

 /* --------------------------------------------------------------------- */
 /*               address mode dependent code emitters                    */
 /* --------------------------------------------------------------------- */

static void emit_imm(int op, int arg)
{
    mon_emit_code(2, op, arg);
}

static void emit_abs(int op, int addr)
{
    mon_emit_code(3, op, BYTE0(addr), BYTE1(addr));
}

static void emit_abs_zp(int op_abs, int op_zp, int addr)
{
    if (addr < 0x100)
	    mon_emit_code(2, op_zp, addr);
	else
	    mon_emit_code(3, op_abs, BYTE0(addr), BYTE1(addr));
}


int mon_asm6502_assemble_line(char* buffer)
{
    int rc;

    yydebug = 1;

    mon_asm6502_lex_set_buffer(buffer);
    rc = yyparse();
    mon_asm6502_lex_cleanup_buffer();

    return rc;
}

static int yyerror(char *s)
{
    fprintf(stderr, "ERR:%s\n", s);
    return 0;
}

