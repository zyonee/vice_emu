/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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
     TRAIL = 277,
     CMD_SEP = 278,
     LABEL_ASGN_COMMENT = 279,
     CMD_SIDEFX = 280,
     CMD_RETURN = 281,
     CMD_BLOCK_READ = 282,
     CMD_BLOCK_WRITE = 283,
     CMD_UP = 284,
     CMD_DOWN = 285,
     CMD_LOAD = 286,
     CMD_SAVE = 287,
     CMD_VERIFY = 288,
     CMD_IGNORE = 289,
     CMD_HUNT = 290,
     CMD_FILL = 291,
     CMD_MOVE = 292,
     CMD_GOTO = 293,
     CMD_REGISTERS = 294,
     CMD_READSPACE = 295,
     CMD_WRITESPACE = 296,
     CMD_RADIX = 297,
     CMD_MEM_DISPLAY = 298,
     CMD_BREAK = 299,
     CMD_TRACE = 300,
     CMD_IO = 301,
     CMD_BRMON = 302,
     CMD_COMPARE = 303,
     CMD_DUMP = 304,
     CMD_UNDUMP = 305,
     CMD_EXIT = 306,
     CMD_DELETE = 307,
     CMD_CONDITION = 308,
     CMD_COMMAND = 309,
     CMD_ASSEMBLE = 310,
     CMD_DISASSEMBLE = 311,
     CMD_NEXT = 312,
     CMD_STEP = 313,
     CMD_PRINT = 314,
     CMD_DEVICE = 315,
     CMD_HELP = 316,
     CMD_WATCH = 317,
     CMD_DISK = 318,
     CMD_SYSTEM = 319,
     CMD_QUIT = 320,
     CMD_CHDIR = 321,
     CMD_BANK = 322,
     CMD_LOAD_LABELS = 323,
     CMD_SAVE_LABELS = 324,
     CMD_ADD_LABEL = 325,
     CMD_DEL_LABEL = 326,
     CMD_SHOW_LABELS = 327,
     CMD_RECORD = 328,
     CMD_MON_STOP = 329,
     CMD_PLAYBACK = 330,
     CMD_CHAR_DISPLAY = 331,
     CMD_SPRITE_DISPLAY = 332,
     CMD_TEXT_DISPLAY = 333,
     CMD_SCREENCODE_DISPLAY = 334,
     CMD_ENTER_DATA = 335,
     CMD_ENTER_BIN_DATA = 336,
     CMD_KEYBUF = 337,
     CMD_BLOAD = 338,
     CMD_BSAVE = 339,
     CMD_SCREEN = 340,
     CMD_UNTIL = 341,
     CMD_CPU = 342,
     CMD_YYDEBUG = 343,
     CMD_BACKTRACE = 344,
     CMD_SCREENSHOT = 345,
     CMD_PWD = 346,
     CMD_DIR = 347,
     CMD_RESOURCE_GET = 348,
     CMD_RESOURCE_SET = 349,
     CMD_ATTACH = 350,
     CMD_DETACH = 351,
     CMD_MON_RESET = 352,
     CMD_TAPECTRL = 353,
     CMD_CARTFREEZE = 354,
     CMD_CPUHISTORY = 355,
     CMD_MEMMAPZAP = 356,
     CMD_MEMMAPSHOW = 357,
     CMD_MEMMAPSAVE = 358,
     CMD_LABEL_ASGN = 359,
     L_PAREN = 360,
     R_PAREN = 361,
     ARG_IMMEDIATE = 362,
     REG_A = 363,
     REG_X = 364,
     REG_Y = 365,
     COMMA = 366,
     INST_SEP = 367,
     REG_B = 368,
     REG_C = 369,
     REG_D = 370,
     REG_E = 371,
     REG_H = 372,
     REG_L = 373,
     REG_AF = 374,
     REG_BC = 375,
     REG_DE = 376,
     REG_HL = 377,
     REG_IX = 378,
     REG_IY = 379,
     REG_SP = 380,
     REG_IXH = 381,
     REG_IXL = 382,
     REG_IYH = 383,
     REG_IYL = 384,
     STRING = 385,
     FILENAME = 386,
     R_O_L = 387,
     OPCODE = 388,
     LABEL = 389,
     BANKNAME = 390,
     CPUTYPE = 391,
     MON_REGISTER = 392,
     COMPARE_OP = 393,
     RADIX_TYPE = 394,
     INPUT_SPEC = 395,
     CMD_CHECKPT_ON = 396,
     CMD_CHECKPT_OFF = 397,
     TOGGLE = 398,
     MASK = 399
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
#define TRAIL 277
#define CMD_SEP 278
#define LABEL_ASGN_COMMENT 279
#define CMD_SIDEFX 280
#define CMD_RETURN 281
#define CMD_BLOCK_READ 282
#define CMD_BLOCK_WRITE 283
#define CMD_UP 284
#define CMD_DOWN 285
#define CMD_LOAD 286
#define CMD_SAVE 287
#define CMD_VERIFY 288
#define CMD_IGNORE 289
#define CMD_HUNT 290
#define CMD_FILL 291
#define CMD_MOVE 292
#define CMD_GOTO 293
#define CMD_REGISTERS 294
#define CMD_READSPACE 295
#define CMD_WRITESPACE 296
#define CMD_RADIX 297
#define CMD_MEM_DISPLAY 298
#define CMD_BREAK 299
#define CMD_TRACE 300
#define CMD_IO 301
#define CMD_BRMON 302
#define CMD_COMPARE 303
#define CMD_DUMP 304
#define CMD_UNDUMP 305
#define CMD_EXIT 306
#define CMD_DELETE 307
#define CMD_CONDITION 308
#define CMD_COMMAND 309
#define CMD_ASSEMBLE 310
#define CMD_DISASSEMBLE 311
#define CMD_NEXT 312
#define CMD_STEP 313
#define CMD_PRINT 314
#define CMD_DEVICE 315
#define CMD_HELP 316
#define CMD_WATCH 317
#define CMD_DISK 318
#define CMD_SYSTEM 319
#define CMD_QUIT 320
#define CMD_CHDIR 321
#define CMD_BANK 322
#define CMD_LOAD_LABELS 323
#define CMD_SAVE_LABELS 324
#define CMD_ADD_LABEL 325
#define CMD_DEL_LABEL 326
#define CMD_SHOW_LABELS 327
#define CMD_RECORD 328
#define CMD_MON_STOP 329
#define CMD_PLAYBACK 330
#define CMD_CHAR_DISPLAY 331
#define CMD_SPRITE_DISPLAY 332
#define CMD_TEXT_DISPLAY 333
#define CMD_SCREENCODE_DISPLAY 334
#define CMD_ENTER_DATA 335
#define CMD_ENTER_BIN_DATA 336
#define CMD_KEYBUF 337
#define CMD_BLOAD 338
#define CMD_BSAVE 339
#define CMD_SCREEN 340
#define CMD_UNTIL 341
#define CMD_CPU 342
#define CMD_YYDEBUG 343
#define CMD_BACKTRACE 344
#define CMD_SCREENSHOT 345
#define CMD_PWD 346
#define CMD_DIR 347
#define CMD_RESOURCE_GET 348
#define CMD_RESOURCE_SET 349
#define CMD_ATTACH 350
#define CMD_DETACH 351
#define CMD_MON_RESET 352
#define CMD_TAPECTRL 353
#define CMD_CARTFREEZE 354
#define CMD_CPUHISTORY 355
#define CMD_MEMMAPZAP 356
#define CMD_MEMMAPSHOW 357
#define CMD_MEMMAPSAVE 358
#define CMD_LABEL_ASGN 359
#define L_PAREN 360
#define R_PAREN 361
#define ARG_IMMEDIATE 362
#define REG_A 363
#define REG_X 364
#define REG_Y 365
#define COMMA 366
#define INST_SEP 367
#define REG_B 368
#define REG_C 369
#define REG_D 370
#define REG_E 371
#define REG_H 372
#define REG_L 373
#define REG_AF 374
#define REG_BC 375
#define REG_DE 376
#define REG_HL 377
#define REG_IX 378
#define REG_IY 379
#define REG_SP 380
#define REG_IXH 381
#define REG_IXL 382
#define REG_IYH 383
#define REG_IYL 384
#define STRING 385
#define FILENAME 386
#define R_O_L 387
#define OPCODE 388
#define LABEL 389
#define BANKNAME 390
#define CPUTYPE 391
#define MON_REGISTER 392
#define COMPARE_OP 393
#define RADIX_TYPE 394
#define INPUT_SPEC 395
#define CMD_CHECKPT_ON 396
#define CMD_CHECKPT_OFF 397
#define TOGGLE 398
#define MASK 399




/* Copy the first part of user declarations.  */
#line 1 "mon_parse.y"

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

#include "vice.h"

#ifndef MINIXVMD
#ifdef __GNUC__
#undef alloca
#define        alloca(n)       __builtin_alloca (n)
#else
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#else  /* Not HAVE_ALLOCA_H.  */
#if !defined(_AIX) && !defined(WINCE)
#ifndef _MSC_VER
extern char *alloca();
#else
#define alloca(n)   _alloca(n)
#endif  /* MSVC */
#endif /* Not AIX and not WINCE.  */
#endif /* HAVE_ALLOCA_H.  */
#endif /* GCC.  */
#endif /* MINIXVMD */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "asm.h"
#include "console.h"
#include "lib.h"
#include "machine.h"
#include "mon_breakpoint.h"
#include "mon_command.h"
#include "mon_disassemble.h"
#include "mon_drive.h"
#include "mon_file.h"
#include "mon_memory.h"
#include "mon_util.h"
#include "montypes.h"
#include "types.h"
#include "uimon.h"


#define join_ints(x,y) (LO16_TO_HI16(x)|y)
#define separate_int1(x) (HI16_TO_LO16(x))
#define separate_int2(x) (LO16(x))

static int yyerror(char *s);
static int temp;
static int resolve_datatype(unsigned guess_type, const char *num);
static int resolve_range(enum t_memspace memspace, MON_ADDR range[2], 
                         const char *num);
static void mon_quit(void);

#ifdef __IBMC__
static void __yy_memcpy (char *to, char *from, int count);
#endif

/* Defined in the lexer */
extern int new_cmd, opt_asm;
extern void free_buffer(void);
extern void make_buffer(char *str);
extern int yylex(void);
extern int cur_len, last_len;

#define ERR_ILLEGAL_INPUT 1     /* Generic error as returned by yacc.  */
#define ERR_RANGE_BAD_START 2
#define ERR_RANGE_BAD_END 3
#define ERR_BAD_CMD 4
#define ERR_EXPECT_BRKNUM 5
#define ERR_EXPECT_END_CMD 6
#define ERR_MISSING_CLOSE_PAREN 7
#define ERR_INCOMPLETE_COMPARE_OP 8
#define ERR_EXPECT_FILENAME 9
#define ERR_ADDR_TOO_BIG 10
#define ERR_IMM_TOO_BIG 11
#define ERR_EXPECT_STRING 12
#define ERR_UNDEFINED_LABEL 13
#define ERR_EXPECT_DEVICE_NUM 14
#define ERR_EXPECT_ADDRESS 15

#define BAD_ADDR (new_addr(e_invalid_space, 0))
#define CHECK_ADDR(x) ((x) == addr_mask(x))

#define YYDEBUG 1



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 118 "mon_parse.y"
{
    MON_ADDR a;
    MON_ADDR range[2];
    int i;
    REG_ID reg;
    CONDITIONAL cond_op;
    cond_node_t *cond_node;
    RADIXTYPE rt;
    ACTION action;
    char *str;
}
/* Line 193 of yacc.c.  */
#line 513 "mon_parse.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 526 "mon_parse.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  281
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1599

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  151
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  261
/* YYNRULES -- Number of states.  */
#define YYNSTATES  514

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   399

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     149,   150,   147,   145,     2,   146,     2,   148,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    15,    17,    19,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    47,    50,    54,    58,    64,    68,    71,
      74,    78,    81,    86,    89,    93,    97,   100,   105,   108,
     113,   116,   121,   124,   129,   132,   134,   137,   141,   145,
     151,   155,   161,   165,   171,   175,   181,   185,   188,   193,
     199,   200,   206,   210,   214,   217,   223,   229,   235,   241,
     247,   251,   254,   258,   261,   265,   268,   272,   275,   279,
     282,   285,   288,   293,   299,   305,   309,   313,   319,   325,
     329,   332,   335,   338,   341,   345,   349,   353,   359,   363,
     366,   372,   378,   383,   387,   390,   394,   397,   401,   404,
     407,   411,   415,   418,   422,   426,   430,   434,   438,   441,
     445,   448,   452,   458,   462,   467,   470,   475,   480,   483,
     489,   495,   501,   505,   510,   516,   521,   527,   532,   538,
     544,   549,   553,   557,   560,   564,   569,   572,   575,   577,
     579,   580,   582,   584,   586,   588,   590,   591,   593,   596,
     600,   602,   606,   608,   610,   612,   614,   618,   620,   624,
     627,   628,   630,   634,   636,   638,   639,   641,   643,   645,
     647,   649,   651,   653,   657,   661,   665,   669,   673,   677,
     679,   683,   687,   691,   695,   697,   699,   701,   705,   707,
     709,   711,   714,   716,   718,   720,   722,   724,   726,   728,
     730,   732,   734,   736,   738,   740,   742,   744,   746,   748,
     750,   754,   758,   761,   764,   766,   768,   771,   773,   777,
     781,   785,   791,   797,   801,   805,   809,   813,   817,   821,
     827,   833,   839,   845,   846,   848,   850,   852,   854,   856,
     858,   860,   862,   864,   866,   868,   870,   872,   874,   876,
     878,   880
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     152,     0,    -1,   153,    -1,   199,    22,    -1,    22,    -1,
     155,    -1,   153,   155,    -1,    23,    -1,    22,    -1,     1,
      -1,   156,    -1,   158,    -1,   161,    -1,   159,    -1,   162,
      -1,   163,    -1,   164,    -1,   165,    -1,   166,    -1,   167,
      -1,   168,    -1,   169,    -1,    13,    -1,    67,   154,    -1,
      67,   184,   154,    -1,    67,   135,   154,    -1,    67,   184,
     183,   135,   154,    -1,    38,   182,   154,    -1,    46,   154,
      -1,    87,   154,    -1,    87,   136,   154,    -1,   100,   154,
      -1,   100,   183,   187,   154,    -1,    26,   154,    -1,    49,
     172,   154,    -1,    50,   172,   154,    -1,    58,   154,    -1,
      58,   183,   187,   154,    -1,    57,   154,    -1,    57,   183,
     187,   154,    -1,    29,   154,    -1,    29,   183,   187,   154,
      -1,    30,   154,    -1,    30,   183,   187,   154,    -1,    85,
     154,    -1,   157,    -1,    39,   154,    -1,    39,   184,   154,
      -1,    39,   176,   154,    -1,    68,   184,   183,   172,   154,
      -1,    68,   172,   154,    -1,    69,   184,   183,   172,   154,
      -1,    69,   172,   154,    -1,    70,   182,   183,   134,   154,
      -1,    71,   134,   154,    -1,    71,   184,   183,   134,   154,
      -1,    72,   184,   154,    -1,    72,   154,    -1,   104,    21,
     182,   154,    -1,   104,    21,   182,    24,   154,    -1,    -1,
      55,   182,   160,   200,   154,    -1,    55,   182,   154,    -1,
      56,   179,   154,    -1,    56,   154,    -1,    37,   180,   183,
     182,   154,    -1,    48,   180,   183,   182,   154,    -1,    36,
     180,   183,   190,   154,    -1,    35,   180,   183,   192,   154,
      -1,    43,   139,   183,   179,   154,    -1,    43,   179,   154,
      -1,    43,   154,    -1,    76,   179,   154,    -1,    76,   154,
      -1,    77,   179,   154,    -1,    77,   154,    -1,    78,   179,
     154,    -1,    78,   154,    -1,    79,   179,   154,    -1,    79,
     154,    -1,   101,   154,    -1,   102,   154,    -1,   102,   183,
     187,   154,    -1,   102,   183,   187,   179,   154,    -1,   103,
     172,   183,   187,   154,    -1,    44,   179,   154,    -1,    86,
     179,   154,    -1,    44,   179,    15,   188,   154,    -1,    62,
     174,   183,   179,   154,    -1,    45,   179,   154,    -1,    44,
     154,    -1,    86,   154,    -1,    45,   154,    -1,    62,   154,
      -1,   141,   178,   154,    -1,   142,   178,   154,    -1,    34,
     178,   154,    -1,    34,   178,   183,   187,   154,    -1,    52,
     178,   154,    -1,    52,   154,    -1,    53,   178,    15,   188,
     154,    -1,    54,   178,   183,   130,   154,    -1,    54,   178,
       1,   154,    -1,    25,   143,   154,    -1,    25,   154,    -1,
      42,   139,   154,    -1,    42,   154,    -1,    60,   184,   154,
      -1,    65,   154,    -1,    51,   154,    -1,    63,   170,   154,
      -1,    59,   187,   154,    -1,    61,   154,    -1,    61,   170,
     154,    -1,    64,   170,   154,    -1,     7,   187,   154,    -1,
      66,   170,   154,    -1,    82,   170,   154,    -1,    89,   154,
      -1,    92,   171,   154,    -1,    91,   154,    -1,    90,   172,
     154,    -1,    90,   172,   183,   187,   154,    -1,    93,   130,
     154,    -1,    94,   130,   130,   154,    -1,    97,   154,    -1,
      97,   183,   187,   154,    -1,    98,   183,   187,   154,    -1,
      99,   154,    -1,    31,   172,   173,   181,   154,    -1,    83,
     172,   173,   181,   154,    -1,    32,   172,   173,   180,   154,
      -1,    32,   172,     1,    -1,    32,   172,   173,     1,    -1,
      84,   172,   173,   180,   154,    -1,    84,   172,   173,     1,
      -1,    33,   172,   173,   182,   154,    -1,    33,   172,   173,
       1,    -1,    27,   187,   187,   181,   154,    -1,    28,   187,
     187,   182,   154,    -1,    95,   172,   187,   154,    -1,    96,
     187,   154,    -1,    73,   172,   154,    -1,    74,   154,    -1,
      75,   172,   154,    -1,    80,   182,   190,   154,    -1,    81,
     154,    -1,    88,   154,    -1,   132,    -1,   132,    -1,    -1,
     131,    -1,     1,    -1,   187,    -1,     1,    -1,    14,    -1,
      -1,   137,    -1,   184,   137,    -1,   176,   111,   177,    -1,
     177,    -1,   175,    21,   197,    -1,   195,    -1,     1,    -1,
     180,    -1,   182,    -1,   182,   183,   182,    -1,     9,    -1,
     184,   183,     9,    -1,   183,   182,    -1,    -1,   185,    -1,
     184,   183,   185,    -1,   134,    -1,   111,    -1,    -1,    16,
      -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,   186,
      -1,   197,    -1,   187,   145,   187,    -1,   187,   146,   187,
      -1,   187,   147,   187,    -1,   187,   148,   187,    -1,   149,
     187,   150,    -1,   149,   187,     1,    -1,   194,    -1,   188,
     138,   188,    -1,   188,   138,     1,    -1,   105,   188,   106,
      -1,   105,   188,     1,    -1,   189,    -1,   175,    -1,   197,
      -1,   190,   183,   191,    -1,   191,    -1,   197,    -1,   130,
      -1,   192,   193,    -1,   193,    -1,   197,    -1,   144,    -1,
     130,    -1,   197,    -1,   175,    -1,     4,    -1,    12,    -1,
      11,    -1,    10,    -1,    12,    -1,    11,    -1,    10,    -1,
       3,    -1,     4,    -1,     5,    -1,     6,    -1,   196,    -1,
     198,   112,   199,    -1,   199,   112,   199,    -1,   199,   112,
      -1,   133,   201,    -1,   199,    -1,   198,    -1,   107,   197,
      -1,   197,    -1,   197,   111,   109,    -1,   197,   111,   110,
      -1,   105,   197,   106,    -1,   105,   197,   111,   109,   106,
      -1,   105,   197,   106,   111,   110,    -1,   105,   120,   106,
      -1,   105,   121,   106,    -1,   105,   122,   106,    -1,   105,
     123,   106,    -1,   105,   124,   106,    -1,   105,   125,   106,
      -1,   105,   197,   106,   111,   108,    -1,   105,   197,   106,
     111,   122,    -1,   105,   197,   106,   111,   123,    -1,   105,
     197,   106,   111,   124,    -1,    -1,   108,    -1,   113,    -1,
     114,    -1,   115,    -1,   116,    -1,   117,    -1,   126,    -1,
     128,    -1,   118,    -1,   127,    -1,   129,    -1,   119,    -1,
     120,    -1,   121,    -1,   122,    -1,   123,    -1,   124,    -1,
     125,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   175,   175,   176,   177,   180,   181,   184,   185,   186,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   204,   206,   208,   210,   212,   214,   216,
     218,   220,   222,   224,   226,   228,   230,   232,   234,   236,
     238,   240,   242,   244,   246,   248,   251,   253,   255,   258,
     263,   268,   270,   272,   274,   276,   278,   280,   282,   286,
     293,   292,   295,   297,   299,   303,   305,   307,   309,   311,
     313,   315,   317,   319,   321,   323,   325,   327,   329,   331,
     333,   335,   337,   339,   341,   345,   350,   355,   361,   367,
     372,   374,   376,   378,   383,   385,   387,   389,   391,   393,
     395,   397,   399,   403,   405,   410,   412,   430,   432,   434,
     438,   440,   442,   444,   446,   448,   450,   452,   454,   456,
     458,   460,   462,   464,   466,   468,   470,   472,   474,   479,
     481,   483,   485,   487,   489,   491,   493,   495,   497,   499,
     501,   503,   507,   509,   511,   515,   517,   521,   525,   528,
     529,   532,   533,   536,   537,   540,   541,   544,   545,   548,
     549,   552,   556,   557,   560,   561,   564,   565,   567,   571,
     572,   575,   580,   585,   595,   596,   599,   600,   601,   602,
     603,   606,   608,   610,   611,   612,   613,   614,   615,   616,
     619,   624,   626,   628,   630,   634,   640,   648,   649,   652,
     653,   656,   657,   660,   661,   662,   665,   666,   669,   670,
     671,   672,   675,   676,   677,   680,   681,   682,   683,   684,
     687,   688,   689,   692,   702,   703,   706,   710,   715,   720,
     725,   727,   729,   731,   732,   733,   734,   735,   736,   737,
     739,   741,   743,   745,   746,   747,   748,   749,   750,   751,
     752,   753,   754,   755,   756,   757,   758,   759,   760,   761,
     762,   763
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "H_NUMBER", "D_NUMBER", "O_NUMBER",
  "B_NUMBER", "CONVERT_OP", "B_DATA", "H_RANGE_GUESS", "D_NUMBER_GUESS",
  "O_NUMBER_GUESS", "B_NUMBER_GUESS", "BAD_CMD", "MEM_OP", "IF",
  "MEM_COMP", "MEM_DISK8", "MEM_DISK9", "MEM_DISK10", "MEM_DISK11",
  "EQUALS", "TRAIL", "CMD_SEP", "LABEL_ASGN_COMMENT", "CMD_SIDEFX",
  "CMD_RETURN", "CMD_BLOCK_READ", "CMD_BLOCK_WRITE", "CMD_UP", "CMD_DOWN",
  "CMD_LOAD", "CMD_SAVE", "CMD_VERIFY", "CMD_IGNORE", "CMD_HUNT",
  "CMD_FILL", "CMD_MOVE", "CMD_GOTO", "CMD_REGISTERS", "CMD_READSPACE",
  "CMD_WRITESPACE", "CMD_RADIX", "CMD_MEM_DISPLAY", "CMD_BREAK",
  "CMD_TRACE", "CMD_IO", "CMD_BRMON", "CMD_COMPARE", "CMD_DUMP",
  "CMD_UNDUMP", "CMD_EXIT", "CMD_DELETE", "CMD_CONDITION", "CMD_COMMAND",
  "CMD_ASSEMBLE", "CMD_DISASSEMBLE", "CMD_NEXT", "CMD_STEP", "CMD_PRINT",
  "CMD_DEVICE", "CMD_HELP", "CMD_WATCH", "CMD_DISK", "CMD_SYSTEM",
  "CMD_QUIT", "CMD_CHDIR", "CMD_BANK", "CMD_LOAD_LABELS",
  "CMD_SAVE_LABELS", "CMD_ADD_LABEL", "CMD_DEL_LABEL", "CMD_SHOW_LABELS",
  "CMD_RECORD", "CMD_MON_STOP", "CMD_PLAYBACK", "CMD_CHAR_DISPLAY",
  "CMD_SPRITE_DISPLAY", "CMD_TEXT_DISPLAY", "CMD_SCREENCODE_DISPLAY",
  "CMD_ENTER_DATA", "CMD_ENTER_BIN_DATA", "CMD_KEYBUF", "CMD_BLOAD",
  "CMD_BSAVE", "CMD_SCREEN", "CMD_UNTIL", "CMD_CPU", "CMD_YYDEBUG",
  "CMD_BACKTRACE", "CMD_SCREENSHOT", "CMD_PWD", "CMD_DIR",
  "CMD_RESOURCE_GET", "CMD_RESOURCE_SET", "CMD_ATTACH", "CMD_DETACH",
  "CMD_MON_RESET", "CMD_TAPECTRL", "CMD_CARTFREEZE", "CMD_CPUHISTORY",
  "CMD_MEMMAPZAP", "CMD_MEMMAPSHOW", "CMD_MEMMAPSAVE", "CMD_LABEL_ASGN",
  "L_PAREN", "R_PAREN", "ARG_IMMEDIATE", "REG_A", "REG_X", "REG_Y",
  "COMMA", "INST_SEP", "REG_B", "REG_C", "REG_D", "REG_E", "REG_H",
  "REG_L", "REG_AF", "REG_BC", "REG_DE", "REG_HL", "REG_IX", "REG_IY",
  "REG_SP", "REG_IXH", "REG_IXL", "REG_IYH", "REG_IYL", "STRING",
  "FILENAME", "R_O_L", "OPCODE", "LABEL", "BANKNAME", "CPUTYPE",
  "MON_REGISTER", "COMPARE_OP", "RADIX_TYPE", "INPUT_SPEC",
  "CMD_CHECKPT_ON", "CMD_CHECKPT_OFF", "TOGGLE", "MASK", "'+'", "'-'",
  "'*'", "'/'", "'('", "')'", "$accept", "top_level", "command_list",
  "end_cmd", "command", "machine_state_rules", "register_mod",
  "symbol_table_rules", "asm_rules", "@1", "memory_rules",
  "checkpoint_rules", "checkpoint_control_rules", "monitor_state_rules",
  "monitor_misc_rules", "disk_rules", "cmd_file_rules", "data_entry_rules",
  "monitor_debug_rules", "rest_of_line", "opt_rest_of_line", "filename",
  "device_num", "opt_mem_op", "register", "reg_list", "reg_asgn",
  "breakpt_num", "address_opt_range", "address_range", "opt_address",
  "address", "opt_sep", "memspace", "memloc", "memaddr", "expression",
  "cond_expr", "compare_operand", "data_list", "data_element", "hunt_list",
  "hunt_element", "value", "d_number", "guess_default", "number",
  "assembly_instr_list", "assembly_instruction", "post_assemble",
  "asm_operand_mode", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,    43,    45,    42,    47,    40,
      41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   151,   152,   152,   152,   153,   153,   154,   154,   154,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   157,   157,   157,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     160,   159,   159,   159,   159,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   164,   164,   164,   164,   164,   164,   164,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   167,   167,   167,   168,   168,   169,   170,   171,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   178,   178,   179,   179,   180,   180,   180,   181,
     181,   182,   182,   182,   183,   183,   184,   184,   184,   184,
     184,   185,   186,   187,   187,   187,   187,   187,   187,   187,
     188,   188,   188,   188,   188,   189,   189,   190,   190,   191,
     191,   192,   192,   193,   193,   193,   194,   194,   195,   195,
     195,   195,   196,   196,   196,   197,   197,   197,   197,   197,
     198,   198,   198,   199,   200,   200,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   201
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     3,     5,     3,     2,     2,
       3,     2,     4,     2,     3,     3,     2,     4,     2,     4,
       2,     4,     2,     4,     2,     1,     2,     3,     3,     5,
       3,     5,     3,     5,     3,     5,     3,     2,     4,     5,
       0,     5,     3,     3,     2,     5,     5,     5,     5,     5,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       2,     2,     4,     5,     5,     3,     3,     5,     5,     3,
       2,     2,     2,     2,     3,     3,     3,     5,     3,     2,
       5,     5,     4,     3,     2,     3,     2,     3,     2,     2,
       3,     3,     2,     3,     3,     3,     3,     3,     2,     3,
       2,     3,     5,     3,     4,     2,     4,     4,     2,     5,
       5,     5,     3,     4,     5,     4,     5,     4,     5,     5,
       4,     3,     3,     2,     3,     4,     2,     2,     1,     1,
       0,     1,     1,     1,     1,     1,     0,     1,     2,     3,
       1,     3,     1,     1,     1,     1,     3,     1,     3,     2,
       0,     1,     3,     1,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     1,     1,     3,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     2,     1,     1,     2,     1,     3,     3,
       3,     5,     5,     3,     3,     3,     3,     3,     3,     5,
       5,     5,     5,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,    22,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   150,     0,
       0,     0,     0,     0,   175,     0,     0,     0,     0,     0,
       0,   243,     0,     0,     0,     2,     5,    10,    45,    11,
      13,    12,    14,    15,    16,    17,    18,    19,    20,    21,
       0,   215,   216,   217,   218,   214,   213,   212,   176,   177,
     178,   179,   180,   157,     0,   207,     0,     0,   189,   219,
     206,     9,     8,     7,     0,   104,    33,     0,     0,   174,
      40,     0,    42,     0,   152,   151,     0,     0,     0,   163,
     208,   211,   210,   209,     0,   162,   167,   173,   175,   175,
     175,   171,   181,   182,   175,   175,     0,   175,    46,     0,
       0,   160,     0,     0,   106,   175,    71,     0,   164,   175,
      90,     0,    92,     0,    28,   175,     0,     0,   109,     9,
      99,     0,     0,     0,     0,    64,     0,    38,     0,    36,
       0,     0,     0,   148,   112,     0,   155,    93,   175,     0,
       0,   108,     0,     0,    23,     0,     0,   175,     0,   175,
     175,     0,   175,    57,     0,     0,   143,     0,    73,     0,
      75,     0,    77,     0,    79,     0,     0,   146,     0,     0,
       0,    44,    91,     0,     0,    29,   147,   118,     0,   120,
     149,     0,     0,     0,     0,     0,   125,     0,     0,   128,
      31,     0,    80,    81,     0,   175,     0,     0,     0,   244,
     245,   246,   247,   248,   249,   252,   255,   256,   257,   258,
     259,   260,   261,   250,   253,   251,   254,   227,   223,     0,
       0,     1,     6,     3,     0,   158,     0,     0,     0,     0,
     115,   103,   175,     0,     0,     0,   154,   175,   153,   132,
       0,     0,    96,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,    48,    47,   105,     0,    70,     0,    85,
      89,     0,    34,    35,    98,     0,     0,     0,    62,     0,
      63,     0,     0,   111,   107,   113,     0,   110,   114,   116,
      25,    24,     0,    50,     0,    52,     0,     0,    54,     0,
      56,   142,   144,    72,    74,    76,    78,   200,     0,   198,
     199,   117,   175,     0,    86,    30,   121,     0,   119,   123,
       0,     0,   141,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   226,     0,    94,    95,
     188,   187,   183,   184,   185,   186,     0,     0,     0,    41,
      43,     0,   133,     0,   137,     0,     0,   205,   204,     0,
     202,   203,   166,   168,   172,     0,     0,   161,   159,     0,
       0,   195,     0,   194,   196,     0,     0,   102,     0,   225,
     224,     0,    39,    37,     0,     0,     0,     0,     0,     0,
     145,     0,     0,   135,     0,     0,   124,   140,   126,   127,
      32,    82,     0,     0,     0,    58,   233,   234,   235,   236,
     237,   238,   230,     0,   228,   229,   138,   169,   139,   129,
     131,   136,    97,    68,   201,    67,    65,    69,     0,     0,
      87,    66,   100,   101,     0,   222,    61,    88,    26,    49,
      51,    53,    55,   197,   130,   134,   122,    83,    84,    59,
       0,     0,   193,   192,   191,   190,   220,   221,   239,   232,
     240,   241,   242,   231
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    84,    85,   125,    86,    87,    88,    89,    90,   329,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   195,
     241,   136,   297,   198,   115,   160,   161,   144,   167,   168,
     396,   169,   397,   116,   151,   152,   298,   422,   423,   358,
     359,   409,   410,   118,   145,   119,   153,   429,   100,   431,
     278
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -322
static const yytype_int16 yypact[] =
{
    1110,   833,  -322,  -322,    17,   229,   833,   833,   257,   257,
      30,    30,    30,   496,  1444,  1444,  1444,  1465,   222,    54,
     968,  1081,  1081,   229,  1444,    30,    30,   229,   897,   496,
     496,  1465,  1081,   257,   257,   833,   564,    92,  1362,  -102,
    -102,   229,  -102,   541,   380,   380,  1465,   178,   394,    30,
     229,    30,  1081,  1081,  1081,  1081,  1465,   229,  -102,    30,
      30,   229,  1081,   130,   229,   229,    30,   229,   -89,   -76,
     -70,    30,   833,   257,   -63,   229,   257,   229,   257,    30,
      48,   939,   496,   496,    62,  1228,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
      56,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,   833,  -322,   -53,    72,  -322,  -322,
    -322,  -322,  -322,  -322,   229,  -322,  -322,   811,   811,  -322,
    -322,   833,  -322,   833,  -322,  -322,   425,   628,   425,  -322,
    -322,  -322,  -322,  -322,   257,  -322,  -322,  -322,   -63,   -63,
     -63,  -322,  -322,  -322,   -63,   -63,   229,   -63,  -322,    65,
     185,  -322,    84,   229,  -322,   -63,  -322,   229,  -322,   282,
    -322,   124,  -322,   229,  -322,   -63,   229,   229,  -322,   232,
    -322,   229,    76,    11,    80,  -322,   229,  -322,   833,  -322,
     833,    72,   229,  -322,  -322,   229,  -322,  -322,   -63,   229,
     229,  -322,   229,   229,  -322,   187,   229,   -63,   229,   -63,
     -63,   229,   -63,  -322,   229,   229,  -322,   229,  -322,   229,
    -322,   229,  -322,   229,  -322,   229,   609,  -322,   229,   425,
     425,  -322,  -322,   229,   229,  -322,  -322,  -322,   257,  -322,
    -322,   229,   229,   -38,   833,    72,  -322,   833,   833,  -322,
    -322,   833,  -322,  -322,   833,   -63,  1465,   651,   686,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,   -15,  -322,   229,
     229,  -322,  -322,  -322,    34,  -322,   833,   833,   833,   833,
    -322,  -322,    89,   862,    72,    72,  -322,   291,   -27,  1220,
    1385,  1426,  -322,   833,    41,  1465,   661,   609,  1465,  -322,
     686,   686,   404,  -322,  -322,  -322,  1444,  -322,  1330,  -322,
    -322,  1465,  -322,  -322,  -322,  1330,   229,   -25,  -322,   -23,
    -322,    72,    72,  -322,  -322,  -322,  1444,  -322,  -322,  -322,
    -322,  -322,   -19,  -322,    30,  -322,    30,   -26,  -322,   -11,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,   882,  -322,
    -322,  -322,   291,  1405,  -322,  -322,  -322,   833,  -322,  -322,
     229,    72,  -322,    72,    72,    72,   787,   833,   368,    22,
      23,    39,    44,    49,    51,   -82,  -322,   -72,  -322,  -322,
    -322,  -322,   -73,   -73,  -322,  -322,   229,  1465,   229,  -322,
    -322,   229,  -322,   229,  -322,   229,    72,  -322,  -322,    10,
    -322,  -322,  -322,  -322,  -322,   882,   229,  -322,  -322,   229,
    1330,  -322,    78,  -322,  -322,   229,    78,  -322,   229,    50,
      52,   229,  -322,  -322,   229,   229,   229,   229,   229,   229,
    -322,   609,   229,  -322,   229,    72,  -322,  -322,  -322,  -322,
    -322,  -322,   229,    72,   229,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,    57,    58,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,    18,  1020,
    -322,  -322,  -322,  -322,   -23,   -23,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
     366,    64,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -322,  -322,  -322,   310,    87,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,   242,
    -322,    38,   -96,  -322,   -17,  -322,  -134,   421,     4,    -7,
    -274,   159,    -6,   555,  -245,  -322,   652,  -321,  -322,  -116,
    -239,  -322,  -198,  -322,  -322,  -322,    -1,  -322,  -319,  -322,
    -322
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -176
static const yytype_int16 yytable[] =
{
     120,   159,   131,   133,   426,   120,   120,   148,   154,   155,
     430,   121,   326,   101,   102,   103,   104,   175,   121,   502,
     105,   106,   107,   401,   462,   171,   173,   188,   190,   463,
     193,   134,   122,   123,   120,   390,   186,   464,   465,   122,
     123,   300,   301,   240,   101,   102,   103,   104,   129,   137,
     138,   105,   106,   107,   242,   121,   219,   221,   223,   225,
     243,   414,   281,   176,   177,   414,   233,   247,   248,   256,
     251,   120,   254,   121,   288,   289,   122,   123,   283,   121,
     277,   121,   206,   208,   285,   121,   311,   215,   442,   217,
    -170,   325,   370,   121,   122,   123,   387,   229,   230,   478,
     122,   123,   122,   123,   238,   428,   122,   123,   438,   244,
      81,  -170,  -170,   120,   122,   123,   435,   255,   286,   287,
     288,   289,   129,   439,   503,   121,   120,   120,   456,   457,
     120,   121,   120,   362,   363,   120,   120,   120,   303,   318,
     407,  -175,   304,   305,   306,   458,   122,   123,   307,   308,
     459,   310,   122,   123,   408,   460,   479,   461,   505,   316,
     124,   135,   484,   305,   485,   506,   507,   501,   500,   321,
     513,   407,   282,   149,   149,   149,   156,   327,   418,   286,
     287,   288,   289,   149,   391,   408,   121,   120,   121,   120,
     184,   415,   336,   163,   108,   109,   110,   111,   112,   342,
     129,   344,   493,   346,   347,   210,   349,   122,   123,   122,
     123,   474,     0,   -60,     0,   226,   479,   286,   287,   288,
     289,   285,     0,   121,   193,   360,     0,     0,   120,   120,
     121,     0,   367,  -163,   286,   287,   288,   289,   108,   109,
     110,   111,   112,   120,   122,   123,   120,   120,     0,   377,
     120,   122,   123,   120,  -163,  -163,   385,   386,   121,     0,
    -175,  -175,  -175,  -175,     0,     0,   234,  -175,  -175,  -175,
       0,     0,     0,  -175,  -175,  -175,  -175,  -175,     0,   122,
     123,   199,   200,  -165,   202,   120,   120,   120,   120,     0,
       0,     0,  -170,   403,     0,   159,   312,  -165,   129,     0,
     228,   421,   120,   411,  -165,  -165,   360,     0,   421,     0,
     417,     0,   211,  -170,  -170,   126,     0,   424,   130,   132,
     419,     0,  -175,     0,   424,     0,     0,     0,   158,   164,
     166,   170,   172,   174,     0,     0,     0,   178,   180,     0,
     434,     0,   185,   187,   189,     0,     0,   194,   197,     0,
       0,   201,   441,   204,     0,     0,   444,     0,   213,   113,
     216,     0,   218,   220,   222,   224,   120,   227,   129,   121,
       0,   231,   232,   235,   236,   237,   120,   239,     0,     0,
     452,   134,   436,   246,   437,   249,   250,   252,   253,     0,
     122,   123,   454,   129,  -175,   121,   108,   109,   110,   111,
     112,     0,   129,   421,     0,     0,  -175,     0,   411,   441,
     108,   109,   110,   111,   112,   378,   122,   123,     0,   424,
     108,   109,   110,   111,   112,     0,   296,   290,   101,   102,
     103,   104,     0,     0,   291,   105,   106,   107,     0,     0,
     360,   108,   109,   110,   111,   112,     0,     0,     0,   181,
     182,   183,   398,     0,   302,     0,     0,     0,     0,   149,
     405,     0,   421,     0,   412,     0,   309,   416,     0,     0,
     313,     0,   314,   315,   508,     0,   509,   317,   424,     0,
     425,   319,     0,   320,     0,     0,   322,   323,   510,   511,
     512,   324,     0,     0,   328,     0,   330,   139,     0,     0,
     140,   333,   334,   279,   280,   335,   141,   142,   143,   337,
     338,   135,   339,   340,     0,   341,   343,     0,   345,     0,
       0,   348,   149,     0,   350,   351,     0,   352,     0,   353,
       0,   354,     0,   355,     0,   356,     0,     0,   361,     0,
       0,   113,   121,   364,   365,     0,     0,     0,   366,     0,
       0,   368,   369,     0,     0,   372,   467,   108,   109,   110,
     111,   112,   113,   122,   123,     0,     0,     0,     0,   150,
     150,   150,   157,   162,   114,   150,   150,   150,     0,   150,
     108,   109,   110,   111,   112,     0,   157,   150,     0,   388,
     389,   192,     0,     0,     0,     0,     0,     0,   205,   207,
     209,   157,   212,   214,   399,   400,     0,   150,   150,   150,
     150,   157,   101,   102,   103,   104,     0,   150,     0,   105,
     106,   107,     0,     0,     0,     0,     0,     0,     0,   299,
       0,   101,   102,   103,   104,     0,   427,     0,   105,   106,
     107,   432,   433,     0,   108,   109,   110,   111,   112,     0,
       0,     0,     0,   117,   101,   102,   103,   104,   127,   128,
       0,   105,   106,   107,   101,   102,   103,   104,   440,     0,
     413,   105,   106,   107,     0,     0,   203,     0,     0,     0,
     446,   447,     0,   448,   449,   450,   451,   191,   455,   101,
     102,   103,   104,     0,     0,     0,   105,   106,   107,     0,
       0,     0,     0,     0,     0,     0,   466,     0,   468,     0,
       0,   469,     0,   470,     0,   471,   472,     0,     0,   473,
       0,     0,     0,     0,   245,   475,   476,     0,     0,   477,
       0,     0,   480,     0,     0,   481,   482,     0,   483,   357,
       0,   486,     0,     0,   487,   488,   489,   490,   491,   492,
       0,     0,   494,     0,   495,   496,     0,     0,     0,     0,
       0,     0,   497,   498,   499,   113,   284,     0,     0,     0,
       0,   379,   380,   381,   382,   383,   384,   114,     0,   292,
     293,     0,     0,   294,     0,   295,     0,     0,   121,     0,
     101,   102,   103,   104,     0,     0,   146,   105,   106,   107,
       0,     0,     0,   108,   109,   110,   111,   112,     0,   122,
     123,   157,     0,     0,   101,   102,   103,   104,     0,     0,
       0,   105,   106,   107,     0,     0,     0,   108,   109,   110,
     111,   112,     0,     0,     0,     0,   101,   102,   103,   104,
     331,     0,   332,   105,   106,   107,     0,     0,   157,   108,
     109,   110,   111,   112,     0,   150,   157,     0,     0,     0,
     157,     0,     0,   157,     0,   101,   102,   103,   104,     0,
       0,   150,   105,   106,   107,     0,   157,     0,   108,   109,
     110,   111,   112,   121,     0,  -175,  -175,  -175,  -175,     0,
       0,   150,  -175,  -175,  -175,     0,   371,     0,   179,   373,
     374,   140,     0,   375,   122,   123,   376,   141,   142,   143,
       0,     0,     0,     0,     0,     0,     0,     0,   150,   122,
     123,   147,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   150,   286,   287,   288,   289,     0,     0,   392,   393,
     394,   395,   101,   102,   103,   104,     0,     0,   113,   105,
     106,   107,   157,     0,     0,   406,   286,   287,   288,   289,
     114,     0,     0,     0,     0,     0,     0,     0,     0,   121,
     113,   101,   102,   103,   104,     0,     0,   146,   105,   106,
     107,     0,   114,     0,   108,   109,   110,   111,   112,     0,
     122,   123,     0,   129,     0,     0,   147,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   286,   287,   288,
     289,     0,  -175,     0,     0,     0,     0,     0,     0,   445,
       0,   504,     0,   101,   102,   103,   104,     0,     0,   453,
     105,   106,   107,     0,     0,     0,   108,   109,   110,   111,
     112,     0,     0,     0,   257,     0,   258,   259,     0,     0,
       0,     0,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   121,     0,   101,   102,   103,   104,     0,     0,
     146,   105,   106,   107,     0,     0,     0,   108,   109,   110,
     111,   112,   147,   122,   123,     0,     0,   165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     2,     0,   420,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       0,     0,    19,    20,    21,    22,    23,   113,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,   147,     0,     0,     0,     0,
       0,  -154,     0,  -154,  -154,  -154,  -154,     0,     0,  -154,
    -154,  -154,  -154,     0,     0,     1,  -154,  -154,  -154,  -154,
    -154,     2,     0,    81,     0,     0,     0,     0,     0,     0,
       0,    82,    83,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,     0,     0,
      19,    20,    21,    22,    23,     0,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,   101,   102,   103,   104,     0,     0,     0,
     105,   106,   107,     0,     0,     0,   108,   109,   110,   111,
     112,     0,     0,     0,  -154,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,  -156,  -156,  -156,  -156,    82,
      83,  -156,  -156,  -156,  -156,     0,   196,     0,  -156,  -156,
    -156,  -156,  -156,     0,   122,   123,   402,     0,   101,   102,
     103,   104,     0,     0,   146,   105,   106,   107,     0,     0,
       0,   108,   109,   110,   111,   112,   443,     0,   101,   102,
     103,   104,     0,     0,   146,   105,   106,   107,     0,     0,
       0,   108,   109,   110,   111,   112,     0,   404,     0,   101,
     102,   103,   104,     0,     0,   420,   105,   106,   107,     0,
       0,     0,   108,   109,   110,   111,   112,   101,   102,   103,
     104,     0,     0,   146,   105,   106,   107,     0,     0,     0,
     108,   109,   110,   111,   112,     0,     0,   113,   101,   102,
     103,   104,     0,  -156,     0,   105,   106,   107,     0,     0,
       0,   108,   109,   110,   111,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -156,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     147,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147
};

static const yytype_int16 yycheck[] =
{
       1,    18,     8,     9,   325,     6,     7,    14,    15,    16,
     329,     1,     1,     3,     4,     5,     6,    24,     1,     1,
      10,    11,    12,   297,   106,    21,    22,    33,    34,   111,
     132,     1,    22,    23,    35,     1,    32,   109,   110,    22,
      23,   137,   138,   132,     3,     4,     5,     6,   111,    11,
      12,    10,    11,    12,   130,     1,    52,    53,    54,    55,
     130,   306,     0,    25,    26,   310,    62,    73,    74,    21,
      76,    72,    78,     1,   147,   148,    22,    23,    22,     1,
      81,     1,    44,    45,   137,     1,    21,    49,   362,    51,
       1,    15,   130,     1,    22,    23,   111,    59,    60,   420,
      22,    23,    22,    23,    66,   130,    22,    23,   134,    71,
     133,    22,    23,   114,    22,    23,   135,    79,   145,   146,
     147,   148,   111,   134,   106,     1,   127,   128,   106,   106,
     131,     1,   133,   229,   230,   136,   137,   138,   144,    15,
     130,   130,   148,   149,   150,   106,    22,    23,   154,   155,
     106,   157,    22,    23,   144,   106,   138,   106,   479,   165,
     143,   131,   112,   169,   112,   484,   485,   109,   111,   175,
     106,   130,    85,    14,    15,    16,    17,   183,   312,   145,
     146,   147,   148,    24,   150,   144,     1,   188,     1,   190,
      31,   307,   198,   139,    16,    17,    18,    19,    20,   205,
     111,   207,   441,   209,   210,    46,   212,    22,    23,    22,
      23,   409,    -1,   133,    -1,    56,   138,   145,   146,   147,
     148,   137,    -1,     1,   132,   226,    -1,    -1,   229,   230,
       1,    -1,   238,     1,   145,   146,   147,   148,    16,    17,
      18,    19,    20,   244,    22,    23,   247,   248,    -1,   255,
     251,    22,    23,   254,    22,    23,   257,   258,     1,    -1,
       3,     4,     5,     6,    -1,    -1,   136,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    -1,    22,
      23,    39,    40,     1,    42,   286,   287,   288,   289,    -1,
      -1,    -1,     1,   300,    -1,   312,   111,    15,   111,    -1,
      58,   318,   303,   304,    22,    23,   307,    -1,   325,    -1,
     311,    -1,   134,    22,    23,     5,    -1,   318,     8,     9,
     316,    -1,   135,    -1,   325,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    27,    28,    -1,
     336,    -1,    32,    33,    34,    -1,    -1,    37,    38,    -1,
      -1,    41,   358,    43,    -1,    -1,   363,    -1,    48,   137,
      50,    -1,    52,    53,    54,    55,   367,    57,   111,     1,
      -1,    61,    62,    63,    64,    65,   377,    67,    -1,    -1,
     376,     1,   344,    73,   346,    75,    76,    77,    78,    -1,
      22,    23,    24,   111,   137,     1,    16,    17,    18,    19,
      20,    -1,   111,   420,    -1,    -1,   149,    -1,   409,   415,
      16,    17,    18,    19,    20,   256,    22,    23,    -1,   420,
      16,    17,    18,    19,    20,    -1,     1,   117,     3,     4,
       5,     6,    -1,    -1,   124,    10,    11,    12,    -1,    -1,
     441,    16,    17,    18,    19,    20,    -1,    -1,    -1,    28,
      29,    30,   293,    -1,   144,    -1,    -1,    -1,    -1,   300,
     301,    -1,   479,    -1,   305,    -1,   156,   308,    -1,    -1,
     160,    -1,   162,   163,   108,    -1,   110,   167,   479,    -1,
     321,   171,    -1,   173,    -1,    -1,   176,   177,   122,   123,
     124,   181,    -1,    -1,   184,    -1,   186,     1,    -1,    -1,
       4,   191,   192,    82,    83,   195,    10,    11,    12,   199,
     200,   131,   202,   203,    -1,   205,   206,    -1,   208,    -1,
      -1,   211,   363,    -1,   214,   215,    -1,   217,    -1,   219,
      -1,   221,    -1,   223,    -1,   225,    -1,    -1,   228,    -1,
      -1,   137,     1,   233,   234,    -1,    -1,    -1,   238,    -1,
      -1,   241,   242,    -1,    -1,   245,   397,    16,    17,    18,
      19,    20,   137,    22,    23,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,   149,    20,    21,    22,    -1,    24,
      16,    17,    18,    19,    20,    -1,    31,    32,    -1,   279,
     280,    36,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      45,    46,    47,    48,   294,   295,    -1,    52,    53,    54,
      55,    56,     3,     4,     5,     6,    -1,    62,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
      -1,     3,     4,     5,     6,    -1,   326,    -1,    10,    11,
      12,   331,   332,    -1,    16,    17,    18,    19,    20,    -1,
      -1,    -1,    -1,     1,     3,     4,     5,     6,     6,     7,
      -1,    10,    11,    12,     3,     4,     5,     6,   358,    -1,
       9,    10,    11,    12,    -1,    -1,   135,    -1,    -1,    -1,
     370,   371,    -1,   373,   374,   375,   376,    35,   378,     3,
       4,     5,     6,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   396,    -1,   398,    -1,
      -1,   401,    -1,   403,    -1,   405,   406,    -1,    -1,   409,
      -1,    -1,    -1,    -1,    72,   415,   416,    -1,    -1,   419,
      -1,    -1,   422,    -1,    -1,   425,   426,    -1,   428,   130,
      -1,   431,    -1,    -1,   434,   435,   436,   437,   438,   439,
      -1,    -1,   442,    -1,   444,   445,    -1,    -1,    -1,    -1,
      -1,    -1,   452,   453,   454,   137,   114,    -1,    -1,    -1,
      -1,   120,   121,   122,   123,   124,   125,   149,    -1,   127,
     128,    -1,    -1,   131,    -1,   133,    -1,    -1,     1,    -1,
       3,     4,     5,     6,    -1,    -1,     9,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    -1,    22,
      23,   256,    -1,    -1,     3,     4,     5,     6,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
     188,    -1,   190,    10,    11,    12,    -1,    -1,   293,    16,
      17,    18,    19,    20,    -1,   300,   301,    -1,    -1,    -1,
     305,    -1,    -1,   308,    -1,     3,     4,     5,     6,    -1,
      -1,   316,    10,    11,    12,    -1,   321,    -1,    16,    17,
      18,    19,    20,     1,    -1,     3,     4,     5,     6,    -1,
      -1,   336,    10,    11,    12,    -1,   244,    -1,     1,   247,
     248,     4,    -1,   251,    22,    23,   254,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   363,    22,
      23,   134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   376,   145,   146,   147,   148,    -1,    -1,   286,   287,
     288,   289,     3,     4,     5,     6,    -1,    -1,   137,    10,
      11,    12,   397,    -1,    -1,   303,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
     137,     3,     4,     5,     6,    -1,    -1,     9,    10,    11,
      12,    -1,   149,    -1,    16,    17,    18,    19,    20,    -1,
      22,    23,    -1,   111,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   145,   146,   147,
     148,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,   367,
      -1,     1,    -1,     3,     4,     5,     6,    -1,    -1,   377,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    -1,    -1,    -1,   105,    -1,   107,   108,    -1,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,     3,     4,     5,     6,    -1,    -1,
       9,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      19,    20,   134,    22,    23,    -1,    -1,   139,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    42,    43,    44,    45,    46,   137,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   134,    -1,    -1,    -1,    -1,
      -1,     1,    -1,     3,     4,     5,     6,    -1,    -1,     9,
      10,    11,    12,    -1,    -1,     7,    16,    17,    18,    19,
      20,    13,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,     3,     4,     5,     6,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    -1,    -1,    -1,   134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,     3,     4,     5,     6,   141,
     142,     9,    10,    11,    12,    -1,    14,    -1,    16,    17,
      18,    19,    20,    -1,    22,    23,     1,    -1,     3,     4,
       5,     6,    -1,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,     1,    -1,     3,     4,
       5,     6,    -1,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    -1,     1,    -1,     3,
       4,     5,     6,    -1,    -1,   105,    10,    11,    12,    -1,
      -1,    -1,    16,    17,    18,    19,    20,     3,     4,     5,
       6,    -1,    -1,     9,    10,    11,    12,    -1,    -1,    -1,
      16,    17,    18,    19,    20,    -1,    -1,   137,     3,     4,
       5,     6,    -1,   111,    -1,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,    13,    22,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    42,
      43,    44,    45,    46,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   133,   141,   142,   152,   153,   155,   156,   157,   158,
     159,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     199,     3,     4,     5,     6,    10,    11,    12,    16,    17,
      18,    19,    20,   137,   149,   175,   184,   187,   194,   196,
     197,     1,    22,    23,   143,   154,   154,   187,   187,   111,
     154,   183,   154,   183,     1,   131,   172,   172,   172,     1,
       4,    10,    11,    12,   178,   195,     9,   134,   180,   182,
     184,   185,   186,   197,   180,   180,   182,   184,   154,   175,
     176,   177,   184,   139,   154,   139,   154,   179,   180,   182,
     154,   179,   154,   179,   154,   180,   172,   172,   154,     1,
     154,   178,   178,   178,   182,   154,   179,   154,   183,   154,
     183,   187,   184,   132,   154,   170,    14,   154,   174,   170,
     170,   154,   170,   135,   154,   184,   172,   184,   172,   184,
     182,   134,   184,   154,   184,   172,   154,   172,   154,   179,
     154,   179,   154,   179,   154,   179,   182,   154,   170,   172,
     172,   154,   154,   179,   136,   154,   154,   154,   172,   154,
     132,   171,   130,   130,   172,   187,   154,   183,   183,   154,
     154,   183,   154,   154,   183,   172,    21,   105,   107,   108,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   197,   201,   178,
     178,     0,   155,    22,   187,   137,   145,   146,   147,   148,
     154,   154,   187,   187,   187,   187,     1,   173,   187,     1,
     173,   173,   154,   183,   183,   183,   183,   183,   183,   154,
     183,    21,   111,   154,   154,   154,   183,   154,    15,   154,
     154,   183,   154,   154,   154,    15,     1,   183,   154,   160,
     154,   187,   187,   154,   154,   154,   183,   154,   154,   154,
     154,   154,   183,   154,   183,   154,   183,   183,   154,   183,
     154,   154,   154,   154,   154,   154,   154,   130,   190,   191,
     197,   154,   173,   173,   154,   154,   154,   183,   154,   154,
     130,   187,   154,   187,   187,   187,   187,   183,   182,   120,
     121,   122,   123,   124,   125,   197,   197,   111,   154,   154,
       1,   150,   187,   187,   187,   187,   181,   183,   182,   154,
     154,   181,     1,   180,     1,   182,   187,   130,   144,   192,
     193,   197,   182,     9,   185,   190,   182,   197,   177,   179,
     105,   175,   188,   189,   197,   182,   188,   154,   130,   198,
     199,   200,   154,   154,   179,   135,   172,   172,   134,   134,
     154,   183,   181,     1,   180,   187,   154,   154,   154,   154,
     154,   154,   179,   187,    24,   154,   106,   106,   106,   106,
     106,   106,   106,   111,   109,   110,   154,   182,   154,   154,
     154,   154,   154,   154,   193,   154,   154,   154,   188,   138,
     154,   154,   154,   154,   112,   112,   154,   154,   154,   154,
     154,   154,   154,   191,   154,   154,   154,   154,   154,   154,
     111,   109,     1,   106,     1,   188,   199,   199,   108,   110,
     122,   123,   124,   106
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 175 "mon_parse.y"
    { (yyval.i) = 0; }
    break;

  case 3:
#line 176 "mon_parse.y"
    { (yyval.i) = 0; }
    break;

  case 4:
#line 177 "mon_parse.y"
    { new_cmd = 1; asm_mode = 0;  (yyval.i) = 0; }
    break;

  case 9:
#line 186 "mon_parse.y"
    { return ERR_EXPECT_END_CMD; }
    break;

  case 22:
#line 201 "mon_parse.y"
    { return ERR_BAD_CMD; }
    break;

  case 23:
#line 205 "mon_parse.y"
    { mon_bank(e_default_space, NULL); }
    break;

  case 24:
#line 207 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (3)].i), NULL); }
    break;

  case 25:
#line 209 "mon_parse.y"
    { mon_bank(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 26:
#line 211 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 27:
#line 213 "mon_parse.y"
    { mon_jump((yyvsp[(2) - (3)].a)); }
    break;

  case 28:
#line 215 "mon_parse.y"
    { mon_display_io_regs(); }
    break;

  case 29:
#line 217 "mon_parse.y"
    { monitor_cpu_type_set(""); }
    break;

  case 30:
#line 219 "mon_parse.y"
    { monitor_cpu_type_set((yyvsp[(2) - (3)].str)); }
    break;

  case 31:
#line 221 "mon_parse.y"
    { mon_cpuhistory(-1); }
    break;

  case 32:
#line 223 "mon_parse.y"
    { mon_cpuhistory((yyvsp[(3) - (4)].i)); }
    break;

  case 33:
#line 225 "mon_parse.y"
    { mon_instruction_return(); }
    break;

  case 34:
#line 227 "mon_parse.y"
    { machine_write_snapshot((yyvsp[(2) - (3)].str),0,0,0); /* FIXME */ }
    break;

  case 35:
#line 229 "mon_parse.y"
    { machine_read_snapshot((yyvsp[(2) - (3)].str), 0); }
    break;

  case 36:
#line 231 "mon_parse.y"
    { mon_instructions_step(-1); }
    break;

  case 37:
#line 233 "mon_parse.y"
    { mon_instructions_step((yyvsp[(3) - (4)].i)); }
    break;

  case 38:
#line 235 "mon_parse.y"
    { mon_instructions_next(-1); }
    break;

  case 39:
#line 237 "mon_parse.y"
    { mon_instructions_next((yyvsp[(3) - (4)].i)); }
    break;

  case 40:
#line 239 "mon_parse.y"
    { mon_stack_up(-1); }
    break;

  case 41:
#line 241 "mon_parse.y"
    { mon_stack_up((yyvsp[(3) - (4)].i)); }
    break;

  case 42:
#line 243 "mon_parse.y"
    { mon_stack_down(-1); }
    break;

  case 43:
#line 245 "mon_parse.y"
    { mon_stack_down((yyvsp[(3) - (4)].i)); }
    break;

  case 44:
#line 247 "mon_parse.y"
    { mon_display_screen(); }
    break;

  case 46:
#line 252 "mon_parse.y"
    { (monitor_cpu_for_memspace[default_memspace]->mon_register_print)(default_memspace); }
    break;

  case 47:
#line 254 "mon_parse.y"
    { (monitor_cpu_for_memspace[(yyvsp[(2) - (3)].i)]->mon_register_print)((yyvsp[(2) - (3)].i)); }
    break;

  case 49:
#line 259 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(4) - (5)].str));
                    }
    break;

  case 50:
#line 264 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(2) - (3)].str));
                    }
    break;

  case 51:
#line 269 "mon_parse.y"
    { mon_save_symbols((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 52:
#line 271 "mon_parse.y"
    { mon_save_symbols(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 53:
#line 273 "mon_parse.y"
    { mon_add_name_to_symbol_table((yyvsp[(2) - (5)].a), (yyvsp[(4) - (5)].str)); }
    break;

  case 54:
#line 275 "mon_parse.y"
    { mon_remove_name_from_symbol_table(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 55:
#line 277 "mon_parse.y"
    { mon_remove_name_from_symbol_table((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 56:
#line 279 "mon_parse.y"
    { mon_print_symbol_table((yyvsp[(2) - (3)].i)); }
    break;

  case 57:
#line 281 "mon_parse.y"
    { mon_print_symbol_table(e_default_space); }
    break;

  case 58:
#line 283 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (4)].a), mon_prepend_dot_to_name((yyvsp[(1) - (4)].str)));
                    }
    break;

  case 59:
#line 287 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (5)].a), mon_prepend_dot_to_name((yyvsp[(1) - (5)].str)));
                    }
    break;

  case 60:
#line 293 "mon_parse.y"
    { mon_start_assemble_mode((yyvsp[(2) - (2)].a), NULL); }
    break;

  case 61:
#line 294 "mon_parse.y"
    { }
    break;

  case 62:
#line 296 "mon_parse.y"
    { mon_start_assemble_mode((yyvsp[(2) - (3)].a), NULL); }
    break;

  case 63:
#line 298 "mon_parse.y"
    { mon_disassemble_lines((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1]); }
    break;

  case 64:
#line 300 "mon_parse.y"
    { mon_disassemble_lines(BAD_ADDR, BAD_ADDR); }
    break;

  case 65:
#line 304 "mon_parse.y"
    { mon_memory_move((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], (yyvsp[(4) - (5)].a)); }
    break;

  case 66:
#line 306 "mon_parse.y"
    { mon_memory_compare((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[0], (yyvsp[(4) - (5)].a)); }
    break;

  case 67:
#line 308 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); }
    break;

  case 68:
#line 310 "mon_parse.y"
    { mon_memory_hunt((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); }
    break;

  case 69:
#line 312 "mon_parse.y"
    { mon_memory_display((yyvsp[(2) - (5)].rt), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], DF_PETSCII); }
    break;

  case 70:
#line 314 "mon_parse.y"
    { mon_memory_display(default_radix, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); }
    break;

  case 71:
#line 316 "mon_parse.y"
    { mon_memory_display(default_radix, BAD_ADDR, BAD_ADDR, DF_PETSCII); }
    break;

  case 72:
#line 318 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 8, 8); }
    break;

  case 73:
#line 320 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 8, 8); }
    break;

  case 74:
#line 322 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 24, 21); }
    break;

  case 75:
#line 324 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 24, 21); }
    break;

  case 76:
#line 326 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); }
    break;

  case 77:
#line 328 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_PETSCII); }
    break;

  case 78:
#line 330 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_SCREEN_CODE); }
    break;

  case 79:
#line 332 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_SCREEN_CODE); }
    break;

  case 80:
#line 334 "mon_parse.y"
    { mon_memmap_zap(); }
    break;

  case 81:
#line 336 "mon_parse.y"
    { mon_memmap_show(-1,BAD_ADDR,BAD_ADDR); }
    break;

  case 82:
#line 338 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (4)].i),BAD_ADDR,BAD_ADDR); }
    break;

  case 83:
#line 340 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].range)[0],(yyvsp[(4) - (5)].range)[1]); }
    break;

  case 84:
#line 342 "mon_parse.y"
    { mon_memmap_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); }
    break;

  case 85:
#line 346 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], FALSE, FALSE,
                                                    FALSE, FALSE);
                  }
    break;

  case 86:
#line 351 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], FALSE, FALSE,
                                                    FALSE, TRUE);
                  }
    break;

  case 87:
#line 356 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], FALSE,
                                                           FALSE, FALSE, FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  }
    break;

  case 88:
#line 362 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], FALSE,
                      ((yyvsp[(2) - (5)].i) == e_load || (yyvsp[(2) - (5)].i) == e_load_store),
                      ((yyvsp[(2) - (5)].i) == e_store || (yyvsp[(2) - (5)].i) == e_load_store), FALSE);
                  }
    break;

  case 89:
#line 368 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], TRUE, FALSE, FALSE,
                                                    FALSE);
                  }
    break;

  case 90:
#line 373 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 91:
#line 375 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 92:
#line 377 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 93:
#line 379 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 94:
#line 384 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_ON, (yyvsp[(2) - (3)].i)); }
    break;

  case 95:
#line 386 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_OFF, (yyvsp[(2) - (3)].i)); }
    break;

  case 96:
#line 388 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (3)].i), -1); }
    break;

  case 97:
#line 390 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); }
    break;

  case 98:
#line 392 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint((yyvsp[(2) - (3)].i)); }
    break;

  case 99:
#line 394 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint(-1); }
    break;

  case 100:
#line 396 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_condition((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].cond_node)); }
    break;

  case 101:
#line 398 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_command((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 102:
#line 400 "mon_parse.y"
    { return ERR_EXPECT_STRING; }
    break;

  case 103:
#line 404 "mon_parse.y"
    { sidefx = (((yyvsp[(2) - (3)].action) == e_TOGGLE) ? (sidefx ^ 1) : (yyvsp[(2) - (3)].action)); }
    break;

  case 104:
#line 406 "mon_parse.y"
    {
                         mon_out("I/O side effects are %s\n",
                                   sidefx ? "enabled" : "disabled");
                     }
    break;

  case 105:
#line 411 "mon_parse.y"
    { default_radix = (yyvsp[(2) - (3)].rt); }
    break;

  case 106:
#line 413 "mon_parse.y"
    {
                         const char *p;

                         if (default_radix == e_hexadecimal)
                             p = "Hexadecimal";
                         else if (default_radix == e_decimal)
                             p = "Decimal";
                         else if (default_radix == e_octal)
                             p = "Octal";
                         else if (default_radix == e_binary)
                             p = "Binary";
                         else
                             p = "Unknown";

                         mon_out("Default radix is %s\n", p);
                     }
    break;

  case 107:
#line 431 "mon_parse.y"
    { monitor_change_device((yyvsp[(2) - (3)].i)); }
    break;

  case 108:
#line 433 "mon_parse.y"
    { mon_quit(); YYACCEPT; }
    break;

  case 109:
#line 435 "mon_parse.y"
    { exit_mon = 1; YYACCEPT; }
    break;

  case 110:
#line 439 "mon_parse.y"
    { mon_drive_execute_disk_cmd((yyvsp[(2) - (3)].str)); }
    break;

  case 111:
#line 441 "mon_parse.y"
    { mon_out("\t%d\n",(yyvsp[(2) - (3)].i)); }
    break;

  case 112:
#line 443 "mon_parse.y"
    { mon_command_print_help(NULL); }
    break;

  case 113:
#line 445 "mon_parse.y"
    { mon_command_print_help((yyvsp[(2) - (3)].str)); }
    break;

  case 114:
#line 447 "mon_parse.y"
    { printf("SYSTEM COMMAND: %s\n",(yyvsp[(2) - (3)].str)); }
    break;

  case 115:
#line 449 "mon_parse.y"
    { mon_print_convert((yyvsp[(2) - (3)].i)); }
    break;

  case 116:
#line 451 "mon_parse.y"
    { mon_change_dir((yyvsp[(2) - (3)].str)); }
    break;

  case 117:
#line 453 "mon_parse.y"
    { mon_keyboard_feed((yyvsp[(2) - (3)].str)); }
    break;

  case 118:
#line 455 "mon_parse.y"
    { mon_backtrace(); }
    break;

  case 119:
#line 457 "mon_parse.y"
    { mon_show_dir((yyvsp[(2) - (3)].str)); }
    break;

  case 120:
#line 459 "mon_parse.y"
    { mon_show_pwd(); }
    break;

  case 121:
#line 461 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (3)].str),-1); }
    break;

  case 122:
#line 463 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); }
    break;

  case 123:
#line 465 "mon_parse.y"
    { mon_resource_get((yyvsp[(2) - (3)].str)); }
    break;

  case 124:
#line 467 "mon_parse.y"
    { mon_resource_set((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].str)); }
    break;

  case 125:
#line 469 "mon_parse.y"
    { mon_reset_machine(-1); }
    break;

  case 126:
#line 471 "mon_parse.y"
    { mon_reset_machine((yyvsp[(3) - (4)].i)); }
    break;

  case 127:
#line 473 "mon_parse.y"
    { mon_tape_ctrl((yyvsp[(3) - (4)].i)); }
    break;

  case 128:
#line 475 "mon_parse.y"
    { mon_cart_freeze(); }
    break;

  case 129:
#line 480 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), FALSE); }
    break;

  case 130:
#line 482 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), TRUE); }
    break;

  case 131:
#line 484 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], FALSE); }
    break;

  case 132:
#line 486 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; }
    break;

  case 133:
#line 488 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 134:
#line 490 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], TRUE); }
    break;

  case 135:
#line 492 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 136:
#line 494 "mon_parse.y"
    { mon_file_verify((yyvsp[(2) - (5)].str),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 137:
#line 496 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 138:
#line 498 "mon_parse.y"
    { mon_drive_block_cmd(0,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 139:
#line 500 "mon_parse.y"
    { mon_drive_block_cmd(1,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 140:
#line 502 "mon_parse.y"
    { mon_attach((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].i)); }
    break;

  case 141:
#line 504 "mon_parse.y"
    { mon_detach((yyvsp[(2) - (3)].i)); }
    break;

  case 142:
#line 508 "mon_parse.y"
    { mon_record_commands((yyvsp[(2) - (3)].str)); }
    break;

  case 143:
#line 510 "mon_parse.y"
    { mon_end_recording(); }
    break;

  case 144:
#line 512 "mon_parse.y"
    { mon_playback_init((yyvsp[(2) - (3)].str)); }
    break;

  case 145:
#line 516 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (4)].a), BAD_ADDR, (unsigned char *)(yyvsp[(3) - (4)].str)); }
    break;

  case 146:
#line 518 "mon_parse.y"
    { printf("Not yet.\n"); }
    break;

  case 147:
#line 522 "mon_parse.y"
    { yydebug = 1; }
    break;

  case 148:
#line 525 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 149:
#line 528 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 150:
#line 529 "mon_parse.y"
    { (yyval.str) = NULL; }
    break;

  case 152:
#line 533 "mon_parse.y"
    { return ERR_EXPECT_FILENAME; }
    break;

  case 154:
#line 537 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; }
    break;

  case 155:
#line 540 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 156:
#line 541 "mon_parse.y"
    { (yyval.i) = e_load_store; }
    break;

  case 157:
#line 544 "mon_parse.y"
    { (yyval.i) = new_reg(default_memspace, (yyvsp[(1) - (1)].reg)); }
    break;

  case 158:
#line 545 "mon_parse.y"
    { (yyval.i) = new_reg((yyvsp[(1) - (2)].i), (yyvsp[(2) - (2)].reg)); }
    break;

  case 161:
#line 553 "mon_parse.y"
    { (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (3)].i))]->mon_register_set_val)(reg_memspace((yyvsp[(1) - (3)].i)), reg_regid((yyvsp[(1) - (3)].i)), (WORD) (yyvsp[(3) - (3)].i)); }
    break;

  case 162:
#line 556 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 163:
#line 557 "mon_parse.y"
    { return ERR_EXPECT_BRKNUM; }
    break;

  case 165:
#line 561 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (1)].a); (yyval.range)[1] = BAD_ADDR; }
    break;

  case 166:
#line 564 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (3)].a); (yyval.range)[1] = (yyvsp[(3) - (3)].a); }
    break;

  case 167:
#line 566 "mon_parse.y"
    { if (resolve_range(e_default_space, (yyval.range), (yyvsp[(1) - (1)].str))) return ERR_ADDR_TOO_BIG; }
    break;

  case 168:
#line 568 "mon_parse.y"
    { if (resolve_range((yyvsp[(1) - (3)].i), (yyval.range), (yyvsp[(3) - (3)].str))) return ERR_ADDR_TOO_BIG; }
    break;

  case 169:
#line 571 "mon_parse.y"
    { (yyval.a) = (yyvsp[(2) - (2)].a); }
    break;

  case 170:
#line 572 "mon_parse.y"
    { (yyval.a) = BAD_ADDR; }
    break;

  case 171:
#line 576 "mon_parse.y"
    {
             (yyval.a) = new_addr(e_default_space,(yyvsp[(1) - (1)].i));
             if (opt_asm) new_cmd = asm_mode = 1; 
         }
    break;

  case 172:
#line 581 "mon_parse.y"
    {
             (yyval.a) = new_addr((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i));
             if (opt_asm) new_cmd = asm_mode = 1; 
         }
    break;

  case 173:
#line 586 "mon_parse.y"
    {
             temp = mon_symbol_table_lookup_addr(e_default_space, (yyvsp[(1) - (1)].str));
             if (temp >= 0)
                 (yyval.a) = new_addr(e_default_space, temp);
             else
                 return ERR_UNDEFINED_LABEL;
         }
    break;

  case 176:
#line 599 "mon_parse.y"
    { (yyval.i) = e_comp_space; }
    break;

  case 177:
#line 600 "mon_parse.y"
    { (yyval.i) = e_disk8_space; }
    break;

  case 178:
#line 601 "mon_parse.y"
    { (yyval.i) = e_disk9_space; }
    break;

  case 179:
#line 602 "mon_parse.y"
    { (yyval.i) = e_disk10_space; }
    break;

  case 180:
#line 603 "mon_parse.y"
    { (yyval.i) = e_disk11_space; }
    break;

  case 181:
#line 606 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); if (!CHECK_ADDR((yyvsp[(1) - (1)].i))) return ERR_ADDR_TOO_BIG; }
    break;

  case 182:
#line 608 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 183:
#line 610 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) + (yyvsp[(3) - (3)].i); }
    break;

  case 184:
#line 611 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) - (yyvsp[(3) - (3)].i); }
    break;

  case 185:
#line 612 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) * (yyvsp[(3) - (3)].i); }
    break;

  case 186:
#line 613 "mon_parse.y"
    { (yyval.i) = ((yyvsp[(3) - (3)].i)) ? ((yyvsp[(1) - (3)].i) / (yyvsp[(3) - (3)].i)) : 1; }
    break;

  case 187:
#line 614 "mon_parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 188:
#line 615 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; }
    break;

  case 189:
#line 616 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 190:
#line 620 "mon_parse.y"
    {
               (yyval.cond_node) = new_cond; (yyval.cond_node)->is_parenthized = FALSE;
               (yyval.cond_node)->child1 = (yyvsp[(1) - (3)].cond_node); (yyval.cond_node)->child2 = (yyvsp[(3) - (3)].cond_node); (yyval.cond_node)->operation = (yyvsp[(2) - (3)].cond_op);
           }
    break;

  case 191:
#line 625 "mon_parse.y"
    { return ERR_INCOMPLETE_COMPARE_OP; }
    break;

  case 192:
#line 627 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(2) - (3)].cond_node); (yyval.cond_node)->is_parenthized = TRUE; }
    break;

  case 193:
#line 629 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; }
    break;

  case 194:
#line 631 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(1) - (1)].cond_node); }
    break;

  case 195:
#line 634 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->reg_num = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = TRUE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          }
    break;

  case 196:
#line 640 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->value = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = FALSE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          }
    break;

  case 199:
#line 652 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); }
    break;

  case 200:
#line 653 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); }
    break;

  case 203:
#line 660 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); }
    break;

  case 204:
#line 661 "mon_parse.y"
    { mon_add_number_masked_to_buffer((yyvsp[(1) - (1)].i), 0x00); }
    break;

  case 205:
#line 662 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); }
    break;

  case 206:
#line 665 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 207:
#line 666 "mon_parse.y"
    { (yyval.i) = (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (1)].i))]->mon_register_get_val)(reg_memspace((yyvsp[(1) - (1)].i)), reg_regid((yyvsp[(1) - (1)].i))); }
    break;

  case 208:
#line 669 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 209:
#line 670 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 210:
#line 671 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 211:
#line 672 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 212:
#line 675 "mon_parse.y"
    { (yyval.i) = resolve_datatype(B_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 213:
#line 676 "mon_parse.y"
    { (yyval.i) = resolve_datatype(O_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 214:
#line 677 "mon_parse.y"
    { (yyval.i) = resolve_datatype(D_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 215:
#line 680 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 216:
#line 681 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 217:
#line 682 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 218:
#line 683 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 219:
#line 684 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 223:
#line 692 "mon_parse.y"
    { (yyval.i) = 0;
                                                if ((yyvsp[(1) - (2)].str)) {
                                                    (monitor_cpu_for_memspace[default_memspace]->mon_assemble_instr)((yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].i));
                                                } else {
                                                    new_cmd = 1;
                                                    asm_mode = 0;
                                                }
                                                opt_asm = 0;
                                              }
    break;

  case 225:
#line 703 "mon_parse.y"
    { asm_mode = 0; }
    break;

  case 226:
#line 706 "mon_parse.y"
    { if ((yyvsp[(2) - (2)].i) > 0xff)
                          (yyval.i) = join_ints(ASM_ADDR_MODE_IMMEDIATE_16,(yyvsp[(2) - (2)].i));
                        else
                          (yyval.i) = join_ints(ASM_ADDR_MODE_IMMEDIATE,(yyvsp[(2) - (2)].i)); }
    break;

  case 227:
#line 710 "mon_parse.y"
    { if ((yyvsp[(1) - (1)].i) < 0x100)
               (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE,(yyvsp[(1) - (1)].i));
             else
               (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE,(yyvsp[(1) - (1)].i));
           }
    break;

  case 228:
#line 715 "mon_parse.y"
    { if ((yyvsp[(1) - (3)].i) < 0x100)
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE_X,(yyvsp[(1) - (3)].i));
                          else
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_X,(yyvsp[(1) - (3)].i));
                        }
    break;

  case 229:
#line 720 "mon_parse.y"
    { if ((yyvsp[(1) - (3)].i) < 0x100)
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE_Y,(yyvsp[(1) - (3)].i));
                          else
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_Y,(yyvsp[(1) - (3)].i));
                        }
    break;

  case 230:
#line 726 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABS_INDIRECT,(yyvsp[(2) - (3)].i)); }
    break;

  case 231:
#line 728 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_X,(yyvsp[(2) - (5)].i)); }
    break;

  case 232:
#line 730 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_Y,(yyvsp[(2) - (5)].i)); }
    break;

  case 233:
#line 731 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_BC,0); }
    break;

  case 234:
#line 732 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_DE,0); }
    break;

  case 235:
#line 733 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_HL,0); }
    break;

  case 236:
#line 734 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_IX,0); }
    break;

  case 237:
#line 735 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_IY,0); }
    break;

  case 238:
#line 736 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_SP,0); }
    break;

  case 239:
#line 738 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_A,(yyvsp[(2) - (5)].i)); }
    break;

  case 240:
#line 740 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_HL,(yyvsp[(2) - (5)].i)); }
    break;

  case 241:
#line 742 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_IX,(yyvsp[(2) - (5)].i)); }
    break;

  case 242:
#line 744 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_IY,(yyvsp[(2) - (5)].i)); }
    break;

  case 243:
#line 745 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_IMPLIED,0); }
    break;

  case 244:
#line 746 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ACCUMULATOR,0); }
    break;

  case 245:
#line 747 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_B,0); }
    break;

  case 246:
#line 748 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_C,0); }
    break;

  case 247:
#line 749 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_D,0); }
    break;

  case 248:
#line 750 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_E,0); }
    break;

  case 249:
#line 751 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_H,0); }
    break;

  case 250:
#line 752 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IXH,0); }
    break;

  case 251:
#line 753 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IYH,0); }
    break;

  case 252:
#line 754 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_L,0); }
    break;

  case 253:
#line 755 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IXL,0); }
    break;

  case 254:
#line 756 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IYL,0); }
    break;

  case 255:
#line 757 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_AF,0); }
    break;

  case 256:
#line 758 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_BC,0); }
    break;

  case 257:
#line 759 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_DE,0); }
    break;

  case 258:
#line 760 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_HL,0); }
    break;

  case 259:
#line 761 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IX,0); }
    break;

  case 260:
#line 762 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IY,0); }
    break;

  case 261:
#line 763 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_SP,0); }
    break;


/* Line 1267 of yacc.c.  */
#line 3676 "mon_parse.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 767 "mon_parse.y"


void parse_and_execute_line(char *input)
{
   char *temp_buf;
   int i, rc;

   temp_buf = (char *)lib_malloc(strlen(input) + 3);
   strcpy(temp_buf,input);
   i = (int)strlen(input);
   temp_buf[i++] = '\n';
   temp_buf[i++] = '\0';
   temp_buf[i++] = '\0';

   make_buffer(temp_buf);
   if ( (rc =yyparse()) != 0) {
       mon_out("ERROR -- ");
       switch(rc) {
         case ERR_BAD_CMD:
           mon_out("Bad command:\n");
           break;
         case ERR_RANGE_BAD_START:
           mon_out("Bad first address in range:\n");
           break;
         case ERR_RANGE_BAD_END:
           mon_out("Bad second address in range:\n");
           break;
         case ERR_EXPECT_BRKNUM:
           mon_out("Checkpoint number expected:\n");
           break;
         case ERR_EXPECT_END_CMD:
           mon_out("Unexpected token:\n");
           break;
         case ERR_MISSING_CLOSE_PAREN:
           mon_out("')' expected:\n");
           break;
         case ERR_INCOMPLETE_COMPARE_OP:
           mon_out("Compare operation missing an operand:\n");
           break;
         case ERR_EXPECT_FILENAME:
           mon_out("Expecting a filename:\n");
           break;
         case ERR_ADDR_TOO_BIG:
           mon_out("Address too large:\n");
           break;
         case ERR_IMM_TOO_BIG:
           mon_out("Immediate argument too large:\n");
           break;
         case ERR_EXPECT_STRING:
           mon_out("Expecting a string.\n");
           break;
         case ERR_UNDEFINED_LABEL:
           mon_out("Found an undefined label.\n");
           break;
         case ERR_EXPECT_DEVICE_NUM:
           mon_out("Expecting a device number.\n");
           break;
         case ERR_EXPECT_ADDRESS:
           mon_out("Expecting an address.\n");
           break;
         case ERR_ILLEGAL_INPUT:
         default:
           mon_out("Wrong syntax:\n");
       }
       mon_out("  %s\n", input);
       for (i = 0; i < last_len; i++)
           mon_out(" ");
       mon_out("  ^\n");
       asm_mode = 0;
       new_cmd = 1;
   }
   lib_free(temp_buf);
   free_buffer();
}

static int yyerror(char *s)
{
   fprintf(stderr, "ERR:%s\n", s);
   return 0;
}

static int resolve_datatype(unsigned guess_type, const char *num)
{
   /* FIXME: Handle cases when default type is non-numerical */
   if (default_radix == e_hexadecimal) {
       return strtol(num, NULL, 16);
   }

   if ((guess_type == D_NUMBER) || (default_radix == e_decimal)) {
       return strtol(num, NULL, 10);
   }

   if ((guess_type == O_NUMBER) || (default_radix == e_octal)) {
       return strtol(num, NULL, 8);
   }

   return strtol(num, NULL, 2);
}

/*
 * Resolve a character sequence containing 8 hex digits like "08001000". 
 * This could be a lazy version of "0800 1000". If the default radix is not
 * hexadecimal, we handle it like a ordinary number, in the latter case there
 * is only one number in the range.
 */
static int resolve_range(enum t_memspace memspace, MON_ADDR range[2], 
                         const char *num)
{
    char start[5];
    char end[5];
    long sa;

    range[1] = BAD_ADDR;

    switch (default_radix)
    {
    case e_hexadecimal:
        /* checked twice, but as the code must have exactly 8 digits: */
        if (strlen(num) == 8) {
            memcpy(start, num, 4);
            start[4] = '\0';
            memcpy(end, num + 4, 4);
            end[4] = '\0';
            sa = strtol(start, NULL, 16);
            range[1] = new_addr(memspace, strtol(end, NULL, 16));
        }
        else
            sa = strtol(num, NULL, 16);
        break;

    case e_decimal:
       sa = strtol(num, NULL, 10);
       break;

    case e_octal:
       sa = strtol(num, NULL, 8);
       break;

    default:
       sa = strtol(num, NULL, 2);
    }

    if (!CHECK_ADDR(sa))
        return ERR_ADDR_TOO_BIG;

    range[0] = new_addr(memspace, sa);
    return 0;
}

/* If we want 'quit' for OS/2 I couldn't leave the emulator by calling exit(0)
   So I decided to skip this (I think it's unnecessary for OS/2 */
static void mon_quit(void)
{
#ifdef OS2
    /* same as "quit" */
    exit_mon = 1; 
#else
    exit_mon = 2;
#endif
}


