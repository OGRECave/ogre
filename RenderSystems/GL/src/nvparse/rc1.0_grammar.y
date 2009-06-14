%start WholeEnchilada
%name-prefix="rc10_"
%{
void yyerror(const char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "rc1.0_combiners.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"


%}
%union {
  int ival;
  float fval;
  RegisterEnum registerEnum;
  BiasScaleEnum biasScaleEnum;
  MappedRegisterStruct mappedRegisterStruct;
  ConstColorStruct constColorStruct;
  GeneralPortionStruct generalPortionStruct;
  GeneralFunctionStruct generalFunctionStruct;
  OpStruct opStruct;
  GeneralCombinerStruct generalCombinerStruct;
  GeneralCombinersStruct generalCombinersStruct;
  FinalProductStruct finalProductStruct;
  FinalRgbFunctionStruct finalRgbFunctionStruct;
  FinalAlphaFunctionStruct finalAlphaFunctionStruct;
  FinalCombinerStruct finalCombinerStruct;
  CombinersStruct combinersStruct;
}

%token <registerEnum> regVariable constVariable color_sum final_product
%token <ival> expandString halfBiasString unsignedString unsignedInvertString muxString sumString
%token <ival> rgb_portion alpha_portion
%token <ival> openParen closeParen openBracket closeBracket semicolon comma
%token <ival> dot times minus equals plus
%token <biasScaleEnum> bias_by_negative_one_half_scale_by_two bias_by_negative_one_half
%token <biasScaleEnum> scale_by_one_half scale_by_two scale_by_four

%token <ival> clamp_color_sum lerp
%token <ival> fragment_rgb fragment_alpha

%token <fval> floatValue

%type <combinersStruct> WholeEnchilada Combiners

%type <constColorStruct> ConstColor
%type <generalCombinerStruct> GeneralCombiner
%type <generalCombinersStruct> GeneralCombiners
%type <ival> PortionDesignator
%type <opStruct> Mux Sum Dot Mul
%type <generalPortionStruct> GeneralPortion
%type <generalFunctionStruct> GeneralFunction
%type <biasScaleEnum> BiasScale

%type <registerEnum> Register
%type <mappedRegisterStruct> GeneralMappedRegister FinalMappedRegister
%type <finalProductStruct> FinalProduct
%type <ival> ClampColorSum
%type <finalRgbFunctionStruct> FinalRgbFunction
%type <finalAlphaFunctionStruct> FinalAlphaFunction
%type <finalCombinerStruct> FinalCombiner

%%

WholeEnchilada	: Combiners
		{
			$1.Validate();
			$1.Invoke();
		}
		;

Combiners : ConstColor GeneralCombiners FinalCombiner
		{
			CombinersStruct combinersStruct;
			combinersStruct.Init($2, $3, $1);
			$$ = combinersStruct;
		}
		| ConstColor ConstColor GeneralCombiners FinalCombiner
		{
			CombinersStruct combinersStruct;
			combinersStruct.Init($3, $4, $1, $2);
			$$ = combinersStruct;
		}
		| GeneralCombiners FinalCombiner
		{
			CombinersStruct combinersStruct;
			combinersStruct.Init($1, $2);
			$$ = combinersStruct;
		}
		| ConstColor FinalCombiner
		{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, $2, $1);
			$$ = combinersStruct;
		}
		| ConstColor ConstColor FinalCombiner
		{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, $3, $1, $2);
			$$ = combinersStruct;
		}
		| FinalCombiner
		{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, $1);
			$$ = combinersStruct;
		}
		;
 
ConstColor : constVariable equals openParen floatValue comma floatValue comma floatValue comma floatValue closeParen semicolon
		{
			ConstColorStruct constColorStruct;
			constColorStruct.Init($1, $4, $6, $8, $10);
			$$ = constColorStruct;
		}
		;
 
 
GeneralCombiners	 : GeneralCombiners GeneralCombiner
		{
			$1 += $2;
			$$ = $1;
		}
		| GeneralCombiner
		{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init($1);
			$$ = generalCombinersStruct;
		}
		;
 
GeneralCombiner : openBracket GeneralPortion GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($2, $3);
			$$ = generalCombinerStruct;
		}
		| openBracket ConstColor GeneralPortion GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($3, $4, $2);
			$$ = generalCombinerStruct;
		}
		| openBracket ConstColor ConstColor GeneralPortion GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($4, $5, $2, $3);
			$$ = generalCombinerStruct;
		}
		| openBracket GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($2);
			$$ = generalCombinerStruct;
		}
		| openBracket ConstColor GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($3, $2);
			$$ = generalCombinerStruct;
		}
		| openBracket ConstColor ConstColor GeneralPortion closeBracket
		{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init($4, $2, $3);
			$$ = generalCombinerStruct;
		}
		;
 
GeneralPortion	: PortionDesignator openBracket GeneralFunction BiasScale closeBracket
		{
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init($1, $3, $4);
			$$ = generalPortionStruct;
		}
		| PortionDesignator openBracket GeneralFunction closeBracket
		{
			BiasScaleEnum noScale;
			noScale.word = RCP_SCALE_BY_ONE;
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init($1, $3, noScale);
			$$ = generalPortionStruct;
		}
		;
 
PortionDesignator	: rgb_portion
		{
			$$ = $1;
		}
		| alpha_portion
		{
			$$ = $1;
		}
		;

GeneralMappedRegister :     Register
		{
			MappedRegisterStruct reg;
			reg.Init($1, GL_SIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| minus Register
		{
			MappedRegisterStruct reg;
			reg.Init($2, GL_SIGNED_NEGATE_NV);
			$$ = reg;
		}
		| expandString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_EXPAND_NORMAL_NV);
			$$ = reg;
		}
		| minus expandString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($4, GL_EXPAND_NEGATE_NV);
			$$ = reg;
		}
		| halfBiasString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_HALF_BIAS_NORMAL_NV);
			$$ = reg;
		}
		| minus halfBiasString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($4, GL_HALF_BIAS_NEGATE_NV);
			$$ = reg;
		}
		| unsignedString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| unsignedInvertString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_INVERT_NV);
			$$ = reg;
		}
		;
  
GeneralFunction	: Dot Dot
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2);
			$$ = generalFunction;
		}
		| Dot Mul
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2);
			$$ = generalFunction;
		}
		| Mul Dot
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2);
			$$ = generalFunction;
		}
		| Dot
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1);
			$$ = generalFunction;
		}
		| Mul Mul
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2);
			$$ = generalFunction;
		}
		| Mul Mul Mux
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2, $3);
			$$ = generalFunction;
		}
		| Mul Mul Sum
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1, $2, $3);
			$$ = generalFunction;
		}
		| Mul
		{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init($1);
			$$ = generalFunction;
		}
		;
 
Dot : Register equals GeneralMappedRegister dot GeneralMappedRegister semicolon
		{
			OpStruct dotFunction;
			dotFunction.Init(RCP_DOT, $1, $3, $5);
			$$ = dotFunction;
		}
		;
 
Mul : Register equals GeneralMappedRegister times GeneralMappedRegister semicolon
		{
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, $1, $3, $5);
			$$ = mulFunction;
		}
		| Register equals GeneralMappedRegister semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct one;
			one.Init(zero, GL_UNSIGNED_INVERT_NV);
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, $1, $3, one);
			$$ = mulFunction;
		}
		;

Mux : Register equals muxString openParen closeParen semicolon
		{
			OpStruct muxFunction;
			muxFunction.Init(RCP_MUX, $1);
			$$ = muxFunction;
		}
		;

Sum : Register equals sumString openParen closeParen semicolon
		{
			OpStruct sumFunction;
			sumFunction.Init(RCP_SUM, $1);
			$$ = sumFunction;
		}
		;
 
BiasScale	: bias_by_negative_one_half_scale_by_two openParen closeParen semicolon
		{
			$$ = $1;
		}
		| bias_by_negative_one_half openParen closeParen semicolon
		{
			$$ = $1;
		}
		| scale_by_one_half openParen closeParen semicolon
		{
			$$ = $1;
		}
		| scale_by_two openParen closeParen semicolon
		{
			$$ = $1;
		}
		| scale_by_four openParen closeParen semicolon
		{
			$$ = $1;
		}
		;

FinalMappedRegister : Register
		{
			MappedRegisterStruct reg;
			reg.Init($1, GL_UNSIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| unsignedString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| unsignedInvertString openParen Register closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_INVERT_NV);
			$$ = reg;
		}
		| color_sum
		{
			MappedRegisterStruct reg;
			reg.Init($1);
			$$ = reg;
		}
		| unsignedString openParen color_sum closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| unsignedInvertString openParen color_sum closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_INVERT_NV);
			$$ = reg;
		}
		| final_product
		{
			MappedRegisterStruct reg;
			reg.Init($1);
			$$ = reg;
		}
		| unsignedString openParen final_product closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_IDENTITY_NV);
			$$ = reg;
		}
		| unsignedInvertString openParen final_product closeParen
		{
			MappedRegisterStruct reg;
			reg.Init($3, GL_UNSIGNED_INVERT_NV);
			$$ = reg;
		}
		;

FinalCombiner : FinalAlphaFunction FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($2, $1, false);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalAlphaFunction FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($3, $2, true);
			$$ = finalCombinerStruct;
		}
		| FinalProduct FinalAlphaFunction FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($3, $2, false, $1);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalProduct FinalAlphaFunction FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($4, $3, true, $2);
			$$ = finalCombinerStruct;
		}
		| FinalProduct ClampColorSum FinalAlphaFunction FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($4, $3, true, $1);
			$$ = finalCombinerStruct;
		}

		| FinalRgbFunction FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($1, $2, false);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalRgbFunction FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($2, $3, true);
			$$ = finalCombinerStruct;
		}
		| FinalProduct FinalRgbFunction FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($2, $3, false, $1);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalProduct FinalRgbFunction FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($3, $4, true, $2);
			$$ = finalCombinerStruct;
		}
		| FinalProduct ClampColorSum FinalRgbFunction FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init($3, $4, true, $1);
			$$ = finalCombinerStruct;
		}

		| FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init($1, finalAlphaFunctionStruct, false);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init($2, finalAlphaFunctionStruct, true);
			$$ = finalCombinerStruct;
		}
		| FinalProduct FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init($2, finalAlphaFunctionStruct, false, $1);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalProduct FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init($3, finalAlphaFunctionStruct, true, $2);
			$$ = finalCombinerStruct;
		}
		| FinalProduct ClampColorSum FinalRgbFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init($3, finalAlphaFunctionStruct, true, $1);
			$$ = finalCombinerStruct;
		}

		| FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, $1, false);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, $2, true);
			$$ = finalCombinerStruct;
		}
		| FinalProduct FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, $2, false, $1);
			$$ = finalCombinerStruct;
		}
		| ClampColorSum FinalProduct FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, $3, true, $2);
			$$ = finalCombinerStruct;
		}
		| FinalProduct ClampColorSum FinalAlphaFunction
		{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, $3, true, $1);
			$$ = finalCombinerStruct;
		}
		;

ClampColorSum	: clamp_color_sum openParen closeParen semicolon
		{
			$$ = $1;
		}
		; 

FinalProduct	: final_product equals FinalMappedRegister times FinalMappedRegister semicolon
		{
			FinalProductStruct finalProductStruct;
			finalProductStruct.Init($3, $5);
			$$ = finalProductStruct;
		}
		;

FinalRgbFunction	: fragment_rgb equals lerp openParen FinalMappedRegister comma FinalMappedRegister comma FinalMappedRegister closeParen plus FinalMappedRegister semicolon
		{
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($5, $7, $9, $12);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals FinalMappedRegister plus lerp openParen FinalMappedRegister comma FinalMappedRegister comma FinalMappedRegister closeParen semicolon
		{
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($7, $9, $11, $3);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals lerp openParen FinalMappedRegister comma FinalMappedRegister comma FinalMappedRegister closeParen semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg;
			reg.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($5, $7, $9, reg);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals FinalMappedRegister times FinalMappedRegister semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($3, $5, reg1, reg2);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals FinalMappedRegister times FinalMappedRegister plus FinalMappedRegister semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($3, $5, reg1, $7);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals FinalMappedRegister semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(reg1, reg2, reg3, $3);
			$$ = finalRgbFunctionStruct;
		}
		| fragment_rgb equals FinalMappedRegister plus FinalMappedRegister semicolon
		{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_INVERT_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init($3, reg2, reg3, $5);
			$$ = finalRgbFunctionStruct;
		}
		;

FinalAlphaFunction	: fragment_alpha equals FinalMappedRegister semicolon
		{
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.Init($3);
			$$ = finalAlphaFunctionStruct;
		}
		;

Register : constVariable
		{
			$$ = $1;
		}
		| regVariable
		{
			$$ = $1;
		}
		;

%%
void yyerror(const char* s)
{
     errors.set("unrecognized token");
}
