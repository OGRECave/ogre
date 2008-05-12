#ifndef _CODEEDITOR_H_
#define _CODEEDITOR_H_

#include <wx/wxscintilla.h>

class CodeEditor : public wxScintilla
{
public:
	CodeEditor(wxWindow *parent, wxWindowID id = -1,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxVSCROLL
		);

	~CodeEditor();

	void OnSize(wxSizeEvent &event);
	// Edit
	void OnEditRedo(wxCommandEvent &event);
	void OnEditUndo(wxCommandEvent &event);
	void OnEditClear(wxCommandEvent &event);
	void OnEditCut(wxCommandEvent &event);
	void OnEditCopy(wxCommandEvent &event);
	void OnEditPaste(wxCommandEvent &event);
	// Find
	void OnFind(wxCommandEvent &event);
	void OnFindNext(wxCommandEvent &event);
	void OnReplace(wxCommandEvent &event);
	void OnReplaceNext(wxCommandEvent &event);
	void OnBraceMatch(wxCommandEvent &event);
	void OnGoto(wxCommandEvent &event);
	void OnEditIndentInc(wxCommandEvent &event);
	void OnEditIndentRed(wxCommandEvent &event);
	void OnEditSelectAll(wxCommandEvent &event);
	void OnEditSelectLine(wxCommandEvent &event);
	// View
	void OnHilightLang(wxCommandEvent &event);
	void OnDisplayEOL(wxCommandEvent &event);
	void OnIndentGuide(wxCommandEvent &event);
	void OnLineNumber(wxCommandEvent &event);
	void OnLongLineOn(wxCommandEvent &event);
	void OnWhiteSpace(wxCommandEvent &event);
	void OnFoldToggle(wxCommandEvent &event);
	void OnSetOverType(wxCommandEvent &event);
	void OnSetReadOnly(wxCommandEvent &event);
	void OnWrapModeOn(wxCommandEvent &event);
	void OnUseCharset(wxCommandEvent &event);
	// Extra
	void OnChangeCase(wxCommandEvent &event);
	void OnConvertEOL(wxCommandEvent &event);
	// Styled text
	void OnMarginClick(wxScintillaEvent &event);
	void OnCharAdded(wxScintillaEvent &event);
	void OnUpdateUI(wxScintillaEvent &event);

	// Language/Lexer
	//wxString DeterminePrefs (const wxString &filename);
	//bool InitializePrefs (const wxString &filename);
	//bool UserSettings (const wxString &filename);
	//LanguageInfo const* GetLanguageInfo () {return m_language;};

	// Load/Save file
	bool LoadFile();
	bool LoadFile(const wxString &filename);
	bool SaveFile();
	bool SaveFile(const wxString &filename);
	bool Modified();
	wxString GetFilename() {return mFileName;};
	void SetFilename(const wxString &filename) { mFileName = filename; };

protected:
	wxChar GetLastNonWhitespaceChar(int position = -1);
	wxString CodeEditor::GetLineIndentString(int line);
	int FindBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested = true);

	void HighlightBraces();

private:
	// File
	wxString mFileName;

	// lanugage properties
	//LanguageInfo const* m_language;

	// Margin variables
	int mLineNumID;
	int mLineNumMargin;
	int mFoldingID;
	int mFoldingMargin;
	int mDividerID;

	DECLARE_EVENT_TABLE()
};

#endif // _CODEEDITOR_H_

