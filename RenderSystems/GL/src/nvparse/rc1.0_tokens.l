/* definitions */
digit    [0-9]
char     [a-zA-Z_.]
alphanum [0-9a-zA-Z_]

%option prefix="rc10_"
%{  /* code to be included */

#include <stdlib.h>
#include "rc1.0_combiners.h"
#include "_rc1.0_parser.h"

#include "nvparse_errors.h"
#include "nvparse_externs.h"

#define YY_INPUT(buf,result,max_size) \
{ \
    result = 0;                        \
    while (*myin && result < max_size) \
    buf[result++] = *myin++;           \
}

#define YY_ALWAYS_INTERACTIVE 1

%}
/* end of definitions */
%%

\/\*			{
			    A:
				switch (yyinput()) {
				  case '*': goto B;
				  case 0: errors.set("end-of-file in comment", line_number); break;
				  case '\n': line_number++;
				  default:  goto A;
				}
			    B:
				switch (yyinput()) {
				  case '*': goto B;
				  case '/': break;
				  case 0: errors.set("end-of-file in comment", line_number); break;
				  case '\n': line_number++;
				  default:  goto A;
				}
			}

\/\/		{
			    char ch;
			    while ((ch = yyinput()) != '\n');
			    if (ch == '\n') line_number++;
			}

\#			{
			    char ch;
			    while ((ch = yyinput()) != '\n');
			    if (ch == '\n') line_number++;
			}

!!RC1\.0	{ /* eat header */ }

fog\.rgb	{
			    rc10_lval.registerEnum.word = RCP_FOG_RGB;
			    return(regVariable);
			}
fog\.a		{
			    rc10_lval.registerEnum.word = RCP_FOG_ALPHA;
			    return(regVariable);
			}
fog\.b		{
			    rc10_lval.registerEnum.word = RCP_FOG_BLUE;
			    return(regVariable);
			}
fog			{
			    rc10_lval.registerEnum.word = RCP_FOG;
			    return(regVariable);
			}

col0\.rgb	{
			    rc10_lval.registerEnum.word = RCP_PRIMARY_COLOR_RGB;
			    return(regVariable);
			}
col0\.a		{
			    rc10_lval.registerEnum.word = RCP_PRIMARY_COLOR_ALPHA;
			    return(regVariable);
			}
col0\.b		{
			    rc10_lval.registerEnum.word = RCP_PRIMARY_COLOR_BLUE;
			    return(regVariable);
			}
col0		{
			    rc10_lval.registerEnum.word = RCP_PRIMARY_COLOR;
			    return(regVariable);
			}

col1\.rgb	{
			    rc10_lval.registerEnum.word = RCP_SECONDARY_COLOR_RGB;
			    return(regVariable);
			}
col1\.a		{
			    rc10_lval.registerEnum.word = RCP_SECONDARY_COLOR_ALPHA;
			    return(regVariable);
			}
col1\.b		{
			    rc10_lval.registerEnum.word = RCP_SECONDARY_COLOR_BLUE;
			    return(regVariable);
			}
col1		{
			    rc10_lval.registerEnum.word = RCP_SECONDARY_COLOR;
			    return(regVariable);
			}

spare0\.rgb	{
			    rc10_lval.registerEnum.word = RCP_SPARE0_RGB;
			    return(regVariable);
			}
spare0\.a		{
			    rc10_lval.registerEnum.word = RCP_SPARE0_ALPHA;
			    return(regVariable);
			}
spare0\.b		{
			    rc10_lval.registerEnum.word = RCP_SPARE0_BLUE;
			    return(regVariable);
			}
spare0		{
			    rc10_lval.registerEnum.word = RCP_SPARE0;
			    return(regVariable);
			}

spare1\.rgb	{
			    rc10_lval.registerEnum.word = RCP_SPARE1_RGB;
			    return(regVariable);
			}
spare1\.a		{
			    rc10_lval.registerEnum.word = RCP_SPARE1_ALPHA;
			    return(regVariable);
			}
spare1\.b		{
			    rc10_lval.registerEnum.word = RCP_SPARE1_BLUE;
			    return(regVariable);
			}
spare1		{
			    rc10_lval.registerEnum.word = RCP_SPARE1;
			    return(regVariable);
			}

tex0\.rgb	{
			    rc10_lval.registerEnum.word = RCP_TEXTURE0_RGB;
			    return(regVariable);
			}
tex0\.a		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE0_ALPHA;
			    return(regVariable);
			}
tex0\.b		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE0_BLUE;
			    return(regVariable);
			}
tex0		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE0;
			    return(regVariable);
			}

tex1\.rgb	{
			    rc10_lval.registerEnum.word = RCP_TEXTURE1_RGB;
			    return(regVariable);
			}
tex1\.a		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE1_ALPHA;
			    return(regVariable);
			}
tex1\.b		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE1_BLUE;
			    return(regVariable);
			}
tex1		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE1;
			    return(regVariable);
			}

tex2\.rgb	{
			    rc10_lval.registerEnum.word = RCP_TEXTURE2_RGB;
			    return(regVariable);
			}
tex2\.a		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE2_ALPHA;
			    return(regVariable);
			}
tex2\.b		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE2_BLUE;
			    return(regVariable);
			}
tex2		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE2;
			    return(regVariable);
			}

tex3\.rgb	{
			    rc10_lval.registerEnum.word = RCP_TEXTURE3_RGB;
			    return(regVariable);
			}
tex3\.a		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE3_ALPHA;
			    return(regVariable);
			}
tex3\.b		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE3_BLUE;
			    return(regVariable);
			}
tex3		{
			    rc10_lval.registerEnum.word = RCP_TEXTURE3;
			    return(regVariable);
			}

const0\.rgb	{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR0_RGB;
			    return(regVariable);
			}
const0\.a		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR0_ALPHA;
			    return(regVariable);
			}
const0\.b		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR0_BLUE;
			    return(regVariable);
			}
const0		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR0;
			    return(constVariable);
			}

const1\.rgb	{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR1_RGB;
			    return(regVariable);
			}
const1\.a		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR1_ALPHA;
			    return(regVariable);
			}
const1\.b		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR1_BLUE;
			    return(regVariable);
			}
const1		{
			    rc10_lval.registerEnum.word = RCP_CONST_COLOR1;
			    return(constVariable);
			}

zero\.rgb	{
			    rc10_lval.registerEnum.word = RCP_ZERO_RGB;
			    return(regVariable);
			}
zero\.a		{
			    rc10_lval.registerEnum.word = RCP_ZERO_ALPHA;
			    return(regVariable);
			}
zero\.b		{
			    rc10_lval.registerEnum.word = RCP_ZERO_BLUE;
			    return(regVariable);
			}
zero		{
			    rc10_lval.registerEnum.word = RCP_ZERO;
			    return(regVariable);
			}

one\.rgb	{
			    rc10_lval.registerEnum.word = RCP_ONE_RGB;
			    return(regVariable);
			}
one\.a		{
			    rc10_lval.registerEnum.word = RCP_ONE_ALPHA;
			    return(regVariable);
			}
one\.b		{
			    rc10_lval.registerEnum.word = RCP_ONE_BLUE;
			    return(regVariable);
			}
one			{
			    rc10_lval.registerEnum.word = RCP_ONE;
			    return(regVariable);
			}

discard		{
			    rc10_lval.registerEnum.word = RCP_DISCARD;
			    return(regVariable);
			}

out\.rgb		return(fragment_rgb);
out\.a			return(fragment_alpha);

final_product {
			    rc10_lval.registerEnum.word = RCP_FINAL_PRODUCT;
			    return(final_product);
			}

color_sum {
			    rc10_lval.registerEnum.word = RCP_COLOR_SUM;
			    return(color_sum);
			}

expand			return(expandString);
half_bias		return(halfBiasString);
unsigned		return(unsignedString);
unsigned_invert	return(unsignedInvertString);
mux				{
					rc10_lval.ival = GL_TRUE;
					return(muxString);
				}

sum				{
					rc10_lval.ival = GL_FALSE;
					return(sumString);
				}

rgb				{
					rc10_lval.ival = RCP_RGB;
					return(rgb_portion);
				}
alpha			{
					rc10_lval.ival = RCP_ALPHA;
					return(alpha_portion);
				}

clamp_color_sum {
				rc10_lval.ival = true;
				return(clamp_color_sum);
			}
lerp			return(lerp);


bias_by_negative_one_half_scale_by_two {
				rc10_lval.biasScaleEnum.word = RCP_BIAS_BY_NEGATIVE_ONE_HALF_SCALE_BY_TWO;
				return(bias_by_negative_one_half_scale_by_two);
			}
bias_by_negative_one_half {
				rc10_lval.biasScaleEnum.word = RCP_BIAS_BY_NEGATIVE_ONE_HALF;
				return(bias_by_negative_one_half);
			}
scale_by_one_half {
				rc10_lval.biasScaleEnum.word = RCP_SCALE_BY_ONE_HALF;
				return(scale_by_one_half);
			}
scale_by_two {
				rc10_lval.biasScaleEnum.word = RCP_SCALE_BY_TWO;
				return(scale_by_two);
			}
scale_by_four {
				rc10_lval.biasScaleEnum.word = RCP_SCALE_BY_FOUR;
				return(scale_by_four);
			}

[+-]?[0-9]+\.[0-9]*	{
				rc10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

[+-]?[0-9]*\.[0-9]+	{
				rc10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

[+-]?[0-9]+		{
				rc10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

\{			return(openBracket);
\}			return(closeBracket);
\(			return(openParen);
\)			return(closeParen);
;			return(semicolon);
,			return(comma);
\.			return(dot);
\*			return(times);
-			return(minus);
\+			return(plus);
=			return(equals);

\r*\n		{
				line_number++;
			}

[ \t]+		{}

{alphanum}+ {
				errors.set("unrecognized token", line_number);
			}
				

%%

bool rc10_init_more();

bool rc10_init(char* inputString)
{
    myin = inputString;
	return rc10_init_more();
}

#ifndef rc10_wrap
int rc10_wrap(void)
{
  return(1);
}
#endif
