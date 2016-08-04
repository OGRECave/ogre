/* definitions */
digit    [0-9]
char     [a-zA-Z_.]
alphanum [0-9a-zA-Z_]

%option prefix="ps10_"
%{  /* code to be included */

#include <stdlib.h>
#include <list>
#include <vector>

#include "ps1.0_program.h"

using namespace std;
using namespace ps10;

#include "_ps1.0_parser.h"

#include "nvparse_errors.h"
#include "nvparse_externs.h"
#define YY_NO_UNPUT

#define YY_INPUT(buf,result,max_size)                            \
{                                                                \
	int c = *myin++;                                             \
	result = (c == 0) ? YY_NULL : (buf[0] = c, 1);               \
}

#define YY_ALWAYS_INTERACTIVE 1

//#define DBG_MESG(msg, line)  	errors.set(msg, line)
#define DBG_MESG(msg, line)

static char buf[80];


%}

%s DEFSTATE

/* end of definitions */
%%

; |
\/\/			{
			    char ch;
			    while ((ch = yyinput()) != '\n')
					;
				line_number++;
				DBG_MESG("dbg: comment, NEWLINE", line_number-1);
				return NEWLINE;
			}

<DEFSTATE>[+-]?[0-9]+\.[0-9]* |
<DEFSTATE>[+-]?[0-9]*\.[0-9]+	|
<DEFSTATE>[+-]?[0-9]+	{
				ps10_lval.fval = (float)atof(yytext);
				// debug
				DBG_MESG("dbg: NUMBER", line_number);
				return NUMBER;
			}




def			{
				// debug
				DBG_MESG("dbg: DEF", line_number);
				BEGIN DEFSTATE;
				return DEF;
			}


((1[ \t]*)?-[ \t]*)?r[01](\.a|\.b|\.w|\.rgb|\.xyz|_bias|_bx2)?   |
((1[ \t]*)?-[ \t]*)?c[0-7](\.a|\.b|\.w|\.rgb|\.xyz|_bias|_bx2)?  |
((1[ \t]*)?-[ \t]*)?t[0-3](\.a|\.b|\.w|\.rgb|\.xyz|_bias|_bx2)?  |
((1[ \t]*)?-[ \t]*)?v[01](\.a|\.b|\.w|\.rgb|\.xyz|_bias|_bx2)?   {
				sprintf(buf, "dbg: REG = %s", yytext);
				// debug
				DBG_MESG(buf, line_number);
				ps10_lval.sval = new string(yytext);
				return REG;
			}

add(_x2|_x4|_d2)?(_sat)? |
cnd(_x2|_x4|_d2)?(_sat)? |
dp3(_x2|_x4|_d2)?(_sat)? |
lrp(_x2|_x4|_d2)?(_sat)? |
mad(_x2|_x4|_d2)?(_sat)? |
mov(_x2|_x4|_d2)?(_sat)? |
mul(_x2|_x4|_d2)?(_sat)? |
sub(_x2|_x4|_d2)?(_sat)? {
				sprintf(buf, "dbg: BLENDOP = %s", yytext);
				// debug
				DBG_MESG(buf, line_number);
				ps10_lval.sval = new string(yytext);
				return BLENDOP;
			}

tex          |
texbem       |
texbeml      |
texcoord     |
texkill      |
texm3x2pad   |
texm3x2tex   |
texreg2ar    |
texreg2gb    |
texm3x3pad   |
texm3x3spec  |
texm3x3tex   |
texm3x3vspec	{
				sprintf(buf, "dbg: ADDROP = %s", yytext);
				// debug
				DBG_MESG(buf, line_number);
				ps10_lval.sval = new string(yytext);
				return ADDROP;
			}



([ \t]*\r?\n)+	{
				line_number++;
				BEGIN 0;
				// debug
				DBG_MESG("dbg: NEWLINE", line_number-1);
				return NEWLINE;
			}

[ \t]+		{
			}

[Pp]s\.1.[01]		{
				return HEADER;
			}

.			{
				char buf[40];
				sprintf(buf, "character token == '%c'", *yytext);
				DBG_MESG(buf, line_number);
				return *yytext;
			}

%%
bool ps10_init(char* inputString)
{
    myin = inputString;
	return init_extensions();
}

#ifndef ps10_wrap
int ps10_wrap(void)
{
  return(1);
}
#endif
