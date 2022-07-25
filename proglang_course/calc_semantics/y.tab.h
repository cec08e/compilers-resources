/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SEMInumber = 258,
     LPARENnumber = 259,
     RPARENnumber = 260,
     ICONSTnumber = 261,
     BEGINnumber = 262,
     PROGRAMnumber = 263,
     MINUSnumber = 264,
     TIMESnumber = 265,
     VARnumber = 266,
     INTnumber = 267,
     COMMAnumber = 268,
     IDnumber = 269,
     ENDnumber = 270,
     ISnumber = 271,
     PLUSnumber = 272,
     DIVnumber = 273,
     PRINTnumber = 274,
     EQnumber = 275
   };
#endif
/* Tokens.  */
#define SEMInumber 258
#define LPARENnumber 259
#define RPARENnumber 260
#define ICONSTnumber 261
#define BEGINnumber 262
#define PROGRAMnumber 263
#define MINUSnumber 264
#define TIMESnumber 265
#define VARnumber 266
#define INTnumber 267
#define COMMAnumber 268
#define IDnumber 269
#define ENDnumber 270
#define ISnumber 271
#define PLUSnumber 272
#define DIVnumber 273
#define PRINTnumber 274
#define EQnumber 275




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 29 "calc.y"
{
  int sv;
  struct {
    int v;
    char s[1000];
  } attr;
}
/* Line 1529 of yacc.c.  */
#line 97 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;
