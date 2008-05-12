/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "PropertiesPanel.h"

#include <boost/any.hpp>
#include <boost/bind.hpp>

#include <wx/button.h>
#include <wx/dcclient.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>

#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgrePass.h"
#include "OgreTechnique.h"

#include "EventArgs.h"
#include "MaterialController.h"
#include "MaterialEventArgs.h"
#include "MaterialPropertyGridPage.h"
#include "MaterialWizard.h"
#include "PassController.h"
#include "PassPropertyGridPage.h"
#include "Project.h"
#include "ProjectEventArgs.h"
#include "ProjectWizard.h"
#include "PassPropertyGridPage.h"
#include "SelectionEventArgs.h"
#include "SelectionService.h"
#include "TechniqueController.h"
#include "TechniqueEventArgs.h"
#include "TechniquePropertyGridPage.h"
#include "TechniqueWizard.h"
#include "Workspace.h"
#include "WorkspaceEventArgs.h"

BEGIN_EVENT_TABLE(PropertiesPanel, wxPanel)
END_EVENT_TABLE()

PropertiesPanel::PropertiesPanel(wxWindow* parent,
							   wxWindowID id /* = wxID_ANY */,
							   const wxPoint& pos /* = wxDefaultPosition */,
							   const wxSize& size /* = wxDefaultSize */,
							   long style /* = wxTAB_TRAVERSAL | wxNO_BORDER */,
							   const wxString& name /* = wxT("Workspace Panel")) */)
							   : wxPanel(parent, id, pos, size, style, name)
{
	mGridSizer = new wxGridSizer(1, 1, 0, 0);

	mPropertyGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION | wxPGMAN_DEFAULT_STYLE);

	// Adding a page sets target page to the one added, so
	// we don't have to call SetTargetPage if we are filling
	// it right after adding.
	//MaterialController* controller = new MaterialController((MaterialPtr)MaterialManager::getSingletonPtr()->create("Test", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));
	//TechniqueController* tc = controller->createTechnique();
	//PassController* pc = tc->createPass();
	//PassPropertyGridPage* page = new PassPropertyGridPage(pc);
	//TechniquePropertyGridPage* page = new TechniquePropertyGridPage(tc);
	//MaterialPropertyGridPage* page = new MaterialPropertyGridPage(controller);
	//mPropertyGrid->AddPage(wxEmptyString, wxPG_NULL_BITMAP, page);
	//page->populate();

	// For total safety, finally reset the target page.
	//mPropertyGrid->SetTargetPage(0);

	mGridSizer->Add(mPropertyGrid, 0, wxALL | wxEXPAND, 0);

	SetSizer(mGridSizer);
	Layout();

	SelectionService::getSingletonPtr()->subscribe(SelectionService::SelectionChanged, boost::bind(&PropertiesPanel::selectionChanged, this, _1));
	Workspace::getSingletonPtr()->subscribe(Workspace::ProjectRemoved, boost::bind(&PropertiesPanel::projectRemoved, this, _1));
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::selectionChanged(EventArgs& args)
{
	SelectionEventArgs sea = dynamic_cast<SelectionEventArgs&>(args);
	SelectionList selection = sea.getSelection();
	if(!selection.empty())
	{
		boost::any sel = selection.front();
		if(sel.type() == typeid(Project))
		{
		}
		else if(sel.type() == typeid(MaterialController*))
		{
			MaterialController* mc = any_cast<MaterialController*>(sel);

			MaterialPageIndexMap::iterator it = mMaterialPageIndexMap.find(mc);
			if(it != mMaterialPageIndexMap.end())
			{
				int index = mMaterialPageIndexMap[mc];
				mPropertyGrid->SelectPage(index);
			}
			else
			{
				MaterialPropertyGridPage* page = new MaterialPropertyGridPage(mc);

				int index = mPropertyGrid->AddPage(wxEmptyString, wxPG_NULL_BITMAP, page);
				page->populate();

				mMaterialPageIndexMap[mc] = index;

				mPropertyGrid->SelectPage(index);
			}
		}
		else if(sel.type() == typeid(TechniqueController*))
		{
			TechniqueController* tc = any_cast<TechniqueController*>(sel);

			TechniquePageIndexMap::iterator it = mTechniquePageIndexMap.find(tc);
			if(it != mTechniquePageIndexMap.end())
			{
				int index = mTechniquePageIndexMap[tc];
				mPropertyGrid->SelectPage(index);
			}
			else
			{
				TechniquePropertyGridPage* page = new TechniquePropertyGridPage(tc);

				int index = mPropertyGrid->AddPage(wxEmptyString, wxPG_NULL_BITMAP, page);
				page->populate();

				mTechniquePageIndexMap[tc] = index;

				mPropertyGrid->SelectPage(index);
			}
		}
		else if(sel.type() == typeid(PassController*))
		{
			PassController* pc = any_cast<PassController*>(sel);

			PassPageIndexMap::iterator it = mPassPageIndexMap.find(pc);
			if(it != mPassPageIndexMap.end())
			{
				int index = mPassPageIndexMap[pc];
				mPropertyGrid->SelectPage(index);
			}
			else
			{
				PassPropertyGridPage* page = new PassPropertyGridPage(pc);

				int index = mPropertyGrid->AddPage(wxEmptyString, wxPG_NULL_BITMAP, page);
				page->populate();

				mPassPageIndexMap[pc] = index;

				mPropertyGrid->SelectPage(index);
			}
		}

		mPropertyGrid->Refresh();
	}
}

void PropertiesPanel::projectRemoved(EventArgs& args)
{
	// Consider: Should this method also attempt to remove all
	//           of the page associated with this Projects, 
	//           Materials, Techniques, and Passes?
}

void PropertiesPanel::materialRemoved(EventArgs& args)
{
	// Consider: Should this method also attempt to remove all
	//           of the page associated with this Materials, Techniques,
	//           and Passes?

	ProjectEventArgs pea = dynamic_cast<ProjectEventArgs&>(args);
	MaterialController* mc = pea.getMaterial();

	MaterialPageIndexMap::iterator it = mMaterialPageIndexMap.find(mc);
	if(it != mMaterialPageIndexMap.end())
	{
		mPropertyGrid->RemovePage(mMaterialPageIndexMap[mc]);
		mMaterialPageIndexMap.erase(it);
	}
}

void PropertiesPanel::techniqueRemoved(EventArgs& args)
{
	// Consider: Should this method also attempt to remove all
	//           of the page associated with this Techniques, Passes?

	MaterialEventArgs mea = dynamic_cast<MaterialEventArgs&>(args);
	TechniqueController* tc = mea.getTechniqueController();

	TechniquePageIndexMap::iterator it = mTechniquePageIndexMap.find(tc);
	if(it != mTechniquePageIndexMap.end())
	{
		mPropertyGrid->RemovePage(mTechniquePageIndexMap[tc]);
		mTechniquePageIndexMap.erase(it);
	}
}

void PropertiesPanel::passRemoved(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	PassController* pc = tea.getPassController();

	PassPageIndexMap::iterator it = mPassPageIndexMap.find(pc);
	if(it != mPassPageIndexMap.end())
	{
		mPropertyGrid->RemovePage(mPassPageIndexMap[pc]);
		mPassPageIndexMap.erase(it);
	}
}
