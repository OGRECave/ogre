/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_VS10_VS1_0_PARSER_H_INCLUDED
# define YY_VS10_VS1_0_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int vs10_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    VERTEX_SHADER = 258,
    ADD_INSTR = 259,
    DP3_INSTR = 260,
    DP4_INSTR = 261,
    DST_INSTR = 262,
    EXP_INSTR = 263,
    EXPP_INSTR = 264,
    FRC_INSTR = 265,
    LIT_INSTR = 266,
    LOG_INSTR = 267,
    LOGP_INSTR = 268,
    M3X2_INSTR = 269,
    M3X3_INSTR = 270,
    M3X4_INSTR = 271,
    M4X3_INSTR = 272,
    M4X4_INSTR = 273,
    MAD_INSTR = 274,
    MAX_INSTR = 275,
    MIN_INSTR = 276,
    MOV_INSTR = 277,
    MUL_INSTR = 278,
    NOP_INSTR = 279,
    RCP_INSTR = 280,
    RSQ_INSTR = 281,
    SGE_INSTR = 282,
    SLT_INSTR = 283,
    SUB_INSTR = 284,
    ILLEGAL = 285,
    UNKNOWN_STRING = 286,
    INTVAL = 287,
    REGISTER = 288,
    XYZW_MODIFIER = 289,
    COMMENT = 290
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 27 "vs1.0_grammar.y" /* yacc.c:1909  */

  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;

#line 101 "_vs1.0_parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE vs10_lval;

int vs10_parse (void);

#endif /* !YY_VS10_VS1_0_PARSER_H_INCLUDED  */
