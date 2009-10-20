#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

#define KEYWORD_BOXHEADER 1
#define KEYWORD_FOLDCONTRACTED 2

static bool isAnOperator(WordList& opList, const char op)
{
	for(int i = 0; i < opList.len; ++i)
	{
		if(op == *(opList[i]))
			return true;
	}

	return false;
}

static inline bool IsAWordChar(int ch)
{
	return ch < 0x80 && (isalnum(ch) || ch == '_');
}

static inline bool IsAWordStart(int ch) {
	return ch < 0x80 && isalpha(ch);
}


//static inline bool isAWordChar(const int ch)
//{
//	return ((ch > 0x20) && (ch <= 0xFF) && (ch != ' ') && (ch != '\n'));
//}

static void ColouriseOMS(unsigned int startPos, int length, int initStyle, WordList* keywordLists[], Accessor &styler)
{
	WordList &primaryKeywords = *keywordLists[0];
	WordList &secondaryKeywords = *keywordLists[1];
	WordList &valueKeywords = *keywordLists[2];
	WordList &operatorKeywords = *keywordLists[3];

	if(initStyle == SCE_OMS_COMMENT) initStyle = SCE_OMS_DEFAULT;

	StyleContext sc(startPos, length, initStyle, styler);
	for(; sc.More(); sc.Forward())
	{
		switch(sc.state)
		{
		case SCE_OMS_NUMBER:
			if(!IsADigit(sc.ch) && sc.ch != '.')
				sc.SetState(SCE_OMS_DEFAULT);
		break;

		case SCE_OMS_COMMENT:
			if(sc.atLineEnd)
				sc.ForwardSetState(SCE_OMS_DEFAULT);
		break;
			
		case SCE_OMS_OPERATOR:
			sc.SetState(SCE_OMS_DEFAULT);
		break;

		case SCE_OMS_IDENTIFIER:
			if(!IsAWordChar(sc.ch))
			{
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if(primaryKeywords.InList(s))
				{
					sc.ChangeState(SCE_OMS_PRIMARY);
				}
				else if(secondaryKeywords.InList(s))
				{
					sc.ChangeState(SCE_OMS_ATTRIBUTE);
				}
				else if(valueKeywords.InList(s))
				{
					sc.ChangeState(SCE_OMS_VALUE);
				}
				sc.SetState(SCE_OMS_DEFAULT);
			}
		break;
		}

		if(sc.state == SCE_OMS_DEFAULT)
		{
			if(IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext)))
				sc.SetState(SCE_OMS_NUMBER);
			else if(sc.Match('/', '/'))
				sc.SetState(SCE_OMS_COMMENT);
			else if(isAnOperator(operatorKeywords, sc.ch))
				sc.SetState(SCE_OMS_OPERATOR);
			else if(IsAWordStart(sc.ch)) // ???
				sc.SetState(SCE_OMS_IDENTIFIER);
		}
	}
	sc.Complete();
}

static void FoldOMS(unsigned int startPos, int length, int initStyle, WordList* keywordLists[], Accessor &styler)
{
	WordList &primaryKeywords = *keywordLists[0];

	unsigned int lengthDoc = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	int styleNext = styler.StyleAt(startPos);
	StyleContext sc(startPos, length, initStyle, styler);

	for (unsigned int i = startPos; i < lengthDoc; i++)
	{
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (ch == '{')
		{
			levelCurrent++;
		}
		else if (ch == '}')
		{
			levelCurrent--;
		}

		if (atEOL)
		{
			int lev = levelPrev;
			if (visibleChars == 0 && foldCompact) 
			{
				lev |= SC_FOLDLEVELWHITEFLAG;
			}
			
			if ((levelCurrent > levelPrev) && (visibleChars > 0))
			{
				lev |= SC_FOLDLEVELHEADERFLAG;
			}

			if (lev != styler.LevelAt(lineCurrent))
			{
				styler.SetLevel(lineCurrent, lev);
			}

			lineCurrent++;
			levelPrev = levelCurrent;
			visibleChars = 0;
		}

		if (!isspacechar(ch))
		{
			visibleChars++;
		}
	}
	
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}


static const char* const omsWordLists[] = {
	"Primary Keywords",
	"Attribute Keywords",
	"Value Keywords",
	"Operators",
	0,
};

LexerModule lmOMS(SCLEX_OMS, ColouriseOMS, "oms", FoldOMS, omsWordLists);
