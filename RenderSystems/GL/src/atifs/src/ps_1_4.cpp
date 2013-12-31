/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

 
 //---------------------------------------------------------------------------
#include "ps_1_4.h"

//---------------------------------------------------------------------------

/* ********************* START OF PS_1_4 CLASS STATIC DATA ********************************* */


// library of built in symbol types

bool PS_1_4::LibInitialized = false;

#define SYMSTART {
#define SYMDEF  ,0,0,0,0},{
#define SYMEND  ,0,0,0,0}

PS_1_4::SymbolDef PS_1_4::PS_1_4_SymbolTypeLib[] = {
	// pixel shader versions supported
	{ sid_PS_1_4, GL_NONE, ckp_PS_BASE, ckp_PS_1_4, 0, 0, 0 },
	{ sid_PS_1_1, GL_NONE, ckp_PS_BASE, ckp_PS_1_1, 0, 0, 0 },
	{ sid_PS_1_2, GL_NONE, ckp_PS_BASE, ckp_PS_1_2 + ckp_PS_1_1, 0, 0, 0 },
	{ sid_PS_1_3, GL_NONE, ckp_PS_BASE, ckp_PS_1_3 + ckp_PS_1_2 + ckp_PS_1_1, 0, 0, 0 },

	// PS_BASE

	// constants
	SYMSTART sid_C0, GL_CON_0_ATI, ckp_PS_BASE
	SYMDEF sid_C1, GL_CON_1_ATI, ckp_PS_BASE
	SYMDEF sid_C2, GL_CON_2_ATI, ckp_PS_BASE
	SYMDEF sid_C3, GL_CON_3_ATI, ckp_PS_BASE
	SYMDEF sid_C4, GL_CON_4_ATI, ckp_PS_BASE
	SYMDEF sid_C5, GL_CON_5_ATI, ckp_PS_BASE
	SYMDEF sid_C6, GL_CON_6_ATI, ckp_PS_BASE
	SYMDEF sid_C7, GL_CON_7_ATI, ckp_PS_BASE

	// colour
	SYMDEF sid_V0, GL_PRIMARY_COLOR_ARB,  ckp_PS_BASE
	SYMDEF sid_V1, GL_SECONDARY_INTERPOLATOR_ATI,  ckp_PS_BASE

	// instruction ops
	SYMDEF sid_ADD, GL_ADD_ATI, ckp_PS_BASE
	SYMDEF sid_SUB, GL_SUB_ATI, ckp_PS_BASE
	SYMDEF sid_MUL, GL_MUL_ATI, ckp_PS_BASE
	SYMDEF sid_MAD, GL_MAD_ATI, ckp_PS_BASE
	SYMDEF sid_LRP, GL_LERP_ATI, ckp_PS_BASE
	SYMDEF sid_MOV, GL_MOV_ATI, ckp_PS_BASE
	SYMDEF sid_CMP, GL_CND0_ATI, ckp_PS_BASE
	SYMDEF sid_CND, GL_CND_ATI, ckp_PS_BASE
	SYMDEF sid_DP3, GL_DOT3_ATI, ckp_PS_BASE
	SYMDEF sid_DP4, GL_DOT4_ATI, ckp_PS_BASE

	SYMDEF sid_DEF, GL_NONE, ckp_PS_BASE

	// Masks
	SYMDEF sid_R, GL_RED_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_RA, GL_RED_BIT_ATI | ALPHA_BIT, ckp_PS_1_4
	SYMDEF sid_G, GL_GREEN_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_GA, GL_GREEN_BIT_ATI | ALPHA_BIT, ckp_PS_1_4
	SYMDEF sid_B, GL_BLUE_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_BA, GL_BLUE_BIT_ATI | ALPHA_BIT, ckp_PS_1_4
	SYMDEF sid_A, ALPHA_BIT, ckp_PS_BASE
	SYMDEF sid_RGBA, RGB_BITS | ALPHA_BIT, ckp_PS_BASE
	SYMDEF sid_RGB, RGB_BITS,  ckp_PS_BASE
	SYMDEF sid_RG, GL_RED_BIT_ATI | GL_GREEN_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_RGA, GL_RED_BIT_ATI | GL_GREEN_BIT_ATI | ALPHA_BIT, ckp_PS_1_4
	SYMDEF sid_RB, GL_RED_BIT_ATI | GL_BLUE_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_RBA, GL_RED_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT, ckp_PS_1_4
	SYMDEF sid_GB, GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_GBA, GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT, ckp_PS_1_4

	// Rep
	SYMDEF sid_RRRR, GL_RED, ckp_PS_1_4
	SYMDEF sid_GGGG, GL_GREEN, ckp_PS_1_4
	SYMDEF sid_BBBB, GL_BLUE, ckp_PS_BASE
	SYMDEF sid_AAAA, GL_ALPHA, ckp_PS_BASE


	// modifiers
	SYMDEF sid_X2, GL_2X_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_X4, GL_4X_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_D2, GL_HALF_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_SAT, GL_SATURATE_BIT_ATI, ckp_PS_BASE

	// argument modifiers
	SYMDEF sid_BIAS, GL_BIAS_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_INVERT, GL_COMP_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_NEGATE, GL_NEGATE_BIT_ATI, ckp_PS_BASE
	SYMDEF sid_BX2, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI, ckp_PS_BASE
	
	// seperator characters
	SYMDEF sid_COMMA, GL_NONE, ckp_PS_BASE
	SYMDEF sid_VALUE, GL_NONE, ckp_PS_BASE

	// PS_1_4
	// temp R/W registers
	SYMDEF sid_R0, GL_REG_0_ATI, ckp_PS_1_4
	SYMDEF sid_R1, GL_REG_1_ATI, ckp_PS_1_4
	SYMDEF sid_R2, GL_REG_2_ATI, ckp_PS_1_4
	SYMDEF sid_R3, GL_REG_3_ATI, ckp_PS_1_4
	SYMDEF sid_R4, GL_REG_4_ATI, ckp_PS_1_4
	SYMDEF sid_R5, GL_REG_5_ATI, ckp_PS_1_4

	// textures
	SYMDEF sid_T0, GL_TEXTURE0_ARB, ckp_PS_1_4
	SYMDEF sid_T1, GL_TEXTURE1_ARB, ckp_PS_1_4
	SYMDEF sid_T2, GL_TEXTURE2_ARB, ckp_PS_1_4
	SYMDEF sid_T3, GL_TEXTURE3_ARB, ckp_PS_1_4
	SYMDEF sid_T4, GL_TEXTURE4_ARB, ckp_PS_1_4
	SYMDEF sid_T5, GL_TEXTURE5_ARB, ckp_PS_1_4
	SYMDEF sid_DP2ADD, GL_DOT2_ADD_ATI, ckp_PS_1_4

	// modifiers
	SYMDEF sid_X8, GL_8X_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_D8, GL_EIGHTH_BIT_ATI, ckp_PS_1_4
	SYMDEF sid_D4, GL_QUARTER_BIT_ATI, ckp_PS_1_4

	// instructions
	SYMDEF sid_TEXCRD, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEXLD, GL_NONE, ckp_PS_1_4

	// texture swizzlers

	SYMDEF sid_STR, GL_SWIZZLE_STR_ATI - GL_SWIZZLE_STR_ATI, ckp_PS_1_4
	SYMDEF sid_STQ, GL_SWIZZLE_STQ_ATI - GL_SWIZZLE_STR_ATI, ckp_PS_1_4
	SYMDEF sid_STRDR, GL_SWIZZLE_STR_DR_ATI - GL_SWIZZLE_STR_ATI, ckp_PS_1_4
	SYMDEF sid_STQDQ, GL_SWIZZLE_STQ_DQ_ATI - GL_SWIZZLE_STR_ATI, ckp_PS_1_4 

	SYMDEF sid_BEM, GL_NONE, ckp_PS_1_4
	SYMDEF sid_PHASE, GL_NONE, ckp_PS_1_4

	// PS_1_1 
	// temp R/W registers
	// r0, r1 are mapped to r4, r5
	// t0 to t3 are mapped to r0 to r3
	SYMDEF sid_1R0, GL_REG_4_ATI, ckp_PS_1_1
	SYMDEF sid_1R1, GL_REG_5_ATI, ckp_PS_1_1
	SYMDEF sid_1T0, GL_REG_0_ATI, ckp_PS_1_1
	SYMDEF sid_1T1, GL_REG_1_ATI, ckp_PS_1_1
	SYMDEF sid_1T2, GL_REG_2_ATI, ckp_PS_1_1
	SYMDEF sid_1T3, GL_REG_3_ATI, ckp_PS_1_1

	// instructions common to PS_1_1, PS_1_2, PS_1_3
	SYMDEF sid_TEX, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXCOORD, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X2PAD, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X2TEX, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X3PAD, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X3TEX, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X3SPEC, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXM3X3VSPEC, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXREG2AR, GL_NONE, ckp_PS_1_2
	SYMDEF sid_TEXREG2GB, GL_NONE, ckp_PS_1_2

	// PS_1_2 & PS_1_3
	SYMDEF sid_TEXREG2RGB, GL_NONE, ckp_PS_1_2
	SYMDEF sid_TEXDP3, GL_NONE, ckp_PS_1_2
	SYMDEF sid_TEXDP3TEX, GL_NONE, ckp_PS_1_2
	

	// Common
	SYMDEF sid_SKIP, GL_NONE, ckp_PS_BASE
	SYMDEF sid_PLUS, GL_NONE, ckp_PS_BASE

	// Non-Terminal Tokens
	SYMDEF sid_PROGRAM, GL_NONE, ckp_PS_BASE
	SYMDEF sid_PROGRAMTYPE, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DECLCONSTS, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DEFCONST, GL_NONE, ckp_PS_BASE
	SYMDEF sid_CONSTANT, GL_NONE, ckp_PS_BASE
	SYMDEF sid_COLOR, GL_NONE, ckp_PS_BASE 
	SYMDEF sid_TEXSWIZZLE, GL_NONE, ckp_PS_BASE
	SYMDEF sid_UNARYOP, GL_NONE, ckp_PS_BASE
	SYMDEF sid_NUMVAL, GL_NONE, ckp_PS_BASE
	SYMDEF sid_SEPERATOR, GL_NONE, ckp_PS_BASE
	SYMDEF sid_ALUOPS, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TEXMASK, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TEXOP_PS1_1_3, GL_NONE, ckp_PS_1_1 
	SYMDEF sid_TEXOP_PS1_4, GL_NONE, ckp_PS_1_4
	SYMDEF sid_ALU_STATEMENT, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DSTMODSAT, GL_NONE, ckp_PS_BASE
	SYMDEF sid_UNARYOP_ARGS, GL_NONE, ckp_PS_BASE
	SYMDEF sid_REG_PS1_4, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEX_PS1_4, GL_NONE, ckp_PS_1_4
	SYMDEF sid_REG_PS1_1_3, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEX_PS1_1_3, GL_NONE, ckp_PS_1_1
	SYMDEF sid_DSTINFO, GL_NONE, ckp_PS_BASE
	SYMDEF sid_SRCINFO, GL_NONE, ckp_PS_BASE
	SYMDEF sid_BINARYOP_ARGS, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TERNARYOP_ARGS, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TEMPREG, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DSTMASK, GL_NONE, ckp_PS_BASE
	SYMDEF sid_PRESRCMOD, GL_NONE, ckp_PS_BASE
	SYMDEF sid_SRCNAME, GL_NONE, ckp_PS_BASE
	SYMDEF sid_SRCREP, GL_NONE, ckp_PS_BASE
	SYMDEF sid_POSTSRCMOD, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DSTMOD, GL_NONE, ckp_PS_BASE
	SYMDEF sid_DSTSAT, GL_NONE, ckp_PS_BASE
	SYMDEF sid_BINARYOP, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TERNARYOP, GL_NONE, ckp_PS_BASE
	SYMDEF sid_TEXOPS_PHASE1, GL_NONE, ckp_PS_BASE
	SYMDEF sid_COISSUE, GL_NONE, ckp_PS_BASE
	SYMDEF sid_PHASEMARKER, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEXOPS_PHASE2, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEXREG_PS1_4, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEXOPS_PS1_4, GL_NONE, ckp_PS_1_4
	SYMDEF sid_TEXOPS_PS1_1_3, GL_NONE, ckp_PS_1_1
	SYMDEF sid_TEXCISCOP_PS1_1_3, GL_NONE, ckp_PS_1_1
	SYMEND
};


// Rule Path Database for ps.1.x code based on extended Backus Naur Form notation

// <>	- non-terminal token
#define _rule_		{otRULE,		// ::=	- rule definition
#define _is_		,0},{otAND,
#define _and_		,0},{otAND,		//      - blank space is an implied "AND" meaning the token is required
#define _or_		,0},{otOR,		// |	- or
#define _optional_	,0},{otOPTIONAL,	// []	- optional
#define _repeat_	,0},{otREPEAT,	// {}	- repeat until fail
#define _end_		,0},{otEND,0,0,0},
#define _nt_        ,0
// " "  - terminal token string

PS_1_4::TokenRule PS_1_4::PS_1_x_RulePath[] = {

	_rule_ sid_PROGRAM, "Program"

		_is_ sid_PROGRAMTYPE _nt_
		_optional_ sid_DECLCONSTS _nt_
		_optional_ sid_TEXOPS_PHASE1 _nt_
		_optional_ sid_ALUOPS _nt_
		_optional_ sid_PHASEMARKER _nt_
		_optional_ sid_TEXOPS_PHASE2 _nt_
		_optional_ sid_ALUOPS _nt_
		_end_ 

	_rule_ sid_PROGRAMTYPE, "<ProgramType>"

		_is_ sid_PS_1_4, "ps.1.4"
		_or_ sid_PS_1_1, "ps.1.1"
		_or_ sid_PS_1_2, "ps.1.2"
		_or_ sid_PS_1_3, "ps.1.3"
		_end_

	_rule_ sid_PHASEMARKER, "<PhaseMarker>"

		_is_ sid_PHASE, "phase"
		_end_

	_rule_ sid_DECLCONSTS, "<DeclareConstants>"

		_repeat_ sid_DEFCONST _nt_
		_end_


	_rule_ sid_TEXOPS_PHASE1, "<TexOps_Phase1>"

		_is_ sid_TEXOPS_PS1_1_3 _nt_
		_or_ sid_TEXOPS_PS1_4 _nt_
		_end_

	_rule_ sid_TEXOPS_PHASE2, "<TexOps_Phase2>"

		_is_ sid_TEXOPS_PS1_4 _nt_
		_end_

	_rule_ sid_NUMVAL, "<NumVal>"

		_is_ sid_VALUE, "Float Value"
		_end_

	_rule_ sid_TEXOPS_PS1_1_3, "<TexOps_PS1_1_3>"

		_repeat_ sid_TEXOP_PS1_1_3 _nt_
		_end_

	_rule_ sid_TEXOPS_PS1_4, "<TexOps_PS1_4>"

		_repeat_ sid_TEXOP_PS1_4 _nt_
		_end_

	_rule_ sid_TEXOP_PS1_1_3, "<TexOp_PS1_1_3>"

		_is_  sid_TEXCISCOP_PS1_1_3 _nt_
		_and_ sid_TEX_PS1_1_3 _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_TEX_PS1_1_3 _nt_

		_or_  sid_TEXCOORD, "texcoord"
		_and_ sid_TEX_PS1_1_3 _nt_

		_or_  sid_TEX, "tex"
		_and_ sid_TEX_PS1_1_3 _nt_



		_end_

	_rule_ sid_TEXOP_PS1_4, "<TexOp_PS1_4>"

		_is_  sid_TEXCRD, "texcrd"
		_and_ sid_REG_PS1_4 _nt_
		_optional_ sid_TEXMASK _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_TEXREG_PS1_4 _nt_

		_or_  sid_TEXLD, "texld"
		_and_ sid_REG_PS1_4 _nt_
		_optional_ sid_TEXMASK _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_TEXREG_PS1_4 _nt_
		_end_

	_rule_ sid_ALUOPS, "<ALUOps>"

		_repeat_ sid_ALU_STATEMENT _nt_
		_end_

	_rule_ sid_ALU_STATEMENT, "<ALUStatement>"

		_is_ sid_COISSUE _nt_
		_and_ sid_UNARYOP _nt_
		_optional_ sid_DSTMODSAT _nt_
		_and_ sid_UNARYOP_ARGS _nt_ 

		_or_ sid_COISSUE _nt_
		_and_ sid_BINARYOP _nt_
		_optional_ sid_DSTMODSAT _nt_
		_and_ sid_BINARYOP_ARGS _nt_
		
		_or_ sid_COISSUE _nt_
		_and_ sid_TERNARYOP _nt_
		_optional_ sid_DSTMODSAT _nt_
		_and_ sid_TERNARYOP_ARGS _nt_
		_end_


	_rule_ sid_TEXREG_PS1_4, "<TexReg_PS1_4>"

		_is_ sid_TEX_PS1_4  _nt_
		_optional_ sid_TEXSWIZZLE _nt_
		_or_ sid_REG_PS1_4  _nt_
		_optional_ sid_TEXSWIZZLE _nt_
		_end_

	_rule_ sid_UNARYOP_ARGS, "<UnaryOpArgs>"

		_is_  sid_DSTINFO _nt_
		_and_ sid_SRCINFO _nt_
		_end_

	_rule_ sid_BINARYOP_ARGS, "<BinaryOpArgs>"
	
		_is_  sid_DSTINFO _nt_
		_and_ sid_SRCINFO _nt_
		_and_ sid_SRCINFO _nt_
		_end_

	_rule_ sid_TERNARYOP_ARGS, "<TernaryOpArgs>"
		
		_is_  sid_DSTINFO _nt_
		_and_ sid_SRCINFO _nt_
		_and_ sid_SRCINFO _nt_
		_and_ sid_SRCINFO _nt_
		_end_
 
	_rule_ sid_DSTINFO, "<DstInfo>"

		_is_ sid_TEMPREG _nt_
		_optional_ sid_DSTMASK _nt_
		_end_

	_rule_ sid_SRCINFO, "<SrcInfo>"
	
		_is_ sid_SEPERATOR _nt_
		_optional_ sid_PRESRCMOD _nt_
		_and_ sid_SRCNAME _nt_
		_optional_ sid_POSTSRCMOD _nt_
		_optional_ sid_SRCREP _nt_
		_end_

	_rule_ sid_SRCNAME, "<SrcName>"
	
		_is_ sid_TEMPREG _nt_
		_or_ sid_CONSTANT _nt_
		_or_ sid_COLOR _nt_
		_end_

	_rule_ sid_DEFCONST, "<DefineConstant>"

		_is_ sid_DEF, "def"
		_and_ sid_CONSTANT _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_NUMVAL _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_NUMVAL _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_NUMVAL _nt_
		_and_ sid_SEPERATOR _nt_
		_and_ sid_NUMVAL _nt_
		_end_

	_rule_ sid_CONSTANT, "<Constant>"

		_is_ sid_C0, "c0"
		_or_ sid_C1, "c1"
		_or_ sid_C2, "c2"
		_or_ sid_C3, "c3"
		_or_ sid_C4, "c4"
		_or_ sid_C5, "c5"
		_or_ sid_C6, "c6"
		_or_ sid_C7, "c7"
		_end_


	_rule_ sid_TEXCISCOP_PS1_1_3, "<TexCISCOp_PS1_1_3>"

		_is_ sid_TEXDP3TEX,		"texdp3tex"
		_or_ sid_TEXDP3,		"texdp3"
		_or_ sid_TEXM3X2PAD,	"texm3x2pad"
		_or_ sid_TEXM3X2TEX,	"texm3x2tex"
		_or_ sid_TEXM3X3PAD,	"texm3x3pad"
		_or_ sid_TEXM3X3TEX,	"texm3x3tex"
		_or_ sid_TEXM3X3SPEC,	"texm3x3spec"
		_or_ sid_TEXM3X3VSPEC,	"texm3x3vspec"
		_or_ sid_TEXREG2RGB,	"texreg2rgb"
		_or_ sid_TEXREG2AR,		"texreg2ar"
		_or_ sid_TEXREG2GB,		"texreg2gb"
		_end_


	_rule_ sid_TEXSWIZZLE, "<TexSwizzle>"

		_is_ sid_STQDQ,	"_dw.xyw"
		_or_ sid_STQDQ,	"_dw"
		_or_ sid_STQDQ,	"_da.rga"
		_or_ sid_STQDQ,	"_da"
		_or_ sid_STRDR,	"_dz.xyz"
		_or_ sid_STRDR,	"_dz"
		_or_ sid_STRDR,	"_db.rgb"
		_or_ sid_STRDR,	"_db"
		_or_ sid_STR,	".xyz"
		_or_ sid_STR,	".rgb"
		_or_ sid_STQ,	".xyw"
		_or_ sid_STQ,	".rga"
		_end_ 

	_rule_ sid_TEXMASK, "<TexMask>"

		_is_ sid_RGB,	".rgb"
		_or_ sid_RGB,	".xyz"
		_or_ sid_RG,	".rg"
		_or_ sid_RG,	".xy"
		_end_

	_rule_ sid_SEPERATOR, "<Seperator>"

		_is_ sid_COMMA, ","
		_end_

	_rule_ sid_REG_PS1_4, "<Reg_PS1_4>"

		_is_ sid_R0, "r0"
		_or_ sid_R1, "r1"
		_or_ sid_R2, "r2"
		_or_ sid_R3, "r3"
		_or_ sid_R4, "r4"
		_or_ sid_R5, "r5"
		_end_

	_rule_ sid_TEX_PS1_4, "<Tex_PS1_4>"

		_is_ sid_T0, "t0"
		_or_ sid_T1, "t1"
		_or_ sid_T2, "t2"
		_or_ sid_T3, "t3"
		_or_ sid_T4, "t4"
		_or_ sid_T5, "t5"
		_end_

	_rule_ sid_REG_PS1_1_3, "<Reg_PS1_1_3>"

		_is_ sid_1R0, "r0"
		_or_ sid_1R1, "r1"
		_end_

	_rule_ sid_TEX_PS1_1_3, "<Tex_PS1_1_3>"

		_is_ sid_1T0, "t0"
		_or_ sid_1T1, "t1"
		_or_ sid_1T2, "t2"
		_or_ sid_1T3, "t3"
		_end_

	_rule_ sid_COLOR, "<Color>"

		_is_ sid_V0, "v0"
		_or_ sid_V1, "v1"
		_end_


	_rule_ sid_TEMPREG, "<TempReg>"

		_is_ sid_REG_PS1_4 _nt_
		_or_ sid_REG_PS1_1_3 _nt_
		_or_ sid_TEX_PS1_1_3 _nt_
		_end_

	_rule_ sid_DSTMODSAT, "<DstModSat>"
	
		_optional_ sid_DSTMOD _nt_
		_optional_ sid_DSTSAT _nt_
		_end_

	_rule_  sid_UNARYOP, "<UnaryOp>"

		_is_ sid_MOV, "mov"
		_end_

	_rule_ sid_BINARYOP, "<BinaryOP>"
	
		_is_ sid_ADD, "add"
		_or_ sid_MUL, "mul"
		_or_ sid_SUB, "sub"
		_or_ sid_DP3, "dp3"
		_or_ sid_DP4, "dp4"
		_or_ sid_BEM, "bem"
		_end_

	_rule_ sid_TERNARYOP, "<TernaryOp>"
	
		_is_ sid_MAD, "mad"
		_or_ sid_LRP, "lrp"
		_or_ sid_CND, "cnd"
		_or_ sid_CMP, "cmp"
		_end_

	_rule_ sid_DSTMASK, "<DstMask>"

		_is_ sid_RGBA,	".rgba"
		_or_ sid_RGBA,	".xyzw"
		_or_ sid_RGB,	".rgb"
		_or_ sid_RGB,	".xyz"
		_or_ sid_RGA,	".xyw"
		_or_ sid_RGA,	".rga"
		_or_ sid_RBA,	".rba"
		_or_ sid_RBA,	".xzw"
		_or_ sid_GBA,	".gba"
		_or_ sid_GBA,	".yzw"
		_or_ sid_RG,	".rg"
		_or_ sid_RG,	".xy"
		_or_ sid_RB,	".xz"
		_or_ sid_RB,	".rb"
		_or_ sid_RA,	".xw"
		_or_ sid_RA,	".ra"
		_or_ sid_GB,	".gb"
		_or_ sid_GB,	".yz"
		_or_ sid_GA,	".yw"
		_or_ sid_GA,	".ga"
		_or_ sid_BA,	".zw"
		_or_ sid_BA,	".ba"
		_or_ sid_R,		".r"
		_or_ sid_R,		".x"
		_or_ sid_G,		".g"
		_or_ sid_G,		".y"
		_or_ sid_B,		".b"
		_or_ sid_B,		".z"
		_or_ sid_A,		".a"
		_or_ sid_A,		".w"
		_end_

	_rule_ sid_SRCREP, "<SrcRep>"
	
		_is_ sid_RRRR, ".r"
		_or_ sid_RRRR, ".x"
		_or_ sid_GGGG, ".g"
		_or_ sid_GGGG, ".y"
		_or_ sid_BBBB, ".b"
		_or_ sid_BBBB, ".z"
		_or_ sid_AAAA, ".a"
		_or_ sid_AAAA, ".w"
		_end_

	_rule_ sid_PRESRCMOD, "<PreSrcMod>"

		_is_ sid_INVERT, "1-"
		_or_ sid_INVERT, "1 -"
		_or_ sid_NEGATE, "-"
		_end_

	_rule_ sid_POSTSRCMOD, "<PostSrcMod>"

		_is_ sid_BX2, "_bx2"
		_or_ sid_X2, "_x2"
		_or_ sid_BIAS, "_bias"
		_end_

	_rule_ sid_DSTMOD, "<DstMod>"

		_is_ sid_X2, "_x2"
		_or_ sid_X4, "_x4"
		_or_ sid_D2, "_d2"
		_or_ sid_X8, "_x8"
		_or_ sid_D4, "_d4"
		_or_ sid_D8, "_d8"
		_end_

	_rule_ sid_DSTSAT, "<DstSat>"

		_is_ sid_SAT, "_sat"
		_end_

	_rule_ sid_COISSUE, "<CoIssue>"

		_optional_ sid_PLUS, "+"
		_end_

};

//***************************** MACROs for PS1_1 , PS1_2, PS1_3 CISC instructions **************************************

// macro to make the macro text data easier to read
#define _token_ ,0,0},{
#define _token_end_ ,0,0}
// macro token expansion for ps_1_2 instruction: texreg2ar
PS_1_4::TokenInst PS_1_4::texreg2ar[] = {
	// mov r(x).r, r(y).a
	{		sid_UNARYOP,	sid_MOV
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_DSTMASK,	sid_R
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_ sid_SRCREP,		sid_AAAA

	// mov r(x).g, r(y).r
	_token_ sid_UNARYOP,	sid_MOV
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_DSTMASK,	sid_G
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_ sid_SRCREP,		sid_RRRR

	// texld r(x), r(x)
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_end_
};

PS_1_4::RegModOffset PS_1_4::texreg2xx_RegMods[] = {
	{1, R_BASE, 0},
	{7, R_BASE, 0},
	{13, R_BASE, 0},
	{15, R_BASE, 0},
	{4, R_BASE, 1},
	{10, R_BASE, 1},

};

PS_1_4::MacroRegModify PS_1_4::texreg2ar_MacroMods = {
	texreg2ar, ARRAYSIZE(texreg2ar),
	texreg2xx_RegMods, ARRAYSIZE(texreg2xx_RegMods)
};

// macro token expansion for ps_1_2 instruction: texreg2gb
PS_1_4::TokenInst PS_1_4::texreg2gb[] = {
	// mov r(x).r, r(y).g
	{		sid_UNARYOP,	sid_MOV
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_DSTMASK,	sid_R
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_ sid_SRCREP,		sid_GGGG

	// mov r(x).g, r(y).b
	_token_ sid_UNARYOP,	sid_MOV
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_DSTMASK,	sid_G
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_ sid_SRCREP,		sid_BBBB

	// texld r(x), r(x)
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_end_
};

PS_1_4::MacroRegModify PS_1_4::texreg2gb_MacroMods = {
	texreg2gb, ARRAYSIZE(texreg2gb),
	texreg2xx_RegMods, ARRAYSIZE(texreg2xx_RegMods)
};


// macro token expansion for ps_1_1 instruction: texdp3
PS_1_4::TokenInst PS_1_4::texdp3[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T1

	// dp3 r(x), r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_end_
};


PS_1_4::RegModOffset PS_1_4::texdp3_RegMods[] = {
	{1, T_BASE, 0},
	{3, R_BASE, 0},
	{5, R_BASE, 0},
	{7, R_BASE, 1},

};


PS_1_4::MacroRegModify PS_1_4::texdp3_MacroMods = {
	texdp3, ARRAYSIZE(texdp3),
	texdp3_RegMods, ARRAYSIZE(texdp3_RegMods)
};


// macro token expansion for ps_1_1 instruction: texdp3tex
PS_1_4::TokenInst PS_1_4::texdp3tex[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T1

	// dp3 r1, r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0

	// texld r(x), r(x)
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_end_
};


PS_1_4::RegModOffset PS_1_4::texdp3tex_RegMods[] = {
	{1, T_BASE, 0},
	{3, R_BASE, 0},
	{5, R_BASE, 0},
	{7, R_BASE, 1},
	{9, R_BASE, 1},
	{11, R_BASE, 1},

};


PS_1_4::MacroRegModify PS_1_4::texdp3tex_MacroMods = {
	texdp3tex, ARRAYSIZE(texdp3tex),
	texdp3tex_RegMods, ARRAYSIZE(texdp3tex_RegMods)
};


// macro token expansion for ps_1_1 instruction: texm3x2pad
PS_1_4::TokenInst PS_1_4::texm3x2pad[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T0

	// dp3 r4.r, r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_R
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_end_

};


PS_1_4::RegModOffset PS_1_4::texm3xxpad_RegMods[] = {
	{1, T_BASE, 0},
	{6, R_BASE, 0},
	{8, R_BASE, 1},
};


PS_1_4::MacroRegModify PS_1_4::texm3x2pad_MacroMods = {
	texm3x2pad, ARRAYSIZE(texm3x2pad),
	texm3xxpad_RegMods, ARRAYSIZE(texm3xxpad_RegMods)
};


// macro token expansion for ps_1_1 instruction: texm3x2tex
PS_1_4::TokenInst PS_1_4::texm3x2tex[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T1

	// dp3 r4.g, r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_G
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0

	// texld r(x), r4
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4
	_token_end_

};

PS_1_4::RegModOffset PS_1_4::texm3xxtex_RegMods[] = {
	{1, T_BASE, 0},
	{6, R_BASE, 0},
	{8, R_BASE, 1},
	{10, R_BASE, 0}
};

PS_1_4::MacroRegModify PS_1_4::texm3x2tex_MacroMods = {
	texm3x2tex, ARRAYSIZE(texm3x2tex),
	texm3xxtex_RegMods, ARRAYSIZE(texm3xxtex_RegMods)
};

// macro token expansion for ps_1_1 instruction: texm3x3tex
PS_1_4::TokenInst PS_1_4::texm3x3pad[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T0

	// dp3 r4.b, r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_B
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0
	_token_end_

};


PS_1_4::MacroRegModify PS_1_4::texm3x3pad_MacroMods = {
	texm3x3pad, ARRAYSIZE(texm3x3pad),
	texm3xxpad_RegMods, ARRAYSIZE(texm3xxpad_RegMods)
};


// macro token expansion for ps_1_1 instruction: texm3x3pad
PS_1_4::TokenInst PS_1_4::texm3x3tex[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T1

	// dp3 r4.b, r(x), r(y)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_B
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0

	// texld r1, r4
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R1
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4
	_token_end_
};






PS_1_4::MacroRegModify PS_1_4::texm3x3tex_MacroMods = {
	texm3x3tex, ARRAYSIZE(texm3x3tex),
	texm3xxtex_RegMods, ARRAYSIZE(texm3xxtex_RegMods)
};


// macro token expansion for ps_1_1 instruction: texm3x3spec
PS_1_4::TokenInst PS_1_4::texm3x3spec[] = {
	// texcoord t(x)
	{		sid_TEXOP_PS1_1_3, sid_TEXCOORD
	_token_ sid_TEX_PS1_1_3, sid_1T3

	// dp3 r4.b, r3, r(x)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_B
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R3
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R0

	// dp3_x2 r3, r4, c(x)
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_DSTMOD,		sid_X2
	_token_ sid_REG_PS1_4,	sid_R3
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_CONSTANT,	sid_C0

	// mul r3, r3, c(x)
	_token_ sid_UNARYOP,	sid_MUL
	_token_ sid_REG_PS1_4,	sid_R3
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R3
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_CONSTANT,	sid_C0

	// dp3 r2, r4, r4
	_token_ sid_BINARYOP,	sid_DP3
	_token_ sid_REG_PS1_4,	sid_R2
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4

	// mad r4.rgb, 1-c(x), r2, r3
	_token_ sid_TERNARYOP,	sid_MAD
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_RGB
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_PRESRCMOD,	sid_INVERT
	_token_ sid_CONSTANT,	sid_C0
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R2
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R3

	// + mov r4.a, r2.r
	_token_ sid_UNARYOP,	sid_MOV
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_DSTMASK,	sid_A
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R2
	_token_ sid_SRCREP,		sid_RRRR

	// texld r3, r4.xyz_dz
	_token_ sid_TEXOP_PS1_4, sid_TEXLD
	_token_ sid_REG_PS1_4,	sid_R3
	_token_ sid_SEPERATOR,	sid_COMMA
	_token_ sid_REG_PS1_4,	sid_R4
	_token_ sid_TEXSWIZZLE,	sid_STRDR
	_token_end_

};

PS_1_4::RegModOffset PS_1_4::texm3x3spec_RegMods[] = {
	{8, R_BASE, 1},
	{15, R_BASE, 2},
	{21, C_BASE, 2},
	{33, C_BASE, 2},

};

PS_1_4::MacroRegModify PS_1_4::texm3x3spec_MacroMods = {
	texm3x3spec, ARRAYSIZE(texm3x3spec),
	texm3x3spec_RegMods, ARRAYSIZE(texm3x3spec_RegMods)
};


/* ********************* END OF CLASS STATIC DATA ********************************* */

PS_1_4::PS_1_4()
{
	// allocate enough room for a large pixel shader
	mPhase1TEX_mi.reserve(50);
	mPhase2TEX_mi.reserve(30);
	mPhase1ALU_mi.reserve(100);
	mPhase2ALU_mi.reserve(100);


	mSymbolTypeLib = PS_1_4_SymbolTypeLib;
	mSymbolTypeLibCnt = ARRAYSIZE(PS_1_4_SymbolTypeLib);
	mRootRulePath = PS_1_x_RulePath;
	mRulePathLibCnt = ARRAYSIZE(PS_1_x_RulePath);
	// tell compiler what the symbol id is for a numeric value
	mValueID = sid_VALUE;
	// The type library must have text definitions initialized
	// before compiler is invoked

	// only need to initialize the rule database once
	if(LibInitialized == false) {
		InitSymbolTypeLib();
		LibInitialized = true;
	}

	// set initial context to recognize PS base instructions
	mActiveContexts = ckp_PS_BASE;

}


bool PS_1_4::bindMachineInstInPassToFragmentShader(const MachineInstContainer & PassMachineInstructions)
{
  size_t instIDX = 0;
  size_t instCount = PassMachineInstructions.size();
  bool error = false;

  while ((instIDX < instCount) && !error) {
    switch(PassMachineInstructions[instIDX]) {
      case mi_COLOROP1:
        if((instIDX+7) < instCount)
          glColorFragmentOp1ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2], // dst
            PassMachineInstructions[instIDX+3], // dstMask
            PassMachineInstructions[instIDX+4], // dstMod
            PassMachineInstructions[instIDX+5], // arg1
            PassMachineInstructions[instIDX+6], // arg1Rep
            PassMachineInstructions[instIDX+7]);// arg1Mod
        instIDX += 8;
        break;

      case mi_COLOROP2:
        if((instIDX+10) < instCount)
          glColorFragmentOp2ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2], // dst
            PassMachineInstructions[instIDX+3], // dstMask
            PassMachineInstructions[instIDX+4], // dstMod
            PassMachineInstructions[instIDX+5], // arg1
            PassMachineInstructions[instIDX+6], // arg1Rep
            PassMachineInstructions[instIDX+7], // arg1Mod
            PassMachineInstructions[instIDX+8], // arg2
            PassMachineInstructions[instIDX+9], // arg2Rep
            PassMachineInstructions[instIDX+10]);// arg2Mod
        instIDX += 11;
        break;

      case mi_COLOROP3:
        if((instIDX+13) < instCount)
          glColorFragmentOp3ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],  // dst
            PassMachineInstructions[instIDX+3],  // dstMask
            PassMachineInstructions[instIDX+4],  // dstMod
            PassMachineInstructions[instIDX+5],  // arg1
            PassMachineInstructions[instIDX+6],  // arg1Rep
            PassMachineInstructions[instIDX+7],  // arg1Mod
            PassMachineInstructions[instIDX+8],  // arg2
            PassMachineInstructions[instIDX+9],  // arg2Rep
            PassMachineInstructions[instIDX+10], // arg2Mod
            PassMachineInstructions[instIDX+11], // arg2
            PassMachineInstructions[instIDX+12], // arg2Rep
            PassMachineInstructions[instIDX+13]);// arg2Mod
        instIDX += 14;
        break;

      case mi_ALPHAOP1:
        if((instIDX+6) < instCount)
          glAlphaFragmentOp1ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6]);  // arg1Mod
        instIDX += 7;
        break;

      case mi_ALPHAOP2:
        if((instIDX+9) < instCount)
          glAlphaFragmentOp2ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6],   // arg1Mod
            PassMachineInstructions[instIDX+7],   // arg2
            PassMachineInstructions[instIDX+8],   // arg2Rep
            PassMachineInstructions[instIDX+9]);  // arg2Mod
        instIDX += 10;
        break;

      case mi_ALPHAOP3:
        if((instIDX+12) < instCount)
          glAlphaFragmentOp3ATI(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6],   // arg1Mod
            PassMachineInstructions[instIDX+7],   // arg2
            PassMachineInstructions[instIDX+8],   // arg2Rep
            PassMachineInstructions[instIDX+9],   // arg2Mod
            PassMachineInstructions[instIDX+10],  // arg2
            PassMachineInstructions[instIDX+11],  // arg2Rep
            PassMachineInstructions[instIDX+12]); // arg2Mod
        instIDX += 13;
        break;

      case mi_SETCONSTANTS:
        if((instIDX+2) < instCount)
          glSetFragmentShaderConstantATI(PassMachineInstructions[instIDX+1], // dst
            &mConstants[PassMachineInstructions[instIDX+2]]);
        instIDX += 3;
        break;

      case mi_PASSTEXCOORD:
        if((instIDX+3) < instCount)
          glPassTexCoordATI(PassMachineInstructions[instIDX+1], // dst
            PassMachineInstructions[instIDX+2], // coord
            PassMachineInstructions[instIDX+3]); // swizzle
        instIDX += 4;
        break;

      case mi_SAMPLEMAP:
        if((instIDX+3) < instCount)
          glSampleMapATI(PassMachineInstructions[instIDX+1], // dst
            PassMachineInstructions[instIDX+2], // interp
            PassMachineInstructions[instIDX+3]); // swizzle
        instIDX += 4;
        break;

	  default:
		instIDX = instCount;
		// should generate an error since an unknown instruction was found
		// instead for now the bind process is terminated and the fragment program may still function
		// but its output may not be what was programmed

    } // end of switch

    error = (glGetError() != GL_NO_ERROR);
  }// end of while

  return !error;

}


size_t PS_1_4::getMachineInst( size_t Idx)
{
	if (Idx < mPhase1TEX_mi.size()) {
		return mPhase1TEX_mi[Idx];
	}
	else {
		Idx -= mPhase1TEX_mi.size();
		if (Idx < mPhase1ALU_mi.size()) {
			return mPhase1ALU_mi[Idx];
		}
		else {
			Idx -= mPhase1ALU_mi.size();
			if (Idx < mPhase2TEX_mi.size()) {
				return mPhase2TEX_mi[Idx];
			}
			else {
				Idx -= mPhase2TEX_mi.size();
				if (Idx < mPhase2ALU_mi.size()) {
					return mPhase2ALU_mi[Idx];
				}

			}

		}

	}

	return 0;

}


void PS_1_4::addMachineInst(const PhaseType phase, const uint inst)
{
	switch(phase) {

		case ptPHASE1TEX:
			mPhase1TEX_mi.push_back(inst);
			break;

		case ptPHASE1ALU:
			mPhase1ALU_mi.push_back(inst);
			break;

		case ptPHASE2TEX:
			mPhase2TEX_mi.push_back(inst);

			break;

		case ptPHASE2ALU:
			mPhase2ALU_mi.push_back(inst);
			break;


	} // end switch(phase)

}

size_t PS_1_4::getMachineInstCount()
{

	return (mPhase1TEX_mi.size() + mPhase1ALU_mi.size() + mPhase2TEX_mi.size() + mPhase2ALU_mi.size());
}


bool PS_1_4::bindAllMachineInstToFragmentShader()
{
	bool passed;

	// there are 4 machine instruction ques to pass to the ATI fragment shader
	passed = bindMachineInstInPassToFragmentShader(mPhase1TEX_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase1ALU_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase2TEX_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase2ALU_mi);
	return passed;

}


bool PS_1_4::expandMacro(const MacroRegModify & MacroMod)
{

	RegModOffset * regmod;

	// set source and destination registers in macro expansion
	for (uint i = 0; i < MacroMod.RegModSize; i++) {
		regmod = &MacroMod.RegMods[i];
		MacroMod.Macro[regmod->MacroOffset].mID = regmod->RegisterBase + mOpParrams[regmod->OpParramsIndex].Arg;

	}

	// turn macro support on so that ps.1.4 ALU instructions get put in phase 1 alu instruction sequence container
	mMacroOn = true;
	// pass macro tokens on to be turned into machine instructions
	// expand macro to ps.1.4 by doing recursive call to doPass2
	bool passed = Pass2scan(MacroMod.Macro, MacroMod.MacroSize);
	mMacroOn = false;

	return passed;
}


bool PS_1_4::BuildMachineInst()
{

	// check the states to see if a machine instruction can be assembled

	// assume all arguments have been set up
	bool passed = false;

	

	passed = true; // assume everything will go okay until proven otherwise

	// start with machine NOP instruction
	// this is used after the switch to see if an instruction was set up
	// determine which MachineInstID is required based on the op instruction
	mOpType = mi_NOP;

	switch(mOpInst) {
		// ALU operations
		case sid_ADD:
		case sid_SUB:
		case sid_MUL:
		case sid_MAD:
		case sid_LRP:
		case sid_MOV:
		case sid_CMP:
		case sid_CND:
		case sid_DP2ADD:
		case sid_DP3:
		case sid_DP4:
			mOpType = (MachineInstID)(mi_COLOROP1 + mArgCnt - 1);

			// if context is ps.1.x and Macro not on or a phase marker was found then put all ALU ops in phase 2 ALU container
			if (((mActiveContexts & ckp_PS_1_1) && !mMacroOn) || mPhaseMarkerFound) mInstructionPhase = ptPHASE2ALU;
			else mInstructionPhase = ptPHASE1ALU;
			// check for alpha op in destination register which is OpParrams[0]
			// if no Mask for destination then make it .rgba
			if(mOpParrams[0].MaskRep == 0) mOpParrams[0].MaskRep =
			GL_RED_BIT_ATI | GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT;
			if (mOpParrams[0].MaskRep & ALPHA_BIT) {
				mDo_Alpha = true;
				mOpParrams[0].MaskRep -= ALPHA_BIT;
				if(mOpParrams[0].MaskRep == 0) mOpType = mi_NOP; // only do alpha op
			}
			break;

		case sid_TEXCRD:
			mOpType = mi_PASSTEXCOORD;
			if (mPhaseMarkerFound) mInstructionPhase = ptPHASE2TEX;
			else mInstructionPhase = ptPHASE1TEX;
			break;

		case sid_TEXLD:
			mOpType = mi_SAMPLEMAP;
			if (mPhaseMarkerFound) mInstructionPhase = ptPHASE2TEX;
			else mInstructionPhase = ptPHASE1TEX;
			break;

		case sid_TEX: // PS_1_1 emulation
			mOpType = mi_TEX;
			mInstructionPhase = ptPHASE1TEX;
			break;

		case sid_TEXCOORD: // PS_1_1 emulation
			mOpType = mi_TEXCOORD;
			mInstructionPhase = ptPHASE1TEX;
			break;

		case sid_TEXREG2AR:
			passed = expandMacro(texreg2ar_MacroMods);
			break;

		case sid_TEXREG2GB:
			passed = expandMacro(texreg2gb_MacroMods);
			break;

		case sid_TEXDP3:
			passed = expandMacro(texdp3_MacroMods);
			break;

		case sid_TEXDP3TEX:
			passed = expandMacro(texdp3tex_MacroMods);
			break;

		case sid_TEXM3X2PAD:
			passed = expandMacro(texm3x2pad_MacroMods);
			break;

		case sid_TEXM3X2TEX:
			passed = expandMacro(texm3x2tex_MacroMods);
			break;

		case sid_TEXM3X3PAD:
			// only 2 texm3x3pad instructions allowed
			// use count to modify macro to select which mask to use
			if(mTexm3x3padCount<2) {
				texm3x3pad[4].mID = sid_R + mTexm3x3padCount;
				mTexm3x3padCount++;
				passed = expandMacro(texm3x3pad_MacroMods);

			}
			else passed = false;

			break;

		case sid_TEXM3X3TEX:
			passed = expandMacro(texm3x3tex_MacroMods);
			break;

		case sid_DEF:
			mOpType = mi_SETCONSTANTS;
			mInstructionPhase = ptPHASE1TEX;
			break;

		case sid_PHASE: // PS_1_4 only
			mPhaseMarkerFound = true;
			break;

	} // end of switch

	if(passed) passed = expandMachineInstruction();

	return passed;
}


bool PS_1_4::expandMachineInstruction()
{
	// now push instructions onto MachineInstructions container
	// assume that an instruction will be expanded
	bool passed = true;

	if (mOpType != mi_NOP) {

		// a machine instruction will be built
		// this is currently the last one being built so keep track of it
		if (mInstructionPhase == ptPHASE2ALU) { 
			mSecondLastInstructionPos = mLastInstructionPos;
			mLastInstructionPos = mPhase2ALU_mi.size();
		}


		switch (mOpType) {
			case mi_COLOROP1:
			case mi_COLOROP2:
			case mi_COLOROP3:
				{
					addMachineInst(mInstructionPhase, mOpType);
					addMachineInst(mInstructionPhase, mSymbolTypeLib[mOpInst].mPass2Data);
					// send all parameters to machine inst container
					for(int i=0; i<=mArgCnt; i++) {
						addMachineInst(mInstructionPhase, mOpParrams[i].Arg);
						addMachineInst(mInstructionPhase, mOpParrams[i].MaskRep);
						addMachineInst(mInstructionPhase, mOpParrams[i].Mod);
						// check if source register read is valid in this phase
						passed &= isRegisterReadValid(mInstructionPhase, i);
					}

					// record which registers were written to and in which phase
					// mOpParrams[0].Arg is always the destination register r0 -> r5
					updateRegisterWriteState(mInstructionPhase);

				}
				break;

			case mi_SETCONSTANTS:
				addMachineInst(mInstructionPhase, mOpType);
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg); // dst
				addMachineInst(mInstructionPhase, mConstantsPos); // index into constants array
				break;

			case mi_PASSTEXCOORD:
			case mi_SAMPLEMAP:
				// if source is a temp register than place instruction in phase 2 Texture ops
				if ((mOpParrams[1].Arg >= GL_REG_0_ATI) && (mOpParrams[1].Arg <= GL_REG_5_ATI)) {
					mInstructionPhase = ptPHASE2TEX;
				}

				addMachineInst(mInstructionPhase, mOpType);
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg); // dst
				addMachineInst(mInstructionPhase, mOpParrams[1].Arg); // coord
				addMachineInst(mInstructionPhase, mOpParrams[1].MaskRep + GL_SWIZZLE_STR_ATI); // swizzle
				// record which registers were written to and in which phase
				// mOpParrams[0].Arg is always the destination register r0 -> r5
				updateRegisterWriteState(mInstructionPhase);
				break;

			case mi_TEX: // PS_1_1 emulation - turn CISC into RISC - phase 1
				addMachineInst(mInstructionPhase, mi_SAMPLEMAP);
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg); // dst
				// tex tx becomes texld rx, tx with x: 0 - 3
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg - GL_REG_0_ATI + GL_TEXTURE0_ARB); // interp
				// default to str which fills rgb of destination register
				addMachineInst(mInstructionPhase, GL_SWIZZLE_STR_ATI); // swizzle
				// record which registers were written to and in which phase
				// mOpParrams[0].Arg is always the destination register r0 -> r5
				updateRegisterWriteState(mInstructionPhase);
				break;

			case mi_TEXCOORD: // PS_1_1 emulation - turn CISC into RISC - phase 1
				addMachineInst(mInstructionPhase, mi_PASSTEXCOORD);
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg); // dst
				// texcoord tx becomes texcrd rx, tx with x: 0 - 3
				addMachineInst(mInstructionPhase, mOpParrams[0].Arg - GL_REG_0_ATI + GL_TEXTURE0_ARB); // interp
				// default to str which fills rgb of destination register
				addMachineInst(mInstructionPhase, GL_SWIZZLE_STR_ATI); // swizzle
				// record which registers were written to and in which phase
				// mOpParrams[0].Arg is always the destination register r0 -> r5
				updateRegisterWriteState(mInstructionPhase);
				break;

            case mi_ALPHAOP1:
            case mi_ALPHAOP2:
            case mi_ALPHAOP3:
            case mi_TEXREG2RGB:
            case mi_NOP:
                break;


		} // end of switch (mOpType)
	} // end of if (mOpType != mi_NOP)

	if(mDo_Alpha) {
		// process alpha channel
		//
		// a scaler machine instruction will be built
		// this is currently the last one being built so keep track of it
		if (mInstructionPhase == ptPHASE2ALU) { 
			mSecondLastInstructionPos = mLastInstructionPos;
			mLastInstructionPos = mPhase2ALU_mi.size();
		}

		MachineInstID alphaoptype = (MachineInstID)(mi_ALPHAOP1 + mArgCnt - 1);
		addMachineInst(mInstructionPhase, alphaoptype);
		addMachineInst(mInstructionPhase, mSymbolTypeLib[mOpInst].mPass2Data);
		// put all parameters in instruction que
		for(int i=0; i<=mArgCnt; i++) {
			addMachineInst(mInstructionPhase, mOpParrams[i].Arg);
			// destination parameter has no mask since it is the alpha channel
			// don't push mask for parrameter 0 (dst)
			if(i>0) addMachineInst(mInstructionPhase, mOpParrams[i].MaskRep);
			addMachineInst(mInstructionPhase, mOpParrams[i].Mod);
			// check if source register read is valid in this phase
			passed &= isRegisterReadValid(mInstructionPhase, i);
		}

		updateRegisterWriteState(mInstructionPhase);
	}

	// instruction passed on to machine instruction so clear the pipe
	clearMachineInstState();

	return passed;

}


void PS_1_4::updateRegisterWriteState(const PhaseType phase)
{
	int reg_offset = mOpParrams[0].Arg - GL_REG_0_ATI;

	switch(phase) {

		case ptPHASE1TEX:
		case ptPHASE1ALU:
			Phase_RegisterUsage[reg_offset].Phase1Write = true;
			break;

		case ptPHASE2TEX:
		case ptPHASE2ALU:
			Phase_RegisterUsage[reg_offset].Phase2Write = true;
			break;

	} // end switch(phase)

}


bool PS_1_4::isRegisterReadValid(const PhaseType phase, const int param)
{
	bool passed = true; // assume everything will go alright
	// if in phase 2 ALU and argument is a source
	if((phase == ptPHASE2ALU) && (param>0)) {
		// is source argument a temp register r0 - r5?
		if((mOpParrams[param].Arg >= GL_REG_0_ATI) && (mOpParrams[param].Arg <= GL_REG_5_ATI)) {
			int reg_offset = mOpParrams[param].Arg - GL_REG_0_ATI;
			// if register was not written to in phase 2 but was in phase 1
			if((Phase_RegisterUsage[reg_offset].Phase2Write == false) && Phase_RegisterUsage[reg_offset].Phase1Write) {
				// only perform register pass if there are ALU instructions in phase 1
				
				if(mPhase1ALU_mi.size() > 0) {
					// build machine instructions for passing a register from phase 1 to phase 2
					// NB: only rgb components of register will get passed

					addMachineInst(ptPHASE2TEX, mi_PASSTEXCOORD);
					addMachineInst(ptPHASE2TEX, mOpParrams[param].Arg); // dst
					addMachineInst(ptPHASE2TEX, mOpParrams[param].Arg); // coord
					addMachineInst(ptPHASE2TEX, GL_SWIZZLE_STR_ATI); // swizzle
					// mark register as being written to
					Phase_RegisterUsage[reg_offset].Phase2Write = true;
				}

			}
			// register can not be used because it has not been written to previously
			else passed = false;
		}

	}

	return passed;

}


void PS_1_4::optimize()
{
	// perform some optimizations on ps.1.1 machine instructions
	if (mActiveContexts & ckp_PS_1_1) {
		// need to check last few instructions to make sure r0 is set
		// ps.1.1 emulation uses r4 for r0 so last couple of instructions will probably require
		// changine destination register back to r0
		if (mLastInstructionPos < mPhase2ALU_mi.size()) {
			// first argument at mLastInstructionPos + 2 is destination register for all ps.1.1 ALU instructions
			mPhase2ALU_mi[mLastInstructionPos + 2] = GL_REG_0_ATI; 
			// if was an alpha op only then modify second last instruction destination register
			if ((mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP1) ||
				(mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP2) ||
				(mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP3)
				
				) {

				mPhase2ALU_mi[mSecondLastInstructionPos + 2] = GL_REG_0_ATI; 
			}

		}// end if (mLastInstructionPos < mMachineInstructions.size())

	}// end if (mActiveContexts & ckp_PS_1_1)

}


void PS_1_4::clearMachineInstState()
{
	// set current Machine Instruction State to baseline
	mOpType = mi_NOP;
	mOpInst = sid_INVALID;
	mDo_Alpha = false;
	mArgCnt = 0;

	for(int i=0; i<MAXOPPARRAMS; i++) {
		mOpParrams[i].Arg = GL_NONE;
		mOpParrams[i].Filled = false;
		mOpParrams[i].MaskRep = GL_NONE;
		mOpParrams[i].Mod = GL_NONE;
	}

}


void PS_1_4::clearAllMachineInst()
{

	mPhase1TEX_mi.clear();
	mPhase1ALU_mi.clear();
	mPhase2TEX_mi.clear();
	mPhase2ALU_mi.clear();

	// reset write state for all registers
	for(int i = 0; i<6; i++) {
		Phase_RegisterUsage[i].Phase1Write = false;
		Phase_RegisterUsage[i].Phase2Write = false;

	}

	mPhaseMarkerFound = false;
	mConstantsPos = -4;
	// keep track of the last instruction built
	// this info is used at the end of pass 2 to optimize the machine code
	mLastInstructionPos = 0;
	mSecondLastInstructionPos = 0;

	mMacroOn = false;  // macro's off at the beginning
	mTexm3x3padCount = 0;

}

bool PS_1_4::doPass2()
{
	clearAllMachineInst();
	// if pass 2 was successful, optimize the machine instructions
	bool passed = Pass2scan(&mTokenInstructions[0], mTokenInstructions.size());
	if (passed) optimize();  

	return passed;

}


bool PS_1_4::Pass2scan(const TokenInst * Tokens, const size_t size)
{

	// execute TokenInstructions to build MachineInstructions
	bool passed = true;
	SymbolDef* cursymboldef;
	uint ActiveNTTRuleID;

	clearMachineInstState();


	// iterate through all the tokens and build machine instruction
	// for each machine instruction need: optype, opinst, and up to 5 parameters
	for(uint i = 0; i < size; i++) {
		// lookup instruction type in library

		cursymboldef = &mSymbolTypeLib[Tokens[i].mID];
		ActiveNTTRuleID = Tokens[i].mNTTRuleID;
		mCurrentLine = Tokens[i].mLine;
		mCharPos = Tokens[i].mPos;

		switch(ActiveNTTRuleID) {

			case sid_CONSTANT:
			case sid_COLOR:
			case sid_REG_PS1_4:
			case sid_TEX_PS1_4:
			case sid_REG_PS1_1_3:
			case sid_TEX_PS1_1_3:
				// registars can be used for read and write so they can be used for dst and arg
				passed = setOpParram(cursymboldef);
				break;


			case sid_DEFCONST:
			case sid_UNARYOP:
			case sid_BINARYOP:
			case sid_TERNARYOP:
			case sid_TEXOP_PS1_1_3:
			case sid_TEXOP_PS1_4:
			case sid_PHASEMARKER:
			case sid_TEXCISCOP_PS1_1_3:
				// if the last instruction has not been passed on then do it now
				// make sure the pipe is clear for a new instruction
				BuildMachineInst();
				if(mOpInst == sid_INVALID) {
					mOpInst = cursymboldef->mID;
				}
				else passed = false;
				break;

			case sid_DSTMASK:
			case sid_SRCREP:
			case sid_TEXSWIZZLE:
				// could be a dst mask or a arg replicator
				// if dst mask and alpha included then make up a alpha instruction: maybe best to wait until instruction args completed
				mOpParrams[mArgCnt].MaskRep = cursymboldef->mPass2Data;
				break;

			case sid_DSTMOD:
			case sid_DSTSAT:
			case sid_PRESRCMOD:
			case sid_POSTSRCMOD:
				mOpParrams[mArgCnt].Mod |= cursymboldef->mPass2Data;
				break;


			case sid_NUMVAL:
				passed = setOpParram(cursymboldef);
				// keep track of how many values are used
				// update Constants array position
				mConstantsPos++;
				break;

			case sid_SEPERATOR:
				mArgCnt++;
				break;
		} // end of switch

		if(!passed) break;
	}// end of for: i<TokenInstCnt

	// check to see if there is still an instruction left in the pipe
	if(passed) {
		BuildMachineInst();
		// if there are no more instructions in the pipe than OpInst should be invalid
		if(mOpInst != sid_INVALID) passed = false;
	}


	return passed;
}



bool PS_1_4::setOpParram(const SymbolDef* symboldef)
{
  bool success = true;
  if(mArgCnt<MAXOPPARRAMS) {
    if(mOpParrams[mArgCnt].Filled) mArgCnt++;
  }
  if (mArgCnt<MAXOPPARRAMS) {
    mOpParrams[mArgCnt].Filled = true;
    mOpParrams[mArgCnt].Arg = symboldef->mPass2Data;
  }
  else success = false;

  return success;
}




// *********************************************************************************
//  this is where the tests are carried out to make sure the PS_1_4 compiler works

#ifdef _DEBUG
// check the functionality of functions in PS_1_4: each test will print to the output file PASSED of FAILED
void PS_1_4::test()
{
  

  struct test1result{
    char character;
    int line;
  };

  struct testfloatresult{
    char *teststr;
    float fvalue;
    int charsize;
  };

  char TestStr1[] = "   \n\r  //c  \n\r// test\n\r  \t  c   - \n\r ,  e";
  test1result test1results[] = {
    {'c', 4},
    {'-', 4},
    {',', 5},
    {'e', 5}
  };

  testfloatresult testfloatresults[] = {
    {"1 test", 1.0f, 1},
    {"2.3f test", 2.3f, 3},
    {"-0.5 test", -0.5f, 4},
    {" 23.6 test", 23.6f, 5},
    {"  -0.021 test", -0.021f, 8},
    {"12 test", 12.0f, 2},
    {"3test", 3.0f, 1}
  };



  SymbolID test3result[] = { sid_MOV, sid_COMMA, sid_MUL, sid_ADD, sid_NEGATE, sid_T0
  };

  #define PART2INST 17
  char TestStr3[] = "mov r0,c1";
  char TestSymbols[] = "mov";
  char passed[] = "PASSED\n";
  char failed[] = "***** FAILED *****\n";

  int resultID = 0;

  // loop variable used in for loops
  int i;
  fp = fopen("ASMTests.txt", "wt");

// **************************************************************
  // first test: see if positionToNextSymbol can find a valid Symbol
  fprintf(fp, "Testing: positionToNextSymbol\n");

  mSource = TestStr1;
  mCharPos = 0;
  mCurrentLine = 1;
  mEndOfSource = (int)strlen(mSource);
  while (positionToNextSymbol()) {
    fprintf(fp,"  character found: [%c]   Line:%d  : " , mSource[mCharPos], mCurrentLine);
    if( (mSource[mCharPos] == test1results[resultID].character) && (mCurrentLine==test1results[resultID].line)) fprintf(fp, passed);
    else fprintf(fp, failed);
    resultID++;
    mCharPos++;
  }
  fprintf(fp, "finished testing: positionToNextSymbol\n");


// **************************************************************
  // Second Test
  // did the type lib get initialized properly with a default name index
  fprintf(fp, "\nTesting: getTypeDefText\n");
  const char* resultstr = getTypeDefText(sid_MOV);
  fprintf(fp, "  default name of mov is: [%s]: %s", resultstr, (strcmp("mov", resultstr)==0)?passed:failed);
  fprintf(fp, "finished testing: getTypeDefText\n");

// **************************************************************
// **************************************************************
  // fourth test - does isSymbol work correctly
  fprintf(fp, "\nTesting: isSymbol\n");
  mSource = TestStr3;
  mCharPos = 0;
  fprintf(fp, "  before: [%s]\n", mSource + mCharPos);
  fprintf(fp, "  symbol to find: [%s]\n", TestSymbols);
  if(isSymbol(TestSymbols, resultID)) {
    fprintf(fp, "  after: [%s] : %s", mSource + resultID + 1, (mSource[resultID + 1] == 'r')? passed:failed);
  }
  else fprintf(fp, failed);
  fprintf(fp,"  symbol size: %d\n", resultID);
  fprintf(fp, "finished testing: isSymbol\n");

// **************************************************************
  fprintf(fp, "\nTesting: isFloatValue\n");
  float fvalue = 0;
  int charsize = 0;
  char teststrfloat1[] = "1 test";
  mCharPos = 0;
  int testsize = ARRAYSIZE(testfloatresults);
  for(i=0; i<testsize; i++) {
    mSource = testfloatresults[i].teststr;
    fprintf(fp, "  test string [%s]\n", mSource);
    isFloatValue(fvalue, charsize);
    fprintf(fp, "   value is: %f should be %f: %s", fvalue, testfloatresults[i].fvalue, (fvalue == testfloatresults[i].fvalue)?passed:failed);
    fprintf(fp, "   char size is: %d should be %d: %s", charsize, testfloatresults[i].charsize, (charsize == testfloatresults[i].charsize)?passed:failed);
  }

  fprintf(fp, "finished testing: isFloatValue\n");


// **************************************************************

  // simple compile test:
  char CompileTest1src[] = "ps.1.4\n";
  SymbolID CompileTest1result[] = {sid_PS_1_4};

  testCompile("Basic PS_1_4", CompileTest1src, CompileTest1result, ARRAYSIZE(CompileTest1result));

// **************************************************************
  char CompileTest2src[] = "ps.1.1\n";
  SymbolID CompileTest2result[] = {sid_PS_1_1};

  testCompile("Basic PS_1_1", CompileTest2src, CompileTest2result, ARRAYSIZE(CompileTest2result));

// **************************************************************
  char CompileTest3src[] = "ps.1.4\ndef c0, 1.0, 2.0, 3.0, 4.0\n";
  SymbolID CompileTest3result[] = {sid_PS_1_4, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE};

  testCompile("PS_1_4 with defines", CompileTest3src, CompileTest3result, ARRAYSIZE(CompileTest3result));


// **************************************************************
  char CompileTest4src[] = "ps.1.4\n//test kkl \ndef c0, 1.0, 2.0, 3.0, 4.0\ndef c3, 1.0, 2.0, 3.0, 4.0\n";
  SymbolID CompileTest4result[] = {sid_PS_1_4, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE,sid_DEF, sid_C3, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE};

  testCompile("PS_1_4 with 2 defines", CompileTest4src, CompileTest4result, ARRAYSIZE(CompileTest4result));

// **************************************************************
  char CompileTest5src[] = "ps.1.4\ndef c0, 1.0, 2.0, 3.0, 4.0\n";
  SymbolID CompileTest5result[] = {sid_PS_1_4, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE};
  GLuint CompileTest5MachinInstResults[] = {mi_SETCONSTANTS, GL_CON_0_ATI, 0};

  testCompile("PS_1_4 with defines", CompileTest3src, CompileTest3result, ARRAYSIZE(CompileTest3result), CompileTest5MachinInstResults, ARRAYSIZE(CompileTest5MachinInstResults));
// **************************************************************
  char CompileTest6Src[] = "ps.1.4\nmov r0.xzw, c1 \nmul r3, r2, c3";
  SymbolID CompileTest6result[] = {sid_PS_1_4, sid_MOV, sid_R0, sid_RBA, sid_COMMA, sid_C1,
	  sid_MUL, sid_R3, sid_COMMA, sid_R2, sid_COMMA, sid_C3};
  
  testCompile("PS_1_4 ALU simple", CompileTest6Src, CompileTest6result, ARRAYSIZE(CompileTest6result));

// **************************************************************
  // test to see if PS_1_4 compile pass 2 generates the proper machine instructions
	char CompileTest7Src[] = "ps.1.4\ndef c0,1.0,2.0,3.0,4.0\nmov_x8 r1,v0\nmov r0,r1.g";

	SymbolID CompileTest7result[] = {
		sid_PS_1_4, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_MOV, sid_X8, sid_R1, sid_COMMA,
		sid_V0, sid_MOV, sid_R0, sid_COMMA, sid_R1, sid_GGGG
	};

	GLuint CompileTest7MachinInstResults[] = {
		mi_SETCONSTANTS, GL_CON_0_ATI, 0,
		mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, RGB_BITS, GL_8X_BIT_ATI,	GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE,
		mi_ALPHAOP1, GL_MOV_ATI, GL_REG_1_ATI, GL_8X_BIT_ATI, GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE,
		mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE,GL_REG_1_ATI, GL_GREEN, GL_NONE,
		mi_ALPHAOP1, GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_1_ATI, GL_GREEN, GL_NONE,
	};


	testCompile("PS_1_4 ALU simple modifier", CompileTest7Src, CompileTest7result, ARRAYSIZE(CompileTest7result), CompileTest7MachinInstResults, ARRAYSIZE(CompileTest7MachinInstResults));

// **************************************************************
// test to see if a PS_1_1 can be compiled - pass 1 and pass 2 are checked

	char TestStr5[] = "ps.1.1\ndef c0,1.0,2.0,3.0,4.0\ntex t0\n// test\ntex t1\ndp3 t0.rgb, t0_bx2, t1_bx2\n+ mov r0,1 - t0";

	SymbolID test5result[] = {
		sid_PS_1_1, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_TEX, sid_1T0, sid_TEX, sid_1T1,
		sid_DP3, sid_1T0, sid_RGB, sid_COMMA, sid_1T0, sid_BX2, sid_COMMA, sid_1T1, sid_BX2,
		
		sid_PLUS, sid_MOV, sid_1R0, sid_COMMA, sid_INVERT, sid_1T0
	};

	GLuint test5MachinInstResults[] = {
		mi_SETCONSTANTS, GL_CON_0_ATI, 0, mi_SAMPLEMAP, GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI, mi_SAMPLEMAP, GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI,
		mi_COLOROP2, GL_DOT3_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_COMP_BIT_ATI, mi_ALPHAOP1, GL_MOV_ATI, GL_REG_0_ATI,
		GL_NONE, GL_REG_0_ATI, GL_NONE, GL_COMP_BIT_ATI,
	};


	testCompile("PS_1_1 Texture simple", TestStr5, test5result, ARRAYSIZE(test5result), test5MachinInstResults, ARRAYSIZE(test5MachinInstResults));


// **************************************************************
// test to see if a PS_1_2 CISC instructions can be compiled - pass 1 and pass 2 are checked

	char TestStr6[] = "ps.1.2\ndef c0,1.0,2.0,3.0,4.0\ntex t0\n// test\ntexreg2ar t1, t0";

	SymbolID test6result[] = {
		sid_PS_1_2, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE,
		sid_TEX, sid_1T0,
		sid_TEXREG2AR, sid_1T1, sid_COMMA, sid_1T0
	};

	GLuint test6MachinInstResults[] = {
		// def c0
		mi_SETCONSTANTS, GL_CON_0_ATI, 0,
		// texld r0, t0.str
		mi_SAMPLEMAP, GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI,
		// mov r1.r, r0.a 
		mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, GL_RED_BIT_ATI, GL_NONE, GL_REG_0_ATI, GL_ALPHA, GL_NONE,
		// mov r1.g, r0.r
		mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, GL_GREEN_BIT_ATI, GL_NONE, GL_REG_0_ATI, GL_RED, GL_NONE,
		// texld r1, r1
		mi_SAMPLEMAP, GL_REG_1_ATI, GL_REG_1_ATI, GL_SWIZZLE_STR_ATI,
	};


	testCompile("PS_1_2 CISC instructions", TestStr6, test6result, ARRAYSIZE(test6result), test6MachinInstResults, ARRAYSIZE(test6MachinInstResults));

// **************************************************************
// test to see if a PS_1_4 two phase can be compiled - pass 1 and pass 2 are checked

	char TestStr7[] = "ps.1.4\ndef c0,1.0,2.0,3.0,4.0\ntexld r0, t0\n// test\nmul r0, r0, c0\nphase\ntexld r1, r0\nmul r0,r0,r1\n";

	SymbolID test7result[] = {
		sid_PS_1_4,
		// def c0,1.0,2.0,3.0,4.0
		sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE,
		// texld r0, t0
		sid_TEXLD, sid_R0, sid_COMMA, sid_T0,
		// mul r0, r0, c0
		sid_MUL, sid_R0, sid_COMMA, sid_R0, sid_COMMA, sid_C0,
		// phase
		sid_PHASE,
		// texld r1, r0
		sid_TEXLD, sid_R1, sid_COMMA, sid_R0,
		// mul r0, r0, r1
		sid_MUL, sid_R0, sid_COMMA, sid_R0, sid_COMMA, sid_R1,

	};

	GLuint test7MachinInstResults[] = {
		// def c0
		mi_SETCONSTANTS, GL_CON_0_ATI, 0,
		// texld r0, t0.str
		mi_SAMPLEMAP, GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI,
		// mul r0, r0, c0
		mi_COLOROP2, GL_MUL_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_CON_0_ATI, GL_NONE, GL_NONE,
		mi_ALPHAOP2, GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_CON_0_ATI, GL_NONE, GL_NONE,
		// phase
		// texld r1, r0.str
		mi_SAMPLEMAP, GL_REG_1_ATI, GL_REG_0_ATI, GL_SWIZZLE_STR_ATI,
		// pass ro register
		mi_PASSTEXCOORD, GL_REG_0_ATI, GL_REG_0_ATI, GL_SWIZZLE_STR_ATI,
		// mul r0, r0, r1
		mi_COLOROP2, GL_MUL_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_NONE,
		// mul r0.a, r0
		mi_ALPHAOP2, GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_NONE,
	};


	testCompile("PS_1_4 texture complex : Phase - instructions", TestStr7, test7result, ARRAYSIZE(test7result), test7MachinInstResults, ARRAYSIZE(test7MachinInstResults));


	fclose(fp);
	fp = NULL;
	//reset contexts

// **************************************************************
}

void PS_1_4::testCompile(char* testname, char* teststr, SymbolID* testresult, uint testresultsize, GLuint* MachinInstResults, uint MachinInstResultsSize)
{

	char passed[] = "PASSED\n";
	char failed[] = "***** FAILED ****\n";

	setActiveContexts(ckp_PS_BASE);

	fprintf(fp, "\n*** TESTING: %s Compile: Check Pass 1 and 2\n", testname);
	fprintf(fp, "  source to compile:\n[\n%s\n]\n", teststr);
	bool compiled = compile(teststr);
	fprintf(fp, "  Pass 1 Lines scaned: %d, Tokens produced: %d out of %d: %s",
		mCurrentLine, mTokenInstructions.size(), testresultsize,
		(mTokenInstructions.size() == (uint)testresultsize) ? passed : failed);

	fprintf(fp, "\n  Validating Pass 1:\n");

	fprintf(fp, "\n  Tokens:\n");
    uint i;
	for(i = 0; i<(mTokenInstructions.size()); i++) {
		fprintf(fp,"    Token[%d] [%s] %d: [%s] %d: %s", i, getTypeDefText(mTokenInstructions[i].mID),
			mTokenInstructions[i].mID, getTypeDefText(testresult[i]), testresult[i],
			(mTokenInstructions[i].mID == (uint)testresult[i]) ? passed : failed);
	}

	if(MachinInstResults != NULL) {
		fprintf(fp, "\n  Machine Instructions:\n");

		fprintf(fp, "  Pass 2 Machine Instructions generated: %d out of %d: %s", getMachineInstCount(),
			MachinInstResultsSize, (getMachineInstCount() == MachinInstResultsSize) ? passed : failed);

		size_t MIcount = getMachineInstCount();

		fprintf(fp, "\n  Validating Pass 2:\n");
		for(i = 0; i<MIcount; i++) {
			fprintf(fp,"    instruction[%d] = 0x%x : 0x%x : %s", i, getMachineInst(i), MachinInstResults[i], (getMachineInst(i) == MachinInstResults[i]) ? passed : failed);
		}

		fprintf(fp, "\n  Constants:\n");
		for(i=0; i<4; i++) {
			fprintf(fp, "    Constants[%d] = %f : %s", i, mConstants[i], (mConstants[i] == (1.0f+i)) ? passed : failed);
		}
	}
	if(!compiled) fprintf(fp, failed);

	fprintf(fp, "\nfinished testing: %s Compile: Check Pass 2\n\n", testname);

	setActiveContexts(ckp_PS_BASE);
}

void PS_1_4::testbinder()
{
  FILE* fp;
  char BindTestStr[] = "mov_x8 r0,v0";
  char passed[] = "PASSED\n";
  char failed[] = "FAILED\n";
  #define BinderInstCnt 8
  GLuint BindMachinInst[BinderInstCnt] = {mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_8X_BIT_ATI,
   GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE};

  fp = fopen("ASMTests.txt", "at");
  fprintf(fp,"Testing: bindMachineInstToFragmentShader\n");
  // fill Machin instruction container with predefined code
  clearAllMachineInst();
  for(int i=0; i<BinderInstCnt; i++) {
    mPhase2ALU_mi.push_back(BindMachinInst[i]);
  }
  fprintf(fp,"bindMachineInstToFragmentShader succes: %s\n",bindAllMachineInstToFragmentShader()?passed:failed);
  fprintf(fp,"finished testing: bindAllMachineInstToFragmentShader\n");
  fclose(fp);
}

#endif



