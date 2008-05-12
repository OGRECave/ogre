#include "DocPanel.h"

#include <typeinfo>

#include <boost/bind.hpp>

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include "OgreString.h"

#include "CgEditor.h"
#include "Editor.h"
#include "EditorEventArgs.h"
#include "EditorManager.h"
#include "EventArgs.h"
#include "GLSLEditor.h"
#include "HLSLEditor.h"
#include "MaterialScriptEditor.h"
#include "ScintillaEditor.h"

DocPanel::DocPanel(wxWindow* parent, wxWindowID id /* = wxID_ANY */, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDeafultSize */, long style /* = wxTAB_TRAVERSAL */, const wxString& name /* =  */)
: wxPanel(parent, id, pos, size, style, name), mEditor(NULL)
{
	mBoxSizer = new wxBoxSizer(wxVERTICAL);

	mTextControl = new wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxSize(200,150), wxNO_BORDER | wxTE_MULTILINE);
	mTextControl->SetEditable(false);
	mTextControl->SetBackgroundColour(wxColor(255, 255, 225));
	mTextControl->AppendText("N/A");
	mBoxSizer->Add(mTextControl, 1, wxEXPAND | wxALL, 0);

	SetSizer(mBoxSizer);
	Layout();

	EditorManager::getSingletonPtr()->subscribe(EditorManager::ActiveEditorChanged,  boost::bind(&DocPanel::OnActiveEditorChanged, this, _1));
}

DocPanel::~DocPanel()
{
}

void DocPanel::OnActiveEditorChanged(EventArgs& args)
{
	EditorEventArgs eea = dynamic_cast<EditorEventArgs&>(args);
	Editor* editor = eea.getEditor();

	// TODO: Unsubscribe from previous editor

	// FIXME
	if(typeid(*editor) == typeid(ScintillaEditor) || 
	   typeid(*editor) == typeid(MaterialScriptEditor) ||
	   typeid(*editor) == typeid(CgEditor) ||
	   typeid(*editor) == typeid(GLSLEditor) ||
	   typeid(*editor) == typeid(HLSLEditor))
	{
		mEditor = dynamic_cast<ScintillaEditor*>(editor);
		mEditor->subscribe(ScintillaEditor::FocusedWordChanged, boost::bind(&DocPanel::OnFocusedWordChanged, this, _1));
	}
	else mEditor = NULL;
}

void DocPanel::OnFocusedWordChanged(EventArgs& args)
{
	mTextControl->Clear();
	if(mEditor != NULL)
	{
		ScintillaEditorEventArgs seea = dynamic_cast<ScintillaEditorEventArgs&>(args);
		wxString* doc = mEditor->getDocManager().find(seea.getFocusedWord());

		if(doc != NULL && !doc->IsEmpty() && (*doc) != "")
		{
			mTextControl->AppendText(*doc);
			return;
		}
	}

	mTextControl->AppendText("N/A");
}