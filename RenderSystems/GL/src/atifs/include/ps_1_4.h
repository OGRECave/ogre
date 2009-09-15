/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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


/**
	A number of invaluable references were used to put together this ps.1.x compiler for ATI_fragment_shader execution

	References:
		1. MSDN: DirectX 8.1 Reference
		2. Wolfgang F. Engel "Fundamentals of Pixel Shaders - Introduction to Shader Programming Part III" on gamedev.net
		3. Martin Ecker - XEngine
		4. Shawn Kirst - ps14toATIfs
		5. Jason L. Mitchell "Real-Time 3D Graphics With Pixel Shaders" 
		6. Jason L. Mitchell "1.4 Pixel Shaders"
		7. Jason L. Mitchell and Evan Hart "Hardware Shading with EXT_vertex_shader and ATI_fragment_shader"
		6. ATI 8500 SDK
		7. GL_ATI_fragment_shader extension reference

*/
//---------------------------------------------------------------------------
#ifndef ps_1_4H
#define ps_1_4H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OgreGLPrerequisites.h"
#include "Compiler2Pass.h"


//---------------------------------------------------------------------------
// macro to get the size of a static array
#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))

#define ALPHA_BIT 0x08
#define RGB_BITS 0x07

// Context key patterns
#define ckp_PS_BASE 0x1
#define ckp_PS_1_1  0x2
#define ckp_PS_1_2  0x4
#define ckp_PS_1_3  0x8
#define ckp_PS_1_4  0x10

#define ckp_PS_1_4_BASE (ckp_PS_BASE + ckp_PS_1_4)




/** Subclasses Compiler2Pass to provide a ps_1_x compiler that takes DirectX pixel shader assembly
	and converts it to a form that can be used by ATI_fragment_shader OpenGL API
@remarks
	all ps_1_1, ps_1_2, ps_1_3, ps_1_4 assembly instructions are recognized but not all are passed
	on to ATI_fragment_shader.	ATI_fragment_shader does not have an equivelant directive for
	texkill or texdepth instructions.

	The user must provide the GL binding interfaces.

	A Test method is provided to verify the basic operation of the compiler which outputs the test
	results to a file.


*/
class PS_1_4 : public Compiler2Pass{
private:
	enum RWAflags {rwa_NONE = 0, rwa_READ = 1, rwa_WRITE = 2};

	enum MachineInstID {mi_COLOROP1, mi_COLOROP2, mi_COLOROP3, mi_ALPHAOP1, mi_ALPHAOP2,
						mi_ALPHAOP3, mi_SETCONSTANTS, mi_PASSTEXCOORD, mi_SAMPLEMAP, mi_TEX,
						mi_TEXCOORD, mi_TEXREG2RGB, mi_NOP
	};

	struct  TokenInstType{
	  char* Name;
	  GLuint ID;

	};

	struct RegisterUsage {
		bool Phase1Write;
		bool Phase2Write;
	};

	// Token ID enumeration
	enum SymbolID {
		// Terminal Tokens section

		// DirectX pixel shader source formats 
		sid_PS_1_4, sid_PS_1_1, sid_PS_1_2, sid_PS_1_3,
					
		// PS_BASE
		sid_C0, sid_C1, sid_C2, sid_C3, sid_C4, sid_C5, sid_C6, sid_C7,
		sid_V0, sid_V1,
		sid_ADD, sid_SUB, sid_MUL, sid_MAD, sid_LRP, sid_MOV, sid_CMP, sid_CND,
		sid_DP3, sid_DP4, sid_DEF,
		sid_R, sid_RA, sid_G, sid_GA, sid_B, sid_BA, sid_A, sid_RGBA, sid_RGB,
		sid_RG, sid_RGA, sid_RB, sid_RBA, sid_GB, sid_GBA,
		sid_RRRR, sid_GGGG, sid_BBBB, sid_AAAA,
		sid_X2, sid_X4, sid_D2, sid_SAT,
		sid_BIAS, sid_INVERT, sid_NEGATE, sid_BX2,
		sid_COMMA, sid_VALUE,

		//PS_1_4 sid
		sid_R0, sid_R1, sid_R2, sid_R3, sid_R4, sid_R5,
		sid_T0, sid_T1, sid_T2, sid_T3, sid_T4, sid_T5,
		sid_DP2ADD,
		sid_X8, sid_D8, sid_D4,
		sid_TEXCRD, sid_TEXLD,
		sid_STR, sid_STQ,
		sid_STRDR, sid_STQDQ,
		sid_BEM,
		sid_PHASE,

		//PS_1_1 sid
		sid_1R0, sid_1R1, sid_1T0, sid_1T1, sid_1T2, sid_1T3,
		sid_TEX, sid_TEXCOORD, sid_TEXM3X2PAD,
		sid_TEXM3X2TEX, sid_TEXM3X3PAD, sid_TEXM3X3TEX, sid_TEXM3X3SPEC, sid_TEXM3X3VSPEC,
		sid_TEXREG2AR, sid_TEXREG2GB,
		
		//PS_1_2 side
		sid_TEXREG2RGB, sid_TEXDP3, sid_TEXDP3TEX,

		// common
		sid_SKIP, sid_PLUS,

		// non-terminal tokens section
		sid_PROGRAM, sid_PROGRAMTYPE, sid_DECLCONSTS, sid_DEFCONST,
		sid_CONSTANT, sid_COLOR,
		sid_TEXSWIZZLE, sid_UNARYOP,
		sid_NUMVAL, sid_SEPERATOR, sid_ALUOPS, sid_TEXMASK, sid_TEXOP_PS1_1_3,
		sid_TEXOP_PS1_4,
		sid_ALU_STATEMENT, sid_DSTMODSAT, sid_UNARYOP_ARGS, sid_REG_PS1_4,
		sid_TEX_PS1_4, sid_REG_PS1_1_3, sid_TEX_PS1_1_3, sid_DSTINFO,
		sid_SRCINFO, sid_BINARYOP_ARGS, sid_TERNARYOP_ARGS, sid_TEMPREG,
		sid_DSTMASK, sid_PRESRCMOD, sid_SRCNAME, sid_SRCREP, sid_POSTSRCMOD,
		sid_DSTMOD, sid_DSTSAT, sid_BINARYOP,  sid_TERNARYOP,
		sid_TEXOPS_PHASE1, sid_COISSUE, sid_PHASEMARKER, sid_TEXOPS_PHASE2, 
		sid_TEXREG_PS1_4, sid_TEXOPS_PS1_4, sid_TEXOPS_PS1_1_3, sid_TEXCISCOP_PS1_1_3,


		// last token
		sid_INVALID = BAD_TOKEN // must be last in enumeration
	};

	/// structure used to keep track of arguments and instruction parameters
	struct OpParram {
	  GLuint Arg;		// type of argument
	  bool Filled;		// has it been filled yet
	  GLuint MaskRep;	// Mask/Replicator flags
	  GLuint Mod;		// argument modifier
	};

	typedef std::vector<uint> MachineInstContainer;
	//typedef MachineInstContainer::iterator MachineInstIterator;


	// there are 2 phases with 2 subphases each
	enum PhaseType {ptPHASE1TEX, ptPHASE1ALU, ptPHASE2TEX, ptPHASE2ALU };

	struct RegModOffset {
		uint MacroOffset;
		uint RegisterBase;
		uint OpParramsIndex;
	};

	struct MacroRegModify {
		TokenInst *		Macro;
		uint			MacroSize;
		RegModOffset *	RegMods;
		uint			RegModSize;

	};

	#define R_BASE  (sid_R0 - GL_REG_0_ATI)
	#define C_BASE  (sid_C0 - GL_CON_0_ATI)
	#define T_BASE  (sid_1T0 - GL_REG_0_ATI)

	// static library database for tokens and BNF rules
	static SymbolDef PS_1_4_SymbolTypeLib[];
	static TokenRule PS_1_x_RulePath[];
	static bool LibInitialized;

	// Static Macro database for ps.1.1 ps.1.2 ps.1.3 instructions

	static TokenInst texreg2ar[];
	static RegModOffset texreg2xx_RegMods[];
	static MacroRegModify texreg2ar_MacroMods;

	static TokenInst texreg2gb[];
	static MacroRegModify texreg2gb_MacroMods;

	static TokenInst texdp3[];
	static RegModOffset texdp3_RegMods[];
	static MacroRegModify texdp3_MacroMods;

	static TokenInst texdp3tex[];
	static RegModOffset texdp3tex_RegMods[];
	static MacroRegModify texdp3tex_MacroMods;

	static TokenInst texm3x2pad[];
	static RegModOffset texm3xxpad_RegMods[];
	static MacroRegModify texm3x2pad_MacroMods;

	static TokenInst texm3x2tex[];
	static RegModOffset texm3xxtex_RegMods[];
	static MacroRegModify texm3x2tex_MacroMods;

	static TokenInst texm3x3pad[];
	static MacroRegModify texm3x3pad_MacroMods;

	static TokenInst texm3x3tex[];
	static MacroRegModify texm3x3tex_MacroMods;

	static TokenInst texm3x3spec[];
	static RegModOffset texm3x3spec_RegMods[];
	static MacroRegModify texm3x3spec_MacroMods;

	static TokenInst texm3x3vspec[];
	static RegModOffset texm3x3vspec_RegMods[];
	static MacroRegModify texm3x3vspec_MacroMods;


	MachineInstContainer mPhase1TEX_mi; /// machine instructions for phase one texture section
	MachineInstContainer mPhase1ALU_mi; /// machine instructions for phase one ALU section
	MachineInstContainer mPhase2TEX_mi; /// machine instructions for phase two texture section
	MachineInstContainer mPhase2ALU_mi; /// machine instructions for phase two ALU section

	MachineInstContainer* mActivePhaseMachineInstructions;
	// vars used during pass 2
	MachineInstID mOpType;
	uint mOpInst;
	bool mDo_Alpha;
	PhaseType mInstructionPhase;
	int mArgCnt;
	int mConstantsPos;

	#define MAXOPPARRAMS 5 // max number of parrams bound to an instruction
	
	OpParram mOpParrams[MAXOPPARRAMS];

	/// keeps track of which registers are written to in each phase
	/// if a register is read from but has not been written to in phase 2
	/// then if it was written to in phase 1 perform a register pass function
	/// at the begining of phase2 so that the register has something worthwhile in it
	/// NB: check ALU and TEX section of phase 1 and phase 2
	/// there are 6 temp registers r0 to r5 to keep track off
	/// checks are performed in pass 2 when building machine instructions
	RegisterUsage Phase_RegisterUsage[6];

	bool mMacroOn; // if true then put all ALU instructions in phase 1

	uint mTexm3x3padCount; // keep track of how many texm3x3pad instructions are used so know which mask to use

	size_t mLastInstructionPos; // keep track of last phase 2 ALU instruction to check for R0 setting
	size_t mSecondLastInstructionPos;

	// keep track if phase marker found: determines which phase the ALU instructions go into
	bool mPhaseMarkerFound; 

#ifdef _DEBUG
	FILE* fp;
	// full compiler test with output results going to a text file
	void testCompile(char* testname, char* teststr, SymbolID* testresult,
		uint testresultsize, GLuint* MachinInstResults = NULL, uint MachinInstResultsSize = 0);
#endif // _DEBUG


	/** attempt to build a machine instruction using current tokens
		determines what phase machine insturction should be in and if an Alpha Op is required
		calls expandMachineInstruction() to expand the token into machine instructions
	*/
	bool BuildMachineInst();
	
	void clearMachineInstState();

	bool setOpParram(const SymbolDef* symboldef);

	/** optimizes machine instructions depending on pixel shader context
		only applies to ps.1.1 ps.1.2 and ps.1.3 since they use CISC instructions
		that must be transformed into RISC instructions
	*/
	void optimize();

	// the method is expected to be recursive to allow for inline expansion of instructions if required
	bool Pass2scan(const TokenInst * Tokens, const size_t size);

	// supply virtual functions for Compiler2Pass
	/// Pass 1 is completed so now take tokens generated and build machine instructions
	bool doPass2();

	/** Build a machine instruction from token and ready it for expansion
		will expand CISC tokens using macro database

	*/
	bool bindMachineInstInPassToFragmentShader(const MachineInstContainer & PassMachineInstructions);

	/** Expand CISC tokens into PS1_4 token equivalents

	*/
	bool expandMacro(const MacroRegModify & MacroMod);

	/** Expand Machine instruction into operation type and arguments and put into proper machine
		instruction container
		also expands scaler alpha machine instructions if required

	*/
	bool expandMachineInstruction();

	// mainly used by tests - too slow for use in binding
	size_t getMachineInst(size_t Idx);

	size_t getMachineInstCount();

	void addMachineInst(PhaseType phase, const uint inst);

	void clearAllMachineInst();

	void updateRegisterWriteState(const PhaseType phase);

	bool isRegisterReadValid(const PhaseType phase, const int param);

public:

	/// constructor
	PS_1_4();

	/// binds machine instructions generated in Pass 2 to the ATI GL fragment shader
	bool bindAllMachineInstToFragmentShader();

#ifdef _DEBUG
	/// perform compiler tests - only available in _DEBUG mode
	void test();
	void testbinder();

#endif
};


#endif

