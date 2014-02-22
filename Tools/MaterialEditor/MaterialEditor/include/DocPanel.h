#ifndef _DOCPANEL_H_
#define _DOCPANEL_H_

#include <wx/panel.h>

#include "OgreString.h"

class wxBoxSizer;
class wxTextCtrl;

class EventArgs;
class ScintillaEditor;

using Ogre::String;

class DocPanel : public wxPanel
{
public:
    DocPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "LogPanel");
    ~DocPanel();

    void OnActiveEditorChanged(EventArgs& args);
    void OnFocusedWordChanged(EventArgs& args);

protected:
    wxBoxSizer* mBoxSizer;
    wxTextCtrl* mTextControl;

    ScintillaEditor* mEditor;
};

#endif // _DOCPANEL_H_