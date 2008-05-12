/* definitions */
digit    [0-9]
char     [a-zA-Z_.]
alphanum [0-9a-zA-Z_]
%option prefix="ts10_"
%{  /* code to be included */
#include <stdlib.h>
#include "ts1.0_inst.h"
#include "ts1.0_inst_list.h"
#include "_ts1.0_parser.h"
#ifdef _WIN32
# include <windows.h>
#endif
#include "nvparse_errors.h"
#include "nvparse_externs.h"


#define YY_INPUT(buf,result,max_size) \
{ \
	int c = *myin++; \
	result = (c == 0) ? YY_NULL : (buf[0] = c, 1); \
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

\(			return(openParen);
\)			return(closeParen);
;			return(semicolon);
,			return(comma);

!!TS1\.0    { /* eat header */ }

tex0		{
			    ts10_lval.fval = (float)0;
			    return(texVariable);
			}

tex1		{
			    ts10_lval.fval = (float)1;
			    return(texVariable);
			}

tex2		{
			    ts10_lval.fval = (float)2;
			    return(texVariable);
			}

expand		return(expandString);

GEQUAL_TO_ZERO	{
				ts10_lval.fval = (float)GL_GEQUAL;
				return (gequal);
			}

LESS_THAN_ZERO	{
				ts10_lval.fval = (float)GL_LESS;
				return(less);
			}

nop			return(nop);
texture_1d		return(texture_1d);
texture_2d		return(texture_2d);
texture_rectangle	return(texture_rectangle);
texture_3d		return(texture_3d);
texture_cube_map return(texture_cube_map);
cull_fragment	return(cull_fragment);
pass_through	return(pass_through);
offset_2d_scale return(offset_2d_scale);
offset_2d	return(offset_2d);
offset_rectangle_scale return(offset_rectangle_scale);
offset_rectangle	return(offset_rectangle);
dependent_ar	return(dependent_ar);
dependent_gb	return(dependent_gb);

dot_product_2d_1of2	return(dot_product_2d_1of2);
dot_product_2d_2of2 return(dot_product_2d_2of2);

dot_product_rectangle_1of2	return(dot_product_rectangle_1of2);
dot_product_rectangle_2of2 return(dot_product_rectangle_2of2);

dot_product_depth_replace_1of2	return(dot_product_depth_replace_1of2);
dot_product_depth_replace_2of2 return(dot_product_depth_replace_2of2);

dot_product_3d_1of3	return(dot_product_3d_1of3);
dot_product_3d_2of3	return(dot_product_3d_2of3);
dot_product_3d_3of3	return(dot_product_3d_3of3);

dot_product_cube_map_1of3	return(dot_product_cube_map_1of3);
dot_product_cube_map_2of3	return(dot_product_cube_map_2of3);
dot_product_cube_map_3of3	return(dot_product_cube_map_3of3);

dot_product_reflect_cube_map_eye_from_qs_1of3	return(dot_product_reflect_cube_map_eye_from_qs_1of3);
dot_product_reflect_cube_map_eye_from_qs_2of3	return(dot_product_reflect_cube_map_eye_from_qs_2of3);
dot_product_reflect_cube_map_eye_from_qs_3of3	return(dot_product_reflect_cube_map_eye_from_qs_3of3);

dot_product_reflect_cube_map_const_eye_1of3	return(dot_product_reflect_cube_map_const_eye_1of3);
dot_product_reflect_cube_map_const_eye_2of3	return(dot_product_reflect_cube_map_const_eye_2of3);
dot_product_reflect_cube_map_const_eye_3of3	return(dot_product_reflect_cube_map_const_eye_3of3);

dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3	return(dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3);
dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3	return(dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3);
dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3	return(dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3);

dot_product_cube_map_and_reflect_cube_map_const_eye_1of3	return(dot_product_cube_map_and_reflect_cube_map_const_eye_1of3);
dot_product_cube_map_and_reflect_cube_map_const_eye_2of3	return(dot_product_cube_map_and_reflect_cube_map_const_eye_2of3);
dot_product_cube_map_and_reflect_cube_map_const_eye_3of3	return(dot_product_cube_map_and_reflect_cube_map_const_eye_3of3);

[+-]?[0-9]+\.[0-9]*	{
				ts10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

[+-]?[0-9]*\.[0-9]+	{
				ts10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

[+-]?[0-9]+		{
				ts10_lval.fval = (float)atof(yytext);
				return(floatValue);
			}

\r*\n		{
				line_number++;
			}

[ \t]+		{}

{alphanum}+ {
				errors.set("unrecognized token", line_number);
			}
				

%%

bool ts10_init_more();

bool ts10_init(char* inputString)
{
    myin = inputString;
	return ts10_init_more();
}


#ifndef ts10_wrap
int ts10_wrap(void)
{
  return(1);
}
#endif
