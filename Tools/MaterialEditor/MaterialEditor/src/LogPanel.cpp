#include "LogPanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

#include "OgreString.h"
#include "LogToTextRedirector.h"


LogPanel::LogPanel(wxWindow* parent, wxWindowID id /* = wxID_ANY */, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDeafultSize */, long style /* = wxTAB_TRAVERSAL */, const wxString& name /* =  */)
: wxPanel(parent, id, pos, size, style, name)
{
    mBoxSizer = new wxBoxSizer(wxVERTICAL);
    
    mTextControl = new wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxSize(200,150), wxNO_BORDER | wxTE_MULTILINE);
    mTextControl->SetEditable(false);
    mBoxSizer->Add(mTextControl, 1, wxEXPAND | wxALL, 0);

    SetSizer(mBoxSizer);
    Layout();
}

LogPanel::~LogPanel()
{}

void LogPanel::attachLog(Ogre::Log *log)
{
    log->addListener(this);
}

void LogPanel::detachLog(Ogre::Log *log)
{
    log->removeListener(this);
}

void LogPanel::messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName)
{
    if(lml == Ogre::LML_CRITICAL || lml == Ogre::LML_NORMAL)
    {
        mTextControl->AppendText(wxT(message.c_str()));
        mTextControl->AppendText(wxT("\n"));
    }
}
