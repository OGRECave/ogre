/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include <QuickTime/QuickTime.h>
#include "OgreLogManager.h"
#include "OgreConfigDialog.h"

namespace Ogre {

	ConfigDialog* dlg = NULL;
	ConfigDialog::ConfigDialog() 
	{
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
	}
	
	void ConfigDialog::initialise()
	{
	}
	
	bool ConfigDialog::display()
	{
		// TODO: Fix OS X Config dialog
		const RenderSystemList& renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers.front();

		// WARNING: restoreConfig() should not be invoked here as Root calls
		// it before this method anyway, and invoking restoreConfig() here
		// forces the client application to use Ogre.cfg, while it may have
		// different plans.
		if(!Root::getSingleton().restoreConfig())
		{
			// Set some defaults
			renderer->setConfigOption("Video Mode", "800 x 600");
			renderer->setConfigOption("Colour Depth", "32");
			renderer->setConfigOption("FSAA", "0");
			renderer->setConfigOption("Full Screen", "No");
			renderer->setConfigOption("RTT Preferred Mode", "FBO");
			// Set the rendersystem and save the config.
			Root::getSingleton().setRenderSystem(renderer);
		}
		return true;
	}

};
