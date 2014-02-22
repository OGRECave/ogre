#ifndef _SCINTILLAEDITOR_H_
#define _SCINTILLAEDITOR_H_

#include <wx/wxscintilla.h>

#include "CallTipManager.h"
#include "DocManager.h"
#include "Editor.h"
#include "EventArgs.h"

class ScintillaEditor;

class ScintillaEditorEventArgs : public EventArgs
{
public:
    ScintillaEditorEventArgs(ScintillaEditor* editor, wxString word) : mEditor(editor), mFocusedWord(word) {}
    ~ScintillaEditorEventArgs() {}

    ScintillaEditor* getEditor() { return mEditor; }
    wxString& getFocusedWord() { return mFocusedWord; }

protected:
    ScintillaEditor* mEditor;
    wxString mFocusedWord;
};

class ScintillaEditor : public wxScintilla, public Editor
{
public:
    enum ScintillaEditorEvent
    {
        NameChanged, // Editor Event
        DirtyStateChanged, // Editor Event
        FocusedWordChanged
    };

    ScintillaEditor(wxWindow* parent, wxWindowID id = -1,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL
        );

    virtual ~ScintillaEditor();

    virtual void activate();
    virtual void deactivate();

    DocManager& getDocManager();
    CallTipManager& getCallTipManager();

    virtual bool isDirty();
    virtual void save();
    virtual void saveAs();
    virtual bool isSaveAsAllowed();

    virtual bool isRedoable();
    virtual void redo();
    virtual bool isUndoable();
    virtual void undo();

    virtual bool isCuttable();
    virtual void cut();
    virtual bool isCopyable();
    virtual void copy();
    virtual bool isPastable();
    virtual void paste(); 
    
    virtual void loadKeywords(wxString& path);

    virtual bool loadFile();
    virtual bool loadFile(const wxString &filename);

    void OnSize(wxSizeEvent &event);
    void OnMarginClick(wxScintillaEvent &event);
    void OnCharAdded(wxScintillaEvent &event);
    void OnUpdateUI(wxScintillaEvent &event);

protected:
    wxString getSurroundingWord(int pos = -1);
    wxChar getLastNonWhitespaceChar(int position = -1);
    wxString getLineIndentString(int line);
    int findBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested = true);

    void highlightBraces();

    void setDirty(bool dirty);

    CallTipManager mCallTipManager;
    DocManager mDocManager;

    bool mDirty;

    int mLastPos;
    wxString mLastWord;

    // File
    wxString mFileName;

private:
    // Margin variables
    int mLineNumID;
    int mLineNumMargin;
    int mFoldingID;
    int mFoldingMargin;
    int mDividerID;

    DECLARE_EVENT_TABLE()
};

#endif // _SCINTILLAEDITOR_H_


