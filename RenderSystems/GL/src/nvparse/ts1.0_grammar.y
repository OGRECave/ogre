%start WholeEnchilada
%name-prefix="ts10_"
%{
void yyerror(const char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ts1.0_inst.h"
#include "ts1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

%}
%union {
  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;
}

%token <fval> floatValue gequal less texVariable expandString
%token <fval> openParen closeParen semicolon comma
%token <fval> nop
%token <fval> texture_1d
%token <fval> texture_2d
%token <fval> texture_rectangle
%token <fval> texture_3d
%token <fval> texture_cube_map 
%token <fval> cull_fragment
%token <fval> pass_through
%token <fval> offset_2d_scale 
%token <fval> offset_2d
%token <fval> offset_rectangle_scale 
%token <fval> offset_rectangle
%token <fval> dependent_ar
%token <fval> dependent_gb
%token <fval> dot_product_2d_1of2
%token <fval> dot_product_2d_2of2 
%token <fval> dot_product_rectangle_1of2
%token <fval> dot_product_rectangle_2of2 
%token <fval> dot_product_depth_replace_1of2
%token <fval> dot_product_depth_replace_2of2 
%token <fval> dot_product_3d_1of3
%token <fval> dot_product_3d_2of3
%token <fval> dot_product_3d_3of3
%token <fval> dot_product_cube_map_1of3
%token <fval> dot_product_cube_map_2of3
%token <fval> dot_product_cube_map_3of3
%token <fval> dot_product_reflect_cube_map_eye_from_qs_1of3
%token <fval> dot_product_reflect_cube_map_eye_from_qs_2of3
%token <fval> dot_product_reflect_cube_map_eye_from_qs_3of3
%token <fval> dot_product_reflect_cube_map_const_eye_1of3
%token <fval> dot_product_reflect_cube_map_const_eye_2of3
%token <fval> dot_product_reflect_cube_map_const_eye_3of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_const_eye_1of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_const_eye_2of3
%token <fval> dot_product_cube_map_and_reflect_cube_map_const_eye_3of3

%type <instList> WholeEnchilada InstListDesc
%type <inst> InstDesc
%type <variable> MappedVariableDesc
%type <fval> CondDesc

%%

WholeEnchilada: InstListDesc
		{
			$1->Validate();
			$1->Invoke();
		    delete $1;
		}
		;

InstListDesc: InstListDesc InstDesc semicolon
		{
		    *($1) += $2;
			delete $2;
		    $$ = $1;
		}
		| InstDesc semicolon
		{
		    InstListPtr instList = new InstList;
		    *instList += $1;
			delete $1;
		    $$ = instList;
		}
		;


MappedVariableDesc:	expandString openParen texVariable closeParen
		{
			$$ = new MappedVariable;
			$$->var = $3;
			$$->expand = true;
		}
		| texVariable
		{
			$$ = new MappedVariable;
			$$->var = $1;
			$$->expand = false;
		}
		;

InstDesc	: nop openParen closeParen
		{
		    $$ = new Inst(TSP_NOP);
		}
		| texture_1d openParen closeParen
		{
		    $$ = new Inst(TSP_TEXTURE_1D);
		}
		| texture_2d openParen closeParen
		{
		    $$ = new Inst(TSP_TEXTURE_2D);
		}
		| texture_rectangle openParen closeParen
		{
		    $$ = new Inst(TSP_TEXTURE_RECTANGLE);
		}
		| texture_3d openParen closeParen
		{
		    $$ = new Inst(TSP_TEXTURE_3D);
		}
		| texture_cube_map openParen closeParen
		{
		    $$ = new Inst(TSP_TEXTURE_CUBE_MAP);
		}
		| cull_fragment openParen CondDesc comma CondDesc comma CondDesc comma CondDesc closeParen
		{
		    $$ = new Inst(TSP_CULL_FRAGMENT, $3, $5, $7, $9);
		}
		| pass_through openParen closeParen
		{
		    $$ = new Inst(TSP_PASS_THROUGH);
		}
		| offset_2d_scale openParen texVariable comma floatValue comma floatValue comma floatValue comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_OFFSET_2D_SCALE, $3, $5, $7, $9, $11, $13, $15);
		}
		| offset_2d openParen texVariable comma floatValue comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_OFFSET_2D, $3, $5, $7, $9, $11);
		}
		| offset_rectangle_scale openParen texVariable comma floatValue comma floatValue comma floatValue comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_OFFSET_RECTANGLE_SCALE, $3, $5, $7, $9, $11, $13, $15);
		}
		| offset_rectangle openParen texVariable comma floatValue comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_OFFSET_RECTANGLE, $3, $5, $7, $9, $11);
		}
		| dependent_ar openParen texVariable closeParen
		{
		    $$ = new Inst(TSP_DEPENDENT_AR, $3);
		}
		| dependent_gb openParen texVariable closeParen
		{
		    $$ = new Inst(TSP_DEPENDENT_GB, $3);
		}
		| dot_product_2d_1of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_2D_1_OF_2, $3);
			delete $3;
		}
		| dot_product_2d_2of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_2D_2_OF_2, $3);
			delete $3;
		}
		| dot_product_rectangle_1of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_RECTANGLE_1_OF_2, $3);
			delete $3;
		}
		| dot_product_rectangle_2of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_RECTANGLE_2_OF_2, $3);
			delete $3;
		}
		| dot_product_depth_replace_1of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_1_OF_2, $3);
			delete $3;
		}
		| dot_product_depth_replace_2of2 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_2_OF_2, $3);
			delete $3;
		}
		| dot_product_3d_1of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_3D_1_OF_3, $3);
			delete $3;
		}
		| dot_product_3d_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_3D_2_OF_3, $3);
			delete $3;
		}
		| dot_product_3d_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_3D_3_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_1of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_1_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_2_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_3_OF_3, $3);
			delete $3;
		}
		| dot_product_reflect_cube_map_eye_from_qs_1of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, $3);
			delete $3;
		}
		| dot_product_reflect_cube_map_eye_from_qs_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, $3);
			delete $3;
		}
		| dot_product_reflect_cube_map_eye_from_qs_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, $3);
			delete $3;
		}
		| dot_product_reflect_cube_map_const_eye_1of3 openParen MappedVariableDesc comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, $3, $5, $7, $9);
			delete $3;
		}
		| dot_product_reflect_cube_map_const_eye_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, $3);
			delete $3;
		}
		| dot_product_reflect_cube_map_const_eye_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_const_eye_1of3 openParen MappedVariableDesc comma floatValue comma floatValue comma floatValue closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, $3, $5, $7, $9);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_const_eye_2of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, $3);
			delete $3;
		}
		| dot_product_cube_map_and_reflect_cube_map_const_eye_3of3 openParen MappedVariableDesc closeParen
		{
		    $$ = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, $3);
			delete $3;
		}
		;

CondDesc :	gequal
		{
			$$ = $1;
		}
		| 	less
		{
			$$ = $1;
		}
		;


%%
void yyerror(const char* s)
{
     errors.set("unrecognized token");
}
