/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/
// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/file.h>
#include <wx/filename.h>

#include "CodeEditor.h"

#include "SharedDefs.h"

BEGIN_EVENT_TABLE(CodeEditor, wxScintilla)
EVT_SIZE (CodeEditor::OnSize)
// Edit
EVT_MENU (wxID_CLEAR,		CodeEditor::OnEditClear)
EVT_MENU (wxID_CUT,			CodeEditor::OnEditCut)
EVT_MENU (wxID_COPY,		CodeEditor::OnEditCopy)
EVT_MENU (wxID_PASTE,		CodeEditor::OnEditPaste)
EVT_MENU (ID_INDENTINC,		CodeEditor::OnEditIndentInc)
EVT_MENU (ID_INDENTRED,		CodeEditor::OnEditIndentRed)
EVT_MENU (wxID_SELECTALL,	CodeEditor::OnEditSelectAll)
EVT_MENU (ID_SELECTLINE,	CodeEditor::OnEditSelectLine)
EVT_MENU (wxID_REDO,		CodeEditor::OnEditRedo)
EVT_MENU (wxID_UNDO,		CodeEditor::OnEditUndo)
// Find
EVT_MENU (wxID_FIND,		CodeEditor::OnFind)
EVT_MENU (ID_FINDNEXT,		CodeEditor::OnFindNext)
EVT_MENU (ID_REPLACE,		CodeEditor::OnReplace)
EVT_MENU (ID_REPLACENEXT,	CodeEditor::OnReplaceNext)
EVT_MENU (ID_BRACEMATCH,	CodeEditor::OnBraceMatch)
EVT_MENU (ID_GOTO,			CodeEditor::OnGoto)
// View
EVT_MENU_RANGE (ID_HILIGHTFIRST, ID_HILIGHTLAST, CodeEditor::OnHilightLang)
EVT_MENU (ID_DISPLAYEOL,	CodeEditor::OnDisplayEOL)
EVT_MENU (ID_INDENTGUIDE,	CodeEditor::OnIndentGuide)
EVT_MENU (ID_LINENUMBER,	CodeEditor::OnLineNumber)
EVT_MENU (ID_LONGLINEON,	CodeEditor::OnLongLineOn)
EVT_MENU (ID_WHITESPACE,	CodeEditor::OnWhiteSpace)
EVT_MENU (ID_FOLDTOGGLE,	CodeEditor::OnFoldToggle)
EVT_MENU (ID_OVERTYPE,		CodeEditor::OnSetOverType)
EVT_MENU (ID_READONLY,		CodeEditor::OnSetReadOnly)
EVT_MENU (ID_WRAPMODEON,	CodeEditor::OnWrapModeOn)
EVT_MENU (ID_CHARSETANSI,	CodeEditor::OnUseCharset)
EVT_MENU (ID_CHARSETMAC,	CodeEditor::OnUseCharset)
// Extra
EVT_MENU (ID_CHANGELOWER,	CodeEditor::OnChangeCase)
EVT_MENU (ID_CHANGEUPPER,	CodeEditor::OnChangeCase)
EVT_MENU (ID_CONVERTCR,		CodeEditor::OnConvertEOL)
EVT_MENU (ID_CONVERTCRLF,	CodeEditor::OnConvertEOL)
EVT_MENU (ID_CONVERTLF,		CodeEditor::OnConvertEOL)
// scintilla
EVT_SCI_MARGINCLICK (-1, CodeEditor::OnMarginClick)
EVT_SCI_CHARADDED (-1,   CodeEditor::OnCharAdded)
EVT_SCI_UPDATEUI(-1, CodeEditor::OnUpdateUI)

END_EVENT_TABLE()

CodeEditor::CodeEditor(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
	: wxScintilla (parent, id, pos, size, style)
{
	mFileName = _T("");
	//m_language = NULL;

	mLineNumID = 0;
	mLineNumMargin = TextWidth(wxSCI_STYLE_LINENUMBER, _T("99999"));
	mFoldingID = 1;
	mFoldingMargin = 16;
	mDividerID = 1;

	SetProperty(wxT("fold"), wxT("1"));
	SetFoldFlags(16);
	SetMarginType(mFoldingID, wxSCI_MARGIN_SYMBOL);
	SetMarginMask(mFoldingID, wxSCI_MASK_FOLDERS);
	SetMarginSensitive(mFoldingID, true);
	SetMarginWidth(mFoldingID, mFoldingMargin);

	MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDER, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_VLINE);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERSUB, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERSUB, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_LCORNER);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERTAIL, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERTAIL, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_BOXPLUSCONNECTED);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEREND, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEREND, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_BOXMINUSCONNECTED);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPENMID, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPENMID, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_TCORNER);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERMIDTAIL, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERMIDTAIL, wxColour(0x80, 0x80, 0x80));

	// Set defaults, these should eventually be set via user prefs
	SetViewEOL(false);
	SetIndentationGuides(false);
	SetMarginWidth(mLineNumID, mLineNumMargin);
	//SetMarginWidth(mFoldingID, mFoldingMargin);
	//SetMarginSensitive(mFoldingID, true);
	SetEdgeMode(wxSCI_EDGE_LINE);
	//SetViewWhiteSpace(wxSCI_WS_VISIBLEALWAYS);
	SetOvertype(false);
	SetReadOnly(false);
	SetWrapMode(wxSCI_WRAP_NONE);

	wxFont font(10, wxTELETYPE, wxNORMAL, wxNORMAL);
	StyleSetFont(wxSCI_STYLE_DEFAULT, font);
	StyleSetForeground(wxSCI_STYLE_DEFAULT, wxColour(wxT("BLACK")));
	StyleSetBackground(wxSCI_STYLE_DEFAULT, wxColour(wxT("WHITE")));
	StyleSetForeground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("DARK BLUE")));
	StyleSetBackground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("WHITE")));
	StyleSetForeground(wxSCI_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));

	StyleSetBold(wxSCI_STYLE_BRACELIGHT, true);
	
	//InitializePrefs(DEFAULT_LANGUAGE);
	SetTabWidth(4);
	SetUseTabs(false);
	SetTabIndents(true);
	SetBackSpaceUnIndents(true);
	SetIndent(4);

	// Set visibility
	SetVisiblePolicy(wxSCI_VISIBLE_STRICT | wxSCI_VISIBLE_SLOP, 1);
	SetXCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);
	SetYCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);

	SetCaretLineVisible(true);
	SetCaretLineBackground(wxColour(225, 235, 224));

	// Markers
	//MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
	//MarkerSetBackground(wxSCI_MARKNUM_FOLDER, wxColour(_T("BLACK")));
	//MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(_T("WHITE")));
	//MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
	//MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("BLACK")));
	//MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("WHITE")));
	//MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_EMPTY);
	//MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_SHORTARROW);
	//MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_ARROWDOWN);
	//MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_EMPTY);
	//MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_EMPTY);

	// Clear wrong default keys
#if !defined(__WXGTK__)
	//CmdKeyClear(wxSCI_KEY_TAB, 0);
	CmdKeyClear(wxSCI_KEY_TAB, wxSCI_SCMOD_SHIFT);
#endif
	CmdKeyClear('A', wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('C', wxSCI_SCMOD_CTRL);
#endif
	CmdKeyClear('D', wxSCI_SCMOD_CTRL);
	CmdKeyClear('D', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('F', wxSCI_SCMOD_ALT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('L', wxSCI_SCMOD_CTRL);
	CmdKeyClear('L', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('T', wxSCI_SCMOD_CTRL);
	CmdKeyClear('T', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('U', wxSCI_SCMOD_CTRL);
	CmdKeyClear('U', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('V', wxSCI_SCMOD_CTRL);
	CmdKeyClear('X', wxSCI_SCMOD_CTRL);
#endif
	CmdKeyClear('Y', wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('Z', wxSCI_SCMOD_CTRL);
#endif

	UsePopUp(0);
	SetLayoutCache(wxSCI_CACHE_PAGE);
	SetBufferedDraw(1);
}

CodeEditor::~CodeEditor() 
{

}

wxChar CodeEditor::GetLastNonWhitespaceChar(int position /* = -1 */)
{
	if (position == -1)
		position = GetCurrentPos();

	int count = 0; // Used to count the number of blank lines
	bool foundlf = false; // For the rare case of CR's without LF's
	while (position)
	{
		wxChar c = GetCharAt(--position);
		int style = GetStyleAt(position);
		bool inComment = style == wxSCI_C_COMMENT ||
			style == wxSCI_C_COMMENTDOC ||
			style == wxSCI_C_COMMENTDOCKEYWORD ||
			style == wxSCI_C_COMMENTDOCKEYWORDERROR ||
			style == wxSCI_C_COMMENTLINE ||
			style == wxSCI_C_COMMENTLINEDOC;
		if (c == wxT('\n'))
		{
			count++;
			foundlf = true;
		}
		else if (c == wxT('\r') && !foundlf)
			count++;
		else
			foundlf = false;
		if (count > 1) return 0; // Don't over-indent
		if (!inComment && c != wxT(' ') && c != wxT('\t') && c != wxT('\n') && c != wxT('\r'))
			return c;
	}

	return 0;
}

wxString CodeEditor::GetLineIndentString(int line)
{
	int currLine = (line == -1) ? LineFromPosition(GetCurrentPos()) : line;

	wxString text = GetLine(currLine);
	int length = (int)text.Length();
	wxString indent;
	for (int i = 0; i < length; ++i)
	{
		if (text[i] == wxT(' ') || text[i] == wxT('\t'))
			indent << text[i];
		else
			break;
	}

	return indent;
}

int CodeEditor::FindBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested /* = true */)
{
	int level = 0;
	wxChar ch = GetCharAt(position);
	while (ch)
	{
		if (ch == blockEnd)
			++level;

		else if (ch == blockStart)
		{
			if (level == 0) return position;
			--level;
		}

		--position;

		ch = GetCharAt(position);
	}

	return -1;
}

void CodeEditor::HighlightBraces()
{
	int currPos = GetCurrentPos();
	int newPos = BraceMatch(currPos);
	if (newPos == wxSCI_INVALID_POSITION)
	{
		if(currPos > 0)
			newPos = BraceMatch(--currPos);
	}

	wxChar ch = GetCharAt(currPos);
	if (ch == wxT('{') || ch == wxT('[') || ch == wxT('(') ||
		ch == wxT('}') || ch == wxT(']') || ch == wxT(')'))
	{
		if (newPos != wxSCI_INVALID_POSITION)
		{
			BraceHighlight(currPos, newPos);
		}
		else
		{
			BraceBadLight(currPos);
		}
	}
	else BraceHighlight(-1, -1);

	Refresh(false);
}


//----------------------------------------------------------------------------
// Common event handlers
void CodeEditor::OnSize(wxSizeEvent& event)
{
	int x = GetClientSize().x + mLineNumMargin + mFoldingMargin;

	if (x > 0) SetScrollWidth(x);

	event.Skip();
}

// Edit event handlers
void CodeEditor::OnEditRedo(wxCommandEvent &WXUNUSED(event))
{
	if (!CanRedo()) return;

	Redo();
}

void CodeEditor::OnEditUndo(wxCommandEvent &WXUNUSED(event))
{
	if (!CanUndo()) return;

	Undo();
}

void CodeEditor::OnEditClear(wxCommandEvent &WXUNUSED(event))
{
	if (GetReadOnly()) return;

	Clear();
}

void CodeEditor::OnEditCut(wxCommandEvent &WXUNUSED(event))
{
	if (GetReadOnly() || (GetSelectionEnd()-GetSelectionStart() <= 0)) return;

	Cut();
}

void CodeEditor::OnEditCopy(wxCommandEvent &WXUNUSED(event))
{
	if (GetSelectionEnd()-GetSelectionStart() <= 0) return;

	Copy();
}

void CodeEditor::OnEditPaste(wxCommandEvent &WXUNUSED(event))
{
	if(!CanPaste()) return;

	Paste();
}

void CodeEditor::OnFind(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnFindNext(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnReplace(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnReplaceNext(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnBraceMatch(wxCommandEvent &WXUNUSED(event))
{
	int min = GetCurrentPos();
	int max = BraceMatch(min);
	if (max > (min+1))
	{
		BraceHighlight(min + 1, max);
		SetSelection(min + 1, max);
	}
	else
	{
		BraceBadLight(min);
	}
}

void CodeEditor::OnGoto(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnEditIndentInc(wxCommandEvent &WXUNUSED(event))
{
	CmdKeyExecute(wxSCI_CMD_TAB);
}

void CodeEditor::OnEditIndentRed(wxCommandEvent &WXUNUSED(event))
{
	CmdKeyExecute(wxSCI_CMD_DELETEBACK);
}

void CodeEditor::OnEditSelectAll(wxCommandEvent &WXUNUSED(event))
{
	SetSelection(0, GetLength());
}

void CodeEditor::OnEditSelectLine(wxCommandEvent &WXUNUSED(event))
{
	int lineStart = PositionFromLine(GetCurrentLine());
	int lineEnd = PositionFromLine(GetCurrentLine() + 1);
	SetSelection(lineStart, lineEnd);
}

void CodeEditor::OnHilightLang(wxCommandEvent &event)
{
	//InitializePrefs(g_LanguagePrefs [event.GetId() - myID_HILIGHTFIRST].name);
}

void CodeEditor::OnDisplayEOL(wxCommandEvent &WXUNUSED(event))
{
	SetViewEOL (!GetViewEOL());
}

void CodeEditor::OnIndentGuide(wxCommandEvent &WXUNUSED(event))
{
	SetIndentationGuides(!GetIndentationGuides());
}

void CodeEditor::OnLineNumber(wxCommandEvent &WXUNUSED(event))
{
	SetMarginWidth(mLineNumID, GetMarginWidth(mLineNumID) == 0 ? mLineNumMargin : 0);
}

void CodeEditor::OnLongLineOn(wxCommandEvent &WXUNUSED(event))
{
	SetEdgeMode(GetEdgeMode() == 0? wxSCI_EDGE_LINE: wxSCI_EDGE_NONE);
}

void CodeEditor::OnWhiteSpace(wxCommandEvent &WXUNUSED(event))
{
	SetViewWhiteSpace (GetViewWhiteSpace() == 0 ? wxSCI_WS_VISIBLEALWAYS : wxSCI_WS_INVISIBLE);
}

void CodeEditor::OnFoldToggle(wxCommandEvent &WXUNUSED(event))
{
	ToggleFold(GetFoldParent(GetCurrentLine()));
}

void CodeEditor::OnSetOverType(wxCommandEvent &WXUNUSED(event))
{
	SetOvertype(!GetOvertype());
}

void CodeEditor::OnSetReadOnly(wxCommandEvent &WXUNUSED(event))
{
	SetReadOnly(!GetReadOnly());
}

void CodeEditor::OnWrapModeOn(wxCommandEvent &WXUNUSED(event))
{
	SetWrapMode(GetWrapMode() == 0 ? wxSCI_WRAP_WORD : wxSCI_WRAP_NONE);
}

void CodeEditor::OnUseCharset(wxCommandEvent &event)
{
	/*
	int Nr;
	int charset = GetCodePage();

	switch (event.GetId())
	{
		case ID_CHARSETANSI: { charset = wxSCI_CHARSET_ANSI; break; }
		case ID_CHARSETMAC: { charset = wxSCI_CHARSET_ANSI; break; }
	}

	for (Nr = 0; Nr < wxSCI_STYLE_LASTPREDEFINED; Nr++)
	{
		StyleSetCharacterSet(Nr, charset);
	}

	SetCodePage(charset);
	*/
}

void CodeEditor::OnChangeCase(wxCommandEvent &event)
{
	int id = event.GetId();
	if(id == ID_CHANGELOWER) CmdKeyExecute(wxSCI_CMD_LOWERCASE);
	else if(id == ID_CHANGEUPPER) CmdKeyExecute(wxSCI_CMD_UPPERCASE);
}

void CodeEditor::OnConvertEOL(wxCommandEvent &event)
{
	int eolMode = GetEOLMode();

	const int id = event.GetId();
	if(id == ID_CONVERTCR) eolMode = wxSCI_EOL_CR;
	else if(id == ID_CONVERTCRLF) eolMode = wxSCI_EOL_CRLF;
	else if(id == ID_CONVERTLF) eolMode = wxSCI_EOL_LF;

	ConvertEOLs(eolMode);
	SetEOLMode(eolMode);
}

void CodeEditor::OnMarginClick(wxScintillaEvent &event)
{
	if (event.GetMargin() == 1)
	{
		int lineClick = LineFromPosition(event.GetPosition());
		int levelClick = GetFoldLevel(lineClick);
		if ((levelClick & wxSCI_FOLDLEVELHEADERFLAG) > 0)
		{
			ToggleFold (lineClick);
		}
	}
}

void CodeEditor::OnCharAdded(wxScintillaEvent &event)
{
	char ch = event.GetKey();
	int currentLine = GetCurrentLine();
	int pos = GetCurrentPos();

	if (ch == wxT('\n') && currentLine > 0)
	{
		BeginUndoAction();

		wxString indent = GetLineIndentString(currentLine - 1);

		wxChar b = GetLastNonWhitespaceChar();
		if(b == wxT('{'))
		{
			if(GetUseTabs())
				indent << wxT("\t");
			else
				indent << wxT("    ");
		}

		InsertText(pos, indent);
		GotoPos((int)(pos + indent.Length()));
		ChooseCaretX();

		EndUndoAction();
	}
	else if(ch == wxT('}'))
	{
		BeginUndoAction();

		wxString line = GetLine(currentLine);
		line.Trim(false);
		line.Trim(true);
		if(line.Matches(wxT("}")))
		{
			pos = GetCurrentPos() - 2;
			pos = FindBlockStart(pos, wxT('{'), wxT('}'));

			if(pos != -1)
			{
				wxString indent = GetLineIndentString(LineFromPosition(pos));
				indent << wxT('}');
				DelLineLeft();
				DelLineRight();
				pos = GetCurrentPos();
				InsertText(pos, indent);
				GotoPos((int)(pos + indent.Length()));
				ChooseCaretX();
			}
		}

		EndUndoAction();
	}
}

void CodeEditor::OnUpdateUI(wxScintillaEvent &event)
{
	HighlightBraces();
}

bool CodeEditor::LoadFile() 
{
	// Get filname
	if (!mFileName)
	{
		wxFileDialog dlg (this, _T("Open file"), _T(""), _T(""),
			_T("Any file (*)|*"), wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);
		if (dlg.ShowModal() != wxID_OK) return false;
		mFileName = dlg.GetPath();
	}

	// Load file
	return LoadFile(mFileName);
}

bool CodeEditor::LoadFile(const wxString &filename)
{
	// Load file in edit and clear undo
	if (!filename.IsEmpty()) mFileName = filename;
	if (!wxScintilla::LoadFile(mFileName)) return false;

	// Determine lexer language
	//wxFileName fname(mFileName);
	//InitializePrefs(DeterminePrefs(fname.GetFullName()));

	return true;
}

bool CodeEditor::SaveFile()
{
	// Return if no change
	if (!Modified()) return true;

	// Get file name
	if (!mFileName)
	{
		wxFileDialog dlg (this, _T("Save file"), _T(""), _T(""), _T("Any file (*)|*"),
			wxSAVE | wxOVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) return false;
		mFileName = dlg.GetPath();
	}

	// Save file
	return SaveFile(mFileName);
}

bool CodeEditor::SaveFile(const wxString &filename)
{
	// Return if no change
	if (!Modified()) return true;

	return wxScintilla::SaveFile(filename);
}

bool CodeEditor::Modified()
{
	// Return modified state
	return (GetModify() && !GetReadOnly());
}