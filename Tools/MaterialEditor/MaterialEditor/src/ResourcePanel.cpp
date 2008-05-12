#include "ResourcePanel.h"

#include <wx/sizer.h>
#include <wx/treectrl.h>

#include "OgreResourceGroupManager.h"
#include "OgreResourceManager.h"
#include "OgreStringVector.h"

using Ogre::ResourceGroupManager;
using Ogre::ResourceManager;
using Ogre::StringVector;

ResourcePanel::ResourcePanel(wxWindow* parent, wxWindowID id /* = wxID_ANY */, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDeafultSize */, long style /* = wxTAB_TRAVERSAL */, const wxString& name /* =  */)
: wxPanel(parent, id, pos, size, style, name)
{
	mBoxSizer = new wxBoxSizer(wxVERTICAL);

	mTreeControl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	mBoxSizer->Add(mTreeControl, 1, wxEXPAND | wxALL);

	wxTreeItemId rootId = mTreeControl->AddRoot("Resources");
	
	StringVector groups = ResourceGroupManager::getSingletonPtr()->getResourceGroups();
	for(StringVector::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		wxTreeItemId groupId = mTreeControl->AppendItem(rootId, *it);
		ResourceGroupManager::ResourceDeclarationList resources = ResourceGroupManager::getSingletonPtr()->getResourceDeclarationList(*it);
		for(ResourceGroupManager::ResourceDeclarationList::iterator rit = resources.begin(); rit != resources.end(); ++rit)
		{
			mTreeControl->AppendItem(groupId, (*rit).resourceName);
		}
	}
	

	SetSizer(mBoxSizer);
	Layout();
}

ResourcePanel::~ResourcePanel()
{}