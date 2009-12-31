/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
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
