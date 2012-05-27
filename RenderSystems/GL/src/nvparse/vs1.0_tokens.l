%option prefix="vs10_"

%x INCLUDE DEFINE DEFSTR DEFSPACE SKIPLINE EATCOMMENT EATSTRING SAVELINE 
%x MACRONAME MACROBODY MACROPARM EATMACRO EATDEFINE MODIFIER MACROPARMSTART
%x IFDEFNAME IFDEFBODY ENDMACRO MACROPARMEND

%{

#include <stdarg.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#  ifdef __GNUC__
#    include <sys/types.h>
#    include <ctype.h>
#  endif
#else
#include <sys/types.h>
#include <ctype.h>
#define _stat stat
#define _open open
#define _O_RDONLY O_RDONLY
#define _fstat fstat
#define _close close
#define stricmp strcasecmp

#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "macro.h"
#include "nvparse_errors.h"
#include "vs1.0_inst_list.h"
#include "_vs1.0_parser.h"
#define yylineno line_number
#include "nvparse_externs.h"

#define yylineno line_number
int line_incr;
void LexError(const char *format, ...);
void LexWarning(const char *format, ...);
char *ReadTextFile(const char * filename);

unsigned int MakeRegisterMask(char *findName);
unsigned int FindSwizzleValue(char *swizzleText);


enum ERROR_VALUES {
	ERROR_NONE = 0,
	ERROR_MEMORY_ALLOC,
	ERROR_FILE_OPEN,
	ERROR_UNSUCCESSFUL_ASSEMBLE,
	ERROR_TOO_MANY_PARMS,
	ERROR_DEST_WRITE,
	ERROR_LIST_OPEN,
	ERROR_DEST_OPEN,
	ERROR_NO_ARGUMENTS,
	ERROR_MACRO_OVERRUN
};



//extern void GenSwitchFileNames(char *fileName);
//extern unsigned int gLinesAssembled;
unsigned int gLinesAssembled;

#define YY_INPUT(buf,result,max_size) \
{ \
	int c = *myin++; \
	result = (c == 0) ? YY_NULL : (buf[0] = c, 1); \
}

#define SAFEDELETEARRAY(x) if ((x) != NULL) \
						delete [] (x)
#define SAFEFREE(x) if ((x) != NULL) \
						free((x))

#define MAXREPLACESTRING 255

char gReplaceText[MAXREPLACESTRING+1];

//
// forward prototypes for macro functions
//
void MacroIncFunction(char *, unsigned int *, char **);
void MacroDecFunction(char *, unsigned int *, char **);
void MacroAddFunction(char *, unsigned int *, char **);
void MacroSubFunction(char *, unsigned int *, char **);

MACROFUNCTIONS gMacroFunctions[] = { 
	{ "inc(", MacroIncFunction },
	{ "dec(", MacroDecFunction },
	{ "add(", MacroAddFunction },
	{ "sub(", MacroSubFunction }
};

#define NUM_MACRO_FUNCTIONS (sizeof(gMacroFunctions) / sizeof(MACROFUNCTIONS))

#define MAX_INCLUDE_DEPTH 1024
typedef struct INCLUDEINFO
{
	char	*fileName;
	unsigned int lineNo;
	YY_BUFFER_STATE buffer;
	MACROENTRY *lastInvokeMacro;				// save off in case nested macros.
	MACROENTRY *lastParseMacro;				// recursive macros
	MACROTEXT *lastMacroLineParse;			// save off for recursive lines of macros working on.
	bool lastbInsideMacro;					// save off for recursive macros
	bool lastbInsideDefine;					// save off for recursive macros/defines
	bool lastbInsideInclude;
	bool lastbProcessingIFDEF; 				// save off #define information
//	FILE *fileHandle;
	char *prevString;
	char *nextString;
} INCLUDEINFO;

INCLUDEINFO gIncludeStack[MAX_INCLUDE_DEPTH];
int gIncludeStackIndex = 0;

IFDEFINFO gIfDefStack[MAX_IFDEF_DEPTH];
int gIfDefStackIndex = 0;

unsigned int &base_linenumber = gIncludeStack[0].lineNo;

bool gbInsideInclude = false;
bool gbProcessingBuiltIn = false;
bool gbProcessingDefine = false;
unsigned int gCountParen = 0;

bool gbProcessingIFDEF = false;
bool gbIFDEF = false;
bool gbCompareDefine = false;
unsigned int gIfDefStartLine;


MACROENTRY *gLastMacro;
MACROENTRY *gInvokeMacro;
MACROENTRY *gTempMacro;					// until all the parameters are read
MACROENTRY *FindMacro(char *macroName);
MACROENTRY *FindNMacro(char *macroName, unsigned int sLen);

MACROFUNCTIONPTR gMacroCallFunction;

const char *builtInMacros =	"macro m3x2 reg1, reg2, reg3\n"
						"	dp3	%reg1.x, %reg2, %reg3\n"
						"	dp3 %reg1.y, %reg2, %inc(%reg3)\n"
						"endm";

//
// local prototypes
//
void CleanUp();
void ReplaceMacroParms(char *srcLine, char *destLine, 
							MACROENTRY *srcParms, MACROENTRY *invParms);

MACROTEXT *SaveMacroText(char *srcText, MACROTEXT *lastMacroText);
void FreeMacroEntry(MACROENTRY *macEntry);
void EndMacroParms();
char *FindAlphaNum(char *srcStr, unsigned int *sLen);
void DebugUnhandledState();


unsigned int gCommentStartLine;
unsigned int gMacroStartLine;

char *gCurFileName = NULL;

#define MAXSAVELINE 4095

char gSaveLine[MAXSAVELINE+1];
char gMacroLine[MAXSAVELINE+1];

#if 1
#ifdef _DEBUG
#define ECHO DebugUnhandledState();
#else
#define ECHO
#endif
#endif

bool gbInsideMacro = false;		// flag if we are doing a macro replace or not.
bool gbTempInsideMacro = false;
unsigned int gInvokeState = INITIAL;


MACROENTRY *gParseMacro;		// which source macro entry we are using
MACROENTRY *gTempParseMacro;	// temporary holder until parameters are received.
MACROTEXT *gMacroLineParse;		// which line we are currently parsing inside the macro invocation

enum OPCODETYPE
{
	TYPE_NONE = 0,
	TYPE_VERTEX_SHADER = 1,
	TYPE_PIXEL_SHADER = 2
};
typedef struct OPCODEMAP
{
	const char *string;				// string for opcode
	int tokenName;              // name of the corresponding token
	int numArguments;			// number of arguments for opcode
	float version;				// minimum version supported in.
	int opcodeTypeFlags;		// whether opcode can be used in vertex shader or pixel shader
	bool opcodeModify;			// if opcode modifiers can be used
	bool textureOpcode;			// only outputs to the texture unit
} OPCODEMAP;

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

OPCODEMAP theOpcodes[] = {
	{ "add",  ADD_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "dp3",  DP3_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "dp4",  DP4_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "dst",  DST_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "exp",  EXP_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "expp", EXPP_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "frc",  FRC_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "lit",  LIT_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "log",  LOG_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "logp", LOGP_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "m3x2", M3X2_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "m3x3", M3X3_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "m3x4", M3X4_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "m4x3", M4X3_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "m4x4", M4X4_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "mad",  MAD_INSTR, 4, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "max",  MAX_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "min",  MIN_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "mov",  MOV_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "mul",  MUL_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "nop",  NOP_INSTR, 0, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
	{ "rcp",  RCP_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "rsq",  RSQ_INSTR, 2, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "sge",  SGE_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "slt",  SLT_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER, FALSE, FALSE },
	{ "sub",  SUB_INSTR, 3, 1.0f, TYPE_VERTEX_SHADER | TYPE_PIXEL_SHADER, TRUE, FALSE },
};

#define NUMOPCODES (sizeof(theOpcodes) / sizeof(OPCODEMAP))
OPCODEMAP *FindOpcode(char *findName);


%}

digits	([0-9]+)
digit	([0-9])
pt	"."
sign		[+-]?
exponent	([eE]{sign}{digits})
alpha [a-zA-Z_]
alphadigs [a-zA-Z0-9_]
notAlphaDigs ([^a-zA-Z0-9_])

identifier		{alpha}{alphadigs}*


%%

<SAVELINE>.*\n {
	gbProcessingDefine = false;
	gSaveLine[0] = '\0';
	strncat(gSaveLine, yytext, MAXSAVELINE);
//	GenDebugLine();
	if (gbProcessingIFDEF && (gbCompareDefine != gbIFDEF))
	{
		BEGIN(IFDEFBODY);
	}
	else
	{
		BEGIN(INITIAL);
	}
	yyless(0);
}

<SAVELINE>.* {
	gbProcessingDefine = false;
	gSaveLine[0] = '\0';
	strncat(gSaveLine, yytext, MAXSAVELINE);
//	GenDebugLine();
	if (gbProcessingIFDEF && (gbCompareDefine != gbIFDEF))
	{
		BEGIN(IFDEFBODY);
	}
	else
	{
		BEGIN(INITIAL);
	}
	yyless(0);
}

a{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_ADDRESS_REG;
	vs10_lval.reg.index = atoi(&yytext[1]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

v{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_VERTEX_ATTRIB_REG;
	vs10_lval.reg.index = atoi(&yytext[1]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

r{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_TEMPORARY_REG;
	vs10_lval.reg.index = atoi(&yytext[1]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

c{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_CONSTANT_MEM_REG;
	vs10_lval.reg.index = atoi(&yytext[1]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

oT{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_TEXTURE_RESULT_REG;
	vs10_lval.reg.index = atoi(&yytext[2]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

oD{digits}[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_COLOR_RESULT_REG;
	vs10_lval.reg.index = atoi(&yytext[2]);
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

oFog[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_FOG_RESULT_REG;
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

oPos[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_POSITION_RESULT_REG;
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

oPts[ \t]*[\n]?	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.reg.type = TYPE_POINTS_RESULT_REG;
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return REGISTER;
}

[a-zA-Z][a-zA-Z0-9]+[ \t\n_] {

	unsigned int offset;
	
    offset = strcspn(yytext, " \t\n_");
	yyless(offset);

	OPCODEMAP *opcodeMap = FindOpcode(yytext);
	if ( opcodeMap != NULL )
	{
//		fprintf( stderr, "%s\t", opcodeMap->string );
		return( opcodeMap->tokenName );
	}
	else
	{
		gTempParseMacro = FindMacro(yytext);

		if (gTempParseMacro != NULL)
		{
			if (gIncludeStackIndex >= MAX_INCLUDE_DEPTH )
			{
				LexError("macros nested too deeply");
				exit( 1 );
			}

			if (gTempParseMacro->firstMacroLines != NULL)
			{

				gTempMacro = (MACROENTRY *)malloc(sizeof(MACROENTRY));
				if (gTempMacro == NULL)
				{
					LexError("Out of memory allocating MACROENTRY structure.\n");
				}
				else
				{

					gTempMacro->next = NULL;
					gTempMacro->prev = NULL;
					gTempMacro->macroName = NULL;
					gTempMacro->firstMacroParms = NULL;
					gTempMacro->lastMacroParms = NULL;
					gTempMacro->firstMacroLines = NULL;
					gTempMacro->lastMacroLines = NULL;
					gTempMacro->numParms = 0;
					gTempMacro->nLines = 0;

					gbTempInsideMacro = true;		// flag we are currently doing a macro replace.
					gInvokeState = YYSTATE;
					if (gTempParseMacro->numParms > 0)
					{
						BEGIN(MACROPARMSTART);
					}
					else
					{
						EndMacroParms();
						gbTempInsideMacro = false;	// no longer waiting for macro invocation
					}

					
				}
			}
		}
		else
		{
//			fprintf( stderr, "Opcode: \"%s\" not found\n", yytext );
			REJECT;
		}
	}

	//unsigned int offset;
	//
	//INSTRMAP *opcodeMap;
	//
	//offset = strcspn(yytext, " \t\n_");
	//yyless(offset);	
	//opcodeMap = FindInstruction(yytext);
	//if (opcodeMap == NULL)
	//{
	//	REJECT;
	//}
	//
	//yylval.opcodeInfo.opcodeMap = opcodeMap;
	//
	//return OPCODE;
}


";".*  {
//	fprintf( stderr, "%s", yytext );
	char *cmt = new char[yyleng+1];
	strncpy( cmt, yytext, yyleng );
	cmt[0] = '#';
	cmt[yyleng] = '\0';
	vs10_lval.comment = cmt;
	return COMMENT;
}

"//".* {
//	fprintf( stderr, "%s", yytext );
	char *cmt = new char[yyleng+1];
	strncpy( cmt+1, yytext+1, yyleng-1 );
	cmt[0] = '#';
	cmt[1] = ' ';
	cmt[yyleng] = '\0';
	vs10_lval.comment = cmt;
	return COMMENT;
}

"+"[ \t]*\n {
	fprintf( stderr, "COISSUE found\n" );
	yyless(yyleng-1);
	//return COISSUE;
}

^[ \t]*"+"[ \t]* {
	fprintf( stderr, "COISSUE found\n" );
	//return COISSUE;
}

<INITIAL,MACROBODY>"/*" {
	gCommentStartLine = yylineno;
	yyless(0);
	BEGIN(EATCOMMENT);
}

"#include"[ \t]+ { 
	BEGIN(INCLUDE);
}

<INCLUDE>[^ \t].*\n {	/* got the include file name */

//	FILE *newyyin;
char *newyyin;
	char incFileName[1024];
	unsigned long sLen;
	bool validFileName;

	if ( gIncludeStackIndex >= MAX_INCLUDE_DEPTH )
	{
		LexError("Includes nested too deeply, aborting\n");
		exit( 1 );
	}

//	GenDebugLine();
//	GenListString();
	yylineno++;
	gLinesAssembled++;

	validFileName = true;
	// zap "" and <>
	if ((yytext[0] == '"') || (yytext[0] == '<'))
	{
		char *endQuote;
		endQuote = strchr(&yytext[1], yytext[0]);
		sLen = (endQuote - yytext)-1;
		if (endQuote == NULL)
		{
			LexError("Unable to open include file %s\n", incFileName);
			BEGIN(INITIAL);
			validFileName = false;
		}
		else
		{
			incFileName[0] ='\0';
			strncat(incFileName, &yytext[1], sLen);
		}
	}
	else
	{
		strcpy(incFileName, yytext);
	}

	if (validFileName)
	{
		sLen = strlen(incFileName);
		if ((incFileName[sLen-1] == '"') || (incFileName[sLen-1] == '>'))
		{
			incFileName[sLen-1] = '\0';
		}


		newyyin = ReadTextFile( incFileName );
//		newyyin = fopen( incFileName, "r" );

		if ( ! newyyin )
		{
			LexError("Unable to open include file %s\n", incFileName);
			BEGIN(SAVELINE);
		}
		else
		{
			gIncludeStack[gIncludeStackIndex].fileName = gCurFileName;
			gIncludeStack[gIncludeStackIndex].lineNo = yylineno;
//			gIncludeStack[gIncludeStackIndex].fileHandle = yyin;
			gIncludeStack[gIncludeStackIndex].prevString = myin;
			gIncludeStack[gIncludeStackIndex].nextString = newyyin;
			gIncludeStack[gIncludeStackIndex].lastInvokeMacro = gInvokeMacro;
			gIncludeStack[gIncludeStackIndex].lastParseMacro = gParseMacro;
			gIncludeStack[gIncludeStackIndex].lastMacroLineParse = gMacroLineParse;
			gIncludeStack[gIncludeStackIndex].lastbInsideMacro = gbInsideMacro;
			gIncludeStack[gIncludeStackIndex].lastbInsideInclude = gbInsideInclude;
			gIncludeStack[gIncludeStackIndex].buffer = YY_CURRENT_BUFFER;
			gIncludeStack[gIncludeStackIndex].lastbProcessingIFDEF = gbProcessingIFDEF;
			gIncludeStackIndex++;

			gbProcessingIFDEF = false;

			gCurFileName = strdup(incFileName);
//			yyin = newyyin;
			myin = newyyin;

//			GenSwitchFileNames(gCurFileName);

			yylineno = 1;

			yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );

			gbInsideInclude = true;

			BEGIN(SAVELINE);
		}
	}
}

<EATCOMMENT><<EOF>> {
	LexError("End of file reached before end of comment started on line %d.\n", gCommentStartLine);
	BEGIN(INITIAL);
}

<EATCOMMENT>.*\n {
	char *endComment;
	unsigned int keepSize;

	strcpy(gSaveLine, yytext);
	endComment = strstr(yytext, "*/");

	char *cmt;
	if (endComment != NULL)
	{
		keepSize = (endComment - yytext+2);
		yyless(keepSize);
		BEGIN(INITIAL);

		if ( yytext[0] == '/' && yytext[1] == '*' )
		{
			cmt = new char[yyleng];
			strncpy( cmt+3, yytext+2, yyleng-2 );
			cmt[0] = '#';
			cmt[1] = ' ';
			cmt[2] = ' ';
			cmt[yyleng-1] = '\0';
		}
		else
		{
			cmt = new char[yyleng];
			strncpy( cmt+1, yytext, yyleng-2 );
			cmt[0] = '#';
			cmt[yyleng-1] = '\0';
		}
		vs10_lval.comment = cmt;
		return COMMENT;
	}
	else
	{
//		GenDebugLine();
//		GenListString();
		gLinesAssembled++;
		yylineno++;

		if ( yytext[0] == '/' && yytext[1] == '*' )
		{
			cmt = new char[yyleng+2];
			strncpy( cmt+3, yytext+2, yyleng-2 );
			cmt[0] = '#';
			cmt[1] = ' ';
			cmt[2] = ' ';
			cmt[yyleng+1] = '\0';
		}
		else
		{
			cmt = new char[yyleng+2];
			strncpy( cmt+1, yytext, yyleng );
			cmt[0] = '#';
			cmt[yyleng+1] = '\0';
		}
		vs10_lval.comment = cmt;
		return COMMENT;
	}
}

<DEFSTR><<EOF>> {
	LexError("#define was incomplete before end of file\n");
	BEGIN(INITIAL);
}

<DEFINE><<EOF>> {
	LexError("#define was incomplete before end of file\n");
	BEGIN(INITIAL);
}

<DEFSPACE><<EOF>> {
	LexError("#define was incomplete before end of file\n");
	BEGIN(INITIAL);
}

<INCLUDE><<EOF>> {
	LexError("#include was incomplete before end of file\n");
	BEGIN(INITIAL);
}

<MACROBODY><<EOF>> {
	LexError("End of file reached before end of #define or endm was found, macro started on line %d.\n", gMacroStartLine);
	BEGIN(INITIAL);
}

<IFDEFBODY><<EOF>> {
	LexError("End of file reached before #endif found, macro started on line %d.\n", gIfDefStartLine);
	BEGIN(INITIAL);
}

<DEFSTR>\n {
	LexError("#define was incomplete before end of line\n");
	BEGIN(SAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<DEFINE>\n {
	LexError("#define was incomplete before end of line\n");
	BEGIN(SAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<DEFSPACE>\n {
	LexError("#define was incomplete before end of line\n");
	BEGIN(SAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

"#ifdef"[ \t]+ {
	if (gIfDefStackIndex >= MAX_IFDEF_DEPTH)
	{
		LexError("Out of stack space for #ifdef, aborting.\n");
		exit( 1 );
	}
	else
	{
		gIfDefStack[gIfDefStackIndex].lastbProcessingIFDEF = gbProcessingIFDEF;
		gIfDefStack[gIfDefStackIndex].lastbIFDEF = gbIFDEF;
		gIfDefStack[gIfDefStackIndex].lastbCompareDefine = gbCompareDefine;
		gIfDefStack[gIfDefStackIndex].lastIfDefStartLine = gIfDefStartLine;
		gIfDefStackIndex++;
		gIfDefStartLine = yylineno;

		gbCompareDefine = true;
		BEGIN(IFDEFNAME);
	}
}

"#ifndef"[ \t]+ {
	if (gIfDefStackIndex >= MAX_IFDEF_DEPTH)
	{
		LexError("Out of stack space for #ifdef, aborting.\n");
		exit( 1 );
	}
	else
	{
		gIfDefStack[gIfDefStackIndex].lastbProcessingIFDEF = gbProcessingIFDEF;
		gIfDefStack[gIfDefStackIndex].lastbIFDEF = gbIFDEF;
		gIfDefStack[gIfDefStackIndex].lastbCompareDefine = gbCompareDefine;
		gIfDefStack[gIfDefStackIndex].lastIfDefStartLine = gIfDefStartLine;
		gIfDefStackIndex++;
		gIfDefStartLine = yylineno;

		gbCompareDefine = false;
		BEGIN(IFDEFNAME);
	}
}

<INITIAL,IFDEFBODY>"#else"[ \t]*((";"|"//").*)* {
	if (!gbProcessingIFDEF)
	{
		LexError("Unexpected #else found at line %d, skipping.\n", yylineno);
	}
	else
	{
		gbCompareDefine = !gbCompareDefine;
		BEGIN(INITIAL);
	}
}

<IFDEFNAME>.*\n {
	char *defineName;
	unsigned int sLen;


	defineName = FindAlphaNum(yytext, &sLen);
	if (defineName == NULL)
	{
		defineName = strdup(yytext);
		defineName[yyleng-1] = '\0';	// kill \n
		LexWarning("Mangled name (%s) for #ifdef, assuming not defined.\n", defineName);
		free(defineName);
		gbIFDEF = false;
	}
	else
	{
		if (FindNMacro(defineName, sLen) != NULL)
		{
			gbIFDEF = true;
		}
		else
		{
			gbIFDEF = false;
		}
	}

	gbProcessingIFDEF = true;
	if (gbIFDEF != gbCompareDefine)
	{
		BEGIN(IFDEFBODY);
	}
	else
	{
		BEGIN(SAVELINE);
	}

//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<INITIAL,IFDEFBODY>[ \t]*"#endif"[ \t]*((";"|"//").*)* {
	if (!gbProcessingIFDEF)
	{
		LexError("Unexpected #endif found at line %d, skipping.\n", yylineno);
	}
	else
	{
		gIfDefStackIndex--;
		gbProcessingIFDEF = gIfDefStack[gIfDefStackIndex].lastbProcessingIFDEF;
		gbIFDEF = gIfDefStack[gIfDefStackIndex].lastbIFDEF;
		gbCompareDefine = gIfDefStack[gIfDefStackIndex].lastbCompareDefine;
		gIfDefStartLine = gIfDefStack[gIfDefStackIndex].lastIfDefStartLine;

	}

	if (YYSTATE == IFDEFBODY)
	{
		strncpy(gSaveLine, yytext, MAXSAVELINE);
	}

	BEGIN(ENDMACRO);

}

<ENDMACRO>.* {
	LexWarning("Garbage at end of #endif or endm will be ignored.\n");
}

<ENDMACRO>\n {
	BEGIN(SAVELINE);
	return '\n';
}

<ENDMACRO><<EOF>> {
	BEGIN(INITIAL);
}

<IFDEFBODY>.* {
	// eat line, because we are not in a TRUE #ifdef, or FALSE #ifndef
	strncpy(gSaveLine, yytext, MAXSAVELINE);
}

<IFDEFBODY>\n {
	strcat(gSaveLine, yytext);
//	GenDebugLine();
//	GenListString();
	yylineno++;
	gLinesAssembled++;
}

"#define"[ \t]+   { 
	gbProcessingDefine = true;
	gMacroStartLine = yylineno;
	gCountParen = 0;
	BEGIN(MACRONAME); 
} 

<SKIPLINE>.*\n	{
	BEGIN(SAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

"vs."{digits}"."{digits}[ \t]*[\n]? {
//	unsigned int majorVersion;
//	unsigned int minorVersion;
//	int minorOffset;
//	
//
//	majorVersion = (unsigned int)(atoi(&yytext[3]));
//	// skip "ps." + second '.'
//	minorOffset = strcspn(&yytext[3], ".")+4;
//	minorVersion = (unsigned int)(atoi(&yytext[minorOffset]));
//	yylval.ival = D3DVS_VERSION(majorVersion, minorVersion);
//

//	fprintf( stderr, "%s", yytext );
	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;
	return VERTEX_SHADER;
}

{digits}	{ 
//	fprintf( stderr, "%s", yytext );
	vs10_lval.ival = atoi(yytext);
	return INTVAL;
}


{pt} { 
		BEGIN(MODIFIER);
//fprintf( stderr, "." );
		return yytext[0];
}

<MODIFIER>[w-z][w-z][w-z][w-z][ \t]*[\n]? {
//	fprintf( stderr, "%s", yytext );
	BEGIN(INITIAL);

	vs10_lval.mask[0] = tolower(yytext[0]);
	vs10_lval.mask[1] = tolower(yytext[1]);
	vs10_lval.mask[2] = tolower(yytext[2]);
	vs10_lval.mask[3] = tolower(yytext[3]);

	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;

	return XYZW_MODIFIER;

#if 0
 	char temp[6];

	temp[0] = '\0';
	strncat(temp, yytext, 4);
	strlwr(temp);
	vs10_lval.lval = FindSwizzleValue(temp);

	BEGIN(INITIAL);
	return SWIZZLE_MODIFIER;
#endif

}

<MODIFIER>[w-z][w-z]?[w-z]?[w-z]?[ \t]*[\n]? {
//	fprintf( stderr, "%s", yytext );
	BEGIN(INITIAL);

	int validLen = strspn(yytext, "xyzw");
        int i;
	for ( i = 0; i < validLen; i++ )
		vs10_lval.mask[i] = tolower( yytext[i] );
	while ( i < 4 )
	{
		vs10_lval.mask[i] = 0;
		i++;
	}

	if ( yytext[yyleng-1] == '\n' )
		line_incr = 1;

	return XYZW_MODIFIER;

#if 0
 	//char temp[6];
 	char *temp = new char[6];
	unsigned int registerMask;
	unsigned int validLen;

	temp[0] = '\0';
	validLen = strspn(yytext, "xyzw");
	strncat(temp, yytext,  validLen);
	for ( int i = 0; i < validLen; i++ )
		temp[i] = tolower( temp[i] );
	registerMask = MakeRegisterMask(temp);

	if (registerMask != 0)
	{
		//vs10_lval.sval = temp;
		vs10_lval.lval = registerMask;
		BEGIN(INITIAL);
		return XYZW_MODIFIER;
	}
	else
	{
		//vs10_lval.sval = temp;
		vs10_lval.lval = FindSwizzleValue(temp);
		BEGIN(INITIAL);
		return SWIZZLE_MODIFIER;	
	}
#endif
}

<MODIFIER>. {
		BEGIN(INITIAL);
		yyless(0);
}

<MACRONAME>[^ \t(]+ {
	/* setup and save off #define/macro name */
	if (FindMacro(yytext) != NULL)
	{
		LexWarning("Redefinition of #define/macro %s, ignoring.\n", yytext);
		if (gbProcessingDefine)
		{
			BEGIN(EATDEFINE);
		}
		else
		{
			BEGIN(EATMACRO);
		}
	}
	else
	{

		BEGIN(MACROPARMSTART);
		// %%%%% This should be setup to use memory pools
		gTempMacro = (MACROENTRY *)malloc(sizeof(MACROENTRY));
		if (gTempMacro == NULL)
		{
			LexError("Out of memory for macro table.\n");
			if (gbProcessingDefine)
			{
				BEGIN(EATDEFINE);
			}
			else
			{
				BEGIN(EATMACRO);
			}
		}
		else
		{
			gTempMacro->prev = gLastMacro;
			gTempMacro->next = NULL;

			gTempMacro->firstMacroParms = NULL;
			gTempMacro->lastMacroParms = NULL;
			gTempMacro->firstMacroLines = NULL;
			gTempMacro->lastMacroLines = NULL;
			gTempMacro->numParms = 0;
			gTempMacro->bIsDefine = gbProcessingDefine;
			gTempMacro->nLines = 0;

			if (gCurFileName != NULL)
			{
				gTempMacro->fileName = strdup(gCurFileName);
			}
			else
			{
				gTempMacro->fileName = NULL;
			}

			gTempMacro->lineNo = yylineno;

			/* %%%%% this should be set up in memory pools. */
			gTempMacro->macroName = (char *)malloc(strlen(yytext)+1);
			if (gTempMacro->macroName == NULL)
			{
				LexError("Out of memory for string table.\n");
				SAFEFREE(gTempMacro);
				if (gbProcessingDefine)
				{
					BEGIN(EATDEFINE);
				}
				else
				{
					BEGIN(EATMACRO);
				}
			}
			else
			{
				strcpy(gTempMacro->macroName, yytext);
			}
		}
	}
}

<MACRONAME>\n {
	LexError("No macro name specified, skipping macro definition.\n");
	SAFEFREE(gTempMacro->fileName);
	SAFEFREE(gTempMacro);
	if (gbProcessingDefine)
	{
		BEGIN(EATDEFINE);
	}
	else
	{
		BEGIN(EATMACRO);
	}

//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<MACROPARMSTART>"(" {
	gCountParen++; 
}

<MACROPARMSTART>[ \t]+ {}

<MACROPARMSTART>. {
	if (gbProcessingDefine && (gCountParen == 0))
	{
		EndMacroParms();
	}
	else
	{
		BEGIN(MACROPARM);
	}
	yyless(0);
}

<MACROPARM>[ \t]* { 
	if ((gCountParen == 0) && gbProcessingDefine)
	{
		EndMacroParms();
	}
}

<MACROPARM>(";"|"//").* {
	if (gCountParen == 0)
	{
		EndMacroParms();
	}	
}

<MACROPARM>"," {}

<MACROPARM><<EOF>> {
	EndMacroParms();
//	GenDebugLine();
//	GenListString();
	yylineno++;
	gLinesAssembled++;
	BEGIN(INITIAL);
}

<MACROPARM>\n {
	if (gbProcessingDefine && (gCountParen > 0))
	{
		LexError("Malformed #define, skipping.\n");
		BEGIN(SAVELINE);
	}
	else
	{
		EndMacroParms();
//		GenDebugLine();
//		GenListString();
		yylineno++;
		gLinesAssembled++;
		if (gbProcessingDefine)
		{
			gbProcessingDefine = false;
			BEGIN(SAVELINE);
		}
	}
}


<MACROPARM>[^\n,]+ { 

	MACROTEXT *tMacro;
	char *macroParmEnd;
	unsigned int startOffset;
	unsigned int leftParenCount;
	unsigned int rightParenCount;

	// sheesh, we gotta count the parenthesis....
	macroParmEnd = yytext;
	leftParenCount = 0;
	rightParenCount = 0;
	while (*macroParmEnd)
	{
		if (*macroParmEnd == ')')
		{
			rightParenCount++;
		}
		if (*macroParmEnd == '(')
		{
			leftParenCount++;
		}

		macroParmEnd++;
	}

	// if we found the last right parenthesis.
	if (rightParenCount == leftParenCount+1)
	{
		// find if we got the last parenthesis on this line
		macroParmEnd = strrchr(yytext, ')');
		yyless((macroParmEnd - yytext));
		BEGIN(MACROPARMEND);
	}

	startOffset = strspn(yytext, " \t");

	tMacro = SaveMacroText(&yytext[startOffset], gTempMacro->lastMacroParms);
	if (tMacro == NULL)
	{
		LexError("Out of memory for string table for macro parameter(s).\n");
		FreeMacroEntry(gTempMacro);
		BEGIN(EATMACRO);
	}
	else
	{
		// if first one wasn't set then set it
		if (gTempMacro->firstMacroParms == NULL)
		{
			gTempMacro->firstMacroParms = tMacro;
		}

		gTempMacro->lastMacroParms = tMacro;

		gTempMacro->numParms++;
	}

}

<MACROPARMEND>")"[ \t]*"\\"*\n* {
	if (!gbProcessingDefine && !gbTempInsideMacro)
	{
		LexError("Malformed  macro, skipping.\n");
		BEGIN(EATMACRO);
	}
	else
	{
		gCountParen--;

		// we can get multiple \n's here
		while (yytext[yyleng-2] == '\n')
		{
			yyleng--;
		}
		yyless(yyleng);

		// if there isn't a \n on this line, macro starts on this line,
		// not next, like in a macro definition
		if (yytext[yyleng-1] != '\n')
		{
			EndMacroParms();
		}
		else
		{
			if (yytext[yyleng-1] == '\n')
			{
				gTempMacro->lineNo++;
			}
			// count this line
			gTempMacro->nLines++;
//			GenDebugLine();
//			GenListString();
			EndMacroParms();
			if (!gbInsideMacro)
			{
				yylineno++;
			}

			gLinesAssembled++;
		}

	}
}

<MACROPARMEND>")"[ \t]*(";"|"//").*\n {
	if (!gbProcessingDefine && !gbTempInsideMacro)
	{
		LexError("Malformed  macro, skipping.\n");
		BEGIN(EATMACRO);
	}
	else
	{

		// no matter what count this line
		gTempMacro->nLines++;
		gCountParen--;
		EndMacroParms();
		if (!gbInsideMacro)
		{
			yylineno++;
		}

		gLinesAssembled++;
	}
}

<MACROPARMEND>")"[ \t]+ {
	if (!gbProcessingDefine && !gbTempInsideMacro)
	{
		LexError("Malformed  macro, skipping.\n");
		BEGIN(EATMACRO);
	}
	else
	{
		gCountParen--;
		if (gCountParen == 0)
		{
			// no matter what count this line
			gTempMacro->nLines++;
			EndMacroParms();
			if (!gbInsideMacro)
			{
				yylineno++;
			}

			gLinesAssembled++;
		}
		else
		{
			REJECT;
		}
	}
}

<MACROBODY>.*"\\"[ \t]*\n {
	MACROTEXT *tMacro;
	unsigned int copyLen;
	char *endLine;

	gSaveLine[0] ='\0';
	endLine = strchr(yytext, '\\');
	copyLen = (endLine - yytext);
	if (copyLen > MAXSAVELINE)
	{
		copyLen = MAXSAVELINE;
	}

	strncat(gSaveLine, yytext, copyLen);
	strcat(gSaveLine, "\n");
	tMacro = SaveMacroText(gSaveLine, gLastMacro->lastMacroLines);
	if (tMacro == NULL)
	{
		LexError("Out of memory for string table for macro parameter(s).\n");
		BEGIN(EATDEFINE);
	}
	else
	{
		gLastMacro->nLines++;
		// if first one wasn't set then set it
		if (gLastMacro->firstMacroLines == NULL)
		{
			gLastMacro->firstMacroLines = tMacro;
		}

		gLastMacro->lastMacroLines = tMacro;
	}

//	GenDebugLine();
//	GenListString();
	yylineno++;
	gLinesAssembled++;
}

<MACROBODY>[ \t]*"endm"[ \t]*((";"|"//").*)* {

	strncpy(gSaveLine, yytext, MAXSAVELINE);
	if (gbProcessingDefine)
	{
		LexError("Malformed #define, skipping.\n");
	}

	BEGIN(ENDMACRO);
}

<MACROBODY>[^\n]* {
		MACROTEXT *tMacro;

		// check if processing #define and only one line, if not then append \n
		if (!gbProcessingDefine || (gLastMacro->nLines >= 1))
		{
			gSaveLine[0] = '\0';
			strncat(gSaveLine, yytext, MAXSAVELINE);
			strcat(gSaveLine, "\n");
			tMacro = SaveMacroText(gSaveLine, gLastMacro->lastMacroLines);
			gLastMacro->nLines++;
		}
		else if (gLastMacro->numParms > 0)	// check if parameters were there
		{
			// if so, we need the '\n' appended
			gMacroLine[0] = '\0';
			strncat(gMacroLine, yytext, MAXSAVELINE);
			strcat(gMacroLine, "\n");
			tMacro = SaveMacroText(gMacroLine, gLastMacro->lastMacroLines);
			gLastMacro->nLines++;
		}
		else	// straight no newline macro replace
		{
			tMacro = SaveMacroText(yytext, gLastMacro->lastMacroLines);
		}

		if (tMacro == NULL)
		{
			LexError("Out of memory for string table for macro parameter(s).\n");
			BEGIN(EATMACRO);
		}
		else
		{
			// if first one wasn't set then set it
			if (gLastMacro->firstMacroLines == NULL)
			{
				gLastMacro->firstMacroLines = tMacro;
			}

			gLastMacro->lastMacroLines = tMacro;
		}
}

<MACROBODY>\n {

	MACROTEXT *tMacro;
//	GenDebugLine();
//	GenListString();
	yylineno++;
	gLinesAssembled++;
	if (gbProcessingDefine)
	{
		gbProcessingDefine = false;
		BEGIN(SAVELINE);
	}
	else
	{
		// this means \n by itself inside macro body
		if (((yylineno-1) - gLastMacro->lineNo) !=  gLastMacro->nLines)
		{
			strcpy(gMacroLine, "\n");
			tMacro = SaveMacroText(gMacroLine, gLastMacro->lastMacroLines);
			gLastMacro->nLines++;

			if (tMacro == NULL)
			{
				LexError("Out of memory for string table for macro parameter(s).\n");
				BEGIN(EATMACRO);
			}
			else
			{
				// if first one wasn't set then set it
				if (gLastMacro->firstMacroLines == NULL)
				{
					gLastMacro->firstMacroLines = tMacro;
				}

				gLastMacro->lastMacroLines = tMacro;
			}
		}
	}
}

<EATMACRO>[ \t]*"endm"[ \t]*\n {
	BEGIN(SAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<EATMACRO>.*\n {
	strncpy(gSaveLine, yytext, MAXSAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<EATDEFINE>.*"\\"\n {
	strncpy(gSaveLine, yytext, MAXSAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
}

<EATDEFINE>.*\n {
	strncpy(gSaveLine, yytext, MAXSAVELINE);
//	GenDebugLine();
//	GenListString();
	gLinesAssembled++;
	yylineno++;
	BEGIN(SAVELINE);
}

<INITIAL,MODIFIER>{alpha}{alphadigs}* {

	gTempParseMacro = FindMacro(yytext);

	if (gTempParseMacro != NULL)
	{
		if (gIncludeStackIndex >= MAX_INCLUDE_DEPTH )
		{
			LexError("macros nested too deeply");
			exit( 1 );
		}

		if (gTempParseMacro->firstMacroLines != NULL)
		{

			gTempMacro = (MACROENTRY *)malloc(sizeof(MACROENTRY));
			if (gTempMacro == NULL)
			{
				LexError("Out of memory allocating MACROENTRY structure.\n");
			}
			else
			{

				gTempMacro->next = NULL;
				gTempMacro->prev = NULL;
				gTempMacro->macroName = NULL;
				gTempMacro->firstMacroParms = NULL;
				gTempMacro->lastMacroParms = NULL;
				gTempMacro->firstMacroLines = NULL;
				gTempMacro->lastMacroLines = NULL;
				gTempMacro->numParms = 0;
				gTempMacro->nLines = 0;

				gbTempInsideMacro = true;		// flag we are currently doing a macro replace.
				gInvokeState = YYSTATE;
				if (gTempParseMacro->numParms > 0)
				{
					BEGIN(MACROPARMSTART);
				}
				else
				{
					EndMacroParms();
					gbTempInsideMacro = false;	// no longer waiting for macro invocation
				}				
			}
		}
	}
	else
	{
		BEGIN(INITIAL);
		REJECT;
	}
}

[,\[\]\-\+\(\)\*\<\>\/\%_\.] { 
//    fprintf( stderr, "%c ", yytext[0] );
	return yytext[0];
}


[ \t]+	{}

<DEFINE>\n {
	LexError("Didn't find label string for #define.\n");
	BEGIN(SAVELINE);
//	return '\n';
}

"\n" {
//fprintf(stderr, "\n");
//	line_incr = 1;
	line_incr++;
	BEGIN(SAVELINE);
	return '\n';
}

<EATSTRING>{alphadigs}+ {
	BEGIN(INITIAL);
//	fprintf( stderr, "%s", yytext );
	if (yyleng == 1)
		return yytext[0];
	else
		LexError("Unrecognized Token: %s\n", yytext);
	return UNKNOWN_STRING;
}

<EATSTRING>[\001-\040]		{
//	vs10_lval.ival = yytext[0];
	LexError("Illegal character: %d decimal.\n", yytext[0]);
	return(ILLEGAL);
}

[\001-\040]		{
//	vs10_lval.ival = yytext[0];
	LexError("Illegal character: %d decimal.\n", yytext[0]);
	return(ILLEGAL); 
}

<EATSTRING>. {
	return yytext[0];
}

. {
	BEGIN(EATSTRING);
	yyless(0);
}

<<EOF>> {
	bool wasInMacro;
	bool oneLiner;
	char *macroText;

	wasInMacro = gbInsideMacro;
	oneLiner = false;


	// if we are inside the macro then do next line until their are no more
	if (gbInsideMacro)
	{
		oneLiner = (gParseMacro->nLines == 0);

		// free the temporary parameter replaced line we were working on.
		// get next line in macro text, if any
		gMacroLineParse = gMacroLineParse->next;
		// more lines to parse?
		if (gMacroLineParse != NULL)
		{
			macroText = gMacroLine;
			// if no replacement text, just use source line
			if (gParseMacro->firstMacroParms == NULL)
			{
				macroText = gMacroLineParse->macroText;
			}
			else
			{
				// replace the macro parameters
				ReplaceMacroParms(gMacroLineParse->macroText, gMacroLine, gParseMacro, gInvokeMacro);
			}

//			if (gExpandMacros)
//			{
//				strcpy(gSaveLine, macroText);
//			}

			BEGIN(INITIAL);
			// and lex it.
			yy_scan_string(macroText);
		}
		else
		{
			// no more lines in this macro, so free the working macro
			SAFEFREE(gInvokeMacro);
			// shut off flag for inside a macro replacement state.
			gbInsideMacro = false;
		}
	}

	if (gbProcessingIFDEF && !wasInMacro)
	{
		LexError("End of file reached before #endif found, macro started on line %d.\n", gIfDefStartLine);
	}

	if (!gbInsideMacro)
	{
		if ( gIncludeStackIndex == 0 )
		{
			if (!gbProcessingBuiltIn)
				CleanUp();
			return 0;
//			return TOKEN_EOF;
		}
		else
		{
			yy_delete_buffer( YY_CURRENT_BUFFER );
			SAFEFREE(gCurFileName);
//			SAFEDELETE(myin);
//			SAFECLOSE(yyin);
		}

		gIncludeStackIndex--;
		SAFEDELETEARRAY( gIncludeStack[gIncludeStackIndex].nextString );
		yy_switch_to_buffer(gIncludeStack[gIncludeStackIndex].buffer );
		gCurFileName = gIncludeStack[gIncludeStackIndex].fileName;
//		yyin = gIncludeStack[gIncludeStackIndex].fileHandle;
		myin = gIncludeStack[gIncludeStackIndex].prevString;
		yylineno = gIncludeStack[gIncludeStackIndex].lineNo;
		gInvokeMacro = gIncludeStack[gIncludeStackIndex].lastInvokeMacro;
		gParseMacro = gIncludeStack[gIncludeStackIndex].lastParseMacro;
		gMacroLineParse = gIncludeStack[gIncludeStackIndex].lastMacroLineParse;
		gbInsideInclude = gIncludeStack[gIncludeStackIndex].lastbInsideInclude;
		gbInsideMacro = gIncludeStack[gIncludeStackIndex].lastbInsideMacro;
		gbProcessingIFDEF = gIncludeStack[gIncludeStackIndex].lastbProcessingIFDEF;

		if (!gbInsideMacro && !oneLiner)
		{
//			GenSwitchFileNames(gCurFileName);
			BEGIN(SAVELINE);
		}
		else
		{
			BEGIN(INITIAL);
		}

		// gSaveLine was last line saved, before macro invocation
		if (wasInMacro && !gbInsideMacro && !oneLiner)
		{
//			GenDebugLine();
//			GenListString();
			gLinesAssembled++;
			yylineno++;
		}

	}

}

%%


//=====================================================================
// Function:	FindNMacro
// Description:	Look through macros and see if it had been predefined
// Parameters:	findName = name to lookup
//				sLen = # characters valid in source (findName)
// Returns:		MACROENTRY * = pointer to macro entry if found
//=====================================================================
MACROENTRY *FindNMacro(char *findName, unsigned int sLen)
{
	MACROENTRY *curEntry;

	curEntry = gLastMacro;
	while (curEntry != NULL)
	{
		if (strlen(curEntry->macroName) == sLen)
		{
			if (!strncmp(curEntry->macroName, findName, sLen))
			{
				break;
			}
		}

		curEntry = curEntry->prev;
	}

	return curEntry;
	
}

//=====================================================================
// Function:	FindMacro
// Description:	Look through macros and see if it had been predefined
// Parameters:	findName = name to lookup
// Returns:		MACROENTRY * = pointer to macro entry if found
//=====================================================================
MACROENTRY *FindMacro(char *findName)
{
	MACROENTRY *curEntry;

	curEntry = gLastMacro;
	while (curEntry != NULL)
	{
		if (!strcmp(curEntry->macroName, findName))
		{
			break;
		}

		curEntry = curEntry->prev;
	}

	return curEntry;
	
}

//=====================================================================
// Function:	CleanUp
// Description:	Clean up the #define strings
// Parameters:	.
// Returns:		.
//=====================================================================
void CleanUp()
{
	void *tPtr;

	// free up the macros that were alloced
	while (gLastMacro != NULL)
	{

		FreeMacroEntry(gLastMacro);

		tPtr = gLastMacro;
		gLastMacro = gLastMacro->prev;
		SAFEFREE(tPtr);
	}

}

//=====================================================================
// Function:	FreeMacroEntry
// Description:	Frees up the macro entry data, (parms, lines of text)
// Parameters:	macEntry = pointer to the MACROENTRY structure
// Returns:		.
//=====================================================================
void FreeMacroEntry(MACROENTRY *macEntry)
{
	MACROTEXT *tText;
	MACROTEXT *tNext;

	SAFEFREE(macEntry->macroName);
	SAFEFREE(macEntry->fileName);
	// free the macro lines that were alloced
	tText = macEntry->lastMacroLines;
	while (tText != NULL)
	{
		tNext = tText->prev;
		SAFEFREE(tText);
		tText = tNext;
	}

	// free the text of the macro parms that were alloced
	tText = macEntry->lastMacroParms;
	while (tText != NULL)
	{
		tNext = tText->prev;
		SAFEFREE(tText);
		tText = tNext;
	}
}

//=====================================================================
// Function:	CheckMacroFunctions
// Description:	Find if this text is a builtin macro function
// Parameters:	lookString = non-null terminated string of possible
//				and if found set global macro function call
// Returns:		.
//=====================================================================
void CheckMacroFunctions(char *lookString, unsigned int *recognizedLen, char **invString)
{

	unsigned int i;
	unsigned int sLen;
	
	for (i=0; i< NUM_MACRO_FUNCTIONS; i++)
	{
		sLen = strlen(gMacroFunctions[i].name);
		if (!strncmp(gMacroFunctions[i].name, lookString, sLen))
		{
			gMacroCallFunction = gMacroFunctions[i].function;
			*recognizedLen = sLen;
			*invString = NULL;
			return;
		}
	}
}

//=====================================================================
// Function:	FindAlphaNum
// Description:	Find a whole alpha numeric string, ie consists of
//				[A-Za-z0-9_] only
// Parameters:	srcStr = source string to search through.
//				sLen = unsinged int pointer to length of string found
// Returns:		pointer to found start of string.
//				NULL if none.
//=====================================================================
char *FindAlphaNum(char *srcStr, unsigned int *sLen)
{
	char curChar;
	char *foundStr;

	while (*srcStr != '\0')
	{
		curChar = toupper(*srcStr);
		if ((curChar >= 'A') && (curChar <= 'Z'))
			break;

		if ((curChar >= '0') && (curChar <='9'))
			break;

		if (curChar == '_')
			break;

		srcStr++;
	}

	if (*srcStr == '\0')
	{
		return NULL;
	}

	foundStr = srcStr;

	*sLen = 0;
	// now search for end of string of [A-Za-z0-9_]
	while (*srcStr != '\0')
	{
		curChar = toupper(*srcStr);
		if ((curChar < 'A') || (curChar > 'Z'))
		{
			if ((curChar < '0') || (curChar > '9'))
			{
				if (curChar != '_')
					break;
			}
		}

		(*sLen)++;
		srcStr++;
	}

	return foundStr;

}

//=====================================================================
// Function:	FindDefineParm
// Description:	Find if the MACROENTRY->macroText linked list contains
//				replaceable parameters.
// Parameters:	srcParms = pointer to MACROENTRY structure for source
//						parameters
//				invParms = MACROENTRY pointer to invocation parameters
//				lookString = non-null terminated string of possible
//							replaceable string
//				recognizedLen = replacement string matched length
//				invString = invocation string to replace with
// Returns:		pointer to first character found in lookstring
//=====================================================================
char *FindDefineParm(MACROENTRY *srcParms, MACROENTRY *invParms, 
						char *lookString, unsigned int *recognizedLen, char **invString)
{
	MACROTEXT *srcText;
	MACROTEXT *invText;
	char *checkStr;
	unsigned int checkLen = 0;
	unsigned int sLen;

	checkStr = lookString;
	*invString = NULL;

	// first search for first [A-Za-z0-9_] only string
	checkStr = FindAlphaNum(lookString, &checkLen);

	while (checkStr != NULL)
	{
		// check all the #define parameters for match
		srcText = srcParms->firstMacroParms;
		invText = invParms->firstMacroParms;
		while (srcText)
		{
			sLen = strlen(srcText->macroText);
			// lengths should match
			if (sLen == checkLen)
			{
				if (!strncmp(checkStr, srcText->macroText, checkLen))
				{
					// it matched so return replacement text
					*invString = invText->macroText;
					// and length that we recognized
					*recognizedLen = checkLen;
					return checkStr;
				}
			}

			srcText = srcText->next;
			invText = invText->next;
		}

		// not found yet, so go to next string.
		checkStr = FindAlphaNum(checkStr+checkLen, &checkLen);
	}

	return NULL;
}

//=====================================================================
// Function:	FindReplaceParm
// Description:	Find if the MACROENTRY->macroText linked list contains
//				a replaceable parameters.
// Parameters:	srcParms = pointer to MACROENTRY structure for source
//						parameters
//				invParms = MACROENTRY pointer to invocation parameters
//				lookString = non-null terminated string of possible
//							replaceable string
//				recognizedLen = replacement string matched length
//				invString = invocation string to replace with
// Returns:		.
//=====================================================================
void FindReplaceParm(MACROENTRY *srcParms, MACROENTRY *invParms, 
						char *lookString, unsigned int *recognizedLen, char **invString)
{
	unsigned int sLen;
	MACROTEXT *srcText;
	MACROTEXT *invText;

	*recognizedLen = 0;
	*invString = NULL;

	srcText = srcParms->firstMacroParms;
	invText = invParms->firstMacroParms;

	if (srcText != NULL)
	{
		// go until srcText # strings ends
		while (srcText != NULL)
		{
			sLen = strlen(srcText->macroText);
			if (!strncmp(srcText->macroText, lookString, sLen))
			{
				// found it so return src, replacement string
				*recognizedLen = strlen(srcText->macroText);
				*invString = invText->macroText;
				// call function macro if it was invoked prior.
				if (gMacroCallFunction != NULL)
				{
					gMacroCallFunction(lookString, recognizedLen, invString);
					gMacroCallFunction = NULL;
				}
				return;
			}

			srcText = srcText->next;
			invText = invText->next;
		}
	}

	// ok, it wasn't found, look through builtin macro functions
	CheckMacroFunctions(lookString, recognizedLen, invString);
}

//=====================================================================
// Function:	ReplaceMacroParms
// Description:	Replace macro parameters when macro was defined, with
//				those specified on the macro invocation line
// Parameters:	srcLine = source line to replace src macro parms with
//				destLine = destination line save to.
//				invocation macro parameters.
//				parseMacro = currently parsing macro entry
//				invParms = invocation macro entry
// Returns:		.
//=====================================================================
void ReplaceMacroParms(char *srcLine, char *destLine, 
							MACROENTRY *srcParms, MACROENTRY *invParms)
{
	char *findReplace;
	char *invString;
	unsigned int sLen;
	unsigned int dLen;
	unsigned int copyLen;
	unsigned int subLen;
	unsigned int recognizedLen;

	destLine[0]= '\0';

	sLen = strlen(srcLine);
	dLen = 0;

	while (sLen > 0)
	{
		// strtok might work better except it modifies the string, so
		// kind of do my own....
		if (!srcParms->bIsDefine)
		{
			findReplace = strchr(srcLine, '%');
			if (findReplace != NULL)
			{
				// bypass % sign in findReplacement
				findReplace++;
				// figure out length of source before %
				copyLen = (findReplace - srcLine)-1;
				// check if there is a replacement string
				FindReplaceParm(srcParms, invParms, findReplace, &recognizedLen, &invString);
			}
			else
			{
				strcat(destLine, srcLine);
				return;
			}
		}
		else
		{
			findReplace = FindDefineParm(srcParms, invParms, srcLine, &recognizedLen, &invString);
			if (findReplace != NULL)
			{
				// figure out length of source before %
				copyLen = findReplace - srcLine;
			}
			else
			{
				strcat(destLine, srcLine);
				return;
			}
		}


		if (invString != NULL)
		{
			// figure out how much we are going to substitute
			subLen = strlen(invString);
		}
		else
		{
			subLen = 0;
		}

		if ((dLen + copyLen + subLen) > MAXSAVELINE)
		{
			LexError("Macro string overrun.\n");
			CleanUp();
			exit(ERROR_MACRO_OVERRUN);
		}

		if (copyLen > 0)
		{
			strncat(destLine, srcLine, copyLen);
			dLen += copyLen;
		}

		srcLine += copyLen;
		sLen -= copyLen;
		// in macro so skip % part of variable
		if (!srcParms->bIsDefine)
		{
			// skip %, also
			srcLine++;
			sLen--;
		}

		if (invString != NULL)
		{
			strcat(destLine, invString);
			dLen += strlen(invString);
		}

		srcLine += recognizedLen;
		sLen -= recognizedLen;
	}

}

//=====================================================================
// Function:	SaveMacroText
// Description:	Adds a string to a linked list of MACROTEXT structures
// Parameters:	srcText = pointer to source text to save
//				lastMacroText = last allocated, or NULL
// Returns:		newly allocated MACROTEXT structure, or NULL
//=====================================================================
MACROTEXT *SaveMacroText(char *srcText, MACROTEXT *lastMacroText)
{
	MACROTEXT *curMacroText;

	curMacroText = (MACROTEXT *)malloc(sizeof(MACROTEXT));
	if (curMacroText == NULL)
	{
		return NULL;
	}
	else
	{
		// no next entry but set up previous with previously alloced macro parameter
		curMacroText->next = NULL;
		curMacroText->prev = lastMacroText;

		// if the macroParm pointer is null then we are the first allocated
		// so if not set the last one allocate next pointer to newly allocated structure
		if (lastMacroText != NULL)
		{
			lastMacroText->next = curMacroText;
		}

		/* %%%%% this should be set up in memory pools. */
		curMacroText->macroText = strdup(srcText);
		if (curMacroText->macroText == NULL)
		{
			SAFEFREE(curMacroText);
			return NULL;
		}
	}

	return curMacroText;
}

//=====================================================================
// Function:	ParseBuiltInMacroParms
// Description:	parse parameters of string and fill in MACROENTRY
//				structure.
// Parameters:	parsedMacro = pointer to MACROENTRY structure that gets
//				filled in with parameter pointers and count
//				parmStr = string to parse parameters from
// Returns:		false if error
//=====================================================================
bool ParseBuiltInMacroParms(MACROENTRY *parsedMacro, char *parmStr)
{
	char *endStr;
	char *foundParm;
	MACROTEXT *prevMT;
	MACROTEXT *curMT;

	parsedMacro->numParms = 0;
	parsedMacro->firstMacroParms = NULL;
	
	foundParm = strdup(parmStr);
	if (foundParm == NULL)
	{
		LexError("Out of memory parsing bultin macro parameters.\n");
		return false;
	}

	// assume a ')' is on the end.
	endStr = strrchr(foundParm, ')');
	if (endStr == NULL)
	{
		LexWarning("Ending parenthesis not found for macro %s.\n", parsedMacro->macroName);
		endStr = foundParm + strlen(foundParm);
	}

	prevMT = NULL;
	// strip out and separate parameters
	while (foundParm < endStr)
	{
		// allocate a macro text structure
		curMT = (MACROTEXT *)malloc(sizeof(MACROTEXT));
		if (curMT == NULL)
		{
			free(parmStr);
			LexError("Out of memory parsing bultin macro parameters.\n");
			return false;
		} 
		curMT->next = NULL;
		curMT->prev = prevMT;
		parsedMacro->numParms++;

		if (prevMT != NULL)
		{
			prevMT->next = curMT;
		}
		else
		{
			parsedMacro->firstMacroParms = curMT;
		}

		curMT->macroText = foundParm;
		// search for next parameters, delimited by comma
		foundParm = strchr(foundParm, ',');
		if (foundParm == NULL)
		{
			foundParm = endStr;
			*foundParm = '\0';
		}
		else
		{
			// skip comma
			*foundParm = '\0';
			foundParm++;
		}
		prevMT = curMT;
	}

	return true;
}

//=====================================================================
// Function:	MacroMathFunction
// Description:	Comes here after macro replacement is done to perform
//				some mathematic function on parameter (macro replacement
//				 string (ie, register))
// Parameters:	invMacro = macroentry pointer containing macro information
//				recognizedLen = # characters recoginized so far
//				invStr = invoked replacement string so far
//				mathStr = "-", "+", etc for mathematic function
// Returns:		new recognizedLen, invStr, with incremented #
//=====================================================================
void MacroMathFunction(MACROENTRY *invMacro, unsigned int *recognizedLen, char **invStr,
						const char *mathStr)
{
	char *numStartStr;
	unsigned int sLen;
	char numberStr[256];
	unsigned int number = 0;
	char *operand;


	// verify enough paramters to complete operation
	if (invMacro->numParms != 2)
	{
		LexError("Two parameters are required for %s macro\n", invMacro->macroName);
		return;
	}

	// get second macro parm, which is add by amount.
	operand = invMacro->firstMacroParms->next->macroText;

	// first find inner most bracket if any
	numStartStr = strrchr(*invStr, ']');
	if (numStartStr == NULL)
	{
		numStartStr = strrchr(*invStr, ')');
	}

	if (numStartStr != NULL)
	{
		if ((strlen(*invStr)+strlen(operand)+1) > MAXREPLACESTRING)
		{
			LexError("Out of Temporary string replacement memory inside builtin macro %s\n",
					invMacro->macroName);
		}
		else
		{
			sLen = (numStartStr - *invStr);
			gReplaceText[0] = '\0';
			strncat(gReplaceText, *invStr, sLen);
			strcat(gReplaceText, mathStr);
			strcat(gReplaceText, operand);
			strcat(gReplaceText, numStartStr);
			*invStr = gReplaceText;
		}
	}
	else
	{
		numStartStr = strpbrk(*invStr, "0123456789");
		if (numStartStr != NULL)
		{
			// put up to number we found
			sLen = numStartStr - *invStr;
			if (sLen > MAXREPLACESTRING)
				goto ErrOut;

			gReplaceText[0] = '\0';
			strncat(gReplaceText, *invStr, sLen);

			switch (mathStr[0])
			{
				case '-':
					number = atoi(numStartStr)-atoi(operand);
					break;
				case '+':
					number = atoi(numStartStr)+atoi(operand);
					break;
			}
			sprintf(numberStr, "%d", number);

			if ((strlen(gReplaceText) + strlen(numberStr)) > MAXREPLACESTRING)
				goto ErrOut;

			strcat(gReplaceText, numberStr);

			while ((*numStartStr != '\0') && (*numStartStr >= '0' && *numStartStr <= '9'))
				numStartStr++;

			if ((strlen(gReplaceText) + strlen(numStartStr)) > MAXREPLACESTRING)
				goto ErrOut;

			strcat(gReplaceText, numStartStr);

			*invStr = gReplaceText;
		}
		else
		{
			if ((strlen(*invStr)+strlen(operand)+1) > MAXREPLACESTRING)
			{
				LexError("Out of Temporary string replacement memory inside builtin macro %s\n",
					invMacro->macroName);
			}
			else
			{
				sprintf(gReplaceText, "%s%s%s", *invStr, mathStr, operand);
				*invStr = gReplaceText;
			}
		}
	}


	return;

ErrOut:
	LexError("Out of Temporary string replacement memory inside builtin macro %s\n",
				invMacro->macroName);
	// skip ')'
	(*recognizedLen)++;
}

//=====================================================================
// Function:	MacroIncFunction
// Description:	Comes here after macro replacement is done to increment
//				macro replacement string (ie, register)
// Parameters:	lookStr = string after '(', so we can get parameters
//				recognizedLen = # characters recoginized so far
//				invStr = invoked replacement string so far
// Returns:		new recognizedLen, invStr, with incremented #
//=====================================================================
void MacroIncFunction(char *lookStr, unsigned int *recognizedLen, char **invStr)
{
	MACROENTRY tMEntry;
	MACROTEXT parm1;
	MACROTEXT parm2;

	tMEntry.macroName = (char *)"%inc()";
	tMEntry.numParms = 2;
	tMEntry.firstMacroParms = &parm1;
	parm1.prev = NULL;
	parm1.next = &parm2;
	parm1.macroText = *invStr;
	parm2.prev = &parm1;
	parm2.next = NULL;
	parm2.macroText = (char *)"1";

	MacroMathFunction(&tMEntry, recognizedLen, invStr, "+");
	// skip ')'
	(*recognizedLen)++;
}

//=====================================================================
// Function:	MacroDecFunction
// Description:	Comes here after macro replacement is done to decrement
//				macro replacement string (ie, register)
// Parameters:	lookStr = string after '(', so we can get parameters
//				recognizedLen = # characters recoginized so far
//				invStr = invoked replacement string so far
// Returns:		new recognizedLen, invStr, with decremented #
//=====================================================================
void MacroDecFunction(char *lookStr, unsigned int *recognizedLen, char **invStr)
{
	MACROENTRY tMEntry;
	MACROTEXT parm1;
	MACROTEXT parm2;

	tMEntry.macroName = (char *)"%dec()";
	tMEntry.numParms = 2;
	tMEntry.firstMacroParms = &parm1;
	parm1.prev = NULL;
	parm1.next = &parm2;
	parm1.macroText = *invStr;
	parm2.prev = &parm1;
	parm2.next = NULL;
	parm2.macroText = (char *)"1";

	MacroMathFunction(&tMEntry, recognizedLen, invStr, "-");
	// skip ')'
	(*recognizedLen)++;
}

//=====================================================================
// Function:	MacroAddFunction
// Description:	Comes here after macro replacement is done to add
//				macro replacement string (ie, register)
// Parameters:	lookStr = string after '(', so we can get parameters
//				recognizedLen = # characters recoginized so far
//				invStr = invoked replacement string so far
// Returns:		new recognizedLen, invStr, with incremented #
//=====================================================================
void MacroAddFunction(char *lookStr, unsigned int *recognizedLen, char **invStr)
{
	MACROENTRY tMEntry;
	MACROTEXT *curMT;
	MACROTEXT *nextMT;
	unsigned int i;

	tMEntry.macroName = (char *)"%add()";
	if (strlen(lookStr) > MAXREPLACESTRING)
	{
		LexError("Out of Temporary string replacement memory inside builtin macro %add()\n");
		return;
	}
	if (ParseBuiltInMacroParms(&tMEntry, lookStr))
	{
		MacroMathFunction(&tMEntry, recognizedLen, invStr, "+");
		// skip ',' strlen(parm2)+ ')'
		(*recognizedLen) += strlen(tMEntry.firstMacroParms->next->macroText)+2;
	}

	curMT = tMEntry.firstMacroParms;
	// in this case only one string was allocated
	free(curMT->macroText);
	for (i=0; i<tMEntry.numParms; i++)
	{
		nextMT = curMT->next;
		free(curMT);
		curMT = nextMT;
	}
}

//=====================================================================
// Function:	MacroSubFunction
// Description:	Comes here after macro replacement is done to subtract
//				macro replacement string (ie, register)
// Parameters:	invParms, parameters that macro was invoked with
//				recognizedLen = # characters recoginized so far
//				invStr = invoked replacement string so far
// Returns:		new recognizedLen, invStr, with incremented #
//=====================================================================
void MacroSubFunction(char *lookStr, unsigned int *recognizedLen, char **invStr)
{
	MACROENTRY tMEntry;
	MACROTEXT *curMT;
	MACROTEXT *nextMT;
	unsigned int i;

	tMEntry.macroName = (char *)"%sub()";
	if (ParseBuiltInMacroParms(&tMEntry, lookStr))
	{
		MacroMathFunction(&tMEntry, recognizedLen, invStr, "-");
		// skip ',' strlen(parm2)+ ')'
		(*recognizedLen) += strlen(tMEntry.firstMacroParms->next->macroText)+2;
	}
	curMT = tMEntry.firstMacroParms;
	// in this case only one string was allocated
	free(curMT->macroText);
	for (i=0; i<tMEntry.numParms; i++)
	{
		nextMT = curMT->next;
		free(curMT);
		curMT = nextMT;
	}
}

//=====================================================================
// Function:	EndMacroParms
// Description:	Does update and cleanup one end of macro parameters
//				is reached
// Parameters:	.
// Returns:		.
//=====================================================================
void EndMacroParms()
{
	char *curFileName;
	char *macroFileName;
	char tempStr[1024];
	char *macroText;

	if (gbTempInsideMacro)
	{
		if (gTempParseMacro->numParms != gTempMacro->numParms)
		{
			LexError("Macro invocation number of parameters do not match macro definition, skipping\n");
			BEGIN(INITIAL);
			SAFEFREE(gTempMacro);
		}
		else
		{
			// we got all the parameters for the MACRO invocation, so start inside
			// the macro now, by saving off current state on stack
			gIncludeStack[gIncludeStackIndex].lineNo = yylineno;
			gIncludeStack[gIncludeStackIndex].fileName = gCurFileName;
//			gIncludeStack[gIncludeStackIndex].fileHandle = yyin;
//fprintf( stderr, "Chris fix this code with myin stuff\n" );
			gIncludeStack[gIncludeStackIndex].prevString = myin;
			gIncludeStack[gIncludeStackIndex].nextString = NULL;
			gIncludeStack[gIncludeStackIndex].lastInvokeMacro = gInvokeMacro;
			gIncludeStack[gIncludeStackIndex].lastParseMacro = gParseMacro;
			gIncludeStack[gIncludeStackIndex].lastMacroLineParse = gMacroLineParse;
			gIncludeStack[gIncludeStackIndex].lastbInsideMacro = gbInsideMacro;
			gIncludeStack[gIncludeStackIndex].lastbInsideInclude = gbInsideInclude;
			gIncludeStack[gIncludeStackIndex].buffer = YY_CURRENT_BUFFER;
			gIncludeStack[gIncludeStackIndex].lastbProcessingIFDEF = gbProcessingIFDEF;
			gIncludeStackIndex++;

			gParseMacro = gTempParseMacro;
			gInvokeMacro = gTempMacro;
			gbInsideMacro = gbTempInsideMacro;

			gbTempInsideMacro = false;

//			yyin = NULL;
			myin = NULL;
			curFileName = gCurFileName;
			if (curFileName == NULL)
				curFileName = (char *)"";

			macroFileName = gParseMacro->fileName;
			if (macroFileName == NULL)
				macroFileName = (char *)"";

			sprintf(tempStr, "%s(%d) : References ->\n%s", curFileName, yylineno, macroFileName); 
			gCurFileName = strdup(tempStr);
			gMacroLineParse = gParseMacro->firstMacroLines;

			macroText = gMacroLine;
			// if no replacement text, just use source line
			if (gParseMacro->firstMacroParms == NULL)
			{
				macroText = gMacroLineParse->macroText;
			}
			else
			{
				// replace the macro parameters
				ReplaceMacroParms(gMacroLineParse->macroText, gMacroLine, gParseMacro, gInvokeMacro);
			}

			yylineno = gParseMacro->lineNo;
			if (gParseMacro->nLines >= 1)
			{
				strcpy(gSaveLine, macroText);
			}

//			if (gExpandMacros && (gParseMacro->nLines >= 1))
//			{
//				// in case there is anything there dump it out
//				GenDebugLine();
//				GenListString();			
//				if (gInvokeMacro->nLines >= 1)
//					GenSwitchFileNames(macroFileName);
//			}

			BEGIN(gInvokeState);
			yy_scan_string(macroText);
			gInvokeState = INITIAL;
		}
	}
	else
	{
		if (gLastMacro != NULL)
		{
			gLastMacro->next = gTempMacro;
		}
		gLastMacro = gTempMacro;
		BEGIN(MACROBODY);
	}
}

//=====================================================================
// Function:	FindSwizzleValue
// Description:	see if valid swizzle value and return the bits
// Parameters:	swizzleTex = pointer to characters to analyze
// Returns:		unsigned int = bits for swizzle values, or 0 for error
//=====================================================================
unsigned int FindSwizzleValue(char *swizzleText)
{
	unsigned int swizzleBits;
	unsigned int sLen;
	unsigned int i;
	unsigned int lastMask;

	sLen = strlen(swizzleText);
	swizzleBits = 0;
	lastMask = 0;

	for (i=0; i<sLen; i++)
	{
		switch (swizzleText[i])
		{
		case 'x':
			swizzleBits |= (WRITEMASK_X << (4*(3-i)));
			lastMask = WRITEMASK_X;
			break;
		case 'y':
			swizzleBits |= (WRITEMASK_Y << (4*(3-i)));
			lastMask = WRITEMASK_Y;
			break;
		case 'z':
			swizzleBits |= (WRITEMASK_Z << (4*(3-i)));
			lastMask = WRITEMASK_Z;
			break;
		case 'w':
			swizzleBits |= (WRITEMASK_W << (4*(3-i)));
			lastMask = WRITEMASK_W;
			break;
		}
	}

	for (; i<4; i++)
	{
		swizzleBits |= (lastMask << (4*(3-i)));
	}

	return swizzleBits;
}

#if 0
unsigned int FindSwizzleValue(char *swizzleText)
{

	DWORD swizzleBits;
	DWORD sLen;
	DWORD i;
	DWORD lastIndex;

	sLen = strlen(swizzleText);
	swizzleBits = 0;
	lastIndex = 0;

	for (i=0; i<sLen; i++)
	{
		switch (swizzleText[i])
		{
		case 'x':
			swizzleBits |= (0 << (D3DVS_SWIZZLE_SHIFT + (i * 2)));
			lastIndex = 0;
			break;
		case 'y':
			swizzleBits |= (1 << (D3DVS_SWIZZLE_SHIFT + (i * 2)));
			lastIndex = 1;
			break;
		case 'z':
			swizzleBits |= (2 << (D3DVS_SWIZZLE_SHIFT + (i * 2)));
			lastIndex = 2;
			break;
		case 'w':
			swizzleBits |= (3 << (D3DVS_SWIZZLE_SHIFT + (i * 2)));
			lastIndex = 3;
			break;
		}
	}

	for (; i<4; i++)
	{
		swizzleBits |= (lastIndex << (D3DVS_SWIZZLE_SHIFT + (i * 2)));
	}

	return swizzleBits;

	
}
#endif

//=====================================================================
// Function:	FindRegisterMask
// Description:	Look through register mask strings
// Parameters:	findName = name to lookup
// Returns:		unsigned int with token value
//=====================================================================
unsigned int MakeRegisterMask(char *findName)
{

	unsigned int regMask;
	char *findFirst;

	regMask = 0;

	findFirst = strchr(findName, 'x');
	if (findFirst != NULL)
	{
		if (strchr(findFirst+1, 'x') != NULL)
		{
			return 0;
		}

		regMask |= WRITEMASK_X;
	}

	findFirst = strchr(findName, 'y');
	if (findFirst != NULL)
	{
		regMask |= WRITEMASK_Y;
		// invalide write mask, must be swizzle
		if (strchr(findFirst+1, 'x') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'y') != NULL)
		{
			return 0;
		}

	}

	findFirst = strchr(findName, 'z');
	if (findFirst != NULL)
	{
		regMask |= WRITEMASK_Z;
		if (strchr(findFirst+1, 'x') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'y') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'z') != NULL)
		{
			return 0;
		}
	}

	findFirst = strchr(findName, 'w');
	if (findFirst != NULL)
	{

		regMask |= WRITEMASK_W;
		if (strchr(findFirst+1, 'x') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'y') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'z') != NULL)
		{
			return 0;
		}

		if (strchr(findFirst+1, 'w') != NULL)
		{
			return 0;
		}
	}

	return regMask;
	
}

//=====================================================================
// Function:	LexError
// Description:	output an error to the stdout
// Parameters:	typical printf like format
// Returns:		.
//=====================================================================
void LexError(const char *format, ...)
{
	char errstring[4096];
	va_list marker;

//	fprintf( stderr,"(%d) : Error : ", yylineno);
	if ( gbInsideInclude )
		{
		sprintf( errstring, "%s", gCurFileName );
		sprintf( errstring+strlen(errstring),"(%d) : Error : ", yylineno);
		}
	else
		{
		sprintf( errstring,"(%d) : Error : ", yylineno);
		}

	va_start(marker, format);
//	vprintf(format, marker);
	vsprintf(errstring+strlen(errstring), format, marker);
	va_end(marker);
	errors.set( errstring );
}

//=====================================================================
// Function:	LexWarning
// Description:	output a warning to the stdout
// Parameters:	typical printf like format
// Returns:		.
//=====================================================================
void LexWarning(const char *format, ...)
{
	char errstring[4096];
	va_list marker;

//	fprintf( stderr,"(%d) : warning : ", yylineno);
	if ( gbInsideInclude )
		sprintf( errstring, "%s", gCurFileName );
	sprintf( errstring+strlen(errstring),"(%d) : Warning : ", yylineno);
//	sprintf( errstring,"(%d) : Warning : ", yylineno);

	va_start(marker, format);
//	vprintf(format, marker);
	vsprintf(errstring+strlen(errstring), format, marker);
	va_end(marker);
	errors.set( errstring );
}

//=====================================================================
// Function:	DebugUnhandledState
// Description:	Come here in debug mode, when a state isn't handled
//				for the Lexer
// Parameters:	.
// Returns:		.
//=====================================================================
void DebugUnhandledState()
{
	fprintf( stderr,"Unhandled state reached, with %s text.\n", yytext);
}

//=====================================================================
// Function:	FindOpcode
// Description:	Look through opcodes and see if in the table
// Parameters:	findName = name to lookup
// Returns:		OPCODEMAP * = pointer to opcode map entry, if found
//=====================================================================
OPCODEMAP *FindOpcode(char *findName)
{

	unsigned i;

	// just do linear search for now
	for (i=0; i<NUMOPCODES; i++)
	{
		if (!stricmp(theOpcodes[i].string, findName))
		{
			return &theOpcodes[i];
		}

	}

	return NULL;
}

char *ReadTextFile(const char * filename)
{
	char path[3][32] = { ".\0",
					     "../../data/programs\0",
					     "../../../data/programs\0" };
	char name[8192];
	int i;

    if (!filename) return 0;

    struct _stat status;
    int found = 0;
    for ( i = 0; i < 3; i++ )
    {
	sprintf( name, "%s/%s", path[i], filename );

	int fh = ::_open(name, _O_RDONLY);

	if(fh != -1)
        {
            int result = _fstat( fh, &status );
            if( result != 0 )
            {
                fprintf( stderr, "An fstat error occurred.\n" );
                break;
            }
            ::_close( fh );
	    found = i+1;
            break;
        }
    }

    if ( 0 == found )
	{
		fprintf(stderr,"Cannot open \"%s\" for stat read!\n", filename);
		return NULL;
	}
    long size = status.st_size;

    char * buf = new char[size+1];

	FILE *fp = 0;
    if (!(fp = fopen(name, "r")))
	{
		fprintf(stderr,"Cannot open \"%s\" for read!\n", name);
		return NULL;
	}

	int bytes;
	bytes = fread(buf, 1, size, fp);

    buf[bytes] = 0;

	fclose(fp);
	return buf;
}

bool vs10_init_more();

bool vs10_init(char* inputString)
{
	BEGIN(SAVELINE);
    myin = inputString;
	line_incr = 0;
	return vs10_init_more();
}

#ifndef vs10_wrap
int vs10_wrap(void)
{
  return(1);
}
#endif
