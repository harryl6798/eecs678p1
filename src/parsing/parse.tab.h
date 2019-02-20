/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED
# define YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 22 "src/parsing/parse.y" /* yacc.c:1909  */

#include <stdbool.h>

#include "command.h"
#include "parsing_interface.h"
#include "parse.tab.h"
#include "memory_pool.h"

#line 53 "src/parsing/parse.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PIPE = 258,
    BCKGRND = 259,
    SQUOTE = 260,
    EQUALS = 261,
    REDIRIN = 262,
    REDIROUT = 263,
    REDIROUTAPP = 264,
    END = 265,
    ECHO_TOK = 266,
    EXPORT_TOK = 267,
    CD_TOK = 268,
    PWD_TOK = 269,
    JOBS_TOK = 270,
    KILL_TOK = 271,
    EOC_TOK = 272,
    STR = 273,
    SIM_STR = 274,
    ID = 275,
    NUM = 276,
    EXIT_TOK = 277
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 31 "src/parsing/parse.y" /* yacc.c:1909  */

  int integer;
  char* str;
  Command cmd;
  CommandHolder holder;
  CommandHolder* holder_arr;
  CmdStrs cmd_strs;
  Cmds cmd_list;
  Redirect redirect;

#line 99 "src/parsing/parse.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (CommandHolder** __ret_cmds);

#endif /* !YY_YY_SRC_PARSING_PARSE_TAB_H_INCLUDED  */
