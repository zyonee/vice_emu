/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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

/* SunOS 4.x specific stuff */
#if defined(sun) || defined(__sun)
#  if !defined(__SVR4) && !defined(__svr4__)
#    ifdef __sparc__
#      define YYFREE
#    endif
#  endif
#endif

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
#define ERR_EXPECT_CHECKNUM 5
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



/* Line 189 of yacc.c  */
#line 199 "mon_parse.c"

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
     CMD_COMMENT = 359,
     CMD_LIST = 360,
     CMD_EXPORT = 361,
     CMD_AUTOSTART = 362,
     CMD_AUTOLOAD = 363,
     CMD_LABEL_ASGN = 364,
     L_PAREN = 365,
     R_PAREN = 366,
     ARG_IMMEDIATE = 367,
     REG_A = 368,
     REG_S = 369,
     REG_X = 370,
     REG_Y = 371,
     REG_Z = 372,
     COMMA = 373,
     L_BRACK = 374,
     R_BRACK = 375,
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
#define CMD_COMMENT 359
#define CMD_LIST 360
#define CMD_EXPORT 361
#define CMD_AUTOSTART 362
#define CMD_AUTOLOAD 363
#define CMD_LABEL_ASGN 364
#define L_PAREN 365
#define R_PAREN 366
#define ARG_IMMEDIATE 367
#define REG_A 368
#define REG_S 369
#define REG_X 370
#define REG_Y 371
#define REG_Z 372
#define COMMA 373
#define L_BRACK 374
#define R_BRACK 375
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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 555 "mon_parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 567 "mon_parse.c"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  299
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1773

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  160
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  286
/* YYNRULES -- Number of states.  */
#define YYNSTATES  576

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   408

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     158,   159,   156,   154,     2,   155,     2,   157,     2,     2,
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
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    15,    17,    19,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    47,    50,    54,    58,    64,    68,    71,
      75,    78,    82,    85,    90,    93,    97,   101,   104,   109,
     112,   117,   120,   125,   128,   133,   136,   138,   141,   145,
     149,   155,   159,   165,   169,   175,   179,   185,   189,   192,
     197,   203,   204,   210,   214,   218,   221,   227,   233,   239,
     245,   251,   255,   258,   262,   265,   269,   272,   276,   279,
     283,   286,   289,   292,   297,   303,   309,   313,   319,   324,
     331,   334,   338,   344,   349,   356,   359,   363,   369,   374,
     381,   384,   388,   391,   395,   399,   403,   409,   413,   416,
     422,   428,   433,   437,   440,   444,   447,   451,   454,   457,
     460,   464,   468,   471,   475,   479,   483,   487,   491,   494,
     498,   501,   505,   511,   515,   520,   523,   528,   533,   536,
     540,   546,   552,   558,   562,   567,   573,   578,   584,   589,
     595,   601,   604,   608,   613,   617,   621,   627,   631,   637,
     641,   644,   648,   653,   656,   659,   661,   663,   664,   666,
     668,   670,   672,   674,   675,   677,   680,   684,   686,   690,
     692,   694,   696,   698,   702,   704,   708,   711,   712,   714,
     718,   720,   722,   723,   725,   727,   729,   731,   733,   735,
     737,   741,   745,   749,   753,   757,   761,   763,   767,   771,
     775,   779,   781,   783,   785,   789,   791,   793,   795,   798,
     800,   802,   804,   806,   808,   810,   812,   814,   816,   818,
     820,   822,   824,   826,   828,   830,   832,   834,   838,   842,
     845,   848,   850,   852,   855,   857,   861,   865,   869,   876,
     881,   885,   889,   893,   899,   905,   911,   919,   925,   929,
     933,   937,   941,   945,   949,   955,   961,   967,   973,   974,
     976,   978,   980,   982,   984,   986,   988,   990,   992,   994,
     996,   998,  1000,  1002,  1004,  1006,  1008
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     161,     0,    -1,   162,    -1,   208,    22,    -1,    22,    -1,
     164,    -1,   162,   164,    -1,    23,    -1,    22,    -1,     1,
      -1,   165,    -1,   167,    -1,   170,    -1,   168,    -1,   171,
      -1,   172,    -1,   173,    -1,   174,    -1,   175,    -1,   176,
      -1,   177,    -1,   178,    -1,    13,    -1,    67,   163,    -1,
      67,   193,   163,    -1,    67,   144,   163,    -1,    67,   193,
     192,   144,   163,    -1,    38,   191,   163,    -1,    46,   163,
      -1,    46,   191,   163,    -1,    87,   163,    -1,    87,   145,
     163,    -1,   100,   163,    -1,   100,   192,   196,   163,    -1,
      26,   163,    -1,    49,   181,   163,    -1,    50,   181,   163,
      -1,    58,   163,    -1,    58,   192,   196,   163,    -1,    57,
     163,    -1,    57,   192,   196,   163,    -1,    29,   163,    -1,
      29,   192,   196,   163,    -1,    30,   163,    -1,    30,   192,
     196,   163,    -1,    85,   163,    -1,   166,    -1,    39,   163,
      -1,    39,   193,   163,    -1,    39,   185,   163,    -1,    68,
     193,   192,   181,   163,    -1,    68,   181,   163,    -1,    69,
     193,   192,   181,   163,    -1,    69,   181,   163,    -1,    70,
     191,   192,   143,   163,    -1,    71,   143,   163,    -1,    71,
     193,   192,   143,   163,    -1,    72,   193,   163,    -1,    72,
     163,    -1,   109,    21,   191,   163,    -1,   109,    21,   191,
      24,   163,    -1,    -1,    55,   191,   169,   209,   163,    -1,
      55,   191,   163,    -1,    56,   188,   163,    -1,    56,   163,
      -1,    37,   189,   192,   191,   163,    -1,    48,   189,   192,
     191,   163,    -1,    36,   189,   192,   199,   163,    -1,    35,
     189,   192,   201,   163,    -1,    43,   148,   192,   188,   163,
      -1,    43,   188,   163,    -1,    43,   163,    -1,    76,   188,
     163,    -1,    76,   163,    -1,    77,   188,   163,    -1,    77,
     163,    -1,    78,   188,   163,    -1,    78,   163,    -1,    79,
     188,   163,    -1,    79,   163,    -1,   101,   163,    -1,   102,
     163,    -1,   102,   192,   196,   163,    -1,   102,   192,   196,
     188,   163,    -1,   103,   181,   192,   196,   163,    -1,    44,
     188,   163,    -1,    44,   188,    15,   197,   163,    -1,    44,
     183,   188,   163,    -1,    44,   183,   188,    15,   197,   163,
      -1,    44,   163,    -1,    62,   188,   163,    -1,    62,   188,
      15,   197,   163,    -1,    62,   183,   188,   163,    -1,    62,
     183,   188,    15,   197,   163,    -1,    62,   163,    -1,    45,
     188,   163,    -1,    45,   188,    15,   197,   163,    -1,    45,
     183,   188,   163,    -1,    45,   183,   188,    15,   197,   163,
      -1,    45,   163,    -1,    86,   188,   163,    -1,    86,   163,
      -1,   150,   187,   163,    -1,   151,   187,   163,    -1,    34,
     187,   163,    -1,    34,   187,   192,   196,   163,    -1,    52,
     187,   163,    -1,    52,   163,    -1,    53,   187,    15,   197,
     163,    -1,    54,   187,   192,   139,   163,    -1,    54,   187,
       1,   163,    -1,    25,   152,   163,    -1,    25,   163,    -1,
      42,   148,   163,    -1,    42,   163,    -1,    60,   193,   163,
      -1,   106,   163,    -1,    65,   163,    -1,    51,   163,    -1,
      63,   179,   163,    -1,    59,   196,   163,    -1,    61,   163,
      -1,    61,   179,   163,    -1,    64,   179,   163,    -1,     7,
     196,   163,    -1,    66,   179,   163,    -1,    82,   139,   163,
      -1,    89,   163,    -1,    92,   180,   163,    -1,    91,   163,
      -1,    90,   181,   163,    -1,    90,   181,   192,   196,   163,
      -1,    93,   139,   163,    -1,    94,   139,   139,   163,    -1,
      97,   163,    -1,    97,   192,   196,   163,    -1,    98,   192,
     196,   163,    -1,    99,   163,    -1,   104,   180,   163,    -1,
      31,   181,   182,   190,   163,    -1,    83,   181,   182,   190,
     163,    -1,    32,   181,   182,   189,   163,    -1,    32,   181,
       1,    -1,    32,   181,   182,     1,    -1,    84,   181,   182,
     189,   163,    -1,    84,   181,   182,     1,    -1,    33,   181,
     182,   191,   163,    -1,    33,   181,   182,     1,    -1,    27,
     196,   196,   190,   163,    -1,    28,   196,   196,   191,   163,
      -1,   105,   163,    -1,   105,   182,   163,    -1,    95,   181,
     196,   163,    -1,    96,   196,   163,    -1,   107,   181,   163,
      -1,   107,   181,   192,   206,   163,    -1,   108,   181,   163,
      -1,   108,   181,   192,   206,   163,    -1,    73,   181,   163,
      -1,    74,   163,    -1,    75,   181,   163,    -1,    80,   191,
     199,   163,    -1,    81,   163,    -1,    88,   163,    -1,   141,
      -1,   141,    -1,    -1,   140,    -1,     1,    -1,   196,    -1,
       1,    -1,    14,    -1,    -1,   146,    -1,   193,   146,    -1,
     185,   118,   186,    -1,   186,    -1,   184,    21,   206,    -1,
     204,    -1,     1,    -1,   189,    -1,   191,    -1,   191,   192,
     191,    -1,     9,    -1,   193,   192,     9,    -1,   192,   191,
      -1,    -1,   194,    -1,   193,   192,   194,    -1,   143,    -1,
     118,    -1,    -1,    16,    -1,    17,    -1,    18,    -1,    19,
      -1,    20,    -1,   195,    -1,   206,    -1,   196,   154,   196,
      -1,   196,   155,   196,    -1,   196,   156,   196,    -1,   196,
     157,   196,    -1,   158,   196,   159,    -1,   158,   196,     1,
      -1,   203,    -1,   197,   147,   197,    -1,   197,   147,     1,
      -1,   110,   197,   111,    -1,   110,   197,     1,    -1,   198,
      -1,   184,    -1,   206,    -1,   199,   192,   200,    -1,   200,
      -1,   206,    -1,   139,    -1,   201,   202,    -1,   202,    -1,
     206,    -1,   153,    -1,   139,    -1,   206,    -1,   184,    -1,
       4,    -1,    12,    -1,    11,    -1,    10,    -1,    12,    -1,
      11,    -1,    10,    -1,     3,    -1,     4,    -1,     5,    -1,
       6,    -1,   205,    -1,   207,   121,   208,    -1,   208,   121,
     208,    -1,   208,   121,    -1,   142,   210,    -1,   208,    -1,
     207,    -1,   112,   206,    -1,   206,    -1,   206,   118,   115,
      -1,   206,   118,   116,    -1,   206,   118,   114,    -1,   112,
     206,   118,   206,   118,   206,    -1,   112,   206,   118,   206,
      -1,   206,   118,   206,    -1,   110,   206,   111,    -1,   119,
     206,   120,    -1,   110,   206,   118,   115,   111,    -1,   110,
     206,   111,   118,   116,    -1,   110,   206,   111,   118,   117,
      -1,   110,   206,   118,   114,   111,   118,   116,    -1,   119,
     206,   120,   118,   116,    -1,   110,   129,   111,    -1,   110,
     130,   111,    -1,   110,   131,   111,    -1,   110,   132,   111,
      -1,   110,   133,   111,    -1,   110,   134,   111,    -1,   110,
     206,   111,   118,   113,    -1,   110,   206,   111,   118,   131,
      -1,   110,   206,   111,   118,   132,    -1,   110,   206,   111,
     118,   133,    -1,    -1,   113,    -1,   122,    -1,   123,    -1,
     124,    -1,   125,    -1,   126,    -1,   135,    -1,   137,    -1,
     127,    -1,   136,    -1,   138,    -1,   128,    -1,   129,    -1,
     130,    -1,   131,    -1,   132,    -1,   133,    -1,   134,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   187,   187,   188,   189,   192,   193,   196,   197,   198,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   216,   218,   220,   222,   224,   226,   228,
     230,   232,   234,   236,   238,   240,   242,   244,   246,   248,
     250,   252,   254,   256,   258,   260,   262,   265,   267,   269,
     272,   277,   282,   284,   286,   288,   290,   292,   294,   296,
     300,   307,   306,   309,   311,   313,   317,   319,   321,   323,
     325,   327,   329,   331,   333,   335,   337,   339,   341,   343,
     345,   347,   349,   351,   353,   355,   359,   363,   368,   372,
     377,   380,   384,   389,   393,   398,   401,   405,   410,   414,
     419,   422,   426,   431,   433,   435,   437,   439,   441,   443,
     445,   447,   451,   453,   458,   460,   478,   480,   482,   484,
     488,   490,   492,   494,   496,   498,   500,   502,   504,   506,
     508,   510,   512,   514,   516,   518,   520,   522,   524,   526,
     530,   532,   534,   536,   538,   540,   542,   544,   546,   548,
     550,   552,   554,   556,   558,   560,   562,   564,   566,   570,
     572,   574,   578,   580,   584,   588,   591,   592,   595,   596,
     599,   600,   603,   604,   607,   608,   611,   612,   615,   619,
     620,   623,   624,   627,   628,   630,   634,   635,   638,   643,
     648,   658,   659,   662,   663,   664,   665,   666,   669,   671,
     673,   674,   675,   676,   677,   678,   679,   682,   687,   689,
     691,   693,   697,   703,   711,   712,   715,   716,   719,   720,
     723,   724,   725,   728,   729,   732,   733,   734,   735,   738,
     739,   740,   743,   744,   745,   746,   747,   750,   751,   752,
     755,   765,   766,   769,   778,   786,   794,   799,   800,   803,
     806,   807,   812,   817,   822,   824,   826,   828,   830,   831,
     832,   833,   834,   835,   836,   838,   840,   842,   844,   845,
     846,   847,   848,   849,   850,   851,   852,   853,   854,   855,
     856,   857,   858,   859,   860,   861,   862
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
  "CMD_MEMMAPZAP", "CMD_MEMMAPSHOW", "CMD_MEMMAPSAVE", "CMD_COMMENT",
  "CMD_LIST", "CMD_EXPORT", "CMD_AUTOSTART", "CMD_AUTOLOAD",
  "CMD_LABEL_ASGN", "L_PAREN", "R_PAREN", "ARG_IMMEDIATE", "REG_A",
  "REG_S", "REG_X", "REG_Y", "REG_Z", "COMMA", "L_BRACK", "R_BRACK",
  "INST_SEP", "REG_B", "REG_C", "REG_D", "REG_E", "REG_H", "REG_L",
  "REG_AF", "REG_BC", "REG_DE", "REG_HL", "REG_IX", "REG_IY", "REG_SP",
  "REG_IXH", "REG_IXL", "REG_IYH", "REG_IYL", "STRING", "FILENAME",
  "R_O_L", "OPCODE", "LABEL", "BANKNAME", "CPUTYPE", "MON_REGISTER",
  "COMPARE_OP", "RADIX_TYPE", "INPUT_SPEC", "CMD_CHECKPT_ON",
  "CMD_CHECKPT_OFF", "TOGGLE", "MASK", "'+'", "'-'", "'*'", "'/'", "'('",
  "')'", "$accept", "top_level", "command_list", "end_cmd", "command",
  "machine_state_rules", "register_mod", "symbol_table_rules", "asm_rules",
  "$@1", "memory_rules", "checkpoint_rules", "checkpoint_control_rules",
  "monitor_state_rules", "monitor_misc_rules", "disk_rules",
  "cmd_file_rules", "data_entry_rules", "monitor_debug_rules",
  "rest_of_line", "opt_rest_of_line", "filename", "device_num",
  "opt_mem_op", "register", "reg_list", "reg_asgn", "checkpt_num",
  "address_opt_range", "address_range", "opt_address", "address",
  "opt_sep", "memspace", "memloc", "memaddr", "expression", "cond_expr",
  "compare_operand", "data_list", "data_element", "hunt_list",
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
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,    43,    45,    42,    47,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   160,   161,   161,   161,   162,   162,   163,   163,   163,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   166,   166,   166,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   169,   168,   168,   168,   168,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   173,   173,   173,   173,   173,   173,   173,   173,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   176,
     176,   176,   177,   177,   178,   179,   180,   180,   181,   181,
     182,   182,   183,   183,   184,   184,   185,   185,   186,   187,
     187,   188,   188,   189,   189,   189,   190,   190,   191,   191,
     191,   192,   192,   193,   193,   193,   193,   193,   194,   195,
     196,   196,   196,   196,   196,   196,   196,   197,   197,   197,
     197,   197,   198,   198,   199,   199,   200,   200,   201,   201,
     202,   202,   202,   203,   203,   204,   204,   204,   204,   205,
     205,   205,   206,   206,   206,   206,   206,   207,   207,   207,
     208,   209,   209,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     3,     5,     3,     2,     3,
       2,     3,     2,     4,     2,     3,     3,     2,     4,     2,
       4,     2,     4,     2,     4,     2,     1,     2,     3,     3,
       5,     3,     5,     3,     5,     3,     5,     3,     2,     4,
       5,     0,     5,     3,     3,     2,     5,     5,     5,     5,
       5,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     2,     2,     4,     5,     5,     3,     5,     4,     6,
       2,     3,     5,     4,     6,     2,     3,     5,     4,     6,
       2,     3,     2,     3,     3,     3,     5,     3,     2,     5,
       5,     4,     3,     2,     3,     2,     3,     2,     2,     2,
       3,     3,     2,     3,     3,     3,     3,     3,     2,     3,
       2,     3,     5,     3,     4,     2,     4,     4,     2,     3,
       5,     5,     5,     3,     4,     5,     4,     5,     4,     5,
       5,     2,     3,     4,     3,     3,     5,     3,     5,     3,
       2,     3,     4,     2,     2,     1,     1,     0,     1,     1,
       1,     1,     1,     0,     1,     2,     3,     1,     3,     1,
       1,     1,     1,     3,     1,     3,     2,     0,     1,     3,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     1,     1,     1,     3,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     2,
       2,     1,     1,     2,     1,     3,     3,     3,     6,     4,
       3,     3,     3,     5,     5,     5,     7,     5,     3,     3,
       3,     3,     3,     3,     5,     5,     5,     5,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
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
       0,     0,     0,     0,     0,     0,     0,     0,   167,     0,
       0,     0,     0,     0,   192,     0,     0,     0,     0,     0,
     167,     0,     0,     0,     0,     0,   268,     0,     0,     0,
       2,     5,    10,    46,    11,    13,    12,    14,    15,    16,
      17,    18,    19,    20,    21,     0,   232,   233,   234,   235,
     231,   230,   229,   193,   194,   195,   196,   197,   174,     0,
     224,     0,     0,   206,   236,   223,     9,     8,     7,     0,
     113,    34,     0,     0,   191,    41,     0,    43,     0,   169,
     168,     0,     0,     0,   180,   225,   228,   227,   226,     0,
     179,   184,   190,   192,   192,   192,   188,   198,   199,   192,
     192,     0,   192,    47,     0,     0,   177,     0,     0,   115,
     192,    72,     0,   181,   192,   172,    90,     0,     0,   100,
       0,     0,    28,     0,   192,     0,     0,   119,     9,   108,
       0,     0,     0,     0,    65,     0,    39,     0,    37,     0,
       0,     0,   165,   122,     0,    95,     0,     0,     0,     0,
     118,     0,     0,    23,     0,     0,   192,     0,   192,   192,
       0,   192,    58,     0,     0,   160,     0,    74,     0,    76,
       0,    78,     0,    80,     0,     0,   163,     0,     0,     0,
      45,   102,     0,     0,    30,   164,   128,     0,   130,   166,
       0,     0,     0,     0,     0,   135,     0,     0,   138,    32,
       0,    81,    82,     0,   192,     0,     9,   151,     0,   170,
     117,     0,     0,     0,     0,     0,   269,     0,   270,   271,
     272,   273,   274,   277,   280,   281,   282,   283,   284,   285,
     286,   275,   278,   276,   279,   244,   240,     0,     0,     1,
       6,     3,     0,   175,     0,     0,     0,     0,   125,   112,
     192,     0,     0,     0,   171,   192,   143,     0,     0,   105,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
      49,    48,   114,     0,    71,     0,     0,    86,     0,     0,
      96,    29,     0,    35,    36,   107,     0,     0,     0,    63,
       0,    64,     0,     0,   121,   116,   123,     0,     0,    91,
     120,   124,   126,    25,    24,     0,    51,     0,    53,     0,
       0,    55,     0,    57,   159,   161,    73,    75,    77,    79,
     217,     0,   215,   216,   127,   192,     0,   101,    31,   131,
       0,   129,   133,     0,     0,   154,     0,     0,     0,     0,
       0,   139,   152,   155,     0,   157,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   243,     0,     0,   103,   104,
     205,   204,   200,   201,   202,   203,     0,     0,     0,    42,
      44,     0,   144,     0,   148,     0,     0,   222,   221,     0,
     219,   220,   183,   185,   189,     0,     0,   178,   176,     0,
       0,    88,     0,   212,     0,   211,   213,     0,    98,     0,
       0,     0,   111,     0,   242,   241,     0,    40,    38,     0,
      93,     0,     0,     0,     0,     0,     0,   162,     0,     0,
     146,     0,     0,   134,   153,   136,   137,    33,    83,     0,
       0,     0,     0,     0,    59,   258,   259,   260,   261,   262,
     263,   251,     0,     0,   252,   247,   245,   246,   250,   149,
     186,   150,   140,   142,   147,   106,    69,   218,    68,    66,
      70,     0,     0,     0,    87,     0,    97,    67,   109,   110,
       0,   239,    62,     0,    92,    26,    50,    52,    54,    56,
     214,   141,   145,   132,    84,    85,   156,   158,    60,     0,
       0,     0,   249,     0,    89,   210,   209,   208,   207,    99,
     237,   238,    94,   264,   254,   255,   265,   266,   267,     0,
     253,     0,   257,     0,   248,   256
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    89,    90,   130,    91,    92,    93,    94,    95,   350,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   204,
     250,   141,   268,   177,   120,   165,   166,   149,   172,   173,
     426,   174,   427,   121,   156,   157,   269,   454,   455,   381,
     382,   439,   440,   123,   150,   124,   158,   464,   105,   466,
     296
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -338
static const yytype_int16 yypact[] =
{
    1199,  1046,  -338,  -338,     3,    19,  1046,  1046,   731,   731,
       6,     6,     6,   653,  1590,  1590,  1590,  1613,   104,     1,
    1071,  1312,  1312,  1485,  1590,     6,     6,    19,   306,   653,
     653,  1613,  1462,   731,   731,  1046,   675,    56,  1312,  -125,
    -125,    19,  -125,   270,   262,   262,  1613,   279,   654,     6,
      19,     6,  1462,  1462,  1462,  1462,  1613,    19,  -102,     6,
       6,    19,  1462,    28,    19,    19,     6,    19,   -95,   -86,
     -84,     6,  1046,   731,   -60,    19,   731,    19,   731,     6,
     -95,   868,    19,     6,     6,    48,  1635,   653,   653,    70,
    1326,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,    50,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  1046,
    -338,   -65,    10,  -338,  -338,  -338,  -338,  -338,  -338,    19,
    -338,  -338,  1028,  1028,  -338,  -338,  1046,  -338,  1046,  -338,
    -338,   414,   927,   414,  -338,  -338,  -338,  -338,  -338,   731,
    -338,  -338,  -338,   -60,   -60,   -60,  -338,  -338,  -338,   -60,
     -60,    19,   -60,  -338,    62,   219,  -338,    26,    19,  -338,
     -60,  -338,    19,  -338,   208,  -338,  -338,  1590,   435,  -338,
    1590,   444,  -338,    19,   -60,    19,    19,  -338,    72,  -338,
      19,    74,    18,    64,  -338,    19,  -338,  1046,  -338,  1046,
      10,    19,  -338,  -338,    19,  -338,  1590,   468,    19,    19,
    -338,    19,    19,  -338,   238,    19,   -60,    19,   -60,   -60,
      19,   -60,  -338,    19,    19,  -338,    19,  -338,    19,  -338,
      19,  -338,    19,  -338,    19,   832,  -338,    19,   414,   414,
    -338,  -338,    19,    19,  -338,  -338,  -338,   731,  -338,  -338,
      19,    19,   -43,  1046,    10,  -338,  1046,  1046,  -338,  -338,
    1046,  -338,  -338,  1046,   -60,    19,   116,  -338,    19,   525,
    -338,   583,   583,  1613,  1001,  1143,  -338,  1143,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,   -18,  -338,    19,    19,  -338,
    -338,  -338,    21,  -338,  1046,  1046,  1046,  1046,  -338,  -338,
     111,   703,    10,    10,  -338,   311,  1508,  1528,  1570,  -338,
    1046,   207,  1613,  1133,   832,  1613,  -338,  1143,  1143,   445,
    -338,  -338,  -338,  1590,  -338,   496,  1433,  -338,   514,  1433,
    -338,  -338,  1613,  -338,  -338,  -338,  1433,    19,   -38,  -338,
     -40,  -338,    10,    10,  -338,  -338,  -338,   630,  1433,  -338,
    -338,  -338,  -338,  -338,  -338,   -37,  -338,     6,  -338,     6,
     -32,  -338,   -30,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,   483,  -338,  -338,  -338,   311,  1548,  -338,  -338,  -338,
    1046,  -338,  -338,    19,    10,  -338,    10,    10,    10,   972,
    1046,  -338,  -338,  -338,  1143,  -338,  1143,    44,     5,     8,
      32,    33,    34,    39,   -90,    12,    36,   903,  -338,  -338,
    -338,  -338,  -126,  -126,  -338,  -338,    19,  1613,    19,  -338,
    -338,    19,  -338,    19,  -338,    19,    10,  -338,  -338,    87,
    -338,  -338,  -338,  -338,  -338,   483,    19,  -338,  -338,    19,
    1433,  -338,  1433,  -338,    13,  -338,  -338,  1433,  -338,    13,
      19,    13,  -338,    19,    38,    47,    19,  -338,  -338,  1433,
    -338,    13,    19,    19,    19,    19,    19,  -338,   832,    19,
    -338,    19,    10,  -338,  -338,  -338,  -338,  -338,  -338,    19,
      10,    19,    19,    19,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,    53,   -53,  1143,    63,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,    13,    14,  1100,  -338,    13,  -338,  -338,  -338,  -338,
     -40,   -40,  -338,    13,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,   656,
      77,    80,    83,    79,  -338,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,    85,
    -338,  1143,  -338,    89,  -338,  -338
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -338,  -338,  -338,   331,   117,  -338,  -338,  -338,  -338,  -338,
    -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,  -338,   193,
     128,   103,   235,   -21,   -15,  -338,  -114,   547,    22,    -6,
    -303,   644,    30,   594,  -271,  -338,   696,  -299,  -338,  -108,
    -257,  -338,  -214,  -338,  -338,  -338,    -1,  -338,  -337,  -338,
    -338
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -193
static const yytype_int16 yytable[] =
{
     125,   180,   126,   164,   126,   125,   125,   139,   153,   159,
     160,   126,   431,   465,   126,   555,   202,   206,   184,   347,
     126,   501,   420,   127,   128,   127,   128,   126,   502,   126,
     306,   307,   127,   128,   125,   127,   128,   237,   136,   138,
     459,   127,   128,   178,   181,   126,   249,   461,   127,   128,
     127,   128,   444,   251,   195,   252,   444,   126,   134,   471,
     207,   550,   551,   197,   199,   126,   127,   128,   493,   273,
     299,   125,   301,  -180,   228,   230,   232,   234,   127,   128,
     125,   303,   479,   328,   242,   295,   127,   128,   126,   346,
     106,   107,   108,   109,  -180,  -180,   393,   110,   111,   112,
     417,   463,    86,   256,   257,   126,   260,   472,   263,   127,
     128,   475,  -187,   476,   142,   143,   495,  -171,   125,   496,
     113,   114,   115,   116,   117,   556,   127,   128,   185,   186,
     503,   125,   125,  -187,  -187,   125,   134,   125,  -171,  -171,
     125,   125,   125,   497,   498,   499,   140,   215,   217,   168,
     500,   521,   224,   522,   226,   129,   504,  -192,   525,   530,
     523,   523,   238,   239,   304,   305,   306,   307,   531,   247,
     533,   549,   303,   243,   253,   304,   305,   306,   307,   320,
     421,   553,   264,   321,   322,   323,   271,   272,   569,   324,
     325,   570,   327,   560,   561,   572,   125,   202,   125,   335,
     333,   571,   338,   573,   322,   575,   -61,   300,   265,  -182,
     106,   107,   108,   109,   342,   448,   445,   110,   111,   112,
     126,   540,   348,  -182,   558,   517,   437,     0,   357,   134,
    -182,  -182,   208,   209,   383,   211,     0,   125,   125,   126,
     438,   127,   128,     0,   365,     0,   367,     0,   369,   370,
     118,   372,   125,     0,     0,   125,   125,     0,     0,   125,
     127,   128,   125,   139,     0,   304,   305,   306,   307,     0,
       0,   126,     0,   414,   415,     0,   416,   390,   113,   114,
     115,   116,   117,     0,     0,     0,   113,   114,   115,   116,
     117,     0,   127,   128,   400,   113,   114,   115,   116,   117,
       0,   404,   406,   125,   125,   125,   125,   188,     0,     0,
     145,   433,  -187,     0,   164,     0,   146,   147,   148,   125,
     441,   453,     0,   383,   453,     0,   134,   447,   127,   128,
       0,   453,     0,  -187,  -187,   456,   131,   329,   456,   135,
     137,     0,     0,   453,     0,   456,   437,     0,     0,   163,
     169,   171,   176,   179,   182,   449,   134,   456,   187,   189,
     438,     0,     0,   194,   196,   198,     0,     0,   203,   205,
       0,     0,   210,     0,   213,     0,   315,   317,   318,   222,
     481,   225,  -192,   227,   229,   231,   233,     0,   236,   125,
       0,     0,   240,   241,   244,   245,   246,     0,   248,   125,
       0,     0,   140,   491,   255,   492,   258,   259,   261,   262,
       0,   478,   267,   270,   212,   314,   508,   106,   107,   108,
     109,   489,   220,     0,   110,   111,   112,     0,     0,   134,
     113,   114,   115,   116,   117,   453,   126,   453,   441,     0,
       0,     0,   453,     0,     0,   126,     0,     0,     0,   456,
     336,   456,     0,   308,   453,     0,   456,   127,   128,   339,
     309,   113,   114,   115,   116,   117,   127,   128,   456,   126,
     473,     0,   474,   385,   386,   478,     0,   383,     0,     0,
     319,     0,     0,   358,   126,     0,  -192,  -192,  -192,  -192,
     127,   128,   326,  -192,  -192,  -192,   330,   126,   331,   332,
       0,     0,   552,   334,     0,   127,   128,     0,   453,   337,
       0,   450,   340,     0,   341,   126,   343,   344,   127,   128,
       0,   345,   456,     0,   349,     0,   351,     0,     0,   457,
       0,   354,   355,     0,     0,   356,   127,   128,   359,   360,
     361,     0,   362,   363,     0,   364,   366,     0,   368,     0,
       0,   371,     0,     0,   373,   374,     0,   375,     0,   376,
     118,   377,     0,   378,     0,   379,     0,     0,   384,     0,
     574,     0,   119,   387,   388,   190,   191,   192,   389,     0,
       0,   391,   392,     0,   126,   395,  -192,  -192,  -192,  -192,
       0,   118,     0,  -192,  -192,  -192,   401,     0,     0,   402,
       0,   134,   403,   405,     0,   127,   128,     0,   155,   155,
     155,   162,   167,     0,   155,   155,   155,   162,   155,     0,
       0,     0,  -192,     0,     0,   162,   155,     0,   418,   419,
     201,   126,   155,     0,   297,   298,     0,   214,   216,   218,
     162,   221,   223,   429,   430,   469,   155,   155,   155,   155,
     162,     0,   127,   128,   144,   126,   155,   145,   154,   154,
     154,   161,     0,   146,   147,   148,   451,   183,   154,   458,
     113,   114,   115,   116,   117,   193,   127,   128,   462,   304,
     305,   306,   307,   467,   468,     0,     0,     0,   470,     0,
     219,   113,   114,   115,   116,   117,     0,   122,     0,     0,
     235,   134,   132,   133,     0,     0,   106,   107,   108,   109,
       0,     0,   477,   110,   111,   112,     0,     0,     0,   113,
     114,   115,   116,   117,   483,   484,     0,   485,   486,   487,
     488,   200,   126,     0,  -192,  -192,  -192,  -192,   494,     0,
       0,  -192,  -192,  -192,     0,     0,     0,  -192,  -192,  -192,
    -192,  -192,     0,   127,   128,     0,     0,   509,     0,   511,
       0,     0,   512,     0,   513,     0,   514,   515,   254,   563,
     516,   155,   564,   565,   155,     0,   518,   519,     0,     0,
     520,     0,     0,     0,     0,   524,     0,   566,   567,   568,
     526,   527,   528,     0,   529,     0,     0,   532,     0,     0,
     155,     0,   534,   535,   536,   537,   538,   539,     0,     0,
     541,     0,   542,   543,     0,   302,     0,     0,     0,     0,
     544,   545,   546,   547,   548,     0,     0,     0,   310,   311,
       0,     0,   312,     0,   313,   106,   107,   108,   109,     0,
       0,     0,   110,   111,   112,     0,   152,     0,     0,   134,
       0,     0,   554,     0,     0,     0,   559,   304,   305,   306,
     307,     0,     0,     0,   562,     0,     0,   162,     0,   266,
       0,   106,   107,   108,   109,     0,     0,  -192,   110,   111,
     112,     0,     0,     0,   113,   114,   115,   116,   117,  -192,
     127,   128,     0,   352,     0,   353,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   162,   106,   107,   108,   109,
       0,   155,   162,   110,   111,   112,   162,   407,     0,   162,
       0,     0,     0,     0,     0,     0,     0,   155,   316,     0,
     106,   107,   108,   109,     0,     0,   162,   110,   111,   112,
       0,     0,     0,   113,   114,   115,   116,   117,     0,   394,
       0,     0,   396,   397,     0,   428,   398,     0,     0,   399,
       0,   154,   435,     0,     0,     0,   442,     0,     0,   446,
       0,   380,     0,   126,     0,   106,   107,   108,   109,     0,
     155,   151,   110,   111,   112,     0,   460,     0,   113,   114,
     115,   116,   117,   155,   127,   128,     0,     0,     0,     0,
     422,   423,   424,   425,   106,   107,   108,   109,     0,     0,
       0,   110,   111,   112,   118,     0,   436,   505,   506,   507,
       0,   162,     0,     0,     0,     0,   119,     0,     0,     0,
     154,   106,   107,   108,   109,     0,     0,     0,   110,   111,
     112,     0,     0,     0,   113,   114,   115,   116,   117,   106,
     107,   108,   109,     0,     0,     0,   110,   111,   112,     0,
       0,     0,   113,   114,   115,   116,   117,     0,     0,     0,
       0,   510,   126,   118,   106,   107,   108,   109,     0,     0,
     151,   110,   111,   112,     0,   119,   482,   113,   114,   115,
     116,   117,     0,   127,   128,     0,   490,     0,     0,     0,
       0,   557,     0,   106,   107,   108,   109,     0,     0,     0,
     110,   111,   112,     0,     0,   152,   113,   114,   115,   116,
     117,     0,     0,     0,     0,     0,   304,   305,   306,   307,
     408,   409,   410,   411,   412,   413,   106,   107,   108,   109,
       0,     0,   443,   110,   111,   112,   106,   107,   108,   109,
       0,     0,     0,   110,   111,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   118,     0,     0,     0,     0,     0,
       0,     0,   304,   305,   306,   307,   119,     0,     0,     0,
       0,     0,   118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   119,     0,     1,     0,     0,     0,
     452,     0,     2,     0,   152,     0,     0,     0,     0,   170,
       0,     3,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,     0,
       0,    19,    20,    21,    22,    23,   118,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,     0,
       0,     0,     0,   126,     0,   106,   107,   108,   109,     0,
       0,   151,   110,   111,   112,     0,   175,     0,   113,   114,
     115,   116,   117,     1,   127,   128,     0,     0,     0,     2,
       0,    86,     0,     0,     0,     0,     0,     0,     0,    87,
      88,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,     0,     0,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,   106,   107,   108,   109,
       0,     0,     0,   110,   111,   112,     0,     0,     0,   113,
     114,   115,   116,   117,     0,   152,     0,     0,     0,     0,
       0,     0,     0,   126,     0,   106,   107,   108,   109,     0,
       0,   151,   110,   111,   112,     0,    87,    88,   113,   114,
     115,   116,   117,     0,   127,   128,   126,     0,   106,   107,
     108,   109,     0,     0,     0,   110,   111,   112,     0,     0,
       0,   113,   114,   115,   116,   117,     0,   127,   128,  -171,
       0,  -171,  -171,  -171,  -171,     0,     0,  -171,  -171,  -171,
    -171,     0,     0,     0,  -171,  -171,  -171,  -171,  -171,   432,
       0,   106,   107,   108,   109,     0,     0,   151,   110,   111,
     112,     0,     0,   452,   113,   114,   115,   116,   117,   480,
       0,   106,   107,   108,   109,     0,     0,   151,   110,   111,
     112,     0,     0,     0,   113,   114,   115,   116,   117,     0,
       0,   434,     0,   106,   107,   108,   109,     0,     0,   118,
     110,   111,   112,     0,     0,     0,   113,   114,   115,   116,
     117,     0,     0,   106,   107,   108,   109,     0,     0,   151,
     110,   111,   112,     0,     0,   152,   113,   114,   115,   116,
     117,     0,     0,     0,     0,     0,   106,   107,   108,   109,
       0,     0,     0,   110,   111,   112,     0,     0,   152,   113,
     114,   115,   116,   117,     0,     0,     0,     0,   106,   107,
     108,   109,     0,     0,     0,   110,   111,   112,     0,     0,
       0,  -171,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   152,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   152,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   152,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   152,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   274,     0,   275,   276,     0,
       0,     0,     0,     0,   277,     0,   152,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294
};

static const yytype_int16 yycheck[] =
{
       1,    22,     1,    18,     1,     6,     7,     1,    14,    15,
      16,     1,   315,   350,     1,     1,   141,    38,    24,     1,
       1,   111,     1,    22,    23,    22,    23,     1,   118,     1,
     156,   157,    22,    23,    35,    22,    23,   139,     8,     9,
     339,    22,    23,    21,    22,     1,   141,   346,    22,    23,
      22,    23,   323,   139,    32,   139,   327,     1,   118,   358,
      38,   114,   115,    33,    34,     1,    22,    23,    24,    21,
       0,    72,    22,     1,    52,    53,    54,    55,    22,    23,
      81,   146,   385,    21,    62,    86,    22,    23,     1,    15,
       3,     4,     5,     6,    22,    23,   139,    10,    11,    12,
     118,   139,   142,    73,    74,     1,    76,   144,    78,    22,
      23,   143,     1,   143,    11,    12,   111,     1,   119,   111,
      16,    17,    18,    19,    20,   111,    22,    23,    25,    26,
     118,   132,   133,    22,    23,   136,   118,   138,    22,    23,
     141,   142,   143,   111,   111,   111,   140,    44,    45,   148,
     111,   450,    49,   452,    51,   152,   120,   139,   457,   121,
     147,   147,    59,    60,   154,   155,   156,   157,   121,    66,
     469,   118,   146,   145,    71,   154,   155,   156,   157,   149,
     159,   118,    79,   153,   154,   155,    83,    84,   111,   159,
     160,   111,   162,   530,   531,   116,   197,   141,   199,   177,
     170,   118,   180,   118,   174,   116,   142,    90,    80,     1,
       3,     4,     5,     6,   184,   329,   324,    10,    11,    12,
       1,   478,   192,    15,   523,   439,   139,    -1,   206,   118,
      22,    23,    39,    40,   235,    42,    -1,   238,   239,     1,
     153,    22,    23,    -1,   214,    -1,   216,    -1,   218,   219,
     146,   221,   253,    -1,    -1,   256,   257,    -1,    -1,   260,
      22,    23,   263,     1,    -1,   154,   155,   156,   157,    -1,
      -1,     1,    -1,   274,   275,    -1,   277,   247,    16,    17,
      18,    19,    20,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    -1,    22,    23,   264,    16,    17,    18,    19,    20,
      -1,   271,   272,   304,   305,   306,   307,     1,    -1,    -1,
       4,   317,     1,    -1,   329,    -1,    10,    11,    12,   320,
     321,   336,    -1,   324,   339,    -1,   118,   328,    22,    23,
      -1,   346,    -1,    22,    23,   336,     5,   118,   339,     8,
       9,    -1,    -1,   358,    -1,   346,   139,    -1,    -1,    18,
      19,    20,    21,    22,    23,   333,   118,   358,    27,    28,
     153,    -1,    -1,    32,    33,    34,    -1,    -1,    37,    38,
      -1,    -1,    41,    -1,    43,    -1,   141,   142,   143,    48,
     386,    50,   144,    52,    53,    54,    55,    -1,    57,   390,
      -1,    -1,    61,    62,    63,    64,    65,    -1,    67,   400,
      -1,    -1,   140,   404,    73,   406,    75,    76,    77,    78,
      -1,   381,    81,    82,   144,     1,   417,     3,     4,     5,
       6,   399,   143,    -1,    10,    11,    12,    -1,    -1,   118,
      16,    17,    18,    19,    20,   450,     1,   452,   439,    -1,
      -1,    -1,   457,    -1,    -1,     1,    -1,    -1,    -1,   450,
      15,   452,    -1,   122,   469,    -1,   457,    22,    23,    15,
     129,    16,    17,    18,    19,    20,    22,    23,   469,     1,
     367,    -1,   369,   238,   239,   445,    -1,   478,    -1,    -1,
     149,    -1,    -1,    15,     1,    -1,     3,     4,     5,     6,
      22,    23,   161,    10,    11,    12,   165,     1,   167,   168,
      -1,    -1,   503,   172,    -1,    22,    23,    -1,   523,   178,
      -1,    15,   181,    -1,   183,     1,   185,   186,    22,    23,
      -1,   190,   523,    -1,   193,    -1,   195,    -1,    -1,    15,
      -1,   200,   201,    -1,    -1,   204,    22,    23,   207,   208,
     209,    -1,   211,   212,    -1,   214,   215,    -1,   217,    -1,
      -1,   220,    -1,    -1,   223,   224,    -1,   226,    -1,   228,
     146,   230,    -1,   232,    -1,   234,    -1,    -1,   237,    -1,
     571,    -1,   158,   242,   243,    28,    29,    30,   247,    -1,
      -1,   250,   251,    -1,     1,   254,     3,     4,     5,     6,
      -1,   146,    -1,    10,    11,    12,   265,    -1,    -1,   268,
      -1,   118,   271,   272,    -1,    22,    23,    -1,    14,    15,
      16,    17,    18,    -1,    20,    21,    22,    23,    24,    -1,
      -1,    -1,   139,    -1,    -1,    31,    32,    -1,   297,   298,
      36,     1,    38,    -1,    87,    88,    -1,    43,    44,    45,
      46,    47,    48,   312,   313,    15,    52,    53,    54,    55,
      56,    -1,    22,    23,     1,     1,    62,     4,    14,    15,
      16,    17,    -1,    10,    11,    12,   335,    23,    24,   338,
      16,    17,    18,    19,    20,    31,    22,    23,   347,   154,
     155,   156,   157,   352,   353,    -1,    -1,    -1,   357,    -1,
      46,    16,    17,    18,    19,    20,    -1,     1,    -1,    -1,
      56,   118,     6,     7,    -1,    -1,     3,     4,     5,     6,
      -1,    -1,   381,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    19,    20,   393,   394,    -1,   396,   397,   398,
     399,    35,     1,    -1,     3,     4,     5,     6,   407,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    -1,    22,    23,    -1,    -1,   426,    -1,   428,
      -1,    -1,   431,    -1,   433,    -1,   435,   436,    72,   113,
     439,   177,   116,   117,   180,    -1,   445,   446,    -1,    -1,
     449,    -1,    -1,    -1,    -1,   454,    -1,   131,   132,   133,
     459,   460,   461,    -1,   463,    -1,    -1,   466,    -1,    -1,
     206,    -1,   471,   472,   473,   474,   475,   476,    -1,    -1,
     479,    -1,   481,   482,    -1,   119,    -1,    -1,    -1,    -1,
     489,   490,   491,   492,   493,    -1,    -1,    -1,   132,   133,
      -1,    -1,   136,    -1,   138,     3,     4,     5,     6,    -1,
      -1,    -1,    10,    11,    12,    -1,   143,    -1,    -1,   118,
      -1,    -1,   521,    -1,    -1,    -1,   525,   154,   155,   156,
     157,    -1,    -1,    -1,   533,    -1,    -1,   273,    -1,     1,
      -1,     3,     4,     5,     6,    -1,    -1,   146,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    19,    20,   158,
      22,    23,    -1,   197,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   311,     3,     4,     5,     6,
      -1,   317,   318,    10,    11,    12,   322,   273,    -1,   325,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   333,     1,    -1,
       3,     4,     5,     6,    -1,    -1,   342,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    -1,   253,
      -1,    -1,   256,   257,    -1,   311,   260,    -1,    -1,   263,
      -1,   317,   318,    -1,    -1,    -1,   322,    -1,    -1,   325,
      -1,   139,    -1,     1,    -1,     3,     4,     5,     6,    -1,
     386,     9,    10,    11,    12,    -1,   342,    -1,    16,    17,
      18,    19,    20,   399,    22,    23,    -1,    -1,    -1,    -1,
     304,   305,   306,   307,     3,     4,     5,     6,    -1,    -1,
      -1,    10,    11,    12,   146,    -1,   320,   114,   115,   116,
      -1,   427,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
     386,     3,     4,     5,     6,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    19,    20,     3,
       4,     5,     6,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    -1,    -1,    -1,
      -1,   427,     1,   146,     3,     4,     5,     6,    -1,    -1,
       9,    10,    11,    12,    -1,   158,   390,    16,    17,    18,
      19,    20,    -1,    22,    23,    -1,   400,    -1,    -1,    -1,
      -1,     1,    -1,     3,     4,     5,     6,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,   143,    16,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,   154,   155,   156,   157,
     129,   130,   131,   132,   133,   134,     3,     4,     5,     6,
      -1,    -1,     9,    10,    11,    12,     3,     4,     5,     6,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,   155,   156,   157,   158,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,     7,    -1,    -1,    -1,
     110,    -1,    13,    -1,   143,    -1,    -1,    -1,    -1,   148,
      -1,    22,    -1,    -1,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    42,    43,    44,    45,    46,   146,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    -1,
      -1,    -1,    -1,     1,    -1,     3,     4,     5,     6,    -1,
      -1,     9,    10,    11,    12,    -1,    14,    -1,    16,    17,
      18,    19,    20,     7,    22,    23,    -1,    -1,    -1,    13,
      -1,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     3,     4,     5,     6,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    -1,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,     3,     4,     5,     6,    -1,
      -1,     9,    10,    11,    12,    -1,   150,   151,    16,    17,
      18,    19,    20,    -1,    22,    23,     1,    -1,     3,     4,
       5,     6,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    -1,    22,    23,     1,
      -1,     3,     4,     5,     6,    -1,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    19,    20,     1,
      -1,     3,     4,     5,     6,    -1,    -1,     9,    10,    11,
      12,    -1,    -1,   110,    16,    17,    18,    19,    20,     1,
      -1,     3,     4,     5,     6,    -1,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    19,    20,    -1,
      -1,     1,    -1,     3,     4,     5,     6,    -1,    -1,   146,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    -1,    -1,     3,     4,     5,     6,    -1,    -1,     9,
      10,    11,    12,    -1,    -1,   143,    16,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,   143,    16,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   110,    -1,   112,   113,    -1,
      -1,    -1,    -1,    -1,   119,    -1,   143,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138
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
     104,   105,   106,   107,   108,   109,   142,   150,   151,   161,
     162,   164,   165,   166,   167,   168,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   208,     3,     4,     5,     6,
      10,    11,    12,    16,    17,    18,    19,    20,   146,   158,
     184,   193,   196,   203,   205,   206,     1,    22,    23,   152,
     163,   163,   196,   196,   118,   163,   192,   163,   192,     1,
     140,   181,   181,   181,     1,     4,    10,    11,    12,   187,
     204,     9,   143,   189,   191,   193,   194,   195,   206,   189,
     189,   191,   193,   163,   184,   185,   186,   193,   148,   163,
     148,   163,   188,   189,   191,    14,   163,   183,   188,   163,
     183,   188,   163,   191,   189,   181,   181,   163,     1,   163,
     187,   187,   187,   191,   163,   188,   163,   192,   163,   192,
     196,   193,   141,   163,   179,   163,   183,   188,   179,   179,
     163,   179,   144,   163,   193,   181,   193,   181,   193,   191,
     143,   193,   163,   193,   181,   163,   181,   163,   188,   163,
     188,   163,   188,   163,   188,   191,   163,   139,   181,   181,
     163,   163,   188,   145,   163,   163,   163,   181,   163,   141,
     180,   139,   139,   181,   196,   163,   192,   192,   163,   163,
     192,   163,   163,   192,   181,   180,     1,   163,   182,   196,
     163,   181,   181,    21,   110,   112,   113,   119,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   206,   210,   187,   187,     0,
     164,    22,   196,   146,   154,   155,   156,   157,   163,   163,
     196,   196,   196,   196,     1,   182,     1,   182,   182,   163,
     192,   192,   192,   192,   192,   192,   163,   192,    21,   118,
     163,   163,   163,   192,   163,   188,    15,   163,   188,    15,
     163,   163,   192,   163,   163,   163,    15,     1,   192,   163,
     169,   163,   196,   196,   163,   163,   163,   188,    15,   163,
     163,   163,   163,   163,   163,   192,   163,   192,   163,   192,
     192,   163,   192,   163,   163,   163,   163,   163,   163,   163,
     139,   199,   200,   206,   163,   182,   182,   163,   163,   163,
     192,   163,   163,   139,   196,   163,   196,   196,   196,   196,
     192,   163,   163,   163,   192,   163,   192,   191,   129,   130,
     131,   132,   133,   134,   206,   206,   206,   118,   163,   163,
       1,   159,   196,   196,   196,   196,   190,   192,   191,   163,
     163,   190,     1,   189,     1,   191,   196,   139,   153,   201,
     202,   206,   191,     9,   194,   199,   191,   206,   186,   188,
      15,   163,   110,   184,   197,   198,   206,    15,   163,   197,
     191,   197,   163,   139,   207,   208,   209,   163,   163,    15,
     163,   197,   144,   181,   181,   143,   143,   163,   192,   190,
       1,   189,   196,   163,   163,   163,   163,   163,   163,   188,
     196,   206,   206,    24,   163,   111,   111,   111,   111,   111,
     111,   111,   118,   118,   120,   114,   115,   116,   206,   163,
     191,   163,   163,   163,   163,   163,   163,   202,   163,   163,
     163,   197,   197,   147,   163,   197,   163,   163,   163,   163,
     121,   121,   163,   197,   163,   163,   163,   163,   163,   163,
     200,   163,   163,   163,   163,   163,   163,   163,   163,   118,
     114,   115,   206,   118,   163,     1,   111,     1,   197,   163,
     208,   208,   163,   113,   116,   117,   131,   132,   133,   111,
     111,   118,   116,   118,   206,   116
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1464 of yacc.c  */
#line 187 "mon_parse.y"
    { (yyval.i) = 0; }
    break;

  case 3:

/* Line 1464 of yacc.c  */
#line 188 "mon_parse.y"
    { (yyval.i) = 0; }
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 189 "mon_parse.y"
    { new_cmd = 1; asm_mode = 0;  (yyval.i) = 0; }
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 198 "mon_parse.y"
    { return ERR_EXPECT_END_CMD; }
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 213 "mon_parse.y"
    { return ERR_BAD_CMD; }
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 217 "mon_parse.y"
    { mon_bank(e_default_space, NULL); }
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 219 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (3)].i), NULL); }
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 221 "mon_parse.y"
    { mon_bank(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 223 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 225 "mon_parse.y"
    { mon_jump((yyvsp[(2) - (3)].a)); }
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 227 "mon_parse.y"
    { mon_display_io_regs(0); }
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 229 "mon_parse.y"
    { mon_display_io_regs((yyvsp[(2) - (3)].a)); }
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 231 "mon_parse.y"
    { monitor_cpu_type_set(""); }
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 233 "mon_parse.y"
    { monitor_cpu_type_set((yyvsp[(2) - (3)].str)); }
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 235 "mon_parse.y"
    { mon_cpuhistory(-1); }
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 237 "mon_parse.y"
    { mon_cpuhistory((yyvsp[(3) - (4)].i)); }
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 239 "mon_parse.y"
    { mon_instruction_return(); }
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 241 "mon_parse.y"
    { machine_write_snapshot((yyvsp[(2) - (3)].str),0,0,0); /* FIXME */ }
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 243 "mon_parse.y"
    { machine_read_snapshot((yyvsp[(2) - (3)].str), 0); }
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 245 "mon_parse.y"
    { mon_instructions_step(-1); }
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 247 "mon_parse.y"
    { mon_instructions_step((yyvsp[(3) - (4)].i)); }
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 249 "mon_parse.y"
    { mon_instructions_next(-1); }
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 251 "mon_parse.y"
    { mon_instructions_next((yyvsp[(3) - (4)].i)); }
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 253 "mon_parse.y"
    { mon_stack_up(-1); }
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 255 "mon_parse.y"
    { mon_stack_up((yyvsp[(3) - (4)].i)); }
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 257 "mon_parse.y"
    { mon_stack_down(-1); }
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 259 "mon_parse.y"
    { mon_stack_down((yyvsp[(3) - (4)].i)); }
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 261 "mon_parse.y"
    { mon_display_screen(); }
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 266 "mon_parse.y"
    { (monitor_cpu_for_memspace[default_memspace]->mon_register_print)(default_memspace); }
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 268 "mon_parse.y"
    { (monitor_cpu_for_memspace[(yyvsp[(2) - (3)].i)]->mon_register_print)((yyvsp[(2) - (3)].i)); }
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 273 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(4) - (5)].str));
                    }
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 278 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(2) - (3)].str));
                    }
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 283 "mon_parse.y"
    { mon_save_symbols((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 285 "mon_parse.y"
    { mon_save_symbols(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 287 "mon_parse.y"
    { mon_add_name_to_symbol_table((yyvsp[(2) - (5)].a), (yyvsp[(4) - (5)].str)); }
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 289 "mon_parse.y"
    { mon_remove_name_from_symbol_table(e_default_space, (yyvsp[(2) - (3)].str)); }
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 291 "mon_parse.y"
    { mon_remove_name_from_symbol_table((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 293 "mon_parse.y"
    { mon_print_symbol_table((yyvsp[(2) - (3)].i)); }
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 295 "mon_parse.y"
    { mon_print_symbol_table(e_default_space); }
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 297 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (4)].a), mon_prepend_dot_to_name((yyvsp[(1) - (4)].str)));
                    }
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 301 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (5)].a), mon_prepend_dot_to_name((yyvsp[(1) - (5)].str)));
                    }
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 307 "mon_parse.y"
    { mon_start_assemble_mode((yyvsp[(2) - (2)].a), NULL); }
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 308 "mon_parse.y"
    { }
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 310 "mon_parse.y"
    { mon_start_assemble_mode((yyvsp[(2) - (3)].a), NULL); }
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 312 "mon_parse.y"
    { mon_disassemble_lines((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1]); }
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 314 "mon_parse.y"
    { mon_disassemble_lines(BAD_ADDR, BAD_ADDR); }
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 318 "mon_parse.y"
    { mon_memory_move((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], (yyvsp[(4) - (5)].a)); }
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 320 "mon_parse.y"
    { mon_memory_compare((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], (yyvsp[(4) - (5)].a)); }
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 322 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); }
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 324 "mon_parse.y"
    { mon_memory_hunt((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); }
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 326 "mon_parse.y"
    { mon_memory_display((yyvsp[(2) - (5)].rt), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], DF_PETSCII); }
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 328 "mon_parse.y"
    { mon_memory_display(default_radix, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); }
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 330 "mon_parse.y"
    { mon_memory_display(default_radix, BAD_ADDR, BAD_ADDR, DF_PETSCII); }
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 332 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 8, 8); }
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 334 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 8, 8); }
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 336 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 24, 21); }
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 338 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 24, 21); }
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 340 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); }
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 342 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_PETSCII); }
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 344 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_SCREEN_CODE); }
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 346 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_SCREEN_CODE); }
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 348 "mon_parse.y"
    { mon_memmap_zap(); }
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 350 "mon_parse.y"
    { mon_memmap_show(-1,BAD_ADDR,BAD_ADDR); }
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 352 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (4)].i),BAD_ADDR,BAD_ADDR); }
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 354 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].range)[0],(yyvsp[(4) - (5)].range)[1]); }
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 356 "mon_parse.y"
    { mon_memmap_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); }
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 360 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], TRUE, e_exec, FALSE);
                  }
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 364 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], TRUE, e_exec, FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  }
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 369 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(3) - (4)].range)[0], (yyvsp[(3) - (4)].range)[1], TRUE, (yyvsp[(2) - (4)].i), FALSE);
                  }
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 373 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (6)].range)[0], (yyvsp[(3) - (6)].range)[1], TRUE, (yyvsp[(2) - (6)].i), FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(5) - (6)].cond_node));
                  }
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 378 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 381 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], TRUE, e_load | e_store, FALSE);
                  }
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 385 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], TRUE, e_load | e_store, FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  }
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 390 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(3) - (4)].range)[0], (yyvsp[(3) - (4)].range)[1], TRUE, (yyvsp[(2) - (4)].i), FALSE);
                  }
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 394 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (6)].range)[0], (yyvsp[(3) - (6)].range)[1], TRUE, (yyvsp[(2) - (6)].i), FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(5) - (6)].cond_node));
                  }
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 399 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 402 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], FALSE, e_load | e_store | e_exec, FALSE);
                  }
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 406 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], FALSE, e_load | e_store | e_exec, FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  }
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 411 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(3) - (4)].range)[0], (yyvsp[(3) - (4)].range)[1], FALSE, (yyvsp[(2) - (4)].i), FALSE);
                  }
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 415 "mon_parse.y"
    {
                      temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (6)].range)[0], (yyvsp[(3) - (6)].range)[1], FALSE, (yyvsp[(2) - (6)].i), FALSE);
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(5) - (6)].cond_node));
                  }
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 420 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 423 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], TRUE, e_exec, TRUE);
                  }
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 427 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); }
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 432 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_ON, (yyvsp[(2) - (3)].i)); }
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 434 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_OFF, (yyvsp[(2) - (3)].i)); }
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 436 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (3)].i), -1); }
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 438 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); }
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 440 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint((yyvsp[(2) - (3)].i)); }
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 442 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint(-1); }
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 444 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_condition((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].cond_node)); }
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 446 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_command((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); }
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 448 "mon_parse.y"
    { return ERR_EXPECT_STRING; }
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 452 "mon_parse.y"
    { sidefx = (((yyvsp[(2) - (3)].action) == e_TOGGLE) ? (sidefx ^ 1) : (yyvsp[(2) - (3)].action)); }
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 454 "mon_parse.y"
    {
                         mon_out("I/O side effects are %s\n",
                                   sidefx ? "enabled" : "disabled");
                     }
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 459 "mon_parse.y"
    { default_radix = (yyvsp[(2) - (3)].rt); }
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 461 "mon_parse.y"
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

  case 116:

/* Line 1464 of yacc.c  */
#line 479 "mon_parse.y"
    { monitor_change_device((yyvsp[(2) - (3)].i)); }
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 481 "mon_parse.y"
    { mon_export(); }
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 483 "mon_parse.y"
    { mon_quit(); YYACCEPT; }
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 485 "mon_parse.y"
    { exit_mon = 1; YYACCEPT; }
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 489 "mon_parse.y"
    { mon_drive_execute_disk_cmd((yyvsp[(2) - (3)].str)); }
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 491 "mon_parse.y"
    { mon_out("\t%d\n",(yyvsp[(2) - (3)].i)); }
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 493 "mon_parse.y"
    { mon_command_print_help(NULL); }
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 495 "mon_parse.y"
    { mon_command_print_help((yyvsp[(2) - (3)].str)); }
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 497 "mon_parse.y"
    { printf("SYSTEM COMMAND: %s\n",(yyvsp[(2) - (3)].str)); }
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 499 "mon_parse.y"
    { mon_print_convert((yyvsp[(2) - (3)].i)); }
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 501 "mon_parse.y"
    { mon_change_dir((yyvsp[(2) - (3)].str)); }
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 503 "mon_parse.y"
    { mon_keyboard_feed((yyvsp[(2) - (3)].str)); }
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 505 "mon_parse.y"
    { mon_backtrace(); }
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 507 "mon_parse.y"
    { mon_show_dir((yyvsp[(2) - (3)].str)); }
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 509 "mon_parse.y"
    { mon_show_pwd(); }
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 511 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (3)].str),-1); }
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 513 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); }
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 515 "mon_parse.y"
    { mon_resource_get((yyvsp[(2) - (3)].str)); }
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 517 "mon_parse.y"
    { mon_resource_set((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].str)); }
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 519 "mon_parse.y"
    { mon_reset_machine(-1); }
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 521 "mon_parse.y"
    { mon_reset_machine((yyvsp[(3) - (4)].i)); }
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 523 "mon_parse.y"
    { mon_tape_ctrl((yyvsp[(3) - (4)].i)); }
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 525 "mon_parse.y"
    { mon_cart_freeze(); }
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 527 "mon_parse.y"
    { }
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 531 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), FALSE); }
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 533 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), TRUE); }
    break;

  case 142:

/* Line 1464 of yacc.c  */
#line 535 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], FALSE); }
    break;

  case 143:

/* Line 1464 of yacc.c  */
#line 537 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; }
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 539 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 145:

/* Line 1464 of yacc.c  */
#line 541 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], TRUE); }
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 543 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 545 "mon_parse.y"
    { mon_file_verify((yyvsp[(2) - (5)].str),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 547 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; }
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 549 "mon_parse.y"
    { mon_drive_block_cmd(0,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 551 "mon_parse.y"
    { mon_drive_block_cmd(1,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); }
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 553 "mon_parse.y"
    { mon_drive_list(-1); }
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 555 "mon_parse.y"
    { mon_drive_list((yyvsp[(2) - (3)].i)); }
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 557 "mon_parse.y"
    { mon_attach((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].i)); }
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 559 "mon_parse.y"
    { mon_detach((yyvsp[(2) - (3)].i)); }
    break;

  case 155:

/* Line 1464 of yacc.c  */
#line 561 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (3)].str),0,1); }
    break;

  case 156:

/* Line 1464 of yacc.c  */
#line 563 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i),1); }
    break;

  case 157:

/* Line 1464 of yacc.c  */
#line 565 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (3)].str),0,0); }
    break;

  case 158:

/* Line 1464 of yacc.c  */
#line 567 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i),0); }
    break;

  case 159:

/* Line 1464 of yacc.c  */
#line 571 "mon_parse.y"
    { mon_record_commands((yyvsp[(2) - (3)].str)); }
    break;

  case 160:

/* Line 1464 of yacc.c  */
#line 573 "mon_parse.y"
    { mon_end_recording(); }
    break;

  case 161:

/* Line 1464 of yacc.c  */
#line 575 "mon_parse.y"
    { mon_playback_init((yyvsp[(2) - (3)].str)); }
    break;

  case 162:

/* Line 1464 of yacc.c  */
#line 579 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (4)].a), BAD_ADDR, (unsigned char *)(yyvsp[(3) - (4)].str)); }
    break;

  case 163:

/* Line 1464 of yacc.c  */
#line 581 "mon_parse.y"
    { printf("Not yet.\n"); }
    break;

  case 164:

/* Line 1464 of yacc.c  */
#line 585 "mon_parse.y"
    { yydebug = 1; }
    break;

  case 165:

/* Line 1464 of yacc.c  */
#line 588 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 166:

/* Line 1464 of yacc.c  */
#line 591 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 167:

/* Line 1464 of yacc.c  */
#line 592 "mon_parse.y"
    { (yyval.str) = NULL; }
    break;

  case 169:

/* Line 1464 of yacc.c  */
#line 596 "mon_parse.y"
    { return ERR_EXPECT_FILENAME; }
    break;

  case 171:

/* Line 1464 of yacc.c  */
#line 600 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; }
    break;

  case 172:

/* Line 1464 of yacc.c  */
#line 603 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 173:

/* Line 1464 of yacc.c  */
#line 604 "mon_parse.y"
    { (yyval.i) = e_load | e_store | e_exec; }
    break;

  case 174:

/* Line 1464 of yacc.c  */
#line 607 "mon_parse.y"
    { (yyval.i) = new_reg(default_memspace, (yyvsp[(1) - (1)].reg)); }
    break;

  case 175:

/* Line 1464 of yacc.c  */
#line 608 "mon_parse.y"
    { (yyval.i) = new_reg((yyvsp[(1) - (2)].i), (yyvsp[(2) - (2)].reg)); }
    break;

  case 178:

/* Line 1464 of yacc.c  */
#line 616 "mon_parse.y"
    { (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (3)].i))]->mon_register_set_val)(reg_memspace((yyvsp[(1) - (3)].i)), reg_regid((yyvsp[(1) - (3)].i)), (WORD) (yyvsp[(3) - (3)].i)); }
    break;

  case 179:

/* Line 1464 of yacc.c  */
#line 619 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 180:

/* Line 1464 of yacc.c  */
#line 620 "mon_parse.y"
    { return ERR_EXPECT_CHECKNUM; }
    break;

  case 182:

/* Line 1464 of yacc.c  */
#line 624 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (1)].a); (yyval.range)[1] = BAD_ADDR; }
    break;

  case 183:

/* Line 1464 of yacc.c  */
#line 627 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (3)].a); (yyval.range)[1] = (yyvsp[(3) - (3)].a); }
    break;

  case 184:

/* Line 1464 of yacc.c  */
#line 629 "mon_parse.y"
    { if (resolve_range(e_default_space, (yyval.range), (yyvsp[(1) - (1)].str))) return ERR_ADDR_TOO_BIG; }
    break;

  case 185:

/* Line 1464 of yacc.c  */
#line 631 "mon_parse.y"
    { if (resolve_range((yyvsp[(1) - (3)].i), (yyval.range), (yyvsp[(3) - (3)].str))) return ERR_ADDR_TOO_BIG; }
    break;

  case 186:

/* Line 1464 of yacc.c  */
#line 634 "mon_parse.y"
    { (yyval.a) = (yyvsp[(2) - (2)].a); }
    break;

  case 187:

/* Line 1464 of yacc.c  */
#line 635 "mon_parse.y"
    { (yyval.a) = BAD_ADDR; }
    break;

  case 188:

/* Line 1464 of yacc.c  */
#line 639 "mon_parse.y"
    {
             (yyval.a) = new_addr(e_default_space,(yyvsp[(1) - (1)].i));
             if (opt_asm) new_cmd = asm_mode = 1; 
         }
    break;

  case 189:

/* Line 1464 of yacc.c  */
#line 644 "mon_parse.y"
    {
             (yyval.a) = new_addr((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i));
             if (opt_asm) new_cmd = asm_mode = 1; 
         }
    break;

  case 190:

/* Line 1464 of yacc.c  */
#line 649 "mon_parse.y"
    {
             temp = mon_symbol_table_lookup_addr(e_default_space, (yyvsp[(1) - (1)].str));
             if (temp >= 0)
                 (yyval.a) = new_addr(e_default_space, temp);
             else
                 return ERR_UNDEFINED_LABEL;
         }
    break;

  case 193:

/* Line 1464 of yacc.c  */
#line 662 "mon_parse.y"
    { (yyval.i) = e_comp_space; }
    break;

  case 194:

/* Line 1464 of yacc.c  */
#line 663 "mon_parse.y"
    { (yyval.i) = e_disk8_space; }
    break;

  case 195:

/* Line 1464 of yacc.c  */
#line 664 "mon_parse.y"
    { (yyval.i) = e_disk9_space; }
    break;

  case 196:

/* Line 1464 of yacc.c  */
#line 665 "mon_parse.y"
    { (yyval.i) = e_disk10_space; }
    break;

  case 197:

/* Line 1464 of yacc.c  */
#line 666 "mon_parse.y"
    { (yyval.i) = e_disk11_space; }
    break;

  case 198:

/* Line 1464 of yacc.c  */
#line 669 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); if (!CHECK_ADDR((yyvsp[(1) - (1)].i))) return ERR_ADDR_TOO_BIG; }
    break;

  case 199:

/* Line 1464 of yacc.c  */
#line 671 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 200:

/* Line 1464 of yacc.c  */
#line 673 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) + (yyvsp[(3) - (3)].i); }
    break;

  case 201:

/* Line 1464 of yacc.c  */
#line 674 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) - (yyvsp[(3) - (3)].i); }
    break;

  case 202:

/* Line 1464 of yacc.c  */
#line 675 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) * (yyvsp[(3) - (3)].i); }
    break;

  case 203:

/* Line 1464 of yacc.c  */
#line 676 "mon_parse.y"
    { (yyval.i) = ((yyvsp[(3) - (3)].i)) ? ((yyvsp[(1) - (3)].i) / (yyvsp[(3) - (3)].i)) : 1; }
    break;

  case 204:

/* Line 1464 of yacc.c  */
#line 677 "mon_parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); }
    break;

  case 205:

/* Line 1464 of yacc.c  */
#line 678 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; }
    break;

  case 206:

/* Line 1464 of yacc.c  */
#line 679 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 207:

/* Line 1464 of yacc.c  */
#line 683 "mon_parse.y"
    {
               (yyval.cond_node) = new_cond; (yyval.cond_node)->is_parenthized = FALSE;
               (yyval.cond_node)->child1 = (yyvsp[(1) - (3)].cond_node); (yyval.cond_node)->child2 = (yyvsp[(3) - (3)].cond_node); (yyval.cond_node)->operation = (yyvsp[(2) - (3)].cond_op);
           }
    break;

  case 208:

/* Line 1464 of yacc.c  */
#line 688 "mon_parse.y"
    { return ERR_INCOMPLETE_COMPARE_OP; }
    break;

  case 209:

/* Line 1464 of yacc.c  */
#line 690 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(2) - (3)].cond_node); (yyval.cond_node)->is_parenthized = TRUE; }
    break;

  case 210:

/* Line 1464 of yacc.c  */
#line 692 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; }
    break;

  case 211:

/* Line 1464 of yacc.c  */
#line 694 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(1) - (1)].cond_node); }
    break;

  case 212:

/* Line 1464 of yacc.c  */
#line 697 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->reg_num = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = TRUE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          }
    break;

  case 213:

/* Line 1464 of yacc.c  */
#line 703 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->value = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = FALSE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          }
    break;

  case 216:

/* Line 1464 of yacc.c  */
#line 715 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); }
    break;

  case 217:

/* Line 1464 of yacc.c  */
#line 716 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); }
    break;

  case 220:

/* Line 1464 of yacc.c  */
#line 723 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); }
    break;

  case 221:

/* Line 1464 of yacc.c  */
#line 724 "mon_parse.y"
    { mon_add_number_masked_to_buffer((yyvsp[(1) - (1)].i), 0x00); }
    break;

  case 222:

/* Line 1464 of yacc.c  */
#line 725 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); }
    break;

  case 223:

/* Line 1464 of yacc.c  */
#line 728 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 224:

/* Line 1464 of yacc.c  */
#line 729 "mon_parse.y"
    { (yyval.i) = (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (1)].i))]->mon_register_get_val)(reg_memspace((yyvsp[(1) - (1)].i)), reg_regid((yyvsp[(1) - (1)].i))); }
    break;

  case 225:

/* Line 1464 of yacc.c  */
#line 732 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 226:

/* Line 1464 of yacc.c  */
#line 733 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 227:

/* Line 1464 of yacc.c  */
#line 734 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 228:

/* Line 1464 of yacc.c  */
#line 735 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); }
    break;

  case 229:

/* Line 1464 of yacc.c  */
#line 738 "mon_parse.y"
    { (yyval.i) = resolve_datatype(B_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 230:

/* Line 1464 of yacc.c  */
#line 739 "mon_parse.y"
    { (yyval.i) = resolve_datatype(O_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 231:

/* Line 1464 of yacc.c  */
#line 740 "mon_parse.y"
    { (yyval.i) = resolve_datatype(D_NUMBER,(yyvsp[(1) - (1)].str)); }
    break;

  case 232:

/* Line 1464 of yacc.c  */
#line 743 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 233:

/* Line 1464 of yacc.c  */
#line 744 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 234:

/* Line 1464 of yacc.c  */
#line 745 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 235:

/* Line 1464 of yacc.c  */
#line 746 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 236:

/* Line 1464 of yacc.c  */
#line 747 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); }
    break;

  case 240:

/* Line 1464 of yacc.c  */
#line 755 "mon_parse.y"
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

  case 242:

/* Line 1464 of yacc.c  */
#line 766 "mon_parse.y"
    { asm_mode = 0; }
    break;

  case 243:

/* Line 1464 of yacc.c  */
#line 769 "mon_parse.y"
    {
                        if ((yyvsp[(2) - (2)].i) > 0xffff) {
                          (yyval.i) = join_ints(ASM_ADDR_MODE_IMMEDIATE_24 | ((yyvsp[(2) - (2)].i) >> 16),((yyvsp[(2) - (2)].i) & 0xffff));
                        } else if ((yyvsp[(2) - (2)].i) > 0xff) {
                          (yyval.i) = join_ints(ASM_ADDR_MODE_IMMEDIATE_16,(yyvsp[(2) - (2)].i));
                        } else {
                          (yyval.i) = join_ints(ASM_ADDR_MODE_IMMEDIATE,(yyvsp[(2) - (2)].i));
                        }
                                       }
    break;

  case 244:

/* Line 1464 of yacc.c  */
#line 778 "mon_parse.y"
    { if ((yyvsp[(1) - (1)].i) >= 0x10000) {
               (yyval.i) = join_ints((ASM_ADDR_MODE_ABSOLUTE_LONG) | (((yyvsp[(1) - (1)].i) >> 16) << 8),((yyvsp[(1) - (1)].i) & 0xffff));
             } else if ((yyvsp[(1) - (1)].i) < 0x100) {
               (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE,(yyvsp[(1) - (1)].i));
             } else {
               (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE,(yyvsp[(1) - (1)].i));
             }
           }
    break;

  case 245:

/* Line 1464 of yacc.c  */
#line 786 "mon_parse.y"
    { if ((yyvsp[(1) - (3)].i) >= 0x10000) {
                            (yyval.i) = join_ints((ASM_ADDR_MODE_ABSOLUTE_LONG_X) | (((yyvsp[(1) - (3)].i) >> 16) << 8),((yyvsp[(1) - (3)].i) & 0xffff));
                          } else if ((yyvsp[(1) - (3)].i) < 0x100) {
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE_X,(yyvsp[(1) - (3)].i));
                          } else {
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_X,(yyvsp[(1) - (3)].i));
                          }
                        }
    break;

  case 246:

/* Line 1464 of yacc.c  */
#line 794 "mon_parse.y"
    { if ((yyvsp[(1) - (3)].i) < 0x100)
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE_Y,(yyvsp[(1) - (3)].i));
                          else
                            (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_Y,(yyvsp[(1) - (3)].i));
                        }
    break;

  case 247:

/* Line 1464 of yacc.c  */
#line 799 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_STACK_RELATIVE, (yyvsp[(1) - (3)].i)); }
    break;

  case 248:

/* Line 1464 of yacc.c  */
#line 800 "mon_parse.y"
    { if ((yyvsp[(2) - (6)].i) < 8)
                                                         (yyval.i) = join_ints((ASM_ADDR_MODE_ZERO_PAGE_BIT0_RELATIVE + (yyvsp[(2) - (6)].i)) | ((yyvsp[(4) - (6)].i) << 8),(yyvsp[(6) - (6)].i));
                                                   }
    break;

  case 249:

/* Line 1464 of yacc.c  */
#line 803 "mon_parse.y"
    { if ((yyvsp[(2) - (4)].i) < 8)
                                                         (yyval.i) = join_ints(ASM_ADDR_MODE_ZERO_PAGE_BIT0 + (yyvsp[(2) - (4)].i),(yyvsp[(4) - (4)].i));
                                      }
    break;

  case 250:

/* Line 1464 of yacc.c  */
#line 806 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_MOVE, ((yyvsp[(3) - (3)].i) << 8) | (yyvsp[(1) - (3)].i)); }
    break;

  case 251:

/* Line 1464 of yacc.c  */
#line 807 "mon_parse.y"
    { if ((yyvsp[(2) - (3)].i) < 0x100)
                               (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT,(yyvsp[(2) - (3)].i));
                             else
                               (yyval.i) = join_ints(ASM_ADDR_MODE_ABS_INDIRECT,(yyvsp[(2) - (3)].i));
                           }
    break;

  case 252:

/* Line 1464 of yacc.c  */
#line 812 "mon_parse.y"
    { if ((yyvsp[(2) - (3)].i) < 0x100)
                               (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_LONG,(yyvsp[(2) - (3)].i));
                             else
                               (yyval.i) = join_ints(ASM_ADDR_MODE_ABS_IND_LONG,(yyvsp[(2) - (3)].i));
                           }
    break;

  case 253:

/* Line 1464 of yacc.c  */
#line 817 "mon_parse.y"
    { if ((yyvsp[(2) - (5)].i) < 0x100)
                                           (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_X,(yyvsp[(2) - (5)].i));
                                         else
                                           (yyval.i) = join_ints(ASM_ADDR_MODE_ABS_INDIRECT_X,(yyvsp[(2) - (5)].i));
                                       }
    break;

  case 254:

/* Line 1464 of yacc.c  */
#line 823 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_Y,(yyvsp[(2) - (5)].i)); }
    break;

  case 255:

/* Line 1464 of yacc.c  */
#line 825 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_Z,(yyvsp[(2) - (5)].i)); }
    break;

  case 256:

/* Line 1464 of yacc.c  */
#line 827 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_STACK_RELATIVE_Y,(yyvsp[(2) - (7)].i)); }
    break;

  case 257:

/* Line 1464 of yacc.c  */
#line 829 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_INDIRECT_LONG_Y,(yyvsp[(2) - (5)].i)); }
    break;

  case 258:

/* Line 1464 of yacc.c  */
#line 830 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_BC,0); }
    break;

  case 259:

/* Line 1464 of yacc.c  */
#line 831 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_DE,0); }
    break;

  case 260:

/* Line 1464 of yacc.c  */
#line 832 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_HL,0); }
    break;

  case 261:

/* Line 1464 of yacc.c  */
#line 833 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_IX,0); }
    break;

  case 262:

/* Line 1464 of yacc.c  */
#line 834 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_IY,0); }
    break;

  case 263:

/* Line 1464 of yacc.c  */
#line 835 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IND_SP,0); }
    break;

  case 264:

/* Line 1464 of yacc.c  */
#line 837 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_A,(yyvsp[(2) - (5)].i)); }
    break;

  case 265:

/* Line 1464 of yacc.c  */
#line 839 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_HL,(yyvsp[(2) - (5)].i)); }
    break;

  case 266:

/* Line 1464 of yacc.c  */
#line 841 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_IX,(yyvsp[(2) - (5)].i)); }
    break;

  case 267:

/* Line 1464 of yacc.c  */
#line 843 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ABSOLUTE_IY,(yyvsp[(2) - (5)].i)); }
    break;

  case 268:

/* Line 1464 of yacc.c  */
#line 844 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_IMPLIED,0); }
    break;

  case 269:

/* Line 1464 of yacc.c  */
#line 845 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_ACCUMULATOR,0); }
    break;

  case 270:

/* Line 1464 of yacc.c  */
#line 846 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_B,0); }
    break;

  case 271:

/* Line 1464 of yacc.c  */
#line 847 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_C,0); }
    break;

  case 272:

/* Line 1464 of yacc.c  */
#line 848 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_D,0); }
    break;

  case 273:

/* Line 1464 of yacc.c  */
#line 849 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_E,0); }
    break;

  case 274:

/* Line 1464 of yacc.c  */
#line 850 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_H,0); }
    break;

  case 275:

/* Line 1464 of yacc.c  */
#line 851 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IXH,0); }
    break;

  case 276:

/* Line 1464 of yacc.c  */
#line 852 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IYH,0); }
    break;

  case 277:

/* Line 1464 of yacc.c  */
#line 853 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_L,0); }
    break;

  case 278:

/* Line 1464 of yacc.c  */
#line 854 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IXL,0); }
    break;

  case 279:

/* Line 1464 of yacc.c  */
#line 855 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IYL,0); }
    break;

  case 280:

/* Line 1464 of yacc.c  */
#line 856 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_AF,0); }
    break;

  case 281:

/* Line 1464 of yacc.c  */
#line 857 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_BC,0); }
    break;

  case 282:

/* Line 1464 of yacc.c  */
#line 858 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_DE,0); }
    break;

  case 283:

/* Line 1464 of yacc.c  */
#line 859 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_HL,0); }
    break;

  case 284:

/* Line 1464 of yacc.c  */
#line 860 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IX,0); }
    break;

  case 285:

/* Line 1464 of yacc.c  */
#line 861 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_IY,0); }
    break;

  case 286:

/* Line 1464 of yacc.c  */
#line 862 "mon_parse.y"
    { (yyval.i) = join_ints(ASM_ADDR_MODE_REG_SP,0); }
    break;



/* Line 1464 of yacc.c  */
#line 4472 "mon_parse.c"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



/* Line 1684 of yacc.c  */
#line 866 "mon_parse.y"


void parse_and_execute_line(char *input)
{
   char *temp_buf;
   int i, rc;

   temp_buf = lib_malloc(strlen(input) + 3);
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
         case ERR_EXPECT_CHECKNUM:
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


