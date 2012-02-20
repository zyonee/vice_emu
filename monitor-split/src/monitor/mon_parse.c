
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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
#define YYBISON_VERSION "2.4.1"

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

/* Substitute the variable and function names.  */
#define yyparse         mon_yyparse
#define yylex           mon_yylex
#define yyerror         mon_yyerror
#define yylval          mon_yylval
#define yychar          mon_yychar
#define yydebug         mon_yydebug
#define yynerrs         mon_yynerrs


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


static int yyerror(char *s);
static int temp;
static int resolve_datatype(unsigned guess_type, const char *num);
static int resolve_range(enum t_memspace memspace, MON_ADDR range[2],
                         const char *num);
static void mon_quit(void);

/* Defined in the lexer */
int yylex(void);

extern int new_cmd, opt_asm;

#define BAD_ADDR (new_addr(e_invalid_space, 0))
#define CHECK_ADDR(x) ((x) == addr_mask(x))

#define YYDEBUG 1



/* Line 189 of yacc.c  */
#line 171 "mon_parse.c"

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
     CMD_STOPWATCH = 361,
     RESET = 362,
     CMD_EXPORT = 363,
     CMD_AUTOSTART = 364,
     CMD_AUTOLOAD = 365,
     CMD_LABEL_ASGN = 366,
     L_PAREN = 367,
     R_PAREN = 368,
     ARG_IMMEDIATE = 369,
     REG_A = 370,
     REG_X = 371,
     REG_Y = 372,
     COMMA = 373,
     INST_SEP = 374,
     L_BRACKET = 375,
     R_BRACKET = 376,
     LESS_THAN = 377,
     REG_U = 378,
     REG_S = 379,
     REG_PC = 380,
     REG_PCR = 381,
     REG_B = 382,
     REG_C = 383,
     REG_D = 384,
     REG_E = 385,
     REG_H = 386,
     REG_L = 387,
     REG_AF = 388,
     REG_BC = 389,
     REG_DE = 390,
     REG_HL = 391,
     REG_IX = 392,
     REG_IY = 393,
     REG_SP = 394,
     REG_IXH = 395,
     REG_IXL = 396,
     REG_IYH = 397,
     REG_IYL = 398,
     PLUS = 399,
     MINUS = 400,
     STRING = 401,
     FILENAME = 402,
     R_O_L = 403,
     LABEL = 404,
     BANKNAME = 405,
     CPUTYPE = 406,
     MON_REGISTER = 407,
     COMPARE_OP = 408,
     RADIX_TYPE = 409,
     INPUT_SPEC = 410,
     CMD_CHECKPT_ON = 411,
     CMD_CHECKPT_OFF = 412,
     TOGGLE = 413,
     MASK = 414
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 92 "mon_parse.y"

    MON_ADDR a;
    MON_ADDR range[2];
    int i;
    REG_ID reg;
    CONDITIONAL cond_op;
    cond_node_t *cond_node;
    RADIXTYPE rt;
    ACTION action;
    char *str;
    asm_mode_addr_info_t mode;



/* Line 214 of yacc.c  */
#line 381 "mon_parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 393 "mon_parse.c"

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
#define YYFINAL  273
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1572

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  166
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  230
/* YYNRULES -- Number of states.  */
#define YYNSTATES  485

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   414

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     164,   165,   162,   160,     2,   161,     2,   163,     2,     2,
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
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    45,    49,    53,    59,    63,    66,    70,    73,
      77,    80,    85,    88,    92,    96,    99,   104,   107,   112,
     115,   120,   123,   128,   131,   133,   136,   140,   144,   150,
     154,   160,   164,   170,   174,   180,   184,   187,   192,   198,
     202,   207,   211,   214,   220,   226,   232,   238,   244,   248,
     251,   255,   258,   262,   265,   269,   272,   276,   279,   282,
     285,   290,   296,   302,   308,   311,   315,   318,   324,   327,
     333,   336,   340,   344,   348,   354,   358,   361,   367,   373,
     378,   382,   385,   389,   392,   396,   399,   402,   405,   409,
     413,   416,   420,   424,   428,   432,   436,   439,   443,   446,
     450,   456,   460,   465,   468,   473,   478,   481,   485,   489,
     492,   498,   504,   510,   514,   519,   525,   530,   536,   541,
     547,   553,   556,   560,   565,   569,   573,   579,   583,   589,
     593,   596,   600,   605,   608,   611,   613,   615,   616,   618,
     620,   622,   624,   627,   629,   631,   632,   634,   637,   641,
     643,   647,   649,   651,   653,   655,   659,   661,   665,   668,
     669,   671,   675,   677,   679,   680,   682,   684,   686,   688,
     690,   692,   694,   698,   702,   706,   710,   714,   718,   720,
     723,   724,   728,   732,   736,   740,   742,   744,   746,   750,
     752,   754,   756,   759,   761,   763,   765,   767,   769,   771,
     773,   775,   777,   779,   781,   783,   785,   787,   789,   791,
     793
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     167,     0,    -1,   168,    -1,    22,    -1,   170,    -1,   168,
     170,    -1,    23,    -1,    22,    -1,   171,    -1,   173,    -1,
     175,    -1,   174,    -1,   176,    -1,   177,    -1,   178,    -1,
     179,    -1,   180,    -1,   181,    -1,   182,    -1,   183,    -1,
      13,    -1,    67,   169,    -1,    67,   199,   169,    -1,    67,
     150,   169,    -1,    67,   199,   198,   150,   169,    -1,    38,
     197,   169,    -1,    46,   169,    -1,    46,   197,   169,    -1,
      87,   169,    -1,    87,   151,   169,    -1,   100,   169,    -1,
     100,   198,   202,   169,    -1,    26,   169,    -1,    49,   186,
     169,    -1,    50,   186,   169,    -1,    58,   169,    -1,    58,
     198,   202,   169,    -1,    57,   169,    -1,    57,   198,   202,
     169,    -1,    29,   169,    -1,    29,   198,   202,   169,    -1,
      30,   169,    -1,    30,   198,   202,   169,    -1,    85,   169,
      -1,   172,    -1,    39,   169,    -1,    39,   199,   169,    -1,
      39,   191,   169,    -1,    68,   199,   198,   186,   169,    -1,
      68,   186,   169,    -1,    69,   199,   198,   186,   169,    -1,
      69,   186,   169,    -1,    70,   197,   198,   149,   169,    -1,
      71,   149,   169,    -1,    71,   199,   198,   149,   169,    -1,
      72,   199,   169,    -1,    72,   169,    -1,   111,    21,   197,
     169,    -1,   111,    21,   197,    24,   169,    -1,    55,   197,
     169,    -1,    55,   197,     1,   169,    -1,    56,   194,   169,
      -1,    56,   169,    -1,    37,   195,   198,   197,   169,    -1,
      48,   195,   198,   197,   169,    -1,    36,   195,   198,   206,
     169,    -1,    35,   195,   198,   208,   169,    -1,    43,   154,
     198,   194,   169,    -1,    43,   194,   169,    -1,    43,   169,
      -1,    76,   194,   169,    -1,    76,   169,    -1,    77,   194,
     169,    -1,    77,   169,    -1,    78,   194,   169,    -1,    78,
     169,    -1,    79,   194,   169,    -1,    79,   169,    -1,   101,
     169,    -1,   102,   169,    -1,   102,   198,   202,   169,    -1,
     102,   198,   202,   194,   169,    -1,   103,   186,   198,   202,
     169,    -1,    44,   189,   194,   203,   169,    -1,    44,   169,
      -1,    86,   194,   169,    -1,    86,   169,    -1,    62,   189,
     194,   203,   169,    -1,    62,   169,    -1,    45,   189,   194,
     203,   169,    -1,    45,   169,    -1,   156,   193,   169,    -1,
     157,   193,   169,    -1,    34,   193,   169,    -1,    34,   193,
     198,   202,   169,    -1,    52,   193,   169,    -1,    52,   169,
      -1,    53,   193,    15,   204,   169,    -1,    54,   193,   198,
     146,   169,    -1,    54,   193,     1,   169,    -1,    25,   158,
     169,    -1,    25,   169,    -1,    42,   154,   169,    -1,    42,
     169,    -1,    60,   199,   169,    -1,   108,   169,    -1,    65,
     169,    -1,    51,   169,    -1,    63,   184,   169,    -1,    59,
     202,   169,    -1,    61,   169,    -1,    61,   184,   169,    -1,
      64,   184,   169,    -1,     7,   202,   169,    -1,    66,   184,
     169,    -1,    82,   184,   169,    -1,    89,   169,    -1,    92,
     185,   169,    -1,    91,   169,    -1,    90,   186,   169,    -1,
      90,   186,   198,   202,   169,    -1,    93,   146,   169,    -1,
      94,   146,   146,   169,    -1,    97,   169,    -1,    97,   198,
     202,   169,    -1,    98,   198,   202,   169,    -1,    99,   169,
      -1,   104,   185,   169,    -1,   106,   107,   169,    -1,   106,
     169,    -1,    31,   186,   187,   196,   169,    -1,    83,   186,
     187,   196,   169,    -1,    32,   186,   187,   195,   169,    -1,
      32,   186,     1,    -1,    32,   186,   187,     1,    -1,    84,
     186,   187,   195,   169,    -1,    84,   186,   187,     1,    -1,
      33,   186,   187,   197,   169,    -1,    33,   186,   187,     1,
      -1,    27,   202,   202,   196,   169,    -1,    28,   202,   202,
     197,   169,    -1,   105,   169,    -1,   105,   187,   169,    -1,
      95,   186,   202,   169,    -1,    96,   202,   169,    -1,   109,
     186,   169,    -1,   109,   186,   198,   213,   169,    -1,   110,
     186,   169,    -1,   110,   186,   198,   213,   169,    -1,    73,
     186,   169,    -1,    74,   169,    -1,    75,   186,   169,    -1,
      80,   197,   206,   169,    -1,    81,   169,    -1,    88,   169,
      -1,   148,    -1,   148,    -1,    -1,   147,    -1,     1,    -1,
     202,    -1,     1,    -1,   188,    14,    -1,    14,    -1,   188,
      -1,    -1,   152,    -1,   199,   152,    -1,   191,   118,   192,
      -1,   192,    -1,   190,    21,   213,    -1,   211,    -1,     1,
      -1,   195,    -1,   197,    -1,   197,   198,   197,    -1,     9,
      -1,   199,   198,     9,    -1,   198,   197,    -1,    -1,   200,
      -1,   199,   198,   200,    -1,   149,    -1,   118,    -1,    -1,
      16,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
     201,    -1,   213,    -1,   202,   160,   202,    -1,   202,   161,
     202,    -1,   202,   162,   202,    -1,   202,   163,   202,    -1,
     164,   202,   165,    -1,   164,   202,     1,    -1,   210,    -1,
      15,   204,    -1,    -1,   204,   153,   204,    -1,   204,   153,
       1,    -1,   112,   204,   113,    -1,   112,   204,     1,    -1,
     205,    -1,   190,    -1,   213,    -1,   206,   198,   207,    -1,
     207,    -1,   213,    -1,   146,    -1,   208,   209,    -1,   209,
      -1,   213,    -1,   159,    -1,   146,    -1,   213,    -1,   190,
      -1,     4,    -1,    12,    -1,    11,    -1,    10,    -1,    12,
      -1,    11,    -1,    10,    -1,     3,    -1,     4,    -1,     5,
      -1,     6,    -1,   212,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   154,   154,   155,   158,   159,   162,   163,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   181,   183,   185,   187,   189,   191,   193,   195,   197,
     199,   201,   203,   205,   207,   209,   211,   213,   215,   217,
     219,   221,   223,   225,   227,   230,   232,   234,   237,   242,
     247,   249,   251,   253,   255,   257,   259,   261,   265,   271,
     273,   275,   277,   281,   283,   285,   287,   289,   291,   293,
     295,   297,   299,   301,   303,   305,   307,   309,   311,   313,
     315,   317,   319,   323,   332,   335,   339,   342,   351,   354,
     363,   368,   370,   372,   374,   376,   378,   380,   382,   384,
     388,   390,   395,   397,   415,   417,   419,   421,   425,   427,
     429,   431,   433,   435,   437,   439,   441,   443,   445,   447,
     449,   451,   453,   455,   457,   459,   461,   463,   465,   467,
     471,   473,   475,   477,   479,   481,   483,   485,   487,   489,
     491,   493,   495,   497,   499,   501,   503,   505,   507,   511,
     513,   515,   519,   521,   525,   529,   532,   533,   536,   537,
     540,   541,   544,   545,   548,   549,   552,   553,   556,   557,
     560,   564,   565,   568,   569,   572,   573,   575,   579,   580,
     583,   587,   591,   601,   602,   605,   606,   607,   608,   609,
     612,   614,   616,   617,   618,   619,   620,   621,   622,   625,
     626,   628,   633,   635,   637,   639,   643,   649,   657,   658,
     661,   662,   665,   666,   669,   670,   671,   674,   675,   678,
     679,   680,   681,   684,   685,   686,   689,   690,   691,   692,
     693
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
  "CMD_LIST", "CMD_STOPWATCH", "RESET", "CMD_EXPORT", "CMD_AUTOSTART",
  "CMD_AUTOLOAD", "CMD_LABEL_ASGN", "L_PAREN", "R_PAREN", "ARG_IMMEDIATE",
  "REG_A", "REG_X", "REG_Y", "COMMA", "INST_SEP", "L_BRACKET", "R_BRACKET",
  "LESS_THAN", "REG_U", "REG_S", "REG_PC", "REG_PCR", "REG_B", "REG_C",
  "REG_D", "REG_E", "REG_H", "REG_L", "REG_AF", "REG_BC", "REG_DE",
  "REG_HL", "REG_IX", "REG_IY", "REG_SP", "REG_IXH", "REG_IXL", "REG_IYH",
  "REG_IYL", "PLUS", "MINUS", "STRING", "FILENAME", "R_O_L", "LABEL",
  "BANKNAME", "CPUTYPE", "MON_REGISTER", "COMPARE_OP", "RADIX_TYPE",
  "INPUT_SPEC", "CMD_CHECKPT_ON", "CMD_CHECKPT_OFF", "TOGGLE", "MASK",
  "'+'", "'-'", "'*'", "'/'", "'('", "')'", "$accept", "top_level",
  "command_list", "end_cmd", "command", "machine_state_rules",
  "register_mod", "symbol_table_rules", "asm_rules", "memory_rules",
  "checkpoint_rules", "checkpoint_control_rules", "monitor_state_rules",
  "monitor_misc_rules", "disk_rules", "cmd_file_rules", "data_entry_rules",
  "monitor_debug_rules", "rest_of_line", "opt_rest_of_line", "filename",
  "device_num", "mem_op", "opt_mem_op", "register", "reg_list", "reg_asgn",
  "checkpt_num", "address_opt_range", "address_range", "opt_address",
  "address", "opt_sep", "memspace", "memloc", "memaddr", "expression",
  "opt_if_cond_expr", "cond_expr", "compare_operand", "data_list",
  "data_element", "hunt_list", "hunt_element", "value", "d_number",
  "guess_default", "number", 0
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
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
      43,    45,    42,    47,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   166,   167,   167,   168,   168,   169,   169,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   172,   172,   172,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   173,   174,
     174,   174,   174,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   176,   176,   176,   176,   176,   176,   176,
     176,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     178,   178,   178,   178,   178,   178,   178,   178,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   181,
     181,   181,   182,   182,   183,   184,   185,   185,   186,   186,
     187,   187,   188,   188,   189,   189,   190,   190,   191,   191,
     192,   193,   193,   194,   194,   195,   195,   195,   196,   196,
     197,   197,   197,   198,   198,   199,   199,   199,   199,   199,
     200,   201,   202,   202,   202,   202,   202,   202,   202,   203,
     203,   204,   204,   204,   204,   204,   205,   205,   206,   206,
     207,   207,   208,   208,   209,   209,   209,   210,   210,   211,
     211,   211,   211,   212,   212,   212,   213,   213,   213,   213,
     213
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     3,     5,     3,     2,     3,     2,     3,
       2,     4,     2,     3,     3,     2,     4,     2,     4,     2,
       4,     2,     4,     2,     1,     2,     3,     3,     5,     3,
       5,     3,     5,     3,     5,     3,     2,     4,     5,     3,
       4,     3,     2,     5,     5,     5,     5,     5,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     2,     2,
       4,     5,     5,     5,     2,     3,     2,     5,     2,     5,
       2,     3,     3,     3,     5,     3,     2,     5,     5,     4,
       3,     2,     3,     2,     3,     2,     2,     2,     3,     3,
       2,     3,     3,     3,     3,     3,     2,     3,     2,     3,
       5,     3,     4,     2,     4,     4,     2,     3,     3,     2,
       5,     5,     5,     3,     4,     5,     4,     5,     4,     5,
       5,     2,     3,     4,     3,     3,     5,     3,     5,     3,
       2,     3,     4,     2,     2,     1,     1,     0,     1,     1,
       1,     1,     2,     1,     1,     0,     1,     2,     3,     1,
       3,     1,     1,     1,     1,     3,     1,     3,     2,     0,
       1,     3,     1,     1,     0,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     1,     2,
       0,     3,     3,     3,     3,     1,     1,     1,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    20,     3,     0,     0,     0,     0,   184,   184,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   165,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   184,     0,     0,     0,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   157,     0,
       0,     0,     0,   184,   184,     0,   184,     0,   184,     0,
     157,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     4,     8,    44,     9,    11,    10,    12,    13,    14,
      15,    16,    17,    18,    19,   226,   227,   228,   229,   225,
     224,   223,   185,   186,   187,   188,   189,   166,     0,   218,
       0,     0,   198,   230,   217,     7,     6,     0,   101,    32,
       0,     0,   183,    39,     0,    41,     0,   159,   158,     0,
       0,     0,   172,   219,   222,   221,   220,   184,   171,   176,
     182,   184,   184,   184,   180,   190,   191,   184,   184,     0,
     184,    45,     0,     0,   169,     0,     0,   103,   184,    69,
       0,   173,   184,   163,    84,   164,     0,    90,     0,    26,
       0,   184,     0,     0,   107,    96,     0,     0,     0,     0,
      62,     0,    37,     0,    35,     0,     0,     0,   155,   110,
       0,    88,     0,     0,     0,   106,     0,     0,    21,   184,
       0,   184,     0,   184,   184,     0,   184,    56,     0,     0,
     150,     0,    71,     0,    73,     0,    75,     0,    77,     0,
       0,   153,     0,     0,     0,    43,    86,     0,     0,    28,
     154,   116,   184,   118,   156,     0,     0,     0,     0,     0,
     123,     0,     0,   126,    30,     0,    78,    79,     0,   184,
       0,   161,   141,     0,   160,     0,   129,   105,   184,   184,
       0,     0,     0,     1,     5,     0,   167,     0,     0,     0,
       0,   113,   100,   184,     0,     0,     0,   184,   133,     0,
       0,    93,     0,     0,     0,     0,     0,     0,    25,     0,
       0,     0,    47,    46,   102,     0,    68,   162,   200,   200,
      27,     0,    33,    34,    95,     0,     0,     0,     0,    59,
      61,     0,     0,   109,   104,   111,   200,   108,   112,   114,
      23,    22,     0,    49,     0,    51,     0,     0,    53,     0,
      55,   149,   151,    70,    72,    74,    76,   211,   184,   209,
     210,   115,   184,     0,    85,    29,   119,     0,   117,   121,
       0,     0,   144,     0,     0,     0,     0,     0,   127,   142,
     128,   145,     0,   147,     0,     0,    91,    92,   197,   196,
     192,   193,   194,   195,     0,     0,     0,    40,    42,     0,
     134,     0,   138,     0,     0,   216,   215,     0,   213,   214,
     175,   177,   181,   184,     0,   170,   168,     0,     0,     0,
       0,     0,     0,   206,     0,   205,   207,    99,     0,    60,
      38,    36,     0,     0,     0,     0,     0,     0,   152,     0,
       0,   136,     0,     0,   122,   143,   124,   125,    31,    80,
       0,     0,     0,     0,     0,    57,   139,   178,   140,   130,
     132,   137,    94,    66,   212,    65,    63,    67,   199,    83,
      89,    64,     0,     0,    97,    98,    87,    24,    48,    50,
      52,    54,   208,   131,   135,   120,    81,    82,   146,   148,
      58,   204,   203,   202,   201
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    89,    90,   128,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   200,   245,
     139,   263,   175,   176,   119,   163,   164,   147,   170,   171,
     384,   172,   385,   120,   154,   155,   264,   409,   414,   415,
     348,   349,   397,   398,   122,   148,   123,   156
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -322
static const yytype_int16 yypact[] =
{
    1070,   857,  -322,  -322,    18,    96,   857,   857,   130,   130,
       2,     2,     2,   501,  1405,  1405,  1405,  1423,   341,    -3,
     904,   220,   220,  1382,  1405,     2,     2,    96,   487,   501,
     501,  1423,  1361,   130,   130,   857,   594,    80,   220,  -135,
    -135,    96,  -135,   325,   206,   206,  1423,   269,   498,     2,
      96,     2,  1361,  1361,  1361,  1361,  1423,    96,  -135,     2,
       2,    96,  1361,    39,    96,    96,     2,    96,  -127,  -113,
    -111,     2,   857,   130,   -76,    96,   130,    96,   130,     2,
    -127,    89,   187,    96,     2,     2,    26,   501,   501,    54,
    1186,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,   857,  -322,
     -66,   103,  -322,  -322,  -322,  -322,  -322,    96,  -322,  -322,
     695,   695,  -322,  -322,   857,  -322,   857,  -322,  -322,   299,
     466,   299,  -322,  -322,  -322,  -322,  -322,   130,  -322,  -322,
    -322,   -76,   -76,   -76,  -322,  -322,  -322,   -76,   -76,    96,
     -76,  -322,    83,   163,  -322,    16,    96,  -322,   -76,  -322,
      96,  -322,   122,  -322,  -322,    82,  1405,  -322,  1405,  -322,
      96,   -76,    96,    96,  -322,  -322,    96,   113,    13,   238,
    -322,    96,  -322,   857,  -322,   857,   103,    96,  -322,  -322,
      96,  -322,  1405,    96,    96,  -322,    96,    96,  -322,   130,
      96,   -76,    96,   -76,   -76,    96,   -76,  -322,    96,    96,
    -322,    96,  -322,    96,  -322,    96,  -322,    96,  -322,    96,
     318,  -322,    96,   299,   299,  -322,  -322,    96,    96,  -322,
    -322,  -322,   130,  -322,  -322,    96,    96,   -14,   857,   103,
    -322,   857,   857,  -322,  -322,   857,  -322,  -322,   857,   -76,
      96,  -322,  -322,    96,   214,    96,  -322,  -322,   130,   130,
    1423,    96,    96,  -322,  -322,    17,  -322,   857,   857,   857,
     857,  -322,  -322,    75,    40,   103,   103,   165,   632,  1185,
    1343,  -322,   857,   110,  1423,   802,   318,  1423,  -322,   819,
     819,   436,  -322,  -322,  -322,  1405,  -322,  -322,   119,   119,
    -322,  1423,  -322,  -322,  -322,  1295,    96,   -10,    96,  -322,
    -322,   103,   103,  -322,  -322,  -322,   119,  -322,  -322,  -322,
    -322,  -322,    -9,  -322,     2,  -322,     2,    -2,  -322,     8,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,   130,  -322,
    -322,  -322,   165,  1319,  -322,  -322,  -322,   857,  -322,  -322,
      96,   103,  -322,   103,   103,   103,   883,   857,  -322,  -322,
    -322,  -322,   819,  -322,   819,   175,  -322,  -322,  -322,  -322,
     -39,   -39,  -322,  -322,    96,  1423,    96,  -322,  -322,    96,
    -322,    96,  -322,    96,   103,  -322,  -322,    62,  -322,  -322,
    -322,  -322,  -322,   130,    96,  -322,  -322,    96,  1295,    96,
      96,    96,  1295,  -322,     5,  -322,  -322,  -322,    96,  -322,
    -322,  -322,    96,    96,    96,    96,    96,    96,  -322,   318,
      96,  -322,    96,   103,  -322,  -322,  -322,  -322,  -322,  -322,
      96,   103,    96,    96,    96,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,     9,  -322,
    -322,  -322,    30,   959,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -322,  -322,  -322,   360,    79,  -322,  -322,  -322,  -322,  -322,
    -322,  -322,  -322,  -322,  -322,  -322,  -322,  -322,   292,    90,
       4,   319,  -322,   -12,   -17,  -322,  -106,   297,   -30,    -7,
    -283,   150,     3,   641,  -217,  -322,   703,  -199,  -321,  -322,
     -92,  -224,  -322,  -186,  -322,  -322,  -322,    -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -185
static const yytype_int16 yytable[] =
{
     124,   162,   191,   137,   389,   124,   124,   151,   157,   158,
     178,   134,   136,   198,   316,   140,   141,   181,   378,   125,
     126,   244,   223,   225,   227,   229,   202,   125,   126,   182,
     183,   481,   237,   246,   124,   247,   193,   195,   125,   126,
     125,   126,   132,   105,   106,   107,   108,   270,   210,   212,
     109,   110,   111,   219,   273,   221,   112,   113,   114,   115,
     116,   125,   126,   233,   234,   105,   106,   107,   108,   430,
     242,   124,   109,   110,   111,   248,   251,   252,   402,   255,
     124,   258,   402,   259,   125,   126,   276,   458,   268,   269,
     261,   462,   105,   106,   107,   108,   307,  -179,  -179,   109,
     110,   111,   125,   126,   300,   112,   113,   114,   115,   116,
     410,   125,   126,   105,   106,   107,   108,   124,   125,   126,
     109,   110,   111,   279,   280,   125,   126,   422,   315,   124,
     124,   132,   360,   124,   408,   124,   418,  -174,   124,   124,
     124,   423,   484,   482,  -174,  -174,   308,   426,   309,   138,
     292,   166,   125,   126,   293,   294,   295,   427,   463,  -184,
     296,   297,   463,   299,   152,   152,   152,   159,   276,   274,
     260,   305,   326,   180,   152,   294,   127,   277,   278,   279,
     280,   189,   379,   463,   311,   125,   126,  -179,  -179,   150,
     238,   317,   124,   132,   124,   406,   214,   125,   126,   444,
     277,   278,   279,   280,   403,   472,   230,   137,   395,   125,
     126,   454,   332,     0,   334,     0,   336,   337,     0,   339,
       0,   396,   112,   113,   114,   115,   116,     0,   198,   350,
       0,     0,   124,   124,   173,   277,   278,   279,   280,   318,
     132,   117,   125,   126,     0,   357,     0,   124,   132,     0,
     124,   124,     0,   118,   124,     0,   395,   124,     0,     0,
     125,   126,   367,   277,   278,   279,   280,     0,     0,   396,
       0,   372,   374,     0,     0,   407,   124,   124,   124,   124,
       0,   301,   391,   132,   162,   112,   113,   114,   115,   116,
       0,   124,   399,     0,   265,   350,     0,     0,   413,   405,
     261,     0,   105,   106,   107,   108,     0,     0,     0,   109,
     110,   111,     0,     0,   416,   112,   113,   114,   115,   116,
       0,   105,   106,   107,   108,   186,   187,   188,   109,   110,
     111,   203,   204,     0,   206,     0,   440,     0,   424,     0,
     425,   112,   113,   114,   115,   116,   432,   125,   126,     0,
     232,   429,     0,   138,     0,     0,   124,   112,   113,   114,
     115,   116,     0,   125,   126,   129,   124,     0,   133,   135,
       0,   442,     0,   443,   277,   278,   279,   280,   161,   167,
     169,   174,   177,   179,   271,   272,     0,   184,   185,     0,
       0,   413,   190,   192,   194,   413,   399,   199,   201,     0,
       0,   205,     0,   208,     0,     0,   429,   416,   217,     0,
     220,   416,   222,   224,   226,   228,     0,   231,   215,     0,
     375,   235,   236,   239,   240,   241,     0,   243,   350,     0,
       0,     0,     0,   250,   386,   253,   254,   256,   257,   152,
     393,   262,   266,   267,   400,     0,   413,   404,     0,     0,
       0,   117,   112,   113,   114,   115,   116,     0,   287,   289,
     290,   411,   416,   118,   347,     0,     0,   288,     0,   105,
     106,   107,   108,     0,     0,   207,   109,   110,   111,     0,
       0,   281,   112,   113,   114,   115,   116,   282,   142,     0,
       0,   143,     0,   117,     0,     0,     0,   144,   145,   146,
       0,     0,   142,   152,     0,   143,     0,   291,     0,   125,
     126,   144,   145,   146,   112,   113,   114,   115,   116,   298,
     125,   126,     0,   302,     0,   303,   304,     0,     0,     0,
     306,     0,     0,     0,     0,   447,     0,     0,     0,     0,
     310,     0,   312,   313,     0,     0,   314,     0,     0,   319,
       0,   320,   352,   353,     0,     0,   323,   324,     0,     0,
     325,     0,     0,   327,   328,     0,   329,   330,     0,   331,
     333,     0,   335,     0,     0,   338,     0,     0,   340,   341,
       0,   342,     0,   343,     0,   344,     0,   345,   117,   346,
       0,     0,   351,     0,     0,     0,     0,   354,   355,     0,
       0,     0,   356,     0,     0,   358,   359,     0,     0,   362,
     112,   113,   114,   115,   116,     0,     0,     0,   117,     0,
     368,     0,     0,   369,     0,   370,     0,     0,   371,   373,
     118,   376,   377,  -161,     0,  -161,  -161,  -161,  -161,     0,
       0,  -161,  -161,  -161,  -161,   387,   388,     0,  -161,  -161,
    -161,  -161,  -161,     0,     0,   153,   153,   153,   160,   165,
       0,   153,     0,     0,   160,   153,     0,     0,     0,     0,
       0,     0,   160,   153,     0,     0,   417,   197,   419,     0,
       0,   420,   421,     0,   209,   211,   213,   160,   216,   218,
       0,     0,     0,   153,   153,   153,   153,   160,   105,   106,
     107,   108,     0,   153,   121,   109,   110,   111,   428,   130,
     131,   112,   113,   114,   115,   116,     0,     0,     0,     0,
     434,   435,     0,   436,   437,   438,   439,     0,     0,     0,
       0,     0,     0,     0,     0,   445,     0,     0,   196,     0,
       0,     0,     0,     0,   446,     0,   448,     0,     0,   449,
       0,   450,     0,   451,   452,     0,     0,   453,     0,     0,
       0,     0,     0,   455,   456,     0,     0,   457,     0,   459,
     460,   461,     0,     0,   464,   249,     0,     0,   465,     0,
       0,  -161,   466,   467,   468,   469,   470,   471,     0,     0,
     473,     0,   474,   475,     0,     0,     0,     0,     0,     0,
     476,   477,   478,   479,   480,   105,   106,   107,   108,     0,
       0,   401,   109,   110,   111,     0,     0,   153,     0,   153,
       0,   275,   105,   106,   107,   108,     0,     0,     0,   109,
     110,   111,     0,   283,   284,     0,     0,   285,     0,   286,
       0,     0,     0,   153,     0,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,   277,   278,   279,   280,   118,
     105,   106,   107,   108,     0,     0,     0,   109,   110,   111,
       0,     0,     0,   112,   113,   114,   115,   116,     0,     0,
       0,     0,     0,     0,     0,     0,   105,   106,   107,   108,
       0,     0,   149,   109,   110,   111,   321,     0,   322,   112,
     113,   114,   115,   116,     0,   125,   126,   105,   106,   107,
     108,   160,     0,   149,   109,   110,   111,     0,     0,     0,
     112,   113,   114,   115,   116,   160,   125,   126,     0,     0,
     153,   160,     0,     0,     0,   160,     0,     0,   160,     0,
       0,     0,     0,     0,     0,     0,   153,     0,     0,     0,
       0,   361,   160,     0,   363,   364,     0,     0,   365,     0,
     483,   366,   105,   106,   107,   108,     0,     0,     0,   109,
     110,   111,     0,     0,     0,   112,   113,   114,   115,   116,
     380,   381,   382,   383,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   153,   394,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   153,     0,   117,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   118,     0,     0,     0,     0,   160,     0,     0,     0,
       0,     0,   150,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   277,   278,   279,   280,     0,     0,     0,
       0,     0,     0,   150,     0,     0,     0,     0,   168,     0,
     433,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     441,   412,     0,     0,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       0,   117,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,    83,    84,
      85,    86,     0,     0,     0,     0,   390,     0,   105,   106,
     107,   108,     0,     1,   149,   109,   110,   111,     0,     2,
       0,   112,   113,   114,   115,   116,     0,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    87,    88,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,    83,    84,    85,    86,   105,   106,
     107,   108,     0,     0,     0,   109,   110,   111,     0,     0,
       0,   112,   113,   114,   115,   116,     0,     0,     0,     0,
     431,     0,   105,   106,   107,   108,     0,     0,   149,   109,
     110,   111,     0,     0,   150,   112,   113,   114,   115,   116,
       0,     0,    87,    88,   392,     0,   105,   106,   107,   108,
       0,     0,     0,   109,   110,   111,     0,     0,     0,   112,
     113,   114,   115,   116,   105,   106,   107,   108,     0,     0,
     149,   109,   110,   111,     0,     0,     0,   112,   113,   114,
     115,   116,     0,   125,   126,   105,   106,   107,   108,     0,
       0,     0,   109,   110,   111,     0,     0,     0,   112,   113,
     114,   115,   116,     0,   125,   126,     0,   412,   105,   106,
     107,   108,     0,     0,   149,   109,   110,   111,     0,     0,
       0,   112,   113,   114,   115,   116,   105,   106,   107,   108,
       0,     0,     0,   109,   110,   111,     0,     0,     0,   112,
     113,   114,   115,   116,     0,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   150,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   150,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     150,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   150,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   150,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   150
};

static const yytype_int16 yycheck[] =
{
       1,    18,    32,     1,   287,     6,     7,    14,    15,    16,
      22,     8,     9,   148,     1,    11,    12,    24,     1,    22,
      23,   148,    52,    53,    54,    55,    38,    22,    23,    25,
      26,     1,    62,   146,    35,   146,    33,    34,    22,    23,
      22,    23,   118,     3,     4,     5,     6,    21,    44,    45,
      10,    11,    12,    49,     0,    51,    16,    17,    18,    19,
      20,    22,    23,    59,    60,     3,     4,     5,     6,   352,
      66,    72,    10,    11,    12,    71,    73,    74,   295,    76,
      81,    78,   299,    79,    22,    23,   152,   408,    84,    85,
       1,   412,     3,     4,     5,     6,    14,    22,    23,    10,
      11,    12,    22,    23,    21,    16,    17,    18,    19,    20,
     309,    22,    23,     3,     4,     5,     6,   118,    22,    23,
      10,    11,    12,   162,   163,    22,    23,   326,    15,   130,
     131,   118,   146,   134,    15,   136,   146,    15,   139,   140,
     141,   150,   463,   113,    22,    23,   176,   149,   178,   147,
     147,   154,    22,    23,   151,   152,   153,   149,   153,   146,
     157,   158,   153,   160,    14,    15,    16,    17,   152,    90,
      80,   168,   202,    23,    24,   172,   158,   160,   161,   162,
     163,    31,   165,   153,   181,    22,    23,    22,    23,   149,
     151,   188,   193,   118,   195,   301,    46,    22,    23,    24,
     160,   161,   162,   163,   296,   429,    56,     1,   146,    22,
      23,   397,   209,    -1,   211,    -1,   213,   214,    -1,   216,
      -1,   159,    16,    17,    18,    19,    20,    -1,   148,   230,
      -1,    -1,   233,   234,    14,   160,   161,   162,   163,     1,
     118,   152,    22,    23,    -1,   242,    -1,   248,   118,    -1,
     251,   252,    -1,   164,   255,    -1,   146,   258,    -1,    -1,
      22,    23,   259,   160,   161,   162,   163,    -1,    -1,   159,
      -1,   268,   269,    -1,    -1,   305,   277,   278,   279,   280,
      -1,   118,   289,   118,   301,    16,    17,    18,    19,    20,
      -1,   292,   293,    -1,   107,   296,    -1,    -1,   315,   300,
       1,    -1,     3,     4,     5,     6,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,   315,    16,    17,    18,    19,    20,
      -1,     3,     4,     5,     6,    28,    29,    30,    10,    11,
      12,    39,    40,    -1,    42,    -1,   366,    -1,   334,    -1,
     336,    16,    17,    18,    19,    20,   353,    22,    23,    -1,
      58,   348,    -1,   147,    -1,    -1,   357,    16,    17,    18,
      19,    20,    -1,    22,    23,     5,   367,    -1,     8,     9,
      -1,   372,    -1,   374,   160,   161,   162,   163,    18,    19,
      20,    21,    22,    23,    87,    88,    -1,    27,    28,    -1,
      -1,   408,    32,    33,    34,   412,   397,    37,    38,    -1,
      -1,    41,    -1,    43,    -1,    -1,   403,   408,    48,    -1,
      50,   412,    52,    53,    54,    55,    -1,    57,   149,    -1,
     270,    61,    62,    63,    64,    65,    -1,    67,   429,    -1,
      -1,    -1,    -1,    73,   284,    75,    76,    77,    78,   289,
     290,    81,    82,    83,   294,    -1,   463,   297,    -1,    -1,
      -1,   152,    16,    17,    18,    19,    20,    -1,   139,   140,
     141,   311,   463,   164,   146,    -1,    -1,     1,    -1,     3,
       4,     5,     6,    -1,    -1,   150,    10,    11,    12,    -1,
      -1,   121,    16,    17,    18,    19,    20,   127,     1,    -1,
      -1,     4,    -1,   152,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,     1,   353,    -1,     4,    -1,   147,    -1,    22,
      23,    10,    11,    12,    16,    17,    18,    19,    20,   159,
      22,    23,    -1,   163,    -1,   165,   166,    -1,    -1,    -1,
     170,    -1,    -1,    -1,    -1,   385,    -1,    -1,    -1,    -1,
     180,    -1,   182,   183,    -1,    -1,   186,    -1,    -1,   189,
      -1,   191,   233,   234,    -1,    -1,   196,   197,    -1,    -1,
     200,    -1,    -1,   203,   204,    -1,   206,   207,    -1,   209,
     210,    -1,   212,    -1,    -1,   215,    -1,    -1,   218,   219,
      -1,   221,    -1,   223,    -1,   225,    -1,   227,   152,   229,
      -1,    -1,   232,    -1,    -1,    -1,    -1,   237,   238,    -1,
      -1,    -1,   242,    -1,    -1,   245,   246,    -1,    -1,   249,
      16,    17,    18,    19,    20,    -1,    -1,    -1,   152,    -1,
     260,    -1,    -1,   263,    -1,   265,    -1,    -1,   268,   269,
     164,   271,   272,     1,    -1,     3,     4,     5,     6,    -1,
      -1,     9,    10,    11,    12,   285,   286,    -1,    16,    17,
      18,    19,    20,    -1,    -1,    14,    15,    16,    17,    18,
      -1,    20,    -1,    -1,    23,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    32,    -1,    -1,   316,    36,   318,    -1,
      -1,   321,   322,    -1,    43,    44,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    55,    56,     3,     4,
       5,     6,    -1,    62,     1,    10,    11,    12,   348,     6,
       7,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
     360,   361,    -1,   363,   364,   365,   366,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   375,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,   384,    -1,   386,    -1,    -1,   389,
      -1,   391,    -1,   393,   394,    -1,    -1,   397,    -1,    -1,
      -1,    -1,    -1,   403,   404,    -1,    -1,   407,    -1,   409,
     410,   411,    -1,    -1,   414,    72,    -1,    -1,   418,    -1,
      -1,   149,   422,   423,   424,   425,   426,   427,    -1,    -1,
     430,    -1,   432,   433,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,     3,     4,     5,     6,    -1,
      -1,     9,    10,    11,    12,    -1,    -1,   176,    -1,   178,
      -1,   118,     3,     4,     5,     6,    -1,    -1,    -1,    10,
      11,    12,    -1,   130,   131,    -1,    -1,   134,    -1,   136,
      -1,    -1,    -1,   202,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   160,   161,   162,   163,   164,
       3,     4,     5,     6,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
      -1,    -1,     9,    10,    11,    12,   193,    -1,   195,    16,
      17,    18,    19,    20,    -1,    22,    23,     3,     4,     5,
       6,   270,    -1,     9,    10,    11,    12,    -1,    -1,    -1,
      16,    17,    18,    19,    20,   284,    22,    23,    -1,    -1,
     289,   290,    -1,    -1,    -1,   294,    -1,    -1,   297,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   305,    -1,    -1,    -1,
      -1,   248,   311,    -1,   251,   252,    -1,    -1,   255,    -1,
       1,   258,     3,     4,     5,     6,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    16,    17,    18,    19,    20,
     277,   278,   279,   280,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   353,   292,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   366,    -1,   152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   164,    -1,    -1,    -1,    -1,   385,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   160,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,   154,    -1,
     357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     367,   112,    -1,    -1,    -1,    -1,    -1,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    -1,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,   152,    42,    43,    44,    45,    46,    -1,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    -1,   108,   109,
     110,   111,    -1,    -1,    -1,    -1,     1,    -1,     3,     4,
       5,     6,    -1,     7,     9,    10,    11,    12,    -1,    13,
      -1,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,   156,   157,    42,    43,
      44,    45,    46,    -1,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,    -1,   108,   109,   110,   111,     3,     4,
       5,     6,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
       1,    -1,     3,     4,     5,     6,    -1,    -1,     9,    10,
      11,    12,    -1,    -1,   149,    16,    17,    18,    19,    20,
      -1,    -1,   156,   157,     1,    -1,     3,     4,     5,     6,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    19,    20,     3,     4,     5,     6,    -1,    -1,
       9,    10,    11,    12,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    -1,    22,    23,     3,     4,     5,     6,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    16,    17,
      18,    19,    20,    -1,    22,    23,    -1,   112,     3,     4,
       5,     6,    -1,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,     3,     4,     5,     6,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   149
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
     104,   105,   106,   108,   109,   110,   111,   156,   157,   167,
     168,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,     3,     4,     5,     6,    10,
      11,    12,    16,    17,    18,    19,    20,   152,   164,   190,
     199,   202,   210,   212,   213,    22,    23,   158,   169,   169,
     202,   202,   118,   169,   198,   169,   198,     1,   147,   186,
     186,   186,     1,     4,    10,    11,    12,   193,   211,     9,
     149,   195,   197,   199,   200,   201,   213,   195,   195,   197,
     199,   169,   190,   191,   192,   199,   154,   169,   154,   169,
     194,   195,   197,    14,   169,   188,   189,   169,   189,   169,
     197,   195,   186,   186,   169,   169,   193,   193,   193,   197,
     169,   194,   169,   198,   169,   198,   202,   199,   148,   169,
     184,   169,   189,   184,   184,   169,   184,   150,   169,   199,
     186,   199,   186,   199,   197,   149,   199,   169,   199,   186,
     169,   186,   169,   194,   169,   194,   169,   194,   169,   194,
     197,   169,   184,   186,   186,   169,   169,   194,   151,   169,
     169,   169,   186,   169,   148,   185,   146,   146,   186,   202,
     169,   198,   198,   169,   169,   198,   169,   169,   198,   186,
     185,     1,   169,   187,   202,   107,   169,   169,   186,   186,
      21,   193,   193,     0,   170,   202,   152,   160,   161,   162,
     163,   169,   169,   202,   202,   202,   202,   187,     1,   187,
     187,   169,   198,   198,   198,   198,   198,   198,   169,   198,
      21,   118,   169,   169,   169,   198,   169,    14,   194,   194,
     169,   198,   169,   169,   169,    15,     1,   198,     1,   169,
     169,   202,   202,   169,   169,   169,   194,   169,   169,   169,
     169,   169,   198,   169,   198,   169,   198,   198,   169,   198,
     169,   169,   169,   169,   169,   169,   169,   146,   206,   207,
     213,   169,   187,   187,   169,   169,   169,   198,   169,   169,
     146,   202,   169,   202,   202,   202,   202,   198,   169,   169,
     169,   169,   198,   169,   198,   197,   169,   169,     1,   165,
     202,   202,   202,   202,   196,   198,   197,   169,   169,   196,
       1,   195,     1,   197,   202,   146,   159,   208,   209,   213,
     197,     9,   200,   206,   197,   213,   192,   194,    15,   203,
     203,   197,   112,   190,   204,   205,   213,   169,   146,   169,
     169,   169,   203,   150,   186,   186,   149,   149,   169,   198,
     196,     1,   195,   202,   169,   169,   169,   169,   169,   169,
     194,   202,   213,   213,    24,   169,   169,   197,   169,   169,
     169,   169,   169,   169,   209,   169,   169,   169,   204,   169,
     169,   169,   204,   153,   169,   169,   169,   169,   169,   169,
     169,   169,   207,   169,   169,   169,   169,   169,   169,   169,
     169,     1,   113,     1,   204
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

/* Line 1455 of yacc.c  */
#line 154 "mon_parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 155 "mon_parse.y"
    { mon_lex_new_cmd();  (yyval.i) = 0; ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 178 "mon_parse.y"
    { return ERR_BAD_CMD; ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 182 "mon_parse.y"
    { mon_bank(e_default_space, NULL); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 184 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (3)].i), NULL); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 186 "mon_parse.y"
    { mon_bank(e_default_space, (yyvsp[(2) - (3)].str)); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 188 "mon_parse.y"
    { mon_bank((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 190 "mon_parse.y"
    { mon_jump((yyvsp[(2) - (3)].a)); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 192 "mon_parse.y"
    { mon_display_io_regs(0); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 194 "mon_parse.y"
    { mon_display_io_regs((yyvsp[(2) - (3)].a)); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 196 "mon_parse.y"
    { monitor_cpu_type_set(""); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 198 "mon_parse.y"
    { monitor_cpu_type_set((yyvsp[(2) - (3)].str)); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 200 "mon_parse.y"
    { mon_cpuhistory(-1); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 202 "mon_parse.y"
    { mon_cpuhistory((yyvsp[(3) - (4)].i)); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 204 "mon_parse.y"
    { mon_instruction_return(); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 206 "mon_parse.y"
    { machine_write_snapshot((yyvsp[(2) - (3)].str),0,0,0); /* FIXME */ ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 208 "mon_parse.y"
    { machine_read_snapshot((yyvsp[(2) - (3)].str), 0); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 210 "mon_parse.y"
    { mon_instructions_step(-1); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 212 "mon_parse.y"
    { mon_instructions_step((yyvsp[(3) - (4)].i)); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 214 "mon_parse.y"
    { mon_instructions_next(-1); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 216 "mon_parse.y"
    { mon_instructions_next((yyvsp[(3) - (4)].i)); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 218 "mon_parse.y"
    { mon_stack_up(-1); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 220 "mon_parse.y"
    { mon_stack_up((yyvsp[(3) - (4)].i)); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 222 "mon_parse.y"
    { mon_stack_down(-1); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 224 "mon_parse.y"
    { mon_stack_down((yyvsp[(3) - (4)].i)); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 226 "mon_parse.y"
    { mon_display_screen(); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 231 "mon_parse.y"
    { (monitor_cpu_for_memspace[default_memspace]->mon_register_print)(default_memspace); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 233 "mon_parse.y"
    { (monitor_cpu_for_memspace[(yyvsp[(2) - (3)].i)]->mon_register_print)((yyvsp[(2) - (3)].i)); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 238 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(4) - (5)].str));
                    ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 243 "mon_parse.y"
    {
                        /* What about the memspace? */
                        mon_playback_init((yyvsp[(2) - (3)].str));
                    ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 248 "mon_parse.y"
    { mon_save_symbols((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 250 "mon_parse.y"
    { mon_save_symbols(e_default_space, (yyvsp[(2) - (3)].str)); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 252 "mon_parse.y"
    { mon_add_name_to_symbol_table((yyvsp[(2) - (5)].a), (yyvsp[(4) - (5)].str)); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 254 "mon_parse.y"
    { mon_remove_name_from_symbol_table(e_default_space, (yyvsp[(2) - (3)].str)); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 256 "mon_parse.y"
    { mon_remove_name_from_symbol_table((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 258 "mon_parse.y"
    { mon_print_symbol_table((yyvsp[(2) - (3)].i)); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 260 "mon_parse.y"
    { mon_print_symbol_table(e_default_space); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 262 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (4)].a), mon_prepend_dot_to_name((yyvsp[(1) - (4)].str)));
                    ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 266 "mon_parse.y"
    {
                        mon_add_name_to_symbol_table((yyvsp[(3) - (5)].a), mon_prepend_dot_to_name((yyvsp[(1) - (5)].str)));
                    ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 272 "mon_parse.y"
    { mon_start_assemble_mode((yyvsp[(2) - (3)].a), NULL); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 274 "mon_parse.y"
    { mon_out("Syntax changed, please check 'help a'\n"); return ERR_EXPECT_END_CMD; ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 276 "mon_parse.y"
    { mon_disassemble_lines((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1]); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 278 "mon_parse.y"
    { mon_disassemble_lines(BAD_ADDR, BAD_ADDR); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 282 "mon_parse.y"
    { mon_memory_move((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], (yyvsp[(4) - (5)].a)); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 284 "mon_parse.y"
    { mon_memory_compare((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1], (yyvsp[(4) - (5)].a)); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 286 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 288 "mon_parse.y"
    { mon_memory_hunt((yyvsp[(2) - (5)].range)[0], (yyvsp[(2) - (5)].range)[1],(unsigned char *)(yyvsp[(4) - (5)].str)); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 290 "mon_parse.y"
    { mon_memory_display((yyvsp[(2) - (5)].rt), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], DF_PETSCII); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 292 "mon_parse.y"
    { mon_memory_display(default_radix, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 294 "mon_parse.y"
    { mon_memory_display(default_radix, BAD_ADDR, BAD_ADDR, DF_PETSCII); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 296 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 8, 8); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 298 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 8, 8); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 300 "mon_parse.y"
    { mon_memory_display_data((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], 24, 21); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 302 "mon_parse.y"
    { mon_memory_display_data(BAD_ADDR, BAD_ADDR, 24, 21); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 304 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_PETSCII); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 306 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_PETSCII); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 308 "mon_parse.y"
    { mon_memory_display(0, (yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], DF_SCREEN_CODE); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 310 "mon_parse.y"
    { mon_memory_display(0, BAD_ADDR, BAD_ADDR, DF_SCREEN_CODE); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 312 "mon_parse.y"
    { mon_memmap_zap(); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 314 "mon_parse.y"
    { mon_memmap_show(-1,BAD_ADDR,BAD_ADDR); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 316 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (4)].i),BAD_ADDR,BAD_ADDR); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 318 "mon_parse.y"
    { mon_memmap_show((yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].range)[0],(yyvsp[(4) - (5)].range)[1]); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 320 "mon_parse.y"
    { mon_memmap_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 324 "mon_parse.y"
    {
                      if ((yyvsp[(2) - (5)].i)) {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], TRUE, (yyvsp[(2) - (5)].i), FALSE);
                      } else {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], TRUE, e_exec, FALSE);
                      }
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 333 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 336 "mon_parse.y"
    {
                      mon_breakpoint_add_checkpoint((yyvsp[(2) - (3)].range)[0], (yyvsp[(2) - (3)].range)[1], TRUE, e_exec, TRUE);
                  ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 340 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 343 "mon_parse.y"
    {
                      if ((yyvsp[(2) - (5)].i)) {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], TRUE, (yyvsp[(2) - (5)].i), FALSE);
                      } else {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], TRUE, e_load | e_store, FALSE);
                      }
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 352 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 355 "mon_parse.y"
    {
                      if ((yyvsp[(2) - (5)].i)) {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], FALSE, (yyvsp[(2) - (5)].i), FALSE);
                      } else {
                          temp = mon_breakpoint_add_checkpoint((yyvsp[(3) - (5)].range)[0], (yyvsp[(3) - (5)].range)[1], FALSE, e_load | e_store, FALSE);
                      }
                      mon_breakpoint_set_checkpoint_condition(temp, (yyvsp[(4) - (5)].cond_node));
                  ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 364 "mon_parse.y"
    { mon_breakpoint_print_checkpoints(); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 369 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_ON, (yyvsp[(2) - (3)].i)); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 371 "mon_parse.y"
    { mon_breakpoint_switch_checkpoint(e_OFF, (yyvsp[(2) - (3)].i)); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 373 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (3)].i), -1); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 375 "mon_parse.y"
    { mon_breakpoint_set_ignore_count((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i)); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 377 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint((yyvsp[(2) - (3)].i)); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 379 "mon_parse.y"
    { mon_breakpoint_delete_checkpoint(-1); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 381 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_condition((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].cond_node)); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 383 "mon_parse.y"
    { mon_breakpoint_set_checkpoint_command((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].str)); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 385 "mon_parse.y"
    { return ERR_EXPECT_STRING; ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 389 "mon_parse.y"
    { sidefx = (((yyvsp[(2) - (3)].action) == e_TOGGLE) ? (sidefx ^ 1) : (yyvsp[(2) - (3)].action)); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 391 "mon_parse.y"
    {
                         mon_out("I/O side effects are %s\n",
                                   sidefx ? "enabled" : "disabled");
                     ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 396 "mon_parse.y"
    { default_radix = (yyvsp[(2) - (3)].rt); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 398 "mon_parse.y"
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
                     ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 416 "mon_parse.y"
    { monitor_change_device((yyvsp[(2) - (3)].i)); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 418 "mon_parse.y"
    { mon_export(); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 420 "mon_parse.y"
    { mon_quit(); YYACCEPT; ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 422 "mon_parse.y"
    { exit_mon = 1; YYACCEPT; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 426 "mon_parse.y"
    { mon_drive_execute_disk_cmd((yyvsp[(2) - (3)].str)); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 428 "mon_parse.y"
    { mon_out("\t%d\n",(yyvsp[(2) - (3)].i)); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 430 "mon_parse.y"
    { mon_command_print_help(NULL); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 432 "mon_parse.y"
    { mon_command_print_help((yyvsp[(2) - (3)].str)); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 434 "mon_parse.y"
    { printf("SYSTEM COMMAND: %s\n",(yyvsp[(2) - (3)].str)); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 436 "mon_parse.y"
    { mon_print_convert((yyvsp[(2) - (3)].i)); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 438 "mon_parse.y"
    { mon_change_dir((yyvsp[(2) - (3)].str)); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 440 "mon_parse.y"
    { mon_keyboard_feed((yyvsp[(2) - (3)].str)); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 442 "mon_parse.y"
    { mon_backtrace(); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 444 "mon_parse.y"
    { mon_show_dir((yyvsp[(2) - (3)].str)); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 446 "mon_parse.y"
    { mon_show_pwd(); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 448 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (3)].str),-1); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 450 "mon_parse.y"
    { mon_screenshot_save((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i)); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 452 "mon_parse.y"
    { mon_resource_get((yyvsp[(2) - (3)].str)); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 454 "mon_parse.y"
    { mon_resource_set((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].str)); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 456 "mon_parse.y"
    { mon_reset_machine(-1); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 458 "mon_parse.y"
    { mon_reset_machine((yyvsp[(3) - (4)].i)); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 460 "mon_parse.y"
    { mon_tape_ctrl((yyvsp[(3) - (4)].i)); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 462 "mon_parse.y"
    { mon_cart_freeze(); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 464 "mon_parse.y"
    { ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 466 "mon_parse.y"
    { mon_stopwatch_reset(); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 468 "mon_parse.y"
    { mon_stopwatch_show("Stopwatch: ", "\n"); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 472 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), FALSE); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 474 "mon_parse.y"
    { mon_file_load((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].a), TRUE); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 476 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], FALSE); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 478 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 480 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 482 "mon_parse.y"
    { mon_file_save((yyvsp[(2) - (5)].str), (yyvsp[(3) - (5)].i), (yyvsp[(4) - (5)].range)[0], (yyvsp[(4) - (5)].range)[1], TRUE); ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 484 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 486 "mon_parse.y"
    { mon_file_verify((yyvsp[(2) - (5)].str),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 488 "mon_parse.y"
    { return ERR_EXPECT_ADDRESS; ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 490 "mon_parse.y"
    { mon_drive_block_cmd(0,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 492 "mon_parse.y"
    { mon_drive_block_cmd(1,(yyvsp[(2) - (5)].i),(yyvsp[(3) - (5)].i),(yyvsp[(4) - (5)].a)); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 494 "mon_parse.y"
    { mon_drive_list(-1); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 496 "mon_parse.y"
    { mon_drive_list((yyvsp[(2) - (3)].i)); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 498 "mon_parse.y"
    { mon_attach((yyvsp[(2) - (4)].str),(yyvsp[(3) - (4)].i)); ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 500 "mon_parse.y"
    { mon_detach((yyvsp[(2) - (3)].i)); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 502 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (3)].str),0,1); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 504 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i),1); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 506 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (3)].str),0,0); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 508 "mon_parse.y"
    { mon_autostart((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].i),0); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 512 "mon_parse.y"
    { mon_record_commands((yyvsp[(2) - (3)].str)); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 514 "mon_parse.y"
    { mon_end_recording(); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 516 "mon_parse.y"
    { mon_playback_init((yyvsp[(2) - (3)].str)); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 520 "mon_parse.y"
    { mon_memory_fill((yyvsp[(2) - (4)].a), BAD_ADDR, (unsigned char *)(yyvsp[(3) - (4)].str)); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 522 "mon_parse.y"
    { printf("Not yet.\n"); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 526 "mon_parse.y"
    { yydebug = 1; ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 529 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 532 "mon_parse.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 533 "mon_parse.y"
    { (yyval.str) = NULL; ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 537 "mon_parse.y"
    { return ERR_EXPECT_FILENAME; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 541 "mon_parse.y"
    { return ERR_EXPECT_DEVICE_NUM; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 544 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (2)].i) | (yyvsp[(2) - (2)].i); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 545 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 548 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 549 "mon_parse.y"
    { (yyval.i) = 0; ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 552 "mon_parse.y"
    { (yyval.i) = new_reg(default_memspace, (yyvsp[(1) - (1)].reg)); ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 553 "mon_parse.y"
    { (yyval.i) = new_reg((yyvsp[(1) - (2)].i), (yyvsp[(2) - (2)].reg)); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 561 "mon_parse.y"
    { (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (3)].i))]->mon_register_set_val)(reg_memspace((yyvsp[(1) - (3)].i)), reg_regid((yyvsp[(1) - (3)].i)), (WORD) (yyvsp[(3) - (3)].i)); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 564 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 565 "mon_parse.y"
    { return ERR_EXPECT_CHECKNUM; ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 569 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (1)].a); (yyval.range)[1] = BAD_ADDR; ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 572 "mon_parse.y"
    { (yyval.range)[0] = (yyvsp[(1) - (3)].a); (yyval.range)[1] = (yyvsp[(3) - (3)].a); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 574 "mon_parse.y"
    { if (resolve_range(e_default_space, (yyval.range), (yyvsp[(1) - (1)].str))) return ERR_ADDR_TOO_BIG; ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 576 "mon_parse.y"
    { if (resolve_range((yyvsp[(1) - (3)].i), (yyval.range), (yyvsp[(3) - (3)].str))) return ERR_ADDR_TOO_BIG; ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 579 "mon_parse.y"
    { (yyval.a) = (yyvsp[(2) - (2)].a); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 580 "mon_parse.y"
    { (yyval.a) = BAD_ADDR; ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 584 "mon_parse.y"
    {
             (yyval.a) = new_addr(e_default_space,(yyvsp[(1) - (1)].i));
         ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 588 "mon_parse.y"
    {
             (yyval.a) = new_addr((yyvsp[(1) - (3)].i), (yyvsp[(3) - (3)].i));
         ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 592 "mon_parse.y"
    {
             temp = mon_symbol_table_lookup_addr(e_default_space, (yyvsp[(1) - (1)].str));
             if (temp >= 0)
                 (yyval.a) = new_addr(e_default_space, temp);
             else
                 return ERR_UNDEFINED_LABEL;
         ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 605 "mon_parse.y"
    { (yyval.i) = e_comp_space; ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 606 "mon_parse.y"
    { (yyval.i) = e_disk8_space; ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 607 "mon_parse.y"
    { (yyval.i) = e_disk9_space; ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 608 "mon_parse.y"
    { (yyval.i) = e_disk10_space; ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 609 "mon_parse.y"
    { (yyval.i) = e_disk11_space; ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 612 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); if (!CHECK_ADDR((yyvsp[(1) - (1)].i))) return ERR_ADDR_TOO_BIG; ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 614 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 616 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) + (yyvsp[(3) - (3)].i); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 617 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) - (yyvsp[(3) - (3)].i); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 618 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (3)].i) * (yyvsp[(3) - (3)].i); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 619 "mon_parse.y"
    { (yyval.i) = ((yyvsp[(3) - (3)].i)) ? ((yyvsp[(1) - (3)].i) / (yyvsp[(3) - (3)].i)) : 1; ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 620 "mon_parse.y"
    { (yyval.i) = (yyvsp[(2) - (3)].i); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 621 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 622 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 625 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(2) - (2)].cond_node); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 626 "mon_parse.y"
    { (yyval.cond_node) = 0; ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 629 "mon_parse.y"
    {
               (yyval.cond_node) = new_cond; (yyval.cond_node)->is_parenthized = FALSE;
               (yyval.cond_node)->child1 = (yyvsp[(1) - (3)].cond_node); (yyval.cond_node)->child2 = (yyvsp[(3) - (3)].cond_node); (yyval.cond_node)->operation = (yyvsp[(2) - (3)].cond_op);
           ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 634 "mon_parse.y"
    { return ERR_INCOMPLETE_COMPARE_OP; ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 636 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(2) - (3)].cond_node); (yyval.cond_node)->is_parenthized = TRUE; ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 638 "mon_parse.y"
    { return ERR_MISSING_CLOSE_PAREN; ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 640 "mon_parse.y"
    { (yyval.cond_node) = (yyvsp[(1) - (1)].cond_node); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 643 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->reg_num = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = TRUE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 649 "mon_parse.y"
    { (yyval.cond_node) = new_cond;
                            (yyval.cond_node)->operation = e_INV;
                            (yyval.cond_node)->is_parenthized = FALSE;
                            (yyval.cond_node)->value = (yyvsp[(1) - (1)].i); (yyval.cond_node)->is_reg = FALSE;
                            (yyval.cond_node)->child1 = NULL; (yyval.cond_node)->child2 = NULL;
                          ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 661 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 662 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 669 "mon_parse.y"
    { mon_add_number_to_buffer((yyvsp[(1) - (1)].i)); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 670 "mon_parse.y"
    { mon_add_number_masked_to_buffer((yyvsp[(1) - (1)].i), 0x00); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 671 "mon_parse.y"
    { mon_add_string_to_buffer((yyvsp[(1) - (1)].str)); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 674 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 675 "mon_parse.y"
    { (yyval.i) = (monitor_cpu_for_memspace[reg_memspace((yyvsp[(1) - (1)].i))]->mon_register_get_val)(reg_memspace((yyvsp[(1) - (1)].i)), reg_regid((yyvsp[(1) - (1)].i))); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 678 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 679 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 680 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 681 "mon_parse.y"
    { (yyval.i) = strtol((yyvsp[(1) - (1)].str), NULL, 10); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 684 "mon_parse.y"
    { (yyval.i) = resolve_datatype(B_NUMBER,(yyvsp[(1) - (1)].str)); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 685 "mon_parse.y"
    { (yyval.i) = resolve_datatype(O_NUMBER,(yyvsp[(1) - (1)].str)); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 686 "mon_parse.y"
    { (yyval.i) = resolve_datatype(D_NUMBER,(yyvsp[(1) - (1)].str)); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 689 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 690 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 691 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 692 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 693 "mon_parse.y"
    { (yyval.i) = (yyvsp[(1) - (1)].i); ;}
    break;



/* Line 1455 of yacc.c  */
#line 3755 "mon_parse.c"
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



/* Line 1675 of yacc.c  */
#line 696 "mon_parse.y"


int mon_parse_exec_line(char* buffer)
{
    int rc;

    /*yydebug = 1;*/

  	mon_lex_new_cmd();
    mon_lex_set_buffer(buffer);
    rc = yyparse();
    mon_lex_cleanup_buffer();

    return rc;
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


