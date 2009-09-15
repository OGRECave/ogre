/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#include "ScintillaEditor.h"

#include "OgreDataStream.h"

using Ogre::DataStream;
using Ogre::DataStreamPtr;
using Ogre::FileStreamDataStream;

BEGIN_EVENT_TABLE(ScintillaEditor, wxScintilla)
	EVT_SIZE (ScintillaEditor::OnSize)
	// Scintilla
	EVT_SCI_MARGINCLICK (-1, ScintillaEditor::OnMarginClick)
	EVT_SCI_CHARADDED (-1,   ScintillaEditor::OnCharAdded)
	EVT_SCI_UPDATEUI(-1, ScintillaEditor::OnUpdateUI)
END_EVENT_TABLE()

ScintillaEditor::ScintillaEditor(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
: wxScintilla (parent, id, pos, size, style), mDirty(false)
{
	registerEvent(FocusedWordChanged);

	setControl(this);
	
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

	// Brace Hilighting
	StyleSetBackground(wxSCI_STYLE_BRACELIGHT, wxColour(wxT("GREEN")));
	StyleSetBold(wxSCI_STYLE_BRACELIGHT, true);
	StyleSetBold(wxSCI_STYLE_BRACEBAD, true);

	// Call Tips
	CallTipSetBackground(wxColor(255, 255, 225));
	CallTipSetForeground(wxColor(128, 128, 128));
	CallTipSetForegroundHighlight(wxColor(0, 0, 0));

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

	UsePopUp(0);
	SetLayoutCache(wxSCI_CACHE_PAGE);
	SetBufferedDraw(1);
}

ScintillaEditor::~ScintillaEditor() 
{
}

void ScintillaEditor::activate()
{
	// TODO: Connect to update ui event
}

void ScintillaEditor::deactivate()
{
	// TODO: Disconnect from update ui event
}

bool ScintillaEditor::isDirty()
{
	return mDirty;
}

void ScintillaEditor::save()
{
	if (!mDirty) return;
	if(!mFileName || mFileName.length() == 0)
	{
		saveAs();
		return;
	}

	if(SaveFile(mFileName)) 
	{
		setDirty(false);
		int index = (int)mFileName.find_last_of('\\');
		if(index == -1) index = (int)mFileName.find_last_of('/');
		setName((index != -1) ? mFileName.substr(index + 1, mFileName.Length()) : mFileName);
	}
}

void ScintillaEditor::saveAs()
{
	if (!mDirty) return;

	// Get file name
	wxFileDialog dlg (this, _T("Save file"), _T(""), _T(""), _T("Any file (*)|*"),
		wxSAVE | wxOVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return;
	mFileName = dlg.GetPath();

	save();
}

bool ScintillaEditor::isSaveAsAllowed()
{
	return true;
}

bool ScintillaEditor::isRedoable()
{
	return CanRedo();
}

void ScintillaEditor::redo()
{
	if(!CanRedo()) return;
	
	Redo();
}

bool ScintillaEditor::isUndoable()
{
	return CanUndo();
}

void ScintillaEditor::undo()
{
	if(!CanUndo()) return;
	
	Undo();
}

bool ScintillaEditor::isCuttable()
{
	return GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0);
}

void ScintillaEditor::cut()
{
	if(GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0)) return;
	
	Cut();
}

bool ScintillaEditor::isCopyable()
{
	return GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0);
}

void ScintillaEditor::copy()
{
	if(GetSelectionEnd() - GetSelectionStart() <= 0) return;
	
	Copy();
}

bool ScintillaEditor::isPastable()
{
	return CanPaste();
}

void ScintillaEditor::paste()
{
	if(!CanPaste()) return;
	
	Paste();
}

bool ScintillaEditor::loadFile() 
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

bool ScintillaEditor::loadFile(const wxString &filename)
{
	// Load file in edit and clear undo
	if (!filename.IsEmpty()) mFileName = filename;
	if (!wxScintilla::LoadFile(mFileName)) return false;

	return true;
}

void ScintillaEditor::loadKeywords(wxString& path)
{
	std::ifstream fp;
	fp.open(path, std::ios::in | std::ios::binary);
	if(fp)
	{
		DataStreamPtr stream(new FileStreamDataStream(path.c_str(), &fp, false));
		
		int index = -1;
		String line;
		wxString keywords;
		while(!stream->eof())
		{
			line = stream->getLine();
			
			// Ignore blank lines and comments (comment lines start with '#')
			if(line.length() > 0 && line.at(0) != '#')
			{
				if(line.at(0) == '[')
				{
					if(index != -1)
					{
						SetKeyWords(index, keywords);
						keywords.clear();
					}
					
					++index;
				}
				else
				{
					keywords.Append(line);
					keywords.Append(" ");
				}
			}
		}
		
		SetKeyWords(index, keywords);
	}
}

CallTipManager& ScintillaEditor::getCallTipManager()
{
	return mCallTipManager;
}

DocManager& ScintillaEditor::getDocManager()
{
	return mDocManager;
}

wxString ScintillaEditor::getSurroundingWord(int pos /* = -1 */)
{
	if(pos == -1) pos = GetCurrentPos();

	int lineNum = GetCurrentLine();
	wxString word("");
	if(lineNum != -1)
	{
		wxString line = GetLine(lineNum);

		wxChar ch;
		while(pos)
		{
			ch = GetCharAt(--pos);
			if(ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '.' || ch == '(' || ch == ',' || ch == ';') break;
		}

		if(!pos) return word;

		while(pos < line.Length())
		{
			ch = GetCharAt(++pos);
			if(ch != ' ' && ch != ')' && ch!= '\n' && ch!= '\r' && ch !='\t' && ch != '.' && ch != '(' && ch != ')' && ch != ';' && ch != ',') word += ch;
			else break;
		}
	}

	return word;
}

wxChar ScintillaEditor::getLastNonWhitespaceChar(int position /* = -1 */)
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

wxString ScintillaEditor::getLineIndentString(int line)
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

int ScintillaEditor::findBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested /* = true */)
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

void ScintillaEditor::highlightBraces()
{
	int p = GetCurrentPos();
	int c1 = GetCharAt(p);
	int c2 = (p > 1 ? GetCharAt(p - 1) : 0);
	if(c2 == '(' || c2 == ')' || c1 == '(' || c1 == ')')
	{
		int start = (c2 == '(' || c2 == ')') ? p - 1 : p;
		int end = BraceMatch(start);
		if(end == wxSCI_INVALID_POSITION)
		{
			BraceBadLight(start);
		}
		else
		{
			BraceHighlight(start, end);
		}
	}
	else if(c2 == '{' || c2 == '}' || c1 == '{' || c1 == '}')
	{
		int start = (c2 == '{' || c2 == '}') ? p - 1 : p;
		int end = BraceMatch(start);
		if(end == wxSCI_INVALID_POSITION)
		{
			BraceBadLight(start);
		}
		else
		{
			BraceHighlight(start, end);
		}
	}
	else
	{
		BraceBadLight(wxSCI_INVALID_POSITION);
	}
	/*
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
	*/
}

//----------------------------------------------------------------------------
// Common event handlers
void ScintillaEditor::OnSize(wxSizeEvent& event)
{
	int x = GetClientSize().x;// + GetMarginLeft(); //mLineNumMargin + mFoldingMargin;

	if (x > 0) SetScrollWidth(x);

	event.Skip();
}


void ScintillaEditor::OnMarginClick(wxScintillaEvent &event)
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

void ScintillaEditor::OnCharAdded(wxScintillaEvent &event)
{
	char ch = event.GetKey();
	int currentLine = GetCurrentLine();
	int pos = GetCurrentPos();

	if (ch == wxT('\n') && currentLine > 0)
	{
		BeginUndoAction();

		wxString indent = getLineIndentString(currentLine - 1);

		wxChar b = getLastNonWhitespaceChar();
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
			pos = findBlockStart(pos, wxT('{'), wxT('}'));

			if(pos != -1)
			{
				wxString indent = getLineIndentString(LineFromPosition(pos));
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

	setDirty(true);
}

void ScintillaEditor::OnUpdateUI(wxScintillaEvent &event)
{
	highlightBraces();

	int pos = GetCurrentPos();
	if(pos != mLastPos)
	{
		mLastPos = pos;
		wxString word = getSurroundingWord(pos);
		if(word != mLastWord)
		{
			mLastWord = word;
			fireEvent(FocusedWordChanged, ScintillaEditorEventArgs(this, word));
		}
	}
}

void ScintillaEditor::setDirty(const bool dirty)
{
	if(mDirty == dirty) return;
	mDirty = dirty;
	//fireEvent(DirtyStateChanged, new EditorEventArgs(this, getEditorInput()));
}
