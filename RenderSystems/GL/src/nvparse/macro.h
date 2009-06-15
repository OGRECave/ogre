#ifdef _WIN32
#include <io.h>
#else
#include <ctype.h>
#endif
typedef struct MACROTEXT {
	MACROTEXT *next;
	MACROTEXT *prev;
	char *macroText;
} MACROTEXT;

typedef struct MACROENTRY
{
	MACROENTRY *next;
	MACROENTRY *prev;
	char *macroName;
	MACROTEXT *firstMacroParms;
	MACROTEXT *lastMacroParms;
	MACROTEXT *firstMacroLines;
	MACROTEXT *lastMacroLines;
	unsigned int numParms;
	char *fileName;
	unsigned int lineNo;
	unsigned int nLines;
	bool bIsDefine;
} MACROENTRY;

#define MAX_IFDEF_DEPTH 1024
typedef struct IFDEFINFO
{
	bool			lastbProcessingIFDEF;			// save off for if we were processing #ifdef
	bool			lastbIFDEF;						// wether ifdef was true or not
	bool			lastbCompareDefine;				// wether we compare #ifdef or #ifndef
	unsigned int	lastIfDefStartLine;				// where we started for this #ifdef
} IFDEFINFO;


typedef void (*MACROFUNCTIONPTR)(char *, unsigned int *, char **);

typedef struct MACROFUNCTIONS {
	const char *name;
	MACROFUNCTIONPTR function;
} MACROFUNCTIONS;
