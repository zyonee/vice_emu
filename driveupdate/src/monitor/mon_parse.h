
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     H_NUMBER = 258,
     D_NUMBER = 259,
     O_NUMBER = 260,
     B_NUMBER = 261,
     CONVERT_OP = 262,
     B_DATA = 263,
     H_RANGE_GUESS = 264,
     D_NUMBER_GUESS = 265,
     O_NUMBER_GUESS = 266,
     B_NUMBER_GUESS = 267,
     BAD_CMD = 268,
     MEM_OP = 269,
     IF = 270,
     MEM_COMP = 271,
     MEM_DISK8 = 272,
     MEM_DISK9 = 273,
     MEM_DISK10 = 274,
     MEM_DISK11 = 275,
     EQUALS = 276,
     MEM_FDISK8 = 277,
     MEM_FDISK9 = 278,
     MEM_FDISK10 = 279,
     MEM_FDISK11 = 280,
     TRAIL = 281,
     CMD_SEP = 282,
     LABEL_ASGN_COMMENT = 283,
     CMD_SIDEFX = 284,
     CMD_RETURN = 285,
     CMD_BLOCK_READ = 286,
     CMD_BLOCK_WRITE = 287,
     CMD_UP = 288,
     CMD_DOWN = 289,
     CMD_LOAD = 290,
     CMD_SAVE = 291,
     CMD_VERIFY = 292,
     CMD_IGNORE = 293,
     CMD_HUNT = 294,
     CMD_FILL = 295,
     CMD_MOVE = 296,
     CMD_GOTO = 297,
     CMD_REGISTERS = 298,
     CMD_READSPACE = 299,
     CMD_WRITESPACE = 300,
     CMD_RADIX = 301,
     CMD_MEM_DISPLAY = 302,
     CMD_BREAK = 303,
     CMD_TRACE = 304,
     CMD_IO = 305,
     CMD_BRMON = 306,
     CMD_COMPARE = 307,
     CMD_DUMP = 308,
     CMD_UNDUMP = 309,
     CMD_EXIT = 310,
     CMD_DELETE = 311,
     CMD_CONDITION = 312,
     CMD_COMMAND = 313,
     CMD_ASSEMBLE = 314,
     CMD_DISASSEMBLE = 315,
     CMD_NEXT = 316,
     CMD_STEP = 317,
     CMD_PRINT = 318,
     CMD_DEVICE = 319,
     CMD_HELP = 320,
     CMD_WATCH = 321,
     CMD_DISK = 322,
     CMD_SYSTEM = 323,
     CMD_QUIT = 324,
     CMD_CHDIR = 325,
     CMD_BANK = 326,
     CMD_LOAD_LABELS = 327,
     CMD_SAVE_LABELS = 328,
     CMD_ADD_LABEL = 329,
     CMD_DEL_LABEL = 330,
     CMD_SHOW_LABELS = 331,
     CMD_RECORD = 332,
     CMD_MON_STOP = 333,
     CMD_PLAYBACK = 334,
     CMD_CHAR_DISPLAY = 335,
     CMD_SPRITE_DISPLAY = 336,
     CMD_TEXT_DISPLAY = 337,
     CMD_SCREENCODE_DISPLAY = 338,
     CMD_ENTER_DATA = 339,
     CMD_ENTER_BIN_DATA = 340,
     CMD_KEYBUF = 341,
     CMD_BLOAD = 342,
     CMD_BSAVE = 343,
     CMD_SCREEN = 344,
     CMD_UNTIL = 345,
     CMD_CPU = 346,
     CMD_YYDEBUG = 347,
     CMD_BACKTRACE = 348,
     CMD_SCREENSHOT = 349,
     CMD_PWD = 350,
     CMD_DIR = 351,
     CMD_RESOURCE_GET = 352,
     CMD_RESOURCE_SET = 353,
     CMD_ATTACH = 354,
     CMD_DETACH = 355,
     CMD_MON_RESET = 356,
     CMD_TAPECTRL = 357,
     CMD_CARTFREEZE = 358,
     CMD_CPUHISTORY = 359,
     CMD_MEMMAPZAP = 360,
     CMD_MEMMAPSHOW = 361,
     CMD_MEMMAPSAVE = 362,
     CMD_COMMENT = 363,
     CMD_LIST = 364,
     CMD_EXPORT = 365,
     CMD_AUTOSTART = 366,
     CMD_AUTOLOAD = 367,
     CMD_LABEL_ASGN = 368,
     L_PAREN = 369,
     R_PAREN = 370,
     ARG_IMMEDIATE = 371,
     REG_A = 372,
     REG_X = 373,
     REG_Y = 374,
     COMMA = 375,
     INST_SEP = 376,
     REG_B = 377,
     REG_C = 378,
     REG_D = 379,
     REG_E = 380,
     REG_H = 381,
     REG_L = 382,
     REG_AF = 383,
     REG_BC = 384,
     REG_DE = 385,
     REG_HL = 386,
     REG_IX = 387,
     REG_IY = 388,
     REG_SP = 389,
     REG_IXH = 390,
     REG_IXL = 391,
     REG_IYH = 392,
     REG_IYL = 393,
     STRING = 394,
     FILENAME = 395,
     R_O_L = 396,
     OPCODE = 397,
     LABEL = 398,
     BANKNAME = 399,
     CPUTYPE = 400,
     MON_REGISTER = 401,
     COMPARE_OP = 402,
     RADIX_TYPE = 403,
     INPUT_SPEC = 404,
     CMD_CHECKPT_ON = 405,
     CMD_CHECKPT_OFF = 406,
     TOGGLE = 407,
     MASK = 408
   };
#endif
/* Tokens.  */
#define H_NUMBER 258
#define D_NUMBER 259
#define O_NUMBER 260
#define B_NUMBER 261
#define CONVERT_OP 262
#define B_DATA 263
#define H_RANGE_GUESS 264
#define D_NUMBER_GUESS 265
#define O_NUMBER_GUESS 266
#define B_NUMBER_GUESS 267
#define BAD_CMD 268
#define MEM_OP 269
#define IF 270
#define MEM_COMP 271
#define MEM_DISK8 272
#define MEM_DISK9 273
#define MEM_DISK10 274
#define MEM_DISK11 275
#define EQUALS 276
#define MEM_FDISK8 277
#define MEM_FDISK9 278
#define MEM_FDISK10 279
#define MEM_FDISK11 280
#define TRAIL 281
#define CMD_SEP 282
#define LABEL_ASGN_COMMENT 283
#define CMD_SIDEFX 284
#define CMD_RETURN 285
#define CMD_BLOCK_READ 286
#define CMD_BLOCK_WRITE 287
#define CMD_UP 288
#define CMD_DOWN 289
#define CMD_LOAD 290
#define CMD_SAVE 291
#define CMD_VERIFY 292
#define CMD_IGNORE 293
#define CMD_HUNT 294
#define CMD_FILL 295
#define CMD_MOVE 296
#define CMD_GOTO 297
#define CMD_REGISTERS 298
#define CMD_READSPACE 299
#define CMD_WRITESPACE 300
#define CMD_RADIX 301
#define CMD_MEM_DISPLAY 302
#define CMD_BREAK 303
#define CMD_TRACE 304
#define CMD_IO 305
#define CMD_BRMON 306
#define CMD_COMPARE 307
#define CMD_DUMP 308
#define CMD_UNDUMP 309
#define CMD_EXIT 310
#define CMD_DELETE 311
#define CMD_CONDITION 312
#define CMD_COMMAND 313
#define CMD_ASSEMBLE 314
#define CMD_DISASSEMBLE 315
#define CMD_NEXT 316
#define CMD_STEP 317
#define CMD_PRINT 318
#define CMD_DEVICE 319
#define CMD_HELP 320
#define CMD_WATCH 321
#define CMD_DISK 322
#define CMD_SYSTEM 323
#define CMD_QUIT 324
#define CMD_CHDIR 325
#define CMD_BANK 326
#define CMD_LOAD_LABELS 327
#define CMD_SAVE_LABELS 328
#define CMD_ADD_LABEL 329
#define CMD_DEL_LABEL 330
#define CMD_SHOW_LABELS 331
#define CMD_RECORD 332
#define CMD_MON_STOP 333
#define CMD_PLAYBACK 334
#define CMD_CHAR_DISPLAY 335
#define CMD_SPRITE_DISPLAY 336
#define CMD_TEXT_DISPLAY 337
#define CMD_SCREENCODE_DISPLAY 338
#define CMD_ENTER_DATA 339
#define CMD_ENTER_BIN_DATA 340
#define CMD_KEYBUF 341
#define CMD_BLOAD 342
#define CMD_BSAVE 343
#define CMD_SCREEN 344
#define CMD_UNTIL 345
#define CMD_CPU 346
#define CMD_YYDEBUG 347
#define CMD_BACKTRACE 348
#define CMD_SCREENSHOT 349
#define CMD_PWD 350
#define CMD_DIR 351
#define CMD_RESOURCE_GET 352
#define CMD_RESOURCE_SET 353
#define CMD_ATTACH 354
#define CMD_DETACH 355
#define CMD_MON_RESET 356
#define CMD_TAPECTRL 357
#define CMD_CARTFREEZE 358
#define CMD_CPUHISTORY 359
#define CMD_MEMMAPZAP 360
#define CMD_MEMMAPSHOW 361
#define CMD_MEMMAPSAVE 362
#define CMD_COMMENT 363
#define CMD_LIST 364
#define CMD_EXPORT 365
#define CMD_AUTOSTART 366
#define CMD_AUTOLOAD 367
#define CMD_LABEL_ASGN 368
#define L_PAREN 369
#define R_PAREN 370
#define ARG_IMMEDIATE 371
#define REG_A 372
#define REG_X 373
#define REG_Y 374
#define COMMA 375
#define INST_SEP 376
#define REG_B 377
#define REG_C 378
#define REG_D 379
#define REG_E 380
#define REG_H 381
#define REG_L 382
#define REG_AF 383
#define REG_BC 384
#define REG_DE 385
#define REG_HL 386
#define REG_IX 387
#define REG_IY 388
#define REG_SP 389
#define REG_IXH 390
#define REG_IXL 391
#define REG_IYH 392
#define REG_IYL 393
#define STRING 394
#define FILENAME 395
#define R_O_L 396
#define OPCODE 397
#define LABEL 398
#define BANKNAME 399
#define CPUTYPE 400
#define MON_REGISTER 401
#define COMPARE_OP 402
#define RADIX_TYPE 403
#define INPUT_SPEC 404
#define CMD_CHECKPT_ON 405
#define CMD_CHECKPT_OFF 406
#define TOGGLE 407
#define MASK 408




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 127 "mon_parse.y"

    MON_ADDR a;
    MON_ADDR range[2];
    int i;
    REG_ID reg;
    CONDITIONAL cond_op;
    cond_node_t *cond_node;
    RADIXTYPE rt;
    ACTION action;
    char *str;



/* Line 1676 of yacc.c  */
#line 372 "mon_parse.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


