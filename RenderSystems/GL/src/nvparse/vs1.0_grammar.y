%start VS10Program
%name-prefix "vs10_"
%{
void yyerror(const char *s);
int yylex(void);

#include <math.h>
#include <string>

#include <stdlib.h>
#include "vs1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

//extern bool gbTempInsideMacro;
//extern unsigned int &base_linenumber;
void LexError(const char *format, ...);
extern int line_incr;

#define do_linenum_incr()		{ line_number+=line_incr; line_incr = 0; }
//int get_linenum()			{ return( gbTempInsideMacro ? base_linenumber : line_number ); }
int get_linenum()			{ return( line_number ); }

#define YYDEBUG 1

%}
%union {
  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;
};

%token VERTEX_SHADER

%token ADD_INSTR
%token DP3_INSTR
%token DP4_INSTR
%token DST_INSTR
%token EXP_INSTR
%token EXPP_INSTR
%token FRC_INSTR
%token LIT_INSTR
%token LOG_INSTR
%token LOGP_INSTR
%token M3X2_INSTR
%token M3X3_INSTR
%token M3X4_INSTR
%token M4X3_INSTR
%token M4X4_INSTR
%token MAD_INSTR
%token MAX_INSTR
%token MIN_INSTR
%token MOV_INSTR
%token MUL_INSTR
%token NOP_INSTR
%token RCP_INSTR
%token RSQ_INSTR
%token SGE_INSTR
%token SLT_INSTR
%token SUB_INSTR

%token ILLEGAL 
%token UNKNOWN_STRING

%token <ival> INTVAL
%token <reg> REGISTER
%token <mask> XYZW_MODIFIER
%token <comment> COMMENT

%type <instList> InstSequence
%type <inst> InstLine
%type <inst> Instruction
%type <inst> VECTORopinstruction
%type <inst> SCALARopinstruction
%type <inst> UNARYopinstruction
%type <inst> BINopinstruction
%type <inst> TRIopinstruction
%type <ival> VECTORop
%type <ival> SCALARop
%type <ival> UNARYop
%type <ival> BINop
%type <ival> TRIop

%type <reg> genericReg
%type <reg> genReg
%type <reg> constantReg


%%      /*beginning of rules section */

VS10Program: InstSequence
	{
	$1->Validate();
	$1->Translate();
	delete $1;
	}
	;

InstSequence: InstSequence InstLine
	{
		*($1) += $2;
		delete $2;
		$$ = $1;
	}
	| InstLine
	{
 		VS10InstListPtr instList = new VS10InstList;
		if ( $1 != NULL )
			{
			*instList += $1;
			delete $1;
			}
		$$ = instList;
	}
	;

InstLine: Instruction
	{
		$$ = $1;
		do_linenum_incr();
	}
	| '\n'
	{
		$$ = new VS10Inst( get_linenum() );
		do_linenum_incr();
	}
	;

Instruction: VECTORopinstruction
           | SCALARopinstruction
           | UNARYopinstruction
           | BINopinstruction
           | TRIopinstruction
		   | NOP_INSTR
		   {
		   $$ = new VS10Inst( get_linenum(), VS10_NOP );
		   }
		   | COMMENT
		   {
		   $$ = new VS10Inst( get_linenum(), VS10_COMMENT, $1 );
		   }
		   | VERTEX_SHADER
		   {
		   $$ = new VS10Inst( get_linenum(), VS10_HEADER );
		   }
		   ;

VECTORopinstruction: VECTORop genericReg ',' genericReg
	{
		$$ = new VS10Inst( get_linenum(), $1, $2, $4 );
	}
	;

genericReg : '-' genReg '.' XYZW_MODIFIER
		   {
		   VS10Reg reg;
		   reg = $2;
		   reg.sign = -1;
		   reg.type = $2.type;
		   reg.index = $2.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = $4[i];
		   $$ = reg;
		   }
		   | genReg '.' XYZW_MODIFIER
		   {
		   VS10Reg reg;
		   reg = $1;
		   reg.sign = 1;
		   reg.type = $1.type;
		   reg.index = $1.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = $3[i];
		   $$ = reg;
		   }
		   | '-' genReg
		   {
		   VS10Reg reg;
		   reg = $2;
		   reg.sign = -1;
		   reg.type = $2.type;
		   reg.index = $2.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   $$ = reg;
		   }
		   | genReg
		   {
		   VS10Reg reg;
		   reg = $1;
		   reg.sign = 1;
		   reg.type = $1.type;
		   reg.index = $1.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   $$ = reg;
		   }
		   ;
genReg: REGISTER | constantReg
	  {
	  }
	  ;
constantReg: 'c' '[' INTVAL ']'
		   {
		   VS10Reg reg;
		   reg.type = TYPE_CONSTANT_MEM_REG;
		   reg.index = $3;
		   $$ = reg;
		   }
		   | 'c' '[' REGISTER '.' XYZW_MODIFIER ']'
		   {
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   $$.type = TYPE_CONSTANT_A0_REG;
		   if ( $3.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( $3.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( $5[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || $5[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_REG;
				   $$ = reg;
			   }
		   }
		   | 'c' '[' REGISTER '.' XYZW_MODIFIER '+' INTVAL ']'
		   {
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   $$.type = TYPE_CONSTANT_A0_OFFSET_REG;
		   if ( $3.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( $3.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( $5[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || $5[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
				   reg.index = $7;
				   $$ = reg;
			   }
		   }
		   | 'c' '[' REGISTER ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   | 'c' '[' REGISTER '+' INTVAL ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   | 'c' '[' '-' REGISTER  ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   | 'c' '[' '-' REGISTER '+' INTVAL ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   | 'c' '[' '-' REGISTER '.' XYZW_MODIFIER ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   | 'c' '[' '-' REGISTER '.' XYZW_MODIFIER '+' INTVAL ']'
		   {
		       $$.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   }
		   ;

SCALARopinstruction: SCALARop genericReg ',' genericReg
	{
		$$ = new VS10Inst( get_linenum(), $1, $2, $4 );
	}
	;

UNARYopinstruction: UNARYop genericReg ',' genericReg
	{
		$$ = new VS10Inst( get_linenum(), $1, $2, $4 );
	}
	;

BINopinstruction: BINop genericReg ',' genericReg ',' genericReg
	{
		$$ = new VS10Inst( get_linenum(), $1, $2, $4, $6 );
	}
	;

TRIopinstruction: TRIop genericReg ','
                        genericReg ',' genericReg ',' genericReg
	{
		$$ = new VS10Inst( get_linenum(), $1, $2, $4, $6, $8 );
	}
	;

VECTORop: MOV_INSTR
	{
		$$ = VS10_MOV;
	}
    | LIT_INSTR
	{
		$$ = VS10_LIT;
	}
	;

SCALARop: RCP_INSTR
	{
	$$ = VS10_RCP;
	}
    | RSQ_INSTR
	{
	$$ = VS10_RSQ;
	}
	| EXP_INSTR
	{
	$$ = VS10_EXP;
	}
	| EXPP_INSTR
	{
	$$ = VS10_EXPP;
	}
    | LOG_INSTR
	{
	$$ = VS10_LOG;
	}
    | LOGP_INSTR
	{
	$$ = VS10_LOGP;
	}
	;

UNARYop: FRC_INSTR
	{
	$$ = VS10_FRC;
	}
    	;
	
BINop: MUL_INSTR
	{
	$$ = VS10_MUL;
	}
    | ADD_INSTR
	{
	$$ = VS10_ADD;
	}
    | DP3_INSTR
	{
	$$ = VS10_DP3;
	}
    | DP4_INSTR
	{
	$$ = VS10_DP4;
	}
    | DST_INSTR
	{
	$$ = VS10_DST;
	}
    | MIN_INSTR
	{
	$$ = VS10_MIN;
	}
    | MAX_INSTR
	{
	$$ = VS10_MAX;
	}
    | SLT_INSTR
	{
	$$ = VS10_SLT;
	}
    | SGE_INSTR
	{
	$$ = VS10_SGE;
	}
    | M3X2_INSTR
	{
	$$ = VS10_M3X2;
	}
    | M3X3_INSTR
	{
	$$ = VS10_M3X3;
	}
    | M3X4_INSTR
	{
	$$ = VS10_M3X4;
	}
    | M4X3_INSTR
	{
	$$ = VS10_M4X3;
	}
    | M4X4_INSTR
	{
	$$ = VS10_M4X4;
	}
    | SUB_INSTR
	{
	$$ = VS10_SUB;
	}
	;

TRIop: MAD_INSTR
	{
	$$ = VS10_MAD;
	}
	;


%%
void yyerror(const char* s)
{
    LexError( "Syntax Error.\n" );
    //errors.set(s);
    //errors.set("unrecognized token");
}
